#include <esp_log.h>
#include <esp_task_wdt.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>

#include <rak3172.h>

#include "settings/LoRaWAN_Default.h"

static RAK3172_t _Device                        = RAK3172_DEFAULT_CONFIG(CONFIG_RAK3172_UART_PORT, CONFIG_RAK3172_UART_RX, CONFIG_RAK3172_UART_TX, CONFIG_RAK3172_UART_BAUD);
static RAK3172_MC_Group_t _Group = {
    .Class          = RAK_CLASS_C,
    .DevAddr        = "B8DD8FDB",
    .NwkSKey        = "C8986573F2C9B6A0096D94E25F8A56D6",
    .AppSKey        = "3E9F9E58574AB8A58B027B688364FE82",
    .Datarate       = RAK_DR_0,
    .Frequency      = 868000000,
    .Periodicity    = 0
};

static StackType_t _applicationStack[8192];

static StaticTask_t _applicationBuffer;

static TaskHandle_t _applicationHandle;

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

    Error = RAK3172_LoRaWAN_Init(_Device, 16, RAK_JOIN_OTAA, DEVEUI, APPEUI, APPKEY, RAK_CLASS_A, RAK_BAND_EU868, RAK_SUB_BAND_NONE);
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

    while(true)
    {
        struct tm Now;

        ESP_LOGI(TAG, "Request clock sync...");

        Error = RAK3172_LoRaWAN_Clock_SetLocalTime(_Device, &Now, true, &_Group);
        if(Error == RAK3172_ERR_OK)
        {
            ESP_LOGI(TAG, "Clock sync done...");
            ESP_LOGI(TAG, "Time since 01/01/1970");
            ESP_LOGI(TAG, " Seconds: %u", static_cast<unsigned int>(Now.tm_sec));
            ESP_LOGI(TAG, " Minutes: %u", static_cast<unsigned int>(Now.tm_min));
            ESP_LOGI(TAG, " Hours: %u", static_cast<unsigned int>(Now.tm_hour));
            ESP_LOGI(TAG, " Day of week: %u", static_cast<unsigned int>(Now.tm_wday));
            ESP_LOGI(TAG, " Day: %u", static_cast<unsigned int>(Now.tm_mday));
            ESP_LOGI(TAG, " Month: %u", static_cast<unsigned int>(Now.tm_mon));
            ESP_LOGI(TAG, " Year: %u", static_cast<unsigned int>(Now.tm_year));
        }
        else
        {
            ESP_LOGE(TAG, "Clock sync failed! Error: 0x%X", static_cast<unsigned int>(Error));
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