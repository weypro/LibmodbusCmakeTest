/*=============================================================================|
|  PROJECT SnapModbus                                                          |
|==============================================================================|
|  Copyright (C) 2023 Davide Nardella                                          |
|  All rights reserved.                                                        |
|==============================================================================|
|  SnapModbus is free software: you can redistribute it and/or modify          |
|  it under the terms of the Lesser GNU General Public License as published by |
|  the Free Software Foundation, either version 3 of the License, or           |
|  (at your option) any later version.                                         |
|                                                                              |
|  It means that you can distribute your commercial software linked with       |
|  SnapModbus without the requirement to distribute the source code of your    |
|  application and without the requirement that your application be itself     |
|  distributed under LGPL.                                                     |
|                                                                              |
|  SnapModbus is distributed in the hope that it will be useful,               |
|  but WITHOUT ANY WARRANTY; without even the implied warranty of              |
|  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the               |
|  Lesser GNU General Public License for more details.                         |
|                                                                              |
|  You should have received a copy of the GNU General Public License and a     |
|  copy of Lesser GNU General Public License along with Snap7.                 |
|  If not, see  http://www.gnu.org/licenses/                                   |
|=============================================================================*/
#include "mb_serbroker.h"
//-----------------------------------------------------------------------------
TMBSerBroker::TMBSerBroker()
{
	Connected = false;
	sersck = NULL;
	DisconnectOnError = def_SERDisconnectOnError;

	MaxRetries = def_MaxRetries_rtu;
	AttemptSleep = def_AttemptSleep;
	IsSerialBroker = true;

	CurrentFun = 0;

	ClearBuffers();
	LastError = 0;
	ComSignature = 0;
}
//-----------------------------------------------------------------------------
TMBSerBroker::~TMBSerBroker()
{
}
//-----------------------------------------------------------------------------
void TMBSerBroker::Disconnect()
{
	sersck->Disconnect();
	Connected = sersck->Connected;
}
//-----------------------------------------------------------------------------
int TMBSerBroker::SetLocalParam(byte LocalID, int ParamIndex, int Value)
{
	switch (ParamIndex)
	{
	case par_BaseAddress:
		BaseZero = Value;
		break;
	case par_SendTimeout:
		sersck->SendTimeout = Value;
		break;
	case par_InterframeDelay:
		if (Value < 1) Value = 1;
		sersck->InterframeDelay = Value;
		break;
	case par_MaxRetries:
		MaxRetries = Value;
		if (MaxRetries < 1)
			MaxRetries = 1;
		break;
	case par_AttemptSleep:
		AttemptSleep = longword(Value);
		break;
	default:
		return SetError(errCategoryProcess,errInvalidParamIndex);
	}
	return mbNoError;
}
//------------------------------------------------------------------------------
// Multiplex function tho change the remote device params
//------------------------------------------------------------------------------
int TMBSerBroker::SetRemoteDeviceParam(byte DeviceID, int ParamIndex, int Value)
{
	PDeviceParams Params = GetDeviceParamsPtr(DeviceID);

	switch (ParamIndex)
	{
	case par_SerialFormat:
		Params->SerialFormat = Value;
		break;
	case par_AutoTimeout:
		Params->AutoTimeout = Value;
		break;
	case par_AutoTimeLimitMin:
		Params->AutoTimeLimit_Min = Value;
		break;
	case par_FixedTimeout:
		Params->FixedTimeout_ms = Value;
		break;
	default:
		return SetError(errCategoryProcess,errInvalidParamIndex);
	}

	return mbNoError;
}
//-----------------------------------------------------------------------------
void TMBSerBroker::SetRecvTimeout(byte DeviceID)
{
	PDeviceParams Params = GetDeviceParamsPtr(DeviceID);
	if (Params->AutoTimeout)
		sersck->RecvTimeout = Params->AutoTimeCalc_ms;
	else
		sersck->RecvTimeout = Params->FixedTimeout_ms;
}
//-----------------------------------------------------------------------------
void TMBSerBroker::ClearBuffers()
{
	memset(&SndPacket, 0, sizeof(SndPacket));
	memset(&RcvPacket, 0, sizeof(RcvPacket));
}
//-----------------------------------------------------------------------------
void TMBSerBroker::ClearErrors()
{
	LastError = 0;
	sersck->LastSerError = 0;
}
//-----------------------------------------------------------------------------
void TMBSerBroker::UpdateDeviceStatus(PDeviceStatus DeviceStat, longword LastSerError, bool SameFun)
{
	if (LastSerError != 0)
	{
		if (LastSerError == ERDTOUT)
			DeviceStat->Status = _StatusTimeout;
		else
			DeviceStat->Status = _StatusError;
	}
	else
	{
		if (!SameFun)
			DeviceStat->Status = _StatusProtoError;
		else
			DeviceStat->Status = _StatusOk;
	}
	DeviceStat->LastError = LastSerError;
	DeviceStat->JobTime = SysGetTick() - JobStart;
}
//------------------------------------------------------------------------------
// Prepares the RTU Packet and calcs the CRC16
//------------------------------------------------------------------------------
void TMBSerBroker::PrepareRTURequest(byte DeviceID, pbyte DataOut, word SizeOut)
{
	ClearErrors();
	SndPacket.ADU[0] = DeviceID;
	memcpy(&SndPacket.ADU[1], DataOut, SizeOut);
	uint16_t CRC16 = CalcCRC((uint8_t*)(&SndPacket.ADU), SizeOut + 1);
	SndPacket.ADU[SizeOut + 1] = CRC16 & 0x00FF;
	SndPacket.ADU[SizeOut + 2] = (CRC16 >> 8);
	SndPacket.Size = SizeOut + 3; // Adding UnitID and CRC16
}
//------------------------------------------------------------------------------
// Prepares the ASCII Packet and calcs the LRC
//------------------------------------------------------------------------------
//  The binary PDU is converted into an ASCII string
// 
//  ':'<Ascii Device><Ascii PDU><Ascii LRC><CRLF>
//------------------------------------------------------------------------------
void TMBSerBroker::PrepareASCIIRequest(byte DeviceID, pbyte DataOut, word SizeOut)
{
	ClearErrors();
	SndPacket.ADU[0] = 0x3A; // ';'
	SndPacket.ADU[1] = Ascii[DeviceID >> 4];
	SndPacket.ADU[2] = Ascii[DeviceID & 0x0F];

	byte LRC = byte(DeviceID);  // LRC calculus starts from DeviceID

	int y = 3;
	for (int c = 0; c < SizeOut; c++)
	{
		SndPacket.ADU[y++] = Ascii[DataOut[c] >> 4];   // HI
		SndPacket.ADU[y++] = Ascii[DataOut[c] & 0x0F]; // LO
		LRC += DataOut[c];
	}
	LRC = (byte)(-((byte)LRC));

	SndPacket.ADU[y++] = Ascii[LRC >> 4];
	SndPacket.ADU[y++] = Ascii[LRC & 0x0F];
	SndPacket.ADU[y++] = 0x0D;
	SndPacket.ADU[y++] = 0x0A;
	SndPacket.Size = word(y);
}
//------------------------------------------------------------------------------
// Here we don't know in advance the size expected
//------------------------------------------------------------------------------
int TMBSerBroker::RecvUnknownRTUResponse(byte DeviceID, pbyte DataIn, word& SizeIn, PPDURecvExpected PRE)
{
	int Result = mbNoError;
	int MinimumSize = 5; // UnitID(1) + Error(2) + CRC(2)
	int SizeRead = 0;
	PRTUAnswer Answer = PRTUAnswer(&RcvPacket.ADU);
	PRTUError RtuError = PRTUError(&RcvPacket.ADU);

	Result = sersck->RecvPacket(&RcvPacket.ADU, MaxRtuADUSize, SizeRead);

	if (Result == mbNoError)
	{
		if (SizeRead >= MinimumSize)
		{
			if (Answer->DeviceID == PRE->DeviceID) // Check the Device Address
			{
				if (Answer->Function == PRE->Function) // Check the function
				{
					RcvPacket.Size = word(SizeRead);
					SizeIn = word(SizeRead - 3); // PDU Size, skip DeviceID(1) and CRC(2)
					if (CheckCRC16(pbyte(&RcvPacket.ADU), word(SizeRead)))
					{
						memcpy(DataIn, &RcvPacket.ADU[1], SizeIn);
					}
					else
						Result = errInvalidChecksum;
				}
				else // Function mismatch : it could be a Modbus Error or rubbish
				{
					if ((Answer->Function & 0x80) == 0x80) // Modbus Error
					{
						if (CheckCRC16(pbyte(&RcvPacket.ADU), sizeof(TRTUError))) // Here too we have to check the CRC
						{
							DataIn[0] = RtuError->ErrorCode;
							DataIn[1] = RtuError->Exception;
							SizeIn = 2;
						}
						else
							Result = errInvalidChecksum;
					}
					else // was rubbish
						Result = errInvalidADUReceived;
				}
			}
			else // Malformed ADU
				Result = errInvalidADUReceived;
		}
		else
			Result = errInvalidADUReceived;
	};

	if (Result != mbNoError)
	{
		if (sersck->LastSerError != 0)
		{
			SetError(errCategorySerialSocket, sersck->LastSerError);
			if (Result == EPORTRD) // Severe HW error, we must close the port
				Disconnect();
			else
				if (sersck->LastSerError == ERDTOUT)
					SetError(errCategoryProcess, errTimeout);
		}
		else
		{
			SetError(errCategoryProcess, Result);
			if (DisconnectOnError)
				Disconnect();
			else
				sersck->Flush(RX);
		}
	}

	return LastError;
}
//------------------------------------------------------------------------------
// Receives the RTU Response
//------------------------------------------------------------------------------
//   Wee need of two steps, first get the minimum size for a well formed Answer,
//   then, after some checks, we get the remainder.
//------------------------------------------------------------------------------
int TMBSerBroker::RecvRTUResponse(byte DeviceID, pbyte DataIn, word& SizeIn, PPDURecvExpected PRE)
{
	int Result = mbNoError;
	PRTUAnswer Answer = PRTUAnswer(&RcvPacket.ADU);
	PRTUError RtuError = PRTUError(&RcvPacket.ADU);

	// PRE.Size contains the size that we expect as answer for a correct transaction.
	// But the answer could be an error answer, in this case the smallest well-formed ADU 
	// will be composed by: DeviceID + Error + Exception + CRC H + CRC L = 5byte
	//
	// The idea is to first read 5 byte then parse them.

	Result = sersck->RecvPacket(&RcvPacket.ADU, sizeof(TRTUError)); // The minumum size that we expect for an Answer   
	if (Result == mbNoError)
	{
		RcvPacket.Size = sizeof(TRTUError);
		if (Answer->DeviceID == PRE->DeviceID) // Check the Device Address
		{
			if (Answer->Function == PRE->Function) // Check the function
			{
				word TotalSize = PRE->Size + 3; // UnitID(1) + PDU + CRC(2)

				// Ok, now we can collect the rest of the message
				// We received 5 byte, we need to collect the remaining starting from the sixth byte (i.e. byte N.5)
				// The TotalSize is PRE.Size + 3 (SlaveID and CRC16), we collected 5, thus the Remainder is (PRE.Size + 3) - 5
				word RemainingSize = TotalSize - 5;
				if (RemainingSize > 0)
					Result = sersck->RecvPacket(&RcvPacket.ADU[5], RemainingSize);
				if (Result == 0)
				{
					RcvPacket.Size = TotalSize;
					if (CheckCRC16(pbyte(&RcvPacket.ADU), TotalSize))
					{
						// Everything was ok, we can copy the PDU
						SizeIn = PRE->Size;
						memcpy(DataIn, &RcvPacket.ADU[1], SizeIn); // [1] because we need to skip DeviceID
					}
					else
						Result = errInvalidChecksum;
				};
			}
			else // Function mismatch : it could be a Modbus Error or rubbish
			{
				if ((Answer->Function & 0x80) == 0x80) // Modbus Error
				{
					if (CheckCRC16(pbyte(&RcvPacket.ADU), sizeof(TRTUError))) // Here too we have to check the CRC
					{
						DataIn[0] = RtuError->ErrorCode;
						DataIn[1] = RtuError->Exception;
						SizeIn = 2;
					}
					else
						Result = errInvalidChecksum;
				}
				else // was rubbish
					Result = errInvalidADUReceived;
			}
		}
		else // Malformed ADU
			Result = errInvalidADUReceived;
	};

	if (Result != mbNoError)
	{
		if (sersck->LastSerError != 0)
		{
			SetError(errCategorySerialSocket, sersck->LastSerError);
			if (Result == EPORTRD) // Severe HW error, we must close the port
				Disconnect();
			else 
				if (sersck->LastSerError == ERDTOUT)
					SetError(errCategoryProcess,errTimeout);
		}
		else
		{
			SetError(errCategoryProcess,Result);
			if (DisconnectOnError)
				Disconnect();
			else
				sersck->Flush(RX);
		}
	}

	return LastError;
}
//------------------------------------------------------------------------------
// Receives the ASCII Response
//------------------------------------------------------------------------------
//   The whole packet is read as string LF-Terminated, we don't need to know its
//   size in advanced.
//   Once we got the string we wil perform all checks. Finally the string will be
//   converted from ASCII to RTU. i.e. in DataIn we will find always a binary PDU
//------------------------------------------------------------------------------
int TMBSerBroker::RecvASCIIResponse(byte DeviceID, pbyte DataIn, word& SizeIn, PPDURecvExpected PRE)
{
	int Result = mbNoError;
	int SizeRead;
	int MinimumSize = 11; // The smallest, well formed, ASCII Ansewr Length 
						  // 1(Start ':') + 2(DeviceID) + 2(Function) + (at least 2 error chars) + 2(LRC) + CR + LF 

	Result = sersck->RecvPacket(&RcvPacket.ADU, MaxAscADUSize, SizeRead);

	if (Result == mbNoError)
	{
		// Check Length, Start frame and End of Frame
		if (SizeRead >= MinimumSize && RcvPacket.ADU[0] == ':' && RcvPacket.ADU[SizeRead - 2] == 0x0D && RcvPacket.ADU[SizeRead - 1] == 0x0A)
		{
			byte DeviceID_recvd = (nibble(RcvPacket.ADU[1]) << 4) + nibble(RcvPacket.ADU[2]);

			if (DeviceID_recvd == PRE->DeviceID) // Check the echoed DeviceID
			{
				uint8_t LRC = DeviceID_recvd; // we need LRC also for the DeviceID (which will not inserted into PDU)

				// Now we can convert the ASCII PDU into RTU PDU

				int c = 3; // skip Start, DeviceID, LRC and CR LF
				int y = 0; // PDU start byte
				while (c < SizeRead - 4)
				{
					DataIn[y] = (nibble(RcvPacket.ADU[c++]) << 4);
					DataIn[y] += nibble(RcvPacket.ADU[c++]);
					LRC += DataIn[y++];
				}
				LRC = (byte)(-((byte)LRC)); // LRC Calculated
				SizeIn = word(y);

				// Check LRC Received
				byte LRC_recvd = (nibble(RcvPacket.ADU[SizeRead - 4]) << 4) + nibble(RcvPacket.ADU[SizeRead - 3]);

				if (LRC_recvd != LRC)
					Result = errInvalidChecksum;
			}
			else
				Result = errInvalidADUReceived;
		}
		else
			Result = errInvalidADUReceived;
	};

	if (Result != mbNoError)
	{
		if (sersck->LastSerError != 0)
		{
			SetError(errCategorySerialSocket, sersck->LastSerError);
			if (Result == EPORTRD) // Severe HW error, we must close the port
				Disconnect();
			else
				if (sersck->LastSerError == ERDTOUT)
					SetError(errCategoryProcess,errTimeout);
		}
		else
		{
			SetError(errCategoryProcess, Result);
			if (DisconnectOnError)
				Disconnect();
			else
				sersck->Flush(RX);
		}
	}

	return LastError;
}
//------------------------------------------------------------------------------
// Prepares the Request using RTU or ASCII function
//------------------------------------------------------------------------------
void TMBSerBroker::PrepareRequest(byte DeviceID, pbyte DataOut, word SizeOut)
{
	ClearBuffers(); 
	CurrentFun = *DataOut;
	if (DeviceID > 0)
	{
		if (GetDeviceParamsPtr(DeviceID)->SerialFormat == FormatRTU)
			PrepareRTURequest(DeviceID, DataOut, SizeOut);
		else
			PrepareASCIIRequest(DeviceID, DataOut, SizeOut);
	}
	else  //Broadcast only for RTU devices
		PrepareRTURequest(DeviceID, DataOut, SizeOut);
}
//------------------------------------------------------------------------------
// Sends the prepared Packet, which could be RTU or ASCII
//------------------------------------------------------------------------------
bool TMBSerBroker::SendRequest(byte DeviceID)
{
	// Check connection
	if (sersck->Connected || (Connect() == mbNoError))
	{
		if (sersck->SendPacket(&SndPacket, SndPacket.Size) != mbNoError)
			SetError(errCategorySerialSocket, sersck->LastSerError);
	}
	else
		SetError(errCategorySerialSocket, sersck->LastSerError);

	// Send errors are always Severe, they don't depend of line/devices but are hardware/driver related
	// We need to close the Port
	if (LastError != mbNoError)
		Disconnect();

	return LastError == 0;
}
//------------------------------------------------------------------------------
// Receives the answer
//------------------------------------------------------------------------------
bool TMBSerBroker::RecvResponse(byte DeviceID, pbyte DataIn, word& SizeIn, PPDURecvExpected PRE)
{
	int Result = mbNoError;

	if (DeviceID > 0) // DeviceID == 0 : It's a Broadcast and we will not receive the answer
	{
		SetRecvTimeout(DeviceID);
		longword Elapsed = SysGetTick();

		if (GetDeviceParamsPtr(DeviceID)->SerialFormat == FormatRTU)
		{
			if (PRE->Size == 0) // we don't know the Answer size
				Result = RecvUnknownRTUResponse(DeviceID, pbyte(DataIn), SizeIn, PRE);
			else
				Result = RecvRTUResponse(DeviceID, pbyte(DataIn), SizeIn, PRE);
		}
		else // ASCII format, the Packet is received as string and converted into RTU 
			Result = RecvASCIIResponse(DeviceID, pbyte(DataIn), SizeIn, PRE);

		// Calculates Dynamic timeout 	
		if ((Result & 0x0000000F) != errTimeout)
		{
			if (Result == mbNoError && PRE->Size > 0)
				CalcTimeout(Elapsed, GetDeviceParamsPtr(DeviceID));
		}
		else
			ClearTimeout(GetDeviceParamsPtr(DeviceID)); // increase the autotimeout
	}
	return Result == mbNoError;
}
//------------------------------------------------------------------------------
bool TMBSerBroker::SendRequestRecvResponse(byte DeviceID, pbyte DataIn, word& SizeIn, pbyte DataOut, word SizeOut, PPDURecvExpected PRE)
{
	sersck->Lock();
	try
	{
		int Retries = MaxRetries;
		bool Result;
		PrepareRequest(DeviceID, DataOut, SizeOut);
		do
		{
			ClearErrors();
			Result = SendRequest(DeviceID) && RecvResponse(DeviceID, DataIn, SizeIn, PRE);

			if (DeviceID > 0)
				UpdateDeviceStatus(GetDeviceStatusPtr(DeviceID), sersck->LastSerError, CurrentFun == *DataIn);

			if (!Result && --Retries)
			{
				if (sersck->LastSerError != ERDTOUT)
					SysSleep(AttemptSleep);
				else
					Retries = 0; // we don't recover the timeout since it's a "legal" response
			}

		} while (!Result && Retries);
	}
	catch (...) {}
	sersck->Unlock();

	return LastError == mbNoError;
}
//------------------------------------------------------------------------------
// Get Buffers for debug purpose
//------------------------------------------------------------------------------
int TMBSerBroker::GetIOBufferPtr(int BufferKind, pbyte& Data)
{
	if (BufferKind == BkSnd)
	{
		if (SndPacket.Size > word(sizeof(SndPacket.ADU)))
			SndPacket.Size = word(sizeof(SndPacket.ADU));
		Data = pbyte(&SndPacket.ADU);
		return SndPacket.Size;
	}
	else {
		if (RcvPacket.Size > word(sizeof(RcvPacket.ADU)))
			RcvPacket.Size = word(sizeof(RcvPacket.ADU));
		Data = pbyte(&RcvPacket.ADU);
		return RcvPacket.Size;
	}
}
//------------------------------------------------------------------------------
int TMBSerBroker::GetIOBuffer(int BufferKind, pbyte Data)
{
	if (BufferKind == BkSnd)
	{
		if (SndPacket.Size > word(sizeof(SndPacket.ADU)))
			SndPacket.Size = word(sizeof(SndPacket.ADU));
		memcpy(Data, &SndPacket.ADU, SndPacket.Size);
		return SndPacket.Size;
	}
	else {
		if (RcvPacket.Size > 0)
		{
			if (RcvPacket.Size > word(sizeof(RcvPacket.ADU)))
				RcvPacket.Size = word(sizeof(RcvPacket.ADU));
			memcpy(Data, &RcvPacket.ADU, RcvPacket.Size);
		}
		return RcvPacket.Size;
	}
}
