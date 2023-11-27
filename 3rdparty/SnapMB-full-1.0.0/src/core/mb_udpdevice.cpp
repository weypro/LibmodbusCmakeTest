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
//------------------------------------------------------------------------------
#include "mb_udpdevice.h"
//------------------------------------------------------------------------------

TMBUdpWorker::TMBUdpWorker(TMBUdpDevice* UdpDevice)
{
	LastError = 0;
	LocalHandle = 0;
	LocalBind = 0;
	ClearBuffers();
	memset(&ClientSin, 0, sizeof(ClientSin));
	Device = UdpDevice;
	EventsCount = 0;
	udpsck = new TUdpSocket();
	strncpy(udpsck->LocalAddress, Device->LocalAddress, 16);

	udpsck->LocalPort = Device->LocalPort;
	LastError = udpsck->SckBind();
	if (LastError == 0)
	{
		LocalHandle = udpsck->ClientHandle;
		LocalBind = udpsck->LocalBind;
	}
}
//------------------------------------------------------------------------------
TMBUdpWorker::~TMBUdpWorker()
{
	delete udpsck;
}
//------------------------------------------------------------------------------
bool TMBUdpWorker::UDPRecvTCPIndication()
{
	ClearErrors();
	udpsck->RecvTimeout = Device->WorkInterval;

	int SizeRecvd = udpsck->RecvPacketFrom(&Request, MaxNetADUSize, ClientSin);
	if (SizeRecvd > 0)
	{
		if (Device->CanAccept(ClientHandle(ClientSin)))
		{				
			if (SizeRecvd >= MBAPSize + 1) // At least MBAP + Function number
			{
				// Check the MBAP size declared
				int SizeExpected = SwapWord(Request.MBAP.PduLength) - 1 + MBAPSize;
				if (SizeRecvd == SizeExpected)
				{
					Request.Size = word(SizeRecvd - MBAPSize);
					if ((Device->PacketLog & PacketLog_IN) && ws->PacketLog.OnRequest != NULL)
					{
						ws->PacketLog.cs->Enter();
						try
						{
							ws->PacketLog.OnRequest(ws->PacketLog.UsrPtr, ClientHandle(ClientSin), PacketLog_IN, &Request, SizeRecvd);
						}
						catch (...) {}
						ws->PacketLog.cs->Leave();
					}
					return true;
				}
				else
					NotifyError(evcInvalidADUReceived);
			}
			else
				NotifyError(evcInvalidADUReceived);
		}
	}
	else
		if (udpsck->LastNetError != WSAETIMEDOUT)
			NotifyError(evcNetworkError, udpsck->LastNetError);
		else
			udpsck->LastNetError = 0;

	return false;
}
//------------------------------------------------------------------------------
bool TMBUdpWorker::UDPRecvRTUIndication()
{
	ClearErrors();
	udpsck->RecvTimeout = Device->WorkInterval;

	int SizeRecvd = udpsck->RecvPacketFrom(&Request.MBAP.UnitID, MaxNetADUSize, ClientSin);
	if (SizeRecvd > 0)
	{
		if (Device->CanAccept(ClientHandle(ClientSin)))
		{
			if (SizeRecvd >= MinimumRtuPDUSize && SizeRecvd <= MaxBinPDUSize)
			{
				Request.Size = word(SizeRecvd) - 3;
				if ((Device->PacketLog & PacketLog_IN) && ws->PacketLog.OnRequest != NULL)
				{
					ws->PacketLog.cs->Enter();
					try
					{
						ws->PacketLog.OnRequest(ws->PacketLog.UsrPtr, ClientHandle(ClientSin), PacketLog_IN, &Request.MBAP.UnitID, SizeRecvd);
					}
					catch (...) {}
					ws->PacketLog.cs->Leave();
				}

				if (!CheckCRC16(pbyte(&Request.MBAP.UnitID), word(SizeRecvd)))
					NotifyError(evcCRCError); // CRC error
				else
					return true;
			}
			else
				NotifyError(evcInvalidADUReceived);
		}
	}
	else
		if (udpsck->LastNetError != WSAETIMEDOUT)
			NotifyError(evcNetworkError, udpsck->LastNetError);
		else
			udpsck->LastNetError = 0;


	return false;
}
//------------------------------------------------------------------------------
int TMBUdpWorker::NotifyError(int Error, int SckError)
{
#ifdef SNAP_OS_WINDOWS
	Event.EvtSender = ClientSin.sin_addr.S_un.S_addr;
#else
	Event.EvtSender = ClientSin.sin_addr.s_addr;
#endif	
	Event.EvtCode = Error;
	Event.EvtRetCode = word(SckError);
	Device->DoEvent(PSrvEvent(&Event));
	return Error;
}
//------------------------------------------------------------------------------
void TMBUdpWorker::ClearBuffers()
{
	memset(&Request, 0, sizeof(Request));
	memset(&Answer, 0, sizeof(Answer));
}
//------------------------------------------------------------------------------
void TMBUdpWorker::ClearErrors()
{
	udpsck->LastNetError = 0;
	LastError = 0;
}
//------------------------------------------------------------------------------
longword TMBUdpWorker::ClientHandle(sockaddr_in CSin)
{
#ifdef SNAP_OS_WINDOWS
	return CSin.sin_addr.S_un.S_addr;
#else
	return CSin.sin_addr.s_addr;
#endif				
}
//------------------------------------------------------------------------------
void TMBUdpWorker::Execute()
{
	word TxPDUSize = 0;
	TIndicationResult Result = TIndicationResult::irSendAnswer;

	while (!Terminated) 
	{
		ClearEvents();
		bool IndicationResult;
		if (Device->Proto == ProtoUDP)
			IndicationResult = UDPRecvTCPIndication();
		else
			IndicationResult = UDPRecvRTUIndication();

		if (IndicationResult)
		{
			if (Device->Passthrough && ws->Passthrough.OnRequest != NULL)
			{
				ws->Passthrough.cs->Enter();
				try
				{
					int psResult = ws->Passthrough.OnRequest(ws->Passthrough.UsrPtr, Request.MBAP.UnitID, &Request.PDU, Request.Size, &Answer.PDU, TxPDUSize);
					if (psResult != 0)
					{
						Answer.PDU.Function = Request.PDU.Function + 0x80;
						Answer.PDU.Data[0] = uint8_t(psResult & 0xF);
						TxPDUSize = 2;
					}

					if (TxPDUSize > 0)
						Result = TIndicationResult::irSendAnswer;
					else
						Result = TIndicationResult::irNoAnswer;

					Event.EvtCode = evcPassthrough;
					Event.EvtParam1 = Request.MBAP.UnitID;
					Event.EvtParam2 = Request.PDU.Function;
					Event.EvtParam3 = Request.Size;
					Event.EvtParam4 = TxPDUSize;
					Event.EvtRetCode = word(Result);
				}
				catch (...) {}
				ws->Passthrough.cs->Leave();
			}
			else
				Result = ExecuteIndication(&Request.PDU, Request.Size, &Answer.PDU, TxPDUSize);
			
			if (Event.EvtCode != 0)
			{
				Event.EvtSender = ClientHandle(ClientSin);
				Device->DoEvent(PSrvEvent(&Event));
			}
			
			if (Result == TIndicationResult::irSendAnswer)
			{
				if (Device->Proto == ProtoUDP)
					UDPSendTCPConfirmation(word(TxPDUSize));
				else
					UDPSendRTUConfirmation(word(TxPDUSize));
			}
		}
	}
}
//------------------------------------------------------------------------------
bool TMBUdpWorker::UDPSendTCPConfirmation(word PDUSize)
{
	bool Result = true;
	// Prepares MBAP
	Answer.MBAP.TransID = Request.MBAP.TransID;
	Answer.MBAP.ProtocolID = tcp_mb_protoid;
	Answer.MBAP.UnitID = Request.MBAP.UnitID;
	Answer.MBAP.PduLength = SwapWord(PDUSize + 1);
	Answer.Size = MBAPSize + PDUSize;
	// Sends the Confirmation

	if (udpsck->AnswerPacketTo(&Answer, Answer.Size, ClientSin) != 0)
		Result = NotifyError(evcNetworkError, udpsck->LastNetError) == 0;

	if ((Device->PacketLog & PacketLog_OUT) && ws->PacketLog.OnRequest != NULL)
	{
		ws->PacketLog.cs->Enter();
		try
		{
			ws->PacketLog.OnRequest(ws->PacketLog.UsrPtr, ClientHandle(ClientSin), PacketLog_OUT, &Answer, Answer.Size);
		}
		catch (...) {}
		ws->PacketLog.cs->Leave();
	}

	return Result;
}
//------------------------------------------------------------------------------
bool TMBUdpWorker::UDPSendRTUConfirmation(word PDUSize)
{
	bool Result = true;

	Answer.MBAP.UnitID = Request.MBAP.UnitID;
	word CRC16 = CalcCRC((uint8_t*)&Answer.MBAP.UnitID, word(PDUSize + 1));
	Answer.PDU.Data[PDUSize - 1] = CRC16 & 0x00FF;
	Answer.PDU.Data[PDUSize] = (CRC16 >> 8);
	Answer.Size = PDUSize + 3;

	if (udpsck->AnswerPacketTo(&Answer.MBAP.UnitID, Answer.Size, ClientSin) != 0)
		Result = NotifyError(evcNetworkError, udpsck->LastNetError) == 0;

	if ((Device->PacketLog & PacketLog_OUT) && ws->PacketLog.OnRequest != NULL)
	{
		ws->PacketLog.cs->Enter();
		try
		{
			ws->PacketLog.OnRequest(ws->PacketLog.UsrPtr, ClientHandle(ClientSin), PacketLog_OUT, &Answer.MBAP.UnitID, Answer.Size);
		}
		catch (...) {}
		ws->PacketLog.cs->Leave();
	}

	return Result;
}
//------------------------------------------------------------------------------
int TMBUdpDevice::CreateWorker()
{
	Worker = new TMBUdpWorker(this);
	Worker->ws = ws;
	return Worker->LastError;
}
//------------------------------------------------------------------------------
void TMBUdpDevice::DestroyWorker()
{
	if (Worker != NULL)
	{
		Worker->Terminate();
		if (Worker->WaitFor(WorkerCloseTimeout) != WAIT_OBJECT_0)
		{
			Worker->Kill();
		}
		try {
			delete Worker;
		}
		catch (...) {}
		Worker = NULL;
	}
}
//------------------------------------------------------------------------------
bool TMBUdpDevice::CanAccept(longword PeerHandle)
{
	#define evcClientRefused 0x00000400
	bool Result = true;
	if (PeerListMode != plmDisabled)
	{
		int PeerIndex = FindPeer(PeerHandle);
		Result = ((PeerIndex == -1) && (PeerListMode == plmBlockList)) || ((PeerIndex > -1) && (PeerListMode == plmAllowList));
	}

	if (!Result)
	{
		DoEvent(PeerHandle, evcClientRefused, 0, 0, 0, 0, 0);
		ClientsBlocked++;
	}

	return Result;
}
//------------------------------------------------------------------------------
int TMBUdpDevice::FindPeer(longword Peer)
{
	for (int c = 0; c < PeerCount; c++)
	{
		if (PeerList[c] == Peer)
			return c;
	}
	return -1;
}
//------------------------------------------------------------------------------
TMBUdpDevice::TMBUdpDevice(int Proto, byte DeviceID, const char* Address, int Port)
{
	this->Proto = Proto;
	FDeviceID = DeviceID;
	strncpy(LocalAddress, Address, 15);
	memset(PeerList, 0, sizeof(PeerList));
	LocalPort = word(Port);
	Worker = NULL;
	WorkInterval = def_WorkInterval;
	PeerCount = 0;
	ClientsBlocked = 0;
	PeerListMode = plmDisabled;
	LastError = 0;
	PacketLog = def_PacketLog;
	BaseObjectError = errEthernetDevice;
	ws->IsSerialDevice = false;
}
//------------------------------------------------------------------------------
TMBUdpDevice::~TMBUdpDevice()
{
	Destroying = true;
	Stop();
}
//------------------------------------------------------------------------------
int TMBUdpDevice::Start()
{
	LastError = 0;
	int Result = 0;
	if (Status != DevRunning)
	{
		ClientsBlocked = 0;
		Result = CreateWorker();
		if (Result != 0)
		{
			delete Worker; // No need to call DestroyWorker() since it was not started
			Worker = NULL;
			DoEvent(0, evcDeviceCannotStart, word(Result), 0, 0, 0, 0);
		}
		else
		{
			Worker->Start();
			Status = DevRunning;
			DoEvent(Worker->LocalHandle, evcDeviceStarted, 0, LocalPort, 0, 0, 0);
		};
	};
	return SetError(errCategoryNetSocket, Result);
}
//------------------------------------------------------------------------------
void TMBUdpDevice::Stop()
{
	if (Status == DevRunning)
	{
		DestroyWorker();
		Status = DevStopped;
		if (!Destroying)
			DoEvent(0, evcDeviceStopped, 0, 0, 0, 0, 0);
	};
	LastError = 0;
}
//------------------------------------------------------------------------------
int TMBUdpDevice::SetParam(int ParamIndex, int Value)
{
	switch (ParamIndex)
	{
	case par_TCP_UDP_Port:
		if (Status != DevRunning)
			LocalPort = word(Value);
		else
			return SetError(errCategoryProcess, errDevOpNotAllowed);
		break;
	case par_DeviceID:
		FDeviceID = byte(Value);
		break;
	case par_DevPeerListMode:
		PeerListMode = Value;
		break;
	case par_PacketLog:
		PacketLog = Value;
		break;
	case par_WorkInterval:
		WorkInterval = Value;
		break;
	case par_AllowSerFunOnEth:
		ws->AllowSerFunOnEth = Value != 0;
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
int TMBUdpDevice::SetDeviceBind(byte DeviceID, const char* Address, int Port)
{
	LastError = 0;
	if (Status != DevRunning)
	{
		FDeviceID = DeviceID;
		strncpy(LocalAddress, Address, 15);
		LocalPort = word(Port);
		return 0;
	}
	else
		return SetError(errCategoryProcess, errDevCannotRebindOnRun);
}
//------------------------------------------------------------------------------
int TMBUdpDevice::AddPeer(const char* Address)
{
	if (PeerCount < MaxUDPPeersList)
	{
		PeerList[PeerCount] = inet_addr(Address);
		PeerCount++;
		return mbNoError;
	}
	return SetError(errCategoryProcess, errDevTooManyPeers);
}
//------------------------------------------------------------------------------
int TMBUdpDevice::GetDeviceInfo(TDeviceInfo& DeviceInfo)
{
	DeviceInfo.LastError = LastError;
	DeviceInfo.Running = DevRunning;
	DeviceInfo.ClientsCount = 0;
	DeviceInfo.ClientsBlocked = ClientsBlocked;
	return 0;
}
//------------------------------------------------------------------------------
int TMBUdpDevice::SetUserFunction(byte FunctionID, bool Value)
{
	return SetError(errCategoryProcess, ws->SetCustomFunction(FunctionID, Value));
}


















