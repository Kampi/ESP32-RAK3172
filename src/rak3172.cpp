 /*
 * rak3172.cpp
 *
 *  Copyright (C) Daniel Kampert, 2022
 *	Website: www.kampis-elektroecke.de
 *  File info: RAK3172 driver for ESP32.

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

#include <esp32_log.h>

#include "../include/rak3172.h"

static uart_config_t _UART_Config = {
    .data_bits = UART_DATA_8_BITS,
    .parity    = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    .source_clk = UART_SCLK_APB,
}

static QueueHandle_t uart0_queue;

static const int RX_BUF_SIZE = 1024;

static const char* TAG = "RAK3172";

static void RAK3172_EventTask(void* p_Arg)
{
    uart_event_t event;
    size_t buffered_size;
    uint8_t* dtmp = (uint8_t*)malloc(RX_BUF_SIZE);

    while(true)
    {
        //Waiting for UART event.
        if(xQueueReceive(uart0_queue, (void*)&event, (TickType_t)portMAX_DELAY))
        {
            bzero(dtmp, RX_BUF_SIZE);
            ESP_LOGI(TAG, "uart[%d] event:", UART_NUM_1);
            switch(event.type)
            {
                //Event of UART receving data
                /*We'd better handler data event fast, there would be much more data events than
                other types of events. If we take too much time on data event, the queue might
                be full.*/
                case UART_DATA:
                    ESP_LOGI(TAG, "[UART DATA]: %d", event.size);
                    uart_read_bytes(UART_NUM_1, dtmp, event.size, portMAX_DELAY);
                    ESP_LOGI(TAG, "[DATA EVT]:");
                    uart_write_bytes(UART_NUM_1, (const char*) dtmp, event.size);
                    break;
                //Event of HW FIFO overflow detected
                case UART_FIFO_OVF:
                    ESP_LOGI(TAG, "hw fifo overflow");
                    // If fifo overflow happened, you should consider adding flow control for your application.
                    // The ISR has already reset the rx FIFO,
                    // As an example, we directly flush the rx buffer here in order to read more data.
                    uart_flush_input(UART_NUM_1);
                    xQueueReset(uart0_queue);
                    break;
                //Event of UART ring buffer full
                case UART_BUFFER_FULL:
                    ESP_LOGI(TAG, "ring buffer full");
                    // If buffer full happened, you should consider encreasing your buffer size
                    // As an example, we directly flush the rx buffer here in order to read more data.
                    uart_flush_input(UART_NUM_1);
                    xQueueReset(uart0_queue);
                    break;
                //Event of UART RX break detected
                case UART_BREAK:
                    ESP_LOGI(TAG, "uart rx break");
                    break;
                //Event of UART parity check error
                case UART_PARITY_ERR:
                    ESP_LOGI(TAG, "uart parity error");
                    break;
                //Event of UART frame error
                case UART_FRAME_ERR:
                    ESP_LOGI(TAG, "uart frame error");
                    break;
                //UART_PATTERN_DET
                case UART_PATTERN_DET:
                    uart_get_buffered_data_len(UART_NUM_1, &buffered_size);
                    int pos = uart_pattern_pop_pos(UART_NUM_1);
                    ESP_LOGI(TAG, "[UART PATTERN DETECTED] pos: %d, buffered size: %d", pos, buffered_size);
                    if (pos == -1) {
                        // There used to be a UART_PATTERN_DET event, but the pattern position queue is full so that it can not
                        // record the position. We should set a larger queue size.
                        // As an example, we directly flush the rx buffer here.
                        uart_flush_input(UART_NUM_1);
                    } else {
                        uart_read_bytes(UART_NUM_1, dtmp, pos, 100 / portTICK_PERIOD_MS);
                        uint8_t pat[1 + 1];
                        memset(pat, 0, sizeof(pat));
                        uart_read_bytes(UART_NUM_1, pat, 1, 100 / portTICK_PERIOD_MS);
                        ESP_LOGI(TAG, "read data: %s", dtmp);
                        ESP_LOGI(TAG, "read pat : %s", pat);
                    }
                    break;
                //Others
                default:
                    ESP_LOGI(TAG, "uart event type: %d", event.type);
                    break;
            }
        }
    }

    free(dtmp);
    dtmp = NULL;

    vTaskDelete(NULL);
}

RAK3172_Error_t RAK3172_Init(RAK3172_t* p_Device)
{
    std::string Response;

    if(p_Device == NULL)
    {
        return RAK3172_INVALID_RESPONSE;
    }

    _UART_Config = p_Device->Baudrate,

    p_Device->isInitialized = false;

    if(uart_driver_install(p_Device->Interface, RX_BUF_SIZE * 2, 0, 0, NULL, intr_alloc_flags) ||
        uart_param_config(p_Device->Interface, &_UART_Config) ||
        uart_set_pin(p_Device->Interface, p_Device->Tx, p_Device->Rx, -1, -1))
    {
        return RAK3172_INVALID_STATE;
    }

    if(uart_flush(p_Device->Interface))
    {
        return RAK3172_INVALID_STATE;
    }

    p_Device->isInitialized = true;

    xTaskCreate(RAK3172_EventTask, "RAK3172_EventTask", RX_BUF_SIZE * 2, NULL, configMAX_PRIORITIES, NULL);

    return RAK3172_SoftReset(p_Device);
}

void RAK3172_Deinit(RAK3172_t* p_Device)
{
    if(!p_Device->isInitialized)
    {
        return;
    }

    pinMode(p_Device->Rx, OUTPUT);
    pinMode(p_Device->Tx, OUTPUT);

    digitalWrite(p_Device->Rx, HIGH);
    digitalWrite(p_Device->Tx, LOW);

/*
	if(p_Device->p_Interface)
	{
		p_Device->p_Interface->flush();
		p_Device->p_Interface->end();
	}*/
}

RAK3172_Error_t RAK3172_SoftReset(RAK3172_t* p_Device, uint32_t Timeout)
{
    uint32_t Now;
    std::string Response;

    // Reset the module and read back the slash screen because the current state is unclear.
    //p_Device->p_Interface->print("ATZ\r\n");

    // Wait for the splashscreen and get the data.
    Now = millis();
    //p_Device->p_Interface->getTimeout();
    do
    {
        //Response += p_Device->p_Interface->readStringUntil('\n');
        if((millis() - Now) >= (Timeout * 1000ULL))
        {
            ESP_LOGE(TAG, "Module reset timeout!");

            return RAK3172_TIMEOUT;
        }

        ESP_LOGD(TAG, "Response: %s", Response.c_str());
    } while((Response.indexOf("LoRaWAN.") == -1) && (Response.indexOf("LoRa P2P.") == -1));

    ESP_LOGD(TAG, "SW reset successful");

    return RAK3172_OK;
}

const std::string RAK3172_LibVersion(void)
{
    return "2.1.0";
}

RAK3172_Error_t RAK3172_SendCommand(RAK3172_t* p_Device, std::string Command, std::string* p_Value, std::string* p_Status)
{
    std::string Status_Temp;
    RAK3172_Error_t Error = RAK3172_OK;

    if(p_Device == NULL)
    {
        return RAK3172_INVALID_RESPONSE;
    }

    if(p_Device->isBusy)
    {
        ESP_LOGE(TAG, "Device busy!");

        return RAK3172_FAIL;
    }

    // Transmit the command.
    ESP_LOGD(TAG, "Transmit command: %s", Command.c_str());
    //p_Device->p_Interface->print(Command + "\r\n");

    // Response expected. Read the value.
    if(p_Value != NULL)
    {
        //*p_Value = p_Device->p_Interface->readStringUntil('\n');
        p_Value->replace("\r", "");
        ESP_LOGI(TAG, "    Value: %s", p_Value->c_str());
    }

    // Receive the line feed before the status.
    //p_Device->p_Interface->readStringUntil('\n');

    // Receive the trailing status code.
    //Status_Temp = p_Device->p_Interface->readStringUntil('\n');
    ESP_LOGD(TAG, "    Status: %s", Status_Temp.c_str());

    // Transmission is without error when 'OK' as status code and when no event data are received.
    if((Status_Temp.indexOf("OK") == -1) && (Status_Temp.indexOf("+EVT") == -1))
    {
        Error = RAK3172_FAIL;
    }

    // Copy the status string if needed.
    if(p_Status != NULL)
    {
        *p_Status = Status_Temp;
    }
    ESP_LOGD(TAG, "    Error: %i", (int)Error);

    return Error;
}

RAK3172_Error_t RAK3172_GetFWVersion(RAK3172_t* p_Device, std::string* p_Version)
{
    if(p_Version == NULL)
    {
        return RAK3172_INVALID_RESPONSE;
    }

    return RAK3172_SendCommand(p_Device, "AT+VER=?", p_Version, NULL);
}

RAK3172_Error_t RAK3172_GetSerialNumber(RAK3172_t* p_Device, std::string* p_Serial)
{
    if(p_Serial == NULL)
    {
        return RAK3172_INVALID_RESPONSE;
    }

    return RAK3172_SendCommand(p_Device, "AT+SN=?", p_Serial, NULL);
}

RAK3172_Error_t RAK3172_GetRSSI(RAK3172_t* p_Device, int* p_RSSI)
{
    std::string Value;
    RAK3172_Error_t Error;

    if(p_RSSI == NULL)
    {
        return RAK3172_INVALID_RESPONSE;
    }

    Error = RAK3172_SendCommand(p_Device, "AT+RSSI=?", &Value, NULL);
    if(Error)
    {
        return Error;
    }

    *p_RSSI = Value.toInt();

    return Error;
}

RAK3172_Error_t RAK3172_GetSNR(RAK3172_t* p_Device, int* p_SNR)
{
    std::string Value;
    RAK3172_Error_t Error;

    if(p_SNR == NULL)
    {
        return RAK3172_INVALID_RESPONSE;
    }

    Error = RAK3172_SendCommand(p_Device, "AT+SNR=?", &Value, NULL);
    if(Error)
    {
        return Error;
    }

    *p_SNR = Value.toInt();

    return Error;
}

RAK3172_Error_t RAK3172_SetMode(RAK3172_t* p_Device, RAK3172_Mode_t Mode)
{
    std::string Status;

    if(!p_Device->isInitialized)
    {
        return RAK3172_INVALID_RESPONSE;
    }

    // Transmit the command.
    p_Device->p_Interface->print("AT+NWM=" + std::string(Mode) + "\r\n");

    // Receive the line feed before the status.
    p_Device->p_Interface->readStringUntil('\n');

    // Receive the trailing status code.
    Status = p_Device->p_Interface->readStringUntil('\n');

    // 'OK' received, so the mode wasn´t change. Leave the function
    Status.clear();
    if(Status.indexOf("OK") >= 0)
    {
        return RAK3172_OK;
    }

    // Otherwise read the remaining lines.
    Status.clear();
    do
    {
        Status += p_Device->p_Interface->readStringUntil('\n');
    } while(p_Device->p_Interface->available() > 0);

    return RAK3172_OK;
}

RAK3172_Error_t RAK3172_GetMode(RAK3172_t* p_Device)
{
    std::string Value;
    RAK3172_Error_t Error = RAK3172_OK;

    if(!p_Device->isInitialized)
    {
        return RAK3172_INVALID_RESPONSE;
    }

    Error = RAK3172_SendCommand(p_Device, "AT+NWM=?", &Value, NULL);
    if(Error)
    {
        return Error;
    }

    p_Device->Mode = (RAK3172_Mode_t)Value.toInt();

    return Error;
}

RAK3172_Error_t RAK3172_SetBaud(RAK3172_t* p_Device, RAK3172_Baud_t Baud)
{
    if(!p_Device->isInitialized)
    {
        return RAK3172_INVALID_RESPONSE;
    }

    if(p_Device->Baudrate == Baud)
    {
        return RAK3172_OK;
    }

    return RAK3172_SendCommand(p_Device, "AT+BAUD=" + std::string(Baud), NULL, NULL);
}

RAK3172_Error_t RAK3172_GetBaud(RAK3172_t* p_Device, RAK3172_Baud_t* p_Baud)
{
    std::string Value;
    RAK3172_Error_t Error = RAK3172_OK;

    if(!p_Device->isInitialized)
    {
        return RAK3172_INVALID_RESPONSE;
    }

    Error = RAK3172_SendCommand(p_Device, "AT+BAUD=?", &Value, NULL);
    if(Error)
    {
        return Error;
    }

    *p_Baud = (RAK3172_Baud_t)Value.toInt();

    return Error;
}