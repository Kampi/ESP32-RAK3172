#include <Arduino.h>

#include <rak3172.h>

#include "LoRaP2P_Default.h"

static uint8_t Buffer[] = {1, 2, 3};
static RAK3172_t Device;
static QueueHandle_t Queue;

void setup(void)
{
    String Version;
    String SerialNumber;

    Device.p_Interface = &Serial1;
    Device.Rx = LORAWAN_RX;
    Device.Tx = LORAWAN_TX;
    Device.Baudrate = RAK_BAUD_9600;

    Serial.begin(115200);

    if(RAK3172_Init(&Device) != ESP_OK)
    {
        Serial.println("[ERROR] Can not initialize RAK3172!");

        return;
    }

    if(RAK3172_GetFWVersion(&Device, &Version))
    {
        Serial.println("[ERROR] Can not read firmware version!");

        return;
    }

    if(RAK3172_GetSerialNumber(&Device, &SerialNumber))
    {
        Serial.println("[ERROR] Can not read serial number!");

        return;
    }

    Serial.printf("     Using RAK3172 library version: %s\n\r", RAK3172_LibVersion().c_str());
    Serial.printf("     Using RAK3172 firmware version: %s\n\r", Version.c_str());
    Serial.printf("     Serial number: %s\n\r", SerialNumber.c_str());

    if(RAK3172_Init_P2P(&Device, 868000000, RAK_PSF_9, RAK_BW_125, RAK_CR_0, 8, 15) == ESP_OK)
    {
        Queue = xQueueCreate(32, sizeof(RAK3172_Rx_t));
        RAK3172_P2P_Listen(&Device, &Queue);
    }
    else
    {
        Serial.println("[ERROR] Can not initialize LoRa module in P2P mode!");

        return;
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

        Buffer[1]++;
        RAK3172_P2P_Transmit(&Device, Buffer, 2);
    }

    delay(1000);
}