
 /*
 * rak3172_ymodem.cpp
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

#include <sdkconfig.h>

#if(defined CONFIG_RAK3172_USE_RUI3) & (defined CONFIG_RAK3172_MODE_WITH_UPDATE)

#include <string.h>

#include "rak3172.h"

#define YMODEM_INITIAL_PACKET_SIZE		128
#define YMODEM_PACKET_SIZE				1024
#define YMODEM_HEADER_SIZE				3

/** @brief CRC polynomial used by the Ymodem protocol.
 */
#define YMODEM_CRC_POLY					0x1021

/** @brief Start of a 128 byte data packet.
 */
#define YMODEM_SOH						0x01

/** @brief Transmission acknowledge.
 */
#define YMODEM_ACK						0x06

/** @brief Negative acknowledge for transmission.
 */
#define YMODEM_NAK						0x15

/** @brief Start of a 1024 byte data packet.
 */
#define YMODEM_STX						0x02

/** @brief End of a transmission.
 */
#define YMODEM_EOT						0x04

/** @brief  		Update the CRC16 value with a given input byte.
 * 	@param CRC_In	CRC input
 * 	@param Input	Input byte
 * 	@return			New CRC16 value
 */
static uint16_t RAK3172_Ymodem_UpdateCRC16(uint16_t CRC_In, uint16_t Input)
{
	uint16_t XOR;
	uint16_t Out;

	Out = CRC_In << 1;
	XOR = CRC_In >> 15;

	if(Input)
	{
		Out++;
	}

	if(XOR)
	{
		Out ^= YMODEM_CRC_POLY;
	}

	return Out;
}

/** @brief  		Calculate the CRC16 for a YModem Packet.
 *  @param p_Data	Input data
 *  @param Length	Data length
 *  @return			CRC checksum
 */
static uint16_t RAK3172_Ymodem_CRC16(const uint8_t* p_Data, uint32_t Length)
{
	uint16_t CRC;

	for(CRC = 0; Length > 0; Length--, p_Data++)
	{
		for(uint16_t i = 0x80; i; i >>= 1)
		{
			CRC = RAK3172_Ymodem_UpdateCRC16(CRC, *p_Data & i);
		}
	}

	for(uint16_t i = 0; i < 16; i++)
	{
		CRC = RAK3172_Ymodem_UpdateCRC16(CRC, 0);
	}

	return CRC;
}

/** @brief				Prepare and transmit a packet.
 *  @param p_Device		RAK3172 device object
 *  @param p_Header		Pointer to header data
 *  @param LengthHeader	Length of the data
 *  @param p_Data		Pointer to data
 *  @param LengthData	Length of the data
 *  @return				RAK3172_ERR_OK when successful
 * 						RAK3172_ERR_FAIL when the data cannot be transmitted
 * 						RAK3172_ERR_INVALID_RESPONSE when the update process can´t get started
 */
static RAK3172_Error_t RAK3172_Ymodem_Transmit(RAK3172_t& p_Device, const uint8_t* const p_Header, uint8_t LengthHeader, const uint8_t* const p_Data, uint16_t LengthData)
{
	uint16_t CRC;
	uint8_t Temp[2];

	CRC = RAK3172_Ymodem_CRC16(p_Data, LengthData);

	// Transmit the message header and the initial message payload.
    if((uart_write_bytes(p_Device.UART.Interface, p_Header, LengthHeader) != LengthHeader) &&
	   (uart_write_bytes(p_Device.UART.Interface, p_Data, LengthData) != LengthData)
	  )
	{
		return RAK3172_ERR_FAIL;
	}
 
	// Transmit the CRC.
	Temp[0] = CRC >> 0x08;
	Temp[1] = CRC & 0xFF;
    if(uart_write_bytes(p_Device.UART.Interface, Temp, sizeof(Temp)) != sizeof(Temp))
	{
		return RAK3172_ERR_FAIL;
	}

	return RAK3172_ERR_OK;
}

/** @brief				Prepare and transmit the first block (128 bytes) to initiate the transmission.
 *  @param p_Device		RAK3172 device object
 *  @param p_FileName	Pointer to file name
 *  @param Length		Length of the file name
 *  @param Timeout		(Optional) Timeout in seconds
 *  @return				RAK3172_ERR_OK when successful
 * 						RAK3172_ERR_FAIL when the data cannot be transmitted
 * 						RAK3172_ERR_INVALID_RESPONSE when the update process can´t get started
 */
static RAK3172_Error_t RAK3172_Ymodem_TransmitIntialPacket(RAK3172_t& p_Device, const char* const p_FileName, uint8_t Length, uint8_t Timeout = 1)
{
	uint8_t Header[YMODEM_HEADER_SIZE];
	uint8_t Data[YMODEM_INITIAL_PACKET_SIZE];

	if(Length > YMODEM_INITIAL_PACKET_SIZE)
	{
		return RAK3172_ERR_INVALID_ARG;
	}

	Header[0] = YMODEM_SOH;
	Header[1] = 0x00;
	Header[2] = 0xFF;

	memset(Data, 0, sizeof(Data));

	// Copy the filename into the buffer
	for(uint8_t i = 0; i < Length; i++)
	{
		Data[i] = p_FileName[i];
	}

	RAK3172_ERROR_CHECK(RAK3172_Ymodem_Transmit(p_Device, Header, sizeof(Header), Data, sizeof(Data)));

	// Wait for ACK and 'C'.
	if(uart_read_bytes(p_Device.UART.Interface, Data, 2, (Timeout * 1000UL) / portTICK_RATE_MS) == -1)
	{
		return RAK3172_ERR_INVALID_RESPONSE;
	}

	if((Data[0] == YMODEM_ACK) && (Data[1] = 'C'))
	{
		return RAK3172_ERR_OK;
	}

	return RAK3172_ERR_FAIL;
}

/** @brief			Transmit a data block.
 *  @param p_Device	RAK3172 device object
 *  @param Index	Packet index
 *  @param p_Data	Pointer to data
 *  @param Length	(Optional) Data length
 *  @param Timeout	(Optional) Timeout in seconds
 *  @return         RAK3172_ERR_OK when successful
 * 					RAK3172_ERR_FAIL when the data cannot be transmitted
 * 					RAK3172_ERR_INVALID_RESPONSE when the update process can´t get started
 */
static RAK3172_Error_t RAK3172_Ymodem_TransmitPacket(RAK3172_t& p_Device, uint8_t Index, const uint8_t* const p_Data, uint16_t Length = YMODEM_PACKET_SIZE, uint8_t Timeout = 1)
{
	uint8_t Data[YMODEM_PACKET_SIZE];
	uint8_t Header[YMODEM_HEADER_SIZE];

	if(Length > YMODEM_PACKET_SIZE)
	{
		return RAK3172_ERR_INVALID_ARG;
	}

	memset(Data, 0x00, YMODEM_PACKET_SIZE);

	// Copy the data into the transmit buffer.
	for(uint16_t i = 0; i < Length; i++)
	{
		Data[i] = p_Data[i];
	}

	Header[0] = YMODEM_STX;
	Header[1] = Index;
	Header[2] = 0xFF - Index;

	RAK3172_ERROR_CHECK(RAK3172_Ymodem_Transmit(p_Device, Header, sizeof(Header), Data, sizeof(Data)));

	// Wait for ACK and 'C'.
	if(uart_read_bytes(p_Device.UART.Interface, Data, 1, (Timeout * 1000UL) / portTICK_RATE_MS) == -1)
	{
		return RAK3172_ERR_INVALID_RESPONSE;
	}

	if(Data[1] = 'C')
	{
		return RAK3172_ERR_OK;
	}

	return RAK3172_ERR_FAIL;
}

/** @brief			Transmit the final data block.
 *  @param p_Device	RAK3172 device object
 *  @param Index	Packet index
 *  @param p_Data	Pointer to data
 *  @param Length	Data length
 *  @param Timeout	(Optional) Timeout in seconds
 *  @return         RAK3172_ERR_OK when successful
 * 					RAK3172_ERR_FAIL when the data cannot be transmitted
 * 					RAK3172_ERR_INVALID_RESPONSE when the update process can´t get started
 */
static RAK3172_Error_t RAK3172_Ymodem_TransmitFinalPacket(RAK3172_t& p_Device, uint8_t Index, const uint8_t* const p_Data, uint16_t Length, uint8_t Timeout = 1)
{
	uint8_t Data[YMODEM_PACKET_SIZE];
	uint8_t Header[YMODEM_HEADER_SIZE];

	if(Length > YMODEM_PACKET_SIZE)
	{
		return RAK3172_ERR_INVALID_ARG;
	}

	memset(Data, 0x1A, YMODEM_PACKET_SIZE);

	// Copy the data into the transmit buffer.
	for(uint16_t i = 0; i < Length; i++)
	{
		Data[i] = p_Data[i];
	}

	Header[0] = YMODEM_STX;
	Header[1] = Index;
	Header[2] = 0xFF - Index;

	RAK3172_ERROR_CHECK(RAK3172_Ymodem_Transmit(p_Device, Header, sizeof(Header), Data, sizeof(Data)));

	// Send the first 'EOT'. The device should response with 'NAK'.
	Data[0] = YMODEM_EOT;
	if((uart_write_bytes(p_Device.UART.Interface, Data, 1) != 1) &&
	   (uart_read_bytes(p_Device.UART.Interface, Data, 1, (Timeout * 1000UL) / portTICK_RATE_MS) == -1)
	  )
	{
		return RAK3172_ERR_FAIL;
	}

	if(Data[0] != YMODEM_NAK)
	{
		return RAK3172_ERR_INVALID_RESPONSE;
	}

	// Send the second 'EOT'. The device should response with 'ACK' and 'C'.
	Data[0] = YMODEM_EOT;
	if((uart_write_bytes(p_Device.UART.Interface, Data, 1) != 1) &&
	   (uart_read_bytes(p_Device.UART.Interface, Data, 2, (Timeout * 1000UL) / portTICK_RATE_MS) == -1)
	  )
	{
		return RAK3172_ERR_FAIL;
	}

	if((Data[0] == YMODEM_ACK) && (Data[1] = 'C'))
	{
		return RAK3172_ERR_OK;
	}

	return RAK3172_ERR_FAIL;
}

/** @brief  			Transmit a file using the Ymodem protocol.
 *  @param p_Device 	RAK3172 device object
 *  @param p_Data		Pointer to file data
 *  @param LengthData	File length
 *  @param p_FileName	Pointer to filename
 *  @param LengthName	File name length
 *  @return         	RAK3172_ERR_OK when successful
 */
static RAK3172_Error_t RAK3172_Ymodem_Transmit(RAK3172_t& p_Device, const uint8_t* const p_Data, uint32_t LengthData, const char* const p_FileName, uint8_t LengthName)
{
	uint8_t Index;
	uint8_t* Data;
	uint32_t Length;
	uint32_t BytesToTransmit;
	RAK3172_Error_t Error;

	RAK3172_ERROR_CHECK(RAK3172_Ymodem_TransmitIntialPacket(p_Device, p_FileName, LengthName));

	Data = (uint8_t*)p_Data;
	Length = LengthData - YMODEM_PACKET_SIZE;
	Index = 1;

	while(Length)
	{
		if(Length >= YMODEM_PACKET_SIZE)
		{
			BytesToTransmit = YMODEM_PACKET_SIZE;
		}
		else
		{
			BytesToTransmit = BytesToTransmit;
		}

		Error = RAK3172_Ymodem_TransmitPacket(p_Device, Index, Data, BytesToTransmit);
		if(Error != RAK3172_ERR_OK)
		{
			continue;
		}

		Index++;
		Length -= BytesToTransmit;
		Data += BytesToTransmit;
	}

	// Transmit the last and final packet.
	RAK3172_ERROR_CHECK(RAK3172_Ymodem_TransmitFinalPacket(p_Device, Index, Data, YMODEM_PACKET_SIZE));

	return RAK3172_ERR_OK;
}

RAK3172_Error_t RAK3172_RunUpdate(RAK3172_t& p_Device, const uint8_t* const p_Data, uint32_t Length)
{
	std::string Status;
	RAK3172_Error_t Error;

	if((p_Data == NULL) || (Length == 0))
	{
		return RAK3172_ERR_INVALID_ARG;
	}

	// Put the device into DFU mode.
	RAK3172_SendCommand(p_Device, "AT+BOOT", NULL, &Status);
	if(Status.find("AT_BUSY_ERROR") != std::string::npos)
	{
		return RAK3172_ERR_BUSY;
	}

	vTaskSuspend(p_Device.Internal.Handle);

	Error = RAK3172_Ymodem_Transmit(p_Device, p_Data, Length, NULL, 0);

	uart_flush(p_Device.UART.Interface);
	vTaskResume(p_Device.Internal.Handle);

	// Leave DFU mode.
	Error |= RAK3172_SendCommand(p_Device, "AT+RUN");

	return RAK3172_ERR_OK;
}

#endif