#include <esp_log.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>

#include <rak3172.h>

#include "settings/LoRaWAN_Default.h"

static RAK3172_t _Device                        = RAK3172_DEFAULT_CONFIG(CONFIG_RAK3172_UART_PORT, CONFIG_RAK3172_UART_RX, CONFIG_RAK3172_UART_TX, CONFIG_RAK3172_UART_BAUD, CONFIG_RAK3172_RESET_PIN, false);

static StackType_t _applicationStack[8192];

static StaticTask_t _applicationBuffer;

static TaskHandle_t _applicationHandle;

static const char Payload[]                     = {'{', '}'};

static const char* TAG 							= "main";

static void applicationTask(void* p_Parameter)
{
    bool Status = false;
    RAK3172_Error_t Error;
    RAK3172_Info_t Info;

    _Device.Info = &Info;

    Error = RAK3172_Init(&_Device);
    if(Error != RAK3172_ERR_OK)
    {
        ESP_LOGE(TAG, "Can not initialize RAK3172! Error: 0x%04X", Error);
    }

    ESP_LOGI(TAG, "Firmware: %s", Info.Firmware.c_str());
    ESP_LOGI(TAG, "Serial number: %s", Info.Serial.c_str());
    ESP_LOGI(TAG, "Current mode: %u", _Device.Mode);

    Error = RAK3172_LoRaWAN_Init(&_Device, 16, 3, RAK_JOIN_OTAA, DEVEUI, APPEUI, APPKEY, 'A', RAK_BAND_EU868, RAK_SUB_BAND_NONE);
    if(Error != RAK3172_ERR_OK)
    {
        ESP_LOGE(TAG, "Can not initialize RAK3172 LoRaWAN! Error: 0x%04X", Error);
    }

    Error = RAK3172_LoRaWAN_isJoined(&_Device, &Status);
    if(Error != RAK3172_ERR_OK)
    {
        ESP_LOGE(TAG, "Error: 0x%04X", Error);
    }

    if(Status == false)
    {
        ESP_LOGI(TAG, "Not joined. Rejoin...");

        Error = RAK3172_LoRaWAN_StartJoin(&_Device, RAK3172_NO_TIMEOUT, LORAWAN_JOIN_ATTEMPTS, false, LORAWAN_MAX_JOIN_INTERVAL_S, NULL);
        if(Error != RAK3172_ERR_OK)
        {
            ESP_LOGE(TAG, "Can not join network! Error: 0x%04X", Error);
        }
    }
    else
    {
        ESP_LOGI(TAG, "Joined...");
    }

    while(true)
    {
        Error = RAK3172_LoRaWAN_Transmit(&_Device, 1, Payload, sizeof(Payload), true);
        if(Error == RAK3172_ERR_INVALID_RESPONSE)
        {
            ESP_LOGE(TAG, "Can not transmit message! Error: 0x%04X", Error);
        }
        else
        {
            RAK3172_Rx_t Message;

            ESP_LOGI(TAG, "Message transmitted...");
            Error = RAK3172_LoRaWAN_Receive(&_Device, &Message);
            if(Error != RAK3172_ERR_OK)
            {
                ESP_LOGE(TAG, "Can not receive message! Error: 0x%04X", Error);
            }
            else
            {
                ESP_LOGI(TAG, " RSSI: %i", Message.RSSI);
                ESP_LOGI(TAG, " SNR: %i", Message.SNR);
                ESP_LOGI(TAG, " Port: %u", Message.Port);
                ESP_LOGI(TAG, " Payload: %s", Message.Payload.c_str());
            }
        }

        vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
}

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "Starting application.");

    _applicationHandle = xTaskCreateStatic(applicationTask, "applicationTask", sizeof(_applicationStack), NULL, 1, _applicationStack, &_applicationBuffer);
    if(_applicationHandle == NULL)
    {
        ESP_LOGE(TAG, "    Unable to create application task!");

        esp_restart();
    }
}