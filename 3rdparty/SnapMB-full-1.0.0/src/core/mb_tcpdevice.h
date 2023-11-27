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
#ifndef mb_tcpdevice_h
#define mb_tcpdevice_h
//------------------------------------------------------------------------------
#include "snap_tcpsrvr.h"
#include "mb_defines.h"
#include "mb_utils.h"
#include "mb_device.h"
#include "mb_text.h"

class TMBTcpDevice; // forward declaration

class TMBTcpWorker : public TTcpSocket, public TMBDeviceExecutor
{
private:
    TMBNetDevicePacket Request;
    TMBNetDevicePacket Answer;
    TDeviceParams Params;
    bool DisconnectOnError;
    longword DisElapsed;
    bool TCPSendTCPConfirmation(word PDUSize);
    bool TCPSendRTUConfirmation(word PDUSize);
    void ClearBuffers();
    int TCPRecvTCPIndication();
    int TCPRecvRTUIndication();
protected:
    void DoEvent(longword Code, word RetCode, word Param1, word Param2,
        word Param3, word Param4);
public:
    TMBTcpDevice *Device;
    int NotifyError(int Error, int SckError = 0);
	//--------------------------------------------------------------------------
	TMBTcpWorker();
	~TMBTcpWorker();
	// Worker execution
	virtual int Execute();
};
typedef TMBTcpWorker *PMBWorker;
//------------------------------------------------------------------------------

class TMBTcpDevice : public TCustomTcpServer
{
private:
    char ChannelName[ChannelNameSize];
    int PeerCount;
    TPeerList PeerList;
    TDeviceInfo DeviceStat;
    int PeerListMode;
    int FindPeer(longword Peer);
    void ClearErrors();
protected:
    byte FDeviceID;
    bool Passthrough;
    int PacketLog;
    int Proto;
    int DisconnectTimeout;
    int BaseObjectError;
    int ClientsBlocked;
    PDeviceWorkingSet ws;
    PWorkerSocket CreateWorkerSocket(socket_t Sock);
    bool CanAccept(socket_t Socket);
    int SetError(int Category, int ErrNo);
public:
    int WorkInterval;
    int LastError;
    TMBTcpDevice(int Proto, byte DeviceID, const char* Address, int Port);
    ~TMBTcpDevice();
    int Start();
    int SetParam(int ParamIndex, int Value);
    int SetDeviceBind(byte DeviceID, const char* Address, int Port);
    int SetUserFunction(byte FunctionID, bool Value);
    int AddPeer(const char *Address);
    int GetClientsList(PPeerList List);
    int GetDeviceInfo(TDeviceInfo& DeviceInfo);
    int RegisterArea(int AreaID, void* Data, int Amount);
    int LockArea(int AreaID);
    int UnlockArea(int AreaID);
    int CopyArea(int AreaID, word Address, word Amount, void* Data, int CopyMode);
    int RegisterCallback(int CallbackID, void* cbRequest, void* UsrPtr);
    bool PickEventAsText(char* Text, int TextSize);
    friend class TMBTcpWorker;
};
typedef TMBTcpDevice* PMBTcpDevice;

//------------------------------------------------------------------------------
#endif // mb_tcpdevice_h
