 /*
 * rak3172_lorawan_classb.cpp
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

#if(defined CONFIG_RAK3172_MODE_WITH_LORAWAN) && (defined CONFIG_RAK3172_MODE_WITH_LORAWAN_CLASS_B)

#include <string.h>

#include "rak3172.h"

/** @brief              Get a substring from the input string. The substring is delimited by the given delimiter.
 *  @param p_Input      Pointer to input string
 *                      NOTE: The input string will be modified!
 *  @param Delimiter    Substring delimiter
 *  @return             Substring
 */
static std::string RAK3172_SubstringSplitErase(std::string* p_Input, std::string Delimiter)
{
    size_t Index;
    std::string Result = std::string();

    assert(p_Input);

    Index = p_Input->find(Delimiter);
    if(Index != std::string::npos)
    {
        Result = p_Input->substr(0, Index);
        p_Input->erase(0, Index + 1);
    }

    return Result;
}

RAK3172_Error_t RAK3172_LoRaWAN_GetBeaconFrequency(RAK3172_t& p_Device, RAK3172_DataRate_t* p_Datarate, uint32_t* p_Frequency)
{
    size_t Index;
    std::string Response;

    if((p_Datarate == NULL) || (p_Frequency == NULL))
    {
        return RAK3172_ERR_INVALID_ARG;
    }
    else if(p_Device.Mode != RAK_MODE_LORAWAN)
    {
        return RAK3172_ERR_INVALID_MODE;
    }

    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+BFREQ=?", &Response));

    #ifdef CONFIG_RAK3172_USE_RUI3
        // Remove the leading "BCON: " string.
        Index = Response.find("BCON: ");
        if(Index == std::string::npos)
        {
            return RAK3172_ERR_INVALID_RESPONSE;
        }
        else
        {
            Response.erase(0, Index + std::string("BCON: ").size());
        }
    #endif

    Index = Response.find(",");
    *p_Datarate = static_cast<RAK3172_DataRate_t>(std::stoi(Response.substr(0, Index)));
    Response.erase(0, Index + 1);
    *p_Frequency = static_cast<uint32_t>(std::stoi(Response));

    return RAK3172_ERR_OK;
}

#ifdef CONFIG_RAK3172_USE_RUI3
    RAK3172_Error_t RAK3172_LoRaWAN_GetBeaconTime(RAK3172_t& p_Device, uint32_t* p_Time)
    {
        size_t Index;
        std::string Response;

        if(p_Time == NULL)
        {
            return RAK3172_ERR_INVALID_ARG;
        }
        else if(p_Device.Mode != RAK_MODE_LORAWAN)
        {
            return RAK3172_ERR_INVALID_MODE;
        }

        RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+BTIME=?", &Response));
        Index = Response.find("BTIME: ");
        if(Index == std::string::npos)
        {
            return RAK3172_ERR_INVALID_RESPONSE;
        }

        Response.erase(0, Index + std::string("BTIME: ").size());
        *p_Time = static_cast<uint32_t>(std::stoi(Response));

        return RAK3172_ERR_OK;
    }

    RAK3172_Error_t RAK3172_LoRaWAN_GetGatewayInfo(RAK3172_t& p_Device, std::string* p_NetID, std::string* p_GatewayID, std::string* p_Longitude, std::string* p_Latitude)
    {
        std::string Response;

        if((p_NetID == NULL) || (p_GatewayID == NULL) || (p_Longitude == NULL) || (p_Latitude == NULL))
        {
            return RAK3172_ERR_INVALID_ARG;
        }
        else if(p_Device.Mode != RAK_MODE_LORAWAN)
        {
            return RAK3172_ERR_INVALID_MODE;
        }

        RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+BGW=?", &Response));

        return RAK3172_ERR_OK;
    }
#endif

RAK3172_Error_t RAK3172_LoRaWAN_GetLocalTime(RAK3172_t& p_Device, struct tm* p_DateTime)
{
    size_t Index;
    std::string Response;

    if(p_DateTime == NULL)
    {
        return RAK3172_ERR_INVALID_ARG;
    }
    else if(p_Device.Mode != RAK_MODE_LORAWAN)
    {
        return RAK3172_ERR_INVALID_MODE;
    }

    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+LTIME=?", &Response));
    Index = Response.find("LTIME:");
    if(Index == std::string::npos)
    {
        return RAK3172_ERR_INVALID_RESPONSE;
    }

    memset(p_DateTime, 0, sizeof(struct tm));

    #ifdef CONFIG_RAK3172_USE_RUI3
        Response.erase(0, Index + std::string("LTIME: ").size());
        Index = Response.find(" ");
        Response.erase(Index, std::string(" ").size());

        // We continue with a string with the format "00h37m58s2018-11-14" here.
        p_DateTime->tm_hour = std::stoi(RAK3172_SubstringSplitErase(&Response, "h"));
        p_DateTime->tm_min = std::stoi(RAK3172_SubstringSplitErase(&Response, "m"));
        p_DateTime->tm_sec = std::stoi(RAK3172_SubstringSplitErase(&Response, "s"));
        p_DateTime->tm_year = std::stoi(RAK3172_SubstringSplitErase(&Response, "-")) - 1900;
        p_DateTime->tm_mon = std::stoi(RAK3172_SubstringSplitErase(&Response, "-"));
        p_DateTime->tm_mday = std::stoi(Response);
    #else
        Response.erase(0, Index + std::string("LTIME:").size());

        // Remove the "on" string between the time and the date.
        Index = Response.find(" on ");
        Response.erase(Index, std::string(" on ").size());

        // We continue with a string with the format "00h37m58s14-11-2018" here.
        p_DateTime->tm_hour = std::stoi(RAK3172_SubstringSplitErase(&Response, "h"));
        p_DateTime->tm_min = std::stoi(RAK3172_SubstringSplitErase(&Response, "m"));
        p_DateTime->tm_sec = std::stoi(RAK3172_SubstringSplitErase(&Response, "s"));
        p_DateTime->tm_mday = std::stoi(RAK3172_SubstringSplitErase(&Response, "/"));
        p_DateTime->tm_mon = std::stoi(RAK3172_SubstringSplitErase(&Response, "/"));
        p_DateTime->tm_year = std::stoi(Response) - 1900;
    #endif

    return RAK3172_ERR_OK;
}

RAK3172_Error_t RAK3172_LoRaWAN_SetPeriodicity(RAK3172_t& p_Device, uint8_t Periodicity)
{
    if(Periodicity > 7)
    {
        return RAK3172_ERR_INVALID_ARG;
    }
    else if(p_Device.Mode != RAK_MODE_LORAWAN)
    {
        return RAK3172_ERR_INVALID_MODE;
    }

    return RAK3172_SendCommand(p_Device, "AT+PGSLOT=" + std::to_string(Periodicity));
}

RAK3172_Error_t RAK3172_LoRaWAN_GetPeriodicity(RAK3172_t& p_Device, uint8_t* p_Periodicity)
{
    std::string Response;

    if(p_Periodicity == NULL)
    {
        return RAK3172_ERR_INVALID_ARG;
    }
    else if(p_Device.Mode != RAK_MODE_LORAWAN)
    {
        return RAK3172_ERR_INVALID_MODE;
    }

    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+PGSLOT=?", &Response));

    *p_Periodicity = static_cast<uint8_t>(std::stoi(Response));

    return RAK3172_ERR_OK;
}

#endif