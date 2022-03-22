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

#include <esp_sleep.h>
#include <driver/uart.h>
#include <esp32-hal-log.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>

#include "../include/rak3172.h"

/** @brief Receive task parameter object, initialized by the RAK3172_P2P_Listen function.
 */
struct ReceiveParams_t {
    RAK3172_t* Device;                      /**< Pointer to RAK3172 device. */
    QueueHandle_t* Queue;                   /**< Pointer to message queue. */
    bool Active;                            /**< Task active. */
    uint16_t Timeout;                       /**< Receive timeout */
};

static ReceiveParams_t _Params;

static TaskHandle_t _receiveTaskHandle;

static const char* TAG = "RAK3172_P2P";

static void receiveTask(void* p_Parameter)
{
    String Line;
    ReceiveParams_t* Params = (ReceiveParams_t*)p_Parameter;

    Params->Active = true;

    while(Params->Active)
    {
        Line = Params->Device->p_Interface->readStringUntil('\n');

        if(Line.length() > 0)
        {
            // The RSSI and SNR value indicates the start of the receive frame.
            if(Line.indexOf("RXP2P") != -1)
            {
                int Index;
                String Dummy;
                RAK3172_Rx_t Message;

                // Get the RSSI value.
                Index = Line.indexOf(",");
                Dummy = Line.substring(Index + 7, Index + Line.indexOf(",", Index) + 1);
                Message.RSSI = Dummy.toInt();

                // Get the SNR value.
                Index = Line.lastIndexOf(",");
                Dummy = Line.substring(Index + 6, Line.length() - 1);
                Message.SNR = Dummy.toInt();

                // The next line contains the data.
                Line = Params->Device->p_Interface->readStringUntil('\n');
                Line.replace("+EVT:", "");
                Message.Payload = Line;

                // Leave the task when a message was received successfully and when a single message should be received.
                if(xQueueSend(*Params->Queue, &Message, portMAX_DELAY) == pdPASS)
                {
                    // Only a single message should be received.
                    if(Params->Timeout != RAK_REC_REPEAT)
                    {
                        Params->Device->isBusy = false;
                        Params->Active = false;
                    }
                }
            }
            // Receive timeout.
            else if(Line.indexOf("RECEIVE TIMEOUT") != -1)
            {
                Params->Device->isBusy = false;
                Params->Active = false;
            }
        }

        delay(10);
    }

    vTaskSuspend(NULL);
}

esp_err_t RAK3172_Init_P2P(RAK3172_t* p_Device, uint32_t Frequency, RAK3172_PSF_t Spread, RAK3172_BW_t Bandwidth, RAK3172_CR_t CodeRate, uint16_t Preamble, uint8_t Power, uint32_t Timeout)
{
    String Configuration;
    esp_err_t Error;

    if((p_Device == NULL) || (Frequency > 960000000) || (Frequency < 150000000) || (CodeRate > RAK_CR_3) || (CodeRate < RAK_CR_0) || (Preamble < 2) || (Power > 22) || (Power < 5))
    {
        return ESP_ERR_INVALID_ARG;
    }

    Configuration = String(Frequency) + ":" + String(Spread) + ":" + String(Bandwidth) + ":" + String(CodeRate) + ":" + String(Preamble) + ":" + String(Power);

    ESP_LOGI(TAG, "Initialize module in P2P mode...");

    Error = RAK3172_SetMode(p_Device, RAK_MODE_P2P) || RAK3172_SoftReset(p_Device, Timeout);
    if(Error)
    {
        return Error;
    }

    p_Device->isBusy = false;

    ESP_LOGI(TAG, "     Use configuration: %s", Configuration.c_str());

    return RAK3172_SendCommand(p_Device, "AT+P2P=" + Configuration, NULL, NULL);
}

esp_err_t RAK3172_GetConfig(RAK3172_t* p_Device, String* p_Config)
{
    if(p_Config == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    return RAK3172_SendCommand(p_Device, "AT+P2P=?", p_Config, NULL);
}

esp_err_t RAK3172_SetFrequency(RAK3172_t* p_Device, uint32_t Freq)
{
    if((Freq > 960000000) || (Freq < 150000000))
    {
        return ESP_ERR_INVALID_ARG;
    }

    return RAK3172_SendCommand(p_Device, "AT+PFREQ=" + String(Freq), NULL, NULL);
}

esp_err_t RAK3172_GetFrequency(RAK3172_t* p_Device, uint32_t* p_Freq)
{
    String Value;
    esp_err_t Error;

    if(p_Freq == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    Error = RAK3172_SendCommand(p_Device, "AT+PFREQ=?", &Value, NULL);
    if(Error)
    {
        return Error;
    }

    *p_Freq = Value.toInt();

    return Error;
}

esp_err_t RAK3172_SetSpreading(RAK3172_t* p_Device, RAK3172_PSF_t Spread)
{
    if((Spread > RAK_PSF_12) || (Spread < RAK_PSF_6))
    {
        return ESP_ERR_INVALID_ARG;
    }

    return RAK3172_SendCommand(p_Device, "AT+PSF=" + String(Spread), NULL, NULL);
}

esp_err_t RAK3172_GetSpreading(RAK3172_t* p_Device, RAK3172_PSF_t* p_Spread)
{
    String Value;
    esp_err_t Error;

    if(p_Spread == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    Error = RAK3172_SendCommand(p_Device, "AT+PSF=?", &Value, NULL);
    if(Error)
    {
        return Error;
    }

    *p_Spread = (RAK3172_PSF_t)Value.toInt();

    return Error;
}

esp_err_t RAK3172_SetBandwidth(RAK3172_t* p_Device, RAK3172_BW_t Bandwidth)
{
    if((Bandwidth != RAK_BW_125) && (Bandwidth != RAK_BW_250) && (Bandwidth != RAK_BW_500))
    {
        return ESP_ERR_INVALID_ARG;
    }

    return RAK3172_SendCommand(p_Device, "AT+PBW=" + String(Bandwidth), NULL, NULL);
}

esp_err_t RAK3172_GetBandwidth(RAK3172_t* p_Device, RAK3172_BW_t* p_Bandwidth)
{
    String Value;
    esp_err_t Error;

    if(p_Bandwidth == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    Error = RAK3172_SendCommand(p_Device, "AT+PBW=?", &Value, NULL);
    if(Error)
    {
        return Error;
    }

    *p_Bandwidth = (RAK3172_BW_t)Value.toInt();

    return Error;
}

esp_err_t RAK3172_SetCodeRate(RAK3172_t* p_Device, RAK3172_CR_t CodeRate)
{
    if((CodeRate > RAK_CR_3) || (CodeRate < RAK_CR_0))
    {
        return ESP_ERR_INVALID_ARG;
    }

    return RAK3172_SendCommand(p_Device, "AT+PCR=" + String(CodeRate), NULL, NULL);  
}

esp_err_t RAK3172_GetCodeRate(RAK3172_t* p_Device, RAK3172_CR_t* p_CodeRate)
{
    String Value;
    esp_err_t Error;

    if(p_CodeRate == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    Error = RAK3172_SendCommand(p_Device, "AT+PCR=?", &Value, NULL);
    if(Error)
    {
        return Error;
    }

    *p_CodeRate = (RAK3172_CR_t)Value.toInt();

    return Error;
}

esp_err_t RAK3172_SetPreamble(RAK3172_t* p_Device, uint16_t Preamble)
{
    if(Preamble < 2)
    {
        return ESP_ERR_INVALID_ARG;
    }

    return RAK3172_SendCommand(p_Device, "AT+PPL=" + String(Preamble), NULL, NULL);  
}

esp_err_t RAK3172_GetPreamble(RAK3172_t* p_Device, uint16_t* p_Preamble)
{
    String Value;
    esp_err_t Error;

    if(p_Preamble == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    Error = RAK3172_SendCommand(p_Device, "AT+PPL=?", &Value, NULL);
    if(Error)
    {
        return Error;
    }

    *p_Preamble = (uint16_t)Value.toInt();

    return Error;
}

esp_err_t RAK3172_SetPower(RAK3172_t* p_Device, uint8_t Power)
{
    if((Power > 22) || (Power < 5))
    {
        return ESP_ERR_INVALID_ARG;
    }

    return RAK3172_SendCommand(p_Device, "AT+PTP=" + String(Power), NULL, NULL);  
}

esp_err_t RAK3172_GetPower(RAK3172_t* p_Device, uint8_t* p_Power)
{
    String Value;
    esp_err_t Error;

    if(p_Power == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    Error = RAK3172_SendCommand(p_Device, "AT+PTP=?", &Value, NULL);
    if(Error)
    {
        return Error;
    }

    *p_Power = (uint8_t)Value.toInt();

    return Error;
}

esp_err_t RAK3172_P2P_Transmit(RAK3172_t* p_Device, const uint8_t* p_Buffer, uint8_t Length)
{
    String Payload;
    char Buffer[3];

    if((p_Buffer == NULL) && (Length > 0))
    {
        return ESP_ERR_INVALID_ARG;
    }
    else if(Length == 0)
    {
        return ESP_OK;
    }

    // Encode the payload into an ASCII string.
    for(uint8_t i = 0x00; i < Length; i++)
    {
        sprintf(Buffer, "%02x", ((uint8_t*)p_Buffer)[i]);
        Payload += String(Buffer);
    }

    return RAK3172_SendCommand(p_Device, "AT+PSEND=" + Payload, NULL, NULL);
}

esp_err_t RAK3172_P2P_Receive(RAK3172_t* p_Device, uint16_t Timeout, String* p_Payload, int8_t* p_RSSI, int8_t* p_SNR)
{
    String Line;
    esp_err_t Error;

    if((p_Payload == NULL) || (Timeout > 65534))
    {
        return ESP_ERR_INVALID_ARG;
    }

    Error = RAK3172_SendCommand(p_Device, "AT+PRECV=" + String(Timeout), NULL, NULL);
    if(Error)
    {
        return Error;
    }

    do
    {
        Line = p_Device->p_Interface->readStringUntil('\n');

        if(Line.length() > 0)
        {
            int Index;

            // The RSSI and SNR value indicates the start of the receive frame.
            if(Line.indexOf("RXP2P") != -1)
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

                // The next line contains the data.
                Line = p_Device->p_Interface->readStringUntil('\n');
                Line.replace("+EVT:", "");
                *p_Payload = Line;

                return ESP_OK;
            }
            // Receive timeout.
            else if(Line.indexOf("RECEIVE TIMEOUT") != -1)
            {
                return ESP_ERR_TIMEOUT;
            }
        }

        delay(10);
    } while(true);
}

esp_err_t RAK3172_P2P_Listen(RAK3172_t* p_Device, QueueHandle_t* p_Queue, uint16_t Timeout)
{
    String Version;
    esp_err_t Error;

    if(p_Queue == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    Error = RAK3172_GetFWVersion(p_Device, &Version);
    if(Error)
    {
        return Error;
    }

    _Params.Timeout = Timeout;

    Error = RAK3172_SendCommand(p_Device, "AT+PRECV=" + String(_Params.Timeout), NULL, NULL);
    if(Error)
    {
        return Error;
    }

    // Save the parameter for the event task.
    _Params.Device = p_Device;
    _Params.Queue = p_Queue;

    if(_receiveTaskHandle != NULL)
    {
        vTaskDelete(_receiveTaskHandle);
    }

    xTaskCreatePinnedToCore(receiveTask, "receiveTask", 4096, &_Params, 1, &_receiveTaskHandle, 1);
    if(_receiveTaskHandle == NULL)
    {
        return ESP_FAIL;
    }

    p_Device->isBusy = true;

    return ESP_OK;
}

esp_err_t RAK3172_P2P_Stop(RAK3172_t* p_Device)
{
    esp_err_t Error;

    Error = RAK3172_SendCommand(p_Device, "AT+PRECV=" + String(RAK_REC_STOP), NULL, NULL);
    if(Error)
    {
        return Error;
    }

    if(_receiveTaskHandle != NULL)
    {
        vTaskDelete(_receiveTaskHandle);
    }   

    p_Device->isBusy = false;

    return Error;
}

bool RAK3172_P2P_isListening(RAK3172_t* p_Device)
{
    return p_Device->isBusy;
}
