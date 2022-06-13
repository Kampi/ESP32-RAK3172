 /*
 * rak3172.cpp
 *
 *  Copyright (C) Daniel Kampert, 2022
 *	Website: www.kampis-elektroecke.de
 *  File info: RAK3172 driver for ESP32.

  GNU GENERAL PUBLIC LICENSE:
  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program. If not, see <http://www.gnu.org/licenses/>.

  Errors and commissions should be reported to DanielKampert@kampis-elektroecke.de
 */

#include <esp_log.h>

#include <sdkconfig.h>

#include "../include/rak3172.h"

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
    static gpio_config_t _Reset_Config = {
        .pin_bit_mask = 0,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
#endif

static uart_config_t _UART_Config = {
    .baud_rate              = 9600,
    .data_bits              = UART_DATA_8_BITS,
    .parity                 = UART_PARITY_DISABLE,
    .stop_bits              = UART_STOP_BITS_1,
    .flow_ctrl              = UART_HW_FLOWCTRL_DISABLE,
    .rx_flow_ctrl_thresh    = 0,
#ifdef CONFIG_PM_ENABLE
    .source_clk             = UART_SCLK_REF_TICK,
#else
    .source_clk             = UART_SCLK_APB,
#endif
};

static const char* TAG = "RAK3172";

/** @brief          Receive the splash screen after a reset.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param Timeout  (Optional) Timeout for the splash screen in seconds
 *  @return         RAK3172_ERR_OK when successful
 */
static RAK3172_Error_t RAK3172_ReceiveSplashScreen(RAK3172_t* const p_Device, uint16_t Timeout = 10)
{
    std::string* Response = NULL;

    do
    {
        if(xQueueReceive(p_Device->Internal.RxQueue, &Response, Timeout / portTICK_PERIOD_MS) != pdPASS)
        {
            ESP_LOGE(TAG, "     Timeout!");

            p_Device->Internal.isBusy = false;

            return RAK3172_ERR_TIMEOUT;
        }

        // The driver is compiled for RUI3, but an older splash screen was received (version 1.4.0 and below).
        #ifdef CONFIG_RAK3172_USE_RUI3
            if(Response->find("Version.") != std::string::npos)
            {
                ESP_LOGE(TAG, "Firmware compiled for RUI3, but module firmware doesn´t support RUI3!");

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
        if(xQueueReceive(Device->Internal.EventQueue, (void*)&Event, portMAX_DELAY))
        {
            size_t BufferedSize;
            uint32_t PatternPos;

            ESP_LOGD(TAG, "Event: %u", Event.type);

            switch(Event.type)
            {
                case UART_PATTERN_DET:
                {
                    uart_get_buffered_data_len(Device->Interface, &BufferedSize);

                    PatternPos = uart_pattern_pop_pos(Device->Interface);

                    ESP_LOGD(TAG, "     Pattern detected at position %u. Use buffered size: %u", PatternPos, BufferedSize);

                    if(PatternPos == -1)
                    {
                        uart_flush_input(Device->Interface);
                    }
                    else
                    {
                        std::string* const Response = new std::string();

                        if(Response != NULL)
                        {
                            uart_read_bytes(Device->Interface, Device->Internal.RxBuffer, PatternPos, 100 / portTICK_PERIOD_MS);

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

                            // An asynchronous event was received.
                            if(Response->find("+EVT") != std::string::npos)
                            {
                                ESP_LOGI(TAG, "     Event: %s", Response->c_str());

                                // Join events shouldn´t be passed into the receive queue.
                                if(Response->find("JOINED") != std::string::npos)
                                {
                                    ESP_LOGI(TAG, "Device joined...");

                                    Device->LoRaWAN.isJoined = true;
                                }
                                else if(Response->find("JOIN FAILED") != std::string::npos)
                                {
                                    ESP_LOGI(TAG, "Not joined...");

                                    Device->LoRaWAN.isJoined = false;
                                }
                                else if(Response->find("RX") != std::string::npos)
                                {
                                    ESP_LOGI(TAG, "Message received...");
                                }
                            }
                            // Any other message response.
                            else
                            {
                                xQueueSend(Device->Internal.RxQueue, &Response, 0);
                            }
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

const std::string RAK3172_LibVersion(void)
{
    #if((defined RAK3172_LIB_MAJOR) && (defined RAK3172_LIB_MINOR) && (defined RAK3172_LIB_BUILD))
        return std::string(STRINGIFY(RAK3172_LIB_MAJOR)) + "." + std::string(STRINGIFY(RAK3172_LIB_MINOR)) + "." + std::string(STRINGIFY(RAK3172_LIB_BUILD));
    #else
        return "<Not defined>";
    #endif
}

RAK3172_Error_t RAK3172_Init(RAK3172_t* const p_Device)
{
    RAK3172_Error_t Error;
    std::string Response;

    if(p_Device == NULL)
    {
        return RAK3172_ERR_INVALID_ARG;
    }

    p_Device->Internal.isInitialized = false;

    esp_log_level_set("uart", ESP_LOG_NONE);

    _UART_Config.baud_rate = p_Device->Baudrate;

    ESP_LOGI(TAG, "UART config:");
    ESP_LOGI(TAG, "     Interface: %u", p_Device->Interface);
    ESP_LOGI(TAG, "     Buffer size: %u",CONFIG_RAK3172_TASK_BUFFER_SIZE);
    ESP_LOGI(TAG, "     Queue length: %u", CONFIG_RAK3172_TASK_QUEUE_LENGTH);
    ESP_LOGI(TAG, "     Rx: %u", p_Device->Rx);
    ESP_LOGI(TAG, "     Tx: %u", p_Device->Tx);
    ESP_LOGI(TAG, "     Baudrate: %u", p_Device->Baudrate);
    ESP_LOGI(TAG, "Use library version: %s", RAK3172_LibVersion().c_str());

    ESP_LOGI(TAG, "Modes:");
    #ifdef CONFIG_RAK3172_WITH_LORAWAN
        ESP_LOGI(TAG, "     [x] LoRaWAN");
    #else
        ESP_LOGI(TAG, "     [ ] LoRaWAN");
    #endif

    #ifdef CONFIG_RAK3172_WITH_P2P
        ESP_LOGI(TAG, "     [x] P2P");
    #else
        ESP_LOGI(TAG, "     [ ] P2P");
    #endif

    ESP_LOGI(TAG, "Reset:");
    #ifdef CONFIG_RAK3172_RESET_USE_HW
        ESP_LOGI(TAG, "     Pin: %u", p_Device->Reset);
        ESP_LOGI(TAG, "     [x] Hardware reset");

        _Reset_Config.pin_bit_mask = (1ULL << p_Device->Reset);

        // Configure the pull-up / pull-down resistor.
        #ifdef CONFIG_RAK3172_RESET_USE_PULL
            #ifdef CONFIG_RAK3172_RESET_INVERT
                ESP_LOGI(TAG, "     [x] Internal pull-down");
                _Reset_Config.pull_down_en = GPIO_PULLDOWN_ENABLE,
            #else
                ESP_LOGI(TAG, "     [x] Internal pull-up");
                _Reset_Config.pull_up_en = GPIO_PULLUP_ENABLE;
            #endif
        #else
            ESP_LOGI(TAG, "     [x] No pull-up / pull-down");
        #endif

        if(gpio_config(&_Reset_Config) != ESP_OK)
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
    #endif

    if(uart_driver_install(p_Device->Interface, CONFIG_RAK3172_TASK_BUFFER_SIZE * 2, CONFIG_RAK3172_TASK_BUFFER_SIZE * 2, CONFIG_RAK3172_TASK_QUEUE_LENGTH, &p_Device->Internal.EventQueue, 0) ||
       uart_param_config(p_Device->Interface, &_UART_Config) ||
       uart_set_pin(p_Device->Interface, p_Device->Tx, p_Device->Rx, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE) ||
       uart_enable_pattern_det_baud_intr(p_Device->Interface, '\n', 1, 9, 0, 0) ||
       uart_pattern_queue_reset(p_Device->Interface, CONFIG_RAK3172_TASK_QUEUE_LENGTH))
    {
        return RAK3172_ERR_INVALID_STATE;
    }

    if(uart_flush(p_Device->Interface))
    {
        return RAK3172_ERR_INVALID_STATE;
    }

    p_Device->Internal.RxQueue = xQueueCreate(CONFIG_RAK3172_TASK_QUEUE_LENGTH, sizeof(std::string*));
    if(p_Device->Internal.RxQueue == NULL)
    {
        return RAK3172_ERR_NO_MEM;
    }

    p_Device->Internal.RxBuffer = (uint8_t*)malloc(CONFIG_RAK3172_TASK_BUFFER_SIZE);
    if(p_Device->Internal.RxBuffer == NULL)
    {
        return RAK3172_ERR_NO_MEM;
    }

    #ifdef CONFIG_RAK3172_TASK_CORE_AFFINITY
        xTaskCreatePinnedToCore(RAK3172_UART_EventTask, "RAK3172_EventTask", CONFIG_RAK3172_TASK_BUFFER_SIZE * 2, p_Device, CONFIG_RAK3172_TASK_PRIO, &p_Device->Internal.Handle, CONFIG_RAK3172_TASK_CORE);
    #else
        xTaskCreate(RAK3172_UART_EventTask, "RAK3172_EventTask", CONFIG_RAK3172_TASK_BUFFER_SIZE * 2, p_Device, CONFIG_RAK3172_TASK_PRIO, &p_Device->Internal.Handle);
    #endif

    if(p_Device->Internal.Handle == NULL)
    {
        Error = RAK3172_ERR_INVALID_STATE;

        goto RAK3172_Init_Error;
    }

    p_Device->Internal.isInitialized = true;

    #ifdef CONFIG_RAK3172_FACTORY_RESET
        Error = RAK3172_FactoryReset(p_Device);
        if(Error != RAK3172_ERR_OK)
        {
            goto RAK3172_Init_Error;
        }
    #endif

    #ifdef CONFIG_RAK3172_RESET_USE_HW
        Error = RAK3172_HardReset(p_Device);
        if(Error != RAK3172_ERR_OK)
        {
            goto RAK3172_Init_Error;
        }
    #else
        Error = RAK3172_SoftReset(p_Device);
        if(Error != RAK3172_ERR_OK)
        {
            goto RAK3172_Init_Error;
        }
    #endif

    vTaskDelay(1000 / portTICK_RATE_MS);

    // Check if echo mode is enabled.
    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT", NULL, &Response));
    ESP_LOGD(TAG, "Response from 'AT': %s", Response.c_str());
    if(Response.find("OK") == std::string::npos)
    {
        std::string* Dummy;

        // Echo mode is enabled. Need to receive one more line.
        if(xQueueReceive(p_Device->Internal.RxQueue, &Dummy, RAK3172_WAIT_TIMEOUT / portTICK_PERIOD_MS) != pdPASS)
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
        uart_write_bytes(p_Device->Interface, "ATE\r\n", 5);
        if(xQueueReceive(p_Device->Internal.RxQueue, &Dummy, RAK3172_WAIT_TIMEOUT / portTICK_PERIOD_MS) != pdPASS)
        {
            return RAK3172_ERR_TIMEOUT;
        }
        delete Dummy;

        #ifndef CONFIG_RAK3172_USE_RUI3
            if(xQueueReceive(p_Device->Internal.RxQueue, &Dummy, RAK3172_WAIT_TIMEOUT / portTICK_PERIOD_MS) != pdPASS)
            {
                return RAK3172_ERR_TIMEOUT;
            }
            delete Dummy;
        #endif

        if(xQueueReceive(p_Device->Internal.RxQueue, &Dummy, RAK3172_WAIT_TIMEOUT / portTICK_PERIOD_MS) != pdPASS)
        {
            return RAK3172_ERR_TIMEOUT;
        }

        // Error during initialization when everything else except 'OK' is received.
        if(Dummy->find("OK") == std::string::npos)
        {
            return RAK3172_ERR_FAIL;
        }
        delete Dummy;
    }

    if(p_Device->Info != NULL)
    {
        Error = RAK3172_GetFWVersion(p_Device, &p_Device->Info->Firmware) | RAK3172_GetSerialNumber(p_Device, &p_Device->Info->Serial);
        #ifdef CONFIG_RAK3172_USE_RUI3
            Error |= RAK3172_GetCLIVersion(p_Device, &p_Device->Info->CLI) | RAK3172_GetAPIVersion(p_Device, &p_Device->Info->API) |
                     RAK3172_GetModel(p_Device, &p_Device->Info->Model) | RAK3172_GetHWID(p_Device, &p_Device->Info->HWID) |
                     RAK3172_GetBuildTime(p_Device, &p_Device->Info->BuildTime) | RAK3172_GetRepoInfo(p_Device, &p_Device->Info->RepoInfo);
        #endif
        if(Error != RAK3172_ERR_OK)
        {
            goto RAK3172_Init_Error;
        }
    }

    Error = RAK3172_GetMode(p_Device);
    if(Error != RAK3172_ERR_OK)
    {
        goto RAK3172_Init_Error;
    }

    return RAK3172_ERR_OK;

RAK3172_Init_Error:
	free(p_Device->Internal.RxBuffer);
	p_Device->Internal.isInitialized = false;

	return Error;
}

void RAK3172_Deinit(RAK3172_t* const p_Device)
{
    if(p_Device->Internal.isInitialized == false)
    {
        return;
    }

    free(p_Device->Internal.RxBuffer);
    p_Device->Internal.RxBuffer = NULL;

    vTaskSuspend(p_Device->Internal.Handle);
    vTaskDelete(p_Device->Internal.Handle);

    if(uart_is_driver_installed(p_Device->Interface))
    {
        uart_flush(p_Device->Interface);
        uart_disable_pattern_det_intr(p_Device->Interface);
        uart_driver_delete(p_Device->Interface);
    }

    vQueueDelete(p_Device->Internal.RxQueue);

    gpio_config_t Conf = {
        .pin_bit_mask = ((1ULL << p_Device->Rx) | (1ULL << p_Device->Tx)),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    gpio_config(&Conf);
    gpio_set_level(p_Device->Rx, true);
    gpio_set_level(p_Device->Tx, false);
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

    p_Device->Internal.isBusy = true;

    Command = "ATR\r\n";
    uart_write_bytes(p_Device->Interface, Command.c_str(), Command.length());

    RAK3172_ERROR_CHECK(RAK3172_ReceiveSplashScreen(p_Device, RAK3172_WAIT_TIMEOUT));

    ESP_LOGI(TAG, "     Successful!");

    return RAK3172_ERR_OK;    
}

RAK3172_Error_t RAK3172_SoftReset(RAK3172_t* const p_Device, uint32_t Timeout)
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

    RAK3172_ERROR_CHECK(RAK3172_ReceiveSplashScreen(p_Device, Timeout));

    ESP_LOGI(TAG, "     Successful!");

    return RAK3172_ERR_OK;
}

#ifdef CONFIG_RAK3172_RESET_USE_HW
    RAK3172_Error_t RAK3172_HardReset(RAK3172_t* const p_Device, uint32_t Timeout)
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

        p_Device->Internal.isBusy = true;

        if(p_Device->isResetInverted)
        {
            gpio_set_level(p_Device->Reset, true);
        }
        else
        {
            gpio_set_level(p_Device->Reset, false);
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS);

        if(p_Device->isResetInverted)
        {
            gpio_set_level(p_Device->Reset, false);
        }
        else
        {
            gpio_set_level(p_Device->Reset, true);
        }

        #ifndef CONFIG_RAK3172_USE_RUI3
            RAK3172_ERROR_CHECK(RAK3172_ReceiveSplashScreen(p_Device, Timeout));
        #else
            p_Device->Internal.isBusy = false;
        #endif

        ESP_LOGI(TAG, "     Successful!");

        return RAK3172_ERR_OK;
    }
#endif

RAK3172_Error_t RAK3172_SendCommand(const RAK3172_t* const p_Device, std::string Command, std::string* const p_Value, std::string* const p_Status)
{
    std::string* Response = NULL;
    RAK3172_Error_t Error = RAK3172_ERR_OK;

    if(p_Device == NULL)
    {
        return RAK3172_ERR_INVALID_ARG;
    }
    else if(p_Device->Internal.isBusy)
    {
        ESP_LOGE(TAG, "Device busy!");

        return RAK3172_ERR_FAIL;
    }
    else if(p_Device->Internal.isInitialized == false)
    {
        return RAK3172_ERR_INVALID_STATE;
    }

    // Clear the queue and drop all items.
    xQueueReset(p_Device->Internal.RxQueue);

    // Transmit the command.
    ESP_LOGI(TAG, "Transmit command: %s", Command.c_str());
    uart_write_bytes(p_Device->Interface, (const char*)Command.c_str(), Command.length());
    uart_write_bytes(p_Device->Interface, "\r\n", 2);

    // Copy the value if needed.
    if(p_Value != NULL)
    {
        if(xQueueReceive(p_Device->Internal.RxQueue, &Response, RAK3172_WAIT_TIMEOUT / portTICK_PERIOD_MS) != pdPASS)
        {
            return RAK3172_ERR_TIMEOUT;
        }

        #ifdef CONFIG_RAK3172_USE_RUI3
            // Remove the command from the response.
            size_t Index;

            Index = Response->find("=");
            *Response = Response->substr(Index + 1);
        #endif

        *p_Value = *Response;
        delete Response;

        ESP_LOGI(TAG, "     Value: %s", p_Value->c_str());
    }

    #ifndef CONFIG_RAK3172_USE_RUI3
        // Receive the line feed before the status.
        if(xQueueReceive(p_Device->Internal.RxQueue, &Response, RAK3172_WAIT_TIMEOUT / portTICK_PERIOD_MS) != pdPASS)
        {
            return RAK3172_ERR_TIMEOUT;
        }
        delete Response;
    #endif

    // Receive the trailing status code.
    if(xQueueReceive(p_Device->Internal.RxQueue, &Response, RAK3172_WAIT_TIMEOUT / portTICK_PERIOD_MS) != pdPASS)
    {
        return RAK3172_ERR_TIMEOUT;
    }

    ESP_LOGI(TAG, "     Status: %s", Response->c_str());

    // Transmission is without error when 'OK' as status code and when no event data are received.
    if(Response->find("OK") == std::string::npos)
    {
        Error = RAK3172_ERR_FAIL;
    }

    // Copy the status string if needed.
    if(p_Status != NULL)
    {
        *p_Status = *Response;
    }
    ESP_LOGD(TAG, "    Error: %i", (int)Error);

    delete Response;

    return Error;
}