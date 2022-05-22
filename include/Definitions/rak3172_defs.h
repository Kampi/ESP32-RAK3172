 /*
 * rak3172_defs.h
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

#ifndef RAK3172_DEFS_H_
#define RAK3172_DEFS_H_

#include <driver/uart.h>
#include <driver/gpio.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <freertos/queue.h>

#include <string>
#include <stdint.h>
#include <stdbool.h>

#include "rak3172_errors.h"

#define RAK3172_ERROR_CHECK(Func)                               do                                  \
                                                                {                                   \
                                                                    RAK3172_Error_t Error = Func;   \
                                                                    if(Error != RAK3172_OK)         \
                                                                    {                               \
                                                                        return Error;               \
                                                                    }                               \
                                                                } while(0);

/** @brief Hook for a custom wait callback.
 */
typedef void (*RAK3172_Wait_t)(void);

/** @brief Supported operating modes.
 */
typedef enum
{
    RAK_MODE_P2P        = 0,        /**< LoRa P2P mode. */
    RAK_MODE_LORAWAN,               /**< LoRaWAN mode. */
} RAK3172_Mode_t;

/** @brief Supported join modes.
 */
typedef enum
{
    RAK_JOIN_ABP        = 0,        /**< LoRaWAN ABP mode. */
    RAK_JOIN_OTAA,                  /**< LoRaWAN OTAA mode. */
} RAK3172_JoinMode_t;

/** @brief Supported baudrates.
 */
typedef enum
{
    RAK_BAUD_4800       = 4800,     /**< Baud rate 4800. */
    RAK_BAUD_9600       = 9600,     /**< Baud rate 9600. */
    RAK_BAUD_115200     = 115200,   /**< Baud rate 115200. */
} RAK3172_Baud_t;

/** @brief Supported frequency bands for LoRaWAN.
 */
typedef enum
{
    RAK_BAND_EU433      = 0,        /**< European 433 MHz band. */
    RAK_BAND_CN470,                 /**< Chinese 470 MHz band. */
    RAK_BAND_RU864,                 /**< Russian 864 MHz band. */
    RAK_BAND_IN865,                 /**< Indian 865 MHz band. */
    RAK_BAND_EU868,                 /**< European 868 MHz band. */
    RAK_BAND_US915,                 /**< American 915 MHz band. */
    RAK_BAND_AU915,                 /**< Australia 915 MHz band. */
    RAK_BAND_KR920,                 /**< Korean 920 MHz band. */
    RAK_BAND_AS923,                 /**< American Samoa 923 MHz band. */
} RAK3172_Band_t;

/** @brief LoRaWAN data rate definitions.
 */
typedef enum
{
    RAK_DR_0            = 0,        /**< Datarate 0. */
    RAK_DR_1,                       /**< Datarate 1. */
    RAK_DR_2,                       /**< Datarate 2. */
    RAK_DR_3,                       /**< Datarate 3. */
    RAK_DR_4,                       /**< Datarate 4. */
    RAK_DR_5,                       /**< Datarate 5. */
    RAK_DR_6,                       /**< Datarate 6. */
    RAK_DR_7,                       /**< Datarate 7. */
} RAK3172_DataRate_t;

/** @brief LoRaWAN sub band options.
 */
typedef enum
{
    RAK_SUB_BAND_NONE   = 0,        /**< No sub band used. */
    RAK_SUB_BAND_ALL,               /**< All channels enabled. */
    RAK_SUB_BAND_1,                 /**< Channels 0-7 enabled. */
    RAK_SUB_BAND_2,                 /**< Channels 8-15 enabled. */
    RAK_SUB_BAND_3,                 /**< Channels 16-23 enabled. */
    RAK_SUB_BAND_4,                 /**< Channels 24-31 enabled. */
    RAK_SUB_BAND_5,                 /**< Channels 32-39 enabled. */
    RAK_SUB_BAND_6,                 /**< Channels 40-47 enabled. */
    RAK_SUB_BAND_7,                 /**< Channels 48-55 enabled. */
    RAK_SUB_BAND_8,                 /**< Channels 56-63 enabled. */
    RAK_SUB_BAND_9,                 /**< Channels 64-71 enabled.
                                         NOTE: Only with band CN470! */
    RAK_SUB_BAND_10,                /**< Channels 72-79 enabled.
                                         NOTE: Only with band CN470! */
    RAK_SUB_BAND_11,                /**< Channels 80-87 enabled.
                                         NOTE: Only with band CN470! */
    RAK_SUB_BAND_12,                /**< Channels 88-95 enabled.
                                         NOTE: Only with band CN470! */
} RAK3172_SubBand_t;

/** @brief P2P spreading factor definitions.
 */
typedef enum
{
    RAK_PSF_6           = 6,        /**< Spreading factor 6. */
    RAK_PSF_7,                      /**< Spreading factor 7. */
    RAK_PSF_8,                      /**< Spreading factor 8. */
    RAK_PSF_9,                      /**< Spreading factor 9. */
    RAK_PSF_10,                     /**< Spreading factor 10. */
    RAK_PSF_11,                     /**< Spreading factor 11. */
    RAK_PSF_12,                     /**< Spreading factor 12. */
} RAK3172_PSF_t;

/** @brief P2P bandwith definitions.
 */
typedef enum
{
    RAK_BW_125          = 125,      /**< 125 kHz bandwidth. */
    RAK_BW_250          = 250,      /**< 250 kHz bandwidth. */
    RAK_BW_500          = 500,      /**< 500 kHz bandwidth. */
} RAK3172_BW_t;

/** @brief P2P coding rate definitions.
 */
typedef enum
{
    RAK_CR_0            = 0,        /**< Coding rate 0. */
    RAK_CR_1,                       /**< Coding rate 1. */
    RAK_CR_2,                       /**< Coding rate 2. */
    RAK_CR_3,                       /**< Coding rate 3. */
} RAK3172_CR_t;

/** @brief Supported receive options.
 */
typedef enum
{
    RAK_REC_STOP        = 0,        /**< Stop receiving in LoRa P2P mode. */
    RAK_REC_REPEAT      = 65534,    /**< Receive messages in a loop without timeout in LoRa P2P mode. */
    RAK_REC_SINGLE      = 65535,    /**< Receive one message without timeout in LoRa P2P mode. */
} RAK3172_RxOpt_t;

/** @brief RAK3172 device object.
 */
typedef struct
{
    uart_port_t Interface;          /**< Serial interface used by the device RAK3172 driver. */
    gpio_num_t Rx;                  /**< Rx pin number. */
    gpio_num_t Tx;                  /**< Tx pin number. */
	RAK3172_Baud_t Baudrate;		/**< Baud rate for the module communication. */
    RAK3172_Mode_t Mode;            /**< Current device mode. */
    std::string Firmware;           /**< Firmware version string. */
    std::string Serial;             /**< Serial number string. */
    struct
    {
        TaskHandle_t Handle;        /**< Handle for the receive task.
                                         NOTE: Managed by the driver. */
        bool isInitialized;         /**< #true when the device is initialized.
                                         NOTE: Managed by the driver. */
        bool isBusy;                /**< #true when the device is busy.
                                         NOTE: Managed by the driver. */
        uint8_t* RxBuffer;          /**< Pointer to receive buffer.
                                         NOTE: Managed by the driver. */
        QueueHandle_t Rx_Queue;     /**< Rx queue used by the receiving task.
                                         NOTE: Managed by the driver. */
    } Internal;
    struct
    {
        RAK3172_JoinMode_t Join;    /**< Join mode used by the device.
                                         NOTE: Managed by the driver. */
    } LoRaWAN;
    struct
    {
        QueueHandle_t* Queue;       /**< Pointer to message queue.
                                         NOTE: Managed by the driver. */
        bool Active;                /**< Receive task active.
                                         NOTE: Managed by the driver. */
        uint16_t Timeout;           /**< Receive timeout.
                                         NOTE: Managed by the driver. */
        TaskHandle_t Handle;        /**< Handle for the P2P receive task.
                                         NOTE: Managed by the driver. */
    } P2P;
} RAK3172_t;

/** @brief RAK3172 P2P receive object.
 */
typedef struct
{
    std::string Payload;            /**< Received payload. */
    int8_t RSSI;                    /**< Receiving RSSI value. */
    int8_t SNR;                     /**< Receiving SNR value. */
} RAK3172_Rx_t;

#endif /* RAK3172_DEFS_H_ */