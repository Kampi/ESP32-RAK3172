 /*
 * rak3172_p2p.h
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

#ifndef RAK3172_P2P_H_
#define RAK3172_P2P_H_

#include "Definitions/rak3172_defs.h"
#include "Definitions/rak3172_errors.h"

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
 *  @param Timeout      (Optional) Timeout for the device reset in seconds
 *  @return             RAK3172_ERR_OK when successful
 *                      RAK3172_ERR_INVALID_ARG when an invalid argument was passed
 */
RAK3172_Error_t RAK3172_P2P_Init(RAK3172_t* const p_Device, uint32_t Frequency, RAK3172_PSF_t Spread, RAK3172_BW_t Bandwidth, RAK3172_CR_t CodeRate, uint16_t Preamble, uint8_t Power, uint32_t Timeout = 10);

/** @brief          Read the P2P configuration from the device.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param p_Config Pointer to configuration string
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
RAK3172_Error_t RAK3172_P2P_GetConfig(const RAK3172_t* const p_Device, std::string* const p_Config);

/** @brief          Set the P2P mode frequency.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param Freq     Transmission frequency
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
RAK3172_Error_t RAK3172_P2P_SetFrequency(const RAK3172_t* const p_Device, uint32_t Freq);

/** @brief          Get the P2P mode frequency.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param p_Freq   Pointer to frequency value
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
RAK3172_Error_t RAK3172_P2P_GetFrequency(const RAK3172_t* const p_Device, uint32_t* p_Freq);

/** @brief          Set the P2P mode spreading factor.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param Spread   Spreading factor
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
RAK3172_Error_t RAK3172_P2P_SetSpreading(const RAK3172_t* const p_Device, RAK3172_PSF_t Spread);

/** @brief          Get the P2P mode spreading factor.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param p_Spread Pointer to spreading factor
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
RAK3172_Error_t RAK3172_P2P_GetSpreading(const RAK3172_t* const p_Device, RAK3172_PSF_t* p_Spread);

/** @brief              Set the P2P mode bandwidth.
 *  @param p_Device     Pointer to RAK3172 device object
 *  @param Bandwidth    Transmission bandwidth
 *  @return             RAK3172_ERR_OK when successful
 *                      RAK3172_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
RAK3172_Error_t RAK3172_P2P_SetBandwidth(const RAK3172_t* const p_Device, RAK3172_BW_t Bandwidth);

/** @brief              Get the P2P bandwidth.
 *  @param p_Device     Pointer to RAK3172 device object
 *  @param p_Bandwidth  Pointer to transmission bandwith
 *  @return             RAK3172_ERR_OK when successful
 *                      RAK3172_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
RAK3172_Error_t RAK3172_P2P_GetBandwidth(const RAK3172_t* const p_Device, RAK3172_BW_t* p_Bandwidth);

/** @brief          Set the P2P mode code rate.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param CodeRate Transmission code rate
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
RAK3172_Error_t RAK3172_P2P_SetCodeRate(const RAK3172_t* const p_Device, RAK3172_CR_t CodeRate);

/** @brief              Get the P2P mode code rate.
 *  @param p_Device     Pointer to RAK3172 device object
 *  @param p_CodeRate   Pointer to transmission code rate
 *  @return             RAK3172_ERR_OK when successful
 *                      RAK3172_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
RAK3172_Error_t RAK3172_P2P_GetCodeRate(const RAK3172_t* const p_Device, RAK3172_CR_t* p_CodeRate);

/** @brief          Set the P2P mode preamble.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param Preamble Transmission Preamble
 *                  NOTE: The value must be greater than 2!
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
RAK3172_Error_t RAK3172_P2P_SetPreamble(const RAK3172_t* const p_Device, uint16_t Preamble);

/** @brief              Get the P2P mode preamble.
 *  @param p_Device     Pointer to RAK3172 device object
 *  @param p_Preamble   Pointer to transmission Preamble
 *  @return             RAK3172_ERR_OK when successful
 *                      RAK3172_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
RAK3172_Error_t RAK3172_P2P_GetPreamble(const RAK3172_t* const p_Device, uint16_t* p_Preamble);

/** @brief          Set the P2P mode transmission power.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param Power    Transmission Preamble
 *                  NOTE: The value must be greater than 5 and lower than 22!
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
RAK3172_Error_t RAK3172_P2P_SetPower(const RAK3172_t* const p_Device, uint8_t Power);

/** @brief          Get the P2P mode transmission power.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param p_Power  Pointer to transmission Preamble
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
RAK3172_Error_t RAK3172_P2P_GetPower(const RAK3172_t* const p_Device, uint8_t* const p_Power);

/** @brief          Start a LoRa P2P transmission.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param p_Buffer Pointer to data buffer
 *  @param Length   Data length
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
RAK3172_Error_t RAK3172_P2P_Transmit(RAK3172_t* const p_Device, const uint8_t* const p_Buffer, uint8_t Length);

/** @brief              Receive a single P2P packet.
 *                      NOTE: This is a blocking fuction!
 *  @param p_Device     Pointer to RAK3172 device object
 *  @param Timeout      Timeout in milliseconds
 *                      NOTE: Only values below 65534 are allowed!
 *  @param p_Payload    Pointer to message payload string
 *  @param p_RSSI       (Optional) Pointer to RSSI value
 *  @param p_SNR        (Optional) Pointer to SNR value
 *  @param Listen       (Optional) Listen timeout in milliseconds
 *  @return             RAK3172_ERR_OK when successful
 *                      RAK3172_ERR_TIMEOUT when a timeout was received
 *                      RAK3172_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
RAK3172_Error_t RAK3172_P2P_Receive(RAK3172_t* const p_Device, uint16_t Timeout, std::string* const p_Payload, int8_t* p_RSSI = NULL, int8_t* p_SNR = NULL, uint32_t Listen = 100);

/** @brief          Start the listening mode to receive LoRa P2P message.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param p_Queue  Pointer to receive queue with RAK3172_Rx_t* objects
 *                  NOTE: Make sure you perform a "delete" on these objects after the operation to avoid memory leaks!
 *  @param Timeout  (Optional) Timeout in milliseconds.
 *                  NOTE: 0 will stop the receiving, 65534 will disable the timeout (only with firmware v1.0.3 and later) and 65535 will disable the timeout and the device stops when a packet was received.
 *  @param CoreID   (Optional) Core ID for the listening task
 *  @param Priority (Optional) Listening task priority
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_STATE when the receive task can´t get started
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
RAK3172_Error_t RAK3172_P2P_Listen(RAK3172_t* const p_Device, QueueHandle_t* p_Queue, uint16_t Timeout = RAK_REC_REPEAT, uint8_t CoreID = 1, uint8_t Priority = 16);

/** @brief          Stop the listening mode.
 *  @param p_Device Pointer to RAK3172 device object
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
RAK3172_Error_t RAK3172_P2P_Stop(RAK3172_t* const p_Device);

/** @brief          Get the status of the device listening mode.
 *  @param p_Device Pointer to RAK3172 device object
 *  @return         #true when in listening mode
 */
bool RAK3172_P2P_isListening(const RAK3172_t* const p_Device);

#endif /* RAK3172_P2P_H_ */