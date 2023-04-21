 /*
 * rak3172_lorawan.cpp
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

#ifdef CONFIG_RAK3172_MODE_WITH_LORAWAN

#include "../../Arch/Logging/rak3172_logging.h"
#include "../../Arch/Timer/rak3172_timer.h"

#ifdef CONFIG_RAK3172_PWRMGMT_ENABLE
    #include "../../Arch/PwrMgmt/rak3172_pwrmgmt.h"
#endif

#include "rak3172.h"

static const char* TAG = "RAK3172_LoRaWAN";

RAK3172_Error_t RAK3172_LoRaWAN_Init(RAK3172_t& p_Device, uint8_t TxPwr, RAK3172_JoinMode_t JoinMode, const uint8_t* const p_Key1, const uint8_t* const p_Key2, const uint8_t* const p_Key3, RAK3172_Class_t Class, RAK3172_Band_t Band, RAK3172_SubBand_t Subband, bool UseADR, uint32_t Timeout)
{
    std::string Command;

    if(((Class != RAK_CLASS_A) && (Class != RAK_CLASS_B) && (Class != RAK_CLASS_C)) || (p_Key1 == NULL) || (p_Key2 == NULL) || (p_Key3 == NULL))
    {
        return RAK3172_ERR_INVALID_ARG;
    }
    else if(p_Device.Internal.isInitialized == false)
    {
        return RAK3172_ERR_INVALID_STATE;
    }

    RAK3172_LOGI(TAG, "Initialize module in LoRaWAN mode...");
    RAK3172_ERROR_CHECK(RAK3172_SetMode(p_Device, RAK_MODE_LORAWAN));

    // Stop an ongoing joining process.
    RAK3172_ERROR_CHECK(RAK3172_LoRaWAN_StopJoin(p_Device));
    p_Device.LoRaWAN.isJoined = RAK3172_LoRaWAN_isJoined(p_Device, true);

    p_Device.Internal.isBusy = false;

    Command = "AT+CLASS=";
    Command += Class;
    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, Command));

	RAK3172_ERROR_CHECK(RAK3172_LoRaWAN_SetADR(p_Device, UseADR));
    RAK3172_ERROR_CHECK(RAK3172_LoRaWAN_SetBand(p_Device, Band));

    if(Subband != RAK_SUB_BAND_NONE)
    {
        RAK3172_ERROR_CHECK(RAK3172_LoRaWAN_SetSubBand(p_Device, Subband))
    }

    RAK3172_ERROR_CHECK(RAK3172_LoRaWAN_SetTxPwr(p_Device, TxPwr));
    RAK3172_ERROR_CHECK(RAK3172_LoRaWAN_SetJoinMode(p_Device, JoinMode));

    p_Device.LoRaWAN.Join = JoinMode;
    if(p_Device.LoRaWAN.Join == RAK_JOIN_OTAA)
    {
        RAK3172_LOGI(TAG, "Using OTAA mode");

        return RAK3172_LoRaWAN_SetOTAAKeys(p_Device, p_Key1, p_Key2, p_Key3);
    }
    else
    {
        RAK3172_LOGI(TAG, "Using ABP mode");

        return RAK3172_LoRaWAN_SetABPKeys(p_Device, p_Key1, p_Key2, p_Key3);
    }

    return RAK3172_ERR_INVALID_ARG;
}

RAK3172_Error_t RAK3172_LoRaWAN_SetOTAAKeys(const RAK3172_t& p_Device, const uint8_t* const p_DEVEUI, const uint8_t* const p_APPEUI, const uint8_t* const p_APPKEY)
{
    std::string AppEUIString;
    std::string DevEUIString;
    std::string AppKeyString;

    if((p_DEVEUI == NULL) || (p_APPEUI == NULL) || (p_APPKEY == NULL))
    {
        return RAK3172_ERR_INVALID_ARG;
    }
    else if(p_Device.LoRaWAN.Join != RAK_JOIN_OTAA)
    {
        return RAK3172_ERR_INVALID_STATE;
    }
    else if(p_Device.Mode != RAK_MODE_LORAWAN)
    {
        return RAK3172_ERR_INVALID_MODE;
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

    RAK3172_LOGD(TAG, "DEVEUI: %s - Size: %u", DevEUIString.c_str(), DevEUIString.length());
    RAK3172_LOGD(TAG, "APPEUI: %s - Size: %u", AppEUIString.c_str(), AppEUIString.length());
    RAK3172_LOGD(TAG, "APPKEY: %s - Size: %u", AppKeyString.c_str(), AppKeyString.length());

    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+DEVEUI=" + DevEUIString));
    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+APPEUI=" + AppEUIString));
    return RAK3172_SendCommand(p_Device, "AT+APPKEY=" + AppKeyString);
}

RAK3172_Error_t RAK3172_LoRaWAN_SetABPKeys(const RAK3172_t& p_Device, const uint8_t* const p_APPSKEY, const uint8_t* const p_NWKSKEY, const uint8_t* const p_DEVADDR)
{
    std::string AppSKEYString;
    std::string NwkSKEYString;
    std::string DevADDRString;

    if((p_APPSKEY == NULL) || (p_NWKSKEY == NULL) || (p_DEVADDR == NULL))
    {
        return RAK3172_ERR_INVALID_ARG;
    }
    else if(p_Device.LoRaWAN.Join != RAK_JOIN_ABP)
    {
        return RAK3172_ERR_INVALID_STATE;
    }
    else if(p_Device.Mode != RAK_MODE_LORAWAN)
    {
        return RAK3172_ERR_INVALID_MODE;
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

    RAK3172_LOGD(TAG, "APPSKEY: %s - Size: %u", AppSKEYString.c_str(), AppSKEYString.length());
    RAK3172_LOGD(TAG, "NWKSKEY: %s - Size: %u", NwkSKEYString.c_str(), NwkSKEYString.length());
    RAK3172_LOGD(TAG, "DEVADDR: %s - Size: %u", DevADDRString.c_str(), DevADDRString.length());

    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+APPSKEY=" + AppSKEYString));
    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+NWKSKEY=" + NwkSKEYString));
    return RAK3172_SendCommand(p_Device, "AT+DEVADDR=" + DevADDRString);
}

RAK3172_Error_t RAK3172_LoRaWAN_StartJoin(RAK3172_t& p_Device, uint8_t Attempts, uint32_t Timeout, bool Block, bool EnableAutoJoin, uint8_t Interval, RAK3172_Wait_t on_Wait)
{
    #ifdef CONFIG_RAK3172_USE_RUI3
        uint32_t TimeNow;
    #endif

    if(((Attempts == 0) && (Block == true)) || (Interval < 7))
    {
        return RAK3172_ERR_INVALID_ARG;
    }
    else if(p_Device.Internal.isBusy)
    {
        return RAK3172_ERR_BUSY;
    }
    else if(p_Device.LoRaWAN.isJoined)
    {
        return RAK3172_ERR_OK;
    }
    else if(p_Device.Mode != RAK_MODE_LORAWAN)
    {
        return RAK3172_ERR_INVALID_MODE;
    }
    #ifndef CONFIG_RAK3172_USE_RUI3
        else if(Block == false)
        {
            return RAK3172_ERR_INVALID_ARG;
        }
    #endif

    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+JOIN=1:" + std::to_string(EnableAutoJoin) + ":" + std::to_string(Interval) + ":" + std::to_string(Attempts)));

    #ifndef CONFIG_RAK3172_USE_RUI3
        p_Device.Internal.isJoinEvent = false;
    #endif

    p_Device.LoRaWAN.AttemptCounter = Attempts + 1;
    p_Device.Internal.isBusy = true;

    #ifdef CONFIG_RAK3172_USE_RUI3
        TimeNow = RAK3172_Timer_GetMilliseconds() / 1000ULL;
        do
        {
            if((Timeout > 0) && (((RAK3172_Timer_GetMilliseconds() / 1000ULL) - TimeNow) >= (Timeout * 1000ULL)))
            {
                RAK3172_LOGE(TAG, "Join timeout!");

                RAK3172_LoRaWAN_StopJoin(p_Device);

                p_Device.Internal.isBusy = false;

                return RAK3172_ERR_TIMEOUT;
            }

            if(on_Wait != NULL)
            {
                on_Wait();
            }

            if(p_Device.LoRaWAN.AttemptCounter == 0)
            {
                RAK3172_LoRaWAN_StopJoin(p_Device);

                break;
            }

            RAK3172_PwrMagnt_EnterLightSleep(p_Device);

            // We need this delay to prevent a task watchdog reset on ESP32.
            vTaskDelay(20 / portTICK_PERIOD_MS);
        } while((Block == true) && (p_Device.LoRaWAN.isJoined == false) && (p_Device.Internal.isBusy == true));

        if((Block == true) && (p_Device.LoRaWAN.isJoined == false))
        {
            return RAK3172_ERR_FAIL;
        }
    #else
        do
        {
            // Join event has occured and join was not successful. Start a new join.
            if((p_Device.Internal.isJoinEvent == true) && (p_Device.LoRaWAN.isJoined == false))
            {
                RAK3172_SendCommand(p_Device, "AT+JOIN=1:" + std::to_string(EnableAutoJoin) + ":" + std::to_string(Interval) + ":" + std::to_string(Attempts));
                p_Device.Internal.isJoinEvent = false;
            }
            // Join event has occured and join was successful.
            else if((p_Device.Internal.isJoinEvent == true) && (p_Device.LoRaWAN.isJoined == true))
            {
                break;
            }

            if(on_Wait != NULL)
            {
                on_Wait();
            }

            if(p_Device.LoRaWAN.AttemptCounter == 0)
            {
                RAK3172_LoRaWAN_StopJoin(p_Device);

                break;
            }

            RAK3172_PwrMagnt_EnterLightSleep(p_Device);

            vTaskDelay(20 / portTICK_PERIOD_MS);
        } while(p_Device.LoRaWAN.isJoined == false);

        if(p_Device.LoRaWAN.isJoined == false)
        {
            return RAK3172_ERR_FAIL;
        }
    #endif

    return RAK3172_ERR_OK;
}

RAK3172_Error_t RAK3172_LoRaWAN_StopJoin(const RAK3172_t& p_Device)
{
    return RAK3172_SendCommand(p_Device, "AT+JOIN=0:0:7:0");
}

bool RAK3172_LoRaWAN_isJoined(RAK3172_t& p_Device, bool Refresh)
{
    std::string Response;

    if(p_Device.Mode != RAK_MODE_LORAWAN)
    {
        return false;
    }
    else if(Refresh == false)
    {
        return p_Device.LoRaWAN.isJoined;
    }

    p_Device.LoRaWAN.isJoined = false;

    if(RAK3172_SendCommand(p_Device, "AT+NJS=?", &Response) != RAK3172_ERR_OK)
    {
        return false;
    }

    if(Response.compare("1") == 0)
    {
        p_Device.LoRaWAN.isJoined = true;
    }

    return p_Device.LoRaWAN.isJoined;
}

RAK3172_Error_t RAK3172_LoRaWAN_Transmit(RAK3172_t& p_Device, uint8_t Port, const uint8_t* const p_Buffer, uint16_t Length, uint8_t Retries)
{
    return RAK3172_LoRaWAN_Transmit(p_Device, Port, p_Buffer, Length, Retries);
}

RAK3172_Error_t RAK3172_LoRaWAN_Transmit(RAK3172_t& p_Device, uint8_t Port, const void* const p_Buffer, uint16_t Length, uint8_t Retries, bool Confirmed, RAK3172_Wait_t Wait)
{
    std::string Payload;
    std::string Status;
    char Buffer[3];

    if(((p_Buffer == NULL) && (Length == 0)) || (Length > 1000) || (Port == 0) || (Port > 233) || (Retries > 7))
    {
        return RAK3172_ERR_INVALID_ARG;
    }
    else if(p_Device.Internal.isBusy)
    {
        return RAK3172_ERR_BUSY;
    }
    else if(p_Device.LoRaWAN.isJoined == false)
    {
        return RAK3172_ERR_NOT_CONNECTED;
    }
    else if(p_Device.Mode != RAK_MODE_LORAWAN)
    {
        return RAK3172_ERR_INVALID_MODE;
    }
    else if(Length == 0)
    {
        return RAK3172_ERR_OK;
    }

    if(Confirmed)
    {
        RAK3172_ERROR_CHECK(RAK3172_LoRaWAN_SetRetries(p_Device, Retries));
    }

    // Encode the payload into an ASCII string.
    for(uint8_t i = 0x00; i < Length; i++)
    {
        sprintf(Buffer, "%02x", ((uint8_t*)p_Buffer)[i]);
        Payload += std::string(Buffer);
    }

    if(Length > 500)
    {
        RAK3172_SendCommand(p_Device, "AT+LPSEND=" + std::to_string(Port) + ":" + std::to_string(Confirmed) + ":" + Payload, NULL, &Status);
    }
    else
    {
        RAK3172_ERROR_CHECK(RAK3172_LoRaWAN_SetConfirmation(p_Device, Confirmed));
        RAK3172_SendCommand(p_Device, "AT+SEND=" + std::to_string(Port) + ":" + Payload, NULL, &Status);
    }

    // The device is busy. Leave the function with an invalid state error.
    if(Status.find("AT_BUSY_ERROR") != std::string::npos)
    {
        return RAK3172_ERR_BUSY;
    }
    else if(Status.find("Restricted") != std::string::npos)
    {
        return RAK3172_ERR_RESTRICTED;
    }
    // No transmission error and no confirmation needed.
    else if((Confirmed == false) && (Status.find("OK") == std::string::npos))
    {
        return RAK3172_ERR_OK;
    }

    p_Device.Internal.isBusy = true;
    p_Device.LoRaWAN.ConfirmError = false;

    // Wait for the confirmation if needed.
    if(Confirmed)
    {
        do
        {
            if(Wait)
            {
                Wait();
            }

            RAK3172_PwrMagnt_EnterLightSleep(p_Device);

            // We need this delay to prevent a task watchdog reset on ESP32.
            vTaskDelay(20 / portTICK_PERIOD_MS);
        } while(p_Device.Internal.isBusy);
    }

    p_Device.Internal.isBusy = false;

    if(Confirmed && p_Device.LoRaWAN.ConfirmError)
    {
        return RAK3172_ERR_INVALID_RESPONSE;
    }

    return RAK3172_ERR_OK;
}

RAK3172_Error_t RAK3172_LoRaWAN_Receive(RAK3172_t& p_Device, RAK3172_Rx_t* p_Message, uint32_t Timeout)
{
    RAK3172_Rx_t* FromQueue = NULL;

    if(p_Message == NULL)
    {
        return RAK3172_ERR_INVALID_ARG;
    }
    else if(p_Device.LoRaWAN.isJoined == false)
    {
        return RAK3172_ERR_NOT_CONNECTED;
    }
    else if(p_Device.Mode != RAK_MODE_LORAWAN)
    {
        return RAK3172_ERR_INVALID_MODE;
    }

    if(xQueueReceive(p_Device.Internal.ReceiveQueue, &FromQueue, (Timeout * 1000UL) / portTICK_PERIOD_MS) != pdPASS)
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

RAK3172_Error_t RAK3172_LoRaWAN_SetRetries(const RAK3172_t& p_Device, uint8_t Retries)
{
    if(Retries > 7)
    {
        return RAK3172_ERR_INVALID_ARG;
    }
    else if(p_Device.Mode != RAK_MODE_LORAWAN)
    {
        return RAK3172_ERR_INVALID_MODE;
    }

    return RAK3172_SendCommand(p_Device, "AT+RETY=" + std::to_string(Retries));
}

RAK3172_Error_t RAK3172_LoRaWAN_GetRetries(const RAK3172_t& p_Device, uint8_t* const p_Retries)
{
    std::string Response;

    if(p_Retries == NULL)
    {
        return RAK3172_ERR_INVALID_ARG;
    }
    else if(p_Device.Mode != RAK_MODE_LORAWAN)
    {
        return RAK3172_ERR_INVALID_MODE;
    }

    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+RETY=?", &Response));

    *p_Retries = static_cast<uint8_t>(std::stoi(Response));

    return RAK3172_ERR_OK;
}

RAK3172_Error_t RAK3172_LoRaWAN_SetPNM(const RAK3172_t& p_Device, bool Enable)
{
    if(p_Device.Mode != RAK_MODE_LORAWAN)
    {
        return RAK3172_ERR_INVALID_MODE;
    }

    return RAK3172_SendCommand(p_Device, "AT+PNM=" + std::to_string(Enable));
}

RAK3172_Error_t RAK3172_LoRaWAN_GetPNM(const RAK3172_t& p_Device, bool* const p_Enable)
{
    std::string Response;

    if(p_Enable == NULL)
    {
        return RAK3172_ERR_INVALID_ARG;
    }
    else if(p_Device.Mode != RAK_MODE_LORAWAN)
    {
        return RAK3172_ERR_INVALID_MODE;
    }

    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+PNM=?", &Response));

    *p_Enable = static_cast<bool>(std::stoi(Response));

    return RAK3172_ERR_OK;
}

RAK3172_Error_t RAK3172_LoRaWAN_SetConfirmation(const RAK3172_t& p_Device, bool Enable)
{
    if(p_Device.Mode != RAK_MODE_LORAWAN)
    {
        return RAK3172_ERR_INVALID_MODE;
    }

    return RAK3172_SendCommand(p_Device, "AT+CFM=" + std::to_string(Enable));
}

RAK3172_Error_t RAK3172_LoRaWAN_GetConfirmation(const RAK3172_t& p_Device, bool* const p_Enable)
{
    std::string Response;

    if(p_Enable == NULL)
    {
        return RAK3172_ERR_INVALID_ARG;
    }
    else if(p_Device.Mode != RAK_MODE_LORAWAN)
    {
        return RAK3172_ERR_INVALID_MODE;
    }

    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+CFM=?", &Response));

    *p_Enable = static_cast<bool>(std::stoi(Response));

    return RAK3172_ERR_OK;
}

RAK3172_Error_t RAK3172_LoRaWAN_SetBand(const RAK3172_t& p_Device, RAK3172_Band_t Band)
{
    if(p_Device.Mode != RAK_MODE_LORAWAN)
    {
        return RAK3172_ERR_INVALID_MODE;
    }

    return RAK3172_SendCommand(p_Device, "AT+BAND=" + std::to_string(static_cast<uint8_t>(Band)));
}

RAK3172_Error_t RAK3172_LoRaWAN_GetBand(const RAK3172_t& p_Device, RAK3172_Band_t* const p_Band)
{
    std::string Response;

    if(p_Band == NULL)
    {
        return RAK3172_ERR_INVALID_ARG;
    }
    else if(p_Device.Mode != RAK_MODE_LORAWAN)
    {
        return RAK3172_ERR_INVALID_MODE;
    }

    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+BAND=?", &Response));

    *p_Band = static_cast<RAK3172_Band_t>(std::stoi(Response));

    return RAK3172_ERR_OK;
}

RAK3172_Error_t RAK3172_LoRaWAN_SetSubBand(const RAK3172_t& p_Device, RAK3172_SubBand_t Band)
{
    RAK3172_Band_t Dummy;

    if(p_Device.Mode != RAK_MODE_LORAWAN)
    {
        return RAK3172_ERR_INVALID_MODE;
    }
    else if(Band == RAK_SUB_BAND_NONE)
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

            sprintf(Temp, "%04X", 1 << (Band - 2));
    
            return RAK3172_SendCommand(p_Device, "AT+MASK=" + std::string(Temp));
        }
    }

    return RAK3172_ERR_FAIL;
}

RAK3172_Error_t RAK3172_LoRaWAN_GetSubBand(const RAK3172_t& p_Device, RAK3172_SubBand_t* const p_Band)
{
    RAK3172_Band_t Dummy;
    std::string Response;
    uint32_t Mask;
    uint8_t Shifts = 1;

    if(p_Band == NULL)
    {
        return RAK3172_ERR_INVALID_ARG;
    }
    else if(p_Device.Mode != RAK_MODE_LORAWAN)
    {
        return RAK3172_ERR_INVALID_MODE;
    }

    RAK3172_ERROR_CHECK(RAK3172_LoRaWAN_GetBand(p_Device, &Dummy));

    if((Dummy != RAK_BAND_US915) && (Dummy != RAK_BAND_AU915) && (Dummy != RAK_BAND_CN470))
    {
        *p_Band = RAK_SUB_BAND_NONE;

        return RAK3172_ERR_OK;
    }

    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+MASK=?", &Response));

    Mask = std::stoi(Response);

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

        *p_Band = static_cast<RAK3172_SubBand_t>(Shifts + 2);

        return RAK3172_ERR_OK;
    }
}

RAK3172_Error_t RAK3172_LoRaWAN_SetTxPwr(const RAK3172_t& p_Device, uint8_t TxPwr)
{
    uint8_t TxPwrIndex = 0;
    RAK3172_Band_t Band;

    RAK3172_ERROR_CHECK(RAK3172_LoRaWAN_GetBand(p_Device, &Band));

    RAK3172_LOGD(TAG, "Set Tx power to: %u dBm", TxPwr);

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
            TxPwrIndex = static_cast<uint8_t>((EIRP - TxPwr) / 2);
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
            TxPwrIndex = static_cast<uint8_t>((MaxPwr - TxPwr) / 2);
        }
    }
    else
    {
        RAK3172_LOGE(TAG, "Tx power is not implemented for the selected frequency band!");
    }

    RAK3172_LOGD(TAG, "Set Tx power index: %u", TxPwrIndex);

    return RAK3172_SendCommand(p_Device, "AT+TXP=" + std::to_string(TxPwrIndex));
}

RAK3172_Error_t RAK3172_LoRaWAN_SetJoin1Delay(const RAK3172_t& p_Device, uint32_t Delay)
{
    uint32_t Delay_Temp;

    if(p_Device.Mode != RAK_MODE_LORAWAN)
    {
        return RAK3172_ERR_INVALID_MODE;
    }

    #ifdef CONFIG_RAK3172_USE_RUI3
        if((Delay < 1) || (Delay > 14))
        {
            return RAK3172_ERR_INVALID_ARG;
        }
    #endif

    Delay_Temp = Delay;

    #ifndef CONFIG_RAK3172_USE_RUI3
        Delay_Temp *= 1000;
    #endif

    return RAK3172_SendCommand(p_Device, "AT+JN1DL=" + std::to_string(Delay_Temp));
}

RAK3172_Error_t RAK3172_LoRaWAN_GetJoin1Delay(const RAK3172_t& p_Device, uint32_t* const p_Delay)
{
    std::string Response;

    if(p_Delay == NULL)
    {
        return RAK3172_ERR_INVALID_ARG;
    }
    else if(p_Device.Mode != RAK_MODE_LORAWAN)
    {
        return RAK3172_ERR_INVALID_MODE;
    }

    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+JN1DL=?", &Response));

    *p_Delay = static_cast<uint8_t>(std::stoi(Response));

    return RAK3172_ERR_OK;
}

RAK3172_Error_t RAK3172_LoRaWAN_SetJoin2Delay(const RAK3172_t& p_Device, uint32_t Delay)
{
    uint32_t Delay_Temp;

    if(p_Device.Mode != RAK_MODE_LORAWAN)
    {
        return RAK3172_ERR_INVALID_MODE;
    }

    #ifdef CONFIG_RAK3172_USE_RUI3
        if((Delay < 2) || (Delay > 15))
        {
            return RAK3172_ERR_INVALID_ARG;
        }
    #endif

    Delay_Temp = Delay;

    #ifndef CONFIG_RAK3172_USE_RUI3
        Delay_Temp *= 1000;
    #endif

    return RAK3172_SendCommand(p_Device, "AT+JN2DL=" + std::to_string(Delay_Temp));
}

RAK3172_Error_t RAK3172_LoRaWAN_GetJoin2Delay(const RAK3172_t& p_Device, uint32_t* const p_Delay)
{
    std::string Response;

    if(p_Delay == NULL)
    {
        return RAK3172_ERR_INVALID_ARG;
    }
    else if(p_Device.Mode != RAK_MODE_LORAWAN)
    {
        return RAK3172_ERR_INVALID_MODE;
    }

    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+JN2DL=?", &Response));

    *p_Delay = static_cast<uint8_t>(std::stoi(Response));

    return RAK3172_ERR_OK;
}

RAK3172_Error_t RAK3172_LoRaWAN_SetRX1Delay(const RAK3172_t& p_Device, uint32_t Delay)
{
    uint32_t Delay_Temp;

    if(p_Device.Mode != RAK_MODE_LORAWAN)
    {
        return RAK3172_ERR_INVALID_MODE;
    }
    #ifdef CONFIG_RAK3172_USE_RUI3
        if((Delay < 1) || (Delay > 15))
        {
            return RAK3172_ERR_INVALID_ARG;
        }
    #endif

    Delay_Temp = Delay;

    #ifndef CONFIG_RAK3172_USE_RUI3
        Delay_Temp *= 1000;
    #endif

    return RAK3172_SendCommand(p_Device, "AT+RX1DL=" + std::to_string(Delay_Temp));
}

RAK3172_Error_t RAK3172_LoRaWAN_GetRX1Delay(const RAK3172_t& p_Device, uint32_t* const p_Delay)
{
    std::string Response;

    if(p_Delay == NULL)
    {
        return RAK3172_ERR_INVALID_ARG;
    }
    else if(p_Device.Mode != RAK_MODE_LORAWAN)
    {
        return RAK3172_ERR_INVALID_MODE;
    }

    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+RX1DL=?", &Response));

    *p_Delay = static_cast<uint8_t>(std::stoi(Response));

    return RAK3172_ERR_OK;
}

RAK3172_Error_t RAK3172_LoRaWAN_SetRX2Delay(const RAK3172_t& p_Device, uint32_t Delay)
{
    uint32_t Delay_Temp;

    if(p_Device.Mode != RAK_MODE_LORAWAN)
    {
        return RAK3172_ERR_INVALID_MODE;
    }
    #ifdef CONFIG_RAK3172_USE_RUI3
        if((Delay < 2) || (Delay > 16))
        {
            return RAK3172_ERR_INVALID_ARG;
        }
    #endif

    Delay_Temp = Delay;

    #ifndef CONFIG_RAK3172_USE_RUI3
        Delay_Temp *= 1000;
    #endif

    return RAK3172_SendCommand(p_Device, "AT+RX2DL=" + std::to_string(Delay_Temp));
}

RAK3172_Error_t RAK3172_LoRaWAN_GetRX2Delay(const RAK3172_t& p_Device, uint32_t* const p_Delay)
{
    std::string Response;

    if(p_Delay == NULL)
    {
        return RAK3172_ERR_INVALID_ARG;
    }
    else if(p_Device.Mode != RAK_MODE_LORAWAN)
    {
        return RAK3172_ERR_INVALID_MODE;
    }

    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+RX2DL=?", &Response));

    *p_Delay = static_cast<uint8_t>(std::stoi(Response));

    return RAK3172_ERR_OK;
}

RAK3172_Error_t RAK3172_LoRaWAN_SetRX2Freq(const RAK3172_t& p_Device, uint32_t Frequency)
{
    if(p_Device.Mode != RAK_MODE_LORAWAN)
    {
        return RAK3172_ERR_INVALID_MODE;
    }

    return RAK3172_SendCommand(p_Device, "AT+RX2FQ=" + std::to_string(Frequency));
}

RAK3172_Error_t RAK3172_LoRaWAN_GetRX2Freq(const RAK3172_t& p_Device, uint32_t* const p_Frequency)
{
    std::string Response;

    if(p_Frequency == NULL)
    {
        return RAK3172_ERR_INVALID_ARG;
    }
    else if(p_Device.Mode != RAK_MODE_LORAWAN)
    {
        return RAK3172_ERR_INVALID_MODE;
    }

    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+RX2FQ=?", &Response));

    *p_Frequency = static_cast<uint8_t>(std::stoi(Response));

    return RAK3172_ERR_OK;
}

RAK3172_Error_t RAK3172_LoRaWAN_SetRX2DataRate(const RAK3172_t& p_Device, uint8_t DataRate)
{
    #ifdef CONFIG_RAK3172_USE_RUI3
        RAK3172_Band_t Band;
    #endif

    if(p_Device.Mode != RAK_MODE_LORAWAN)
    {
        return RAK3172_ERR_INVALID_MODE;
    }
    #ifndef CONFIG_RAK3172_USE_RUI3
        if(DataRate > 7)
        {
            return RAK3172_ERR_INVALID_ARG;
        }
    #endif

    #ifdef CONFIG_RAK3172_USE_RUI3
        RAK3172_ERROR_CHECK(RAK3172_LoRaWAN_GetBand(p_Device, &Band));

        if((Band == RAK_BAND_EU433) || (Band == RAK_BAND_RU864) || (Band == RAK_BAND_IN865) || (Band == RAK_BAND_EU868) || (Band == RAK_BAND_CN470) || (Band == RAK_BAND_KR920))
        {
            if(DataRate > 5)
            {
                return RAK3172_ERR_INVALID_ARG;
            }
        }
        else if(Band == RAK_BAND_AS923)
        {
            if((DataRate < 2) || (DataRate > 5))
            {
                return RAK3172_ERR_INVALID_ARG;
            }
        }
        else if((Band == RAK_BAND_US915) || (Band == RAK_BAND_AU915))
        {
            if((DataRate < 8) || (DataRate > 13))
            {
                return RAK3172_ERR_INVALID_ARG;
            }
        }
    #endif

    return RAK3172_SendCommand(p_Device, "AT+RX2DL=" + std::to_string(DataRate));
}

RAK3172_Error_t RAK3172_LoRaWAN_GetRX2DataRate(const RAK3172_t& p_Device, uint32_t* const p_DataRate)
{
    std::string Response;

    if(p_DataRate == NULL)
    {
        return RAK3172_ERR_INVALID_ARG;
    }
    else if(p_Device.Mode != RAK_MODE_LORAWAN)
    {
        return RAK3172_ERR_INVALID_MODE;
    }

    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+RX2DR=?", &Response));

    *p_DataRate = static_cast<uint8_t>(std::stoi(Response));

    return RAK3172_ERR_OK;
}

RAK3172_Error_t RAK3172_LoRaWAN_GetSNR(const RAK3172_t& p_Device, int8_t* const p_SNR)
{
    std::string Response;

    if(p_SNR == NULL)
    {
        return RAK3172_ERR_INVALID_ARG;
    }
    else if(p_Device.Mode != RAK_MODE_LORAWAN)
    {
        return RAK3172_ERR_INVALID_MODE;
    }

    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+SNR=?", &Response));

    *p_SNR = static_cast<int8_t>(std::stoi(Response));

    return RAK3172_ERR_OK;
}

RAK3172_Error_t RAK3172_LoRaWAN_GetRSSI(const RAK3172_t& p_Device, int8_t* const p_RSSI)
{
    std::string Response;

    if(p_RSSI == NULL)
    {
        return RAK3172_ERR_INVALID_ARG;
    }
    else if(p_Device.Mode != RAK_MODE_LORAWAN)
    {
        return RAK3172_ERR_INVALID_MODE;
    }

    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+RSSI=?", &Response));

    *p_RSSI = static_cast<int8_t>(std::stoi(Response));

    return RAK3172_ERR_OK;
}

RAK3172_Error_t RAK3172_LoRaWAN_GetDuty(const RAK3172_t& p_Device, uint8_t* const p_Duty)
{
    std::string Response;
    RAK3172_Band_t Band;

    RAK3172_ERROR_CHECK(RAK3172_LoRaWAN_GetBand(p_Device, &Band));

    if((Band != RAK_BAND_EU868) && (Band != RAK_BAND_RU864) && (Band != RAK_BAND_EU433))
    {
        return RAK3172_ERR_INVALID_ARG;
    }
    else if(p_Device.Mode != RAK_MODE_LORAWAN)
    {
        return RAK3172_ERR_INVALID_MODE;
    }

    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+DUTYTIME=?", &Response));

    *p_Duty = static_cast<uint8_t>(std::stoi(Response));

    return RAK3172_ERR_OK;
}

RAK3172_Error_t RAK3172_LoRaWAN_SetDataRate(const RAK3172_t& p_Device, RAK3172_DataRate_t DR)
{
    if(DR > RAK_DR_7)
    {
        return RAK3172_ERR_INVALID_ARG;
    }
    else if(p_Device.Mode != RAK_MODE_LORAWAN)
    {
        return RAK3172_ERR_INVALID_MODE;
    }

    return RAK3172_SendCommand(p_Device, "AT+DR=" + std::to_string(DR));
}

RAK3172_Error_t RAK3172_LoRaWAN_GetDataRate(const RAK3172_t& p_Device, RAK3172_DataRate_t* const p_DR)
{
    std::string Response;

    if(p_DR == NULL)
    {
        return RAK3172_ERR_INVALID_ARG;
    }
    else if(p_Device.Mode != RAK_MODE_LORAWAN)
    {
        return RAK3172_ERR_INVALID_MODE;
    }

    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+DR=?", &Response));

    *p_DR = static_cast<RAK3172_DataRate_t>(std::stoi(Response));

    return RAK3172_ERR_OK;
}

RAK3172_Error_t RAK3172_LoRaWAN_SetADR(const RAK3172_t& p_Device, bool Enable)
{
    if(p_Device.Mode != RAK_MODE_LORAWAN)
    {
        return RAK3172_ERR_INVALID_MODE;
    }

    return RAK3172_SendCommand(p_Device, "AT+ADR=" + std::to_string(Enable));
}

RAK3172_Error_t RAK3172_LoRaWAN_GetADR(const RAK3172_t& p_Device, bool* const p_Enable)
{
    std::string Response;

    if(p_Enable == NULL)
    {
        return RAK3172_ERR_INVALID_ARG;
    }
    else if(p_Device.Mode != RAK_MODE_LORAWAN)
    {
        return RAK3172_ERR_INVALID_MODE;
    }

    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+ADR=?", &Response));

    *p_Enable = static_cast<bool>(std::stoi(Response));

    return RAK3172_ERR_OK;
}

RAK3172_Error_t RAK3172_LoRaWAN_SetJoinMode(const RAK3172_t& p_Device, RAK3172_JoinMode_t Mode)
{
    if(p_Device.Mode != RAK_MODE_LORAWAN)
    {
        return RAK3172_ERR_INVALID_MODE;
    }

    return RAK3172_SendCommand(p_Device, "AT+NJM=" + std::to_string(Mode));
}

RAK3172_Error_t RAK3172_LoRaWAN_GetJoinMode(const RAK3172_t& p_Device, RAK3172_JoinMode_t* const p_Mode)
{
    std::string Response;

    if(p_Mode == NULL)
    {
        return RAK3172_ERR_INVALID_ARG;
    }
    else if(p_Device.Mode != RAK_MODE_LORAWAN)
    {
        return RAK3172_ERR_INVALID_MODE;
    }

    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+NJM=?", &Response));

    *p_Mode = static_cast<RAK3172_JoinMode_t>(std::stoi(Response));

    return RAK3172_ERR_OK;
}

RAK3172_Error_t RAK3172_LoRaWAN_GetRSSI(const RAK3172_t& p_Device, int* p_RSSI)
{
    std::string Response;

    if(p_RSSI == NULL)
    {
        return RAK3172_ERR_INVALID_ARG;
    }
    else if(p_Device.Mode != RAK_MODE_LORAWAN)
    {
        return RAK3172_ERR_INVALID_MODE;
    }

    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+RSSI=?", &Response));

    *p_RSSI = std::stoi(Response);

    return RAK3172_ERR_OK;
}

RAK3172_Error_t RAK3172_LoRaWAN_GetSNR(const RAK3172_t& p_Device, int* p_SNR)
{
    std::string Response;

    if(p_SNR == NULL)
    {
        return RAK3172_ERR_INVALID_ARG;
    }
    else if(p_Device.Mode != RAK_MODE_LORAWAN)
    {
        return RAK3172_ERR_INVALID_MODE;
    }

    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+SNR=?", &Response));

    *p_SNR = std::stoi(Response);

    return RAK3172_ERR_OK;
}

#endif
