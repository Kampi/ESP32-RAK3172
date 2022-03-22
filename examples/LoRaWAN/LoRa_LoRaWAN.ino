#include <Arduino.h>

#include <rak3172.h>

#include "LoRaWAN_Default.h"

static RAK3172_t Device;
static QueueHandle_t Queue;

void setup(void)
{
    esp_err_t Error;

    Device.p_Interface = &Serial1;
    Device.Rx = LORAWAN_RX;
    Device.Tx = LORAWAN_TX;
    Device.Baudrate = RAK_BAUD_9600;

    Serial.begin(115200);

    if(RAK3172_Init_LoRaWAN(&Device, 16, 3, RAK_JOIN_OTAA, (uint8_t*)DEVEUI, (uint8_t*)APPEUI, (uint8_t*)APPKEY, 'A', RAK_BAND_EU868, RAK_SUB_BAND_NONE) == ESP_OK)
    {
    }
    else
    {
        Serial.println("[ERROR] Can not initialize LoRa module in LoRaWAN mode!");

        return;
    }

}

void loop(void)
{
    bool isJoined;

    if(RAK3172_Joined(&Device, &isJoined))
    {
        Serial.println("[ERROR] Can not read LoRa module!");
    }

    if(!isJoined)
    {
        Serial.println("[INFO] Not joined. Rejoin...");

        if(RAK3172_StartJoin(&Device, 0, LORAWAN_JOIN_ATTEMPTS, true, LORAWAN_MAX_JOIN_INTERVAL_S, NULL) != ESP_OK)
        {
            Serial.println("[ERROR] Can not join LoRaWAN network!");

            RAK3172_StopJoin(&Device);
        }
        else
        {
            String Payload;

            Payload = "{}";

            if(RAK3172_LoRaWAN_Transmit(&Device, 1, Payload.c_str(), Payload.length(), LORAWAN_TX_TIMEOUT_S, true, NULL) != ESP_OK)
            {
                Serial.println("[ERROR] Can not transmit LoRa message!");
            }
            else
            {
                int SNR;
                int RSSI;
                esp_err_t Error;

                Serial.println("[INFO] Message transmitted...");

                Error = RAK3172_LoRaWAN_Receive(&Device, &Payload, &RSSI, &SNR, LORAWAN_RX_TIMEOUT_S);
                if(Error == ESP_OK)
                {
                    Serial.println("[INFO] Message received...");
                }
                else if(Error == ESP_ERR_TIMEOUT)
                {
                    Serial.println("[INFO] No message received...");
                }
                else if(Error == ESP_ERR_INVALID_STATE)
                {
                    Serial.println("[ERROR] Device not joined!");
                }
            }
        }
    }

    delay(1000);
}