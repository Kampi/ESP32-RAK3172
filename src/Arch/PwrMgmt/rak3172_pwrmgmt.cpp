 /*
 * rak3172_pwrmgmt.cpp
 *
 *  Copyright (C) Daniel Kampert, 2023
 *	Website: www.kampis-elektroecke.de
 *  File info: ESP32 Power Management wrapper for the RAK3172 driver.
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

#include <sdkconfig.h>

#ifdef CONFIG_RAK3172_PWRMGMT_ENABLE

#include <esp_sleep.h>

#include <driver/gpio.h>
#include <driver/uart.h>

#include "rak3172_pwrmgmt.h"

void RAK3172_PwrMagnt_EnterLightSleep(RAK3172_t& p_Device)
{
    /*
    gpio_sleep_set_direction(static_cast<gpio_num_t>(p_Device.UART.Rx), GPIO_MODE_INPUT);
    gpio_sleep_set_pull_mode(static_cast<gpio_num_t>(p_Device.UART.Rx), GPIO_PULLUP_ONLY);
    uart_set_wakeup_threshold(static_cast<uart_port_t>(p_Device.UART.Interface), 1);
    esp_sleep_enable_uart_wakeup(static_cast<uart_port_t>(p_Device.UART.Interface));
    esp_light_sleep_start();
    */
}

#endif