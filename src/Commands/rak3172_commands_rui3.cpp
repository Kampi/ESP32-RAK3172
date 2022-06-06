 /*
 * rak3172_commands_rui3.cpp
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

#include <sdkconfig.h>

#ifdef CONFIG_RAK3172_USE_RUI3

#include "../include/rak3172.h"

RAK3172_Error_t RAK3172_GetCLIVersion(RAK3172_t* p_Device, std::string* p_Version)
{
    if(p_Version == NULL)
    {
        return RAK3172_ERR_INVALID_ARG;
    }

    return RAK3172_SendCommand(p_Device, "AT+CLIVER=?", p_Version, NULL);
}

RAK3172_Error_t RAK3172_GetAPIVersion(RAK3172_t* p_Device, std::string* p_Version)
{
    if(p_Version == NULL)
    {
        return RAK3172_ERR_INVALID_ARG;
    }

    return RAK3172_SendCommand(p_Device, "AT+APIVER=?", p_Version, NULL);
}

RAK3172_Error_t RAK3172_GetBuildTime(RAK3172_t* p_Device, std::string* p_BuildTime)
{
    if(p_BuildTime == NULL)
    {
        return RAK3172_ERR_INVALID_ARG;
    }

    return RAK3172_SendCommand(p_Device, "AT+BUILDTIME=?", p_BuildTime, NULL);
}

RAK3172_Error_t RAK3172_GetRepoInfo(RAK3172_t* p_Device, std::string* p_Repo)
{
    if(p_Repo == NULL)
    {
        return RAK3172_ERR_INVALID_ARG;
    }

    return RAK3172_SendCommand(p_Device, "AT+REPOINFO=?", p_Repo, NULL);
}

RAK3172_Error_t RAK3172_GetModel(RAK3172_t* p_Device, std::string* p_Model)
{
    if(p_Model == NULL)
    {
        return RAK3172_ERR_INVALID_ARG;
    }

    return RAK3172_SendCommand(p_Device, "AT+HWMODEL=?", p_Model, NULL);
}

RAK3172_Error_t RAK3172_GetHWID(RAK3172_t* p_Device, std::string* p_ID)
{
    if(p_ID == NULL)
    {
        return RAK3172_ERR_INVALID_ARG;
    }

    return RAK3172_SendCommand(p_Device, "AT+HWID=?", p_ID, NULL);
}

RAK3172_Error_t RAK3172_GetHWID(RAK3172_t* p_Device, uint32_t Period)
{
    return RAK3172_SendCommand(p_Device, "AT+SLEEP=" + std::to_string(Period), NULL, NULL);
}

#endif