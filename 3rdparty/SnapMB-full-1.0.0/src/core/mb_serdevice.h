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
#ifndef mb_serdevice_h
#define mb_serdevice_h
//------------------------------------------------------------------------------
#include "snap_threads.h"
#include "snap_sersock.h"
#include "snap_evtqueue.h"
#include "mb_defines.h"
#include "mb_utils.h"
#include "mb_device.h"
#include "mb_sthdevice.h"

#define MaxErrors 3

class TMBSerDevice; // forward declaration

//------------------------------------------------------------------------------
// Worker
//------------------------------------------------------------------------------
class TMBSerWorker : public TSnapThread, public TMBDeviceExecutor
{
private:
	int ErrorCount;
	TMBSerDevice* Device;
	PSerialSocket sersck;
	TMBRtuDevicePacket Request;
	TMBRtuDevicePacket Answer;
	char AsciiBuffer[mbBulkData];
	void ClearBuffers();
	void ManageError();
	bool IsBroadcast(byte DeviceID, byte Function);
	TIndicationResult RecvRtuIndication(char FirstChar);
	TIndicationResult RecvAscIndication(char FirstChar);
	void SendRtuConfirmation(word PDUSize);
	void SendAscConfirmation(word PDUSize);
public:
	TMBSerWorker(TMBSerDevice* Device, PSerialSocket SerialSocket);
	~TMBSerWorker();
	void Execute();
};
typedef TMBSerWorker* PMBSerWorker;

//------------------------------------------------------------------------------
// Device 
//------------------------------------------------------------------------------
class TMBSerDevice : public TMBSingleThreadDevice
{
private:
	PSerialSocket sersck;
	PMBSerWorker Worker;
	void* FUsrPtr;
	void ClearErrors();
protected:
	byte FDeviceID;
	int Format;
	bool Destroying;
	int WorkInterval;
	void CreateWorker();
	void DestroyWorker();
public:
	int LastError;
	longword LogMask;
	longword EventMask;
	TMBSerDevice(int Format, byte DeviceID, const char* Name, int BaudRate, char Parity, int DataBits, int Stops, int Flow);
	~TMBSerDevice();
	int SetParam(int ParamIndex, int Value);
	int SetDeviceBind(byte DeviceID, const char* Name, int BaudRate, char Parity, int DataBits, int Stops, int Flow);
	int SetUserFunction(byte FunctionID, bool Value);
	int GetInterframe(int& InterframeDelay, int& MaxInterframeDetected);
	int GetDeviceInfo(TDeviceInfo& DeviceInfo);
	int Start();
	void Stop();
	friend class TMBSerWorker;
};
typedef TMBSerDevice* PMBSerDevice;











//------------------------------------------------------------------------------
#endif // mb_serdevice_h
