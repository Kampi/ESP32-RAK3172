#ifndef RAK3172_H_
#define RAK3172_H_

#include <Arduino.h>

/** @brief Hook for a custom wait function.
 */
typedef void (*RAK3172_Wait_t)(void);

/** @brief Supported operating modes.
 */
typedef enum
{
    RAK_MODE_P2P        = 0,        /**< LoRa P2P mode. */
    RAK_MODE_LORAWAN,               /**< LoRaWAN mode. */
} RAK3172_Mode_t;

/** @brief Supported bauRAK_DRates.
 */
typedef enum
{
    RAK_BAUD_4800       = 4800,     /**< Baud rate 4800. */
    RAK_BAUD_9600       = 9600,     /**< Baud rate 9600. */
    RAK_BAUD_115200     = 115200,   /**< Baud rate 115200. */
} RAK3172_Baud_t;

/** @brief Supported frequency bands.
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

/** @brief Data rate definitions.
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

/** @brief Sub band options.
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
} RAK3172_t;

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
 *  @param p_Value  Pointer to returned value.
 *                  NOTE: Only needed when a read command should be executed.
 *  @param p_Status Pointer to status string
 *                  NOTE: Can be set to #NULL when not needed.
 *  @return         ESP_OK when successful
 *                  ESP_ERR_INVALID_ARG when an invalid argument is passed into the function
 *                  ESP_FAIL when an event happens or when the status isn?t "OK"
 */
esp_err_t RAK3172_SendCommand(RAK3172_t* p_Device, String Command, String* p_Value, String* p_Status);

/** @brief              Get the version number of the RAK3172 firmware.
 *  @param p_Device     Pointer to RAK3172 device object
 *  @param p_Version    Pointer to firmware version string
 *  @return             ESP_OK when successful
 *                      ESP_ERR_INVALID_ARG when an invalid argument was passed
 *                      ESP_ERR_INVALID_STATE the when the interface is not initialized
 */
esp_err_t RAK3172_FWVersion(RAK3172_t* p_Device, String* p_Version);

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

/** @brief          Set the bauRAK_DRate of the LoRa module.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param Baud     Module bauRAK_DRate
 *  @return         ESP_OK when successful
 *                  ESP_ERR_INVALID_ARG when an invalid argument was passed
 *                  ESP_ERR_INVALID_STATE the when the interface is not initialized
 */
esp_err_t RAK3172_SetBaud(RAK3172_t* p_Device, RAK3172_Baud_t Baud);

/** @brief          Get the bauRAK_DRate of the LoRa module.
 *  @param p_Device Pointer to RAK3172 device object
 *  @return         ESP_OK when successful
 *                  ESP_ERR_INVALID_ARG when an invalid argument was passed
 *                  ESP_ERR_INVALID_STATE the when the interface is not initialized
 */
esp_err_t RAK3172_GetBaud(RAK3172_t* p_Device);

/**
 * 
 * RAK3172 LoRaWAN functions.
 * 
 */

/** @brief                  Initialize the RAK3172 SoM in LoRaWAN mode.
 *  @param p_Device         Pointer to RAK3172 device object
 *  @param TxPwr            Tx power in dB
 *  @param Retries          Number of confirmed payload retransmissions
 *  @param p_DEVEUI         LoRaWAN DEVEUI (8 Bytes)
 *  @param p_APPEUI         LoRaWAN APPEUI (16 Bytes)
 *  @param p_APPKEY         LoRaWAN APPKEY (8 Bytes)
 *  @param Band             LoRa frequency band
 *  @param Subband          LoRa sub band
 *                          NOTE: Only needed when US915, AU915 or CN470 band is used. Otherwise set it to RAK_SUB_BAND_NONE!
 *  @param Class            Device class
 *  @param UseOTAA          Use OTAA instead of ABP
 *  @param Timeout          Timeout for the device reset in seconds
 *  @return                 ESP_OK when successful
 *                          ESP_ERR_INVALID_ARG when an invalid argument was passed
 */
esp_err_t RAK3172_InitLoRaWAN(RAK3172_t* p_Device, uint8_t TxPwr, uint8_t Retries, uint8_t* p_DEVEUI, uint8_t* p_APPEUI, uint8_t* p_APPKEY, char Class, bool UseOTAA, RAK3172_Band_t Band, RAK3172_SubBand_t Subband, uint32_t Timeout = 10);

/** @brief              Initialize the RAK3172 LoRa SoM.
 *  @param p_Device     Pointer to RAK3172 device object
 *  @param p_APPEUI     Pointer to LoRaWAN APPEUI (8 Bytes)
 *  @param p_DEVEUI     Pointer to LoRaWAN DEVEUI (8 Bytes)
 *  @param p_APPKEY     Pointer to LoRaWAN APPKEY (16 Bytes)
 *  @return             ESP_OK when successful
 *                      ESP_ERR_INVALID_ARG when an invalid argument was passed
 */
esp_err_t RAK3172_SetKeys(RAK3172_t* p_Device, const uint8_t* p_DEVEUI, const uint8_t* p_APPEUI, const uint8_t* p_APPKEY);

/** @brief                  Start the joining process.
 *                          NOTE: This is a blocking function!
 *  @param p_Device         Pointer to RAK3172 device object
 *  @param Timeout          Timeout in seconds
 *                          NOTE: Set to 0 to disable the timeout function
 *  @param Attempts         No. of join attempts
 *  @param EnableAutoJoin   Enable auto join after power up
 *  @param Interval         Reattempt interval
 *  @param on_Wait          Hook for a custom wait function
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

/** @brief          Start a data transmission.
 *                  NOTE: This is a blocking function!
 *  @param p_Device Pointer to RAK3172 device object
 *  @param Port     LoRaWAN port
 *  @param p_Buffer Pointer to data buffer
 *  @param Length   Length of buffer
 *  @return         ESP_OK when successful
 *                  ESP_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
esp_err_t RAK3172_Transmit(RAK3172_t* p_Device, uint8_t Port, const uint8_t* p_Buffer, uint8_t Length);

/** @brief              Start a data transmission.
 *                      NOTE: This is a blocking function!
 *  @param p_Device     Pointer to RAK3172 device object
 *  @param Port         LoRaWAN port
 *  @param p_Buffer     Pointer to data buffer
 *  @param Length       Length of buffer
 *  @param Timeout      Receive timeout in seconds
 *  @param Confirmed    Message confirmation
 *                      NOTE: Only neccessary when payload is greater than 1024 bytes (long payload)
 *  @param Wait         Hook for a custom wait function
 *                      NOTE: The function call is time critical. Prevent long wait periods!
 *  @param Status       Pointer to last status message.
 *  @return             ESP_OK when successful
 *                      ESP_ERR_INVALID_STATE when the device is busy
 *                      ESP_FAIL when a transmission error occurs
 *                      ESP_ERR_INVALID_ARG when an invalid argument is passed into the function
 *                      ESP_ERR_INVALID_RESPONSE when a send confirmation failed
 *                      ESP_ERR_TIMEOUT when a timeout occurs
 */
esp_err_t RAK3172_Transmit(RAK3172_t* p_Device, uint8_t Port, const void* p_Buffer, uint8_t Length, uint32_t Timeout, bool Confirmed = false, RAK3172_Wait_t Wait = NULL, String* Status = NULL);

/** @brief          Read an incomming downlink message from the RAK3172.
 *                  NOTE: This is a blocking function!
 *  @param p_Device Pointer to RAK3172 device object
 *  @param Payload  Pointer to payload string
 *  @param p_RSSI   Pointer to message RSSI value
 *                  NOTE: Can be #NULL
 *  @param p_SNR    Pointer to message SNR value
 *                  NOTE: Can be #NULL
 *  @param Timeout  Receive timeout in seconds (must be greater than 1)
 *  @return         ESP_OK when successful
 *                  ESP_ERR_TIMEOUT when a receive timeout occurs
 *                  ESP_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
esp_err_t RAK3172_Receive(RAK3172_t* p_Device, String* p_Payload, int* p_RSSI = NULL, int* p_SNR = NULL, uint32_t Timeout = 3);

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

/** @brief          Get the current state of the transmission confirmation mode.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param p_Enable Pointer to confirmation mode
 *  @return         ESP_OK when successful
 *                  ESP_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
esp_err_t RAK3172_GetConfirmation(RAK3172_t* p_Device, bool* p_Enable);

/** @brief          Get the status of the confirmation mode.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param p_Enable Pointer to public network mode status
 *  @return         ESP_OK when successful
 *                  ESP_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
esp_err_t RAK3172_GetPNM(RAK3172_t* p_Device, bool* p_Enable);

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

/** @brief          Initialize the RAK3172 SoM in P2P mode.
 *  @param p_Device Pointer to RAK3172 device object
 *  @return         ESP_OK when successful
 *                  ESP_ERR_INVALID_ARG when an invalid argument was passed
 */
esp_err_t RAK3172_InitP2P(RAK3172_t* p_Device);

#endif /* RAK3172_H_ */