 /*
 * rak3172_lorawan_rui3.h
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

#ifndef RAK3172_LORAWAN_RUI3_H_
#define RAK3172_LORAWAN_RUI3_H_

#include "Definitions/rak3172_defs.h"
#include "Definitions/rak3172_errors.h"

/** @brief          Get the network ID of the current network.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param p_Enable Pointer to adaptive data rate status
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument was passed
 *                  RAK3172_ERR_INVALID_STATE the when the interface is not initialized
 */
RAK3172_Error_t RAK3172_LoRaWAN_GetNetID(RAK3172_t* p_Device, std::string* p_ID);

#endif /* RAK3172_LORAWAN_RUI3_H_ */