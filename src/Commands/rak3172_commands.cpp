 /*
 * rak3172_commands.cpp
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

#include <esp_log.h>

#include "rak3172.h"

static const char* TAG = "RAK3172";

RAK3172_Error_t RAK3172_SendCommand(const RAK3172_t& p_Device, std::string Command, std::string* const p_Value, std::string* const p_Status)
{
    std::string* Response = NULL;
    RAK3172_Error_t Error = RAK3172_ERR_OK;

    if(p_Device.Internal.isBusy)
    {
        ESP_LOGE(TAG, "Device busy!");

        return RAK3172_ERR_BUSY;
    }
    else if(p_Device.Internal.isInitialized == false)
    {
        return RAK3172_ERR_INVALID_STATE;
    }

    // Clear the queue and drop all items.
    xQueueReset(p_Device.Internal.MessageQueue);

    // Transmit the command.
    ESP_LOGI(TAG, "Transmit command: %s", Command.c_str());
    uart_write_bytes(p_Device.Interface, (const char*)Command.c_str(), Command.length());
    uart_write_bytes(p_Device.Interface, "\r\n", 2);

    // Copy the value if needed.
    if(p_Value != NULL)
    {
        if(xQueueReceive(p_Device.Internal.MessageQueue, &Response, RAK3172_WAIT_TIMEOUT / portTICK_PERIOD_MS) != pdPASS)
        {
            return RAK3172_ERR_TIMEOUT;
        }

        #ifdef CONFIG_RAK3172_USE_RUI3
            // Remove the command from the response.
            size_t Index;

            Index = Response->find("=");
            *Response = Response->substr(Index + 1);
        #endif

        *p_Value = *Response;
        delete Response;

        ESP_LOGI(TAG, "     Value: %s", p_Value->c_str());
    }

    #ifndef CONFIG_RAK3172_USE_RUI3
        // Receive the line feed before the status.
        if(xQueueReceive(p_Device.Internal.MessageQueue, &Response, RAK3172_WAIT_TIMEOUT / portTICK_PERIOD_MS) != pdPASS)
        {
            return RAK3172_ERR_TIMEOUT;
        }
        delete Response;
    #endif

    // Receive the trailing status code.
    if(xQueueReceive(p_Device.Internal.MessageQueue, &Response, RAK3172_WAIT_TIMEOUT / portTICK_PERIOD_MS) != pdPASS)
    {
        return RAK3172_ERR_TIMEOUT;
    }

    ESP_LOGI(TAG, "     Status: %s", Response->c_str());

    // Transmission is without error when 'OK' as status code and when no event data are received.
    if(Response->find("OK") == std::string::npos)
    {
        Error = RAK3172_ERR_FAIL;
    }

    // Copy the status string if needed.
    if(p_Status != NULL)
    {
        *p_Status = *Response;
    }
    ESP_LOGD(TAG, "    Error: 0x%X", (int)Error);

    delete Response;

    return Error;
}

RAK3172_Error_t RAK3172_GetFWVersion(const RAK3172_t& p_Device, std::string* const p_Version)
{
    if(p_Version == NULL)
    {
        return RAK3172_ERR_INVALID_ARG;
    }

    return RAK3172_SendCommand(p_Device, "AT+VER=?", p_Version);
}

RAK3172_Error_t RAK3172_GetSerialNumber(const RAK3172_t& p_Device, std::string* const p_Serial)
{
    if(p_Serial == NULL)
    {
        return RAK3172_ERR_INVALID_ARG;
    }

    return RAK3172_SendCommand(p_Device, "AT+SN=?", p_Serial);
}

RAK3172_Error_t RAK3172_SetMode(RAK3172_t& p_Device, RAK3172_Mode_t Mode)
{
    std::string Command;
    std::string* Response;
    RAK3172_Error_t Error = RAK3172_ERR_OK;

    if(p_Device.Internal.isInitialized == false)
    {
        return RAK3172_ERR_INVALID_RESPONSE;
    }
    else if(p_Device.Mode == Mode)
    {
        return RAK3172_ERR_OK;
    }

    p_Device.Internal.isBusy = true;

    // Transmit the command.
    Command = "AT+NWM=" + std::to_string((uint32_t)Mode) + "\r\n";
    uart_write_bytes(p_Device.Interface, (const char*)Command.c_str(), Command.length());

    #ifndef CONFIG_RAK3172_USE_RUI3
        // Receive the line feed before the status.
        if(xQueueReceive(p_Device.Internal.MessageQueue, &Response, RAK3172_WAIT_TIMEOUT / portTICK_PERIOD_MS) != pdPASS)
        {
            Error = RAK3172_ERR_TIMEOUT;
            goto RAK3172_SetMode_Exit;
        }
        delete Response;
    #endif

    // Receive the trailing status code.
    if(xQueueReceive(p_Device.Internal.MessageQueue, &Response, RAK3172_WAIT_TIMEOUT / portTICK_PERIOD_MS) != pdPASS)
    {
        Error = RAK3172_ERR_TIMEOUT;
        goto RAK3172_SetMode_Exit;
    }

    // 'OK' received, so the mode wasn´t change. Leave the function.
    if(Response->find("OK") != std::string::npos)
    {
        delete Response;

        Error = RAK3172_ERR_OK;
        goto RAK3172_SetMode_Exit;
    }

    // Otherwise the mode has changed and we have to receive the splash screen.
    delete Response;
    do
    {
        if(xQueueReceive(p_Device.Internal.MessageQueue, &Response, RAK3172_WAIT_TIMEOUT / portTICK_PERIOD_MS) == pdFAIL)
        {
            p_Device.Internal.isBusy = false;
        }
        else
        {
            delete Response;
        }
    } while(p_Device.Internal.isBusy);

    // The mode was changed. Set the new mode.
    p_Device.Mode = Mode;
    ESP_LOGD(TAG, "New mode: %u", p_Device.Mode);

RAK3172_SetMode_Exit:
    p_Device.Internal.isBusy = false;
    return Error;
}

RAK3172_Error_t RAK3172_GetMode(RAK3172_t& p_Device)
{
    std::string Value;

    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+NWM=?", &Value));

    p_Device.Mode = (RAK3172_Mode_t)std::stoi(Value);

    return RAK3172_ERR_OK;
}

RAK3172_Error_t RAK3172_SetBaud(const RAK3172_t& p_Device, RAK3172_Baud_t Baud)
{
    if(p_Device.Baudrate == Baud)
    {
        return RAK3172_ERR_OK;
    }

    return RAK3172_SendCommand(p_Device, "AT+BAUD=" + std::to_string((uint32_t)Baud));
}

RAK3172_Error_t RAK3172_GetBaud(const RAK3172_t& p_Device, RAK3172_Baud_t* p_Baud)
{
    std::string Value;

    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+BAUD=?", &Value));

    *p_Baud = (RAK3172_Baud_t)std::stoi(Value);

    return RAK3172_ERR_OK;
}