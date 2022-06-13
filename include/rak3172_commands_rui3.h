 /*
 * rak3172_commands_rui3.h
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