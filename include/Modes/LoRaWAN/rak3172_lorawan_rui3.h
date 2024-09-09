 /*
 * rak3172_lorawan_rui3.h
 *
 *  Copyright (C) Daniel Kampert, 2025
 *	Website: www.kampis-elektroecke.de
 *  File info: RAK3172 LoRaWAN RUI3 driver.
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

#ifndef RAK3172_LORAWAN_RUI3_H_
#define RAK3172_LORAWAN_RUI3_H_

#include <vector>

#include "rak3172_defs.h"

/** @brief          Get the network ID of the current network.
 *  @param p_Device RAK3172 device object
 *  @param p_Enable Pointer to network ID
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument was passed
 *                  RAK3172_ERR_INVALID_STATE when the interface is not initialized
 *                  RAK3172_ERR_INVALID_MODE when the device is not initialized as LoRaWAN device. Please call \ref RAK3172_LoRaWAN_Init first
 */
RAK3172_Error_t RAK3172_LoRaWAN_GetNetID(const RAK3172_t& p_Device, std::string* const p_ID);

/** @brief          Enable / Disable the single channel mode.
 *  @param p_Device RAK3172 device object
 *  @param Enable   Enable / Disable the masked channel mode
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument was passed
 *                  RAK3172_ERR_INVALID_STATE when the interface is not initialized
 *                  RAK3172_ERR_INVALID_MODE when the device is not initialized as LoRaWAN device. Please call \ref RAK3172_LoRaWAN_Init first
 */
RAK3172_Error_t RAK3172_LoRaWAN_SetSingleChannelMode(const RAK3172_t& p_Device, bool Enable);

/** @brief          Get the status of the single channel mode.
 *  @param p_Device RAK3172 device object
 *  @param p_Enable Pointer to status of masked channel mode
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument was passed
 *                  RAK3172_ERR_INVALID_STATE when the interface is not initialized
 *                  RAK3172_ERR_INVALID_MODE when the device is not initialized as LoRaWAN device. Please call \ref RAK3172_LoRaWAN_Init first
 */
RAK3172_Error_t RAK3172_LoRaWAN_GetSingleChannelMode(const RAK3172_t& p_Device, bool* const p_Enable);

/** @brief          Enable / Disable the eight channel mode.
 *  @param p_Device RAK3172 device object
 *  @param Enable   Enable / Disable the masked channel mode
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument was passed
 *                  RAK3172_ERR_INVALID_STATE when the interface is not initialized
 *                  RAK3172_ERR_INVALID_MODE when the device is not initialized as LoRaWAN device. Please call \ref RAK3172_LoRaWAN_Init first
 */
RAK3172_Error_t RAK3172_LoRaWAN_SetEightChannelMode(const RAK3172_t& p_Device, bool Enable);

/** @brief          Get the status of the eight channel mode.
 *  @param p_Device RAK3172 device object
 *  @param p_Enable Pointer to status of masked channel mode
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument was passed
 *                  RAK3172_ERR_INVALID_STATE when the interface is not initialized
 *                  RAK3172_ERR_INVALID_MODE when the device is not initialized as LoRaWAN device. Please call \ref RAK3172_LoRaWAN_Init first
 */
RAK3172_Error_t RAK3172_LoRaWAN_GetEightChannelMode(const RAK3172_t& p_Device, bool* const p_Enable);

/** @brief          Get the RSSI value from all channels.
 *  @param p_Device RAK3172 device object
 *  @param p_RSSI   Pointer to RSSI list
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument was passed
 *                  RAK3172_ERR_INVALID_STATE when the interface is not initialized
 *                  RAK3172_ERR_INVALID_MODE when the device is not initialized as LoRaWAN device. Please call \ref RAK3172_LoRaWAN_Init first
 */
RAK3172_Error_t RAK3172_LoRaWAN_GetChannelRSSI(const RAK3172_t& p_Device, std::vector<int>* p_RSSI);

#endif /* RAK3172_LORAWAN_RUI3_H_ */