 /*
 * rak3172_gpio.cpp
 *
 *  Copyright (C) Daniel Kampert, 2025
 *	Website: www.kampis-elektroecke.de
 *  File info: ESP32 GPIO wrapper for the RAK3172 driver.
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

#include <driver/gpio.h>

#include "rak3172_gpio.h"

#ifdef CONFIG_RAK3172_RESET_USE_HW
    static gpio_config_t _RAK3172_GPIO_Reset_Config = {
        .pin_bit_mask       = 0,
        .mode               = GPIO_MODE_OUTPUT,
        .pull_up_en         = GPIO_PULLUP_DISABLE,
        .pull_down_en       = GPIO_PULLDOWN_DISABLE,
        .intr_type          = GPIO_INTR_DISABLE,
    };
#endif

RAK3172_Error_t RAK3172_GPIO_Init(RAK3172_t& p_Device)
{
    #ifdef CONFIG_RAK3172_RESET_USE_HW
        _RAK3172_GPIO_Reset_Config.pin_bit_mask = 1ULL << p_Device.Reset;

        // Configure the pull-up / pull-down resistor.
        #ifdef CONFIG_RAK3172_RESET_USE_PULL
            #ifdef CONFIG_RAK3172_RESET_INVERT
                _RAK3172_GPIO_Reset_Config.pull_down_en = GPIO_PULLDOWN_ENABLE;
            #else
                _RAK3172_GPIO_Reset_Config.pull_up_en = GPIO_PULLUP_ENABLE;
            #endif
        #endif

        if(gpio_config(&_RAK3172_GPIO_Reset_Config) != ESP_OK)
        {
            return RAK3172_ERR_INVALID_STATE;
        }

        // Set the reset pin when no pull-up / pull-down resistor should be used.
        #ifdef CONFIG_RAK3172_RESET_INVERT
            gpio_set_level(p_Device.Reset, false);
        #else
            gpio_set_level(p_Device.Reset, true);
        #endif
    #endif

    return RAK3172_ERR_OK;
}

#ifdef CONFIG_RAK3172_RESET_USE_HW
    RAK3172_Error_t RAK3172_GPIO_HardwareReset(RAK3172_t& p_Device)
    {
        if(p_Device.Internal.isInitialized == false)
        {
            return RAK3172_ERR_INVALID_STATE;
        }

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

        return RAK3172_ERR_OK; 
    }
#endif