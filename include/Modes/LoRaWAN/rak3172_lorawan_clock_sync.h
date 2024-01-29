 /*
 * rak3172_lorawan_clock_sync.h
 *
 *  Copyright (C) Daniel Kampert, 2023
 *	Website: www.kampis-elektroecke.de
 *  File info: RAK3172 LoRaWAN clock synchronization driver.
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

#ifndef RAK3172_LORAWAN_CLOCK_SYNC_H_
#define RAK3172_LORAWAN_CLOCK_SYNC_H_

#include "rak3172_defs.h"

/** @brief              Request a clock correction by the server.
 *  @param p_Device     RAK3172 device object
 *  @param p_DateTime   Pointer to datetime object
 *  @param AnsRequired  An answer from the server is required
 *  @param p_Group      (Optional) Use a multicast group instead of a unicast
 *  @param Timeout      (Optional) Response timeout from the server in seconds
 *  @return             RAK3172_ERR_OK when successful
 *                      RAK3172_ERR_FAIL when the response can not be send
 *                      RAK3172_ERR_INVALID_MODE when the device does not operate in LoRaWAN mode
 *                      RAK3172_ERR_WRONG_PORT when the request was received on the wrong port
 *                      RAK3172_ERR_INVALID_ARG when the server has transmitted an invalid request
 */
RAK3172_Error_t RAK3172_LoRaWAN_Clock_SetLocalTime(RAK3172_t& p_Device, struct tm* p_DateTime, bool AnsRequired, RAK3172_MC_Group_t* p_Group = NULL, uint32_t Timeout = 10);

/** @brief          Process a package version request.
 *  @param p_Device RAK3172 device object
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_FAIL when the response can not be send
 *                  RAK3172_ERR_INVALID_MODE when the device does not operate in LoRaWAN mode
 *                  RAK3172_ERR_WRONG_PORT when the request was received on the wrong port
 *                  RAK3172_ERR_INVALID_ARG when the server has transmitted an invalid request
 */
RAK3172_Error_t RAK3172_LoRaWAN_ClockSync_PackageVersion(RAK3172_t& p_Device);

/** @brief                      Check if the server has requested a force resync.
 *  @param p_Device             RAK3172 device object
 *  @param p_NbTransmissions    Pointer to requested transmissions
 *  @param p_Group              (Optional) Use a multicast group instead of a unicast
 *  @param Timeout              (Optional) Response timeout from the server in seconds
 *  @return                     #true when the server as requested a force resync
 *                              #false otherwise
 */
bool RAK3172_LoRaWAN_ClockSync_isForceResync(RAK3172_t& p_Device, uint8_t* p_NbTransmissions, RAK3172_MC_Group_t* p_Group = NULL, uint32_t Timeout = 3);

/** @brief              Handle an app time periodicity request.
 *  @param p_Device     RAK3172 device object
 *  @param p_Period     Period encodes the periodicity of the AppTimeReq transmissions. The actual periodicity in
 *                      seconds is 128.2^Period +- rand(30) where rand(30) is a random integer in the +/-30sec
 *                      range varying with each transmission.
 *  @param Time         (Optional) Time is the current end-device’s clock time captured immediately before the transmission of the radio message.
 *                      NOTE: Only neccessary when \ref NotSupported is set to #false
 *  @param NotSupported (Optional) Set to #true to disable the fuction on this device
 *  @param Timeout      (Optional) Response timeout from the server in seconds
 *  @return             RAK3172_ERR_OK when successful
 *                      RAK3172_ERR_FAIL when the response can not be send
 *                      RAK3172_ERR_INVALID_MODE when the device does not operate in LoRaWAN mode
 *                      RAK3172_ERR_WRONG_PORT when the request was received on the wrong port
 *                      RAK3172_ERR_INVALID_ARG when the server has transmitted an invalid request
 */
RAK3172_Error_t RAK3172_LoRaWAN_ClockSync_HandlePeriodicity(RAK3172_t& p_Device, uint8_t* p_Period, uint32_t Time = 0, bool NotSupported = false, uint32_t Timeout = 3);

#endif /* RAK3172_LORAWAN_CLOCK_SYNC_H_ */