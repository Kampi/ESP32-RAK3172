 /*
 * rak3172_p2p.h
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

#ifdef CONFIG_RAK3172_PROT_WITH_P2P

#include <esp_log.h>
#include <esp_sleep.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <freertos/queue.h>

#include "rak3172.h"

static const char* TAG = "RAK3172_P2P";

/** @brief          LoRa P2P receive task.
 *  @param p_Arg    Pointer to task arguments
 */
static void RAK3172_P2P_ReceiveTask(void* p_Arg)
{
    RAK3172_t* Device = (RAK3172_t*)p_Arg;

    Device->P2P.Active = true;

    while(Device->P2P.Active)
    {
        RAK3172_Rx_t* FromQueue = NULL;

        if(xQueueReceive(Device->Internal.ReceiveQueue, &FromQueue, 20 / portTICK_PERIOD_MS) == pdPASS)
        {
            if(Device->P2P.Timeout != RAK_REC_REPEAT)
            {
                Device->P2P.isRxTimeout = true;
                Device->Internal.isBusy = false;
                Device->P2P.Active = false;
            }

            xQueueSend(Device->P2P.ListenQueue, &FromQueue, 0);
        }
    }

    vTaskSuspend(NULL);
    vTaskDelete(NULL);
}

RAK3172_Error_t RAK3172_P2P_Init(RAK3172_t* const p_Device, uint32_t Frequency, RAK3172_PSF_t Spread, RAK3172_BW_t Bandwidth, RAK3172_CR_t CodeRate, uint16_t Preamble, uint8_t Power, uint32_t Timeout)
{
    std::string Value;

    if((p_Device == NULL) || (Frequency > 960000000) || (Frequency < 150000000) || (CodeRate > RAK_CR_3) || (CodeRate < RAK_CR_0) || (Preamble < 2) || (Power > 22) || (Power < 5))
    {
        return RAK3172_ERR_INVALID_ARG;
    }

    Value = std::to_string(Frequency) + ":" + 
            std::to_string(Spread) + ":" + 
            std::to_string(Bandwidth) + ":" + 
            std::to_string(CodeRate) + ":" + 
            std::to_string(Preamble) + ":" + 
            std::to_string(Power);

    ESP_LOGI(TAG, "Initialize module in P2P mode...");
    RAK3172_ERROR_CHECK(RAK3172_SetMode(p_Device, RAK_MODE_P2P));

    p_Device->Internal.isBusy = false;

    ESP_LOGD(TAG, "     Use configuration: %s", Value.c_str());

    #ifdef CONFIG_RAK3172_USE_RUI3
        RAK3172_ERROR_CHECK(RAK3172_P2P_isEncryptionEnabled(p_Device, &p_Device->P2P.isEncryptionEnabled));
    #endif

    return RAK3172_SendCommand(p_Device, "AT+P2P=" + Value);
}

RAK3172_Error_t RAK3172_P2P_GetConfig(const RAK3172_t* const p_Device, std::string* const p_Config)
{
    if(p_Config == NULL)
    {
        return RAK3172_ERR_INVALID_ARG;
    }

    return RAK3172_SendCommand(p_Device, "AT+P2P=?", p_Config);
}

RAK3172_Error_t RAK3172_P2P_SetFrequency(const RAK3172_t* const p_Device, uint32_t Freq)
{
    if((Freq > 960000000) || (Freq < 150000000))
    {
        return RAK3172_ERR_INVALID_ARG;
    }

    return RAK3172_SendCommand(p_Device, "AT+PFREQ=" + std::to_string(Freq));
}

RAK3172_Error_t RAK3172_P2P_GetFrequency(const RAK3172_t* const p_Device, uint32_t* const p_Freq)
{
    std::string Value;

    if(p_Freq == NULL)
    {
        return RAK3172_ERR_INVALID_ARG;
    }

    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+PFREQ=?", &Value));

    *p_Freq = std::stoi(Value);

    return RAK3172_ERR_OK;
}

RAK3172_Error_t RAK3172_P2P_SetSpreading(const RAK3172_t* const p_Device, RAK3172_PSF_t Spread)
{
    if((Spread > RAK_PSF_12) || (Spread < RAK_PSF_6))
    {
        return RAK3172_ERR_INVALID_ARG;
    }

    return RAK3172_SendCommand(p_Device, "AT+PSF=" + std::to_string(Spread));
}

RAK3172_Error_t RAK3172_P2P_GetSpreading(const RAK3172_t* const p_Device, RAK3172_PSF_t* const p_Spread)
{
    std::string Value;

    if(p_Spread == NULL)
    {
        return RAK3172_ERR_INVALID_ARG;
    }

    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+PSF=?", &Value));

    *p_Spread = (RAK3172_PSF_t)std::stoi(Value);

    return RAK3172_ERR_OK;
}

RAK3172_Error_t RAK3172_P2P_SetBandwidth(const RAK3172_t* const p_Device, RAK3172_BW_t Bandwidth)
{
    if((Bandwidth != RAK_BW_125) && (Bandwidth != RAK_BW_250) && (Bandwidth != RAK_BW_500))
    {
        return RAK3172_ERR_INVALID_ARG;
    }

    return RAK3172_SendCommand(p_Device, "AT+PBW=" + std::to_string(Bandwidth));
}

RAK3172_Error_t RAK3172_P2P_GetBandwidth(const RAK3172_t* const p_Device, RAK3172_BW_t* const p_Bandwidth)
{
    std::string Value;

    if(p_Bandwidth == NULL)
    {
        return RAK3172_ERR_INVALID_ARG;
    }

    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+PBW=?", &Value));

    *p_Bandwidth = (RAK3172_BW_t)std::stoi(Value);

    return RAK3172_ERR_OK;
}

RAK3172_Error_t RAK3172_P2P_SetCodeRate(const RAK3172_t* const p_Device, RAK3172_CR_t CodeRate)
{
    if((CodeRate > RAK_CR_3) || (CodeRate < RAK_CR_0))
    {
        return RAK3172_ERR_INVALID_ARG;
    }

    return RAK3172_SendCommand(p_Device, "AT+PCR=" + std::to_string(CodeRate));  
}

RAK3172_Error_t RAK3172_P2P_GetCodeRate(const RAK3172_t* const p_Device, RAK3172_CR_t* const p_CodeRate)
{
    std::string Value;

    if(p_CodeRate == NULL)
    {
        return RAK3172_ERR_INVALID_ARG;
    }

    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+PCR=?", &Value));

    *p_CodeRate = (RAK3172_CR_t)std::stoi(Value);

    return RAK3172_ERR_OK;
}

RAK3172_Error_t RAK3172_P2P_SetPreamble(const RAK3172_t* const p_Device, uint16_t Preamble)
{
    if(Preamble < 2)
    {
        return RAK3172_ERR_INVALID_ARG;
    }

    return RAK3172_SendCommand(p_Device, "AT+PPL=" + std::to_string(Preamble));  
}

RAK3172_Error_t RAK3172_P2P_GetPreamble(const RAK3172_t* const p_Device, uint16_t* const p_Preamble)
{
    std::string Value;

    if(p_Preamble == NULL)
    {
        return RAK3172_ERR_INVALID_ARG;
    }

    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+PPL=?", &Value));

    *p_Preamble = (uint16_t)std::stoi(Value);

    return RAK3172_ERR_OK;
}

RAK3172_Error_t RAK3172_P2P_SetPower(const RAK3172_t* const p_Device, uint8_t Power)
{
    if((Power > 22) || (Power < 5))
    {
        return RAK3172_ERR_INVALID_ARG;
    }

    return RAK3172_SendCommand(p_Device, "AT+PTP=" + std::to_string(Power));  
}

RAK3172_Error_t RAK3172_P2P_GetPower(const RAK3172_t* const p_Device, uint8_t* const p_Power)
{
    std::string Value;

    if(p_Power == NULL)
    {
        return RAK3172_ERR_INVALID_ARG;
    }

    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+PTP=?", &Value));

    *p_Power = (uint8_t)std::stoi(Value);

    return RAK3172_ERR_OK;
}

RAK3172_Error_t RAK3172_P2P_Transmit(const RAK3172_t* const p_Device, const uint8_t* const p_Buffer, uint8_t Length)
{
    std::string Payload;

    if((p_Device == NULL) || ((p_Buffer == NULL) && (Length > 0)))
    {
        return RAK3172_ERR_INVALID_ARG;
    }
    else if(Length == 0)
    {
        return RAK3172_ERR_OK;
    }

    // Encode the payload into an ASCII string.
    for(uint8_t i = 0; i < Length; i++)
    {
        char Buffer[3];

        sprintf(Buffer, "%02x", ((uint8_t*)p_Buffer)[i]);
        Payload += std::string(Buffer);
    }

    return RAK3172_SendCommand(p_Device, "AT+PSEND=" + Payload);
}

RAK3172_Error_t RAK3172_P2P_Receive(RAK3172_t* const p_Device, RAK3172_Rx_t* const p_Message, uint16_t Timeout)
{
    RAK3172_Rx_t* FromQueue = NULL;

    if((p_Device == NULL) || (p_Message == NULL) || (Timeout > 65534))
    {
        return RAK3172_ERR_INVALID_ARG;
    }

    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+PRECV=" + std::to_string(Timeout)));

    p_Device->P2P.isRxTimeout = false;
    do
    {
        if(xQueueReceive(p_Device->Internal.ReceiveQueue, &FromQueue, 20 / portTICK_PERIOD_MS) == pdPASS)
        {
            p_Message->RSSI = FromQueue->RSSI;
            p_Message->SNR = FromQueue->SNR;
            p_Message->Payload = FromQueue->Payload;

            delete FromQueue;

            return RAK3172_ERR_OK;
        }
    } while(p_Device->P2P.isRxTimeout == false);

    return RAK3172_ERR_TIMEOUT;
}

RAK3172_Error_t RAK3172_P2P_Listen(RAK3172_t* const p_Device, uint16_t Timeout, uint8_t CoreID, uint8_t Priority, uint8_t QueueSize)
{
    if((p_Device == NULL) || (QueueSize == 0))
    {
        return RAK3172_ERR_INVALID_ARG;
    }

    p_Device->P2P.Timeout = Timeout;

    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+PRECV=" + std::to_string(p_Device->P2P.Timeout)));

    p_Device->P2P.ListenQueue = xQueueCreate(QueueSize, sizeof(RAK3172_Rx_t*));
    if(p_Device->P2P.ListenQueue == NULL)
    {
        RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+PRECV=0"));

        return RAK3172_ERR_NO_MEM;
    }

    if(p_Device->P2P.ListenHandle != NULL)
    {
        vTaskDelete(p_Device->P2P.ListenHandle);
    }

    xTaskCreatePinnedToCore(RAK3172_P2P_ReceiveTask, "receiveTask", 2048, p_Device, Priority, &p_Device->P2P.ListenHandle, CoreID);
    if(p_Device->P2P.ListenHandle == NULL)
    {
        vQueueDelete(p_Device->P2P.ListenQueue);
        RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+PRECV=0"));

        return RAK3172_ERR_INVALID_STATE;
    }

    p_Device->P2P.isRxTimeout = false;
    p_Device->Internal.isBusy = true;

    return RAK3172_ERR_OK;
}

RAK3172_Error_t RAK3172_P2P_PopItem(const RAK3172_t* const p_Device, RAK3172_Rx_t* p_Message)
{
    uint8_t Items;
    RAK3172_Rx_t* FromQueue;

    if((p_Device == NULL) || (p_Message == NULL))
    {
        return RAK3172_ERR_INVALID_ARG;
    }
    else if((p_Device->P2P.ListenHandle == NULL) || (p_Device->P2P.ListenQueue == NULL))
    {
        return RAK3172_ERR_INVALID_STATE;
    }

    Items = uxQueueMessagesWaiting(p_Device->P2P.ListenQueue);
    ESP_LOGD(TAG, "Items in queue: %u", Items);

    if(Items == 0)
    {
        return RAK3172_ERR_FAIL;
    }

    if(xQueueReceive(p_Device->P2P.ListenQueue, &FromQueue, 0) != pdPASS)
    {
        return RAK3172_ERR_FAIL;
    }

    *p_Message = *FromQueue;

    delete FromQueue;

    return RAK3172_ERR_OK;
}

RAK3172_Error_t RAK3172_P2P_Stop(RAK3172_t* const p_Device)
{
    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+PRECV=" + std::to_string(RAK_REC_STOP)));

    p_Device->P2P.Active = false;
    p_Device->Internal.isBusy = false;

    if(p_Device->P2P.ListenHandle != NULL)
    {
        vTaskSuspend(p_Device->P2P.ListenHandle);
        vTaskDelete(p_Device->P2P.ListenHandle);
    }

    return RAK3172_ERR_OK;
}

bool RAK3172_P2P_isListening(const RAK3172_t* const p_Device)
{
    if(p_Device == NULL)
    {
        return false;
    }

    return (p_Device->P2P.isRxTimeout == false);
}

#endif