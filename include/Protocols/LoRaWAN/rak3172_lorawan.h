 /*
 * rak3172_lorawan.h
 *
 *  Copyright (C) Daniel Kampert, 2022
 *	Website: www.kampis-elektroecke.de
 *  File info: RAK3172 driver for ESP32.
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

#ifndef RAK3172_LORAWAN_H_
#define RAK3172_LORAWAN_H_

#include "Definitions/rak3172_defs.h"
#include "Definitions/rak3172_errors.h"

#ifdef CONFIG_RAK3172_USE_RUI3
    #include "rak3172_lorawan_rui3.h"
#endif

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
 *  @param Class    Device class (A, B or C)
 *  @param Band     LoRa frequency band
 *  @param Subband  (Optional) LoRa sub band
 *                  NOTE: Only needed when US915, AU915 or CN470 band is used. Otherwise set it to RAK_SUB_BAND_NONE!
 *  @param UseADR   (Optional) Enable adaptive data rate
 *  @param Timeout  (Optional) Timeout for the device reset in seconds
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument was passed
 */
RAK3172_Error_t RAK3172_LoRaWAN_Init(RAK3172_t* const p_Device, uint8_t TxPwr, uint8_t Retries, RAK3172_JoinMode_t JoinMode, const uint8_t* const p_Key1, const uint8_t* const p_Key2, const uint8_t* const p_Key3, char Class, RAK3172_Band_t Band, RAK3172_SubBand_t Subband = RAK_SUB_BAND_NONE, bool UseADR = true, uint32_t Timeout = 10);

/** @brief              Set the keys for OTAA mode.
 *  @param p_Device     Pointer to RAK3172 device object
 *  @param p_DEVEUI     Pointer to LoRaWAN DEVEUI (8 Bytes)
 *  @param p_APPEUI     Pointer to LoRaWAN APPEUI (8 Bytes)
 *  @param p_APPKEY     Pointer to LoRaWAN APPKEY (16 Bytes)
 *  @return             RAK3172_ERR_OK when successful
 *                      RAK3172_ERR_INVALID_ARG when an invalid argument was passed
 */
RAK3172_Error_t RAK3172_LoRaWAN_SetOTAAKeys(const RAK3172_t* const p_Device, const uint8_t* const p_DEVEUI, const uint8_t* const p_APPEUI, const uint8_t* const p_APPKEY);

/** @brief              Set the keys for ABP mode.
 *  @param p_Device     Pointer to RAK3172 device object
 *  @param p_APPSKEY    Pointer to LoRaWAN APPSKEY (16 Bytes)
 *  @param p_NWKSKEY    Pointer to LoRaWAN NWKSKEY (16 Bytes)
 *  @param p_DEVADDR    Pointer to LoRaWAN DEVADDR (4 Bytes)
 *  @return             RAK3172_ERR_OK when successful
 *                      RAK3172_ERR_INVALID_ARG when an invalid argument was passed
 */
RAK3172_Error_t RAK3172_LoRaWAN_SetABPKeys(const RAK3172_t* const p_Device, const uint8_t* const p_APPSKEY, const uint8_t* const p_NWKSKEY, const uint8_t* const p_DEVADDR);

/** @brief                  Start the joining process.
 *                          NOTE: This is a blocking function!
 *  @param p_Device         Pointer to RAK3172 device object
 *  @param Timeout          (Optional) Timeout in seconds
 *                          NOTE: Set to 0 to disable the timeout function
 *  @param Attempts         (Optional) No. of join attempts
 *  @param EnableAutoJoin   (Optional) Enable auto join after power up
 *  @param Interval         (Optional) Reattempt interval
 *  @param on_Wait          (Optional) Hook for a custom wait function
 *                          NOTE: The function call is time critical. Prevent long wait periods!
 *  @return                 RAK3172_ERR_OK when joined
 *                          RAK3172_ERR_FAIL when a transmission error occurs
 *                          RAK3172_ERR_TIMEOUT when the number of JOIN attemps has expired
 *                          RAK3172_ERR_INVALID_ARG when an invalid argument was passed
 */
RAK3172_Error_t RAK3172_LoRaWAN_StartJoin(RAK3172_t* const p_Device, uint32_t Timeout = 0, uint8_t Attempts = 5, bool EnableAutoJoin = false, uint8_t Interval = 10, RAK3172_Wait_t on_Wait = NULL);

/** @brief          Stop the joining process.
 *  @param p_Device Pointer to RAK3172 device object
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
RAK3172_Error_t RAK3172_LoRaWAN_StopJoin(const RAK3172_t* const p_Device);

/** @brief          Check if the module has joined the network.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param p_Joined Pointer to joining state. #true when joined
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument was passed
 *                  RAK3172_ERR_FAIL when a transmission error occurs
 */
RAK3172_Error_t RAK3172_LoRaWAN_isJoined(RAK3172_t* const p_Device, bool* const p_Joined);

/** @brief          Start a LoRaWAN data transmission.
 *                  NOTE: This is a blocking function!
 *  @param p_Device Pointer to RAK3172 device object
 *  @param Port     LoRaWAN port
 *  @param p_Buffer Pointer to data buffer
 *  @param Length   Length of buffer
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_STATE when the device is busy
 *                  RAK3172_ERR_FAIL when a transmission error occurs
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument is passed into the function
 *                  RAK3172_ERR_INVALID_RESPONSE when a send confirmation failed or when the device is busy
 *                  RAK3172_ERR_TIMEOUT when a transmit timeout occurs
 *                  RAK3172_ERR_INVALID_STATE when the device is not joined
 */
RAK3172_Error_t RAK3172_LoRaWAN_Transmit(RAK3172_t* const p_Device, uint8_t Port, const uint8_t* const p_Buffer, uint8_t Length);

/** @brief              Start a LoRaWAN data transmission.
 *                      NOTE: This is a blocking function!
 *  @param p_Device     Pointer to RAK3172 device object
 *  @param Port         LoRaWAN port
 *  @param p_Buffer     Pointer to data buffer
 *  @param Length       Data buffer length
 *  @param Timeout      Receive timeout in seconds
 *  @param Confirmed    (Optional) Enable message confirmation
 *                      NOTE: Only neccessary when payload is greater than 1024 bytes (long payload)
 *  @param Wait         (Optional) Hook for a custom wait function
 *                      NOTE: The function call is time critical. Prevent long wait periods!
 *  @return             RAK3172_ERR_OK when successful
 *                      RAK3172_ERR_INVALID_STATE when the device is busy
 *                      RAK3172_ERR_FAIL when a transmission error occurs
 *                      RAK3172_ERR_INVALID_ARG when an invalid argument is passed into the function
 *                      RAK3172_ERR_INVALID_RESPONSE when a send confirmation failed or when the device is busy
 *                      RAK3172_ERR_TIMEOUT when a transmit timeout occurs
 *                      RAK3172_ERR_INVALID_STATE when the device is not joined
 */
RAK3172_Error_t RAK3172_LoRaWAN_Transmit(RAK3172_t* const p_Device, uint8_t Port, const void* const p_Buffer, uint8_t Length, uint32_t Timeout, bool Confirmed = false, RAK3172_Wait_t Wait = NULL);

/** @brief              Check if a downlin message was received during the last uplink and get the message.
 *  @param p_Device     Pointer to RAK3172 device object
 *  @param p_Message    Pointer to RAK3172 message object
 *  @return             RAK3172_ERR_OK when successful
 *                      RAK3172_ERR_FAIL when no message is available
 */
RAK3172_Error_t RAK3172_LoRaWAN_Receive(RAK3172_t* const p_Device, RAK3172_Rx_t* p_Message);

/** @brief          Set the number of confirmed payload retransmissions.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param Retries  Number of retries
 *                  NOTE: Only values between 0 and 7 are allowed!
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
RAK3172_Error_t RAK3172_LoRaWAN_SetRetries(const RAK3172_t* const p_Device, uint8_t Retries);

/** @brief              Get the number of confirmed payload retransmissions.
 *  @param p_Device     Pointer to RAK3172 device object
 *  @param p_Retries    Pointer to number of retries
 *  @return             RAK3172_ERR_OK when successful
 *                      RAK3172_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
RAK3172_Error_t RAK3172_LoRaWAN_GetRetries(const RAK3172_t* const p_Device, uint8_t* const p_Retries);

/** @brief          Enable / Disable the usage of the public network mode.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param Enable   Enable / Disable the mode
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
RAK3172_Error_t RAK3172_LoRaWAN_SetPNM(const RAK3172_t* const p_Device, bool Enable);

/** @brief          Get the status of the public network mode.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param p_Enable Pointer to public network mode status
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
RAK3172_Error_t RAK3172_LoRaWAN_GetPNM(const RAK3172_t* const p_Device, bool* const p_Enable);

/** @brief          Enable / Disable the usage of the confirmation mode.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param Enable   Enable / Disable the mode
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
RAK3172_Error_t RAK3172_LoRaWAN_SetConfirmation(const RAK3172_t* const p_Device, bool Enable);

/** @brief          Get the current state of the transmission confirmation mode.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param p_Enable Pointer to confirmation mode
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
RAK3172_Error_t RAK3172_LoRaWAN_GetConfirmation(const RAK3172_t* const p_Device, bool* const p_Enable);

/** @brief          Set the used frequency band.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param Band     Pointer to frequency band
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
RAK3172_Error_t RAK3172_LoRaWAN_SetBand(const RAK3172_t* const p_Device, RAK3172_Band_t Band);

/** @brief          Get the used frequency band.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param p_Band   Pointer to frequency band
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
RAK3172_Error_t RAK3172_LoRaWAN_GetBand(const RAK3172_t* const p_Device, RAK3172_Band_t* const p_Band);

/** @brief          Set the sub band for the LoRaWAN communication.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param Band     Target sub band
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument is passed into the function
 *                  RAK3172_ERR_INVALID_RESPONSE when the device is operating in the wrong frequency band
 */
RAK3172_Error_t RAK3172_LoRaWAN_SetSubBand(const RAK3172_t* const p_Device, RAK3172_SubBand_t Band);

/** @brief          Get the sub band for the LoRaWAN communication.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param Band     Pointer to frequency sub band
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
RAK3172_Error_t RAK3172_LoRaWAN_GetSubBand(const RAK3172_t* const p_Device, RAK3172_SubBand_t* const p_Band);

/** @brief          Set the Tx power of the RAK3172.
 *                  NOTE: The index of the Tx power and the resulting power depends on the selected region!
 *  @param p_Device Pointer to RAK3172 device object
 *  @param TxPwr    Tx power index in dBm
 *                  NOTE: The power must be a power of two!
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
RAK3172_Error_t RAK3172_LoRaWAN_SetTxPwr(const RAK3172_t* const p_Device, uint8_t TxPwr);

/** @brief          Set the join delay on the RX window 1.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param Delay    RX1 delay in seconds (1 - 14)
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
RAK3172_Error_t RAK3172_LoRaWAN_SetJoin1Delay(const RAK3172_t* const p_Device, uint8_t Delay);

/** @brief          Get the join delay on the RX window 1.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param p_Delay  Pointer to R1 delay in seconds
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
RAK3172_Error_t RAK3172_LoRaWAN_GetJoin1Delay(const RAK3172_t* const p_Device, uint8_t* const p_Delay);

/** @brief          Set the join delay on the RX window 2.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param Delay    RX2 delay in seconds (2 - 15)
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
RAK3172_Error_t RAK3172_LoRaWAN_SetJoin2Delay(const RAK3172_t* const p_Device, uint8_t Delay);

/** @brief          Get the join delay on the RX window 2.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param p_Delay  Pointer to RX2 delay in seconds
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
RAK3172_Error_t RAK3172_LoRaWAN_GetJoin2Delay(const RAK3172_t* const p_Device, uint8_t* const p_Delay);

/** @brief          Set the delay of RX window 1.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param Delay    RX1 delay in seconds (1 - 15)
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
RAK3172_Error_t RAK3172_LoRaWAN_SetRX1Delay(const RAK3172_t* const p_Device, uint8_t Delay);

/** @brief          Set the delay of RX window 1.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param p_Delay  Pointer to RX1 delay in seconds
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
RAK3172_Error_t RAK3172_LoRaWAN_GetRX1Delay(const RAK3172_t* const p_Device, uint8_t* const p_Delay);

/** @brief          Set the delay of RX window 2.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param Delay    RX2 delay in seconds (2 - 16)
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
RAK3172_Error_t RAK3172_LoRaWAN_SetRX2Delay(const RAK3172_t* const p_Device, uint8_t Delay);

/** @brief          Set the delay of RX window 2.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param p_Delay  Pointer to RX2 delay in seconds
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
RAK3172_Error_t RAK3172_LoRaWAN_GetRX2Delay(const RAK3172_t* const p_Device, uint8_t* const p_Delay);

/** @brief          Get the SNR value of the last packet.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param p_SNR    Pointer to SNR value
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
RAK3172_Error_t RAK3172_LoRaWAN_GetSNR(const RAK3172_t* const p_Device, int8_t* const p_SNR);

/** @brief          Get the RSSI value of the last packet.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param p_RSSI   Pointer to RSSI value
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
RAK3172_Error_t RAK3172_LoRaWAN_GetRSSI(const RAK3172_t* const p_Device, int8_t* p_RSSI);

/** @brief          Get the duty time.
 *                  NOTE: This can only be used when using EU868, RU864 or EU433 frequency band!
 *  @param p_Device Pointer to RAK3172 device object
 *  @param p_Duty   Pointer to duty cycle time im seconds
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument is passed into the function
 *                  RAK3172_ERR_INVALID_RESPONSE when the device is operating in the wrong frequency band
 */
RAK3172_Error_t RAK3172_LoRaWAN_GetDuty(const RAK3172_t* const p_Device, uint8_t* const p_Duty);

/** @brief          Set the data rate of the LoRa module.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param DR       Data rate
 *  @return         ESP_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument was passed
 *                  RAK3172_ERR_INVALID_STATE the when the interface is not initialized
 */
RAK3172_Error_t RAK3172_LoRaWAN_SetDataRate(const RAK3172_t* const p_Device, RAK3172_DataRate_t DR);

/** @brief          Get the data rate of the LoRa module.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param p_DR     Pointer to data rate
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument was passed
 *                  RAK3172_ERR_INVALID_STATE the when the interface is not initialized
 */
RAK3172_Error_t RAK3172_LoRaWAN_GetDataRate(const RAK3172_t* const p_Device, RAK3172_DataRate_t* const p_DR);

/** @brief          Enable / Disable the adaptive data rate.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param Enable   Enable / Disable ADR
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument was passed
 *                  RAK3172_ERR_INVALID_STATE the when the interface is not initialized
 */
RAK3172_Error_t RAK3172_LoRaWAN_SetADR(const RAK3172_t* const p_Device, bool Enable);

/** @brief          Get the status of the adaptive data rate option.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param p_Enable Pointer to adaptive data rate status
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument was passed
 *                  RAK3172_ERR_INVALID_STATE the when the interface is not initialized
 */
RAK3172_Error_t RAK3172_LoRaWAN_GetADR(const RAK3172_t* const p_Device, bool* const p_Enable);

/** @brief          Set the LoRaWAN join mode.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param Mode     Join mode
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument was passed
 *                  RAK3172_ERR_INVALID_STATE the when the interface is not initialized
 */
RAK3172_Error_t RAK3172_LoRaWAN_SetJoinMode(const RAK3172_t* const p_Device, RAK3172_JoinMode_t Mode);

/** @brief          Get the current LoRaWAN join mode.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param p_Mode   Pointer to join mode
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument was passed
 *                  RAK3172_ERR_INVALID_STATE the when the interface is not initialized
 */
RAK3172_Error_t RAK3172_LoRaWAN_GetJoinMode(const RAK3172_t* const p_Device, RAK3172_JoinMode_t* const p_Mode);

#endif /* RAK3172_LORAWAN_H_ */