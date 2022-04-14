 /*
 * rak3172_p2p.h
 *
 *  Copyright (C) Daniel Kampert, 2022
 *	Website: www.kampis-elektroecke.de
 *  File info: LoRa P2P module of the RAK3172 driver for ESP32.

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
#include <esp_sleep.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <freertos/queue.h>

#include "../include/rak3172.h"

static const char* TAG = "RAK3172_P2P";

/** @brief          LoRa P2P receive task.
 *  @param p_Arg    Pointer to task arguments
 */
static void receiveTask(void* p_Arg)
{
    RAK3172_t* Device = (RAK3172_t*)p_Arg;

    Device->P2P.Active = true;

    while(Device->P2P.Active)
    {
        std::string* Meta;

        if(xQueueReceive(Device->Internal.Rx_Queue, &Meta, portMAX_DELAY) == pdPASS)
        {
            // The RSSI and SNR value indicates the start of the receive frame.
            if(Meta->find("RXP2P") != std::string::npos)
            {
                int Index;
                std::string* Payload;
                RAK3172_Rx_t* Message = new RAK3172_Rx_t();

                if(Message != NULL)
                {
                    // Get the RSSI value.
                    Index = Meta->find(",");
                    Message->RSSI = std::stoi(Meta->substr(Index + 7, Index + Meta->find(",", Index) + 1));

                    // Get the SNR value.
                    Index = Meta->find_last_of(",");
                    Message->SNR = std::stoi(Meta->substr(Index + 6, Meta->length() - 1));

                    // The next line contains the data.
                    if(xQueueReceive(Device->Internal.Rx_Queue, &Payload, 100 / portTICK_PERIOD_MS) != pdPASS)
                    {
                        Device->Internal.isBusy = false;
                        Device->P2P.Active = false;
                        Device->P2P.Timeout = true;
                    }

                    Payload->erase(Payload->find("+EVT:"), std::string("+EVT:").length());
                    Message->Payload = *Payload;
                    delete Payload;

                    // Leave the task when a message was received successfully and when a single message should be received.
                    if(xQueueSend(*Device->P2P.Queue, &Message, portMAX_DELAY) == pdPASS)
                    {
                        // Only a single message should be received.
                        if(Device->P2P.Timeout != RAK_REC_REPEAT)
                        {
                            Device->Internal.isBusy = false;
                            Device->P2P.Active = false;
                        }
                    }
                }
            }
            // Receive timeout.
            else if(Meta->find("RECEIVE TIMEOUT") != std::string::npos)
            {
                Device->Internal.isBusy = false;
                Device->P2P.Active = false;
            }

            delete Meta;
        }
    }

    vTaskSuspend(NULL);
}

RAK3172_Error_t RAK3172_Init_P2P(RAK3172_t* p_Device, uint32_t Frequency, RAK3172_PSF_t Spread, RAK3172_BW_t Bandwidth, RAK3172_CR_t CodeRate, uint16_t Preamble, uint8_t Power, uint32_t Timeout)
{
    std::string Value;

    if((p_Device == NULL) || (Frequency > 960000000) || (Frequency < 150000000) || (CodeRate > RAK_CR_3) || (CodeRate < RAK_CR_0) || (Preamble < 2) || (Power > 22) || (Power < 5))
    {
        return RAK3172_INVALID_ARG;
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

    ESP_LOGI(TAG, "     Use configuration: %s", Value.c_str());

    return RAK3172_SendCommand(p_Device, "AT+P2P=" + Value, NULL, NULL);
}

RAK3172_Error_t RAK3172_GetConfig(RAK3172_t* p_Device, std::string* p_Config)
{
    if(p_Config == NULL)
    {
        return RAK3172_INVALID_ARG;
    }

    return RAK3172_SendCommand(p_Device, "AT+P2P=?", p_Config, NULL);
}

RAK3172_Error_t RAK3172_SetFrequency(RAK3172_t* p_Device, uint32_t Freq)
{
    if((Freq > 960000000) || (Freq < 150000000))
    {
        return RAK3172_INVALID_ARG;
    }

    return RAK3172_SendCommand(p_Device, "AT+PFREQ=" + std::to_string(Freq), NULL, NULL);
}

RAK3172_Error_t RAK3172_GetFrequency(RAK3172_t* p_Device, uint32_t* p_Freq)
{
    std::string Value;

    if(p_Freq == NULL)
    {
        return RAK3172_INVALID_ARG;
    }

    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+PFREQ=?", &Value, NULL));

    *p_Freq = std::stoi(Value);

    return RAK3172_OK;
}

RAK3172_Error_t RAK3172_SetSpreading(RAK3172_t* p_Device, RAK3172_PSF_t Spread)
{
    if((Spread > RAK_PSF_12) || (Spread < RAK_PSF_6))
    {
        return RAK3172_INVALID_ARG;
    }

    return RAK3172_SendCommand(p_Device, "AT+PSF=" + std::to_string(Spread), NULL, NULL);
}

RAK3172_Error_t RAK3172_GetSpreading(RAK3172_t* p_Device, RAK3172_PSF_t* p_Spread)
{
    std::string Value;

    if(p_Spread == NULL)
    {
        return RAK3172_INVALID_ARG;
    }

    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+PSF=?", &Value, NULL));

    *p_Spread = (RAK3172_PSF_t)std::stoi(Value);

    return RAK3172_OK;
}

RAK3172_Error_t RAK3172_SetBandwidth(RAK3172_t* p_Device, RAK3172_BW_t Bandwidth)
{
    if((Bandwidth != RAK_BW_125) && (Bandwidth != RAK_BW_250) && (Bandwidth != RAK_BW_500))
    {
        return RAK3172_INVALID_ARG;
    }

    return RAK3172_SendCommand(p_Device, "AT+PBW=" + std::to_string(Bandwidth), NULL, NULL);
}

RAK3172_Error_t RAK3172_GetBandwidth(RAK3172_t* p_Device, RAK3172_BW_t* p_Bandwidth)
{
    std::string Value;

    if(p_Bandwidth == NULL)
    {
        return RAK3172_INVALID_ARG;
    }

    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+PBW=?", &Value, NULL));

    *p_Bandwidth = (RAK3172_BW_t)std::stoi(Value);

    return RAK3172_OK;
}

RAK3172_Error_t RAK3172_SetCodeRate(RAK3172_t* p_Device, RAK3172_CR_t CodeRate)
{
    if((CodeRate > RAK_CR_3) || (CodeRate < RAK_CR_0))
    {
        return RAK3172_INVALID_ARG;
    }

    return RAK3172_SendCommand(p_Device, "AT+PCR=" + std::to_string(CodeRate), NULL, NULL);  
}

RAK3172_Error_t RAK3172_GetCodeRate(RAK3172_t* p_Device, RAK3172_CR_t* p_CodeRate)
{
    std::string Value;

    if(p_CodeRate == NULL)
    {
        return RAK3172_INVALID_ARG;
    }

    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+PCR=?", &Value, NULL));

    *p_CodeRate = (RAK3172_CR_t)std::stoi(Value);

    return RAK3172_OK;
}

RAK3172_Error_t RAK3172_SetPreamble(RAK3172_t* p_Device, uint16_t Preamble)
{
    if(Preamble < 2)
    {
        return RAK3172_INVALID_ARG;
    }

    return RAK3172_SendCommand(p_Device, "AT+PPL=" + std::to_string(Preamble), NULL, NULL);  
}

RAK3172_Error_t RAK3172_GetPreamble(RAK3172_t* p_Device, uint16_t* p_Preamble)
{
    std::string Value;

    if(p_Preamble == NULL)
    {
        return RAK3172_INVALID_ARG;
    }

    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+PPL=?", &Value, NULL));

    *p_Preamble = (uint16_t)std::stoi(Value);

    return RAK3172_OK;
}

RAK3172_Error_t RAK3172_SetPower(RAK3172_t* p_Device, uint8_t Power)
{
    if((Power > 22) || (Power < 5))
    {
        return RAK3172_INVALID_ARG;
    }

    return RAK3172_SendCommand(p_Device, "AT+PTP=" + std::to_string(Power), NULL, NULL);  
}

RAK3172_Error_t RAK3172_GetPower(RAK3172_t* p_Device, uint8_t* p_Power)
{
    std::string Value;

    if(p_Power == NULL)
    {
        return RAK3172_INVALID_ARG;
    }

    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+PTP=?", &Value, NULL));

    *p_Power = (uint8_t)std::stoi(Value);

    return RAK3172_OK;
}

RAK3172_Error_t RAK3172_P2P_Transmit(RAK3172_t* p_Device, const uint8_t* p_Buffer, uint8_t Length)
{
    char Buffer[3];
    std::string Payload;

    if((p_Buffer == NULL) && (Length > 0))
    {
        return RAK3172_INVALID_ARG;
    }
    else if(Length == 0)
    {
        return RAK3172_OK;
    }

    // Encode the payload into an ASCII string.
    for(uint8_t i = 0; i < Length; i++)
    {
        sprintf(Buffer, "%02x", ((uint8_t*)p_Buffer)[i]);
        Payload += std::string(Buffer);
    }

    return RAK3172_SendCommand(p_Device, "AT+PSEND=" + Payload, NULL, NULL);
}

RAK3172_Error_t RAK3172_P2P_Receive(RAK3172_t* p_Device, uint16_t Timeout, std::string* p_Payload, int8_t* p_RSSI, int8_t* p_SNR, uint32_t Listen)
{
    RAK3172_Error_t Error;

    if((p_Payload == NULL) || (Timeout > 65534))
    {
        return RAK3172_INVALID_ARG;
    }

    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+PRECV=" + std::to_string(Timeout), NULL, NULL));

    do
    {
        std::string* Meta;

        if(xQueueReceive(p_Device->Internal.Rx_Queue, &Meta, Listen / portTICK_PERIOD_MS) == pdPASS)
        {
            int Index;
            std::string* Payload;

            // The RSSI and SNR value indicates the start of the receive frame.
            if(Meta->find("RXP2P") != std::string::npos)
            {
                std::string Dummy;

                // Get the RSSI value.
                if(p_RSSI != NULL)
                {
                    Index = Meta->find(",");
                    Dummy = Meta->substr(Index + 7, Index + Meta->find(",", Index) + 1);
                    *p_RSSI = std::stoi(Dummy);
                }

                // Get the SNR value.
                if(p_SNR != NULL)
                {
                    Index = Meta->find_last_of(",");
                    Dummy = Meta->substr(Index + 6, Meta->length() - 1);
                    *p_SNR = std::stoi(Dummy);
                }

                // The next line contains the data.
                if(xQueueReceive(p_Device->Internal.Rx_Queue, &Payload, 100 / portTICK_PERIOD_MS) != pdPASS)
                {
                    return RAK3172_TIMEOUT;
                }

                Payload->replace(Payload->begin(), Payload->end(), "+EVT:", "");
                *p_Payload = *Payload;
                delete Payload;

                return RAK3172_OK;
            }
            // Receive timeout.
            else if(Meta->find("RECEIVE TIMEOUT") != std::string::npos)
            {
                return RAK3172_TIMEOUT;
            }

            delete Meta;
        }

        vTaskDelay(10 / portTICK_PERIOD_MS);
    } while(true);
}

RAK3172_Error_t RAK3172_P2P_Listen(RAK3172_t* p_Device, QueueHandle_t* p_Queue, uint16_t Timeout)
{
    std::string Version;

    if(p_Queue == NULL)
    {
        return RAK3172_INVALID_ARG;
    }

    p_Device->P2P.Timeout = Timeout;

    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+PRECV=" + std::to_string(p_Device->P2P.Timeout), NULL, NULL));

    p_Device->P2P.Queue = p_Queue;

    if(p_Device->P2P.Handle != NULL)
    {
        vTaskDelete(p_Device->P2P.Handle);
    }

    xTaskCreatePinnedToCore(receiveTask, "receiveTask", 2048, p_Device, 1, &p_Device->P2P.Handle, 1);
    if(p_Device->P2P.Handle == NULL)
    {
        return RAK3172_INVALID_STATE;
    }

    p_Device->Internal.isBusy = true;

    return RAK3172_OK;
}

RAK3172_Error_t RAK3172_P2P_Stop(RAK3172_t* p_Device)
{
    RAK3172_ERROR_CHECK(RAK3172_SendCommand(p_Device, "AT+PRECV=" + std::to_string(RAK_REC_STOP), NULL, NULL));

    if(p_Device->P2P.Handle != NULL)
    {
        vTaskDelete(p_Device->P2P.Handle);
    }   

    p_Device->Internal.isBusy = false;

    return RAK3172_OK;
}

bool RAK3172_P2P_isListening(RAK3172_t* p_Device)
{
    if(p_Device == NULL)
    {
        return false;
    }

    return p_Device->Internal.isBusy;
}
