 /*
 * rak3172.cpp
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

#include <driver/uart.h>
#include <esp32-hal-log.h>

#include "../include/rak3172.h"

static const char* TAG = "RAK3172";

esp_err_t RAK3172_Init(RAK3172_t* p_Device)
{
    String Response;

    if(p_Device == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    p_Device->isInitialized = false;

    if(p_Device->p_Interface == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    p_Device->p_Interface->end();
    p_Device->p_Interface->begin(p_Device->Baudrate, SERIAL_8N1, p_Device->Rx, p_Device->Tx);

    // Get all remaining available data to clear the Rx buffer.
    while(p_Device->p_Interface->available())
    {
        p_Device->p_Interface->read();
    }

    p_Device->isInitialized = true;

    return ESP_OK;
}

void RAK3172_Deinit(RAK3172_t* p_Device)
{
    if(!p_Device->isInitialized)
    {
        return;
    }

    pinMode(p_Device->Rx, OUTPUT);
    pinMode(p_Device->Tx, OUTPUT);

    digitalWrite(p_Device->Rx, HIGH);
    digitalWrite(p_Device->Tx, LOW);

	if(p_Device->p_Interface)
	{
		p_Device->p_Interface->flush();
		p_Device->p_Interface->end();
	}
}

esp_err_t RAK3172_SoftReset(RAK3172_t* p_Device, uint32_t Timeout)
{
    uint32_t Now;
    String Response;

    // Reset the module and read back the slash screen because the current state is unclear.
    p_Device->p_Interface->print("ATZ\r\n");

    // Wait for the splashscreen and get the data.
    Now = millis();
    p_Device->p_Interface->getTimeout();
    do
    {
        Response += p_Device->p_Interface->readStringUntil('\n');
        if((millis() - Now) >= (Timeout * 1000ULL))
        {
            ESP_LOGE(TAG, "Module reset timeout!");

            return ESP_ERR_TIMEOUT;
        }

        ESP_LOGD(TAG, "Response: %s", Response.c_str());
    } while((Response.indexOf("LoRaWAN.") == -1) && (Response.indexOf("LoRa P2P.") == -1));

    ESP_LOGD(TAG, "SW reset successful");

    return ESP_OK;
}

String RAK3172_LibVersion(void)
{
    return "2.0.0";
}

esp_err_t RAK3172_SendCommand(RAK3172_t* p_Device, String Command, String* p_Value, String* p_Status)
{
    String Status_Temp;
    esp_err_t Error = ESP_OK;

    if(p_Device == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    if(p_Device->isBusy)
    {
        ESP_LOGE(TAG, "Device busy!");

        return ESP_FAIL;
    }

    // Transmit the command.
    ESP_LOGD(TAG, "Transmit command: %s", Command.c_str());
    p_Device->p_Interface->print(Command + "\r\n");

    // Response expected. Read the value.
    if(p_Value != NULL)
    {
        *p_Value = p_Device->p_Interface->readStringUntil('\n');
        p_Value->replace("\r", "");
        ESP_LOGI(TAG, "    Value: %s", p_Value->c_str());
    }

    // Receive the line feed before the status.
    p_Device->p_Interface->readStringUntil('\n');

    // Receive the trailing status code.
    Status_Temp = p_Device->p_Interface->readStringUntil('\n');
    ESP_LOGD(TAG, "    Status: %s", Status_Temp.c_str());

    // Transmission is without error when 'OK' as status code and when no event data are received.
    if((Status_Temp.indexOf("OK") == -1) && (Status_Temp.indexOf("+EVT") == -1))
    {
        Error = ESP_FAIL;
    }

    // Copy the status string if needed.
    if(p_Status != NULL)
    {
        *p_Status = Status_Temp;
    }
    ESP_LOGD(TAG, "    Error: %i", (int)Error);

    return Error;
}

esp_err_t RAK3172_GetFWVersion(RAK3172_t* p_Device, String* p_Version)
{
    if(p_Version == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    return RAK3172_SendCommand(p_Device, "AT+VER=?", p_Version, NULL);
}

esp_err_t RAK3172_GetSerialNumber(RAK3172_t* p_Device, String* p_Serial)
{
    if(p_Serial == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    return RAK3172_SendCommand(p_Device, "AT+SN=?", p_Serial, NULL);
}

esp_err_t RAK3172_GetRSSI(RAK3172_t* p_Device, int* p_RSSI)
{
    String Value;
    esp_err_t Error;

    if(p_RSSI == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    Error = RAK3172_SendCommand(p_Device, "AT+RSSI=?", &Value, NULL);
    if(Error)
    {
        return Error;
    }

    *p_RSSI = Value.toInt();

    return Error;
}

esp_err_t RAK3172_GetSNR(RAK3172_t* p_Device, int* p_SNR)
{
    String Value;
    esp_err_t Error;

    if(p_SNR == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    Error = RAK3172_SendCommand(p_Device, "AT+SNR=?", &Value, NULL);
    if(Error)
    {
        return Error;
    }

    *p_SNR = Value.toInt();

    return Error;
}

esp_err_t RAK3172_SetMode(RAK3172_t* p_Device, RAK3172_Mode_t Mode)
{
    String Status;

    if(!p_Device->isInitialized)
    {
        return ESP_ERR_INVALID_STATE;
    }

    // Transmit the command.
    p_Device->p_Interface->print("AT+NWM=" + String(Mode) + "\r\n");

    // Receive the line feed before the status.
    p_Device->p_Interface->readStringUntil('\n');

    // Receive the trailing status code.
    Status = p_Device->p_Interface->readStringUntil('\n');

    // 'OK' received, so the mode wasn´t change. Leave the function
    Status.clear();
    if(Status.indexOf("OK") >= 0)
    {
        return ESP_OK;
    }

    // Otherwise read the remaining lines.
    Status.clear();
    do
    {
        Status += p_Device->p_Interface->readStringUntil('\n');
    } while(p_Device->p_Interface->available() > 0);

    return ESP_OK;
}

esp_err_t RAK3172_GetMode(RAK3172_t* p_Device)
{
    String Value;
    esp_err_t Error = ESP_OK;

    if(!p_Device->isInitialized)
    {
        return ESP_ERR_INVALID_STATE;
    }

    Error = RAK3172_SendCommand(p_Device, "AT+NWM=?", &Value, NULL);
    if(Error)
    {
        return Error;
    }

    p_Device->Mode = (RAK3172_Mode_t)Value.toInt();

    return Error;
}

esp_err_t RAK3172_SetBaud(RAK3172_t* p_Device, RAK3172_Baud_t Baud)
{
    if(!p_Device->isInitialized)
    {
        return ESP_ERR_INVALID_STATE;
    }

    if(p_Device->Baudrate == Baud)
    {
        return ESP_OK;
    }

    return RAK3172_SendCommand(p_Device, "AT+BAUD=" + String(Baud), NULL, NULL);
}

esp_err_t RAK3172_GetBaud(RAK3172_t* p_Device, RAK3172_Baud_t* p_Baud)
{
    String Value;
    esp_err_t Error = ESP_OK;

    if(!p_Device->isInitialized)
    {
        return ESP_ERR_INVALID_STATE;
    }

    Error = RAK3172_SendCommand(p_Device, "AT+BAUD=?", &Value, NULL);
    if(Error)
    {
        return Error;
    }

    *p_Baud = (RAK3172_Baud_t)Value.toInt();

    return Error;
}