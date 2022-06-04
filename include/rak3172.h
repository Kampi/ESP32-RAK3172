 /*
 * rak3172.h
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

#ifndef RAK3172_H_
#define RAK3172_H_

#include "Definitions/rak3172_defs.h"
#include "Definitions/rak3172_errors.h"

#ifdef CONFIG_RAK3172_WITH_P2P
    #include "rak3172_p2p.h"
#endif

#ifdef CONFIG_RAK3172_WITH_LORAWAN
    #include "rak3172_lorawan.h"
#endif

#define RAK3172_NO_TIMEOUT                      0

/** @brief  Get the version number of the RAK3172 library.
 *  @return Library version
 */
const std::string RAK3172_LibVersion(void);

/** @brief          Initialize the receiving task and initialize the RAK3172 SoM.
 *  @param p_Device Pointer to RAK3172 device object
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument is passed into the function
 *                  RAK3172_ERR_INVALID_STATE when the serial interface can not initialized
 */
RAK3172_Error_t RAK3172_Init(RAK3172_t* p_Device);

/** @brief          Deinitialize the RAK3172 communication interface.
 *  @param p_Device Pointer to RAK3172 device object
 */
void RAK3172_Deinit(RAK3172_t* p_Device);

/** @brief          Perform a software reset of the device.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param Timeout  (Optional) Timeout for the device reset in seconds
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument is passed into the function
 *                  RAK3172_ERR_INVALID_STATE the when the interface is not initialized
 */
RAK3172_Error_t RAK3172_SoftReset(RAK3172_t* p_Device, uint32_t Timeout = 10);

#ifdef CONFIG_RAK3172_RESET_USE_HW
    /** @brief          
     *  @param p_Device Pointer to RAK3172 device object
     *  @return         RAK3172_ERR_OK when successful
     */
    RAK3172_Error_t RAK3172_HardReset(RAK3172_t* p_Device);
#endif

/** @brief          Transmit an AT command to the RAK3172 module.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param Command  RAK3172 command
 *  @param p_Value  (Optional) Pointer to returned value.
 *  @param p_Status (Optional) Pointer to status string
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument is passed into the function
 *                  RAK3172_ERR_FAIL when an event happens, when the status is not "OK" or when the device is busy
 *                  RAK3172_ERR_TIMEOUT when a receive timeout occurs
 */
RAK3172_Error_t RAK3172_SendCommand(RAK3172_t* p_Device, std::string Command, std::string* p_Value = NULL, std::string* p_Status = NULL);

/** @brief              Get the firmware version of the RAK3172 SoM.
 *  @param p_Device     Pointer to RAK3172 device object
 *  @param p_Version    Pointer to firmware version string
 *  @return             RAK3172_ERR_OK when successful
 *                      RAK3172_ERR_INVALID_ARG when an invalid argument was passed
 *                      RAK3172_ERR_INVALID_STATE the when the interface is not initialized
 */
RAK3172_Error_t RAK3172_GetFWVersion(RAK3172_t* p_Device, std::string* p_Version);

/** @brief              Get the serial number of the RAK3172 SoM.
 *  @param p_Device     Pointer to RAK3172 device object
 *  @param p_Serial     Pointer to serial number string
 *  @return             RAK3172_ERR_OK when successful
 *                      RAK3172_ERR_INVALID_ARG when an invalid argument was passed
 *                      RAK3172_ERR_INVALID_STATE the when the interface is not initialized
 */
RAK3172_Error_t RAK3172_GetSerialNumber(RAK3172_t* p_Device, std::string* p_Serial);

/** @brief          Get the RSSI value of the last packet.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param p_RSSI   Pointer to RSSI value
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument was passed
 *                  RAK3172_ERR_INVALID_STATE the when the interface is not initialized
 */
RAK3172_Error_t RAK3172_GetRSSI(RAK3172_t* p_Device, int* p_RSSI);

/** @brief          Get the SNR value of the last packet.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param p_SNR    Pointer to SNR value
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument was passed
 *                  RAK3172_ERR_INVALID_STATE the when the interface is not initialized
 */
RAK3172_Error_t RAK3172_GetSNR(RAK3172_t* p_Device, int* p_SNR);

/** @brief          Set the current operating mode for the LoRa module.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param Mode     RAK3172 operating mode
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument was passed
 *                  RAK3172_ERR_INVALID_STATE the when the interface is not initialized
 */
RAK3172_Error_t RAK3172_SetMode(RAK3172_t* p_Device, RAK3172_Mode_t Mode);

/** @brief          Get the current operating mode for the LoRa module.
 *  @param p_Device Pointer to RAK3172 device object
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument was passed
 *                  RAK3172_ERR_INVALID_STATE the when the interface is not initialized
 */
RAK3172_Error_t RAK3172_GetMode(RAK3172_t* p_Device);

/** @brief          Set the baudrate of the LoRa module.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param Baud     Module baudrate
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument was passed
 *                  RAK3172_ERR_INVALID_STATE the when the interface is not initialized
 */
RAK3172_Error_t RAK3172_SetBaud(RAK3172_t* p_Device, RAK3172_Baud_t Baud);

/** @brief          Get the baudrate of the LoRa module.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param p_Baud   Pointer to module baudrate
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument was passed
 *                  RAK3172_ERR_INVALID_STATE the when the interface is not initialized
 */
RAK3172_Error_t RAK3172_GetBaud(RAK3172_t* p_Device, RAK3172_Baud_t* p_Baud);

#endif /* RAK3172_H_ */