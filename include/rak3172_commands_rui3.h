 /*
 * rak3172_commands_rui3.h
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

#ifndef RAK3172_COMMANDS_RUI3_H_
#define RAK3172_COMMANDS_RUI3_H_

#include "Definitions/rak3172_defs.h"
#include "Definitions/rak3172_errors.h"

/** @brief              Get the version of the AT commands.
 *  @param p_Device     Pointer to RAK3172 device object
 *  @param p_Version    Pointer to firmware version string
 *  @return             RAK3172_ERR_OK when successful
 *                      RAK3172_ERR_INVALID_ARG when an invalid argument was passed
 *                      RAK3172_ERR_INVALID_STATE the when the interface is not initialized
 */
RAK3172_Error_t RAK3172_GetCLIVersion(const RAK3172_t* const p_Device, std::string* const p_Version);

/** @brief              Get the version of the API.
 *  @param p_Device     Pointer to RAK3172 device object
 *  @param p_Version    Pointer to API version string
 *  @return             RAK3172_ERR_OK when successful
 *                      RAK3172_ERR_INVALID_ARG when an invalid argument was passed
 *                      RAK3172_ERR_INVALID_STATE the when the interface is not initialized
 */
RAK3172_Error_t RAK3172_GetAPIVersion(const RAK3172_t* const p_Device, std::string* const p_Version);

/** @brief              Get the build time of the firmware.
 *  @param p_Device     Pointer to RAK3172 device object
 *  @param p_Version    Pointer to API version string
 *  @return             RAK3172_ERR_OK when successful
 *                      RAK3172_ERR_INVALID_ARG when an invalid argument was passed
 *                      RAK3172_ERR_INVALID_STATE the when the interface is not initialized
 */
RAK3172_Error_t RAK3172_GetBuildTime(const RAK3172_t* const p_Device, std::string* const p_BuildTime);

/** @brief              Get the repo information of the firmware.
 *  @param p_Device     Pointer to RAK3172 device object
 *  @param p_Version    Pointer to API version string
 *  @return             RAK3172_ERR_OK when successful
 *                      RAK3172_ERR_INVALID_ARG when an invalid argument was passed
 *                      RAK3172_ERR_INVALID_STATE the when the interface is not initialized
 */
RAK3172_Error_t RAK3172_GetRepoInfo(const RAK3172_t* const p_Device, std::string* const p_Repo);

/** @brief          Get the device model.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param p_Model  Pointer to model string
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument was passed
 *                  RAK3172_ERR_INVALID_STATE the when the interface is not initialized
 */
RAK3172_Error_t RAK3172_GetModel(const RAK3172_t* const p_Device, std::string* const p_Model);

/** @brief          Get the hardware ID.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param p_ID     Pointer to hardware ID string
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument was passed
 *                  RAK3172_ERR_INVALID_STATE the when the interface is not initialized
 */
RAK3172_Error_t RAK3172_GetHWID(const RAK3172_t* const p_Device, std::string* const p_ID);

/** @brief          Put the device into sleep mode.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param Period   Sleep period
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument was passed
 *                  RAK3172_ERR_INVALID_STATE the when the interface is not initialized
 */
RAK3172_Error_t RAK3172_GetHWID(const RAK3172_t* const p_Device, uint32_t Period);

#endif /* RAK3172_COMMANDS_RUI3_H_ */