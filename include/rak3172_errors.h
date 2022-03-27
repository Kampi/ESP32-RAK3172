 /*
 * rak3172_errors.h
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

#ifndef RAK3172_ERRORS_H_
#define RAK3172_ERRORS_H_

#include <stdint.h>

typedef int RAK3172_Error_t;

#define RAK3172_ERR_BASE                0xA000

/** @brief No error.
 */
#define RAK3172_OK                      0xA000

/** @brief Invalid function argument.
 */
#define RAK3172_INVALID_ARG             0xA001

/** @brief The when the interface is not initialized.
 */
#define RAK3172_INVALID_STATE           0xA002

/** @brief When an event happens, when the status is not "OK" or when the device is busy.
 */
#define RAK3172_FAIL                    0xA003

/** @brief When the number of JOIN attemps has expired.
 */
#define RAK3172_TIMEOUT                 0xA004

/** @brief When a send confirmation failed or when the device is busy.
 */
#define RAK3172_INVALID_RESPONSE        0xA005

#endif /* RAK3172_ERRORS_H_ */