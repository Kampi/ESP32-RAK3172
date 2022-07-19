 /*
 * rak3172.h
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

#ifndef RAK3172_H_
#define RAK3172_H_

#include "Definitions/rak3172_defs.h"
#include "Definitions/rak3172_errors.h"

#ifdef CONFIG_RAK3172_WITH_P2P
    #include "Protocols/P2P/rak3172_p2p.h"
#endif

#ifdef CONFIG_RAK3172_WITH_LORAWAN
    #include "Protocols/LoRaWAN/rak3172_lorawan.h"
#endif

#ifdef CONFIG_RAK3172_USE_RUI3
    #include "rak3172_commands_rui3.h"
#endif

/** @brief  Get the version number of the RAK3172 library.
 *  @return Library version
 */
const inline __attribute__((always_inline)) std::string RAK3172_LibVersion(void)
{
    #if((defined RAK3172_LIB_MAJOR) && (defined RAK3172_LIB_MINOR) && (defined RAK3172_LIB_BUILD))
        return std::string(STRINGIFY(RAK3172_LIB_MAJOR)) + "." + std::string(STRINGIFY(RAK3172_LIB_MINOR)) + "." + std::string(STRINGIFY(RAK3172_LIB_BUILD));
    #else
        return "<Not defined>";
    #endif
}

/** @brief          Initialize the receiving task and initialize the RAK3172 SoM.
 *  @param p_Device Pointer to RAK3172 device object
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument is passed into the function
 *                  RAK3172_ERR_INVALID_STATE when the serial interface can not initialized
 */
RAK3172_Error_t RAK3172_Init(RAK3172_t* const p_Device);

/** @brief          Deinitialize the RAK3172 communication interface.
 *  @param p_Device Pointer to RAK3172 device object
 */
void RAK3172_Deinit(RAK3172_t* const p_Device);

/** @brief          Perform a factory reset of the device.
 *  @param p_Device Pointer to RAK3172 device object
 *  @return         RAK3172_ERR_OK when successful
 */
RAK3172_Error_t RAK3172_FactoryReset(RAK3172_t* const p_Device);

/** @brief          Perform a software reset of the device.
 *  @param p_Device Pointer to RAK3172 device object
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument is passed into the function
 *                  RAK3172_ERR_INVALID_STATE the when the interface is not initialized
 */
RAK3172_Error_t RAK3172_SoftReset(RAK3172_t* const p_Device);

#ifdef CONFIG_RAK3172_RESET_USE_HW
    /** @brief          Perform a hardware reset of the device.
     *  @param p_Device Pointer to RAK3172 device object
     *  @return         RAK3172_ERR_OK when successful
     */
    RAK3172_Error_t RAK3172_HardReset(RAK3172_t* const p_Device);
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
RAK3172_Error_t RAK3172_SendCommand(const RAK3172_t* const p_Device, std::string Command, std::string* const p_Value = NULL, std::string* const p_Status = NULL);

/** @brief              Get the firmware version of the RAK3172 SoM.
 *  @param p_Device     Pointer to RAK3172 device object
 *  @param p_Version    Pointer to firmware version string
 *  @return             RAK3172_ERR_OK when successful
 *                      RAK3172_ERR_INVALID_ARG when an invalid argument was passed
 *                      RAK3172_ERR_INVALID_STATE the when the interface is not initialized
 */
RAK3172_Error_t RAK3172_GetFWVersion(const RAK3172_t* const p_Device, std::string* const p_Version);

/** @brief              Get the serial number of the RAK3172 SoM.
 *  @param p_Device     Pointer to RAK3172 device object
 *  @param p_Serial     Pointer to serial number string
 *  @return             RAK3172_ERR_OK when successful
 *                      RAK3172_ERR_INVALID_ARG when an invalid argument was passed
 *                      RAK3172_ERR_INVALID_STATE the when the interface is not initialized
 */
RAK3172_Error_t RAK3172_GetSerialNumber(const RAK3172_t* const p_Device, std::string* const p_Serial);

/** @brief          Get the RSSI value of the last packet.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param p_RSSI   Pointer to RSSI value
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument was passed
 *                  RAK3172_ERR_INVALID_STATE the when the interface is not initialized
 */
RAK3172_Error_t RAK3172_GetRSSI(const RAK3172_t* const p_Device, int* p_RSSI);

/** @brief          Get the SNR value of the last packet.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param p_SNR    Pointer to SNR value
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument was passed
 *                  RAK3172_ERR_INVALID_STATE the when the interface is not initialized
 */
RAK3172_Error_t RAK3172_GetSNR(const RAK3172_t* const p_Device, int* p_SNR);

/** @brief          Set the current operating mode for the module.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param Mode     RAK3172 operating mode
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument was passed
 *                  RAK3172_ERR_INVALID_STATE the when the interface is not initialized
 */
RAK3172_Error_t RAK3172_SetMode(RAK3172_t* const p_Device, RAK3172_Mode_t Mode);

/** @brief          Get the current operating mode for the module.
 *  @param p_Device Pointer to RAK3172 device object
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument was passed
 *                  RAK3172_ERR_INVALID_STATE the when the interface is not initialized
 */
RAK3172_Error_t RAK3172_GetMode(RAK3172_t* const p_Device);

/** @brief          Set the baudrate of the module.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param Baud     Module baudrate
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument was passed
 *                  RAK3172_ERR_INVALID_STATE the when the interface is not initialized
 */
RAK3172_Error_t RAK3172_SetBaud(RAK3172_t* const p_Device, RAK3172_Baud_t Baud);

/** @brief          Get the baudrate of the module.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param p_Baud   Pointer to module baudrate
 *  @return         RAK3172_ERR_OK when successful
 *                  RAK3172_ERR_INVALID_ARG when an invalid argument was passed
 *                  RAK3172_ERR_INVALID_STATE the when the interface is not initialized
 */
RAK3172_Error_t RAK3172_GetBaud(RAK3172_t* const p_Device, RAK3172_Baud_t* p_Baud);

#endif /* RAK3172_H_ */