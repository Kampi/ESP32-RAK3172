 /*
 * rak3172_configs.h
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

#ifndef RAK3172_CONFIG_H_
#define RAK3172_CONFIG_H_

#include <sdkconfig.h>

#ifdef CONFIG_RAK3172_RESET_USE_HW
    /** @brief                  RAK3172 device object initialization macro.
     *  @param UART_Interface   UART that should be used
     *  @param Rx_Pin           UART Rx pin (MCU)
     *  @param Tx_Pin           UART Tx pin (MCU)
     *  @param Baud             UART baudrate
     */
    #define RAK3172_DEFAULT_CONFIG(UART_Interface, Rx_Pin, Tx_Pin, Baud, Reset_Pin)             {                                                               \
                                                                                                    .UART = {                                                   \
                                                                                                        .Interface = static_cast<uart_port_t>(UART_Interface),  \
                                                                                                        .Rx = static_cast<gpio_num_t>(Rx_Pin),                  \
                                                                                                        .Tx = static_cast<gpio_num_t>(Tx_Pin),                  \
                                                                                                        .Baudrate = static_cast<RAK3172_Baud_t>(Baud),          \
                                                                                                    },                                                          \
                                                                                                    .Reset = static_cast<gpio_num_t>(Reset_Pin),                \
                                                                                                    .Mode = RAK_MODE_P2P,                                       \
                                                                                                    .Info = NULL,                                               \
                                                                                                    .Internal = {                                               \
                                                                                                        .Handle = NULL,                                         \
                                                                                                        .isInitialized = false,                                 \
                                                                                                        .isBusy = false,                                        \
                                                                                                        .RxBuffer = NULL,                                       \
                                                                                                        .MessageQueue = NULL,                                   \
                                                                                                        .EventQueue = NULL,                                     \
                                                                                                        .ReceiveQueue = NULL,                                   \
                                                                                                        .isJoinEvent = false,                                   \
                                                                                                    },                                                          \
                                                                                                    .LoRaWAN = {                                                \
                                                                                                        .Join = RAK_JOIN_ABP,                                   \
                                                                                                        .isJoined = false,                                      \
                                                                                                        .ConfirmError = false,                                  \
                                                                                                        .AttemptCounter = 0,                                    \
                                                                                                    },                                                          \
                                                                                                    .P2P = {                                                    \
                                                                                                        .Active = false,                                        \
                                                                                                        .isEncryptionEnabled = false,                           \
                                                                                                        .isRxTimeout = false,                                   \
                                                                                                        .Timeout = 0,                                           \
                                                                                                        .ListenHandle = NULL,                                   \
                                                                                                        .ListenQueue = NULL,                                    \
                                                                                                    }                                                           \
                                                                                                }
#else
    /** @brief                  RAK3172 device object initialization macro.
     *  @param UART_Interface   UART that should be used
     *  @param Rx_Pin           UART Rx pin (MCU)
     *  @param Tx_Pin           UART Tx pin (MCU)
     *  @param Baud             UART baudrate
     */
    #define RAK3172_DEFAULT_CONFIG(UART_Interface, Rx_Pin, Tx_Pin, Baud)    {                                                                       \
                                                                                .UART = {                                                           \
                                                                                    .Interface = UART_Interface,                                    \
                                                                                    .Rx = static_cast<gpio_num_t>(Rx_Pin),                          \
                                                                                    .Tx = static_cast<gpio_num_t>(Tx_Pin),                          \
                                                                                    .Baudrate = static_cast<RAK3172_Baud_t>(Baud),                  \
                                                                                },                                                                  \
                                                                                .Mode = RAK_MODE_P2P,                                               \
                                                                                .Info = NULL,                                                       \
                                                                                .Internal = {                                                       \
                                                                                    .Handle = NULL,                                                 \
                                                                                    .isInitialized = false,                                         \
                                                                                    .isBusy = false,                                                \
                                                                                    .RxBuffer = NULL,                                               \
                                                                                    .MessageQueue = NULL,                                           \
                                                                                    .EventQueue = NULL,                                             \
                                                                                    .ReceiveQueue = NULL,                                           \
                                                                                    .isJoinEvent = false,                                           \
                                                                                },                                                                  \
                                                                                .LoRaWAN = {                                                        \
                                                                                    .Join = RAK_JOIN_ABP,                                           \
                                                                                    .isJoined = false,                                              \
                                                                                    .ConfirmError = false,                                          \
                                                                                    .AttemptCounter = 0,                                            \
                                                                                    .Class = static_cast<RAK3172_Class_t>('A'),                     \
                                                                                },                                                                  \
                                                                                .P2P = {                                                            \
                                                                                    .Active = false,                                                \
                                                                                    .isEncryptionEnabled = false,                                   \
                                                                                    .isRxTimeout = false,                                           \
                                                                                    .Timeout = 0,                                                   \
                                                                                    .ListenHandle = NULL,                                           \
                                                                                    .ListenQueue = NULL,                                            \
                                                                                }                                                                   \
                                                                            }
#endif

#endif /* RAK3172_CONFIG_H_ */