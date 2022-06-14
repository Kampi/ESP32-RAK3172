 /*
 * rak3172_errors.h
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

#ifndef RAK3172_ERRORS_H_
#define RAK3172_ERRORS_H_

#include <stdint.h>

typedef uint32_t RAK3172_Error_t;

/** @brief RAK3172 error base.
 */
#define RAK3172_ERR_BASE                0xA000

/** @brief No error.
 */
#define RAK3172_ERR_OK                  (RAK3172_ERR_BASE + 0)

/** @brief Invalid function argument.
 */
#define RAK3172_ERR_INVALID_ARG         (RAK3172_ERR_BASE + 1)

/** @brief The when the interface is not initialized.
 */
#define RAK3172_ERR_INVALID_STATE       (RAK3172_ERR_BASE + 2)

/** @brief When an event happens, when the status is not "OK" or when the device is busy.
 */
#define RAK3172_ERR_FAIL                (RAK3172_ERR_BASE + 3)

/** @brief When the number of JOIN attemps has expired.
 */
#define RAK3172_ERR_TIMEOUT             (RAK3172_ERR_BASE + 4)

/** @brief When a send confirmation failed or when the device is busy.
 */
#define RAK3172_ERR_INVALID_RESPONSE    (RAK3172_ERR_BASE + 5)

/** @brief Mo memory for memory allocation available.
 */
#define RAK3172_ERR_NO_MEM              (RAK3172_ERR_BASE + 5)

/** @brief The device is not connected (LoRaWAN only).
 */
#define RAK3172_ERR_NOT_CONNECTED       (RAK3172_ERR_BASE + 6)

#endif /* RAK3172_ERRORS_H_ */