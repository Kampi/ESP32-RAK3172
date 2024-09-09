 /*
 * rak3172_arch.h
 *
 *  Copyright (C) Daniel Kampert, 2023
 *	Website: www.kampis-elektroecke.de
 *  File info: RAK3172 arch specific driver.
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

#ifndef RAK3172_ARCH_H_
#define RAK3172_ARCH_H_

#include <sdkconfig.h>

#include "Flash/rak3172_flash.h"
#include "Logging/rak3172_logging.h"
#include "Timer/rak3172_timer.h"
#include "UART/rak3172_uart.h"
#include "Watchdog/rak3172_watchdog.h"
#include "GPIO/rak3172_gpio.h"

#ifdef CONFIG_RAK3172_PWRMGMT_ENABLE
    #include "PwrMgmt/rak3172_pwrmgmt.h"
#endif

#endif /* RAK3172_ARCH_H_ */