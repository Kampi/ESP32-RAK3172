#include <esp_log.h>
#include <esp_task_wdt.h>
#include <esp32/rom/rtc.h>
#include <esp_sleep.h>
#include <esp_wifi.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>

#include "rak3172.h"

#include "settings/LoRaWAN_Default.h"

static RTC_NOINIT_ATTR RAK3172_t _Device;

static StackType_t _applicationStack[8192];

static StaticTask_t _applicationBuffer;

static TaskHandle_t _applicationHandle;

static const char* TAG 							= "main";

static void applicationTask(void* p_Parameter)
{
    if((rtc_get_reset_reason(0) == POWERON_RESET) && (rtc_get_reset_reason(1) == EXT_CPU_RESET))
    {
        RAK3172_Error_t Error;
        RAK3172_Info_t Info;

		_Device = RAK3172_DEFAULT_CONFIG(UART_NUM_1, GPIO_NUM_12, GPIO_NUM_14, 9600);
        _Device.Info = &Info;

        Error = RAK3172_Init(_Device);
        if(Error != RAK3172_ERR_OK)
        {
            ESP_LOGE(TAG, "Cannot initialize RAK3172! Error: 0x%04X", Error);
        }

        ESP_LOGI(TAG, "Firmware: %s", Info.Firmware.c_str());
        ESP_LOGI(TAG, "Serial number: %s", Info.Serial.c_str());
        ESP_LOGI(TAG, "Current mode: %u", _Device.Mode);

        Error = RAK3172_LoRaWAN_Init(_Device, 16, 3, RAK_JOIN_OTAA, DEVEUI, APPEUI, APPKEY, 'A', RAK_BAND_EU868, RAK_SUB_BAND_NONE);
        if(Error != RAK3172_ERR_OK)
        {
            ESP_LOGE(TAG, "Cannot initialize RAK3172 LoRaWAN! Error: 0x%04X", Error);
        }
    }
    else if((rtc_get_reset_reason(0) == DEEPSLEEP_RESET) || (rtc_get_reset_reason(1) == DEEPSLEEP_RESET))
    {
        RAK3172_Error_t Error;

        RAK3172_WakeUp(_Device);

        if(RAK3172_LoRaWAN_isJoined(_Device) == false)
        {
            ESP_LOGI(TAG, "Not joined. Rejoin...");

            Error = RAK3172_LoRaWAN_StartJoin(_Device, LORAWAN_JOIN_ATTEMPTS, RAK3172_NO_TIMEOUT, false, LORAWAN_MAX_JOIN_INTERVAL_S, NULL);
            if(Error != RAK3172_ERR_OK)
            {
                ESP_LOGE(TAG, "Cannot join network! Error: 0x%04X", Error);
            }
        }

        if(RAK3172_LoRaWAN_isJoined(_Device, false))
        {
            char Payload[] = {'{', '}'};

            ESP_LOGI(TAG, "Joined...");

            Error = RAK3172_LoRaWAN_Transmit(_Device, 1, Payload, sizeof(Payload), true);
            if(Error == RAK3172_ERR_INVALID_RESPONSE)
            {
                ESP_LOGE(TAG, "Cannot transmit message! Error: 0x%04X", Error);
            }
            else
            {
                RAK3172_Rx_t Message;

                ESP_LOGI(TAG, "Message transmitted...");
                Error = RAK3172_LoRaWAN_Receive(_Device, &Message);
                if(Error != RAK3172_ERR_OK)
                {
                    ESP_LOGE(TAG, "Cannot receive message! Error: 0x%04X", Error);
                }
                else
                {
                    ESP_LOGI(TAG, " RSSI: %i", Message.RSSI);
                    ESP_LOGI(TAG, " SNR: %i", Message.SNR);
                    ESP_LOGI(TAG, " Port: %u", Message.Port);
                    ESP_LOGI(TAG, " Payload: %s", Message.Payload.c_str());
                }
            }
        }
    }

	// Prepare the driver for entering sleep mode.
    RAK3172_Deinit(_Device);

    // Disable the RTC fast memory during sleep.
	esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);

	// Disable all wakeup sources.
	esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);

    // Sleep for 60 seconds.
	esp_sleep_enable_timer_wakeup(((uint64_t)60) * 1000000ULL);

	// Disable the WiFi interface.
	esp_wifi_disconnect();
	esp_wifi_stop();
	vTaskDelay(100 / portTICK_PERIOD_MS);
	esp_wifi_deinit();

	esp_deep_sleep_start();
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
