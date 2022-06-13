 /*
 * rak3172_commands.cpp
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

#include <esp_log.h>

#include "../include/rak3172.h"

static const char* TAG = "RAK3172";

RAK3172_Error_t RAK3172_GetFWVersion(const RAK3172_t* const p_Device, std::string* const p_Version)
{
    if(p_Version == NULL)
    {
        return RAK3172_ERR_INVALID_ARG;
    }

    return RAK3172_SendCommand(p_Device, "AT+VER=?", p_Version, NULL);
}

RAK3172_Error_t RAK3172_GetSerialNumber(const RAK3172_t* const p_Device, std::string* const p_Serial)
{
    if(p_Serial == NULL)
    {
        return RAK3172_ERR_INVALID_ARG;
    }

    return RAK3172_SendCommand(p_Device, "AT+SN=?", p_Serial, NULL);
}

RAK3172_Error_t RAK3172_GetRSSI(const RAK3172_t* const p_Device, int* p_RSSI)
{
    std::string Value;

    if(p_RSSI == NULL)
    {
        return RAK3172_ERR_INVALID_ARG;
    }

    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+RSSI=?", &Value, NULL));

    *p_RSSI = std::stoi(Value);

    return RAK3172_ERR_OK;
}

RAK3172_Error_t RAK3172_GetSNR(const RAK3172_t* const p_Device, int* p_SNR)
{
    std::string Value;

    if(p_SNR == NULL)
    {
        return RAK3172_ERR_INVALID_ARG;
    }

    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+SNR=?", &Value, NULL));

    *p_SNR = std::stoi(Value);

    return RAK3172_ERR_OK;
}

RAK3172_Error_t RAK3172_SetMode(RAK3172_t* const p_Device, RAK3172_Mode_t Mode)
{
    std::string Command;
    std::string* Response;
    RAK3172_Error_t Error = RAK3172_ERR_OK;

    if(p_Device == NULL)
    {
        return RAK3172_ERR_INVALID_ARG;
    }
    else if(p_Device->Internal.isInitialized == false)
    {
        return RAK3172_ERR_INVALID_RESPONSE;
    }
    else if(p_Device->Mode == Mode)
    {
        return RAK3172_ERR_OK;
    }

    p_Device->Internal.isBusy = true;

    // Transmit the command.
    Command = "AT+NWM=" + std::to_string((uint32_t)Mode) + "\r\n";
    uart_write_bytes(p_Device->Interface, (const char*)Command.c_str(), Command.length());

    #ifndef CONFIG_RAK3172_USE_RUI3
        // Receive the line feed before the status.
        if(xQueueReceive(p_Device->Internal.RxQueue, &Response, 1000 / portTICK_PERIOD_MS) != pdPASS)
        {
            Error = RAK3172_ERR_TIMEOUT;
            goto RAK3172_SetMode_Error;
        }
        delete Response;
    #endif

    // Receive the trailing status code.
    if(xQueueReceive(p_Device->Internal.RxQueue, &Response, RAK3172_WAIT_TIMEOUT / portTICK_PERIOD_MS) != pdPASS)
    {
        Error = RAK3172_ERR_TIMEOUT;
        goto RAK3172_SetMode_Error;
    }

    // 'OK' received, so the mode wasn´t change. Leave the function.
    if(Response->find("OK") != std::string::npos)
    {
        delete Response;

        p_Device->Internal.isBusy = false;

        return RAK3172_ERR_OK;
    }

    // Otherwise the mode has changed and we have to receive the splash screen.
    delete Response;
    do
    {
        if(xQueueReceive(p_Device->Internal.RxQueue, &Response, RAK3172_WAIT_TIMEOUT / portTICK_PERIOD_MS) == pdFAIL)
        {
            p_Device->Internal.isBusy = false;
        }

        delete Response;
    } while(p_Device->Internal.isBusy);

    // The mode was changed. Set the new mode.
    p_Device->Mode = Mode;
    ESP_LOGI(TAG, "New mode: %u", p_Device->Mode);

RAK3172_SetMode_Error:
    p_Device->Internal.isBusy = false;
    return Error;
}

RAK3172_Error_t RAK3172_GetMode(RAK3172_t* const p_Device)
{
    std::string Value;

    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+NWM=?", &Value, NULL));

    p_Device->Mode = (RAK3172_Mode_t)std::stoi(Value);

    return RAK3172_ERR_OK;
}

RAK3172_Error_t RAK3172_SetBaud(const RAK3172_t* const p_Device, RAK3172_Baud_t Baud)
{
    if(p_Device->Baudrate == Baud)
    {
        return RAK3172_ERR_OK;
    }

    return RAK3172_SendCommand(p_Device, "AT+BAUD=" + std::to_string((uint32_t)Baud), NULL, NULL);
}

RAK3172_Error_t RAK3172_GetBaud(const RAK3172_t* const p_Device, RAK3172_Baud_t* p_Baud)
{
    std::string Value;

    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+BAUD=?", &Value, NULL));

    *p_Baud = (RAK3172_Baud_t)std::stoi(Value);

    return RAK3172_ERR_OK;
}