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
#include "mb_netclient.h"
//-----------------------------------------------------------------------------
TMBEthernetClient::TMBEthernetClient(int Proto, const char* IP, int Port)
{   
    BaseObjectError = errEthernetClient;
    this->Proto = Proto;
    tcpsck = NULL;
    udpsck = NULL;
    netsck = NULL; // netsck is the ancestor, we will use it for common functions (tha same for TCP and UDP)

    if (Proto == ProtoUDP || Proto == ProtoRTUOverUDP) {
        udpsck = new TUdpSocket();
        netsck = udpsck;
    }
    else {
        tcpsck = new TTcpSocket();
        netsck = tcpsck;
    }
    strncpy(netsck->RemoteAddress, IP, 15);
    netsck->RemotePort = word(Port);
    netsck->SendTimeout = def_TCPSndTimeout;

    Connected = false;
    LastTransID = 0;

    // Set default params, they can be changed via SetRemoteDeviceParam function
    DisconnectOnError = def_NETDisconnectOnError;
    Persistent = def_NETPersistence;

    Params.AutoTimeout = def_AutoTimeout;
    Params.AutoTimeLimit_Min = def_AutoTimeLimit_Min;
    Params.FixedTimeout_ms = def_FixedTimeout_ms;
    Params.AcceptBroadcast = def_AcceptBroadcast;
    Params.TimeMin = def_FixedTimeout_ms;
    Params.TimeMax = 0;
    Params.AutoTimeCalc_ms = def_FixedTimeout_ms;

    AttemptSleep = def_AttemptSleep;
    MaxRetries = def_MaxRetries_net;

    ClearBuffers();
    ClearErrors();
    memset(&DeviceStat, 0, sizeof(DeviceStat));
}
//-----------------------------------------------------------------------------
// Local Params
//-----------------------------------------------------------------------------
int TMBEthernetClient::SetLocalParam(int ParamIndex, int Value)
{
    switch (ParamIndex)
    {
    case par_TCP_UDP_Port:
        if (Value != netsck->RemotePort) // avoid disconnection in case of same values
        {
            if (netsck->Connected)   // The Port change forces a disconnection
                netsck->SckDisconnect();
            netsck->RemotePort = word(Value);
        }
        break;
    case par_BaseAddress:
        BaseZero = Value;
        break;
    case par_TcpPersistence:
        Persistent = Value;
        break;
    case par_DisconnectOnError:
        DisconnectOnError = Value;
        break;
    case par_SendTimeout:
        netsck->SendTimeout = Value;
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
        return SetRemoteDeviceParam(0, ParamIndex, Value);      
    }
    return mbNoError;
}
//------------------------------------------------------------------------------
// Multiplex function tho change the device params
//------------------------------------------------------------------------------
int TMBEthernetClient::SetRemoteDeviceParam(byte DeviceID, int ParamIndex, int Value)
{
    switch (ParamIndex)
    {
    case par_AutoTimeout:
        Params.AutoTimeout = Value;
        break;
    case par_AutoTimeLimitMin:
        Params.AutoTimeLimit_Min = Value;
        break;
    case par_FixedTimeout:
        Params.FixedTimeout_ms = Value;
        break;
    default:
        return SetError(errCategoryProcess,errInvalidParamIndex);
    }
    return mbNoError;
}
//-----------------------------------------------------------------------------
TMBEthernetClient::~TMBEthernetClient()
{
    if (tcpsck != NULL)
        delete tcpsck;
    if (udpsck != NULL)
        delete udpsck;
}
//-----------------------------------------------------------------------------
void TMBEthernetClient::ClearBuffers()
{
    memset(&SndPacket, 0, sizeof(SndPacket));
    memset(&RcvPacket, 0, sizeof(RcvPacket));
}
//-----------------------------------------------------------------------------
void TMBEthernetClient::ClearErrors()
{
    LastError = 0;
    netsck->LastNetError = 0;
}
//-----------------------------------------------------------------------------
void TMBEthernetClient::SetStatus(byte DeviceID, longword Time, int JobResult)
{
    DeviceStat.LastError = JobResult;
    DeviceStat.JobTime = Time;
}
//-----------------------------------------------------------------------------
// Create a new Transaction ID
//-----------------------------------------------------------------------------
word TMBEthernetClient::NewTransID()
{
    LastTransID++;
    if (LastTransID == 0xFFFF)
        LastTransID = 1;
    return LastTransID;
}
//------------------------------------------------------------------------------
// Get Buffers for debug purpose
//------------------------------------------------------------------------------
int TMBEthernetClient::GetIOBufferPtr(int BufferKind, pbyte& Data)
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
//-----------------------------------------------------------------------------
int TMBEthernetClient::GetIOBuffer(int BufferKind, pbyte Data)
{
    if (BufferKind == BkSnd)
    {
        if (SndPacket.Size > word(sizeof(SndPacket.ADU)))
            SndPacket.Size = word(sizeof(SndPacket.ADU));
        memcpy(Data, &SndPacket.ADU, SndPacket.Size);
        return SndPacket.Size;
    }
    else {
        if (RcvPacket.Size > word(sizeof(RcvPacket.ADU)))
            RcvPacket.Size = word(sizeof(RcvPacket.ADU));
        memcpy(Data, &RcvPacket.ADU, RcvPacket.Size);
        return RcvPacket.Size;
    }
}
//-----------------------------------------------------------------------------
int TMBEthernetClient::GetDeviceStatus(byte DeviceID, TDeviceStatus& DeviceStatus)
{
    DeviceStatus = DeviceStat;
    DeviceStatus.Connected = netsck->Connected;
    return 0;
}
//-----------------------------------------------------------------------------
int TMBEthernetClient::Connect()
{
    int Result = netsck->SckConnect();
    Connected = Result == mbNoError;
    if (!Connected)
    {
        DeviceStat.Status = _StatusError;
        Result = SetError(errCategoryNetSocket, netsck->LastNetError);
    }
    else
        DeviceStat.Status = _StatusOk;
    DeviceStat.LastError = Result;
    return Result;
}
//-----------------------------------------------------------------------------
// Disconnection
//-----------------------------------------------------------------------------
void TMBEthernetClient::Disconnect()
{
    netsck->SckDisconnect();
    DeviceStat.LastError = 0;
    DeviceStat.JobTime = 0;
    DeviceStat.Status = _StatusOk;
    Connected = false;
}
//-----------------------------------------------------------------------------
void TMBEthernetClient::PrepareTCPRequest(byte DeviceID, pbyte DataOut, word SizeOut)
{
    // Builds the Header
    SndPacket.ADU.MBAP.TransID = SwapWord(NewTransID());
    SndPacket.ADU.MBAP.ProtocolID = tcp_mb_protoid;
    SndPacket.ADU.MBAP.PduLength = SwapWord(SizeOut + 1); // +1 because we need to add the UnitID byte
    SndPacket.ADU.MBAP.UnitID = DeviceID;
    // Copies the data
    memcpy(&SndPacket.ADU.PDU, DataOut, SizeOut);
    SndPacket.Size = MBAPSize + SizeOut;
}
//-----------------------------------------------------------------------------
void TMBEthernetClient::PrepareRTURequest(byte DeviceID, pbyte DataOut, word SizeOut)
{
    PMBSERPacket Request = PMBSERPacket(&SndPacket.ADU);   
    Request->ADU[0] = DeviceID;
    memcpy(&Request->ADU[1], DataOut, SizeOut);
    uint16_t CRC16 = CalcCRC((uint8_t*)(&SndPacket.ADU), SizeOut + 1);
    Request->ADU[SizeOut + 1] = CRC16 & 0x00FF;
    Request->ADU[SizeOut + 2] = (CRC16 >> 8);
    SndPacket.Size = SizeOut + 3; // Adding UnitID and CRC16
}
//-----------------------------------------------------------------------------
// Prepares the Request
//-----------------------------------------------------------------------------
void TMBEthernetClient::PrepareRequest(byte DeviceID, pbyte DataOut, word SizeOut)
{
    ClearBuffers();
    ClearErrors();
    CurrentFun = *DataOut;
    switch (Proto)
    {
    case ProtoTCP:
    case ProtoUDP:
        PrepareTCPRequest(DeviceID, DataOut, SizeOut);
        break;
    case ProtoRTUOverTCP:
    case ProtoRTUOverUDP:
        PrepareRTURequest(DeviceID, DataOut, SizeOut);
        break;
    }
}
//-----------------------------------------------------------------------------
// Sends the Request to the Device using TCP or UDP Socket
//-----------------------------------------------------------------------------
bool TMBEthernetClient::SendRequest(byte DeviceID)
{
    LastError = 0;
    if (Connected || (Connect() == mbNoError))
    {
        if (Proto == ProtoTCP || Proto == ProtoRTUOverTCP)
        {
            if (tcpsck->SendPacket(&SndPacket, SndPacket.Size) != mbNoError)
                SetError(errCategoryNetSocket, tcpsck->LastNetError);
        }
        else
        {
            if (udpsck->SendPacket(&SndPacket, SndPacket.Size) != mbNoError)
                SetError(errCategoryNetSocket, udpsck->LastNetError);
        }
        if (LastError != mbNoError)
            Disconnect();
    }
    else
        SetError(errCategoryNetSocket, netsck->LastNetError);

    return LastError == mbNoError;
}
//------------------------------------------------------------------------------
bool TMBEthernetClient::SendRequestRecvResponse(byte DeviceID, pbyte DataIn, word& SizeIn, pbyte DataOut, word SizeOut, PPDURecvExpected PRE)
{
    int Retries = MaxRetries;
    bool Result;
    PrepareRequest(DeviceID, DataOut, SizeOut);
    do
    {
        ClearErrors();
        Result = SendRequest(DeviceID) && RecvResponse(DeviceID, DataIn, SizeIn, PRE);

        // Updates Device Status
        if (!Result)
        {
            DeviceStat.Status = _StatusError;
            if (netsck->LastNetError != 0)
            {
                if (netsck->LastNetError == WSAETIMEDOUT)
                    DeviceStat.Status = _StatusTimeout;
            }
            else 
                if (CurrentFun != *DataIn)
                    DeviceStat.Status = _StatusProtoError;
        }
        else
            DeviceStat.Status = _StatusOk;

        if (!Result)
        {
            if (netsck->LastNetError == WSAETIMEDOUT) // Do not recover timeout, because it's a "legal" rensponse
                Retries = 0;
            else
                if (--Retries)
                    SysSleep(AttemptSleep);
        }
        
    } while (!Result && Retries);

    if (Connected && !Persistent)
        Disconnect();

    DeviceStat.LastError = LastError;
    DeviceStat.Connected = Connected;

    return LastError == mbNoError;
}
//-----------------------------------------------------------------------------
// Validates the incoming MBAP
//-----------------------------------------------------------------------------
bool TMBEthernetClient::ValidateConfirmationHeader()
{
    return (RcvPacket.ADU.MBAP.ProtocolID == SndPacket.ADU.MBAP.ProtocolID) &&
        (RcvPacket.ADU.MBAP.TransID == SndPacket.ADU.MBAP.TransID) &&
        (RcvPacket.ADU.MBAP.UnitID == SndPacket.ADU.MBAP.UnitID);
}
//-----------------------------------------------------------------------------
// Receives the TCP Packet. First is read the MBAP and then the Body
//-----------------------------------------------------------------------------
int TMBEthernetClient::TCPRecvTCPPacket(void* Data, word& PDUSizeIn)
{
    int Result;

    // Set Timeout
    if (Params.AutoTimeout)
        tcpsck->RecvTimeout = Params.AutoTimeCalc_ms;
    else
        tcpsck->RecvTimeout = Params.FixedTimeout_ms;

    // Receive Header
    Result = tcpsck->RecvPacket(&RcvPacket.ADU.MBAP, MBAPSize);
    if (Result == mbNoError)
    {
        RcvPacket.Size = MBAPSize;
        if (ValidateConfirmationHeader()) // Check MBAP
        {
            PDUSizeIn = SwapWord(RcvPacket.ADU.MBAP.PduLength) - 1; // byte remaining after MBAP, -1 because after PduLength there is another byte which we already received
            if (PDUSizeIn > 0 && PDUSizeIn <= MaxBinPDUSize) // MaxSizeIn to protect against buffer overflow, the PDU consistence will be analyzed at higher level
            {
                Result = tcpsck->RecvPacket(RcvPacket.ADU.PDU, PDUSizeIn);
                if (Result == mbNoError)
                {
                    memcpy(Data, &RcvPacket.ADU.PDU, PDUSizeIn);
                    RcvPacket.Size = PDUSizeIn + MBAPSize;
                }
            }
            else
                Result = errInvalidADUReceived;
        }
        else
            Result = errInvalidADUReceived;
    };
     
    if (Result != mbNoError)
    {
        // Network Error != timeout -> Disconnect
        // InvalidADU Error -> Disconnect if DisconnectOnError==true

        if (Result == errInvalidADUReceived)
        {
            if (DisconnectOnError)
                Disconnect();
            SetError(errCategoryProcess,errInvalidADUReceived);
        }
        else // network error
        { 
            if (tcpsck->LastNetError != WSAETIMEDOUT)
            {
                SetError(errCategoryNetSocket, tcpsck->LastNetError);
                Disconnect();
            }
            else
                SetError(errCategoryProcess,errTimeout);
        }
    }

    return LastError;
}
//-----------------------------------------------------------------------------
int TMBEthernetClient::TCPRecvRTUPacket(void* Data, word& PDUSizeIn)
{
    int Result = mbNoError;
    PRTUAnswer Answer = PRTUAnswer(&RcvPacket.ADU);
    PRTURequest Request = PRTURequest(&SndPacket.ADU);
    PRTUError RtuError = PRTUError(&RcvPacket.ADU);

    // Set Timeout
    if (Params.AutoTimeout)
        tcpsck->RecvTimeout = Params.AutoTimeCalc_ms;
    else
        tcpsck->RecvTimeout = Params.FixedTimeout_ms;

    // Receive the whole Packet
    int SizeRecvd;
    Result = tcpsck->Receive(Answer, mbBulkData, SizeRecvd);

    if (Result == mbNoError)
    {
        if (SizeRecvd >= MinimumRtuADUAnswer && Answer->DeviceID == Request->DeviceID)
        {
            if (Answer->Function == Request->Function)
            {
                RcvPacket.Size = word(SizeRecvd);
                if (CheckCRC16(pbyte(&RcvPacket.ADU), word(SizeRecvd)))
                {
                    // Everything was ok, we can copy the PDU
                    PDUSizeIn = word(SizeRecvd - 3); // skip DeviceID and CRC
                    memcpy(Data, &Answer->Function, PDUSizeIn);
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
                        *pbyte(Data) = RtuError->ErrorCode;
                        *(pbyte(Data) + 1) = RtuError->Exception;
                        PDUSizeIn = 2;
                    }
                    else
                        Result = errInvalidChecksum;
                }
                else // it was rubbish
                    Result = errInvalidADUReceived;
            }
        }
        else // Malformed ADU
            Result = errInvalidADUReceived;
    };

    if (Result != mbNoError)
    {
        // Network Error != timeout -> Disconnect
        // InvalidADU || invalid Checksum  -> Disconnect if DisconnectOnError==true

        if (Result == errInvalidADUReceived || Result == errInvalidChecksum)
        {
            if (DisconnectOnError)
                Disconnect();
            SetError(errCategoryProcess,Result);
        }
        else // network error
        {
            if (tcpsck->LastNetError != WSAETIMEDOUT)
            {
                SetError(errCategoryNetSocket, tcpsck->LastNetError);
                Disconnect();
            }
            else
                SetError(errCategoryProcess,errTimeout);
        }
    }

    return LastError;
}
//-----------------------------------------------------------------------------
// Receives the UDP Packet, it's format just like the TCP one.
// Since the UDP receives a datagram, we get the whole Packet
//-----------------------------------------------------------------------------
int TMBEthernetClient::UDPRecvTCPPacket(void* Data, word& PDUSizeIn)
{
    int Result = 0;

    // Set Timeout
    if (Params.AutoTimeout)
        udpsck->RecvTimeout = Params.AutoTimeCalc_ms;
    else
        udpsck->RecvTimeout = Params.FixedTimeout_ms;

    // Receive the whole Packet into ADU, if the incoming Packet Size is greater, the remaining is
    // discarded in accord to UDP rules (that's good since we don't really need it)
    int SizeRecvd = udpsck->RecvPacket(&RcvPacket.ADU, MaxNetADUSize);
    Result = udpsck->LastNetError;

    if (Result == mbNoError)
    {
        // The smallest, well formatted TCP/UDP ADU, contains MBAP + ErrCode + Exception
        int MinimumSize = MBAPSize + 2;
        if (SizeRecvd >= MinimumSize)
        {
            // Check the MBAP size declared
            int SizeExpected = SwapWord(RcvPacket.ADU.MBAP.PduLength) - 1 + MBAPSize;
            if (SizeRecvd == SizeExpected)
            {
                // The MBAP is well formed, now let's check the remaining fields
                if (ValidateConfirmationHeader()) // Check MBAP
                {
                    PDUSizeIn = word(SizeRecvd - MBAPSize);
                    memcpy(Data, &RcvPacket.ADU.PDU, PDUSizeIn);
                    RcvPacket.Size = word(SizeRecvd);
                }
                else
                    Result = errInvalidADUReceived;
            }
            else
                Result = errInvalidADUReceived;
        }
        else
            Result = errInvalidADUReceived;
    };

    if (Result != mbNoError)
    {
        // Network Error != timeout -> Disconnect
        // InvalidADU Error -> Disconnect if DisconnectOnError==true

        if (Result == errInvalidADUReceived)
        {
            if (DisconnectOnError)
                Disconnect();
            SetError(errCategoryProcess,errInvalidADUReceived);
        }
        else // network error
        {
            if (udpsck->LastNetError != WSAETIMEDOUT)
            {
                SetError(errCategoryNetSocket, udpsck->LastNetError);
                Disconnect();
            }
            else
                SetError(errCategoryProcess,errTimeout);
        }
    }

    return LastError;
}
//------------------------------------------------------------------------------
int TMBEthernetClient::UDPRecvRTUPacket(void* Data, word& PDUSizeIn)
{
    int Result = mbNoError;
    PRTUAnswer Answer = PRTUAnswer(&RcvPacket.ADU);
    PRTURequest Request = PRTURequest(&SndPacket.ADU);
    PRTUError RtuError = PRTUError(&RcvPacket.ADU);

    // Set Timeout
    if (Params.AutoTimeout)
        udpsck->RecvTimeout = Params.AutoTimeCalc_ms;
    else
        udpsck->RecvTimeout = Params.FixedTimeout_ms;

    // Receive the whole Packet into ADU, if the incoming Packet Size is greater, the remaining is
    // discarded in accord to UDP rules (that's good since we don't really need it)

    int SizeRecvd = udpsck->RecvPacket(Answer, mbBulkData);
    Result = udpsck->LastNetError;

    if (Result == mbNoError)
    {
        if (SizeRecvd >= MinimumRtuADUAnswer && Answer->DeviceID == Request->DeviceID)
        {
            if (Answer->Function == Request->Function)
            {
                RcvPacket.Size = word(SizeRecvd);
                if (CheckCRC16(pbyte(&RcvPacket.ADU), word(SizeRecvd)))
                {
                    // Everything was ok, we can copy the PDU
                    PDUSizeIn = word(SizeRecvd - 3); // skip DeviceID and CRC
                    memcpy(Data, &Answer->Function, PDUSizeIn);
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
                        *pbyte(Data) = RtuError->ErrorCode;
                        *(pbyte(Data) + 1) = RtuError->Exception;
                        PDUSizeIn = 2;
                    }
                    else
                        Result = errInvalidChecksum;
                }
                else // it was rubbish
                    Result = errInvalidADUReceived;
            }
        }
        else // Malformed ADU
            Result = errInvalidADUReceived;
    };

    if (Result != mbNoError)
    {
        // Network Error != timeout -> Disconnect
        // InvalidADU || invalid Checksum  -> Disconnect if DisconnectOnError==true

        if (Result == errInvalidADUReceived || Result == errInvalidChecksum)
        {
            if (DisconnectOnError)
                Disconnect();
            SetError(errCategoryProcess,Result);
        }
        else // network error
        {
            if (udpsck->LastNetError != WSAETIMEDOUT)
            {
                SetError(errCategoryNetSocket, udpsck->LastNetError);
                Disconnect();
            }
            else
                SetError(errCategoryProcess,errTimeout);
        }
    }

    return LastError;
}
//------------------------------------------------------------------------------
// Receives the answer
//------------------------------------------------------------------------------
bool TMBEthernetClient::RecvResponse(byte DeviceID, pbyte DataIn, word& PDUSizeIn, PPDURecvExpected PRE)
{
    int Result = 0;
    longword Elapsed = SysGetTick(); // To calc AutoTimeout
    switch (Proto)
    {
    case ProtoTCP:
        Result = TCPRecvTCPPacket(DataIn, PDUSizeIn);
        break;
    case ProtoUDP:
        Result = UDPRecvTCPPacket(DataIn, PDUSizeIn);
        break;
    case ProtoRTUOverTCP:
        Result = TCPRecvRTUPacket(DataIn, PDUSizeIn);
        break;
    case ProtoRTUOverUDP:
        Result = UDPRecvRTUPacket(DataIn, PDUSizeIn);
        break;
    }

    // Calcs Dynamic timeout regardless of Autotimeout parameter, to be ready if Autotimeout is set later
    if (Result == mbNoError) // Conditions to calc AutoTimeout
        CalcTimeout(Elapsed, &Params);

    return Result == mbNoError;
}

