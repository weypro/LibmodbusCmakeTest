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
#include "mb_serdevice.h"
//------------------------------------------------------------------------------
TMBSerWorker::TMBSerWorker(TMBSerDevice* Device, PSerialSocket SerialSocket)
{
	this->Device = Device;
	this->sersck = SerialSocket;
	ClearBuffers();
	ErrorCount = 0;
	EventsCount = 0;
	AsciiBuffer[0] = 0;
}
//------------------------------------------------------------------------------
TMBSerWorker::~TMBSerWorker()
{
}
//------------------------------------------------------------------------------
void TMBSerWorker::ClearBuffers()
{
	memset(&Request, 0, sizeof(Request));
	memset(&Answer, 0, sizeof(Answer));
}
//------------------------------------------------------------------------------
void TMBSerWorker::ManageError()
{
	ErrorCount++;
	sersck->Flush(RTX);

	if (ErrorCount == MaxErrors) 
	{
		sersck->Disconnect();
		Device->DoEvent(0, evcPortReset, 0, 0, 0, 0, 0);
		SysSleep(500);
		sersck->Connect();
		ErrorCount = 0;
	}
	else
		Device->DoEvent(0, evcPortError, word(sersck->LastSerError), 0, 0, 0, 0);
}
//------------------------------------------------------------------------------
bool TMBSerWorker::IsBroadcast(byte DeviceID, byte Function)
{
	return (DeviceID==0) && (
		(Function == fun_WriteSingleCoil) ||
		(Function == fun_WriteSingleRegister) ||
		(Function == fun_WriteMultipleCoils) ||
		(Function == fun_WriteMultipleRegisters) ||
		(Function == fun_WriteFileRecord)
	);
}
//------------------------------------------------------------------------------
TIndicationResult TMBSerWorker::RecvRtuIndication(char FirstChar)
{
	int SizeRead;
	// The smallest, well formed, RTU Request Length 
	// 1(DeviceID) + 1(Function) + 2(CRC) = 4
	// Here it's 3 because we already read the DeviceID (FirstChar)
	int MinimumSize = 3; 

	Request.DeviceID = FirstChar;
	if (sersck->RecvInterframedPacket(&Request.PDU, MaxRtuADUSize - 1, SizeRead) != 0)
	{
		if (sersck->LastSerError == EPORTRD)
			ManageError();
		return TIndicationResult::irSkip;
	}

	Request.Size = word(SizeRead) - 2; // Skip (2)CRC

	if ((Device->PacketLog & PacketLog_IN) && ws->PacketLog.OnRequest != NULL)
	{
		ws->PacketLog.cs->Enter();
		try
		{
			ws->PacketLog.OnRequest(ws->PacketLog.UsrPtr, 0, PacketLog_IN, &Request, SizeRead+1);
		}
		catch (...) {}
		ws->PacketLog.cs->Leave();
	}

	if (SizeRead < MinimumSize)
		return TIndicationResult::irSkip; // bad frame

	if (Request.DeviceID != Device->FDeviceID && !Device->Passthrough && !IsBroadcast(Request.DeviceID, Request.PDU[0]))
		return TIndicationResult::irSkip; // Not our business 

	// Check Incoming CRC
	if (!CheckCRC16(pbyte(&Request), word(SizeRead + 1))) 
	{
		Device->DoEvent(0, evcCRCError, 0, 0, 0, 0, 0);
		return TIndicationResult::irSkip; // Bad CRC
	}
	
	return TIndicationResult::irDone; 
}
//------------------------------------------------------------------------------
TIndicationResult TMBSerWorker::RecvAscIndication(char FirstChar)
{
	AsciiBuffer[0] = FirstChar;

	// The smallest, well formed, ASCII Request Length 
    // 1(Start ':') + 2(DeviceID) + 2(Function) + 2(LRC) + CR + LF 	
	// Here it's 8 because we already read Start (FirstChar)
	int MinimumSize = 8; 
						 
	int SizeRead = 0;
	if (sersck->RecvInterframedPacket(&AsciiBuffer[1], MaxAscADUSize - 1, SizeRead) != 0)
	{
		if (sersck->LastSerError == EPORTRD)
			ManageError();
		return TIndicationResult::irSkip;
	}

	SizeRead++; // the whole buffer, including ':'

	if (AsciiBuffer[0] != ':')
		return TIndicationResult::irSkip; // Maybe rubbish

/*--------------------------------------------------------

         DeviceID
	':'   H   L   [PDU] LRCH LRCL CR LF

	 0    1   2           |   |    |  |
                          |   |    |  +--- SizeRead - 1 
		                  |   |    +------ SizeRead - 2
						  |   +----------- SizeRead - 3
						  +--------------- SizeRead - 4

--------------------------------------------------------*/

	if ((Device->PacketLog & PacketLog_IN) && ws->PacketLog.OnRequest != NULL)
	{
		ws->PacketLog.cs->Enter();
		try
		{
			ws->PacketLog.OnRequest(ws->PacketLog.UsrPtr, 0, PacketLog_IN, &AsciiBuffer, SizeRead);
		}
		catch (...) {}
		ws->PacketLog.cs->Leave();
	}

	if (SizeRead < MinimumSize || AsciiBuffer[SizeRead - 1] != 0x0A || AsciiBuffer[SizeRead - 2] != 0x0D)
		return TIndicationResult::irSkip; // Bad Frame

	Request.DeviceID = (nibble(AsciiBuffer[1]) << 4) + nibble(AsciiBuffer[2]);
	byte LRC = Request.DeviceID;

	if (Request.DeviceID != Device->FDeviceID && !Device->Passthrough && !IsBroadcast(Request.DeviceID, Request.PDU[0]))
		return TIndicationResult::irSkip; // Not our business 

	// ASCII to RTU conversion and LRC calc
	int c = 3; // Skip ':' and DeviceID
	int y = 0;
	while (c < SizeRead - 4)
	{
		Request.PDU[y] = (nibble(AsciiBuffer[c++]) << 4);
		Request.PDU[y] += nibble(AsciiBuffer[c++]);
		LRC += Request.PDU[y++];
	}
	LRC = (byte)(-((byte)LRC)); // LRC Calculated
	Request.Size = word(y);

	// Check LRC Received
	byte LRC_recvd = (nibble(AsciiBuffer[SizeRead - 4]) << 4) + nibble(AsciiBuffer[SizeRead - 3]);
	if (LRC != LRC_recvd)
	{
		Device->DoEvent(0, evcCRCError, 0, 0, 0, 0, 0);
		return TIndicationResult::irSkip; // Bad Checksum 
	}

	return TIndicationResult::irDone;
}
//------------------------------------------------------------------------------
void TMBSerWorker::SendRtuConfirmation(word PDUSize)
{
	Answer.DeviceID = Device->FDeviceID;
	word CRC16 = CalcCRC((uint8_t*) &Answer, word(PDUSize + 1));

	Answer.PDU[PDUSize ] = CRC16 & 0x00FF;
	Answer.PDU[PDUSize + 1] = (CRC16 >> 8);

	Answer.Size = PDUSize + 3;

	if (sersck->SendPacket(&Answer, Answer.Size) == mbNoError)
	{		
		if ((Device->PacketLog & PacketLog_OUT) && ws->PacketLog.OnRequest != NULL)
		{
			ws->PacketLog.cs->Enter();
			try
			{
				ws->PacketLog.OnRequest(ws->PacketLog.UsrPtr, 0, PacketLog_OUT, &Answer, Answer.Size);
			}
			catch (...) {}
			ws->PacketLog.cs->Leave();
		}
	}
	else
		ManageError();
}
//------------------------------------------------------------------------------
void TMBSerWorker::SendAscConfirmation(word PDUSize)
{
	// RTU to ASCII conversion and LRC calc

	byte LRC = Device->FDeviceID;  // LRC calculus starts from DeviceID

	AsciiBuffer[0] = 0x3A; // ':'
	AsciiBuffer[1] = Ascii[Device->FDeviceID >> 4];
	AsciiBuffer[2] = Ascii[Device->FDeviceID & 0x0F];

	int y = 3;
	for (int c = 0; c < PDUSize; c++)
	{
		AsciiBuffer[y++] = Ascii[Answer.PDU[c] >> 4];   // HI
		AsciiBuffer[y++] = Ascii[Answer.PDU[c] & 0x0F]; // LO
		LRC += Answer.PDU[c];
	}
	LRC = (byte)(-((byte)LRC));
	
	AsciiBuffer[y++] = Ascii[LRC >> 4];
	AsciiBuffer[y++] = Ascii[LRC & 0x0F];
	AsciiBuffer[y++] = 0x0D;
	AsciiBuffer[y++] = 0x0A;

	if (sersck->SendPacket(&AsciiBuffer, y) == mbNoError)
	{
		if ((Device->PacketLog & PacketLog_OUT) && ws->PacketLog.OnRequest != NULL)
		{
			ws->PacketLog.cs->Enter();
			try
			{
				ws->PacketLog.OnRequest(ws->PacketLog.UsrPtr, 0, PacketLog_OUT, &AsciiBuffer, y);
			}
			catch (...) {}
			ws->PacketLog.cs->Leave();
		}
	}
	else
		ManageError();
}
//------------------------------------------------------------------------------
void TMBSerWorker::Execute()
{
	char ch;
	word TxPDUSize = 0;
	TIndicationResult Result;
	while (!Terminated)
	{
		ClearEvents();
		if (sersck->CharReady(ch, Device->WorkInterval))
		{
			if (Device->Format == FormatRTU)
				Result = RecvRtuIndication(ch);
			else
				Result = RecvAscIndication(ch);

			if (Result == TIndicationResult::irDone)
			{
				
				if (Device->Passthrough && ws->Passthrough.OnRequest != NULL)
				{
					ws->Passthrough.cs->Enter();
					try
					{
						int psResult = ws->Passthrough.OnRequest(ws->Passthrough.UsrPtr, Request.DeviceID, &Request.PDU, Request.Size, &Answer.PDU, TxPDUSize);
						
						if (psResult != 0)
						{
							Answer.PDU[0] = Request.PDU[0] + 0x80;
							Answer.PDU[1] = uint8_t(psResult & 0xF);
							TxPDUSize = 2;
						}
						
						if (TxPDUSize > 0)
							Result = TIndicationResult::irSendAnswer;
						else
							Result = TIndicationResult::irNoAnswer;
						
						Event.EvtCode = evcPassthrough;
						Event.EvtParam1 = Request.DeviceID;
						Event.EvtParam2 = Request.PDU[0];
						Event.EvtParam3 = Request.Size;
						Event.EvtParam4 = TxPDUSize;
						Event.EvtRetCode = word(Result);
					}
					catch (...) {}
					ws->Passthrough.cs->Leave();
				}
				else
					Result = ExecuteIndication(PMBPDU(&Request.PDU), Request.Size, PMBPDU(&Answer.PDU), TxPDUSize);
				
				if (Event.EvtCode != 0)
					Device->DoEvent(PSrvEvent(&Event));
				
				if (IsBroadcast(Request.DeviceID, Request.PDU[0]))
					Result = TIndicationResult::irNoAnswer;

				if (Result == TIndicationResult::irSendAnswer)
				{
					if (Device->Format == FormatRTU)
						SendRtuConfirmation(TxPDUSize);
					else
						SendAscConfirmation(TxPDUSize);
				}
			}
			ErrorCount = 0;
		}
		else
			if (sersck->LastSerError != 0)
				ManageError();
	}
}
//------------------------------------------------------------------------------
TMBSerDevice::TMBSerDevice(int Format, byte DeviceID, const char* Name, int BaudRate, char Parity, int DataBits, int Stops, int Flow)
{
	this->Format = Format;
	FDeviceID = DeviceID;
	LastError = 0;
	Worker = NULL;
	Destroying = false;
	EventMask = 0xFFFFFFFF;
	LogMask = 0xFFFFFFFF;
	FUsrPtr = NULL;
	sersck = new TSerialSocket();
	sersck->SetComParams(Name, BaudRate, Parity, DataBits, Stops, Flow);
	sersck->InterframeDelay = def_InterframeDelay;
	WorkInterval = def_WorkInterval; 
	PacketLog = def_PacketLog;
	BaseObjectError = errSerialDevice;
	ws->IsSerialDevice = true;
}
//------------------------------------------------------------------------------
TMBSerDevice::~TMBSerDevice()
{
	Destroying = true;
	Stop();
	delete sersck;
}
//------------------------------------------------------------------------------
int TMBSerDevice::SetParam(int ParamIndex, int Value)
{
	switch (ParamIndex)
	{
	case par_SerialFormat:
		if (Status != DevRunning)
		{
			Value ? Format = FormatASC : Format = FormatRTU;
		}
		else
			return SetError(errCategoryProcess, errDevOpNotAllowed);
	case par_DeviceID:
		FDeviceID = byte(Value);
		break;
	case par_PacketLog:
		PacketLog = Value;
		break;
	case par_InterframeDelay:
		if (Value < 1) Value = 1;
		sersck->InterframeDelay = Value;
		break;
	case par_WorkInterval:
		WorkInterval = Value;
		break;
	case par_DevicePassthrough:
		if (Status != DevRunning)
			Passthrough = Value;
		else
			return SetError(errCategoryProcess, errDevOpNotAllowed);
		break;
	default:
		return SetError(errCategoryProcess, errDevInvalidParamIndex);
	}
	return 0;
}
//------------------------------------------------------------------------------
int TMBSerDevice::SetDeviceBind(byte DeviceID, const char* Name, int BaudRate, char Parity, int DataBits, int Stops, int Flow)
{
	LastError = 0;
	if (Status != DevRunning)
	{
		FDeviceID = DeviceID;
		sersck->SetComParams(Name, BaudRate, Parity, DataBits, Stops, Flow);
		return 0;
	}
	else
		return SetError(errCategoryProcess, errDevCannotRebindOnRun);
}
//------------------------------------------------------------------------------
int TMBSerDevice::SetUserFunction(byte FunctionID, bool Value)
{
	return SetError(errCategoryProcess, ws->SetCustomFunction(FunctionID, Value));
}
//------------------------------------------------------------------------------
void TMBSerDevice::ClearErrors()
{
	LastError = 0;
	sersck->LastSerError = 0;
}
//------------------------------------------------------------------------------
void TMBSerDevice::CreateWorker()
{
	// Creates the Worker
	Worker = new TMBSerWorker(this ,sersck);
	Worker->ws = ws;
	Worker->Start();
}
//------------------------------------------------------------------------------
void TMBSerDevice::DestroyWorker()
{
	if (Worker != NULL)
	{
		Worker->Terminate();
		if (Worker->WaitFor(WorkerCloseTimeout) != WAIT_OBJECT_0)
			Worker->Kill();
		try {
			delete Worker;
		}
		catch (...) {}
		Worker = NULL;
	}
}
//------------------------------------------------------------------------------
int TMBSerDevice::GetInterframe(int& InterframeDelay, int& MaxInterframeDetected)
{
	InterframeDelay = sersck->InterframeDelay;
	MaxInterframeDetected = sersck->MaxInterframeDetected;
	return 0;
}
//------------------------------------------------------------------------------
int TMBSerDevice::GetDeviceInfo(TDeviceInfo& DeviceInfo)
{
	DeviceInfo.LastError = LastError;
	DeviceInfo.Running = sersck->Connected;
	DeviceInfo.ClientsCount = 0;
	DeviceInfo.ClientsBlocked = 0;
	return 0;
}
//------------------------------------------------------------------------------
int TMBSerDevice::Start()
{
	LastError = 0;
	int Result = 0;
	if (Status != DevRunning)
	{
		Result = sersck->Connect();
#ifdef SNAP_OS_WINDOWS
		if (Result == mbNoError)
		{	
			// Some adapters fail to set the port timeout the first time
			// a open/close cycle is needed
			Result = sersck->SetTimeout(1, WorkInterval);
			if (Result != mbNoError)
			{
				sersck->Disconnect();
				SysSleep(100);
				Result = sersck->Connect();
			}
		}
#endif
		if (Result == mbNoError)
		{
			CreateWorker();
			Status = DevRunning;
			DoEvent(0, evcDeviceStarted, 0, 0, 0, 0, 0);
		}
		else
			DoEvent(0, evcDeviceCannotStart, word(Result), 0, 0, 0, 0);
	}
	return SetError(errCategorySerialSocket, Result);
}
//------------------------------------------------------------------------------
void TMBSerDevice::Stop()
{
	if (Status == DevRunning)
	{
		DestroyWorker();
		sersck->Disconnect();
		Status = DevStopped;
	}
}
//------------------------------------------------------------------------------
