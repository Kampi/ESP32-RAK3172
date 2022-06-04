#include <esp_log.h>
#include <esp_task_wdt.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>

#include "rak3172.h"

static RAK3172_t _Device = {
    .Interface = (uart_port_t)CONFIG_RAK3172_UART,
    .Rx = (gpio_num_t)CONFIG_RAK3172_RX,
    .Tx = (gpio_num_t)CONFIG_RAK3172_TX,
    .Baudrate = (RAK3172_Baud_t)CONFIG_RAK3172_BAUD,
};

static QueueHandle_t _ListenQueue;

static StackType_t _applicationStack[8192];

static StaticTask_t _applicationBuffer;

static TaskHandle_t _applicationHandle;

static const char* TAG 							= "main";

static void applicationTask(void* p_Parameter)
{
    uint8_t Buffer[] = {1, 2, 3};
    RAK3172_Error_t Error;

    Error = RAK3172_Init(&_Device);
    if(Error != RAK3172_ERR_OK)
    {
        ESP_LOGE(TAG, "Can not initialize RAK3172! Error: 0x%04X", Error);
    }

    ESP_LOGI(TAG, "Firmware: %s", _Device.Firmware.c_str());
    ESP_LOGI(TAG, "Serial number: %s", _Device.Serial.c_str());
    ESP_LOGI(TAG, "Current mode: %u", _Device.Mode);

    Error = RAK3172_Init_P2P(&_Device, 868000000, RAK_PSF_9, RAK_BW_125, RAK_CR_0, 8, 15);
    if(Error != RAK3172_ERR_OK)
    {
        ESP_LOGE(TAG, "Can not initialize RAK3172 LoRa P2P! Error: 0x%04X", Error);
    }

    Error = RAK3172_P2P_Transmit(&_Device, Buffer, sizeof(Buffer) / sizeof(Buffer[0]));
    if(Error != RAK3172_ERR_OK)
    {
        ESP_LOGE(TAG, "Can not transmit LoRa message! Error: 0x%04X", Error);
    }

    vTaskDelay(1000 / portTICK_PERIOD_MS);

    _ListenQueue = xQueueCreate(16, sizeof(RAK3172_Rx_t));
    Error = RAK3172_P2P_Listen(&_Device, &_ListenQueue, RAK_REC_SINGLE);
    if(Error != RAK3172_ERR_OK)
    {
        ESP_LOGE(TAG, "Can not enter LoRa listening mode! Error: 0x%04X", Error);
    }

    while(true)
    {
        RAK3172_Rx_t* Obj;

        esp_task_wdt_reset();

        if(xQueueReceive(_ListenQueue, &Obj, 0) == pdTRUE)
        {
            ESP_LOGI(TAG, "Message received:");
            ESP_LOGI(TAG, "     RSSI: %i", Obj->RSSI);
            ESP_LOGI(TAG, "     SNR: %i", Obj->SNR);
            ESP_LOGI(TAG, "     Payload: %s", Obj->Payload.c_str());

            delete Obj;
        }

        if(RAK3172_P2P_isListening(&_Device))
        {
            ESP_LOGI(TAG, "Listening...");
        }
        else
        {
            ESP_LOGI(TAG, "Not listening...");
        }

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