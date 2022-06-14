 /*
 * rak3172_lorawan_rui3.cpp
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

RAK3172_Error_t RAK3172_LoRaWAN_SetSingleChannelMode(const RAK3172_t* const p_Device, bool Enable)
{
    return RAK3172_SendCommand(p_Device, "AT+CHS=" + std::to_string(Enable), NULL, NULL);
}

RAK3172_Error_t RAK3172_LoRaWAN_GetSingleChannelMode(const RAK3172_t* const p_Device, bool* const p_Enable)
{
    std::string Value;

    if(p_Enable == NULL)
    {
        return RAK3172_ERR_INVALID_ARG;
    }

    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+CHS=?", &Value, NULL));

    *p_Enable = (bool)std::stoi(Value);

    return RAK3172_ERR_OK;
}

RAK3172_Error_t RAK3172_LoRaWAN_SetEightChannelMode(const RAK3172_t* const p_Device, bool Enable)
{
    return RAK3172_SendCommand(p_Device, "AT+CHE=" + std::to_string(Enable), NULL, NULL);
}

RAK3172_Error_t RAK3172_LoRaWAN_GetEightChannelMode(const RAK3172_t* const p_Device, bool* const p_Enable)
{
    std::string Value;

    if(p_Enable == NULL)
    {
        return RAK3172_ERR_INVALID_ARG;
    }

    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+CHE=?", &Value, NULL));

    *p_Enable = (bool)std::stoi(Value);

    return RAK3172_ERR_OK;
}

#endif