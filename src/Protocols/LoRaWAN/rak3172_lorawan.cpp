 /*
 * rak3172_lorawan.cpp
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

#ifdef CONFIG_RAK3172_WITH_LORAWAN

#include <esp_log.h>
#include <esp_sleep.h>

#include "rak3172.h"

static const char* TAG = "RAK3172_LoRaWAN";

RAK3172_Error_t RAK3172_LoRaWAN_Init(RAK3172_t* const p_Device, uint8_t TxPwr, uint8_t Retries, RAK3172_JoinMode_t JoinMode, const uint8_t* const p_Key1, const uint8_t* const p_Key2, const uint8_t* const p_Key3, char Class, RAK3172_Band_t Band, RAK3172_SubBand_t Subband, bool UseADR, uint32_t Timeout)
{
    std::string Command;

    if((p_Device == NULL) || ((Class != 'A') && (Class != 'B') && (Class != 'C')) || (p_Key1 == NULL) || (p_Key2 == NULL) || (p_Key3 == NULL))
    {
        return RAK3172_ERR_INVALID_ARG;
    }

    ESP_LOGI(TAG, "Initialize module in LoRaWAN mode...");
    RAK3172_ERROR_CHECK(RAK3172_SetMode(p_Device, RAK_MODE_LORAWAN));

    // Stop an ongoing joining process.
    RAK3172_ERROR_CHECK(RAK3172_LoRaWAN_StopJoin(p_Device));
    RAK3172_ERROR_CHECK(RAK3172_LoRaWAN_isJoined(p_Device, &p_Device->LoRaWAN.isJoined));

    p_Device->Internal.isBusy = false;

    Command = "AT+CLASS=";
    Command += Class;
    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, Command));

	RAK3172_ERROR_CHECK(RAK3172_LoRaWAN_SetADR(p_Device, UseADR));
    RAK3172_ERROR_CHECK(RAK3172_LoRaWAN_SetBand(p_Device, Band));

    if(Subband != RAK_SUB_BAND_NONE)
    {
        RAK3172_ERROR_CHECK(RAK3172_LoRaWAN_SetSubBand(p_Device, Subband))
    }

    RAK3172_ERROR_CHECK(RAK3172_LoRaWAN_SetRetries(p_Device, Retries));
    RAK3172_ERROR_CHECK(RAK3172_LoRaWAN_SetTxPwr(p_Device, TxPwr));
    RAK3172_ERROR_CHECK(RAK3172_LoRaWAN_SetJoinMode(p_Device, JoinMode));

    p_Device->LoRaWAN.Join = JoinMode;
    if(p_Device->LoRaWAN.Join == RAK_JOIN_OTAA)
    {
        ESP_LOGI(TAG, "Using OTAA mode");

        return RAK3172_LoRaWAN_SetOTAAKeys(p_Device, p_Key1, p_Key2, p_Key3);
    }
    else
    {
        ESP_LOGI(TAG, "Using ABP mode");

        return RAK3172_LoRaWAN_SetABPKeys(p_Device, p_Key1, p_Key2, p_Key3);
    }

    return RAK3172_ERR_INVALID_ARG;
}

RAK3172_Error_t RAK3172_LoRaWAN_SetOTAAKeys(const RAK3172_t* const p_Device, const uint8_t* const p_DEVEUI, const uint8_t* const p_APPEUI, const uint8_t* const p_APPKEY)
{
    std::string AppEUIString;
    std::string DevEUIString;
    std::string AppKeyString;

    if((p_Device == NULL) || (p_DEVEUI == NULL) || (p_APPEUI == NULL) || (p_APPKEY == NULL))
    {
        return RAK3172_ERR_INVALID_ARG;
    }
    else if(p_Device->LoRaWAN.Join != RAK_JOIN_OTAA)
    {
        return RAK3172_ERR_INVALID_STATE;
    }

    // Copy the keys from the buffer into a string.
    char Buffer[3];
    for(uint8_t i = 0; i < 8; i++)
    {
        sprintf(Buffer, "%02X", p_APPEUI[i]);
        AppEUIString += std::string(Buffer);

        sprintf(Buffer, "%02X", p_DEVEUI[i]);
        DevEUIString += std::string(Buffer);
    }

    for(uint8_t i = 0; i < 16; i++)
    {
        sprintf(Buffer, "%02X", p_APPKEY[i]);
        AppKeyString += std::string(Buffer);
    }

    ESP_LOGD(TAG, "DEVEUI: %s - Size: %u", DevEUIString.c_str(), DevEUIString.length());
    ESP_LOGD(TAG, "APPEUI: %s - Size: %u", AppEUIString.c_str(), AppEUIString.length());
    ESP_LOGD(TAG, "APPKEY: %s - Size: %u", AppKeyString.c_str(), AppKeyString.length());

    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+DEVEUI=" + DevEUIString));
    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+APPEUI=" + AppEUIString));
    return RAK3172_SendCommand(p_Device, "AT+APPKEY=" + AppKeyString);
}

RAK3172_Error_t RAK3172_LoRaWAN_SetABPKeys(const RAK3172_t* const p_Device, const uint8_t* const p_APPSKEY, const uint8_t* const p_NWKSKEY, const uint8_t* const p_DEVADDR)
{
    std::string AppSKEYString;
    std::string NwkSKEYString;
    std::string DevADDRString;

    if((p_Device == NULL) || (p_APPSKEY == NULL) || (p_NWKSKEY == NULL) || (p_DEVADDR == NULL))
    {
        return RAK3172_ERR_INVALID_ARG;
    }
    else if(p_Device->LoRaWAN.Join != RAK_JOIN_ABP)
    {
        return RAK3172_ERR_INVALID_STATE;
    }

    // Copy the keys from the buffer into a string.
    char Buffer[3];
    for(uint8_t i = 0; i < 16; i++)
    {
        sprintf(Buffer, "%02X", p_APPSKEY[i]);
        AppSKEYString += std::string(Buffer);

        sprintf(Buffer, "%02X", p_NWKSKEY[i]);
        NwkSKEYString += std::string(Buffer);
    }

    for(uint8_t i = 0; i < 4; i++)
    {
        sprintf(Buffer, "%02X", p_DEVADDR[i]);
        DevADDRString += std::string(Buffer);
    }

    ESP_LOGD(TAG, "APPSKEY: %s - Size: %u", AppSKEYString.c_str(), AppSKEYString.length());
    ESP_LOGD(TAG, "NWKSKEY: %s - Size: %u", NwkSKEYString.c_str(), NwkSKEYString.length());
    ESP_LOGD(TAG, "DEVADDR: %s - Size: %u", DevADDRString.c_str(), DevADDRString.length());

    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+APPSKEY=" + AppSKEYString));
    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+NWKSKEY=" + NwkSKEYString));
    return RAK3172_SendCommand(p_Device, "AT+DEVADDR=" + DevADDRString);
}

RAK3172_Error_t RAK3172_LoRaWAN_StartJoin(RAK3172_t* const p_Device, uint32_t Timeout, uint8_t Attempts, bool EnableAutoJoin, uint8_t Interval, RAK3172_Wait_t on_Wait)
{
    uint32_t TimeNow;

    if((p_Device == NULL) || (Attempts == 0))
    {
        return RAK3172_ERR_INVALID_ARG;
    }
    else if(p_Device->Internal.isBusy)
    {
        return RAK3172_ERR_BUSY;
    }
    else if(p_Device->LoRaWAN.isJoined)
    {
        return RAK3172_ERR_OK;
    }

    // Start the joining procedure.
    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+JOIN=1:" + std::to_string(EnableAutoJoin) + ":" + std::to_string(Interval) + ":" + std::to_string(Attempts)));

    p_Device->Internal.isBusy = true;

    // Wait for a successfull join.
    TimeNow = esp_timer_get_time() / 1000ULL;
    do
    {
        if((Timeout > 0) && (((esp_timer_get_time() / 1000ULL) - TimeNow) >= (Timeout * 1000ULL)))
        {
            ESP_LOGE(TAG, "Join timeout!");

            RAK3172_LoRaWAN_StopJoin(p_Device);

            p_Device->Internal.isBusy = false;

            return RAK3172_ERR_TIMEOUT;
        }

        vTaskDelay(20 / portTICK_PERIOD_MS);
    } while(p_Device->LoRaWAN.isJoined == false);

    p_Device->Internal.isBusy = false;

    return RAK3172_ERR_OK;
}

RAK3172_Error_t RAK3172_LoRaWAN_StopJoin(const RAK3172_t* const p_Device)
{
    return RAK3172_SendCommand(p_Device, "AT+JOIN=0:0:7:0");
}

RAK3172_Error_t RAK3172_LoRaWAN_isJoined(RAK3172_t* const p_Device, bool* const p_Joined)
{
    std::string Value;

    if((p_Device == NULL) || (p_Joined == NULL))
    {
        return RAK3172_ERR_INVALID_ARG;
    }

    p_Device->LoRaWAN.isJoined = false;

    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+NJS=?", &Value));

    if(Value.compare("1") == 0)
    {
        p_Device->LoRaWAN.isJoined = true;
    }

    *p_Joined = p_Device->LoRaWAN.isJoined;

    return RAK3172_ERR_OK;
}

RAK3172_Error_t RAK3172_LoRaWAN_Transmit(RAK3172_t* const p_Device, uint8_t Port, const uint8_t* const p_Buffer, uint8_t Length)
{
    return RAK3172_LoRaWAN_Transmit(p_Device, Port, p_Buffer, Length);
}

RAK3172_Error_t RAK3172_LoRaWAN_Transmit(RAK3172_t* const p_Device, uint8_t Port, const void* const p_Buffer, uint8_t Length, bool Confirmed, RAK3172_Wait_t Wait)
{
    std::string Payload;
    std::string Status;
    char Buffer[3];

    if((p_Device == NULL) || ((p_Buffer == NULL) && (Length == 0)) || (Port == 0))
    {
        return RAK3172_ERR_INVALID_ARG;
    }
    else if(p_Device->Internal.isBusy)
    {
        return RAK3172_ERR_BUSY;
    }
    else if(p_Device->LoRaWAN.isJoined == false)
    {
        return RAK3172_ERR_NOT_CONNECTED;
    }
    else if(Length == 0)
    {
        return RAK3172_ERR_OK;
    }

    // Encode the payload into an ASCII string.
    for(uint8_t i = 0x00; i < Length; i++)
    {
        sprintf(Buffer, "%02x", ((uint8_t*)p_Buffer)[i]);
        Payload += std::string(Buffer);
    }

    RAK3172_ERROR_CHECK(RAK3172_LoRaWAN_SetConfirmation(p_Device, Confirmed));

    // TODO: Use long data send when the packet size is greater than 242 bytes.
    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+SEND=" + std::to_string(Port) + ":" + Payload, NULL, &Status));

    // The device is busy. Leave the function with an invalid state error.
    if(Status.find("AT_BUSY_ERROR") != std::string::npos)
    {
        return RAK3172_ERR_BUSY;
    }

    p_Device->Internal.isBusy = true;

    // Wait for the confirmation if needed.
    if(Confirmed)
    {
        do
        {
            if(Wait)
            {
                Wait();
            }

            vTaskDelay(20 / portTICK_PERIOD_MS);
        } while(p_Device->Internal.isBusy);
    }

    p_Device->Internal.isBusy = false;

    if(Confirmed && p_Device->LoRaWAN.ConfirmError)
    {
        return RAK3172_ERR_INVALID_RESPONSE;
    }

    return RAK3172_ERR_OK;
}

RAK3172_Error_t RAK3172_LoRaWAN_Receive(RAK3172_t* const p_Device, RAK3172_Rx_t* p_Message, uint32_t Timeout)
{
    RAK3172_Rx_t* FromQueue = NULL;

    if((p_Device == NULL) || (p_Message == NULL))
    {
        return RAK3172_ERR_INVALID_ARG;
    }
    else if(p_Device->LoRaWAN.isJoined == false)
    {
        return RAK3172_ERR_NOT_CONNECTED;
    }

    if(xQueueReceive(p_Device->Internal.ReceiveQueue, &FromQueue, (Timeout * 1000UL) / portTICK_PERIOD_MS) != pdPASS)
    {
        return RAK3172_ERR_TIMEOUT;
    }

    p_Message->RSSI = FromQueue->RSSI;
    p_Message->SNR = FromQueue->SNR;
    p_Message->Port = FromQueue->Port;
    p_Message->Payload = FromQueue->Payload;

    delete FromQueue;

    return RAK3172_ERR_OK;
}

RAK3172_Error_t RAK3172_LoRaWAN_SetRetries(const RAK3172_t* const p_Device, uint8_t Retries)
{
    bool Enable = false;

    if(Retries > 7)
    {
        return RAK3172_ERR_INVALID_ARG;
    }

    if(Retries > 0)
    {
        Enable = true;
    }

    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+CFM=" + std::to_string(Enable)));

    return RAK3172_SendCommand(p_Device, "AT+RETY=" + std::to_string(Retries));
}

RAK3172_Error_t RAK3172_LoRaWAN_GetRetries(const RAK3172_t* const p_Device, uint8_t* const p_Retries)
{
    std::string Value;

    if(p_Retries == NULL)
    {
        return RAK3172_ERR_INVALID_ARG;
    }

    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+RETY=?", &Value));

    *p_Retries = (uint8_t)std::stoi(Value);

    return RAK3172_ERR_OK;
}

RAK3172_Error_t RAK3172_LoRaWAN_SetPNM(const RAK3172_t* const p_Device, bool Enable)
{
    return RAK3172_SendCommand(p_Device, "AT+PNM=" + std::to_string(Enable));
}

RAK3172_Error_t RAK3172_LoRaWAN_GetPNM(const RAK3172_t* const p_Device, bool* const p_Enable)
{
    std::string Value;

    if(p_Enable == NULL)
    {
        return RAK3172_ERR_INVALID_ARG;
    }

    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+PNM=?", &Value));

    *p_Enable = (bool)std::stoi(Value);

    return RAK3172_ERR_OK;
}

RAK3172_Error_t RAK3172_LoRaWAN_SetConfirmation(const RAK3172_t* const p_Device, bool Enable)
{
    return RAK3172_SendCommand(p_Device, "AT+CFM=" + std::to_string(Enable));
}

RAK3172_Error_t RAK3172_LoRaWAN_GetConfirmation(const RAK3172_t* const p_Device, bool* const p_Enable)
{
    std::string Value;

    if(p_Enable == NULL)
    {
        return RAK3172_ERR_INVALID_ARG;
    }

    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+CFM=?", &Value));

    *p_Enable = (bool)std::stoi(Value);

    return RAK3172_ERR_OK;
}

RAK3172_Error_t RAK3172_LoRaWAN_SetBand(const RAK3172_t* const p_Device, RAK3172_Band_t Band)
{
    return RAK3172_SendCommand(p_Device, "AT+BAND=" + std::to_string((uint8_t)Band));
}

RAK3172_Error_t RAK3172_LoRaWAN_GetBand(const RAK3172_t* const p_Device, RAK3172_Band_t* const p_Band)
{
    std::string Value;

    if(p_Band == NULL)
    {
        return RAK3172_ERR_INVALID_ARG;
    }

    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+BAND=?", &Value));

    *p_Band = (RAK3172_Band_t)std::stoi(Value);

    return RAK3172_ERR_OK;
}

RAK3172_Error_t RAK3172_LoRaWAN_SetSubBand(const RAK3172_t* const p_Device, RAK3172_SubBand_t Band)
{
    RAK3172_Band_t Dummy;

    if(Band == RAK_SUB_BAND_NONE)
    {
        return RAK3172_ERR_OK;
    }

    RAK3172_ERROR_CHECK(RAK3172_LoRaWAN_GetBand(p_Device, &Dummy));

    // The sub band can only be changed when using US915, AU915 or CN470 frequency band.
    if((Dummy == RAK_BAND_US915) || (Dummy == RAK_BAND_AU915) || (Dummy == RAK_BAND_CN470))
    {
        if((Band > RAK_SUB_BAND_9) && (Dummy != RAK_BAND_CN470))
        {
            return RAK3172_ERR_INVALID_ARG;
        }

        if(Band == RAK_SUB_BAND_ALL)
        {
            return RAK3172_SendCommand(p_Device, "AT+MASK=0000");
        }
        else
        {
            char Temp[5];
            uint32_t Mask = 1;

            Mask = Mask << (Band - 2);

            sprintf(Temp, "%04X", Mask);
    
            return RAK3172_SendCommand(p_Device, "AT+MASK=" + std::string(Temp));
        }
    }

    return RAK3172_ERR_FAIL;
}

RAK3172_Error_t RAK3172_LoRaWAN_GetSubBand(const RAK3172_t* const p_Device, RAK3172_SubBand_t* const p_Band)
{
    RAK3172_Band_t Dummy;
    std::string Value;
    uint32_t Mask;
    uint8_t Shifts = 1;

    if(p_Band == NULL)
    {
        return RAK3172_ERR_INVALID_ARG;
    }

    RAK3172_ERROR_CHECK(RAK3172_LoRaWAN_GetBand(p_Device, &Dummy));

    if((Dummy != RAK_BAND_US915) && (Dummy != RAK_BAND_AU915) && (Dummy != RAK_BAND_CN470))
    {
        *p_Band = RAK_SUB_BAND_NONE;

        return RAK3172_ERR_OK;
    }

    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+MASK=?", &Value));

    Mask = std::stoi(Value);

    if(Mask == 0)
    {
        *p_Band = RAK_SUB_BAND_ALL;

        return RAK3172_ERR_OK;
    }
    else
    {
        while(Mask != 1)
        {
            Mask >>= 1;
            Shifts++;
        }

        *p_Band = (RAK3172_SubBand_t)(Shifts + 2);

        return RAK3172_ERR_OK;
    }
}

RAK3172_Error_t RAK3172_LoRaWAN_SetTxPwr(const RAK3172_t* const p_Device, uint8_t TxPwr)
{
    uint8_t TxPwrIndex = 0;
    RAK3172_Band_t Band;

    RAK3172_ERROR_CHECK(RAK3172_LoRaWAN_GetBand(p_Device, &Band));

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

    return RAK3172_SendCommand(p_Device, "AT+TXP=" + std::to_string(TxPwrIndex));
}

RAK3172_Error_t RAK3172_LoRaWAN_SetJoin1Delay(const RAK3172_t* const p_Device, uint8_t Delay)
{
    if((Delay < 1) || (Delay > 14))
    {
        return RAK3172_ERR_INVALID_ARG;
    }

    return RAK3172_SendCommand(p_Device, "AT+JN1DL=" + std::to_string(Delay));
}

RAK3172_Error_t RAK3172_LoRaWAN_GetJoin1Delay(const RAK3172_t* const p_Device, uint8_t* const p_Delay)
{
    std::string Value;

    if(p_Delay == NULL)
    {
        return RAK3172_ERR_INVALID_ARG;
    }

    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+JN1DL=?", &Value));

    *p_Delay = (uint8_t)std::stoi(Value);

    return RAK3172_ERR_OK;
}

RAK3172_Error_t RAK3172_LoRaWAN_SetJoin2Delay(const RAK3172_t* const p_Device, uint8_t Delay)
{
    if((Delay < 2) || (Delay > 15))
    {
        return RAK3172_ERR_INVALID_ARG;
    }

    return RAK3172_SendCommand(p_Device, "AT+JN2DL=" + std::to_string(Delay));
}

RAK3172_Error_t RAK3172_LoRaWAN_GetJoin2Delay(const RAK3172_t* const p_Device, uint8_t* const p_Delay)
{
    std::string Value;

    if(p_Delay == NULL)
    {
        return RAK3172_ERR_INVALID_ARG;
    }

    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+JN2DL=?", &Value));

    *p_Delay = (uint8_t)std::stoi(Value);

    return RAK3172_ERR_OK;
}

RAK3172_Error_t RAK3172_LoRaWAN_SetRX1Delay(const RAK3172_t* const p_Device, uint8_t Delay)
{
    if((Delay < 1) || (Delay > 15))
    {
        return RAK3172_ERR_INVALID_ARG;
    }

    return RAK3172_SendCommand(p_Device, "AT+RX1DL=" + std::to_string(Delay));
}

RAK3172_Error_t RAK3172_LoRaWAN_GetRX1Delay(const RAK3172_t* const p_Device, uint8_t* const p_Delay)
{
    std::string Value;

    if(p_Delay == NULL)
    {
        return RAK3172_ERR_INVALID_ARG;
    }

    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+RX1DL=?", &Value));

    *p_Delay = (uint8_t)std::stoi(Value);

    return RAK3172_ERR_OK;
}

RAK3172_Error_t RAK3172_LoRaWAN_SetRX2Delay(const RAK3172_t* const p_Device, uint8_t Delay)
{
    if((Delay < 2) || (Delay > 16))
    {
        return RAK3172_ERR_INVALID_ARG;
    }

    return RAK3172_SendCommand(p_Device, "AT+RX2DL=" + std::to_string(Delay));
}

RAK3172_Error_t RAK3172_LoRaWAN_GetRX2Delay(const RAK3172_t* const p_Device, uint8_t* const p_Delay)
{
    std::string Value;

    if(p_Delay == NULL)
    {
        return RAK3172_ERR_INVALID_ARG;
    }

    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+RX2DL=?", &Value));

    *p_Delay = (uint8_t)std::stoi(Value);

    return RAK3172_ERR_OK;
}

RAK3172_Error_t RAK3172_LoRaWAN_GetSNR(const RAK3172_t* const p_Device, int8_t* const p_SNR)
{
    std::string Value;

    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+SNR=?", &Value));

    *p_SNR = (int8_t)std::stoi(Value);

    return RAK3172_ERR_OK;
}

RAK3172_Error_t RAK3172_LoRaWAN_GetRSSI(const RAK3172_t* const p_Device, int8_t* const p_RSSI)
{
    std::string Value;

    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+RSSI=?", &Value));

    *p_RSSI = (int8_t)std::stoi(Value);

    return RAK3172_ERR_OK;
}

RAK3172_Error_t RAK3172_LoRaWAN_GetDuty(const RAK3172_t* const p_Device, uint8_t* const p_Duty)
{
    std::string Value;
    RAK3172_Band_t Band;

    RAK3172_ERROR_CHECK(RAK3172_LoRaWAN_GetBand(p_Device, &Band));

    if((Band != RAK_BAND_EU868) && (Band != RAK_BAND_RU864) && (Band != RAK_BAND_EU433))
    {
        return RAK3172_ERR_INVALID_ARG;
    }

    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+DUTYTIME=?", &Value));

    *p_Duty = (uint8_t)std::stoi(Value);

    return RAK3172_ERR_OK;
}

RAK3172_Error_t RAK3172_LoRaWAN_SetDataRate(const RAK3172_t* const p_Device, RAK3172_DataRate_t DR)
{
    if(DR > RAK_DR_7)
    {
        return RAK3172_ERR_INVALID_ARG;
    }

    return RAK3172_SendCommand(p_Device, "AT+DR=" + std::to_string(DR));
}

RAK3172_Error_t RAK3172_LoRaWAN_GetDataRate(const RAK3172_t* const p_Device, RAK3172_DataRate_t* const p_DR)
{
    std::string Value;

    if(p_DR == NULL)
    {
        return RAK3172_ERR_INVALID_ARG;
    }

    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+DR=?", &Value));

    *p_DR = (RAK3172_DataRate_t)std::stoi(Value);

    return RAK3172_ERR_OK;
}

RAK3172_Error_t RAK3172_LoRaWAN_SetADR(const RAK3172_t* const p_Device, bool Enable)
{
    return RAK3172_SendCommand(p_Device, "AT+ADR=" + std::to_string(Enable));
}

RAK3172_Error_t RAK3172_LoRaWAN_GetADR(const RAK3172_t* const p_Device, bool* const p_Enable)
{
    std::string Value;

    if(p_Enable == NULL)
    {
        return RAK3172_ERR_INVALID_ARG;
    }

    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+ADR=?", &Value));

    *p_Enable = (bool)std::stoi(Value);

    return RAK3172_ERR_OK;
}

RAK3172_Error_t RAK3172_LoRaWAN_SetJoinMode(const RAK3172_t* const p_Device, RAK3172_JoinMode_t Mode)
{
    return RAK3172_SendCommand(p_Device, "AT+NJM=" + std::to_string(Mode));
}

RAK3172_Error_t RAK3172_LoRaWAN_GetJoinMode(const RAK3172_t* const p_Device, RAK3172_JoinMode_t* const p_Mode)
{
    std::string Value;

    if(p_Mode == NULL)
    {
        return RAK3172_ERR_INVALID_ARG;
    }

    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+NJM=?", &Value));

    *p_Mode = (RAK3172_JoinMode_t)std::stoi(Value);

    return RAK3172_ERR_OK;
}

#endif