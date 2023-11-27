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
//---------------------------------------------------------------------------
#include "mb_tcpdevice.h"
//------------------------------------------------------------------------------
TMBTcpWorker::TMBTcpWorker()
{
    Device = NULL;
    ClearBuffers();
    DisElapsed = SysGetTick();
    EventsCount = 0;
}
//------------------------------------------------------------------------------
TMBTcpWorker::~TMBTcpWorker()
{
}
//------------------------------------------------------------------------------
int TMBTcpWorker::TCPRecvTCPIndication()
{
    // Receive Header
    int Result = RecvPacket(&Request.MBAP, MBAPSize);
    if (Result == mbNoError)
    {
        Request.Size = MBAPSize;
        int SizeIn = SwapWord(Request.MBAP.PduLength) - 1; // byte remaining after MBAP, -1 because after PduLength there is another byte which we already received

        // MaxSizeIn to protect against buffer overflow, the PDU consistence will be analyzed at higher level
        if (SizeIn > 0 && SizeIn <= MaxBinPDUSize && Request.MBAP.ProtocolID == tcp_mb_protoid) 
        {
            Result = RecvPacket(&Request.PDU, SizeIn);
            if (Result == mbNoError)
            {
                Request.Size = word(SizeIn);
                if ((Device->PacketLog & PacketLog_IN) && ws->PacketLog.OnRequest != NULL)
                {
                    ws->PacketLog.cs->Enter();
                    try
                    {
                        ws->PacketLog.OnRequest(ws->PacketLog.UsrPtr, this->ClientHandle, PacketLog_IN, &Request, Request.Size + MBAPSize);
                    }
                    catch (...) {}
                    ws->PacketLog.cs->Leave();
                }
            }
            else
                Result = NotifyError(evcNetworkError, LastNetError); // network error
        }
        else
            Result = NotifyError(evcInvalidADUReceived); // data error
    }
    else
        Result = NotifyError(evcNetworkError, LastNetError); // network error
    return Result;
}
//------------------------------------------------------------------------------
int TMBTcpWorker::TCPRecvRTUIndication()
{
    int SizeRecvd;

    int Result = Receive(&Request.MBAP.UnitID, mbBulkData, SizeRecvd);

    if (Result == mbNoError)
    {
        if (SizeRecvd >= MinimumRtuPDUSize && SizeRecvd <= MaxBinPDUSize)
        {
            Request.Size = word(SizeRecvd) - 3; // Skip DeviceID and CRC

            if ((Device->PacketLog & PacketLog_IN) && ws->PacketLog.OnRequest != NULL)
            {
                ws->PacketLog.cs->Enter();
                try
                {
                    ws->PacketLog.OnRequest(ws->PacketLog.UsrPtr, this->ClientHandle, PacketLog_IN, &Request.MBAP.UnitID, SizeRecvd);
                }
                catch (...) {}
                ws->PacketLog.cs->Leave();
            }

            if (!CheckCRC16(pbyte(&Request.MBAP.UnitID), word(SizeRecvd)))
                Result = NotifyError(evcCRCError); // CRC error
        }
        else
            Result = NotifyError(evcInvalidADUReceived); // data error
    }
    else
        Result = NotifyError(evcNetworkError, LastNetError); // network error

    return Result;
}
//------------------------------------------------------------------------------
bool TMBTcpWorker::TCPSendTCPConfirmation(word PDUSize)
{
    bool Result = true;
    // Prepares MBAP
    Answer.MBAP.TransID = Request.MBAP.TransID;
    Answer.MBAP.ProtocolID = tcp_mb_protoid;
    Answer.MBAP.UnitID = Request.MBAP.UnitID;
    Answer.MBAP.PduLength = SwapWord(PDUSize + 1);
    Answer.Size = MBAPSize + PDUSize;
    // Sends the Confirmation
    if (SendPacket(&Answer, Answer.Size) != 0)
        Result= NotifyError(evcNetworkError, LastNetError) == 0; // <-- Result = false

    if ((Device->PacketLog & PacketLog_OUT) && ws->PacketLog.OnRequest != NULL)
    {
        ws->PacketLog.cs->Enter();
        try
        {
            ws->PacketLog.OnRequest(ws->PacketLog.UsrPtr, this->ClientHandle, PacketLog_OUT, &Answer, Answer.Size);
        }
        catch (...) {}
        ws->PacketLog.cs->Leave();
    }

    return Result;
}
//------------------------------------------------------------------------------
bool TMBTcpWorker::TCPSendRTUConfirmation(word PDUSize)
{
    bool Result = true;

    Answer.MBAP.UnitID = Request.MBAP.UnitID;
    word CRC16 = CalcCRC((uint8_t*)&Answer.MBAP.UnitID, word(PDUSize + 1));
    Answer.PDU.Data[PDUSize - 1] = CRC16 & 0x00FF;
    Answer.PDU.Data[PDUSize] = (CRC16 >> 8);
    Answer.Size = PDUSize + 3;

    // Sends the Confirmation
    if (SendPacket(&Answer.MBAP.UnitID, Answer.Size) != 0)
        Result = NotifyError(evcNetworkError, LastNetError) == 0; // <-- Result = false

    if ((Device->PacketLog & PacketLog_OUT) && ws->PacketLog.OnRequest != NULL)
    {
        ws->PacketLog.cs->Enter();
        try
        {
            ws->PacketLog.OnRequest(ws->PacketLog.UsrPtr, this->ClientHandle, PacketLog_OUT, &Answer.MBAP.UnitID, Answer.Size);
        }
        catch (...) {}
        ws->PacketLog.cs->Leave();
    }

    return Result;
}
//------------------------------------------------------------------------------
void TMBTcpWorker::ClearBuffers()
{
    memset(&Request, 0, sizeof(Request));
    memset(&Answer, 0, sizeof(Answer));
}
//------------------------------------------------------------------------------
void TMBTcpWorker::DoEvent(longword Code, word RetCode, word Param1, word Param2, word Param3, word Param4)
{
    Device->DoEvent(this->ClientHandle,Code,RetCode,Param1, Param2, Param3, Param4);
}
//------------------------------------------------------------------------------
int TMBTcpWorker::NotifyError(int Error, int SckError)
{
    if (SckError != WSAECONNRESET) // to avoid the double Event
    {
        Event.EvtSender = this->ClientHandle;
        Event.EvtCode = Error;
        Event.EvtRetCode = word(SckError);
        Device->DoEvent(PSrvEvent(&Event));
    }
    return Error;
}
//------------------------------------------------------------------------------
int TMBTcpWorker::Execute()
{
    word TxPDUSize = 0;
    int WorkerResult = WorkerContinue;
    TIndicationResult Result = TIndicationResult::irSendAnswer;

    // Check Activity Timeout
    if (Device->DisconnectTimeout > 0 && (int)DeltaTime(DisElapsed) > Device->DisconnectTimeout)
        return WorkerTimeout;
    
    if (CanRead(Device->WorkInterval)) // should be Small to avoid time wait during the close
    {
        ClearEvents();
        DisElapsed = SysGetTick(); // Reset Activity Timer

        int IndicationResult;

        if (Device->Proto == ProtoTCP)
            IndicationResult = TCPRecvTCPIndication();
        else
            IndicationResult = TCPRecvRTUIndication();

        if (IndicationResult == mbNoError)
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
                        Answer.PDU.Data[0] = uint8_t(psResult &0xF);
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
                Event.EvtSender = this->ClientHandle;
                Device->DoEvent(PSrvEvent(&Event));
            }

            switch (Result)
            {
            case TIndicationResult::irDisconnect: // Disconnection requested by the user
                WorkerResult =  WorkerTerminate;
                break;
            case TIndicationResult::irSendAnswer:
                if (Device->Proto == ProtoTCP)
                {
                    if (!TCPSendTCPConfirmation(word(TxPDUSize)))
                        WorkerResult = WorkerTerminate;
                }
                else
                {
                    if (!TCPSendRTUConfirmation(word(TxPDUSize)))
                        WorkerResult = WorkerTerminate;
                }
                break;
            case TIndicationResult::irDone:
            case TIndicationResult::irSkip:
            case TIndicationResult::irNoAnswer:
                WorkerResult = WorkerContinue; // <-- Only to avoid Compiler Warning
                break;
            }
        }
        else 
        {
            if (LastNetError != 0 || DisconnectOnError)
                WorkerResult = WorkerTerminate;
        }
    }
    return WorkerResult;
}
//------------------------------------------------------------------------------
// Modbus Slave
//------------------------------------------------------------------------------
TMBTcpDevice::TMBTcpDevice(int Proto, byte DeviceID, const char* Address, int Port)
{   
    this->Proto = Proto;
    FDeviceID = DeviceID;
    strncpy(FLocalAddress, Address, 15);
    strncpy(ChannelName, Address, 15);
    ws = new TDeviceWorkingSet(false);
    memset(PeerList, 0, sizeof(PeerList));
    memset(&DeviceStat, 0, sizeof(DeviceStat));
    LocalPort = word(Port);
    WorkInterval= def_WorkInterval;
    PeerCount=0;
    PeerListMode=plmDisabled;
    LastError = 0;
    ClientsBlocked = 0;
    PacketLog = def_PacketLog;
    DisconnectTimeout = def_DisconnectTimeout;
    Passthrough = false;
    BaseObjectError = errEthernetDevice;
}
//------------------------------------------------------------------------------
TMBTcpDevice::~TMBTcpDevice()
{
    delete ws;
}
//------------------------------------------------------------------------------
int TMBTcpDevice::SetParam(int ParamIndex, int Value)
{
    switch (ParamIndex)
    {
    case par_TCP_UDP_Port:
        if (Status != SrvRunning)
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
    case par_AllowSerFunOnEth:
        ws->AllowSerFunOnEth = Value != 0;
        break;
    case par_DisconnectTimeout:
        DisconnectTimeout = Value;
    case par_DevicePassthrough:
        if (Status != SrvRunning)
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
int TMBTcpDevice::SetDeviceBind(byte DeviceID, const char* Address, int Port)
{
    LastError = 0;
    if (Status != SrvRunning)
    {
        FDeviceID = DeviceID;
        strncpy(FLocalAddress, Address, 15);
        LocalPort = word(Port);
        return 0;
    }
    else
        return SetError(errCategoryProcess, errDevCannotRebindOnRun);
}
//------------------------------------------------------------------------------
int TMBTcpDevice::Start()
{
    ClientsBlocked = 0;
    return SetError(errCategoryNetSocket, TCustomTcpServer::Start());
}
//------------------------------------------------------------------------------
int TMBTcpDevice::SetUserFunction(byte FunctionID, bool Value)
{
    return SetError(errCategoryProcess, ws->SetCustomFunction(FunctionID, Value));
}
//------------------------------------------------------------------------------
PWorkerSocket TMBTcpDevice::CreateWorkerSocket(socket_t Sock)
{
    PWorkerSocket Result;
    Result = new TMBTcpWorker();
    Result->SetSocket(Sock);
    PMBWorker(Result)->Device=this;
    PMBWorker(Result)->ws = ws;
    return Result;
}
//------------------------------------------------------------------------------
bool TMBTcpDevice::CanAccept(socket_t Socket)
{
    bool Result = TCustomTcpServer::CanAccept(Socket);
    if (Result)
    {
        if (PeerListMode!=plmDisabled)
        {
            int PeerIndex = FindPeer(Msg_GetSockAddr(Socket));
            Result=((PeerIndex==-1) && (PeerListMode==plmBlockList)) || ((PeerIndex>-1) && (PeerListMode==plmAllowList));
        }
    }  
    if (!Result)
    {
        sockaddr_in PeerSin;
        int NameLen = sizeof(PeerSin);
#ifdef SNAP_OS_WINDOWS
        getpeername(Socket, (struct sockaddr*)&PeerSin, &NameLen);
        DoEvent(PeerSin.sin_addr.S_un.S_addr, evcClientRefused, 0, 0, 0, 0, 0);
#else
        getpeername(Socket, (struct sockaddr*)&PeerSin, (socklen_t*)&NameLen);
        DoEvent(PeerSin.sin_addr.s_addr, evcClientRefused, 0, 0, 0, 0, 0);
#endif        
        ClientsBlocked++;
    }
    return Result;
}
//------------------------------------------------------------------------------
int TMBTcpDevice::SetError(int Category, int ErrNo)
{
    if (ErrNo)
        LastError = BaseObjectError | Category | ErrNo;
    else
        LastError = 0;
    return LastError;
}
//------------------------------------------------------------------------------
int TMBTcpDevice::FindPeer(longword Peer)
{
    for(int c=0; c<PeerCount; c++)
    {
        if (PeerList[c]==Peer)
            return c;
    }
    return -1;
}
//------------------------------------------------------------------------------
void TMBTcpDevice::ClearErrors()
{
    LastError = 0;
}
//------------------------------------------------------------------------------
int TMBTcpDevice::AddPeer(const char *Address)
{
    if (PeerCount<MaxTCPPeersList)
    {
        PeerList[PeerCount]=inet_addr(Address);
        PeerCount++;
        return mbNoError;
    }
    return SetError(errCategoryProcess, errDevTooManyPeers);
}
//------------------------------------------------------------------------------
int TMBTcpDevice::GetClientsList(PPeerList List)
{
    if (PeerCount > 0)
        memcpy(List, &PeerList, PeerCount * sizeof(longword));
    return PeerCount;
}
//------------------------------------------------------------------------------
int TMBTcpDevice::GetDeviceInfo(TDeviceInfo& DeviceInfo)
{
    DeviceInfo.LastError = LastError;
    DeviceInfo.Running = SrvRunning;
    DeviceInfo.ClientsCount = ClientsCount;
    DeviceInfo.ClientsBlocked = ClientsBlocked;
    return 0;
}
//------------------------------------------------------------------------------
int TMBTcpDevice::RegisterArea(int AreaID, void* Data, int Amount)
{
    if (Status != SrvRunning)
        return SetError(errCategoryProcess, ws->RegisterArea(AreaID, Data, Amount));
    else
        return SetError(errCategoryProcess, errDevOpNotAllowed);
}
//------------------------------------------------------------------------------
int TMBTcpDevice::LockArea(int AreaID)
{
    return SetError(errCategoryProcess, ws->LockArea(AreaID));
}
//------------------------------------------------------------------------------
int TMBTcpDevice::UnlockArea(int AreaID)
{
    return SetError(errCategoryProcess, ws->UnlockArea(AreaID));
}
//------------------------------------------------------------------------------
int TMBTcpDevice::CopyArea(int AreaID, word Address, word Amount, void* Data, int CopyMode)
{
    return SetError(errCategoryProcess, ws->CopyArea(AreaID, Address, Amount, Data, CopyMode));
}
//------------------------------------------------------------------------------
int TMBTcpDevice::RegisterCallback(int CallbackID, void* cbRequest, void* UsrPtr)
{
    if (Status != SrvRunning)
    {
        if (CallbackID == cbkDeviceEvent)
            return SetError(errCategoryProcess, SetEventsCallBack(cbRequest, UsrPtr)); // use EventCallback of the ancestor TCustomTcpServer
        else
            return SetError(errCategoryProcess, ws->RegisterCallback(CallbackID, cbRequest, UsrPtr));
    }
    else
        return SetError(errCategoryProcess, errDevOpNotAllowed);
}
//------------------------------------------------------------------------------
bool TMBTcpDevice::PickEventAsText(char* Text, int TextSize)
{
    TSrvEvent Event;
    bool Result = PickEvent(&Event);
    if (Result)
        _EventText(PDeviceEvent(&Event), Text, TextSize);
    return Result;
}
