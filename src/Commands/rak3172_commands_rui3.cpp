 /*
 * rak3172_commands_rui3.cpp
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

#ifdef CONFIG_RAK3172_USE_RUI3

#include "rak3172.h"

RAK3172_Error_t RAK3172_GetCLIVersion(const RAK3172_t& p_Device, std::string* const p_Version)
{
    if(p_Version == NULL)
    {
        return RAK3172_ERR_INVALID_ARG;
    }

    return RAK3172_SendCommand(p_Device, "AT+CLIVER=?", p_Version);
}

RAK3172_Error_t RAK3172_GetAPIVersion(const RAK3172_t& p_Device, std::string* const p_Version)
{
    if(p_Version == NULL)
    {
        return RAK3172_ERR_INVALID_ARG;
    }

    return RAK3172_SendCommand(p_Device, "AT+APIVER=?", p_Version);
}

RAK3172_Error_t RAK3172_GetBuildTime(const RAK3172_t& p_Device, std::string* const p_BuildTime)
{
    if(p_BuildTime == NULL)
    {
        return RAK3172_ERR_INVALID_ARG;
    }

    return RAK3172_SendCommand(p_Device, "AT+BUILDTIME=?", p_BuildTime);
}

RAK3172_Error_t RAK3172_GetRepoInfo(const RAK3172_t& p_Device, std::string* const p_Repo)
{
    if(p_Repo == NULL)
    {
        return RAK3172_ERR_INVALID_ARG;
    }

    return RAK3172_SendCommand(p_Device, "AT+REPOINFO=?", p_Repo);
}

RAK3172_Error_t RAK3172_GetModel(const RAK3172_t& p_Device, std::string* const p_Model)
{
    if(p_Model == NULL)
    {
        return RAK3172_ERR_INVALID_ARG;
    }

    return RAK3172_SendCommand(p_Device, "AT+HWMODEL=?", p_Model);
}

RAK3172_Error_t RAK3172_GetHWID(const RAK3172_t& p_Device, std::string* const p_ID)
{
    if(p_ID == NULL)
    {
        return RAK3172_ERR_INVALID_ARG;
    }

    return RAK3172_SendCommand(p_Device, "AT+HWID=?", p_ID);
}

RAK3172_Error_t RAK3172_Sleep(const RAK3172_t& p_Device, uint32_t Duration)
{
    return RAK3172_SendCommand(p_Device, "AT+SLEEP=" + std::to_string(Duration));
}

RAK3172_Error_t RAK3172_Lock(const RAK3172_t& p_Device, std::string Password)
{
    if((Password.length() < 1) || (Password.length() > 8))
    {
        return RAK3172_ERR_INVALID_ARG;
    }

    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+PWORD=" + Password));

    uart_write_bytes(p_Device.UART.Interface, "AT+LOCK\r\n", std::string("AT+LOCK\r\n").length());

    return RAK3172_ERR_FAIL;
}

RAK3172_Error_t RAK3172_Unlock(const RAK3172_t& p_Device, std::string Password)
{
    if((Password.length() < 1) || (Password.length() > 8))
    {
        return RAK3172_ERR_INVALID_ARG;
    }

    uart_write_bytes(p_Device.UART.Interface, Password.c_str(), Password.length());

    return RAK3172_ERR_FAIL;
}

#endif