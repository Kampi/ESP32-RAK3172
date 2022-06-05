 /*
 * rak3172_configs.h
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

#ifndef RAK3172_CONFIG_H_
#define RAK3172_CONFIG_H_

#include <sdkconfig.h>

#ifdef CONFIG_RAK3172_RESET_USE_HW
    /** @brief              RAK3172 device object initialization macro.
     *  @param UART         UART that should be used
     *  @param Rx_Pin       UART Rx pin
     *  @param Tx_Pin       UART Tx pin
     *  @param Baud         UART baudrate
     *  @param Reset_Pin    Reset pin
     *  @param Reset_Inv    Invert reset signal
     */
    #define RAK3172_DEFAULT_CONFIG(UART, Rx_Pin, Tx_Pin, Baud, Reset_Pin, Reset_Inv)    {                                                   \
                                                                                            .Interface = UART,                              \
                                                                                            .Rx = (gpio_num_t)Rx_Pin,                       \
                                                                                            .Tx = (gpio_num_t)Tx_Pin,                       \
                                                                                            .Reset = (gpio_num_t)Reset_Pin,                 \
                                                                                            .isResetInverted = Reset_Inv,                   \
                                                                                            .Baudrate = (RAK3172_Baud_t)Baud,               \
                                                                                            .Mode = RAK_MODE_P2P,                           \
                                                                                            .Firmware = "",                                 \
                                                                                            .Serial = "",                                   \
                                                                                            .Internal = {                                   \
                                                                                                .Handle = NULL,                             \
                                                                                                .isInitialized = false,                     \
                                                                                                .isBusy = false,                            \
                                                                                                .RxBuffer = NULL,                           \
                                                                                                .RxQueue = NULL                             \
                                                                                            },                                              \
                                                                                            .LoRaWAN = {                                    \
                                                                                                .Join = RAK_JOIN_ABP,                       \
                                                                                            },                                              \
                                                                                            .P2P = {                                        \
                                                                                                .Queue = NULL,                              \
                                                                                                .Active = false,                            \
                                                                                                .Timeout = 0,                               \
                                                                                                .Handle = NULL                              \
                                                                                            }                                               \
                                                                                        }
#else
    /** @brief          RAK3172 device object initialization macro.
     *  @param UART     UART that should be used
     *  @param Rx_Pin   UART Rx pin
     *  @param Tx_Pin   UART Tx pin
     *  @param Baud     UART baudrate
     */
    #define RAK3172_DEFAULT_CONFIG(UART, Rx_Pin, Tx_Pin, Baud)      {                                                                       \
                                                                        .Interface = UART,                                                  \
                                                                        .Rx = (gpio_num_t)Rx_Pin,                                           \
                                                                        .Tx = (gpio_num_t)Tx_Pin,                                           \
                                                                        .Baudrate = (RAK3172_Baud_t)Baud,                                   \
                                                                        .Mode = RAK_MODE_P2P,                                               \
                                                                        .Firmware = "",                                                     \
                                                                        .Serial = "",                                                       \
                                                                        .Internal = {                                                       \
                                                                            .Handle = NULL,                                                 \
                                                                            .isInitialized = false,                                         \
                                                                            .isBusy = false,                                                \
                                                                            .RxBuffer = NULL,                                               \
                                                                            .RxQueue = NULL                                                 \
                                                                        },                                                                  \
                                                                        .LoRaWAN = {                                                        \
                                                                            .Join = RAK_JOIN_ABP,                                           \
                                                                        },                                                                  \
                                                                        .P2P = {                                                            \
                                                                            .Queue = NULL,                                                  \
                                                                            .Active = false,                                                \
                                                                            .TimeOut = 0,                                                   \
                                                                            .Handle = NULL                                                  \
                                                                        }                                                                   \
                                                                    }
#endif

#endif /* RAK3172_CONFIG_H_ */