 /*
 * rak3172_lorawan_class_b.h
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

#ifndef RAK3172_LORAWAN_CLASS_B_H_
#define RAK3172_LORAWAN_CLASS_B_H_

#include <time.h>

#include "rak3172_defs.h"

/** @brief              Get the current beacon (default broadcast) frequency.
 *  @param p_Device     RAK3172 device object
 *  @param p_Datarate   Pointer to currently used data rate
 *  @param p_Frequency  Pointer to beacon frequency
 *  @return             RAK3172_ERR_OK when successful
 *                      RAK3172_ERR_INVALID_ARG when an invalid argument was passed
 *                      RAK3172_ERR_INVALID_STATE the when the interface is not initialized
 *                      RAK3172_ERR_INVALID_MODE when the device is not initialized as LoRaWAN device. Please call \ref RAK3172_LoRaWAN_Init first
 */
RAK3172_Error_t RAK3172_LoRaWAN_GetBeaconFrequency(RAK3172_t& p_Device, RAK3172_DataRate_t* p_Datarate, uint32_t* p_Frequency);

#ifdef CONFIG_RAK3172_USE_RUI3
    /** @brief          Get the current beacon time.
     *  @param p_Device RAK3172 device object
     *  @param p_Time   Pointer to beacon time
     *  @return         RAK3172_ERR_OK when successful
     *                  RAK3172_ERR_INVALID_ARG when an invalid argument was passed
     *                  RAK3172_ERR_INVALID_STATE the when the interface is not initialized
     *                  RAK3172_ERR_INVALID_MODE when the device is not initialized as LoRaWAN device. Please call \ref RAK3172_LoRaWAN_Init first
     */
    RAK3172_Error_t RAK3172_LoRaWAN_GetBeaconTime(RAK3172_t& p_Device, uint32_t* p_Time);

    /** @brief              Get the Gateway GPS coordinate, NetID, and GwID.
     *  @param p_Device     RAK3172 device object
     *  @param p_NetID      Pointer to network ID
     *  @param p_GatewayID  Pointer to gateway ID
     *  @param p_Longitude  Pointer to GPS longitude
     *  @param p_Latitude   Pointer to GPS latitude
     *  @return             RAK3172_ERR_OK when successful
     *                      RAK3172_ERR_INVALID_ARG when an invalid argument was passed
     *                      RAK3172_ERR_INVALID_STATE the when the interface is not initialized
     *                      RAK3172_ERR_INVALID_MODE when the device is not initialized as LoRaWAN device. Please call \ref RAK3172_LoRaWAN_Init first
     */
    RAK3172_Error_t RAK3172_LoRaWAN_GetGatewayInfo(RAK3172_t& p_Device, std::string* p_NetID, std::string* p_GatewayID, std::string* p_Longitude, std::string* p_Latitude);
#endif

/** @brief              Get the local time.
 *  @param p_Device     RAK3172 device object
 *  @param p_DateTime   Pointer to date and time structure
 *  @return             RAK3172_ERR_OK when successful
 *                      RAK3172_ERR_INVALID_ARG when an invalid argument was passed
 *                      RAK3172_ERR_INVALID_STATE the when the interface is not initialized
 *                      RAK3172_ERR_INVALID_MODE when the device is not initialized as LoRaWAN device. Please call \ref RAK3172_LoRaWAN_Init first
 */
RAK3172_Error_t RAK3172_LoRaWAN_GetLocalTime(RAK3172_t& p_Device, struct tm* p_DateTime);

/** @brief              Set the unicast ping slot periodicity.
 *  @param p_Device     RAK3172 device object
 *  @param Periodicity  Ping slot periodicity.
 *                      NOTE: Only values <8 are allowed.
 *                      0 means that the end device opens a ping slot approximately every second during the beacon_window interval.
 *                      7 means every 128 seconds, which is the maximum ping period.
 *  @return             RAK3172_ERR_OK when successful
 *                      RAK3172_ERR_INVALID_ARG when an invalid argument was passed
 *                      RAK3172_ERR_INVALID_STATE the when the interface is not initialized
 *                      RAK3172_ERR_INVALID_MODE when the device is not initialized as LoRaWAN device. Please call \ref RAK3172_LoRaWAN_Init first
 */
RAK3172_Error_t RAK3172_LoRaWAN_SetPeriodicity(RAK3172_t& p_Device, uint8_t Periodicity);

/** @brief                  Get the unicast ping slot periodicity.
 *  @param p_Device         RAK3172 device object
 *  @param p_Periodicity    Pointer to ping slot periodicity.
 *  @return                 RAK3172_ERR_OK when successful
 *                          RAK3172_ERR_INVALID_ARG when an invalid argument was passed
 *                          RAK3172_ERR_INVALID_STATE the when the interface is not initialized
 *                          RAK3172_ERR_INVALID_MODE when the device is not initialized as LoRaWAN device. Please call \ref RAK3172_LoRaWAN_Init first
 */
RAK3172_Error_t RAK3172_LoRaWAN_GetPeriodicity(RAK3172_t& p_Device, uint8_t* p_Periodicity);

#endif /* RAK3172_LORAWAN_CLASS_B_H_ */