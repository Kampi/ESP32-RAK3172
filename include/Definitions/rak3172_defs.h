 /*
 * rak3172_defs.h
 *
 *  Copyright (C) Daniel Kampert, 2023
 *	Website: www.kampis-elektroecke.de
 *  File info: RAK3172 serial driver.
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

#include <sdkconfig.h>

#include "rak3172_errors.h"
#include "rak3172_config.h"

/** @brief Timeout for UART receive queue.
 */
#define RAK3172_DEFAULT_WAIT_TIMEOUT                            500

/** @brief No timeout definition.
 */
#define RAK3172_NO_TIMEOUT                                      0

/** @brief Hook for a custom wait callback.
 */
typedef void (*RAK3172_Wait_t)(void);

/** @brief  Encryption key definition.
 *          NOTE: Only used with RUI3 API support enabled.
 */
typedef uint8_t RAK3172_EncryptKey_t[8];

/** @brief  Supported channel modes.
 *          NOTE: Only used with RUI3 API support enabled.
 */
typedef enum
{
    RAK_CHANMODE_SINGLE     = 1,        /**< Single channel mode. */
    RAK_CHANMODE_EIGHT      = 2,        /**< Eight channel mode. */
} RAK3172_ChanMode_t;

/** @brief Supported operating modes.
 */
typedef enum
{
    RAK_MODE_P2P            = 0,        /**< LoRa P2P mode. */
    RAK_MODE_LORAWAN,                   /**< LoRaWAN mode. */
    RAK_MODE_P2P_FSK,                   /**< P2P FSK mode. */
} RAK3172_Mode_t;

/** @brief Supported baudrates.
 */
typedef enum
{
    RAK_BAUD_4800           = 4800,     /**< Baud rate 4800. */
    RAK_BAUD_9600           = 9600,     /**< Baud rate 9600. */
    RAK_BAUD_19200          = 19200,    /**< Baud rate 19200. */
    RAK_BAUD_38400          = 38400,    /**< Baud rate 38400. */
    RAK_BAUD_57600          = 57600,    /**< Baud rate 57600. */
    RAK_BAUD_115200         = 115200,   /**< Baud rate 115200. */
} RAK3172_Baud_t;

/** @brief LoRaWAN class definitions.
 */
typedef enum
{
    RAK_CLASS_A             = 'A',      /**< LoRaWAN class A. */
    RAK_CLASS_B             = 'B',      /**< LoRaWAN class B. */
    RAK_CLASS_C             = 'C',      /**< LoRaWAN class C. */
} RAK3172_Class_t;

/** @brief Supported join modes for LoRaWAN.
 */
typedef enum
{
    RAK_JOIN_ABP            = 0,        /**< LoRaWAN ABP mode. */
    RAK_JOIN_OTAA,                      /**< LoRaWAN OTAA mode. */
} RAK3172_JoinMode_t;

/** @brief Supported frequency bands for LoRaWAN.
 */
typedef enum
{
    RAK_BAND_EU433          = 0,        /**< European 433 MHz band. */
    RAK_BAND_CN470,                     /**< Chinese 470 MHz band. */
    RAK_BAND_RU864,                     /**< Russian 864 MHz band. */
    RAK_BAND_IN865,                     /**< Indian 865 MHz band. */
    RAK_BAND_EU868,                     /**< European 868 MHz band. */
    RAK_BAND_US915,                     /**< American 915 MHz band. */
    RAK_BAND_AU915,                     /**< Australia 915 MHz band. */
    RAK_BAND_KR920,                     /**< Korean 920 MHz band. */
    RAK_BAND_AS923,                     /**< American Samoa 923 MHz band. */
} RAK3172_Band_t;

/** @brief LoRaWAN data rate definitions.
 */
typedef enum
{
    RAK_DR_0                = 0,        /**< Data rate 0. */
    RAK_DR_1,                           /**< Data rate 1. */
    RAK_DR_2,                           /**< Data rate 2. */
    RAK_DR_3,                           /**< Data rate 3. */
    RAK_DR_4,                           /**< Data rate 4. */
    RAK_DR_5,                           /**< Data rate 5. */
    RAK_DR_6,                           /**< Data rate 6. */
    RAK_DR_7,                           /**< Data rate 7. */
} RAK3172_DataRate_t;

/** @brief LoRaWAN sub band options.
 */
typedef enum
{
    RAK_SUB_BAND_NONE       = 0,        /**< No sub band used. */
    RAK_SUB_BAND_ALL,                   /**< All channels enabled. */
    RAK_SUB_BAND_1,                     /**< Channels 0-7 enabled. */
    RAK_SUB_BAND_2,                     /**< Channels 8-15 enabled. */
    RAK_SUB_BAND_3,                     /**< Channels 16-23 enabled. */
    RAK_SUB_BAND_4,                     /**< Channels 24-31 enabled. */
    RAK_SUB_BAND_5,                     /**< Channels 32-39 enabled. */
    RAK_SUB_BAND_6,                     /**< Channels 40-47 enabled. */
    RAK_SUB_BAND_7,                     /**< Channels 48-55 enabled. */
    RAK_SUB_BAND_8,                     /**< Channels 56-63 enabled. */
    RAK_SUB_BAND_9,                     /**< Channels 64-71 enabled.
                                             NOTE:Can only used with band CN470! */
    RAK_SUB_BAND_10,                    /**< Channels 72-79 enabled.
                                             NOTE: Can only used with band CN470! */
    RAK_SUB_BAND_11,                    /**< Channels 80-87 enabled.
                                             NOTE: Can only used with band CN470! */
    RAK_SUB_BAND_12,                    /**< Channels 88-95 enabled.
                                             NOTE: Can only used with band CN470! */
} RAK3172_SubBand_t;

/** @brief LoRaWAN receive group definitions
 */
typedef enum
{
    RAK_RX_GROUP_1          = 0,        /**< Receive group 1. */
    RAK_RX_GROUP_2,                     /**< Receive group 2. */
    RAK_RX_GROUP_B,                     /**< Receive group B. */
    RAK_RX_GROUP_C,                     /**< Receive group C. */
} RAK3172_Rx_Group_t;

/** @brief P2P spreading factor definitions.
 */
typedef enum
{
    #ifdef CONFIG_RAK3172_USE_RUI3
        RAK_PSF_5           = 5,        /**< Spreading factor 5. */
    #endif
    RAK_PSF_6               = 6,        /**< Spreading factor 6. */
    RAK_PSF_7,                          /**< Spreading factor 7. */
    RAK_PSF_8,                          /**< Spreading factor 8. */
    RAK_PSF_9,                          /**< Spreading factor 9. */
    RAK_PSF_10,                         /**< Spreading factor 10. */
    RAK_PSF_11,                         /**< Spreading factor 11. */
    RAK_PSF_12,                         /**< Spreading factor 12. */
} RAK3172_PSF_t;

/** @brief P2P bandwith definitions.
 */

#ifdef CONFIG_RAK3172_USE_RUI3
    typedef enum
    {
        RAK_BW_125          = 0,        /**< 125 kHz bandwidth. */
        RAK_BW_250,                     /**< 250 kHz bandwidth. */
        RAK_BW_500,                     /**< 500 kHz bandwidth. */
        RAK_BW_78,                      /**< 7.8 kHz bandwidth. */
        RAK_BW_104,                     /**< 10.4 kHz bandwidth. */
        RAK_BW_1563,                    /**< 15.63 kHz bandwidth. */
        RAK_BW_2083,                    /**< 20.83 kHz bandwidth. */
        RAK_BW_3125,                    /**< 31.25 kHz bandwidth. */
        RAK_BW_625,                     /**< 62.5 kHz bandwidth. */
    } RAK3172_BW_t;
#else
    typedef enum
    {
        RAK_BW_125          = 125,      /**< 125 kHz bandwidth. */
        RAK_BW_250          = 250,      /**< 250 kHz bandwidth. */
        RAK_BW_500          = 500,      /**< 500 kHz bandwidth. */
    } RAK3172_BW_t;
#endif

/** @brief P2P coding rate definitions.
 */
typedef enum
{
    RAK_CR_45               = 0,        /**< Coding rate 4/5. */
    RAK_CR_46,                          /**< Coding rate 4/6. */
    RAK_CR_47,                          /**< Coding rate 4/7. */
    RAK_CR_48,                          /**< Coding rate 4/8. */
} RAK3172_CR_t;

/** @brief Supported receive options for P2P mode.
 */
typedef enum
{
    RAK_REC_STOP            = 0,        /**< Stop receiving in LoRa P2P mode. */
    RAK_REC_REPEAT          = 65534,    /**< Receive messages in a loop without timeout in LoRa P2P mode. */
    RAK_REC_SINGLE          = 65535,    /**< Receive one message without timeout in LoRa P2P mode. */
} RAK3172_RxOpt_t;

/** @brief RAK3172 device information object.
 */
typedef struct
{
    std::string Firmware;               /**< Firmware version string. */
    std::string Serial;                 /**< Serial number string. */
    std::string CLI;                    /**< CLI version string. */
    std::string API;                    /**< API version string. */
    std::string Model;                  /**< Hardware model. */
    std::string HWID;                   /**< Hardware ID. */
    std::string BuildTime;              /**< Firmware build time. */
    std::string RepoInfo;               /**< Firmware repo information. */
} RAK3172_Info_t;

/** @brief RAK3172 device object definition.
 */
typedef struct
{
    struct
    {
        uart_port_t Interface;          /**< Serial interface used by the device RAK3172 driver. */
        gpio_num_t Rx;                  /**< Rx pin number (MCU). */
        gpio_num_t Tx;                  /**< Tx pin number (MCU). */
	    RAK3172_Baud_t Baudrate;		/**< Baud rate for the module communication. */
    } UART;
    #ifdef CONFIG_RAK3172_RESET_USE_HW
        gpio_num_t Reset;               /**< Reset pin number. */
    #endif
    RAK3172_Mode_t Mode;                /**< Current device mode. */
    RAK3172_Info_t* Info;               /**< (Optional) Pointer to device information object. */
    struct
    {
        TaskHandle_t Handle;            /**< Handle for the UART receive task.
                                             NOTE: Managed by the driver. */
        bool isInitialized;             /**< #true when the device driver is initialized.
                                             NOTE: Managed by the driver. */
        bool isBusy;                    /**< #true when the device is busy.
                                             NOTE: Managed by the driver. */
        uint8_t* RxBuffer;              /**< Pointer to receive buffer.
                                             NOTE: Managed by the driver. */
        QueueHandle_t MessageQueue;     /**< Module Rx message queue used by the receiving task.
                                             NOTE: Managed by the driver. */
        QueueHandle_t EventQueue;       /**< Event queue used by the UART driver for the pattern detection.
                                             NOTE: Managed by the driver. */
        QueueHandle_t ReceiveQueue;     /**< Receive message queue.
                                             NOTE: Managed by the driver. */
        bool isJoinEvent;               /**< #true when a join event has occured.
                                             NOTE: Only used for module firmware without RUI3 interface! */
    } Internal;
    struct
    {
        RAK3172_JoinMode_t Join;        /**< Join mode used by the device.
                                             NOTE: Managed by the driver. */
        bool isJoined;                  /**< Join status of the device.
                                             NOTE: Managed by the driver. */
        bool ConfirmError;              /**< Message confirmation failed.
                                             NOTE: Managed by the driver. */
        uint8_t AttemptCounter;         /**< Attempt counter for the join process.
                                             NOTE: Managed by the driver and only used when RUI3 isn´t used. */
    } LoRaWAN;
    struct
    {
        bool Active;                    /**< Receive task active.
                                             NOTE: Managed by the driver. */
        bool isEncryptionEnabled;       /**< LoRa P2P encryption status.
                                             NOTE: Managed by the driver. */
        bool isRxTimeout;               /**< LoRa P2P receive timeout.
                                             NOTE: Managed by the driver. */
        uint16_t Timeout;               /**< Receive timeout.
                                             NOTE: Managed by the driver. */
        TaskHandle_t ListenHandle;      /**< Task handle for the P2P receive task from the "RAK3172_P2P_Listen" function.
                                             NOTE: Managed by the driver. */
        QueueHandle_t ListenQueue;      /**< Listen queue used by the "RAK3172_P2P_Listen" function.
                                             NOTE: Managed by the driver. */
    } P2P;
} RAK3172_t;

/** @brief RAK3172 message receive object.
 */
typedef struct
{
    std::string Payload;                /**< Received payload. */
    int8_t RSSI;                        /**< Receiving RSSI value. */
    int8_t SNR;                         /**< Receiving SNR value. */
    uint8_t Port;                       /**< Port number.
                                             NOTE: Only used in LoRaWAN mode! */
    RAK3172_Rx_Group_t Group;           /**< Receive group.
                                             NOTE: Only used in LoRaWAN mode! */
} RAK3172_Rx_t;

/** @brief RAK3172 multicast group configuration object.
 */
typedef struct
{
    RAK3172_Class_t Class;              /**< LoRaWAN device class. 
                                             NOTE: Only class 'B' and 'C' are allowed in the further application! */
    std::string DevAddr;                /**< Device address. */
    std::string NwkSKey;                /**< Network session key used by this group. */
    std::string AppSKey;                /**< App session key used by this group. */
    RAK3172_DataRate_t Datarate;        /**< Data rate used by this group. */
    uint32_t Frequency;                 /**< LoRaWAN frequency used by this group. */
    uint8_t Periodicity;                /**< LoRaWAN ping periodicity used by this group.
                                             NOTE: Ignored when class is set to 'C' and only values <8 are allowed! */
} RAK3172_MC_Group_t;

#endif /* RAK3172_DEFS_H_ */