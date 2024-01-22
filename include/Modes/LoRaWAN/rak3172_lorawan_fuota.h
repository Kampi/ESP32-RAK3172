 /*
 * rak3172_lorawan_fuota.h
 *
 *  Copyright (C) Daniel Kampert, 2023
 *	Website: www.kampis-elektroecke.de
 *  File info: RAK3172 LoRaWAN FUOTA driver.
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

#ifndef RAK3172_LORAWAN_FUOTA_H_
#define RAK3172_LORAWAN_FOTA_H_

#include "rak3172_defs.h"

/** @brief              
 *                      NOTE: Make sure that you are using the latest version of the RUI3 firmware (min. 4.0.5). Otherwise the function might not work!
 *  @param p_Device     RAK3172 device object
 *  @param p_Group      (Optional) Use a multicast group
 *  @param Timeout      (Optional) Timeout in seconds
 *  @return             RAK3172_ERR_OK when successful
 *                      RAK3172_ERR_INVALID_ARG when an invalid argument was passed
 *                      RAK3172_ERR_INVALID_STATE when the interface is not initialized
 *                      RAK3172_ERR_INVALID_MODE when the device is not initialized as LoRaWAN device. Please call \ref RAK3172_LoRaWAN_Init first
 */
RAK3172_Error_t RAK3172_LoRaWAN_FUOTA_Run(RAK3172_t& p_Device, RAK3172_MC_Group_t* p_Group = NULL, uint32_t Timeout = 10);

#endif /* RAK3172_LORAWAN_FUOTA_H_ */