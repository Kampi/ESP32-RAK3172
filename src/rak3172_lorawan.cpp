 /*
 * rak3172_lorawan.h
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

#include <driver/uart.h>
#include <esp32-hal-log.h>

#include "../include/rak3172.h"

static const char* TAG = "RAK3172_LoRaWAN";

esp_err_t RAK3172_Init_LoRaWAN(RAK3172_t* p_Device, uint8_t TxPwr, uint8_t Retries, RAK3172_JoinMode_t JoinMode, uint8_t* p_Key1, uint8_t* p_Key2, uint8_t* p_Key3, char Class, RAK3172_Band_t Band, RAK3172_SubBand_t Subband, uint32_t Timeout)
{
    esp_err_t Error;
    String Response;

    ESP_LOGI(TAG, "Initialize module in LoRaWAN mode...");

    Error = RAK3172_SetMode(p_Device, RAK_MODE_LORAWAN) || RAK3172_SoftReset(p_Device, Timeout);
    if(Error)
    {
        return Error;
    }

    p_Device->isBusy = false;

    // Check if echo mode is enabled.
    RAK3172_SendCommand(p_Device, "AT", NULL, &Response);
    ESP_LOGD(TAG, "Response from 'AT': %s", Response.c_str());
    if(Response.indexOf("OK") == -1)
    {
        // Echo mode is enabled. Need to receive one more line.
        p_Device->p_Interface->readStringUntil('\n');

        ESP_LOGD(TAG, "Echo mode enabled. Disabling echo mode...");

        // Disable echo mode
        //  -> Transmit the command
        //  -> Receive the echo
        //  -> Receive the value
        //  -> Receive the status
        p_Device->p_Interface->print("ATE\r\n");
        p_Device->p_Interface->readStringUntil('\n');
        p_Device->p_Interface->readStringUntil('\n');
        Response = p_Device->p_Interface->readStringUntil('\n');
        ESP_LOGD(TAG, "Response from 'AT': %s", Response.c_str());

        // Error during initialization when everything else except 'OK' is received.
        if(Response.indexOf("OK") == -1)
        {
            return ESP_FAIL;
        }
    }

    // Stop the joining process.
    Error = RAK3172_SendCommand(p_Device, "AT+JOIN=0:0:10:8", NULL, NULL);
    if(Error)
    {
        return Error;
    }

    // Apply the regional configurations.
    Error = RAK3172_SetBand(p_Device, Band);
    if(Error)
    {
        return Error;
    }

    if(Subband != RAK_SUB_BAND_NONE)
    {
        Error = RAK3172_SetSubBand(p_Device, Subband);
        if(Error)
        {
            return Error;
        }
    }

    Error = RAK3172_SetRetries(p_Device, Retries) ||
            RAK3172_SendCommand(p_Device, "AT+NWM=1", NULL, NULL) ||
            RAK3172_SendCommand(p_Device, "AT+CLASS=" + String(Class), NULL, NULL) ||
            RAK3172_SetTxPwr(p_Device, TxPwr) ||
            RAK3172_SendCommand(p_Device, "AT+ADR=1", NULL, NULL);

    if(Error)
    {
        return Error;
    }

    if(JoinMode == RAK_JOIN_OTAA)
    {
        ESP_LOGD(TAG, "Using OTAA mode");

        Error = RAK3172_SendCommand(p_Device, "AT+NJM=" + String(RAK_JOIN_OTAA), NULL, NULL) || RAK3172_SetOTAAKeys(p_Device, p_Key1, p_Key2, p_Key3);
        if(Error)
        {
            return Error;
        }

        return ESP_OK;
    }
    else
    {
        ESP_LOGD(TAG, "Using ABP mode");

        Error = RAK3172_SendCommand(p_Device, "AT+NJM=" + String(RAK_JOIN_ABP), NULL, NULL) || RAK3172_SetABPKeys(p_Device, p_Key1, p_Key2, p_Key3);
        if(Error)
        {
            return Error;
        }

        return ESP_OK;
    }

    return ESP_ERR_INVALID_ARG;
}

esp_err_t RAK3172_SetOTAAKeys(RAK3172_t* p_Device, const uint8_t* p_DEVEUI, const uint8_t* p_APPEUI, const uint8_t* p_APPKEY)
{
    String AppEUIString;
    String DevEUIString;
    String AppKeyString;

    if((p_Device == NULL) || (p_DEVEUI == NULL) || (p_APPEUI == NULL) || (p_APPKEY == NULL))
    {
        return ESP_ERR_INVALID_ARG;
    }

    // Copy the keys from the buffer into a string.
    char Buffer[3];
    for(uint8_t i = 0; i < 8; i++)
    {
        sprintf(Buffer, "%02X", p_APPEUI[i]);
        AppEUIString += String(Buffer);

        sprintf(Buffer, "%02X", p_DEVEUI[i]);
        DevEUIString += String(Buffer);
    }

    for(uint8_t i = 0; i < 16; i++)
    {
        sprintf(Buffer, "%02X", p_APPKEY[i]);
        AppKeyString += String(Buffer);
    }

    ESP_LOGD(TAG, "DEVEUI: %s - Size: %u", DevEUIString.c_str(), DevEUIString.length());
    ESP_LOGD(TAG, "APPEUI: %s - Size: %u", AppEUIString.c_str(), AppEUIString.length());
    ESP_LOGD(TAG, "APPKEY: %s - Size: %u", AppKeyString.c_str(), AppKeyString.length());

    return RAK3172_SendCommand(p_Device, "AT+DEVEUI=" + DevEUIString, NULL, NULL) ||
           RAK3172_SendCommand(p_Device, "AT+APPEUI=" + AppEUIString, NULL, NULL) ||
           RAK3172_SendCommand(p_Device, "AT+APPKEY=" + AppKeyString, NULL, NULL);
}

esp_err_t RAK3172_SetABPKeys(RAK3172_t* p_Device, const uint8_t* p_APPSKEY, const uint8_t* p_NWKSKEY, const uint8_t* p_DEVADDR)
{
    String AppSKEYString;
    String NwkSKEYString;
    String DevADDRString;

    if((p_Device == NULL) || (p_APPSKEY == NULL) || (p_NWKSKEY == NULL) || (p_DEVADDR == NULL))
    {
        return ESP_ERR_INVALID_ARG;
    }

    // Copy the keys from the buffer into a string.
    char Buffer[3];
    for(uint8_t i = 0; i < 16; i++)
    {
        sprintf(Buffer, "%02X", p_APPSKEY[i]);
        AppSKEYString += String(Buffer);

        sprintf(Buffer, "%02X", p_NWKSKEY[i]);
        NwkSKEYString += String(Buffer);
    }

    for(uint8_t i = 0; i < 4; i++)
    {
        sprintf(Buffer, "%02X", p_DEVADDR[i]);
        DevADDRString += String(Buffer);
    }

    ESP_LOGD(TAG, "APPSKEY: %s - Size: %u", AppSKEYString.c_str(), AppSKEYString.length());
    ESP_LOGD(TAG, "NWKSKEY: %s - Size: %u", NwkSKEYString.c_str(), NwkSKEYString.length());
    ESP_LOGD(TAG, "DEVADDR: %s - Size: %u", DevADDRString.c_str(), DevADDRString.length());

    return RAK3172_SendCommand(p_Device, "AT+APPSKEY=" + AppSKEYString, NULL, NULL) ||
           RAK3172_SendCommand(p_Device, "AT+NWKSKEY=" + NwkSKEYString, NULL, NULL) ||
           RAK3172_SendCommand(p_Device, "AT+DEVADDR=" + DevADDRString, NULL, NULL);
}

esp_err_t RAK3172_StartJoin(RAK3172_t* p_Device, uint32_t Timeout, uint8_t Attempts, bool EnableAutoJoin, uint8_t Interval, RAK3172_Wait_t on_Wait)
{
    String Line;
    uint8_t Attempts_Temp;
    uint32_t TimeNow;
    esp_err_t Error;

    if(Attempts == 0)
    {
        return ESP_ERR_INVALID_ARG;
    }

    Attempts_Temp = Attempts;

    // Start the joining procedure.
    Error = RAK3172_SendCommand(p_Device, "AT+JOIN=1:" + String(EnableAutoJoin) + ":" + String(Interval) + ":" + String(Attempts), NULL, NULL);
    if(Error)
    {
        return Error;
    }

    // Wait for a successfull join.
    TimeNow = millis();
    do
    {
        Line = p_Device->p_Interface->readStringUntil('\n');
        ESP_LOGD(TAG, "Join event: %s", Line.c_str());

        if(on_Wait)
        {
            on_Wait();
        }

        // Join was successful.
        if(Line.indexOf("JOINED") >= 0)
        {
            return ESP_OK;
        }
        // Join failed. Reduce the counter by one and return a timeout when zero.
        else if(Line.indexOf("JOIN FAILED") >= 0)
        {
            Attempts_Temp--;
            ESP_LOGD(TAG, "    Attempts left: %i/%i", Attempts_Temp, Attempts);

            if(Attempts_Temp == 0)
            {
                RAK3172_SendCommand(p_Device, "AT+JOIN=0:0:7:0", NULL, NULL);

                return ESP_ERR_TIMEOUT;
            }
        }

        if((Timeout > 0) && ((millis() - TimeNow) >= (Timeout * 1000ULL)))
        {
            ESP_LOGE(TAG, "Join timeout!");

            RAK3172_SendCommand(p_Device, "AT+JOIN=0:0:7:0", NULL, NULL);

            return ESP_ERR_TIMEOUT;
        }

        Line.clear();

        delay(100);
    } while(true);
}

esp_err_t RAK3172_StopJoin(RAK3172_t* p_Device)
{
    return RAK3172_SendCommand(p_Device, "AT+JOIN=0:0:7:0", NULL, NULL);
}

esp_err_t RAK3172_Joined(RAK3172_t* p_Device, bool* p_Joined)
{
    String Value;
    esp_err_t Error;

    if(p_Joined == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    *p_Joined = false;

    Error = RAK3172_SendCommand(p_Device, "AT+NJS=?", &Value, NULL);
    if(Error)
    {
        return Error;
    }

    if(Value.compareTo("1") == 0)
    {
        *p_Joined = true;
    }

    return ESP_OK;
}

esp_err_t RAK3172_LoRaWAN_Transmit(RAK3172_t* p_Device, uint8_t Port, const uint8_t* p_Buffer, uint8_t Length)
{
    return RAK3172_LoRaWAN_Transmit(p_Device, Port, p_Buffer, Length, 0);
}

esp_err_t RAK3172_LoRaWAN_Transmit(RAK3172_t* p_Device, uint8_t Port, const void* p_Buffer, uint8_t Length, uint32_t Timeout, bool Confirmed, RAK3172_Wait_t Wait)
{
    String Line;
    String Payload;
    String Status;
    uint32_t Now;
    esp_err_t Error;
    char Buffer[3];
    bool Joined;

    if(((p_Buffer == NULL) && (Length == 0)) || (Port == 0))
    {
        return ESP_ERR_INVALID_ARG;
    }
    else if(Length == 0)
    {
        return ESP_OK;
    }

    Error = RAK3172_Joined(p_Device, &Joined);
    if(Error)
    {
        return Error;
    }

    if(!Joined)
    {
        return ESP_ERR_INVALID_STATE;
    }

    // Encode the payload into an ASCII string.
    for(uint8_t i = 0x00; i < Length; i++)
    {
        sprintf(Buffer, "%02x", ((uint8_t*)p_Buffer)[i]);
        Payload += String(Buffer);
    }

    Error = RAK3172_SetConfirmation(p_Device, Confirmed);
    if(Error)
    {
        return Error;
    }

    // TODO: Use long data send when the packet size is greater than 242 bytes.
    Error = RAK3172_SendCommand(p_Device, "AT+SEND=" + String(Port) + ":" + Payload, NULL, &Status);
    if(Error)
    {
        return Error;
    }

    // The device is busy. Leave the function with an invalid state error.
    if(Status.indexOf("AT_BUSY_ERROR") >= 0)
    {
        return ESP_ERR_INVALID_RESPONSE;
    }

    // Wait for the confirmation if needed.
    if(Confirmed)
    {
        Now = millis();
        do
        {
            if(Wait)
            {
                Wait();
            }

            Line = p_Device->p_Interface->readStringUntil('\n');

            if(Line.length() > 0)
            {
                ESP_LOGD(TAG, "Transmission event: %s", Line.c_str());

                // Transmission failed.
                if(Line.indexOf("SEND CONFIRMED FAILED") != -1)
                {
                    return ESP_ERR_INVALID_RESPONSE;
                }
                // Transmission was successful.
                else if(Line.indexOf("SEND CONFIRMED OK") != -1)
                {
                    return ESP_OK;
                }
            }
            else
            {
                ESP_LOGD(TAG, "Wait for Tx...");
            }

            if((Timeout > 0) && ((millis() - Now) >= (Timeout * 1000UL)))
            {
                ESP_LOGE(TAG, "Transmit timeout!");

                return ESP_ERR_TIMEOUT;
            }

            Line.clear();

            delay(100);
        } while(true);
    }

    return ESP_OK;
}

esp_err_t RAK3172_LoRaWAN_Receive(RAK3172_t* p_Device, String* p_Payload, int* p_RSSI, int* p_SNR, uint32_t Timeout)
{
    String Line;
    uint32_t Now;

    if((p_Payload == NULL) || (Timeout <= 1))
    {
        return ESP_ERR_INVALID_ARG;
    }

    Now = millis();
    do
    {
        Line = p_Device->p_Interface->readStringUntil('\n');

        if(Line.length() > 0)
        {
            int Index;

            ESP_LOGD(TAG, "Receive event: %s", Line.c_str());

            // Get the RX metadata first.
            if(Line.indexOf("RX") != -1)
            {
                String Dummy;

                // Get the RSSI value.
                if(p_RSSI != NULL)
                {
                    Index = Line.indexOf(",");
                    Dummy = Line.substring(Index + 7, Index + Line.indexOf(",", Index) + 1);
                    *p_RSSI = Dummy.toInt();
                }

                // Get the SNR value.
                if(p_SNR != NULL)
                {
                    Index = Line.lastIndexOf(",");
                    Dummy = Line.substring(Index + 6, Line.length() - 1);
                    *p_SNR = Dummy.toInt();
                }
            }

            // Then get the data and leave the function.
            if(Line.indexOf("UNICAST") != -1)
            {
                Line = p_Device->p_Interface->readStringUntil('\n');
                ESP_LOGD(TAG, "    Payload: %s", Line.c_str());

                // Clean up the payload string ("+EVT:Port:Payload")
                //  - Remove the "+EVT" indicator
                //  - Remove the port number
                *p_Payload = Line.substring(Line.lastIndexOf(":") + 1, Line.length());

                return ESP_OK;
            }
        }

        if((Timeout > 0) && (((millis() - Now) / 1000ULL) >= Timeout))
        {
            ESP_LOGE(TAG, "Receive timeout!");

            return ESP_ERR_TIMEOUT;
        }

        Line.clear();

        delay(100);
    } while(true);
}

esp_err_t RAK3172_SetRetries(RAK3172_t* p_Device, uint8_t Retries)
{
    bool Enable = false;

    if(Retries > 7)
    {
        return ESP_ERR_INVALID_ARG;
    }

    if(Retries > 0)
    {
        Enable = true;
    }

    return RAK3172_SendCommand(p_Device, "AT+CFM=" + String(Enable), NULL, NULL) || RAK3172_SendCommand(p_Device, "AT+RETY=" + String(Retries), NULL, NULL);
}

esp_err_t RAK3172_GetRetries(RAK3172_t* p_Device, uint8_t* p_Retries)
{
    String Value;
    esp_err_t Error;

    if(p_Retries == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    Error = RAK3172_SendCommand(p_Device, "AT+RETY=?", &Value, NULL);
    if(Error)
    {
        return Error;
    }

    *p_Retries = (uint8_t)Value.toInt();

    return Error;
}

esp_err_t RAK3172_SetPNM(RAK3172_t* p_Device, bool Enable)
{
    return RAK3172_SendCommand(p_Device, "AT+PNM=" + String(Enable), NULL, NULL);
}

esp_err_t RAK3172_GetPNM(RAK3172_t* p_Device, bool* p_Enable)
{
    String Value;
    esp_err_t Error;

    if(p_Enable == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    Error = RAK3172_SendCommand(p_Device, "AT+PNM=?", &Value, NULL);
    if(Error)
    {
        return Error;
    }

    *p_Enable = (bool)Value.toInt();

    return Error;
}

esp_err_t RAK3172_SetConfirmation(RAK3172_t* p_Device, bool Enable)
{
    return RAK3172_SendCommand(p_Device, "AT+CFM=" + String(Enable), NULL, NULL);
}

esp_err_t RAK3172_GetConfirmation(RAK3172_t* p_Device, bool* p_Enable)
{
    String Value;
    esp_err_t Error;

    if(p_Enable == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    Error = RAK3172_SendCommand(p_Device, "AT+CFM=?", &Value, NULL);
    if(Error)
    {
        return Error;
    }

    *p_Enable = (bool)Value.toInt();

    return Error;
}

esp_err_t RAK3172_SetBand(RAK3172_t* p_Device, RAK3172_Band_t Band)
{
    return RAK3172_SendCommand(p_Device, "AT+BAND=" + String((uint8_t)Band), NULL, NULL);
}

esp_err_t RAK3172_GetBand(RAK3172_t* p_Device, RAK3172_Band_t* p_Band)
{
    String Value;
    esp_err_t Error;

    if(p_Band == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    Error = RAK3172_SendCommand(p_Device, "AT+BAND=?", &Value, NULL);
    if(Error)
    {
        return Error;
    }

    *p_Band = (RAK3172_Band_t)Value.toInt();

    return Error;
}

esp_err_t RAK3172_SetSubBand(RAK3172_t* p_Device, RAK3172_SubBand_t Band)
{
    esp_err_t Error;
    RAK3172_Band_t Dummy;

    if(Band == RAK_SUB_BAND_NONE)
    {
        return ESP_OK;
    }

    Error = RAK3172_GetBand(p_Device, &Dummy);
    if(Error)
    {
        return Error;
    }

    // The sub band can only be changed when using US915, AU915 or CN470 frequency band.
    if((Dummy == RAK_BAND_US915) || (Dummy == RAK_BAND_AU915) || (Dummy == RAK_BAND_CN470))
    {
        if((Band > RAK_SUB_BAND_9) && (Dummy != RAK_BAND_CN470))
        {
            return ESP_ERR_INVALID_ARG;
        }

        if(Band == RAK_SUB_BAND_ALL)
        {
            return RAK3172_SendCommand(p_Device, "AT+MASK=0000", NULL, NULL);
        }
        else
        {
            char Temp[5];
            uint32_t Mask = 1;

            Mask = Mask << (Band - 2);

            sprintf(Temp, "%04X", Mask);
    
            return RAK3172_SendCommand(p_Device, "AT+MASK=" + String(Temp), NULL, NULL);
        }
    }

    return ESP_FAIL;
}

esp_err_t RAK3172_GetSubBand(RAK3172_t* p_Device, RAK3172_SubBand_t* p_Band)
{
    esp_err_t Error;
    RAK3172_Band_t Dummy;
    String Value;
    uint32_t Mask;
    uint8_t Shifts = 1;

    if(p_Band == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    Error = RAK3172_GetBand(p_Device, &Dummy);
    if(Error)
    {
        return Error;
    }

    if((Dummy != RAK_BAND_US915) && (Dummy != RAK_BAND_AU915) && (Dummy != RAK_BAND_CN470))
    {
        *p_Band = RAK_SUB_BAND_NONE;

        return ESP_OK;
    }

    Error = RAK3172_SendCommand(p_Device, "AT+MASK=?", &Value, NULL);
    if(Error)
    {
        return Error;
    }

    Mask = Value.toInt();

    if(Mask == 0)
    {
        *p_Band = RAK_SUB_BAND_ALL;

        return ESP_OK;
    }
    else
    {
        while(Mask != 1)
        {
            Mask >>= 1;
            Shifts++;
        }

        *p_Band = (RAK3172_SubBand_t)(Shifts + 2);

        return ESP_OK;
    }
}

esp_err_t RAK3172_SetTxPwr(RAK3172_t* p_Device, uint8_t TxPwr)
{
    esp_err_t Error;
    uint8_t TxPwrIndex = 0;
    RAK3172_Band_t Band;

    Error = RAK3172_GetBand(p_Device, &Band);
    if(Error)
    {
        return Error;
    }

    ESP_LOGD(TAG, "Set Tx power to: %u dBm", TxPwr);

    // For EU868 the maximum transmit power is +16 dB EIRP.
    if(Band == RAK_BAND_EU868)
    {
        const uint8_t EIRP = 16;

        if(TxPwr >= EIRP)
        {
            TxPwrIndex = 0;
        }
        else if(TxPwr < (EIRP - 14))
        {
            TxPwrIndex = 10;
        }
        else
        {
            TxPwrIndex = (uint8_t)((EIRP - TxPwr) / 2);
        }
    }
    // For US915 the maximum transmit power is +30 dBm conducted power.
    else if(Band == RAK_BAND_US915)
    {
        const uint8_t MaxPwr = 30;

        if(TxPwr >= MaxPwr)
        {
            TxPwrIndex = 0;
        }
        else if(TxPwr < 10)
        {
            TxPwrIndex = 10;
        }
        else
        {
            TxPwrIndex = (uint8_t)((MaxPwr - TxPwr) / 2);
        }
    }
    else
    {
        ESP_LOGE(TAG, "Tx power is not implemented for the selected frequency band!");
    }

    ESP_LOGD(TAG, "Set Tx power index: %u", TxPwrIndex);

    return RAK3172_SendCommand(p_Device, "AT+TXP=" + String(TxPwrIndex), NULL, NULL);
}

esp_err_t RAK3172_SetRX1Delay(RAK3172_t* p_Device, uint16_t Delay)
{
    return RAK3172_SendCommand(p_Device, "AT+RX1DL=" + String(Delay), NULL, NULL);
}

esp_err_t RAK3172_SetRX2Delay(RAK3172_t* p_Device, uint16_t Delay)
{
    return RAK3172_SendCommand(p_Device, "AT+RX2DL=" + String(Delay), NULL, NULL);
}

esp_err_t RAK3172_GetSNR(RAK3172_t* p_Device, uint8_t* p_SNR)
{
    String Value;
    esp_err_t Error;

    Error = RAK3172_SendCommand(p_Device, "AT+SNR=?", &Value, NULL);
    if(Error)
    {
        return Error;
    }

    *p_SNR = (uint8_t)Value.toInt();

    return Error;
}

esp_err_t RAK3172_GetRSSI(RAK3172_t* p_Device, int8_t* p_RSSI)
{
    String Value;
    esp_err_t Error;

    Error = RAK3172_SendCommand(p_Device, "AT+RSSI=?", &Value, NULL);
    if(Error)
    {
        return Error;
    }

    *p_RSSI = (int8_t)Value.toInt();

    return Error;
}

esp_err_t RAK3172_GetDuty(RAK3172_t* p_Device, uint8_t* p_Duty)
{
    String Value;
    esp_err_t Error;
    RAK3172_Band_t Band;

    Error = RAK3172_GetBand(p_Device, &Band);
    if(Error)
    {
        return Error;
    }

    if((Band != RAK_BAND_EU868) && (Band != RAK_BAND_RU864) && (Band != RAK_BAND_EU433))
    {
        return ESP_ERR_INVALID_RESPONSE;
    }

    Error = RAK3172_SendCommand(p_Device, "AT+DUTYTIME=?", &Value, NULL);
    if(Error)
    {
        return Error;
    }

    *p_Duty = (uint8_t)Value.toInt();

    return Error;
}
