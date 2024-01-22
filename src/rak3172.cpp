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

#include "Arch/UART/rak3172_uart.h"
#include "Arch/Logging/rak3172_logging.h"

#ifdef CONFIG_RAK3172_RESET_USE_HW
    static gpio_config_t _RAK3172_Reset_Config = {
        .pin_bit_mask       = 0,
        .mode               = GPIO_MODE_OUTPUT,
        .pull_up_en         = GPIO_PULLUP_DISABLE,
        .pull_down_en       = GPIO_PULLDOWN_DISABLE,
        .intr_type          = GPIO_INTR_DISABLE,
    };
#endif

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

    RAK3172_LOGI(TAG, "Modes enabled:");
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

    RAK3172_ERROR_CHECK(RAK3172_UART_Init(p_Device));

    #ifdef CONFIG_RAK3172_RESET_USE_HW
        RAK3172_ERROR_CHECK(RAK3172_HardReset(p_Device));
    #else
        RAK3172_ERROR_CHECK(RAK3172_SoftReset(p_Device));
    #endif

    vTaskDelay(500 / portTICK_PERIOD_MS);

    // Firmware without RUI3 will produce a MIC mismatch when using a factory reset during the initialization.
    #if((defined CONFIG_RAK3172_FACTORY_RESET) && (defined CONFIG_RAK3172_USE_RUI3))
        RAK3172_ERROR_CHECK(RAK3172_FactoryReset(p_Device));
        vTaskDelay(500 / portTICK_PERIOD_MS);
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
        RAK3172_UART_WriteBytes(p_Device, "ATE\r\n", std::string("ATE\r\n").length());
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
    if(p_Device.Internal.Handle != NULL)
    {
        vTaskSuspend(p_Device.Internal.Handle);
        vTaskDelete(p_Device.Internal.Handle);
    }

    RAK3172_UART_Deinit(p_Device);

    if(p_Device.Internal.MessageQueue != NULL)
    {
        vQueueDelete(p_Device.Internal.MessageQueue);
    }

    if(p_Device.Internal.ReceiveQueue != NULL)
    {
        vQueueDelete(p_Device.Internal.ReceiveQueue);
    }

    if(p_Device.Internal.RxBuffer != NULL)
    {
        free(p_Device.Internal.RxBuffer);
        p_Device.Internal.RxBuffer = NULL;
    }

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

    return RAK3172_UART_SetBaudrate(p_Device, Baudrate);
}

RAK3172_Error_t RAK3172_WakeUp(RAK3172_t& p_Device)
{
    std::string Response;

    if(p_Device.Internal.isInitialized == true)
    {
        return RAK3172_ERR_OK;
    }

    RAK3172_LOGI(TAG, "Wake up driver from sleep mode...");

    RAK3172_ERROR_CHECK(RAK3172_UART_Init(p_Device));

    p_Device.Internal.isBusy = false;

    return RAK3172_SendCommand(p_Device, "AT");
}

RAK3172_Error_t RAK3172_FactoryReset(RAK3172_t& p_Device)
{
    std::string Command;

    if(p_Device.Internal.isInitialized == false)
    {
        return RAK3172_ERR_INVALID_STATE;
    }

    RAK3172_LOGI(TAG, "Perform factory reset...");

    p_Device.Internal.isBusy = true;
    Command = "ATR\r\n";
    RAK3172_UART_WriteBytes(p_Device, Command.c_str(), Command.length());
    RAK3172_ERROR_CHECK(RAK3172_ReceiveSplashScreen(p_Device, RAK3172_DEFAULT_WAIT_TIMEOUT));

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
    RAK3172_UART_WriteBytes(p_Device, Command.c_str(), Command.length());

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
