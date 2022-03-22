#include <Arduino.h>

#include <rak3172.h>

#include "LoRaP2P_Default.h"

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

    if(RAK3172_Init_P2P(&Device, 868000000, RAK_PSF_9, RAK_BW_125, RAK_CR_0, 8, 15) == ESP_OK)
    {
        Queue = xQueueCreate(32, sizeof(RAK3172_Rx_t));
        RAK3172_P2P_Listen(&Device, &Queue);
    }
    else
    {
        Serial.println("[ERROR] Can not initialize LoRa module in P2P mode!");
    }

}

void loop(void)
{
    if(RAK3172_P2P_isListening(&Device))
    {
        RAK3172_Rx_t Message;

        if(xQueueReceive(Queue, &Message, 0) == pdTRUE)
        {
            Serial.println("[INFO] Received:");
            Serial.printf("    Payload: %s\n\r", Message.Payload.c_str());
            Serial.printf("    RSSI: %i\n\r", Message.RSSI);
            Serial.printf("    SNR: %i\n\r", Message.SNR);
        }
    }
    else
    {
        Serial.println("[INFO] Not listening...");
    }

    delay(100);
}