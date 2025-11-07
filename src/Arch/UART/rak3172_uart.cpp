 /*
 * rak3172_uart.cpp
 *
 *  Copyright (C) Daniel Kampert, 2025
 *	Website: www.kampis-elektroecke.de
 *  File info: ESP32 UART wrapper for the RAK3172 driver.
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
 * Errors and commissions should be reported to DanielKampert@kampis-elektroecke.de.
 */

#include "rak3172_uart.h"

#include <sdkconfig.h>

#include "../Logging/rak3172_logging.h"

static uart_config_t _RAK3172_UART_Config = {
    .baud_rate              = 9600,
    .data_bits              = UART_DATA_8_BITS,
    .parity                 = UART_PARITY_DISABLE,
    .stop_bits              = UART_STOP_BITS_1,
    .flow_ctrl              = UART_HW_FLOWCTRL_DISABLE,
    .rx_flow_ctrl_thresh    = 0,
    #if(defined CONFIG_PM_ENABLE) && (defined CONFIG_IDF_TARGET_ESP32)
        #ifdef SOC_UART_SUPPORT_REF_TICK
            .source_clk     = UART_SCLK_REF_TICK,
        #else
            .source_clk     = UART_SCLK_RTC,
        #endif
    #else
        .source_clk         = UART_SCLK_DEFAULT,
    #endif
    .flags = {
        .allow_pd              = 0,
        .backup_before_sleep   = 0,
    }
};

#ifdef CONFIG_RAK3172_USE_RUI3
    static uint32_t _RAK3172_UART_Timeout = 10;
#else
    static uint32_t _RAK3172_UART_Timeout = 200;
#endif

static const char* TAG      = "RAK3172_UART";

/** @brief          UART receive task.
 *  @param p_Arg    Pointer to task arguments
 */
static void RAK3172_UART_EventTask(void* p_Arg)
{
    uart_event_t Event;
    RAK3172_t* Device;

    RAK3172_LOGD(TAG, "Start RAK3172 event task");

    Device = static_cast<RAK3172_t*>(p_Arg);

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

                        BytesRead = uart_read_bytes(Device->UART.Interface, Device->Internal.RxBuffer, PatternPos, pdMS_TO_TICKS(_RAK3172_UART_Timeout));
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
                            if((Device->Mode == RAK_MODE_LORAWAN) && (Response->find("EVT") != std::string::npos))
                            {
                                RAK3172_LOGI(TAG, "Event: %s", Response->c_str());

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
                                    RAK3172_LOGI(TAG, " Not joined...");
                                    RAK3172_LOGI(TAG, "  Attempts left: %u", static_cast<unsigned int>(Device->LoRaWAN.AttemptCounter));

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
                                else if(Response->find("TX_DONE") != std::string::npos)
                                {
                                    Device->Internal.isBusy = false;
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
                                    Index = Response->find("RX_");

                                    Received->isMulticast = false;
                                    if(Response->find("MULTICAST") != std::string::npos)
                                    {
                                        Received->isMulticast = true;
                                    }

                                    if(Response->at(Index + 3) == '1')
                                    {
                                        Received->Group = RAK_RX_GROUP_1;
                                    }
                                    else if(Response->at(Index + 3) == '2')
                                    {
                                        Received->Group = RAK_RX_GROUP_2;
                                    }
                                    else if(Response->at(Index + 3) == 'B')
                                    {
                                        Received->Group = RAK_RX_GROUP_B;
                                    }
                                    else if(Response->at(Index + 3) == 'C')
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
                                            Bytes = uart_read_bytes(Device->UART.Interface, &Data, 1, pdMS_TO_TICKS(_RAK3172_UART_Timeout));
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

                                    RAK3172_LOGD(TAG, "RSSI: %i", Received->RSSI);
                                    RAK3172_LOGD(TAG, "SNR: %i", Received->SNR);
                                    RAK3172_LOGD(TAG, "Port: %u", Received->Port);
                                    RAK3172_LOGD(TAG, "Channel: %u", Received->Group);
                                    RAK3172_LOGD(TAG, "Payload: %s", Received->Payload.c_str());
                                    RAK3172_LOGD(TAG, "Multicast: %u", Received->isMulticast);

                                    xQueueSend(Device->Internal.ReceiveQueue, &Received, 0);
                                }

                                delete Response;
                            }
                        #endif
                        #ifdef CONFIG_RAK3172_MODE_WITH_P2P
                            if((Device->Mode == RAK_MODE_P2P) && (Response->find("EVT") != std::string::npos))
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

RAK3172_Error_t RAK3172_UART_Init(RAK3172_t& p_Device)
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

        goto RAK3172_UART_Init_Error_1;
    }

    p_Device.Internal.ReceiveQueue = xQueueCreate(8, sizeof(RAK3172_Rx_t*));
    if(p_Device.Internal.ReceiveQueue == NULL)
    { 
        Error = RAK3172_ERR_NO_MEM;

        goto RAK3172_UART_Init_Error_2;
    }

    p_Device.Internal.RxBuffer = (uint8_t*)malloc(CONFIG_RAK3172_UART_BUFFER_SIZE);
    if(p_Device.Internal.RxBuffer == NULL)
    {
        Error = RAK3172_ERR_NO_MEM;

        goto RAK3172_UART_Init_Error_3;
    }

    #ifdef CONFIG_RAK3172_TASK_CORE_AFFINITY
        xTaskCreatePinnedToCore(RAK3172_UART_EventTask, "RAK3172-Event", CONFIG_RAK3172_TASK_STACK_SIZE, &p_Device, CONFIG_RAK3172_TASK_PRIO, &p_Device.Internal.Handle, CONFIG_RAK3172_TASK_CORE);
    #else
        xTaskCreate(RAK3172_UART_EventTask, "RAK3172-Event", CONFIG_RAK3172_TASK_STACK_SIZE, &p_Device, CONFIG_RAK3172_TASK_PRIO, &p_Device.Internal.Handle);
    #endif

    if(p_Device.Internal.Handle == NULL)
    {
        Error = RAK3172_ERR_NO_MEM;

        goto RAK3172_UART_Init_Error_4;
    }

    if(uart_flush(p_Device.UART.Interface))
    {
        Error = RAK3172_ERR_INVALID_STATE;

        goto RAK3172_UART_Init_Error_4;
    }

    xQueueReset(p_Device.Internal.MessageQueue);
    p_Device.Internal.isInitialized = true;

    return RAK3172_ERR_OK;

RAK3172_UART_Init_Error_4:
    if(p_Device.Internal.Handle != NULL)
    {
        vTaskSuspend(p_Device.Internal.Handle);
        vTaskDelete(p_Device.Internal.Handle);
        p_Device.Internal.Handle = NULL;
    }

RAK3172_UART_Init_Error_3:
    free(p_Device.Internal.RxBuffer);

RAK3172_UART_Init_Error_2:
    vQueueDelete(p_Device.Internal.ReceiveQueue);

RAK3172_UART_Init_Error_1:
    vQueueDelete(p_Device.Internal.MessageQueue);

    uart_driver_delete(p_Device.UART.Interface);

	p_Device.Internal.isInitialized = false;

    return Error;
}

void RAK3172_UART_Deinit(RAK3172_t& p_Device)
{
    if(uart_is_driver_installed(p_Device.UART.Interface))
    {
        uart_disable_pattern_det_intr(p_Device.UART.Interface);
        uart_driver_delete(p_Device.UART.Interface);
    }

    gpio_reset_pin(static_cast<gpio_num_t>(p_Device.UART.Rx));
    gpio_reset_pin(static_cast<gpio_num_t>(p_Device.UART.Tx));
}

RAK3172_Error_t RAK3172_UART_SetBaudrate(RAK3172_t& p_Device, RAK3172_Baud_t Baudrate)
{
    // Deinitialize the UART interface.
    if(uart_driver_delete(p_Device.UART.Interface))
    {
        return RAK3172_ERR_INVALID_STATE;
    }

    // Initialize the interface with the new baudrate. Do a rollback if something is going wrong.
    _RAK3172_UART_Config.baud_rate = Baudrate;
    if(RAK3172_UART_Init(p_Device) != RAK3172_ERR_OK)
    {
        const uint32_t Previous = p_Device.UART.Baudrate;
        _RAK3172_UART_Config.baud_rate = Previous;
        RAK3172_ERROR_CHECK(RAK3172_UART_Init(p_Device));

        return RAK3172_ERR_FAIL;
    }

    p_Device.UART.Baudrate = Baudrate;

    return RAK3172_ERR_OK;
}

int RAK3172_UART_WriteBytes(RAK3172_t& p_Device, const void* p_Buffer, size_t Length)
{
    return uart_write_bytes(p_Device.UART.Interface, p_Buffer, Length);
}