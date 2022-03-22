 /*
 * rak3172.h
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

#ifndef RAK3172_H_
#define RAK3172_H_

#include <esp_err.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <freertos/queue.h>

#include <stdint.h>
#include <stdbool.h>
#include <WString.h>
#include <HardwareSerial.h>

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
    RAK_BAND_IN865,                 /**< */
    RAK_BAND_EU868,                 /**< European 868 MHz band. */
    RAK_BAND_US915,                 /**< American 915 MHz band. */
    RAK_BAND_AU915,                 /**< */
    RAK_BAND_KR920,                 /**< */
    RAK_BAND_AS923,                 /**< */
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
    HardwareSerial* p_Interface;    /**< Serial interface used by the device RAK_DRiver. */
    int Rx;                         /**< Rx pin number. */
    int Tx;                         /**< Tx pin number. */
	RAK3172_Baud_t Baudrate;		/**< Baud rate for the module communication. */
    RAK3172_Mode_t Mode;            /**< Current device mode. */
    bool isInitialized;             /**< #true when the device is initialized. */
    bool isBusy;                    /**< #true when the device is busy. */
} RAK3172_t;

/** @brief RAK3172 P2P receive object.
 */
typedef struct
{
    String Payload;                 /**< Received payload. */
    int8_t RSSI;                    /**< Receiving RSSI value. */
    int8_t SNR;                     /**< Receiving SNR value. */
} RAK3172_Rx_t;

/**
 * 
 * RAK3172 common functions.
 * 
 */

/** @brief          Initialize the RAK3172 SoM.
 *  @param p_Device Pointer to RAK3172 device object
 *  @return         ESP_OK when successful
 *                  ESP_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
esp_err_t RAK3172_Init(RAK3172_t* p_Device);

/** @brief          Deinitialize the RAK3172 communication interface.
 *  @param p_Device Pointer to RAK3172 device object
 */
void RAK3172_Deinit(RAK3172_t* p_Device);

/** @brief          Perform a software reset of the device.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param Timeout  Timeout for the device reset in seconds
 *  @return         ESP_OK when successful
 *                  ESP_ERR_INVALID_ARG when an invalid argument is passed into the function
 *                  ESP_ERR_INVALID_STATE the when the interface is not initialized
 */
esp_err_t RAK3172_SoftReset(RAK3172_t* p_Device, uint32_t Timeout = 10);

/** @brief  Get the version number of the RAK3172 library.
 *  @return Library version
 */
String RAK3172_LibVersion(void);

/** @brief          Transmit an AT command to the RAK3172 module.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param Command  RAK3172 command
 *  @param p_Value  (Optional) Pointer to returned value.
 *                  NOTE: Only needed when a read command should be executed.
 *  @param p_Status (Optional) Pointer to status string
 *                  NOTE: Can be set to #NULL when not needed.
 *  @return         ESP_OK when successful
 *                  ESP_ERR_INVALID_ARG when an invalid argument is passed into the function
 *                  ESP_FAIL when an event happens, when the status is not "OK" or when the device is busy
 */
esp_err_t RAK3172_SendCommand(RAK3172_t* p_Device, String Command, String* p_Value, String* p_Status);

/** @brief              Get the firmware version of the RAK3172 SoM.
 *  @param p_Device     Pointer to RAK3172 device object
 *  @param p_Version    Pointer to firmware version string
 *  @return             ESP_OK when successful
 *                      ESP_ERR_INVALID_ARG when an invalid argument was passed
 *                      ESP_ERR_INVALID_STATE the when the interface is not initialized
 */
esp_err_t RAK3172_GetFWVersion(RAK3172_t* p_Device, String* p_Version);

/** @brief              Get the serial number of the RAK3172 SoM.
 *  @param p_Device     Pointer to RAK3172 device object
 *  @param p_Serial     Pointer to serial number string
 *  @return             ESP_OK when successful
 *                      ESP_ERR_INVALID_ARG when an invalid argument was passed
 *                      ESP_ERR_INVALID_STATE the when the interface is not initialized
 */
esp_err_t RAK3172_GetSerialNumber(RAK3172_t* p_Device, String* p_Serial);

/** @brief          Get the RSSI value of the last packet.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param p_RSSI   Pointer to RSSI value
 *  @return         ESP_OK when successful
 *                  ESP_ERR_INVALID_ARG when an invalid argument was passed
 *                  ESP_ERR_INVALID_STATE the when the interface is not initialized
 */
esp_err_t RAK3172_GetRSSI(RAK3172_t* p_Device, int* p_RSSI);

/** @brief          Get the SNR value of the last packet.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param p_SNR    Pointer to SNR value
 *  @return         ESP_OK when successful
 *                  ESP_ERR_INVALID_ARG when an invalid argument was passed
 *                  ESP_ERR_INVALID_STATE the when the interface is not initialized
 */
esp_err_t RAK3172_GetSNR(RAK3172_t* p_Device, int* p_SNR);

/** @brief          Set the current operating mode for the LoRa module.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param Mode     RAK3172 operating mode
 *  @return         ESP_OK when successful
 *                  ESP_ERR_INVALID_ARG when an invalid argument was passed
 *                  ESP_ERR_INVALID_STATE the when the interface is not initialized
 */
esp_err_t RAK3172_SetMode(RAK3172_t* p_Device, RAK3172_Mode_t Mode);

/** @brief          Get the current operating mode for the LoRa module.
 *  @param p_Device Pointer to RAK3172 device object
 *  @return         ESP_OK when successful
 *                  ESP_ERR_INVALID_ARG when an invalid argument was passed
 *                  ESP_ERR_INVALID_STATE the when the interface is not initialized
 */
esp_err_t RAK3172_GetMode(RAK3172_t* p_Device);

/** @brief          Set the baudrate of the LoRa module.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param Baud     Module baudrate
 *  @return         ESP_OK when successful
 *                  ESP_ERR_INVALID_ARG when an invalid argument was passed
 *                  ESP_ERR_INVALID_STATE the when the interface is not initialized
 */
esp_err_t RAK3172_SetBaud(RAK3172_t* p_Device, RAK3172_Baud_t Baud);

/** @brief          Get the baudrate of the LoRa module.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param p_Baud   Pointer to module baudrate
 *  @return         ESP_OK when successful
 *                  ESP_ERR_INVALID_ARG when an invalid argument was passed
 *                  ESP_ERR_INVALID_STATE the when the interface is not initialized
 */
esp_err_t RAK3172_GetBaud(RAK3172_t* p_Device, RAK3172_Baud_t* p_Baud);

/**
 * 
 * RAK3172 LoRaWAN functions.
 * 
 */

/** @brief          Initialize the RAK3172 SoM in LoRaWAN mode.
 *                  NOTE: You must call RAK3172_Init first!
 *  @param p_Device Pointer to RAK3172 device object
 *  @param TxPwr    Tx power in dB
 *  @param Retries  Number of confirmed payload retransmissions
 *  @param JoinMode LoRaWAN join mode
 *  @param p_Key1   Pointer to key 1
 *                  OTAA: DEVEUI (8 Bytes)
 *                  ABP: APPSKEY (16 bytes)
 *  @param p_Key2   Pointer to key 2
 *                  OTAA: APPEUI (16 Bytes)
 *                  ABP: NWKSKEY (16 bytes)
 *  @param p_Key3   Pointer to key 3
 *                  OTAA: APPKEY (8 Bytes)
 *                  ABP: DEVADDR (4 bytes)
 *  @param Band     LoRa frequency band
 *  @param Subband  LoRa sub band
 *                  NOTE: Only needed when US915, AU915 or CN470 band is used. Otherwise set it to RAK_SUB_BAND_NONE!
 *  @param Class    Device class
 *  @param Timeout  Timeout for the device reset in seconds
 *  @return         ESP_OK when successful
 *                  ESP_ERR_INVALID_ARG when an invalid argument was passed
 */
esp_err_t RAK3172_Init_LoRaWAN(RAK3172_t* p_Device, uint8_t TxPwr, uint8_t Retries, RAK3172_JoinMode_t JoinMode, uint8_t* p_Key1, uint8_t* p_Key2, uint8_t* p_Key3, char Class, RAK3172_Band_t Band, RAK3172_SubBand_t Subband, uint32_t Timeout = 10);

/** @brief              Set the keys for OTAA mode.
 *  @param p_Device     Pointer to RAK3172 device object
 *  @param p_DEVEUI     Pointer to LoRaWAN DEVEUI (8 Bytes)
 *  @param p_APPEUI     Pointer to LoRaWAN APPEUI (8 Bytes)
 *  @param p_APPKEY     Pointer to LoRaWAN APPKEY (16 Bytes)
 *  @return             ESP_OK when successful
 *                      ESP_ERR_INVALID_ARG when an invalid argument was passed
 */
esp_err_t RAK3172_SetOTAAKeys(RAK3172_t* p_Device, const uint8_t* p_DEVEUI, const uint8_t* p_APPEUI, const uint8_t* p_APPKEY);

/** @brief              Set the keys for ABP mode.
 *  @param p_Device     Pointer to RAK3172 device object
 *  @param p_APPSKEY    Pointer to LoRaWAN APPSKEY (16 Bytes)
 *  @param p_NWKSKEY    Pointer to LoRaWAN NWKSKEY (16 Bytes)
 *  @param p_DEVADDR    Pointer to LoRaWAN DEVADDR (4 Bytes)
 *  @return             ESP_OK when successful
 *                      ESP_ERR_INVALID_ARG when an invalid argument was passed
 */
esp_err_t RAK3172_SetABPKeys(RAK3172_t* p_Device, const uint8_t* p_APPSKEY, const uint8_t* p_NWKSKEY, const uint8_t* p_DEVADDR);

/** @brief                  Start the joining process.
 *                          NOTE: This is a blocking function!
 *  @param p_Device         Pointer to RAK3172 device object
 *  @param Timeout          Timeout in seconds
 *                          NOTE: Set to 0 to disable the timeout function
 *  @param Attempts         No. of join attempts
 *  @param EnableAutoJoin   Enable auto join after power up
 *  @param Interval         Reattempt interval
 *  @param on_Wait          (Optional) Hook for a custom wait function
 *                          NOTE: The function call is time critical. Prevent long wait periods!
 *  @return                 ESP_OK when joined
 *                          ESP_FAIL when a transmission error occurs
 *                          ESP_ERR_TIMEOUT when the number of JOIN attemps has expired
 *                          ESP_ERR_INVALID_ARG when an invalid argument was passed
 */
esp_err_t RAK3172_StartJoin(RAK3172_t* p_Device, uint32_t Timeout = 0, uint8_t Attempts = 0, bool EnableAutoJoin = false, uint8_t Interval = 10, RAK3172_Wait_t on_Wait = NULL);

/** @brief          Stop the joining process.
 *  @param p_Device Pointer to RAK3172 device object
 *  @return         ESP_OK when successful
 *                  ESP_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
esp_err_t RAK3172_StopJoin(RAK3172_t* p_Device);

/** @brief          Check if the module has joined the network.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param p_Joined Pointer to joining state. #true when joined
 *  @return         ESP_OK when successful
 *                  ESP_ERR_INVALID_ARG when an invalid argument was passed
 *                  ESP_FAIL when a transmission error occurs
 */
esp_err_t RAK3172_Joined(RAK3172_t* p_Device, bool* p_Joined);

/** @brief          Start a LoRaWAN data transmission.
 *                  NOTE: This is a blocking function!
 *  @param p_Device Pointer to RAK3172 device object
 *  @param Port     LoRaWAN port
 *  @param p_Buffer Pointer to data buffer
 *  @param Length   Length of buffer
 *  @return         ESP_OK when successful
 *                  ESP_ERR_INVALID_STATE when the device is busy
 *                  ESP_FAIL when a transmission error occurs
 *                  ESP_ERR_INVALID_ARG when an invalid argument is passed into the function
 *                  ESP_ERR_INVALID_RESPONSE when a send confirmation failed or when the device is busy
 *                  ESP_ERR_TIMEOUT when a transmit timeout occurs
 *                  ESP_ERR_INVALID_STATE when the device is not joined
 */
esp_err_t RAK3172_LoRaWAN_Transmit(RAK3172_t* p_Device, uint8_t Port, const uint8_t* p_Buffer, uint8_t Length);

/** @brief              Start a LoRaWAN data transmission.
 *                      NOTE: This is a blocking function!
 *  @param p_Device     Pointer to RAK3172 device object
 *  @param Port         LoRaWAN port
 *  @param p_Buffer     Pointer to data buffer
 *  @param Length       Length of buffer
 *  @param Timeout      Receive timeout in seconds
 *  @param Confirmed    Enable message confirmation
 *                      NOTE: Only neccessary when payload is greater than 1024 bytes (long payload)
 *  @param Wait         Hook for a custom wait function
 *                      NOTE: The function call is time critical. Prevent long wait periods!
 *  @return             ESP_OK when successful
 *                      ESP_ERR_INVALID_STATE when the device is busy
 *                      ESP_FAIL when a transmission error occurs
 *                      ESP_ERR_INVALID_ARG when an invalid argument is passed into the function
 *                      ESP_ERR_INVALID_RESPONSE when a send confirmation failed or when the device is busy
 *                      ESP_ERR_TIMEOUT when a transmit timeout occurs
 *                      ESP_ERR_INVALID_STATE when the device is not joined
 */
esp_err_t RAK3172_LoRaWAN_Transmit(RAK3172_t* p_Device, uint8_t Port, const void* p_Buffer, uint8_t Length, uint32_t Timeout, bool Confirmed = false, RAK3172_Wait_t Wait = NULL);

/** @brief          Read an incomming LoRaWAN downlink message from the RAK3172.
 *                  NOTE: This is a blocking function!
 *  @param p_Device Pointer to RAK3172 device object
 *  @param Payload  Pointer to payload string
 *  @param p_RSSI   (Optional) Pointer to message RSSI value
 *  @param p_SNR    (Optional) Pointer to message SNR value
 *  @param Timeout  Receive timeout in seconds (must be greater than 1)
 *  @return         ESP_OK when successful
 *                  ESP_ERR_TIMEOUT when a receive timeout occurs
 *                  ESP_ERR_INVALID_ARG when an invalid argument is passed into the function
 *                  ESP_ERR_INVALID_STATE when the device is not joined
 */
esp_err_t RAK3172_LoRaWAN_Receive(RAK3172_t* p_Device, String* p_Payload, int* p_RSSI = NULL, int* p_SNR = NULL, uint32_t Timeout = 3);

/** @brief          Set the number of confirmed payload retransmissions.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param Retries  Number of retries
 *                  NOTE: Only values between 0 and 7 are allowed!
 *  @return         ESP_OK when successful
 *                  ESP_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
esp_err_t RAK3172_SetRetries(RAK3172_t* p_Device, uint8_t Retries);

/** @brief              Get the number of confirmed payload retransmissions.
 *  @param p_Device     Pointer to RAK3172 device object
 *  @param p_Retries    Pointer to number of retries
 *  @return             ESP_OK when successful
 *                      ESP_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
esp_err_t RAK3172_GetRetries(RAK3172_t* p_Device, uint8_t* p_Retries);

/** @brief          Enable / Disable the usage of the public network mode.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param Enable   Enable / Disable the mode
 *  @return         ESP_OK when successful
 *                  ESP_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
esp_err_t RAK3172_SetPNM(RAK3172_t* p_Device, bool Enable);

/** @brief          Get the status of the public network mode.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param p_Enable Pointer to public network mode status
 *  @return         ESP_OK when successful
 *                  ESP_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
esp_err_t RAK3172_GetPNM(RAK3172_t* p_Device, bool* p_Enable);

/** @brief          Enable / Disable the usage of the confirmation mode.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param Enable   Enable / Disable the mode
 *  @return         ESP_OK when successful
 *                  ESP_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
esp_err_t RAK3172_SetConfirmation(RAK3172_t* p_Device, bool Enable);

/** @brief          Get the current state of the transmission confirmation mode.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param p_Enable Pointer to confirmation mode
 *  @return         ESP_OK when successful
 *                  ESP_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
esp_err_t RAK3172_GetConfirmation(RAK3172_t* p_Device, bool* p_Enable);

/** @brief          Set the used frequency band.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param Band     Pointer to frequency band
 *  @return         ESP_OK when successful
 *                  ESP_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
esp_err_t RAK3172_SetBand(RAK3172_t* p_Device, RAK3172_Band_t Band);

/** @brief          Get the used frequency band.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param p_Band   Pointer to frequency band
 *  @return         ESP_OK when successful
 *                  ESP_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
esp_err_t RAK3172_GetBand(RAK3172_t* p_Device, RAK3172_Band_t* p_Band);

/** @brief          Set the sub band for the LoRaWAN communication.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param Band     Target sub band
 *  @return         ESP_OK when successful
 *                  ESP_ERR_INVALID_ARG when an invalid argument is passed into the function
 *                  ESP_ERR_INVALID_RESPONSE when the device is operating in the wrong frequency band
 */
esp_err_t RAK3172_SetSubBand(RAK3172_t* p_Device, RAK3172_SubBand_t Band);

/** @brief          Get the sub band for the LoRaWAN communication.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param Band     Pointer to frequency sub band
 *  @return         ESP_OK when successful
 *                  ESP_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
esp_err_t RAK3172_GetSubBand(RAK3172_t* p_Device, RAK3172_SubBand_t* p_Band);

/** @brief          Set the Tx power of the RAK3172.
 *                  NOTE: The index of the Tx power and the resulting power depends on the selected region!
 *  @param p_Device Pointer to RAK3172 device object
 *  @param TxPwr    Tx power index in dBm
 *                  NOTE: The power must be a power of two!
 *  @return         ESP_OK when successful
 *                  ESP_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
esp_err_t RAK3172_SetTxPwr(RAK3172_t* p_Device, uint8_t TxPwr);

/** @brief          Set the RX1 delay of the device.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param Delay    RX1 delay in ms
 *  @return         ESP_OK when successful
 *                  ESP_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
esp_err_t RAK3172_SetRX1Delay(RAK3172_t* p_Device, uint16_t Delay);

/** @brief          Set the RX2 delay of the device.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param Delay    RX1 delay in ms
 *  @return         ESP_OK when successful
 *                  ESP_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
esp_err_t RAK3172_SetRX2Delay(RAK3172_t* p_Device, uint16_t Delay);

/** @brief          Get the SNR value of the last packet.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param p_SNR    Pointer to SNR value
 *  @return         ESP_OK when successful
 *                  ESP_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
esp_err_t RAK3172_GetSNR(RAK3172_t* p_Device, uint8_t* p_SNR);

/** @brief          Get the RSSI value of the last packet.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param p_RSSI   Pointer to RSSI value
 *  @return         ESP_OK when successful
 *                  ESP_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
esp_err_t RAK3172_GetRSSI(RAK3172_t* p_Device, int8_t* p_RSSI);

/** @brief          Get the duty time.
 *                  NOTE: This can only be used when using EU868, RU864 or EU433 frequency band!
 *  @param p_Device Pointer to RAK3172 device object
 *  @param p_Duty   Pointer to duty cycle time im seconds
 *  @return         ESP_OK when successful
 *                  ESP_ERR_INVALID_ARG when an invalid argument is passed into the function
 *                  ESP_ERR_INVALID_RESPONSE when the device is operating in the wrong frequency band
 */
esp_err_t RAK3172_GetDuty(RAK3172_t* p_Device, uint8_t* p_Duty);

/**
 * 
 * RAK3172 P2P functions.
 * 
 */

/** @brief              Initialize the RAK3172 SoM in P2P mode.
 *                      NOTE: You must call RAK3172_Init first!
 *  @param p_Device     Pointer to RAK3172 device object
 *  @param Frequency    Transmission frequency
 *  @param Spread       Spreading factor
 *  @param Bandwidth    Transmission bandwidth
 *  @param CodeRate     Transmission coderate
 *  @param Preamble     Transmission preamble
 *                      NOTE: Value must be greater than 2!
 *  @param Power        Transmission power in dBm
 *                      NOTE: Only values between 5 and 22 are allowed!
 *  @param Timeout      Timeout for the device reset in seconds
 *  @return             ESP_OK when successful
 *                      ESP_ERR_INVALID_ARG when an invalid argument was passed
 */
esp_err_t RAK3172_Init_P2P(RAK3172_t* p_Device, uint32_t Frequency, RAK3172_PSF_t Spread, RAK3172_BW_t Bandwidth, RAK3172_CR_t CodeRate, uint16_t Preamble, uint8_t Power, uint32_t Timeout = 10);

/** @brief          Read the P2P configuration from the device.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param p_Config Pointer to configuration string
 *  @return         ESP_OK when successful
 *                  ESP_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
esp_err_t RAK3172_GetConfig(RAK3172_t* p_Device, String* p_Config);

/** @brief          Set the P2P mode frequency.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param Freq     Transmission frequency
 *  @return         ESP_OK when successful
 *                  ESP_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
esp_err_t RAK3172_SetFrequency(RAK3172_t* p_Device, uint32_t Freq);

/** @brief          Get the P2P mode frequency.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param p_Freq   Pointer to frequency value
 *  @return         ESP_OK when successful
 *                  ESP_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
esp_err_t RAK3172_GetFrequency(RAK3172_t* p_Device, uint32_t* p_Freq);

/** @brief          Set the P2P mode spreading factor.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param Spread   Spreading factor
 *  @return         ESP_OK when successful
 *                  ESP_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
esp_err_t RAK3172_SetSpreading(RAK3172_t* p_Device, RAK3172_PSF_t Spread);

/** @brief          Get the P2P mode spreading factor.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param p_Spread Pointer to spreading factor
 *  @return         ESP_OK when successful
 *                  ESP_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
esp_err_t RAK3172_GetSpreading(RAK3172_t* p_Device, RAK3172_PSF_t* p_Spread);

/** @brief              Set the P2P mode bandwidth.
 *  @param p_Device     Pointer to RAK3172 device object
 *  @param Bandwidth    Transmission bandwidth
 *  @return             ESP_OK when successful
 *                      ESP_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
esp_err_t RAK3172_SetBandwidth(RAK3172_t* p_Device, RAK3172_BW_t Bandwidth);

/** @brief              Get the P2P bandwidth.
 *  @param p_Device     Pointer to RAK3172 device object
 *  @param p_Bandwidth  Pointer to transmission bandwith
 *  @return             ESP_OK when successful
 *                      ESP_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
esp_err_t RAK3172_GetBandwidth(RAK3172_t* p_Device, RAK3172_BW_t* p_Bandwidth);

/** @brief          Set the P2P mode code rate.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param CodeRate Transmission code rate
 *  @return         ESP_OK when successful
 *                  ESP_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
esp_err_t RAK3172_SetCodeRate(RAK3172_t* p_Device, RAK3172_CR_t CodeRate);

/** @brief              Get the P2P mode code rate.
 *  @param p_Device     Pointer to RAK3172 device object
 *  @param p_CodeRate   Pointer to transmission code rate
 *  @return             ESP_OK when successful
 *                      ESP_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
esp_err_t RAK3172_GetCodeRate(RAK3172_t* p_Device, RAK3172_CR_t* p_CodeRate);

/** @brief          Set the P2P mode preamble.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param Preamble Transmission Preamble
 *                  NOTE: The value must be greater than 2!
 *  @return         ESP_OK when successful
 *                  ESP_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
esp_err_t RAK3172_SetPreamble(RAK3172_t* p_Device, uint16_t Preamble);

/** @brief              Get the P2P mode preamble.
 *  @param p_Device     Pointer to RAK3172 device object
 *  @param p_Preamble   Pointer to transmission Preamble
 *  @return             ESP_OK when successful
 *                      ESP_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
esp_err_t RAK3172_GetPreamble(RAK3172_t* p_Device, uint16_t* p_Preamble);

/** @brief          Set the P2P mode transmission power.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param Power    Transmission Preamble
 *                  NOTE: The value must be greater than 5 and lower than 22!
 *  @return         ESP_OK when successful
 *                  ESP_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
esp_err_t RAK3172_SetPower(RAK3172_t* p_Device, uint8_t Power);

/** @brief          Get the P2P mode transmission power.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param p_Power  Pointer to transmission Preamble
 *  @return         ESP_OK when successful
 *                  ESP_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
esp_err_t RAK3172_GetPower(RAK3172_t* p_Device, uint8_t* p_Power);

/** @brief          Start a LoRa P2P transmission.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param p_Buffer Pointer to data buffer
 *  @param Length   Data length
 *  @return         ESP_OK when successful
 *                  ESP_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
esp_err_t RAK3172_P2P_Transmit(RAK3172_t* p_Device, const uint8_t* p_Buffer, uint8_t Length);

/** @brief              Receive a single P2P packet.
 *                      NOTE: This is a blocking fuction!
 *  @param p_Device     Pointer to RAK3172 device object
 *  @param Timeout      Timeout in milliseconds
 *                      NOTE: Only values below 65534 are allowed!
 *  @param p_Payload    Pointer to message payload string
 *  @param p_RSSI       Pointer to RSSI value
 *  @param p_SNR        Pointer to SNR value
 *  @return             ESP_OK when successful
 *                      ESP_ERR_TIMEOUT when a timeout was received
 *                      ESP_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
esp_err_t RAK3172_P2P_Receive(RAK3172_t* p_Device, uint16_t Timeout, String* p_Payload, int8_t* p_RSSI = NULL, int8_t* p_SNR = NULL);

/** @brief          Start the listening mode to receive LoRa P2P message.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param p_Queue  Pointer to receive queue
 *  @param Timeout  Timeout in milliseconds.
 *                  NOTE: 0 will stop the receiving, 65534 will disable the timeout (only with firmware v1.0.3 and later) and 65535 will disable the timeout and the device stops when a packet was received.
 *  @return         ESP_OK when successful
 *                  ESP_FAIL when the receive task can´t get started
 *                  ESP_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
esp_err_t RAK3172_P2P_Listen(RAK3172_t* p_Device, QueueHandle_t* p_Queue, uint16_t Timeout = RAK_REC_REPEAT);

/** @brief          Stop the listening mode.
 *  @param p_Device Pointer to RAK3172 device object
 *  @return         ESP_OK when successful
 *                  ESP_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
esp_err_t RAK3172_P2P_Stop(RAK3172_t* p_Device);

/** @brief          Get the status of the device listening mode.
 *  @param p_Device Pointer to RAK3172 device object
 *  @return         #true when in listening mode
 */
bool RAK3172_P2P_isListening(RAK3172_t* p_Device);

#endif /* RAK3172_H_ */