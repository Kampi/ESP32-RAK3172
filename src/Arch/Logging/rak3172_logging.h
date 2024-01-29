 /*
 * rak3172_logging.h
 *
 *  Copyright (C) Daniel Kampert, 2023
 *	Website: www.kampis-elektroecke.de
 *  File info: Logging wrapper for the ESP32.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 * Errors and commissions should be reported to DanielKampert@kampis-elektroecke.de
 */

#ifndef RAK3172_LOGGING_H_
#define RAK3172_LOGGING_H_

#include <esp_log.h>

#include <sdkconfig.h>

#ifdef CONFIG_RAK3172_MISC_ENABLE_LOG
    #define RAK3172_LOGI(tag, format, ...)                      ESP_LOGI(tag, format, ##__VA_ARGS__)
    #define RAK3172_LOGD(tag, format, ...)                      ESP_LOGD(tag, format, ##__VA_ARGS__)
    #define RAK3172_LOGW(tag, format, ...)                      ESP_LOGW(tag, format, ##__VA_ARGS__)
    #define RAK3172_LOGE(tag, format, ...)                      ESP_LOGE(tag, format, ##__VA_ARGS__)
    #define RAK3172_LOG_BUFFER_HEX(tag, buffer, buff_len)       ESP_LOG_BUFFER_HEX(tag, buffer, buff_len)
#else
    #define RAK3172_LOGI(tag, format, ...)
    #define RAK3172_LOGD(tag, format, ...)
    #define RAK3172_LOGW(tag, format, ...)
    #define RAK3172_LOGE(tag, format, ...)
    #define RAK3172_LOG_BUFFER_HEX(tag, buffer, buff_len)
#endif

#endif /* RAK3172_LOGGING_H_ */