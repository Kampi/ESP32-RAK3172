 /*
 * rak3172.cpp
 *
 *  Copyright (C) Daniel Kampert, 2023
 *	Website: www.kampis-elektroecke.de
 *  File info: RAK3172 serial driver.
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

#include <algorithm>

#include <sdkconfig.h>

#include "rak3172.h"

#include "Arch/Logging/rak3172_logging.h"

#define STRINGIFY(s)                            STR(s)
#define STR(s)                                  #s

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
    #ifdef SOC_UART_SUPPORT_REF_TICK
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
 *  @param p_Device RAK3172 device object
 *  @param Timeout  (Optional) Timeout for the splash screen in milliseconds
 *  @return         RAK3172_ERR_OK when successful
 */
static RAK3172_Error_t RAK3172_ReceiveSplashScreen(RAK3172_t& p_Device, uint16_t Timeout = 1000)
{
    std::string* Response = NULL;

    do
    {
        if(xQueueReceive(p_Device.Internal.MessageQueue, &Response, Timeout / portTICK_PERIOD_MS) != pdPASS)
        {
            RAK3172_LOGE(TAG, "     Timeout!");

            p_Device.Internal.isBusy = false;

            return RAK3172_ERR_TIMEOUT;
        }

        RAK3172_LOGD(TAG, "Response: %s", Response->c_str());

        // The driver was compiled for RUI3, but an older splash screen was received (version 1.4.0 and below).
        #ifdef CONFIG_RAK3172_USE_RUI3
            if(Response->find("Version.") != std::string::npos)
            {
                RAK3172_LOGE(TAG, "Firmware compiled for RUI3, but module firmware does not support RUI3!");

                p_Device.Internal.isBusy = false;

                return RAK3172_ERR_INVALID_RESPONSE;
            }
        #endif

        if((Response->find("LoRaWAN.") != std::string::npos) || (Response->find("LoRa P2P.") != std::string::npos))
        {
            p_Device.Internal.isBusy = false;
        }

        delete Response;
    } while(p_Device.Internal.isBusy);

    return RAK3172_ERR_OK;
}

/** @brief          UART receive task.
 *  @param p_Arg    Pointer to task arguments
 */
static void RAK3172_UART_EventTask(void* p_Arg)
{
    uart_event_t Event;
    RAK3172_t* Device = (RAK3172_t*)p_Arg;

    RAK3172_LOGD(TAG, "Start RAK3172 event task");

    while(true)
    {
        if(xQueueReceive(Device->Internal.EventQueue, (void*)&Event, 20 / portTICK_PERIOD_MS) == pdPASS)
        {
            size_t BufferedSize;
            int32_t PatternPos;

            switch(Event.type)
            {
                case UART_FIFO_OVF:
                {
                    ESP_LOGW(TAG, "HW FIFO Overflow");

                    uart_flush(Device->UART.Interface);
                    xQueueReset(Device->Internal.MessageQueue);

                    break;
                }
                case UART_BUFFER_FULL:
                {
                    ESP_LOGW(TAG, "Ring Buffer Full");

                    uart_flush(Device->UART.Interface);
                    xQueueReset(Device->Internal.MessageQueue);

                    break;
                }
                case UART_PATTERN_DET:
                {
                    uart_get_buffered_data_len(Device->UART.Interface, &BufferedSize);

                    PatternPos = uart_pattern_pop_pos(Device->UART.Interface);

                    if(PatternPos == -1)
                    {
                        uart_flush_input(Device->UART.Interface);
                        xQueueReset(Device->Internal.MessageQueue);
                    }
                    else
                    {
                        int BytesRead;
                        std::string* Response = new std::string();

                        RAK3172_LOGD(TAG, "     Pattern detected at position %u. Use buffered size: %u", static_cast<unsigned int>(PatternPos), static_cast<unsigned int>(BufferedSize));

                        BytesRead = uart_read_bytes(Device->UART.Interface, Device->Internal.RxBuffer, PatternPos, 10);
                        if(BytesRead == -1)
                        {
                            uart_flush(Device->UART.Interface);
                            xQueueReset(Device->Internal.MessageQueue);

                            break;
                        }

                        // Copy the data from the buffer into the string.
                        for(uint32_t i = 0; i < BytesRead; i++)
                        {
                            char Character;

                            Character = Device->Internal.RxBuffer[i];

                            if((Character != '\n') && (Character != '\r'))
                            {
                                *Response += Character;
                            }
                        }

                        RAK3172_LOGD(TAG, "     Response: %s", Response->c_str());

                        #ifdef CONFIG_RAK3172_MODE_WITH_LORAWAN
                            if((Device->Mode == RAK_MODE_LORAWAN) && (Response->find("+EVT") != std::string::npos))
                            {
                                RAK3172_LOGD(TAG, "Event: %s", Response->c_str());

                                // Join was successful.
                                if(Response->find("JOINED") != std::string::npos)
                                {
                                    RAK3172_LOGD(TAG, " Joined...");

                                    #ifndef CONFIG_RAK3172_USE_RUI3
                                        Device->Internal.isJoinEvent = true;
                                    #endif

                                    Device->Internal.isBusy = false;
                                    Device->LoRaWAN.isJoined = true;
                                }
                                // Join failed.
                                #ifdef CONFIG_RAK3172_USE_RUI3
                                    else if(Response->find("JOIN_FAILED_RX_TIMEOUT") != std::string::npos)
                                #else
                                    else if(Response->find("JOIN FAILED") != std::string::npos)
                                #endif
                                {
                                    RAK3172_LOGD(TAG, " Not joined...");

                                    if(Device->LoRaWAN.AttemptCounter > 0)
                                    {
                                        Device->LoRaWAN.AttemptCounter--;
                                    }
                                    else
                                    {
                                        Device->Internal.isBusy = false;
                                    }

                                    #ifndef CONFIG_RAK3172_USE_RUI3
                                        Device->Internal.isBusy = false;
                                        Device->Internal.isJoinEvent = true;
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
                                    size_t Index;
                                    std::string Dummy;
                                    RAK3172_Rx_t* Received = new RAK3172_Rx_t();

                                    // Formats documentation:
                                    //  FW 1.03     +EVT:RX_1, RSSI -89, SNR 4

                                    // Remove "+EVT:" from the response.
                                    Response->erase(Response->find("+EVT:"), std::string("+EVT:").length());

                                    // Get the channel number from the "RX_x" part of the response.
                                    Index = Response->find("RX");

                                    if(Response->at(Index + 1) == '1')
                                    {
                                        Received->Group = RAK_RX_GROUP_1;
                                    }
                                    else if(Response->at(Index + 1) == '2')
                                    {
                                        Received->Group = RAK_RX_GROUP_2;
                                    }
                                    else if(Response->at(Index + 1) == 'B')
                                    {
                                        Received->Group = RAK_RX_GROUP_B;
                                    }
                                    else if(Response->at(Index + 1) == 'C')
                                    {
                                        Received->Group = RAK_RX_GROUP_C;
                                    }

                                    // Remove the "RX_x" from the response.
                                    Response->erase(0, sizeof("RX_x"));

                                    // Get the RSSI value.
                                    #ifdef CONFIG_RAK3172_USE_RUI3
                                        Dummy = Response->substr(0, Response->find(":"));
                                        Response->erase(0, std::string(Dummy + ":").length());
                                    #else
                                        Dummy = Response->substr(std::string("RSSI").length() + 1, Response->find(","));
                                        Response->erase(0, std::string(Dummy).length() + 2);
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
                                            Bytes = uart_read_bytes(Device->UART.Interface, &Data, 1, 10);
                                            if((Bytes != 0) && (Data != '\r') && (Data != '\n'))
                                            {
                                                *Response += Data;
                                            }
                                        } while(Bytes != 0);

                                        RAK3172_LOGD(TAG, "Next line: %s", Response->c_str());

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

                                    RAK3172_LOGI(TAG, "RSSI: %i", Received->RSSI);
                                    RAK3172_LOGI(TAG, "SNR: %i", Received->SNR);
                                    RAK3172_LOGI(TAG, "Port: %u", Received->Port);
                                    RAK3172_LOGI(TAG, "Channel: %u", Received->Group);
                                    RAK3172_LOGI(TAG, "Payload: %s", Received->Payload.c_str());

                                    xQueueSend(Device->Internal.ReceiveQueue, &Received, 0);
                                }

                                delete Response;
                            }
                        #endif
                        #ifdef CONFIG_RAK3172_MODE_WITH_P2P
                            if((Device->Mode == RAK_MODE_P2P) && (Response->find("+EVT") != std::string::npos))
                            {
                                RAK3172_LOGD(TAG, "Event: %s", Response->c_str());

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

                                    RAK3172_LOGD(TAG, "RSSI: %i", Received->RSSI);
                                    RAK3172_LOGD(TAG, "SNR: %i", Received->SNR);
                                    RAK3172_LOGD(TAG, "Payload: %s", Received->Payload.c_str());

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
 *  @param p_Device RAK3172 device object
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_NO_MEM when the message queues, the task or the receive buffer Cannot be created
 */
static RAK3172_Error_t RAK3172_BasicInit(RAK3172_t& p_Device)
{
    uint8_t Flags;
    RAK3172_Error_t Error;

    if(p_Device.UART.Tx == p_Device.UART.Rx)
    {
        RAK3172_LOGE(TAG, "Invalid Rx and Tx for UART!");

        return RAK3172_ERR_INVALID_ARG;
    }

    #ifdef CONFIG_RAK3172_UART_IRAM
        Flags = ESP_INTR_FLAG_IRAM;
    #else
        Flags = 0;
    #endif

    RAK3172_LOGI(TAG, "UART config:");
    RAK3172_LOGI(TAG, "     Interface: %u", p_Device.UART.Interface);
    RAK3172_LOGI(TAG, "     Buffer size: %u", CONFIG_RAK3172_UART_BUFFER_SIZE);
    RAK3172_LOGI(TAG, "     Stack size: %u", CONFIG_RAK3172_TASK_STACK_SIZE);
    RAK3172_LOGI(TAG, "     Queue length: %u", CONFIG_RAK3172_UART_QUEUE_LENGTH);
    RAK3172_LOGI(TAG, "     Rx: %u", p_Device.UART.Rx);
    RAK3172_LOGI(TAG, "     Tx: %u", p_Device.UART.Tx);
    RAK3172_LOGI(TAG, "     Baudrate: %u", p_Device.UART.Baudrate);

    esp_log_level_set("uart", ESP_LOG_NONE);
    if(uart_driver_install(p_Device.UART.Interface, CONFIG_RAK3172_UART_BUFFER_SIZE, CONFIG_RAK3172_UART_BUFFER_SIZE, CONFIG_RAK3172_UART_QUEUE_LENGTH, &p_Device.Internal.EventQueue, Flags))
    {
        return RAK3172_ERR_INVALID_STATE;
    }

    _RAK3172_UART_Config.baud_rate = p_Device.UART.Baudrate;
    if(uart_param_config(p_Device.UART.Interface, &_RAK3172_UART_Config) ||
       uart_set_pin(p_Device.UART.Interface, p_Device.UART.Tx, p_Device.UART.Rx, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE) ||
       uart_enable_pattern_det_baud_intr(p_Device.UART.Interface, '\n', 1, 1, 0, 0) ||
       uart_pattern_queue_reset(p_Device.UART.Interface, CONFIG_RAK3172_UART_QUEUE_LENGTH)
      )
    {
        uart_driver_delete(p_Device.UART.Interface);

        return RAK3172_ERR_INVALID_STATE;
    }

    p_Device.Internal.MessageQueue = xQueueCreate(CONFIG_RAK3172_UART_QUEUE_LENGTH, sizeof(std::string*));
    if(p_Device.Internal.MessageQueue == NULL)
    {
        Error = RAK3172_ERR_NO_MEM;

        goto RAK3172_BasicInit_Error_1;
    }

    p_Device.Internal.ReceiveQueue = xQueueCreate(8, sizeof(RAK3172_Rx_t*));
    if(p_Device.Internal.ReceiveQueue == NULL)
    { 
        Error = RAK3172_ERR_NO_MEM;

        goto RAK3172_BasicInit_Error_2;
    }

    p_Device.Internal.RxBuffer = (uint8_t*)malloc(CONFIG_RAK3172_UART_BUFFER_SIZE);
    if(p_Device.Internal.RxBuffer == NULL)
    {
        Error = RAK3172_ERR_NO_MEM;

        goto RAK3172_BasicInit_Error_3;
    }

    #ifdef CONFIG_RAK3172_TASK_CORE_AFFINITY
        xTaskCreatePinnedToCore(RAK3172_UART_EventTask, "RAK3172-Event", CONFIG_RAK3172_TASK_STACK_SIZE, p_Device, CONFIG_RAK3172_TASK_PRIO, &p_Device.Internal.Handle, CONFIG_RAK3172_TASK_CORE);
    #else
        xTaskCreate(RAK3172_UART_EventTask, "RAK3172-Event", CONFIG_RAK3172_TASK_STACK_SIZE, &p_Device, CONFIG_RAK3172_TASK_PRIO, &p_Device.Internal.Handle);
    #endif

    if(p_Device.Internal.Handle == NULL)
    {
        Error = RAK3172_ERR_NO_MEM;

        goto RAK3172_BasicInit_Error_4;
    }

    if(uart_flush(p_Device.UART.Interface))
    {
        Error = RAK3172_ERR_INVALID_STATE;

        goto RAK3172_BasicInit_Error_4;
    }

    xQueueReset(p_Device.Internal.MessageQueue);
    p_Device.Internal.isInitialized = true;

    return RAK3172_ERR_OK;

RAK3172_BasicInit_Error_4:
    vTaskSuspend(p_Device.Internal.Handle);
    vTaskDelete(p_Device.Internal.Handle);

RAK3172_BasicInit_Error_3:
    free(p_Device.Internal.RxBuffer);

RAK3172_BasicInit_Error_2:
    vQueueDelete(p_Device.Internal.ReceiveQueue);

RAK3172_BasicInit_Error_1:
    vQueueDelete(p_Device.Internal.MessageQueue);

    uart_driver_delete(p_Device.UART.Interface);

	p_Device.Internal.isInitialized = false;

    return Error;
}

RAK3172_Error_t RAK3172_Init(RAK3172_t& p_Device)
{
    std::string Response;

    #ifdef CONFIG_RAK3172_RESET_USE_HW
        if((p_Device.Reset == GPIO_NUM_NC) || (p_Device.Reset >= GPIO_NUM_MAX))
        {
            return RAK3172_ERR_INVALID_ARG;
        }
    #endif

    p_Device.Internal.isInitialized = false;
    p_Device.Internal.isBusy = false;

    RAK3172_LOGI(TAG, "Use library version: %s", RAK3172_LibVersion().c_str());

    RAK3172_LOGI(TAG, "Modes:");
    #ifdef CONFIG_RAK3172_MODE_WITH_LORAWAN
        RAK3172_LOGI(TAG, "     [x] LoRaWAN");
    #else
        RAK3172_LOGI(TAG, "     [ ] LoRaWAN");
    #endif

    #ifdef CONFIG_RAK3172_MODE_WITH_P2P
        RAK3172_LOGI(TAG, "     [x] P2P");
    #else
        RAK3172_LOGI(TAG, "     [ ] P2P");
    #endif

    RAK3172_LOGI(TAG, "Reset:");
    #ifdef CONFIG_RAK3172_RESET_USE_HW
        RAK3172_LOGI(TAG, "     Pin: %u", p_Device.Reset);
        RAK3172_LOGI(TAG, "     [x] Hardware reset");
        RAK3172_LOGI(TAG, "     [ ] Software reset");

        _RAK3172_Reset_Config.pin_bit_mask = BIT(p_Device.Reset);

        // Configure the pull-up / pull-down resistor.
        #ifdef CONFIG_RAK3172_RESET_USE_PULL
            #ifdef CONFIG_RAK3172_RESET_INVERT
                RAK3172_LOGI(TAG, "     [x] Internal pull-down");
                _RAK3172_Reset_Config.pull_down_en = GPIO_PULLDOWN_ENABLE,
            #else
                RAK3172_LOGI(TAG, "     [x] Internal pull-up");
                _RAK3172_Reset_Config.pull_up_en = GPIO_PULLUP_ENABLE;
            #endif
        #else
            RAK3172_LOGI(TAG, "     [x] No pull-up / pull-down");
        #endif

        if(gpio_config(&_RAK3172_Reset_Config) != ESP_OK)
        {
            return RAK3172_ERR_INVALID_STATE;
        }

        // Set the reset pin when no pull-up / pull-down resistor should be used.
        #ifdef CONFIG_RAK3172_RESET_USE_PULL
            #ifdef CONFIG_RAK3172_RESET_INVERT
                RAK3172_LOGI(TAG, "     [x] Invert");
                gpio_set_level(p_Device.Reset, false);
            #else
                RAK3172_LOGI(TAG, "     [ ] Invert");
                gpio_set_level(p_Device.Reset, true);
            #endif
        #endif
    #else
        RAK3172_LOGI(TAG, "     [ ] Hardware reset");
        RAK3172_LOGI(TAG, "     [x] Software reset");
    #endif

    RAK3172_ERROR_CHECK(RAK3172_BasicInit(p_Device));

    #ifdef CONFIG_RAK3172_RESET_USE_HW
        RAK3172_ERROR_CHECK(RAK3172_HardReset(p_Device));
    #else
        RAK3172_ERROR_CHECK(RAK3172_SoftReset(p_Device));
    #endif

    vTaskDelay(500 / portTICK_PERIOD_MS);

    // Firmware without RUI3 will produce a MIC mismatch when using a factory reset during the initialization.
    #if((defined CONFIG_RAK3172_FACTORY_RESET) && (defined CONFIG_RAK3172_USE_RUI3))
        RAK3172_ERROR_CHECK(RAK3172_FactoryReset(p_Device));
    #endif

    // Check if echo mode is enabled.
    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT", NULL, &Response));
    RAK3172_LOGD(TAG, "Response from 'AT': %s", Response.c_str());
    if(Response.find("OK") == std::string::npos)
    {
        std::string* Dummy;

        // Echo mode is enabled. Need to receive one more line.
        if(xQueueReceive(p_Device.Internal.MessageQueue, &Dummy, RAK3172_DEFAULT_WAIT_TIMEOUT / portTICK_PERIOD_MS) != pdPASS)
        {
            return RAK3172_ERR_TIMEOUT;
        }
        delete Dummy;

        RAK3172_LOGD(TAG, "Echo mode enabled. Disabling echo mode...");

        // Disable echo mode
        //  -> Transmit the command
        //  -> Receive the echo
        //  -> Receive the value
        //  -> Receive the status
        uart_write_bytes(p_Device.UART.Interface, "ATE\r\n", std::string("ATE\r\n").length());
        if(xQueueReceive(p_Device.Internal.MessageQueue, &Dummy, RAK3172_DEFAULT_WAIT_TIMEOUT / portTICK_PERIOD_MS) != pdPASS)
        {
            return RAK3172_ERR_TIMEOUT;
        }
        delete Dummy;

        #ifndef CONFIG_RAK3172_USE_RUI3
            if(xQueueReceive(p_Device.Internal.MessageQueue, &Dummy, RAK3172_DEFAULT_WAIT_TIMEOUT / portTICK_PERIOD_MS) != pdPASS)
            {
                return RAK3172_ERR_TIMEOUT;
            }
            delete Dummy;
        #endif

        if(xQueueReceive(p_Device.Internal.MessageQueue, &Dummy, RAK3172_DEFAULT_WAIT_TIMEOUT / portTICK_PERIOD_MS) != pdPASS)
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

    if(p_Device.Info != NULL)
    {
        RAK3172_ERROR_CHECK(RAK3172_GetFWVersion(p_Device, &p_Device.Info->Firmware) | RAK3172_GetSerialNumber(p_Device, &p_Device.Info->Serial));

        #ifdef CONFIG_RAK3172_USE_RUI3
            RAK3172_ERROR_CHECK(RAK3172_GetCLIVersion(p_Device, &p_Device.Info->CLI) | RAK3172_GetAPIVersion(p_Device, &p_Device.Info->API) |
                                RAK3172_GetModel(p_Device, &p_Device.Info->Model) | RAK3172_GetHWID(p_Device, &p_Device.Info->HWID) |
                                RAK3172_GetBuildTime(p_Device, &p_Device.Info->BuildTime) | RAK3172_GetRepoInfo(p_Device, &p_Device.Info->RepoInfo));
        #endif
    }

    return RAK3172_GetMode(p_Device);
}

void RAK3172_Deinit(RAK3172_t& p_Device)
{
    if(p_Device.Internal.isInitialized == false)
    {
        return;
    }

    vTaskSuspend(p_Device.Internal.Handle);
    vTaskDelete(p_Device.Internal.Handle);

    if(uart_is_driver_installed(p_Device.UART.Interface))
    {
        uart_disable_pattern_det_intr(p_Device.UART.Interface);
        uart_driver_delete(p_Device.UART.Interface);
    }

    if(p_Device.Internal.MessageQueue != NULL)
    {
        vQueueDelete(p_Device.Internal.MessageQueue);
    }

    if(p_Device.Internal.ReceiveQueue != NULL)
    {
        vQueueDelete(p_Device.Internal.ReceiveQueue);
    }

    free(p_Device.Internal.RxBuffer);
    p_Device.Internal.RxBuffer = NULL;

    gpio_reset_pin(static_cast<gpio_num_t>(p_Device.UART.Rx));
    gpio_reset_pin(static_cast<gpio_num_t>(p_Device.UART.Tx));

    p_Device.Internal.isInitialized = false;
    p_Device.Internal.isBusy = false;
}

RAK3172_Error_t RAK3172_SetBaudrate(RAK3172_t& p_Device, RAK3172_Baud_t Baudrate)
{
    if(p_Device.UART.Baudrate == Baudrate)
    {
        return RAK3172_ERR_OK;
    }

    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+BAUD=" + std::to_string(Baudrate)));

    // Deinitialize the UART interface.
    if(uart_driver_delete(p_Device.UART.Interface))
    {
        return RAK3172_ERR_INVALID_STATE;
    }

    // Initialize the interface with the new baudrate. Do a rollback if something is going wrong.
    _RAK3172_UART_Config.baud_rate = Baudrate;
    if(RAK3172_BasicInit(p_Device) != RAK3172_ERR_OK)
    {
        _RAK3172_UART_Config.baud_rate = p_Device.UART.Baudrate;
        RAK3172_ERROR_CHECK(RAK3172_BasicInit(p_Device));
    }

    p_Device.UART.Baudrate = Baudrate;

    return RAK3172_ERR_OK;
}

RAK3172_Error_t RAK3172_WakeUp(RAK3172_t& p_Device)
{
    std::string Response;

    if(p_Device.Internal.isInitialized == true)
    {
        return RAK3172_ERR_OK;
    }

    RAK3172_LOGI(TAG, "Wake up driver from sleep mode...");

    RAK3172_ERROR_CHECK(RAK3172_BasicInit(p_Device));

    p_Device.Internal.isBusy = false;

    return RAK3172_SendCommand(p_Device, "AT");
}

RAK3172_Error_t RAK3172_FactoryReset(RAK3172_t& p_Device)
{
    if(p_Device.Internal.isInitialized == false)
    {
        return RAK3172_ERR_INVALID_STATE;
    }

    RAK3172_LOGI(TAG, "Perform factory reset...");

    #ifndef CONFIG_RAK3172_USE_RUI3
        std::string Command;

        p_Device.Internal.isBusy = true;
        Command = "ATR\r\n";
        uart_write_bytes(p_Device.UART.Interface, Command.c_str(), Command.length());
        RAK3172_ERROR_CHECK(RAK3172_ReceiveSplashScreen(p_Device, RAK3172_DEFAULT_WAIT_TIMEOUT));
    #else
        RAK3172_SendCommand(p_Device, "ATR");
    #endif

    RAK3172_LOGI(TAG, "     Successful!");

    return RAK3172_ERR_OK;
}

RAK3172_Error_t RAK3172_SoftReset(RAK3172_t& p_Device, uint32_t Timeout)
{
    std::string Command;

	if(p_Device.Internal.isInitialized == false)
	{
        return RAK3172_ERR_INVALID_STATE;
	}

    RAK3172_LOGI(TAG, "Perform software reset...");

    p_Device.Internal.isBusy = true;

    // Reset the module and read back the slash screen because the current state is unclear.
    Command = "ATZ\r\n";
    uart_write_bytes(p_Device.UART.Interface, Command.c_str(), Command.length());

    RAK3172_ERROR_CHECK(RAK3172_ReceiveSplashScreen(p_Device, Timeout * 1000UL));

    RAK3172_LOGI(TAG, "     Successful!");

    return RAK3172_ERR_OK;
}

#ifdef CONFIG_RAK3172_RESET_USE_HW
    RAK3172_Error_t RAK3172_HardReset(RAK3172_t& p_Device, uint32_t Timeout)
    {
        if(p_Device.Internal.isInitialized == false)
        {
            return RAK3172_ERR_INVALID_STATE;
        }

        RAK3172_LOGI(TAG, "Perform hardware reset...");

        #ifdef CONFIG_RAK3172_RESET_INVERT
            gpio_set_level(p_Device.Reset, true);
        #else
            gpio_set_level(p_Device.Reset, false);
        #endif

        vTaskDelay(500 / portTICK_PERIOD_MS);

        #ifdef CONFIG_RAK3172_RESET_INVERT
            gpio_set_level(p_Device.Reset, false);
        #else
            gpio_set_level(p_Device.Reset, true);
        #endif

        vTaskDelay(500 / portTICK_PERIOD_MS);

        p_Device.Internal.isBusy = false;

        #ifdef CONFIG_RAK3172_USE_RUI3
            RAK3172_ERROR_CHECK(RAK3172_ReceiveSplashScreen(p_Device, Timeout * 1000UL));
        #endif

        RAK3172_LOGI(TAG, "     Successful!");

        return RAK3172_ERR_OK;
    }
#endif
