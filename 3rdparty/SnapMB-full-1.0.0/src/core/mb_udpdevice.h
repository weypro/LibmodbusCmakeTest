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
#ifndef mb_udpdevice_h
#define mb_udpdevice_h
//------------------------------------------------------------------------------
#include "snap_threads.h"
#include "snap_msgsock.h"
#include "snap_evtqueue.h"
#include "mb_defines.h"
#include "mb_device.h"
#include "mb_utils.h"
#include "mb_sthdevice.h"
//------------------------------------------------------------------------------
class TMBUdpDevice;

//------------------------------------------------------------------------------
// Worker
//------------------------------------------------------------------------------
class TMBUdpWorker : public TSnapThread, public TMBDeviceExecutor
{
private:
	sockaddr_in ClientSin;
	TMBUdpDevice* Device;
	PUdpSocket udpsck;
	TMBNetDevicePacket Request;
	TMBNetDevicePacket Answer;
	int NotifyError(int Error, int SckError = 0);
	void ClearBuffers();
	void ClearErrors();
	longword ClientHandle(sockaddr_in CSin);
	bool UDPRecvTCPIndication();
	bool UDPRecvRTUIndication();
	bool UDPSendTCPConfirmation(word PDUSize);
	bool UDPSendRTUConfirmation(word PDUSize);
public:
	longword LocalHandle;
	longword LocalBind;
	int LastError;
	TMBUdpWorker(TMBUdpDevice* UdpDevice);
	~TMBUdpWorker();
	void Execute();
};
typedef TMBUdpWorker* PMBUdpWorker;


//------------------------------------------------------------------------------
// Device 
//------------------------------------------------------------------------------
class TMBUdpDevice : public TMBSingleThreadDevice
{
private:
	PUdpSocket udpsck;
	PMBUdpWorker Worker;
	PSnapCriticalSection csEvent;
	PMsgEventQueue FEventQueue;
	int PeerCount;
	TPeerList PeerList;
	int PeerListMode;
	int ClientsBlocked;
	int FindPeer(longword Peer);
	void* FUsrPtr;
protected:
	byte FDeviceID;
	int Proto;
	char LocalAddress[16];
	word LocalPort;
	bool Destroying;
	int CreateWorker();
	void DestroyWorker();
	bool CanAccept(longword PeerHandle);
public:
	int WorkInterval;
	int LastError;
	longword LogMask;
	longword EventMask;
	TMBUdpDevice(int Proto, byte DeviceID, const char* Address, int Port);
	virtual ~TMBUdpDevice();
	int SetParam(int ParamIndex, int Value);
	int SetDeviceBind(byte DeviceID, const char* Address, int Port);
	int SetUserFunction(byte FunctionID, bool Value);
	int Start();
	void Stop();
	int AddPeer(const char* Address);
	int GetDeviceInfo(TDeviceInfo& DeviceInfo);
	friend class TMBUdpWorker;
};
typedef TMBUdpDevice* PMBUdpDevice;
















#endif // mb_udpdevice_h

