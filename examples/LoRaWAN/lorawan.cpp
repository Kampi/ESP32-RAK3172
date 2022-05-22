#include <esp_log.h>
#include <esp_task_wdt.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>

#include "rak3172.h"

#include "LoRaWAN_Default.h"

static RAK3172_t _Device = {
    .Interface = (uart_port_t)CONFIG_RAK3172_UART,
    .Rx = (gpio_num_t)CONFIG_RAK3172_RX,
    .Tx = (gpio_num_t)CONFIG_RAK3172_TX,
    .Baudrate = (RAK3172_Baud_t)CONFIG_RAK3172_BAUD,
};

static StackType_t _applicationStack[8192];

static StaticTask_t _applicationBuffer;

static TaskHandle_t _applicationHandle;

static const char* TAG 							= "main";

static void applicationTask(void* p_Parameter)
{
    bool Status;
    RAK3172_Error_t Error;

    Error = RAK3172_Init(&_Device);
    if(Error != RAK3172_OK)
    {
        ESP_LOGE(TAG, "Can not initialize RAK3172! Error: 0x%04X", Error);
    }

    ESP_LOGI(TAG, "Firmware: %s", _Device.Firmware.c_str());
    ESP_LOGI(TAG, "Serial number: %s", _Device.Serial.c_str());
    ESP_LOGI(TAG, "Current mode: %u", _Device.Mode);

    Error = RAK3172_Init_LoRaWAN(&_Device, 16, 3, RAK_JOIN_OTAA, DEVEUI, APPEUI, APPKEY, 'A', RAK_BAND_EU868, RAK_SUB_BAND_NONE);
    if(Error != RAK3172_OK)
    {
        ESP_LOGE(TAG, "Can not initialize RAK3172 LoRaWAN! Error: 0x%04X", Error);
    }

    Error = RAK3172_Joined(&_Device, &Status);
    if(Error != RAK3172_OK)
    {
        ESP_LOGE(TAG, "Error: 0x%04X", Error);
    }

    if(!Status)
    {
        ESP_LOGI(TAG, "Not joined. Rejoin...");

        Error = RAK3172_StartJoin(&_Device, 0, LORAWAN_JOIN_ATTEMPTS, true, LORAWAN_MAX_JOIN_INTERVAL_S, NULL);
        if(Error != RAK3172_OK)
        {
            ESP_LOGE(TAG, "Can not join network!");
        }
        else
        {
            ESP_LOGI(TAG, "Joined...");

            char Payload[] = {'{', '}'};

            Error = RAK3172_LoRaWAN_Transmit(&_Device, 1, Payload, sizeof(Payload), LORAWAN_TX_TIMEOUT_S, true, NULL);
            if(Error == RAK3172_INVALID_RESPONSE)
            {
                ESP_LOGE(TAG, "Can not transmit message network!");
            }
            else
            {
                ESP_LOGI(TAG, "Message transmitted...");
            }
        }
    }

    while(true)
    {
        esp_task_wdt_reset();

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

static void StartApplication(void)
{
    ESP_LOGI(TAG, "Starting application.");

    _applicationHandle = xTaskCreateStatic(applicationTask, "applicationTask", 8192, NULL, 1, _applicationStack, &_applicationBuffer);
    if(_applicationHandle == NULL)
    {
        ESP_LOGE(TAG, "    Unable to create application task!");

        esp_restart();
    }
}

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "IDF: %s", esp_get_idf_version());

	StartApplication();
}