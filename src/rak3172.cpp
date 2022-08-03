 /*
 * rak3172.cpp
 *
 *  Copyright (C) Daniel Kampert, 2022
 *	Website: www.kampis-elektroecke.de
 *  File info: RAK3172 driver for ESP32.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 * Errors and commissions should be reported to DanielKampert@kampis-elektroecke.de
 */

#include <esp_log.h>

#include <algorithm>

#include <sdkconfig.h>

#include "rak3172.h"

#define STRINGIFY(s)                            STR(s)
#define STR(s)                                  #s

#ifndef CONFIG_RAK3172_TASK_PRIO
    #define CONFIG_RAK3172_TASK_PRIO            12
#endif

#ifndef CONFIG_RAK3172_TASK_BUFFER_SIZE
    #define CONFIG_RAK3172_TASK_BUFFER_SIZE     1024
#endif

#ifndef CONFIG_RAK3172_TASK_QUEUE_LENGTH
    #define CONFIG_RAK3172_TASK_QUEUE_LENGTH    8
#endif

#ifdef CONFIG_RAK3172_RESET_USE_HW
    static gpio_config_t _RAK3172_Reset_Config = {
        .pin_bit_mask       = 0,
        .mode               = GPIO_MODE_OUTPUT,
        .pull_up_en         = GPIO_PULLUP_DISABLE,
        .pull_down_en       = GPIO_PULLDOWN_DISABLE,
        .intr_type          = GPIO_INTR_DISABLE,
    };
#endif

static uart_config_t _RAK3172_UART_Config = {
    .baud_rate              = 9600,
    .data_bits              = UART_DATA_8_BITS,
    .parity                 = UART_PARITY_DISABLE,
    .stop_bits              = UART_STOP_BITS_1,
    .flow_ctrl              = UART_HW_FLOWCTRL_DISABLE,
    .rx_flow_ctrl_thresh    = 0,
#if(defined CONFIG_PM_ENABLE) && (defined CONFIG_IDF_TARGET_ESP32)
    #if SOC_UART_SUPPORT_REF_TICK
        .source_clk         = UART_SCLK_REF_TICK,
    #else
        .source_clk         = UART_SCLK_RTC,
    #endif
#else
    .source_clk             = UART_SCLK_APB,
#endif
};

static const char* TAG      = "RAK3172";

/** @brief          Receive the splash screen after a reset.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param Timeout  (Optional) Timeout for the splash screen in milliseconds
 *  @return         RAK3172_ERR_OK when successful
 */
static RAK3172_Error_t RAK3172_ReceiveSplashScreen(RAK3172_t* const p_Device, uint16_t Timeout = 1000)
{
    std::string* Response = NULL;

    do
    {
        if(xQueueReceive(p_Device->Internal.MessageQueue, &Response, Timeout / portTICK_PERIOD_MS) != pdPASS)
        {
            ESP_LOGE(TAG, "     Timeout!");

            p_Device->Internal.isBusy = false;

            return RAK3172_ERR_TIMEOUT;
        }

        // The driver is compiled for RUI3, but an older splash screen was received (version 1.4.0 and below).
        #ifdef CONFIG_RAK3172_USE_RUI3
            if(Response->find("Version.") != std::string::npos)
            {
                ESP_LOGE(TAG, "Firmware compiled for RUI3, but module firmware does not support RUI3!");

                p_Device->Internal.isBusy = false;

                return RAK3172_ERR_INVALID_RESPONSE;
            }
        #endif

        if((Response->find("LoRaWAN.") != std::string::npos) || (Response->find("LoRa P2P.") != std::string::npos))
        {
            p_Device->Internal.isBusy = false;
        }

        delete Response;
    } while(p_Device->Internal.isBusy);

    return RAK3172_ERR_OK;
}

/** @brief          UART receive task.
 *  @param p_Arg    Pointer to task arguments
 */
static void RAK3172_UART_EventTask(void* p_Arg)
{
    uart_event_t Event;
    RAK3172_t* Device = (RAK3172_t*)p_Arg;

    while(true)
    {
        if(xQueueReceive(Device->Internal.EventQueue, (void*)&Event, 20))
        {
            size_t BufferedSize;
            uint32_t PatternPos;

            switch(Event.type)
            {
                case UART_PATTERN_DET:
                {
                    uart_get_buffered_data_len(Device->Interface, &BufferedSize);

                    PatternPos = uart_pattern_pop_pos(Device->Interface);

                    if(PatternPos == -1)
                    {
                        uart_flush_input(Device->Interface);
                    }
                    else
                    {
                        std::string* Response = new std::string();

                        ESP_LOGD(TAG, "     Pattern detected at position %u. Use buffered size: %u", PatternPos, BufferedSize);

                        uart_read_bytes(Device->Interface, Device->Internal.RxBuffer, PatternPos, 10);

                        // Copy the data from the buffer into the string.
                        for(uint32_t i = 0; i < PatternPos; i++)
                        {
                            char Character = (char)Device->Internal.RxBuffer[i];

                            if((Character != '\n') && (Character != '\r'))
                            {
                                *Response += Character;
                            }
                        }

                        ESP_LOGD(TAG, "     Response: %s", Response->c_str());

                        #ifdef CONFIG_RAK3172_MODE_WITH_LORAWAN
                            if((Device->Mode == RAK_MODE_LORAWAN) && (Response->find("+EVT") != std::string::npos))
                            {
                                ESP_LOGD(TAG, "Event: %s", Response->c_str());

                                // Join was successful
                                if(Response->find("JOINED") != std::string::npos)
                                {
                                    ESP_LOGD(TAG, "Joined...");

                                    #ifdef CONFIG_RAK3172_USE_RUI3
                                        Device->Internal.isBusy = false;
                                    #endif

                                    Device->LoRaWAN.isJoined = true;
                                }
                                // Join failed.
                                #ifdef CONFIG_RAK3172_USE_RUI3
                                    else if(Response->find("JOIN_FAILED_RX_TIMEOUT") != std::string::npos)
                                #else
                                    else if(Response->find("JOIN FAILED") != std::string::npos)
                                #endif
                                {
                                    ESP_LOGD(TAG, "Not joined...");

                                    #ifdef CONFIG_RAK3172_USE_RUI3
                                        Device->Internal.isBusy = false;
                                    #else
                                        if(Device->LoRaWAN.AttemptCounter > 0)
                                        {
                                            Device->LoRaWAN.AttemptCounter--;
                                        }
                                    #endif

                                    Device->LoRaWAN.isJoined = false;
                                }
                                // Transmission failed.
                                #ifdef CONFIG_RAK3172_USE_RUI3
                                    else if(Response->find("SEND_CONFIRMED_FAILED") != std::string::npos)
                                #else
                                    else if(Response->find("SEND CONFIRMED FAILED") != std::string::npos)
                                #endif
                                {
                                    Device->Internal.isBusy = false;
                                    Device->LoRaWAN.ConfirmError = true;
                                }
                                // Transmission was successful.
                                #ifdef CONFIG_RAK3172_USE_RUI3
                                    else if(Response->find("SEND_CONFIRMED_OK") != std::string::npos)
                                #else
                                    else if(Response->find("SEND CONFIRMED OK") != std::string::npos)
                                #endif
                                {
                                    Device->Internal.isBusy = false;
                                    Device->LoRaWAN.ConfirmError = false;
                                }
                                else if(Response->find("RX") != std::string::npos)
                                {
                                    std::string Dummy;
                                    RAK3172_Rx_t* Received = new RAK3172_Rx_t();

                                    // Remove "+EVT:RX_x:" from the response.
                                    Response->erase(Response->find("+EVT:"), std::string("+EVT:").length() + 5);

                                    // Get the RSSI value.
                                    #ifdef CONFIG_RAK3172_USE_RUI3
                                        Dummy = Response->substr(0, Response->find(":"));
                                        Response->erase(0, std::string(Dummy + ":").length());
                                    #else
                                        Dummy = Response->substr(std::string("RSSI").length() + 1, Response->find(","));
                                        Response->erase(0, std::string(Dummy + ":").length() + 1);
                                    #endif
                                    Received->RSSI = std::stoi(Dummy);

                                    // Get the SNR value.
                                    #ifdef CONFIG_RAK3172_USE_RUI3
                                        Dummy = Response->substr(0, Response->find(":"));
                                        Response->erase(0, std::string(Dummy + ":").length());
                                    #else
                                        Dummy = Response->substr(std::string("SNR").length() + 1, Response->find(","));
                                    #endif
                                    Received->SNR = std::stoi(Dummy);

                                    #ifndef CONFIG_RAK3172_USE_RUI3
                                        // The payload is stored in the next line.
                                        char Data;
                                        int Bytes;

                                        Response->clear();
                                        do
                                        {
                                            Bytes = uart_read_bytes(Device->Interface, &Data, 1, 10);
                                            if((Bytes != 0) && (Data != '\r') && (Data != '\n'))
                                            {
                                                *Response += Data;
                                            }
                                        } while(Bytes != 0);

                                        ESP_LOGD(TAG, "Next line: %s", Response->c_str());

                                        // Remove all "+EVT" strings.
                                        Response->erase(0, std::string("+EVT:").length());
                                        Response->erase(Response->find("+EVT"), std::string("+EVT").length());
                                    #endif

                                    // Remove the "UNICAST" or the "MULCAST" from the message.
                                    Response->erase(0, Response->find(":") + 1);

                                    // Get the communication port.
                                    Dummy = Response->substr(0, Response->find(":"));
                                    Response->erase(Response->find(Dummy), std::string(Dummy + ":").length());
                                    Received->Port = std::stoi(Dummy);

                                    // Get the payload.
                                    Received->Payload = *Response;

                                    ESP_LOGD(TAG, "RSSI: %i", Received->RSSI);
                                    ESP_LOGD(TAG, "SNR: %i", Received->SNR);
                                    ESP_LOGD(TAG, "Port: %u", Received->Port);
                                    ESP_LOGD(TAG, "Payload: %s", Received->Payload.c_str());

                                    xQueueSend(Device->Internal.ReceiveQueue, &Received, 0);
                                }

                                delete Response;
                            }
                        #endif
                        #ifdef CONFIG_RAK3172_MODE_WITH_P2P
                            if((Device->Mode == RAK_MODE_P2P) && (Response->find("+EVT") != std::string::npos))
                            {
                                ESP_LOGD(TAG, "Event: %s", Response->c_str());

                                if(Response->find("+EVT:RXP2P RECEIVE TIMEOUT") != std::string::npos)
                                {
                                    Device->P2P.isRxTimeout = true;
                                }
                                else if(Response->find("RX") != std::string::npos)
                                {
                                    std::string Dummy;
                                    RAK3172_Rx_t* Received = new RAK3172_Rx_t();

                                    // Remove "+EVT:RXP2P:" from the response.
                                    Response->erase(Response->find("+EVT:"), std::string("+EVT:").length() + 6);

                                    // Get the RSSI value.
                                    #ifdef CONFIG_RAK3172_USE_RUI3
                                        Dummy = Response->substr(0, Response->find(":"));
                                        Response->erase(0, std::string(Dummy + ":").length());
                                    #else
                                        Dummy = Response->substr(std::string("RSSI").length() + 1, Response->find(","));
                                        Response->erase(0, std::string(Dummy + ":").length() + 1);
                                    #endif
                                    Received->RSSI = std::stoi(Dummy);

                                    // Get the SNR value.
                                    #ifdef CONFIG_RAK3172_USE_RUI3
                                        Dummy = Response->substr(0, Response->find(":"));
                                        Response->erase(0, std::string(Dummy + ":").length());
                                    #else
                                        Dummy = Response->substr(std::string("SNR").length() + 1, Response->find(","));
                                    #endif
                                    Received->SNR = std::stoi(Dummy);

                                    // Get the payload.
                                    Received->Payload = *Response;

                                    ESP_LOGD(TAG, "RSSI: %i", Received->RSSI);
                                    ESP_LOGD(TAG, "SNR: %i", Received->SNR);
                                    ESP_LOGD(TAG, "Payload: %s", Received->Payload.c_str());

                                    xQueueSend(Device->Internal.ReceiveQueue, &Received, 0);
                                }
                            }
                            // Any other messages from the module.
                            else
                        #endif
                        {
                            xQueueSend(Device->Internal.MessageQueue, &Response, 0);
                        }
                    }

                    break;
                }
                default:
                {
                    break;
                }
            }
        }
    }
}

/** @brief          Perform the basic initialization of the driver.
 *  @param p_Device Pointer to RAK3172 device object
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_NO_MEM when the message queues, the task or the receive buffer can not be created
 */
static RAK3172_Error_t RAK3172_BasicInit(RAK3172_t* const p_Device)
{
    RAK3172_Error_t Error;

    esp_log_level_set("uart", ESP_LOG_NONE);

    if(p_Device->Tx == p_Device->Rx)
    {
        ESP_LOGE(TAG, "Invalid Rx and Tx for UART!");

        return RAK3172_ERR_INVALID_ARG;
    }

    if(uart_driver_install(p_Device->Interface, CONFIG_RAK3172_TASK_BUFFER_SIZE * 2, CONFIG_RAK3172_TASK_BUFFER_SIZE * 2, CONFIG_RAK3172_TASK_QUEUE_LENGTH, &p_Device->Internal.EventQueue, 0) ||
       uart_param_config(p_Device->Interface, &_RAK3172_UART_Config) ||
       uart_set_pin(p_Device->Interface, p_Device->Tx, p_Device->Rx, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE) ||
       uart_enable_pattern_det_baud_intr(p_Device->Interface, '\n', 1, 1, 0, 0) ||
       uart_pattern_queue_reset(p_Device->Interface, CONFIG_RAK3172_TASK_QUEUE_LENGTH))
    {
        return RAK3172_ERR_INVALID_STATE;
    }

    p_Device->Internal.MessageQueue = xQueueCreate(CONFIG_RAK3172_TASK_QUEUE_LENGTH, sizeof(std::string*));
    if(p_Device->Internal.MessageQueue == NULL)
    {
        Error = RAK3172_ERR_NO_MEM;

        goto RAK3172_BasicInit_Error_1;
    }

    p_Device->Internal.ReceiveQueue = xQueueCreate(8, sizeof(RAK3172_Rx_t*));
    if(p_Device->Internal.ReceiveQueue == NULL)
    { 
        Error = RAK3172_ERR_NO_MEM;

        goto RAK3172_BasicInit_Error_2;
    }

    p_Device->Internal.RxBuffer = (uint8_t*)malloc(CONFIG_RAK3172_TASK_BUFFER_SIZE);
    if(p_Device->Internal.RxBuffer == NULL)
    {
        Error = RAK3172_ERR_NO_MEM;

        goto RAK3172_BasicInit_Error_3;
    }

    #ifdef CONFIG_RAK3172_TASK_CORE_AFFINITY
        xTaskCreatePinnedToCore(RAK3172_UART_EventTask, "RAK3172_Event", CONFIG_RAK3172_TASK_BUFFER_SIZE * 2, p_Device, CONFIG_RAK3172_TASK_PRIO, &p_Device->Internal.Handle, CONFIG_RAK3172_TASK_CORE);
    #else
        xTaskCreate(RAK3172_UART_EventTask, "RAK3172_Event", CONFIG_RAK3172_TASK_BUFFER_SIZE * 2, p_Device, CONFIG_RAK3172_TASK_PRIO, &p_Device->Internal.Handle);
    #endif

    if(p_Device->Internal.Handle == NULL)
    {
        Error = RAK3172_ERR_NO_MEM;

        goto RAK3172_BasicInit_Error_4;
    }

    if(uart_flush(p_Device->Interface))
    {
        Error = RAK3172_ERR_INVALID_STATE;

        goto RAK3172_BasicInit_Error_4;
    }

    xQueueReset(p_Device->Internal.MessageQueue);
    p_Device->Internal.isInitialized = true;

    return RAK3172_ERR_OK;

RAK3172_BasicInit_Error_4:
    vTaskSuspend(p_Device->Internal.Handle);
    vTaskDelete(p_Device->Internal.Handle);

RAK3172_BasicInit_Error_3:
    free(p_Device->Internal.RxBuffer);

RAK3172_BasicInit_Error_2:
    vQueueDelete(p_Device->Internal.ReceiveQueue);

RAK3172_BasicInit_Error_1:
    vQueueDelete(p_Device->Internal.MessageQueue);

	p_Device->Internal.isInitialized = false;

    return Error;
}

RAK3172_Error_t RAK3172_Init(RAK3172_t* const p_Device)
{
    std::string Response;

    if(p_Device == NULL)
    {
        return RAK3172_ERR_INVALID_ARG;
    }

    p_Device->Internal.isInitialized = false;
    p_Device->Internal.isBusy = false;

    _RAK3172_UART_Config.baud_rate = p_Device->Baudrate;

    ESP_LOGI(TAG, "UART config:");
    ESP_LOGI(TAG, "     Interface: %u", p_Device->Interface);
    ESP_LOGI(TAG, "     Buffer size: %u",CONFIG_RAK3172_TASK_BUFFER_SIZE);
    ESP_LOGI(TAG, "     Queue length: %u", CONFIG_RAK3172_TASK_QUEUE_LENGTH);
    ESP_LOGI(TAG, "     Rx: %u", p_Device->Rx);
    ESP_LOGI(TAG, "     Tx: %u", p_Device->Tx);
    ESP_LOGI(TAG, "     Baudrate: %u", p_Device->Baudrate);
    ESP_LOGI(TAG, "Use library version: %s", RAK3172_LibVersion().c_str());

    ESP_LOGI(TAG, "Modes:");
    #ifdef CONFIG_RAK3172_MODE_WITH_LORAWAN
        ESP_LOGI(TAG, "     [x] LoRaWAN");
    #else
        ESP_LOGI(TAG, "     [ ] LoRaWAN");
    #endif

    #ifdef CONFIG_RAK3172_MODE_WITH_P2P
        ESP_LOGI(TAG, "     [x] P2P");
    #else
        ESP_LOGI(TAG, "     [ ] P2P");
    #endif

    ESP_LOGI(TAG, "Reset:");
    #ifdef CONFIG_RAK3172_RESET_USE_HW
        ESP_LOGI(TAG, "     Pin: %u", p_Device->Reset);
        ESP_LOGI(TAG, "     [x] Hardware reset");
        ESP_LOGI(TAG, "     [ ] Software reset");

        _RAK3172_Reset_Config.pin_bit_mask = (1ULL << p_Device->Reset);

        // Configure the pull-up / pull-down resistor.
        #ifdef CONFIG_RAK3172_RESET_USE_PULL
            #ifdef CONFIG_RAK3172_RESET_INVERT
                ESP_LOGI(TAG, "     [x] Internal pull-down");
                _RAK3172_Reset_Config.pull_down_en = GPIO_PULLDOWN_ENABLE,
            #else
                ESP_LOGI(TAG, "     [x] Internal pull-up");
                _RAK3172_Reset_Config.pull_up_en = GPIO_PULLUP_ENABLE;
            #endif
        #else
            ESP_LOGI(TAG, "     [x] No pull-up / pull-down");
        #endif

        if(gpio_config(&_RAK3172_Reset_Config) != ESP_OK)
        {
            return RAK3172_ERR_INVALID_STATE;
        }

        // Set the reset pin when no pull-up / pull-down resistor should be used.
        #ifdef CONFIG_RAK3172_RESET_USE_PULL
            #ifdef CONFIG_RAK3172_RESET_INVERT
                ESP_LOGI(TAG, "     [x] Invert");
                gpio_set_level(p_Device->Reset, false);
            #else
                ESP_LOGI(TAG, "     [ ] Invert");
                gpio_set_level(p_Device->Reset, true);
            #endif
        #endif
    #else
        ESP_LOGI(TAG, "     [ ] Hardware reset");
        ESP_LOGI(TAG, "     [x] Software reset");
    #endif

    RAK3172_ERROR_CHECK(RAK3172_BasicInit(p_Device));

    #ifdef CONFIG_RAK3172_RESET_USE_HW
        RAK3172_ERROR_CHECK(RAK3172_HardReset(p_Device));
    #else
        RAK3172_ERROR_CHECK(RAK3172_SoftReset(p_Device));
    #endif

    vTaskDelay(500 / portTICK_PERIOD_MS);

    // Firmware without RUI3 will produce a MIC mismatch when using a factory reset during the initialization.
    #if(defined CONFIG_RAK3172_FACTORY_RESET) && (defined CONFIG_RAK3172_USE_RUI3)
        RAK3172_ERROR_CHECK(RAK3172_FactoryReset(p_Device));
    #endif

    // Check if echo mode is enabled.
    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT", NULL, &Response));
    ESP_LOGD(TAG, "Response from 'AT': %s", Response.c_str());
    if(Response.find("OK") == std::string::npos)
    {
        std::string* Dummy;

        // Echo mode is enabled. Need to receive one more line.
        if(xQueueReceive(p_Device->Internal.MessageQueue, &Dummy, RAK3172_WAIT_TIMEOUT / portTICK_PERIOD_MS) != pdPASS)
        {
            return RAK3172_ERR_TIMEOUT;
        }
        delete Dummy;

        ESP_LOGD(TAG, "Echo mode enabled. Disabling echo mode...");

        // Disable echo mode
        //  -> Transmit the command
        //  -> Receive the echo
        //  -> Receive the value
        //  -> Receive the status
        uart_write_bytes(p_Device->Interface, "ATE\r\n", std::string("ATE\r\n").length());
        if(xQueueReceive(p_Device->Internal.MessageQueue, &Dummy, RAK3172_WAIT_TIMEOUT / portTICK_PERIOD_MS) != pdPASS)
        {
            return RAK3172_ERR_TIMEOUT;
        }
        delete Dummy;

        #ifndef CONFIG_RAK3172_USE_RUI3
            if(xQueueReceive(p_Device->Internal.MessageQueue, &Dummy, RAK3172_WAIT_TIMEOUT / portTICK_PERIOD_MS) != pdPASS)
            {
                return RAK3172_ERR_TIMEOUT;
            }
            delete Dummy;
        #endif

        if(xQueueReceive(p_Device->Internal.MessageQueue, &Dummy, RAK3172_WAIT_TIMEOUT / portTICK_PERIOD_MS) != pdPASS)
        {
            return RAK3172_ERR_TIMEOUT;
        }

        // Error during initialization when everything else except 'OK' is received.
        if(Dummy->find("OK") == std::string::npos)
        {
            return RAK3172_ERR_TIMEOUT;
        }
        delete Dummy;
    }

    if(p_Device->Info != NULL)
    {
        RAK3172_ERROR_CHECK(RAK3172_GetFWVersion(p_Device, &p_Device->Info->Firmware) | RAK3172_GetSerialNumber(p_Device, &p_Device->Info->Serial));

        #ifdef CONFIG_RAK3172_USE_RUI3
            RAK3172_ERROR_CHECK(RAK3172_GetCLIVersion(p_Device, &p_Device->Info->CLI) | RAK3172_GetAPIVersion(p_Device, &p_Device->Info->API) |
                                RAK3172_GetModel(p_Device, &p_Device->Info->Model) | RAK3172_GetHWID(p_Device, &p_Device->Info->HWID) |
                                RAK3172_GetBuildTime(p_Device, &p_Device->Info->BuildTime) | RAK3172_GetRepoInfo(p_Device, &p_Device->Info->RepoInfo));
        #endif
    }

    return RAK3172_GetMode(p_Device);
}

void RAK3172_Deinit(RAK3172_t* const p_Device)
{
    if(p_Device->Internal.isInitialized == false)
    {
        return;
    }

    vTaskSuspend(p_Device->Internal.Handle);
    vTaskDelete(p_Device->Internal.Handle);

    if(uart_is_driver_installed(p_Device->Interface))
    {
        uart_flush(p_Device->Interface);
        uart_disable_pattern_det_intr(p_Device->Interface);
        uart_driver_delete(p_Device->Interface);
    }

    vQueueDelete(p_Device->Internal.MessageQueue);
    vQueueDelete(p_Device->Internal.ReceiveQueue);

    free(p_Device->Internal.RxBuffer);
    p_Device->Internal.RxBuffer = NULL;

    gpio_reset_pin((gpio_num_t)p_Device->Rx);
    gpio_reset_pin((gpio_num_t)p_Device->Tx);

	p_Device->Internal.isInitialized = false;
}

void RAK3172_PrepareSleep(RAK3172_t* const p_Device)
{
    if((p_Device == NULL) || (p_Device->Internal.isInitialized == false))
    {
        return;
    }

    gpio_reset_pin(p_Device->Rx);
    gpio_reset_pin(p_Device->Tx);

    p_Device->Internal.isInitialized = false;
    p_Device->Internal.isBusy = false;
}

RAK3172_Error_t RAK3172_WakeUp(RAK3172_t* const p_Device)
{
    std::string Response;

    if(p_Device == NULL)
    {
        return RAK3172_ERR_INVALID_ARG;
    }
    else if(p_Device->Internal.isInitialized == true)
    {
        return RAK3172_ERR_OK;
    }

    p_Device->Internal.isInitialized = true;
    p_Device->Internal.isBusy = false;

    return RAK3172_BasicInit(p_Device);
}

RAK3172_Error_t RAK3172_FactoryReset(RAK3172_t* const p_Device)
{
    std::string Command;

    if(p_Device == NULL)
    {
        return RAK3172_ERR_INVALID_ARG;
    }
    else if(p_Device->Internal.isInitialized == false)
    {
        return RAK3172_ERR_INVALID_STATE;
    }

    ESP_LOGI(TAG, "Perform factory reset...");

    #ifndef CONFIG_RAK3172_USE_RUI3
        p_Device->Internal.isBusy = true;
        Command = "ATR\r\n";
        uart_write_bytes(p_Device->Interface, Command.c_str(), Command.length());
        RAK3172_ERROR_CHECK(RAK3172_ReceiveSplashScreen(p_Device, RAK3172_WAIT_TIMEOUT));
    #else
        RAK3172_SendCommand(p_Device, "ATR");
    #endif

    ESP_LOGI(TAG, "     Successful!");

    return RAK3172_ERR_OK;
}

RAK3172_Error_t RAK3172_SoftReset(RAK3172_t* const p_Device)
{
    std::string Command;

	if(p_Device == NULL)
	{
        return RAK3172_ERR_INVALID_ARG;
	}
	else if(p_Device->Internal.isInitialized == false)
	{
        return RAK3172_ERR_INVALID_STATE;
	}

    ESP_LOGI(TAG, "Perform software reset...");

    p_Device->Internal.isBusy = true;

    // Reset the module and read back the slash screen because the current state is unclear.
    Command = "ATZ\r\n";
    uart_write_bytes(p_Device->Interface, Command.c_str(), Command.length());

    RAK3172_ERROR_CHECK(RAK3172_ReceiveSplashScreen(p_Device, 10 * 1000UL));

    ESP_LOGI(TAG, "     Successful!");

    return RAK3172_ERR_OK;
}

#ifdef CONFIG_RAK3172_RESET_USE_HW
    RAK3172_Error_t RAK3172_HardReset(RAK3172_t* const p_Device)
    {
        if(p_Device == NULL)
        {
            return RAK3172_ERR_INVALID_ARG;
        }
        else if(p_Device->Internal.isInitialized == false)
        {
            return RAK3172_ERR_INVALID_STATE;
        }

        ESP_LOGI(TAG, "Perform hardware reset...");

        if(p_Device->isResetInverted)
        {
            gpio_set_level(p_Device->Reset, true);
        }
        else
        {
            gpio_set_level(p_Device->Reset, false);
        }

        vTaskDelay(500 / portTICK_PERIOD_MS);

        if(p_Device->isResetInverted)
        {
            gpio_set_level(p_Device->Reset, false);
        }
        else
        {
            gpio_set_level(p_Device->Reset, true);
        }

        vTaskDelay(500 / portTICK_PERIOD_MS);

        p_Device->Internal.isBusy = false;

        #ifdef CONFIG_RAK3172_USE_RUI3
            RAK3172_ERROR_CHECK(RAK3172_ReceiveSplashScreen(p_Device, 10 * 1000UL));
        #endif

        ESP_LOGI(TAG, "     Successful!");

        return RAK3172_ERR_OK;
    }
#endif