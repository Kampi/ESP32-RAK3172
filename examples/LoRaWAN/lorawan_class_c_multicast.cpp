#include <esp_log.h>
#include <esp_task_wdt.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>

#include <rak3172.h>

#include "settings/LoRaWAN_Default.h"

static RAK3172_t _Device                        = RAK3172_DEFAULT_CONFIG(CONFIG_RAK3172_UART_PORT, CONFIG_RAK3172_UART_RX, CONFIG_RAK3172_UART_TX, CONFIG_RAK3172_UART_BAUD);

static StackType_t _applicationStack[8192];

static StaticTask_t _applicationBuffer;

static TaskHandle_t _applicationHandle;

static const char Payload[]                     = {'{', '}'};

static const char* TAG 							= "main";

static void applicationTask(void* p_Parameter)
{
    RAK3172_Error_t Error;
    RAK3172_Info_t Info;

    _Device.Info = &Info;

    Error = RAK3172_Init(_Device);
    if(Error != RAK3172_ERR_OK)
    {
        ESP_LOGE(TAG, "Cannot initialize RAK3172! Error: 0x%04X", static_cast<unsigned int>(Error));
    }

    ESP_LOGI(TAG, "Firmware: %s", Info.Firmware.c_str());
    ESP_LOGI(TAG, "Serial number: %s", Info.Serial.c_str());
    ESP_LOGI(TAG, "Current mode: %u", _Device.Mode);

    Error = RAK3172_LoRaWAN_Init(_Device, 16, RAK_JOIN_OTAA, DEVEUI, APPEUI, APPKEY, RAK_CLASS_C, RAK_BAND_EU868, RAK_SUB_BAND_NONE);
    if(Error != RAK3172_ERR_OK)
    {
        ESP_LOGE(TAG, "Cannot initialize RAK3172 LoRaWAN! Error: 0x%04X", static_cast<unsigned int>(Error));
    }

    if(RAK3172_LoRaWAN_isJoined(_Device) == false)
    {
        ESP_LOGI(TAG, "Not joined. Rejoin...");

        Error = RAK3172_LoRaWAN_StartJoin(_Device, LORAWAN_JOIN_ATTEMPTS, RAK3172_NO_TIMEOUT, true, false, LORAWAN_MAX_JOIN_INTERVAL_S, NULL);
        if(Error != RAK3172_ERR_OK)
        {
            ESP_LOGE(TAG, "Cannot join network! Error: 0x%04X", static_cast<unsigned int>(Error));
        }
    }
    else
    {
        ESP_LOGI(TAG, "Joined...");
    }

    Error = RAK3172_LoRaWAN_MC_AddGroup(_Device, RAK_CLASS_C, "B8DD8FDB", "C8986573F2C9B6A0096D94E25F8A56D6", "3E9F9E58574AB8A58B027B688364FE82", 868000000, RAK_DR_0, 0);
    if(Error != RAK3172_ERR_OK)
    {
        ESP_LOGE(TAG, "Cannot add device to multicast group! Error: 0x%04X", static_cast<unsigned int>(Error));
    }

    while(true)
    {
        RAK3172_Rx_t Message;

        esp_task_wdt_reset();

        Error = RAK3172_LoRaWAN_Receive(_Device, &Message);
        if(Error != RAK3172_ERR_OK)
        {
            ESP_LOGE(TAG, "Cannot receive message! Error: 0x%04X", static_cast<unsigned int>(Error));
        }
        else
        {
            ESP_LOGI(TAG, " RSSI: %i", Message.RSSI);
            ESP_LOGI(TAG, " SNR: %i", Message.SNR);
            ESP_LOGI(TAG, " Port: %u", Message.Port);
            ESP_LOGI(TAG, " Channel: %u", Message.Group);
            ESP_LOGI(TAG, " Multicast: %u", Message.isMulticast);
            ESP_LOGI(TAG, " Payload: %s", Message.Payload.c_str());
        }

        vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
}

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "Starting application...");

    _applicationHandle = xTaskCreateStatic(applicationTask, "applicationTask", sizeof(_applicationStack), NULL, 1, _applicationStack, &_applicationBuffer);
    if(_applicationHandle == NULL)
    {
        ESP_LOGE(TAG, "    Unable to create application task!");

        esp_restart();
    }
}