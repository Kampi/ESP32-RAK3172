 /*
 * rak3172_lorawan_multicast.h
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

#ifndef RAK3172_LORAWAN_MULTICAST_H_
#define RAK3172_LORAWAN_MULTICAST_H_

#include "rak3172_defs.h"

/** @brief          Add a multicast group.
 *  @param p_Device RAK3172 device object
 *  @param Group    Multicast group object
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument was passed
 *                  RAK3172_ERR_INVALID_STATE the when the interface is not initialized
 *                  RAK3172_ERR_INVALID_MODE when the device is not initialized as LoRaWAN device. Please call \ref RAK3172_LoRaWAN_Init first
 */
RAK3172_Error_t RAK3172_LoRaWAN_MC_AddGroup(RAK3172_t& p_Device, RAK3172_MC_Group_t Group);

/** @brief              Add a multicast group.
 *  @param p_Device     RAK3172 device object
 *  @param Class        LoRaWAN device class
 *  @param DevAddr      Multicast device address as hex string
 *  @param NwkSKey      NWK session key
 *  @param AppSKey      APP session key
 *  @param Frequency    Frequency used by this multicast group in Hz
 *  @param Datarate     Data rate used by this multicast group
 *  @param Periodicity  Ping slot periodicity
 *                      NOTE: This value will be ignored when class is set to 'C'!
 *  @return             RAK3172_ERR_OK when successful
 *                      RAK3172_ERR_INVALID_ARG when an invalid argument was passed
 *                      RAK3172_ERR_INVALID_STATE the when the interface is not initialized
 *                      RAK3172_ERR_INVALID_MODE when the device is not initialized as LoRaWAN device. Please call \ref RAK3172_LoRaWAN_Init first
 */
RAK3172_Error_t RAK3172_LoRaWAN_MC_AddGroup(RAK3172_t& p_Device, RAK3172_Class_t Class, std::string DevAddr, std::string NwkSKey, std::string AppSKey, uint32_t Frequency, RAK3172_DataRate_t Datarate, uint8_t Periodicity = 0);

/** @brief          Remove a multicast group.
 *  @param p_Device RAK3172 device object
 *  @param Group    Multicast group object
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument was passed
 *                  RAK3172_ERR_INVALID_STATE the when the interface is not initialized
 *                  RAK3172_ERR_INVALID_MODE when the device is not initialized as LoRaWAN device. Please call \ref RAK3172_LoRaWAN_Init first
 */
RAK3172_Error_t RAK3172_LoRaWAN_MC_RemoveGroup(RAK3172_t& p_Device, RAK3172_MC_Group_t Group);

/** @brief          Remove a multicast group.
 *  @param p_Device RAK3172 device object
 *  @param Datarate Data rate used by this multicast group
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument was passed
 *                  RAK3172_ERR_INVALID_STATE the when the interface is not initialized
 *                  RAK3172_ERR_INVALID_MODE when the device is not initialized as LoRaWAN device. Please call \ref RAK3172_LoRaWAN_Init first
 */
RAK3172_Error_t RAK3172_LoRaWAN_MC_RemoveGroup(RAK3172_t& p_Device, std::string DevAddr);

/** @brief          Get the configured multicast group.
 *  @param p_Device RAK3172 device object
 *  @param p_Group  Pointer to multicast group object
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument was passed
 *                  RAK3172_ERR_INVALID_STATE the when the interface is not initialized
 *                  RAK3172_ERR_INVALID_MODE when the device is not initialized as LoRaWAN device. Please call \ref RAK3172_LoRaWAN_Init first
 */
RAK3172_Error_t RAK3172_LoRaWAN_MC_ListGroup(RAK3172_t& p_Device, RAK3172_MC_Group_t* p_Group);

#endif /* RAK3172_LORAWAN_MULTICAST_H_ */