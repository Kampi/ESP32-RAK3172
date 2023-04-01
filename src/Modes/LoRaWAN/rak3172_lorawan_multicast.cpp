 /*
 * rak3172_lorawan_multicast.cpp
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

#include <sdkconfig.h>

#if(defined CONFIG_RAK3172_MODE_WITH_LORAWAN) && (defined CONFIG_RAK3172_MODE_WITH_LORAWAN_MULTICAST)

#include <vector>

#include "rak3172.h"

RAK3172_Error_t RAK3172_LoRaWAN_MC_AddGroup(RAK3172_t& p_Device, RAK3172_MC_Group_t Group)
{
    return RAK3172_LoRaWAN_MC_AddGroup(p_Device, Group.Class, Group.DevAddr, Group.NwkSKey, Group.AppSKey, Group.Frequency, Group.Datarate, Group.Periodicity);
}

RAK3172_Error_t RAK3172_LoRaWAN_MC_AddGroup(RAK3172_t& p_Device, RAK3172_Class_t Class, std::string DevAddr, std::string NwkSKey, std::string AppSKey, uint32_t Frequency, RAK3172_DataRate_t Datarate, uint8_t Periodicity)
{
    std::string Command;

    if(((Class != RAK_CLASS_B) && (Class != RAK_CLASS_C)) || (DevAddr.size() == 0) || (NwkSKey.size() == 0) || (AppSKey.size() == 0) || (Frequency < 150000000) || (Frequency > 960000000) || ((Class == RAK_CLASS_B) && (Periodicity > 7)))
    {
        return RAK3172_ERR_INVALID_ARG;
    }
    else if(p_Device.Mode != RAK_MODE_LORAWAN)
    {
        return RAK3172_ERR_INVALID_MODE;
    }

    Command = "AT+ADDMULC=";
    Command += Class;
    Command += ":" + DevAddr + ":" + NwkSKey + ":" + AppSKey + ":" + std::to_string(Frequency) + ":" + std::to_string(Datarate) + ":" + std::to_string(Periodicity);

    return RAK3172_SendCommand(p_Device, Command);
}

RAK3172_Error_t RAK3172_LoRaWAN_MC_RemoveGroup(RAK3172_t& p_Device, RAK3172_MC_Group_t Group)
{
    return RAK3172_LoRaWAN_MC_RemoveGroup(p_Device, Group.DevAddr);
}

RAK3172_Error_t RAK3172_LoRaWAN_MC_RemoveGroup(RAK3172_t& p_Device, std::string DevAddr)
{
    if(DevAddr.size() == 0)
    {
        return RAK3172_ERR_INVALID_ARG;
    }
    else if(p_Device.Mode != RAK_MODE_LORAWAN)
    {
        return RAK3172_ERR_INVALID_MODE;
    }

    return RAK3172_SendCommand(p_Device, "AT+ADDMULC=" + DevAddr);
}

RAK3172_Error_t RAK3172_LoRaWAN_MC_ListGroup(RAK3172_t& p_Device, RAK3172_MC_Group_t* p_Group)
{
    size_t Index;
    std::string Response;
    std::vector<std::string> SubStrings;

    if(p_Group == NULL)
    {
        return RAK3172_ERR_INVALID_ARG;
    }
    else if(p_Device.Mode != RAK_MODE_LORAWAN)
    {
        return RAK3172_ERR_INVALID_MODE;
    }

    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+LSTMULC=?", &Response));

    do
    {
        Index = Response.find(":");
        if(Index != std::string::npos)
        {
            SubStrings.push_back(Response.substr(0, Index));
            Response.erase(0, Index + 1);
        }
    } while(Index != std::string::npos);

    if(SubStrings.size() != 5)
    {
        return RAK3172_ERR_INVALID_RESPONSE;
    }

    p_Group->Class = static_cast<RAK3172_Class_t>(SubStrings.at(0).at(0));
    p_Group->DevAddr = SubStrings.at(1);
    p_Group->NwkSKey = SubStrings.at(2);
    p_Group->AppSKey = SubStrings.at(3);
    p_Group->Frequency = std::stoi(SubStrings.at(4));
    p_Group->Datarate = static_cast<RAK3172_DataRate_t>(std::stoi(SubStrings.at(5)));

    return RAK3172_ERR_OK;
}

#endif