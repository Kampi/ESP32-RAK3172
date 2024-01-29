 /*
 * rak3172_uart.h
 *
 *  Copyright (C) Daniel Kampert, 2023
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

#ifndef RAK3172_UART_H_
#define RAK3172_UART_H_

#include "rak3172.h"

/** @brief          Perform the basic initialization of the UART driver.
 *  @param p_Device RAK3172 device object
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_NO_MEM when the message queues, the task or the receive buffer Cannot be created
 */
RAK3172_Error_t RAK3172_UART_Init(RAK3172_t& p_Device);

/** @brief          Deinitialize the UART driver.
 *  @param p_Device RAK3172 device object
 */
void RAK3172_UART_Deinit(RAK3172_t& p_Device);

/** @brief          Deinitialize the UART driver.
 *  @param p_Device RAK3172 device object
 *  @param Baudrate New baudrate
 *  @return         RAK3172_ERR_OK when successful
 */
RAK3172_Error_t RAK3172_UART_SetBaudrate(RAK3172_t& p_Device, RAK3172_Baud_t Baudrate);

/** @brief          Transmit bytes via UART.
 *  @param p_Device RAK3172 device object
 *  @param p_Buffer Pointer to data buffer
 *  @param Length   Length of data buffer
 *  @return         Number of bytes written or -1 when error
 */
int RAK3172_UART_WriteBytes(RAK3172_t& p_Device, const void* p_Buffer, size_t Length);

#endif /* RAK3172_UART_H_ */