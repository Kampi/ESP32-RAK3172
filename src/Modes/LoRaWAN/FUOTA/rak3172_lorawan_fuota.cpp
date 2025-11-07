 /*
 * rak3172_lorawan_fuota.cpp
 *
 *  Copyright (C) Daniel Kampert, 2025
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

#include <sdkconfig.h>

#if((defined CONFIG_RAK3172_MODE_WITH_LORAWAN) && (defined CONFIG_RAK3172_MODE_WITH_LORAWAN_FUOTA) && (defined CONFIG_RAK3172_USE_RUI3))

#include <string.h>

#include "rak3172.h"

#include "../../Private/rak3172_tools.h"

#include "../../../Arch/rak3172_arch.h"

#include "Semtech/FragDecoder.h"

/** @brief LoRaWAN FOTA command identifier definitions.
 */
typedef enum
{
    RAK3172_FOTA_CID_PACKAGE_VERSION_REQ    = 0x00,             /**< Used by the AS to request the package version implemented by the end-device. */
    RAK3172_FOTA_CID_PACKAGE_VERSION_ANS    = 0x00,             /**< Conveys the answer to PackageVersionReq. */
    RAK3172_FOTA_CID_FRAG_STATUS_REQ        = 0x01,             /**< Asks an end-device or a group of enddevices to send the status of a fragmentation session. */
    RAK3172_FOTA_CID_FRAG_STATUS_ANS        = 0x01,             /**< Conveys the answer to FragSessionStatus request. */
    RAK3172_FOTA_CID_FRAG_SETUP_REQ         = 0x02,             /**< Defines a fragmentation session. */
    RAK3172_FOTA_CID_FRAG_SETUP_ANS         = 0x02,             /**< */
    RAK3172_FOTA_CID_FRAG_DELETE_REQ        = 0x03,             /**< Used to delete a fragmentation session. */
    RAK3172_FOTA_CID_FRAG_DELETE_ANS        = 0x03,             /**< */
    RAK3172_FOTA_CID_DATA_FRAGMENT          = 0x08,             /**< Carries a fragment of a data block. */
} RAK3172_FOTA_CID_t;

static FragDecoderCallbacks_t _RAK3172_FUOTA_FragCallbacks;

static const char* TAG = "RAK3172_LoRaWAN_FUOTA";

/** @brief          Reads `data` buffer of `size` starting at address `addr`
 *  @param Addr     Address start index to read from.
 *  @param p_Data   Data buffer to be read.
 *  @param Size     Size of data buffer to be read.
 *  @return         Read operation status [0: Success, -1 Fail]
 */
static int8_t _RAK3172_LoRaWAN_FUOTA_FragDecoderWrite(uint32_t Addr, uint8_t* p_Data, uint32_t Size)
{
    return 0;
}

/** @brief          Reads `data` buffer of `size` starting at address `addr`
 *  @param Addr     Address start index to read from.
 *  @param p_Data   Data buffer to be read.
 *  @param Size     Size of data buffer to be read.
 *  @return         Read operation status [0: Success, -1 Fail]
 */
static int8_t _RAK3172_LoRaWAN_FUOTA_FragDecoderRead(uint32_t Addr, uint8_t* p_Data, uint32_t Size)
{
    return 0;
}

RAK3172_Error_t RAK3172_LoRaWAN_FUOTA_Run(RAK3172_t& p_Device, RAK3172_MC_Group_t* p_Group, uint32_t Timeout)
{
    uint8_t* FragMemory;
    uint32_t Now;
    RAK3172_Rx_t Message;
    RAK3172_Error_t Error;
    RAK3172_FragSetup_t FragSetup;
    RAK3172_Class_t Class;

    if(p_Device.Mode != RAK_MODE_LORAWAN)
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
            goto RAK3172_LoRaWAN_FUOTA_Run_Exit;
        }
    }

    FragMemory = NULL;
    Now = RAK3172_Timer_GetMilliseconds();
    while(true)
    {
        uint8_t Command;

        //RAK3172_WDT_Reset();

        Command = 0xFF;
        Message.Port = 0xFF;

        if((RAK3172_Timer_GetMilliseconds() - Now) > (Timeout * 1000UL))
        {
            Error = RAK3172_ERR_TIMEOUT;
            goto RAK3172_LoRaWAN_FUOTA_Run_Exit;
        }

        Error = RAK3172_LoRaWAN_Receive(p_Device, &Message, 1);
        if(Error != RAK3172_ERR_OK)
        {
            //ESP_LOGE(TAG, "Cannot receive message! Error: 0x%04X", static_cast<unsigned int>(Error));
        }

        // Check if the port is valid.
        if(Message.Port != CONFIG_RAK3172_MODE_LORAWAN_FUOTA_PORT)
        {
            continue;
        }

        // Only process the message when at least one byte (2 characters were received).
        if(Message.Payload.size() > 2)
        {
            // The first byte is always the command.
            RAK3172_Tools_Hex2ASCII(Message.Payload.substr(0, 2), &Command);
            Message.Payload.erase(0, 2);
        }

        switch(Command)
        {
            case RAK3172_FOTA_CID_PACKAGE_VERSION_REQ:
            {
                uint8_t Buffer[3];

                RAK3172_LOGI(TAG, "Received package version request");

                Buffer[0] = RAK3172_FOTA_CID_PACKAGE_VERSION_ANS;
                Buffer[1] = 3;
                Buffer[2] = CONFIG_RAK3172_MODE_LORAWAN_FUOTA_PACKAGE_VERSION;
                if(RAK3172_LoRaWAN_Transmit(p_Device, Message.Port, Buffer, sizeof(Buffer)) != RAK3172_ERR_OK)
                {
                    Error = RAK3172_ERR_FAIL;
                    goto RAK3172_LoRaWAN_FUOTA_Run_Exit;
                }

                break;
            }
            case RAK3172_FOTA_CID_FRAG_SETUP_REQ:
            {
                bool isEncodingUnsupported;
                bool isNotEnoughMemory;
                uint8_t Buffer[2];
                uint8_t StatusBitMask;

                RAK3172_LOGI(TAG, "Received fragmentation setup request");

                RAK3172_Tools_Hex2ASCII(Message.Payload, reinterpret_cast<uint8_t*>(&FragSetup));

                RAK3172_LOGI(TAG, "Session setup received...");
                RAK3172_LOGI(TAG, " FragSession: %i", static_cast<unsigned int>(FragSetup.FragSession.Raw));
                RAK3172_LOGI(TAG, " NbFrag: %u", static_cast<unsigned int>(FragSetup.NbFrag));
                RAK3172_LOGI(TAG, " FragSize: %u", static_cast<unsigned int>(FragSetup.FragSize));
                RAK3172_LOGI(TAG, " Control: %u", static_cast<unsigned int>(FragSetup.Control.Raw));
                RAK3172_LOGI(TAG, " Padding: %u", static_cast<unsigned int>(FragSetup.Padding));
                RAK3172_LOGI(TAG, " Descriptor: %u", static_cast<unsigned int>(FragSetup.Descriptor.Raw));

                // Check the "CONTROL field"
                isEncodingUnsupported = true;
                if(FragSetup.Control.Fields.FragAlgo == 0)
                {
                    isEncodingUnsupported = false;
                }

                // Allocate the memory for the fragmented data
                // TODO: Can we replace malloc?
                isNotEnoughMemory = false;
                RAK3172_LOGI(TAG, "Allocate %u bytes of memory", static_cast<unsigned int>(FragSetup.NbFrag * FragSetup.FragSize * sizeof(uint8_t)));
                FragMemory = reinterpret_cast<uint8_t*>(calloc(FragSetup.NbFrag * FragSetup.FragSize, sizeof(uint8_t)));
                if(FragMemory == NULL)
                {
                    isNotEnoughMemory = true;
                    Error = RAK3172_ERR_NO_MEM;
                    goto RAK3172_LoRaWAN_FUOTA_Run_Exit;
                }

                // Bit 0:   Encoding unsupported
                // Bit 1:   Not enough memory
                // Bit 2:   FragSession index not supported
                // Bit 3:   Wrong descriptor
                // Bit 4-5: RFU
                // Bit 6-7: FragIndex
                StatusBitMask = (FragSetup.FragSession.Fields.FragIndex << 6) | (isNotEnoughMemory << 1) | (isEncodingUnsupported << 0);

                _RAK3172_FUOTA_FragCallbacks.FragDecoderRead = _RAK3172_LoRaWAN_FUOTA_FragDecoderRead;
                _RAK3172_FUOTA_FragCallbacks.FragDecoderWrite = _RAK3172_LoRaWAN_FUOTA_FragDecoderWrite;
                FragDecoderInit(FragSetup.NbFrag, FragSetup.FragSize, &_RAK3172_FUOTA_FragCallbacks);

                Buffer[0] = RAK3172_FOTA_CID_FRAG_SETUP_ANS;
                Buffer[1] = StatusBitMask;
                if(RAK3172_LoRaWAN_Transmit(p_Device, Message.Port, Buffer, sizeof(Buffer)) != RAK3172_ERR_OK)
                {
                    Error = RAK3172_ERR_FAIL;
                    goto RAK3172_LoRaWAN_FUOTA_Run_Exit;
                }

                break;
            }
            case RAK3172_FOTA_CID_FRAG_DELETE_REQ:
            {
                uint8_t Status;
                uint8_t Buffer[2];
                uint8_t StatusBitMask;

                RAK3172_LOGI(TAG, "Received fragmentation delete request");

                RAK3172_Tools_Hex2ASCII(Message.Payload, &Status);

                RAK3172_LOGI(TAG, " Status: %u", static_cast<unsigned int>(Status));

                // Bit 0-1: FragIndex
                // Bit 2-7: RFU
                StatusBitMask = FragSetup.FragSession.Fields.FragIndex;

                Buffer[0] = RAK3172_FOTA_CID_FRAG_DELETE_ANS;
                Buffer[1] = StatusBitMask;
                if(RAK3172_LoRaWAN_Transmit(p_Device, Message.Port, Buffer, sizeof(Buffer)) != RAK3172_ERR_OK)
                {
                    Error = RAK3172_ERR_FAIL;
                }

                Error = RAK3172_ERR_OK;
                goto RAK3172_LoRaWAN_FUOTA_Run_Exit;

                break;
            }
            case RAK3172_FOTA_CID_DATA_FRAGMENT:
            {
                int8_t Status; 
                uint8_t FragIndex;
                uint8_t Buffer[2];
                uint16_t N;
                std::string Index;

                Index = Message.Payload.substr(0, 4);
                Message.Payload.erase(0, 4);

                RAK3172_Tools_Hex2ASCII(Index, Buffer);

                FragIndex = (Buffer[0] >> 6) & 0x03;
                N = ((static_cast<uint16_t>(Buffer[0] & 0x3F)) << 6) | Buffer[1];

                RAK3172_Tools_Hex2ASCII(Message.Payload, &FragMemory[(N - 1) * FragSetup.FragSize]);
                Status = FragDecoderProcess(N, &FragMemory[(N - 1) * FragSetup.FragSize]);

                RAK3172_LOGI(TAG, "Received data fragment %u", N);
                RAK3172_LOGI(TAG, " FragIndex: %u", static_cast<unsigned int>(FragIndex));
                RAK3172_LOGI(TAG, " Decoder status: %i", static_cast<signed int>(Status));
                RAK3172_LOG_BUFFER_HEX(TAG, FragMemory, FragSetup.NbFrag * FragSetup.FragSize * sizeof(uint8_t));

                break;
            }
            default:
            {
                break;
            }
        }

        Now = RAK3172_Timer_GetMilliseconds();
    }

    if(FragMemory != NULL)
    {
        free(FragMemory);
    }

RAK3172_LoRaWAN_FUOTA_Run_Exit:
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

#endif