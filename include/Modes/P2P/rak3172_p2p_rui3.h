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

#ifndef RAK3172_P2P_RUI3_H_
#define RAK3172_P2P_RUI3_H_

#include "rak3172_defs.h"

/** @brief          Enable the LoRa P2P encryption.
 *  @param p_Device RAK3172 device object
 *  @param p_Key    Pointer to encryption key (8 bytes)
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument was passed
 *                  RAK3172_ERR_INVALID_MODE when the device is not initialized as P2P device. Please call \ref RAK3172_P2P_Init first
 */
RAK3172_Error_t RAK3172_P2P_EnableEncryption(RAK3172_t& p_Device, const RAK3172_EncryptKey_t p_Key);

/** @brief          Disable the LoRa P2P encryption.
 *  @param p_Device RAK3172 device object
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_MODE when the device is not initialized as P2P device. Please call \ref RAK3172_P2P_Init first
 */
RAK3172_Error_t RAK3172_P2P_DisableEncryption(RAK3172_t& p_Device);

/** @brief              Get the status of the LoRa P2P encryption.
 *  @param p_Device     RAK3172 device object
 *  @param p_Enabled    Pointer to encryption status
 *  @return             RAK3172_ERR_OK when successful
 *                      RAK3172_ERR_INVALID_MODE when the device is not initialized as P2P device. Please call \ref RAK3172_P2P_Init first
 */
RAK3172_Error_t RAK3172_P2P_isEncryptionEnabled(const RAK3172_t& p_Device, bool* const p_Enabled);

#endif /* RAK3172_P2P_RUI3_H_ */