 /*
 * rak3172_lorawan_clock_sync.cpp
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

#include <sdkconfig.h>

#if((defined CONFIG_RAK3172_MODE_WITH_LORAWAN) && (defined CONFIG_RAK3172_MODE_WITH_LORAWAN_CLOCK_SYNC) && (defined CONFIG_RAK3172_USE_RUI3))

#include <time.h>
#include <string.h>

#include "rak3172.h"

#include "../../Private/rak3172_tools.h"

#include "../../../Arch/Timer/rak3172_timer.h"
#include "../../../Arch/Logging/rak3172_logging.h"

/** @brief 
 */
static RAK3172_AppTime_t _RAK3172_AppTime;

static const char* TAG = "RAK3172_LoRaWAN_ClockSync";

/** @brief LoRaWAN clock synchronization command identifier definitions.
 */
typedef enum
{
    RAK3172_CLOCK_CID_PACKAGE_VERSION_REQ   = 0x00,             /**< Used by the AS to request the package version implemented by the end-device. */
    RAK3172_CLOCK_CID_PACKAGE_VERSION_ANS   = 0x00,             /**< Conveys the answer to PackageVersionReq. */
    RAK3172_CLOCK_CID_APP_TIME_REQ          = 0x01,             /**< Used by end-device to request clock correction. */
    RAK3172_CLOCK_CID_APP_TIME_ANS          = 0x01,             /**< Conveys the clock timing correction. */
    RAK3172_CLOCK_CID_PERIODICITY_REQ       = 0x02,             /**< Used by the application server for 2 purposes: \n
                                                                        - Set the periodicity at which the enddevice shall transmit AppTimeReq messages \n
                                                                        - Request an immediate transmission of end-device time */
    RAK3172_CLOCK_CID_PERIODICITY_ANS       = 0x02,             /**< */
    RAK3172_CLOCK_CID_FORCE_RESYNC          = 0x03,             /**< Used by the application server to the end-device to trigger a clock resynchronization. */
} RAK3172_ClockSync_CID_t;

/** @brief              Receive a command from the server and extract the message and the command code.
 *  @param p_Device     RAK3172 device object
 *  @param p_Message    Pointer to received message
 *  @param p_Command    Pointer to command code
 *  @param Timeout      (Optional) Receive timeout in seconds
 *  @return             RAK3172_ERR_OK when successful
 */
static RAK3172_Error_t RAK3172_LoRaWAN_Clock_ReceiveCommand(RAK3172_t& p_Device, RAK3172_Rx_t* p_Message, uint8_t* p_Command, uint32_t Timeout = 3)
{
    RAK3172_Error_t Error;

    Error = RAK3172_LoRaWAN_Receive(p_Device, p_Message, Timeout);
    if(Error != RAK3172_ERR_OK)
    {
        return Error;
    }

    // Check if the port is valid.
    if(p_Message->Port != CONFIG_RAK3172_MODE_LORAWAN_CLOCK_SYNC_PORT)
    {
        return RAK3172_ERR_WRONG_PORT;
    }

    // Only process the message when at least one byte (2 characters were received).
    if(p_Message->Payload.size() < 2)
    {
        return RAK3172_ERR_INVALID_ARG;
    }

    // The first byte is always the command.
    RAK3172_Tools_Hex2ASCII(p_Message->Payload.substr(0, 2), p_Command);
    p_Message->Payload.erase(0, 2);

    return RAK3172_ERR_OK;
}

RAK3172_Error_t RAK3172_LoRaWAN_Clock_SetLocalTime(RAK3172_t& p_Device, struct tm* p_DateTime, bool AnsRequired, RAK3172_MC_Group_t* p_Group, uint32_t Timeout)
{
    uint8_t TokenAns;
    uint8_t Command;
    uint8_t Buffer[6];
    uint32_t Time;
    RAK3172_Rx_t Message;
    RAK3172_Class_t Class;
    RAK3172_Error_t Error;
    time_t Dummy;

    if(p_DateTime == NULL)
    {
        return RAK3172_ERR_INVALID_ARG;
    }
    else if(p_Device.Mode != RAK_MODE_LORAWAN)
    {
        return RAK3172_ERR_INVALID_MODE;
    }

    if(p_Group != NULL)
    {
        RAK3172_LOGD(TAG, "Using multicast for clock synchronization...");

        if(p_Device.LoRaWAN.Class != RAK_CLASS_C)
        {
            RAK3172_LOGD(TAG, "Reconfigure the device in class C...");
        
            RAK3172_ERROR_CHECK(RAK3172_LoRaWAN_GetClass(p_Device, &Class));
            RAK3172_ERROR_CHECK(RAK3172_LoRaWAN_SetClass(p_Device, RAK_CLASS_C));
        }

        if(RAK3172_LoRaWAN_MC_AddGroup(p_Device, p_Group->Class, p_Group->DevAddr, p_Group->NwkSKey, p_Group->AppSKey, p_Group->Frequency, p_Group->Datarate, p_Group->Periodicity) != RAK3172_ERR_OK)
        {
            Error = RAK3172_ERR_FAIL;
            goto RAK3172_LoRaWAN_Clock_SetLocalTime_Exit;
        }
    }

    Error = RAK3172_ERR_OK;

    // Command      AppTimeReq
    // Byte 0-3:    DeviceTime
    // Byte 4:      Param
    //      Bits 0-3:   TokenReq
    //      Bits 4:     AnsRequired
    //      Bits 5-7:   RFU
    Buffer[0] = RAK3172_CLOCK_CID_APP_TIME_REQ;
    memcpy(&Buffer[1], reinterpret_cast<const void*>(&_RAK3172_AppTime), sizeof(_RAK3172_AppTime));
    _RAK3172_AppTime.Param.Fields.AnsRequired = AnsRequired;
    if(RAK3172_LoRaWAN_Transmit(p_Device, CONFIG_RAK3172_MODE_LORAWAN_CLOCK_SYNC_PORT, Buffer, sizeof(Buffer)) != RAK3172_ERR_OK)
    {
        Error = RAK3172_ERR_FAIL;
        goto RAK3172_LoRaWAN_Clock_SetLocalTime_Exit;
    }

    if(RAK3172_LoRaWAN_Clock_ReceiveCommand(p_Device, &Message, &Command, Timeout) != RAK3172_ERR_OK)
    {
        // We expect an answer, but we don´t get an answer.
        if(AnsRequired == true)
        {
            Error = RAK3172_ERR_FAIL;
            goto RAK3172_LoRaWAN_Clock_SetLocalTime_Exit;
        }

        // The server has not transmitted an answer here. So we can abort.
        Error = RAK3172_ERR_OK;
        goto RAK3172_LoRaWAN_Clock_SetLocalTime_Exit;
    }

    // Check if the command is valid.
    if(Command != RAK3172_CLOCK_CID_APP_TIME_ANS)
    {
        Error = RAK3172_ERR_INVALID_ARG;
        goto RAK3172_LoRaWAN_Clock_SetLocalTime_Exit;
    }

    RAK3172_LOGD(TAG, "Received app time answer");

    // Convert the response payload.
    RAK3172_Tools_Hex2ASCII(Message.Payload, Buffer);

    // Check the TokenAns field.
    TokenAns = Buffer[4] & 0x0F;
    RAK3172_LOGD(TAG, "TokenAns: %u", static_cast<unsigned int>(TokenAns));

    // Discard the answer if the counter doesn´t match.
    if(TokenAns != _RAK3172_AppTime.Param.Fields.TokenReq)
    {
        Error = RAK3172_ERR_OK;
        goto RAK3172_LoRaWAN_Clock_SetLocalTime_Exit;
    }

    // We received an answer. So we have to increase the TokenReq counter.
    if(_RAK3172_AppTime.Param.Fields.TokenReq < 15)
    {
        _RAK3172_AppTime.Param.Fields.TokenReq++;
    }
    else
    {
        _RAK3172_AppTime.Param.Fields.TokenReq = 0;
    }

    // Get the time from the buffer.
    memcpy(&Time, Buffer, sizeof(uint32_t));

    // We have to add an offset, because the specification is working with the GPS epoch start and not with the 01/01/1970 as starting point.
    Time += 315964800;

    RAK3172_LOGD(TAG, "Time since 01/01/1970: %u", static_cast<unsigned int>(Time));

    Dummy = Time;
    memcpy(p_DateTime, localtime(&Dummy), sizeof(struct tm));

RAK3172_LoRaWAN_Clock_SetLocalTime_Exit:
    if(p_Group != NULL)
    {
        RAK3172_LoRaWAN_MC_RemoveGroup(p_Device, p_Group->DevAddr);

        if(Class != RAK_CLASS_C)
        {
            RAK3172_LoRaWAN_SetClass(p_Device, Class);
        }
    }

    return Error;
}

RAK3172_Error_t RAK3172_LoRaWAN_ClockSync_PackageVersion(RAK3172_t& p_Device)
{
    uint8_t Command;
    uint8_t Buffer[3];
    RAK3172_Rx_t Message;

    if(p_Device.Mode != RAK_MODE_LORAWAN)
    {
        return RAK3172_ERR_INVALID_MODE;
    }

    RAK3172_ERROR_CHECK(RAK3172_LoRaWAN_Clock_ReceiveCommand(p_Device, &Message, &Command));
    if(Command != RAK3172_CLOCK_CID_PACKAGE_VERSION_REQ)
    {
        return RAK3172_ERR_INVALID_ARG;
    }

    RAK3172_LOGI(TAG, "Received package version request");

    Buffer[0] = RAK3172_CLOCK_CID_PACKAGE_VERSION_ANS;
    Buffer[1] = 1;
    Buffer[2] = CONFIG_RAK3172_MODE_LORAWAN_CLOCK_SYNC_PACKAGE_VERSION;

    return RAK3172_LoRaWAN_Transmit(p_Device, Message.Port, Buffer, sizeof(Buffer));
}

bool RAK3172_LoRaWAN_ClockSync_isForceResync(RAK3172_t& p_Device, uint8_t* p_NbTransmissions, RAK3172_MC_Group_t* p_Group, uint32_t Timeout)
{
    bool Result;
    uint8_t Command;
    uint8_t Buffer[1];
    RAK3172_Rx_t Message;
    RAK3172_Class_t Class;
    RAK3172_Error_t Error;

    if(p_NbTransmissions == NULL)
    {
        return RAK3172_ERR_INVALID_ARG;
    }
    else if(p_Device.Mode != RAK_MODE_LORAWAN)
    {
        return false;
    }

    if(p_Group != NULL)
    {
        RAK3172_LOGD(TAG, "Using multicast for clock synchronization...");

        if(p_Device.LoRaWAN.Class != RAK_CLASS_C)
        {
            RAK3172_LOGD(TAG, "Reconfigure the device in class C...");
        
            if((RAK3172_LoRaWAN_GetClass(p_Device, &Class) != RAK3172_ERR_OK) || (RAK3172_LoRaWAN_SetClass(p_Device, RAK_CLASS_C) != RAK3172_ERR_OK))
            {
                return false;
            }
        }

        if(RAK3172_LoRaWAN_MC_AddGroup(p_Device, p_Group->Class, p_Group->DevAddr, p_Group->NwkSKey, p_Group->AppSKey, p_Group->Frequency, p_Group->Datarate, p_Group->Periodicity) != RAK3172_ERR_OK)
        {
            Result = false;
            goto RAK3172_LoRaWAN_ClockSync_isForceResync_Exit;
        }
    }

    Error = RAK3172_LoRaWAN_Clock_ReceiveCommand(p_Device, &Message, &Command, Timeout);
    RAK3172_LOGI(TAG, "Error: 0x%X", static_cast<unsigned int>(Error));
    if(Error != RAK3172_ERR_OK)
    {
        Result = false;
        goto RAK3172_LoRaWAN_ClockSync_isForceResync_Exit;
    }

    if(Command != RAK3172_CLOCK_CID_PACKAGE_VERSION_REQ)
    {
        Result = false;
        goto RAK3172_LoRaWAN_ClockSync_isForceResync_Exit;
    }

    RAK3172_LOGI(TAG, "Received force resync request");
    RAK3172_Tools_Hex2ASCII(Message.Payload, Buffer);

    *p_NbTransmissions = Buffer[0] & 0x07;

    Result = true;

RAK3172_LoRaWAN_ClockSync_isForceResync_Exit:
    if(p_Group != NULL)
    {
        RAK3172_LoRaWAN_MC_RemoveGroup(p_Device, p_Group->DevAddr);

        if(Class != RAK_CLASS_C)
        {
            RAK3172_LoRaWAN_SetClass(p_Device, Class);
        }
    }

    return Result;
}

RAK3172_Error_t RAK3172_LoRaWAN_ClockSync_HandlePeriodicity(RAK3172_t& p_Device, uint8_t* p_Period, uint32_t Time, bool NotSupported, uint32_t Timeout)
{
    uint8_t Command;
    uint8_t Buffer[6];
    RAK3172_Rx_t Message;

    if(p_Period == NULL)
    {
        return RAK3172_ERR_INVALID_ARG;
    }
    else if(p_Device.Mode != RAK_MODE_LORAWAN)
    {
        return RAK3172_ERR_INVALID_MODE;
    }

    RAK3172_ERROR_CHECK(RAK3172_LoRaWAN_Clock_ReceiveCommand(p_Device, &Message, &Command, Timeout));
    if(Command != RAK3172_CLOCK_CID_PERIODICITY_REQ)
    {
        return RAK3172_ERR_INVALID_ARG;
    }

    RAK3172_LOGI(TAG, "App time periodicity request");

    RAK3172_Tools_Hex2ASCII(Message.Payload, Buffer);

    *p_Period = Buffer[0] & 0x0F;
    RAK3172_LOGD(TAG, "Period: %u", static_cast<unsigned int>(*p_Period));

    memset(Buffer, 0, sizeof(Buffer));

    Buffer[0] = RAK3172_CLOCK_CID_PERIODICITY_ANS;
    Buffer[1] = (Time >> 24) & 0xFF;
    Buffer[2] = (Time >> 16) & 0xFF;
    Buffer[3] = (Time >> 8) & 0xFF;
    Buffer[4] = (Time >> 0) & 0xFF;
    Buffer[5] = (NotSupported << 0x00);

    return RAK3172_LoRaWAN_Transmit(p_Device, Message.Port, Buffer, sizeof(Buffer));
}

#endif