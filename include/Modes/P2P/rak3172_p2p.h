 /*
 * rak3172_p2p.h
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

#ifndef RAK3172_P2P_H_
#define RAK3172_P2P_H_

#include "rak3172_defs.h"

#ifdef CONFIG_RAK3172_USE_RUI3
    #include "rak3172_p2p_rui3.h"
#endif

/** @brief          Get the status of the device listening mode.
 *  @param p_Device RAK3172 device object
 *  @return         #true when in listening mode
 */
inline __attribute__((always_inline)) bool RAK3172_P2P_isListening(RAK3172_t& p_Device)
{
    return (p_Device.P2P.isRxTimeout == false);
}

/** @brief              Initialize the RAK3172 SoM in P2P mode.
 *                      NOTE: You must call RAK3172_Init first!
 *  @param p_Device     RAK3172 device object
 *  @param Frequency    Transmission frequency
 *  @param SF           Spreading factor
 *  @param Bandwidth    Transmission bandwidth
 *  @param CodeRate     Transmission coderate
 *  @param Preamble     Transmission preamble
 *                      NOTE: Value must be greater than 2 or 5 (when using RUI3)!
 *  @param Power        Transmission power in dBm
 *                      NOTE: Only values between 5 and 22 are allowed!
 *  @param Timeout      (Optional) Timeout for the device reset in seconds
 *  @return             RAK3172_ERR_OK when successful
 *                      RAK3172_ERR_INVALID_ARG when an invalid argument was passed
 *                      RAK3172_ERR_INVALID_STATE when the device is not initialized. Please call \ref RAK3172_Init first!
 */
RAK3172_Error_t RAK3172_P2P_Init(RAK3172_t& p_Device, uint32_t Frequency, RAK3172_PSF_t SF, RAK3172_BW_t Bandwidth, RAK3172_CR_t CodeRate, uint16_t Preamble, uint8_t Power, uint32_t Timeout = 10);

/** @brief          Read the P2P configuration from the device.
 *  @param p_Device RAK3172 device object
 *  @param p_Config Pointer to configuration string
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument is passed into the function
 *                  RAK3172_ERR_INVALID_MODE when the device is not initialized as P2P device. Please call \ref RAK3172_P2P_Init first
 */
RAK3172_Error_t RAK3172_P2P_GetConfig(RAK3172_t& p_Device, std::string* const p_Config);

/** @brief              Set the P2P mode frequency.
 *  @param p_Device     RAK3172 device object
 *  @param Frequency    Transmission frequency
 *  @return             RAK3172_ERR_OK when successful
 *                      RAK3172_ERR_INVALID_ARG when an invalid argument is passed into the function
 *                      RAK3172_ERR_INVALID_MODE when the device is not initialized as P2P device. Please call \ref RAK3172_P2P_Init first
 */
RAK3172_Error_t RAK3172_P2P_SetFrequency(const RAK3172_t& p_Device, uint32_t Frequency);

/** @brief              Get the P2P mode frequency.
 *  @param p_Device     RAK3172 device object
 *  @param p_Frequency  Pointer to frequency value
 *  @return             RAK3172_ERR_OK when successful
 *                      RAK3172_ERR_INVALID_ARG when an invalid argument is passed into the function
 *                      RAK3172_ERR_INVALID_MODE when the device is not initialized as P2P device. Please call \ref RAK3172_P2P_Init first
 */
RAK3172_Error_t RAK3172_P2P_GetFrequency(const RAK3172_t& p_Device, uint32_t* const p_Frequency);

/** @brief          Set the P2P mode spreading factor.
 *  @param p_Device RAK3172 device object
 *  @param SF       Spreading factor
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument is passed into the function
 *                  RAK3172_ERR_INVALID_MODE when the device is not initialized as P2P device. Please call \ref RAK3172_P2P_Init first
 */
RAK3172_Error_t RAK3172_P2P_SetSpreading(const RAK3172_t& p_Device, RAK3172_PSF_t SF);

/** @brief          Get the P2P mode spreading factor.
 *  @param p_Device RAK3172 device object
 *  @param p_SF     Pointer to spreading factor
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument is passed into the function
 */
RAK3172_Error_t RAK3172_P2P_GetSpreading(const RAK3172_t& p_Device, RAK3172_PSF_t* const p_SF);

/** @brief              Set the bandwidth for P2P mode.
 *                      NOTE: Use this function only in FSK mode!
 *  @param p_Device     RAK3172 device object
 *  @param Bandwidth    Transmission bandwidth
 *  @return             RAK3172_ERR_OK when successful
 *                      RAK3172_ERR_INVALID_ARG when an invalid argument is passed into the function
 *                      RAK3172_ERR_INVALID_MODE when the device is not initialized as P2P device. Please call \ref RAK3172_P2P_Init first
 */
RAK3172_Error_t RAK3172_P2P_SetBandwidth(const RAK3172_t& p_Device, uint32_t Bandwidth);

/** @brief              Set the bandwidth for P2P mode.
 *  @param p_Device     RAK3172 device object
 *  @param Bandwidth    Transmission bandwidth
 *  @return             RAK3172_ERR_OK when successful
 *                      RAK3172_ERR_INVALID_ARG when an invalid argument is passed into the function
 *                      RAK3172_ERR_INVALID_MODE when the device is not initialized as P2P device. Please call \ref RAK3172_P2P_Init first
 */
RAK3172_Error_t RAK3172_P2P_SetBandwidth(const RAK3172_t& p_Device, RAK3172_BW_t Bandwidth);

/** @brief              Get the P2P mode bandwidth.
 *  @param p_Device     RAK3172 device object
 *  @param p_Bandwidth  Pointer to transmission bandwith
 *  @return             RAK3172_ERR_OK when successful
 *                      RAK3172_ERR_INVALID_ARG when an invalid argument is passed into the function
 *                      RAK3172_ERR_INVALID_MODE when the device is not initialized as P2P device. Please call \ref RAK3172_P2P_Init first
 */
RAK3172_Error_t RAK3172_P2P_GetBandwidth(const RAK3172_t& p_Device, RAK3172_BW_t* const p_Bandwidth);

/** @brief          Set the P2P mode code rate.
 *  @param p_Device RAK3172 device object
 *  @param CodeRate Transmission code rate
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument is passed into the function
 *                  RAK3172_ERR_INVALID_MODE when the device is not initialized as P2P device. Please call \ref RAK3172_P2P_Init first
 */
RAK3172_Error_t RAK3172_P2P_SetCodeRate(const RAK3172_t& p_Device, RAK3172_CR_t CodeRate);

/** @brief              Get the P2P mode code rate.
 *  @param p_Device     RAK3172 device object
 *  @param p_CodeRate   Pointer to transmission code rate
 *  @return             RAK3172_ERR_OK when successful
 *                      RAK3172_ERR_INVALID_ARG when an invalid argument is passed into the function
 *                      RAK3172_ERR_INVALID_MODE when the device is not initialized as P2P device. Please call \ref RAK3172_P2P_Init first
 */
RAK3172_Error_t RAK3172_P2P_GetCodeRate(const RAK3172_t& p_Device, RAK3172_CR_t* const p_CodeRate);

/** @brief          Set the P2P mode preamble.
 *  @param p_Device RAK3172 device object
 *  @param Preamble Transmission Preamble
 *                  NOTE: The value must be greater than 2 or 5 (when using RUI3)!
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument is passed into the function
 *                  RAK3172_ERR_INVALID_MODE when the device is not initialized as P2P device. Please call \ref RAK3172_P2P_Init first
 */
RAK3172_Error_t RAK3172_P2P_SetPreamble(const RAK3172_t& p_Device, uint16_t Preamble);

/** @brief              Get the P2P mode preamble.
 *  @param p_Device     RAK3172 device object
 *  @param p_Preamble   Pointer to transmission Preamble
 *  @return             RAK3172_ERR_OK when successful
 *                      RAK3172_ERR_INVALID_ARG when an invalid argument is passed into the function
 *                      RAK3172_ERR_INVALID_MODE when the device is not initialized as P2P device. Please call \ref RAK3172_P2P_Init first
 */
RAK3172_Error_t RAK3172_P2P_GetPreamble(const RAK3172_t& p_Device, uint16_t* const p_Preamble);

/** @brief          Set the P2P mode transmission power.
 *  @param p_Device RAK3172 device object
 *  @param Power    Transmission Preamble
 *                  NOTE: The value must be greater than 5 and lower than 22!
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument is passed into the function
 *                  RAK3172_ERR_INVALID_MODE when the device is not initialized as P2P device. Please call \ref RAK3172_P2P_Init first
 */
RAK3172_Error_t RAK3172_P2P_SetPower(const RAK3172_t& p_Device, uint8_t Power);

/** @brief          Get the P2P mode transmission power.
 *  @param p_Device RAK3172 device object
 *  @param p_Power  Pointer to transmission Preamble
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument is passed into the function
 *                  RAK3172_ERR_INVALID_MODE when the device is not initialized as P2P device. Please call \ref RAK3172_P2P_Init first
 */
RAK3172_Error_t RAK3172_P2P_GetPower(const RAK3172_t& p_Device, uint8_t* const p_Power);

/** @brief          Start a LoRa P2P transmission.
 *  @param p_Device RAK3172 device object
 *  @param p_Buffer Pointer to data buffer
 *  @param Length   Data length
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument is passed into the function
 *                  RAK3172_ERR_INVALID_MODE when the device is not initialized as P2P device. Please call \ref RAK3172_P2P_Init first
 */
RAK3172_Error_t RAK3172_P2P_Transmit(const RAK3172_t& p_Device, const uint8_t* const p_Buffer, uint8_t Length);

/** @brief              Receive a single P2P packet.
 *                      NOTE: This is a blocking fuction!
 *  @param p_Device     RAK3172 device object
 *  @param p_Message    Pointer to receive message object
 *  @param Timeout      Receive timeout in milliseconds
 *                      NOTE: Only values below 65534 are allowed!
 *  @return             RAK3172_ERR_OK when successful
 *                      RAK3172_ERR_TIMEOUT when no message was received
 *                      RAK3172_ERR_INVALID_ARG when an invalid argument is passed into the function
 *                      RAK3172_ERR_INVALID_MODE when the device is not initialized as P2P device. Please call \ref RAK3172_P2P_Init first
 */
RAK3172_Error_t RAK3172_P2P_Receive(RAK3172_t& p_Device, RAK3172_Rx_t* const p_Message, uint16_t Timeout);

/** @brief              Start the listening mode to receive LoRa P2P message.
 *  @param p_Device     RAK3172 device object
 *  @param Timeout      (Optional) Timeout in milliseconds.
 *                      NOTE: 0 will stop the receiving, 65534 will disable the timeout (only with firmware v1.0.3 and later) and 65535 will disable the timeout and the device stops when a packet was received.
 *  @param CoreID       (Optional) Core ID for the listening task
 *  @param Priority     (Optional) Listening task priority
 *  @param QueueSize    (Optional) Size of the receive queue
 *  @return             RAK3172_ERR_OK when successful
 *                      RAK3172_ERR_INVALID_STATE when the receive task can´t get started
 *                      RAK3172_ERR_INVALID_ARG when an invalid argument is passed into the function
 *                      RAK3172_ERR_INVALID_MODE when the device is not initialized as P2P device. Please call \ref RAK3172_P2P_Init first
 */
RAK3172_Error_t RAK3172_P2P_Listen(RAK3172_t& p_Device, uint16_t Timeout = RAK_REC_REPEAT, uint8_t CoreID = 1, uint8_t Priority = 16, uint8_t QueueSize = 8);

/** @brief              Pop an item from the message queue.
 *  @param p_Device     RAK3172 device object
 *  @param p_Message    Pointer to message object
 *  @return             RAK3172_ERR_OK when successful
 *                      RAK3172_ERR_INVALID_ARG when an invalid argument is passed into the function
 *                      RAK3172_ERR_FAIL when no item was poped from the queue
 *                      RAK3172_ERR_INVALID_MODE when the device is not initialized as P2P device. Please call \ref RAK3172_P2P_Init first
 */
RAK3172_Error_t RAK3172_P2P_PopItem(const RAK3172_t& p_Device, RAK3172_Rx_t* const p_Message);

/** @brief          Stop the listening mode.
 *  @param p_Device RAK3172 device object
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument is passed into the function
 *                  RAK3172_ERR_INVALID_MODE when the device is not initialized as P2P device. Please call \ref RAK3172_P2P_Init first
 */
RAK3172_Error_t RAK3172_P2P_Stop(RAK3172_t& p_Device);

#endif /* RAK3172_P2P_H_ */