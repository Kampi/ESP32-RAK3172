 /*
 * rak3172_lorawan_rui3.cpp
 *
 *  Copyright (C) Daniel Kampert, 2022
 *	Website: www.kampis-elektroecke.de
 *  File info: LoRaWAN module of the RAK3172 driver for ESP32.

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

#include <sdkconfig.h>

#if((defined CONFIG_RAK3172_USE_RUI3) & (defined CONFIG_RAK3172_WITH_LORAWAN))

#include "rak3172.h"

RAK3172_Error_t RAK3172_LoRaWAN_GetNetID(const RAK3172_t* const p_Device, std::string* const p_ID)
{
    if(p_Device == NULL)
    {
        return RAK3172_ERR_INVALID_ARG;
    }
    else if(p_Device->Internal.isInitialized == false)
    {
        return RAK3172_ERR_INVALID_RESPONSE;
    }
    else if(p_Device->LoRaWAN.isJoined == false)
    {
        return RAK3172_ERR_NOT_CONNECTED;
    }

    return RAK3172_SendCommand(p_Device, "AT+NETID=?", p_ID, NULL);
}

#endif