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
#ifndef mb_netcontroller_h
#define mb_netcontroller_h
//-----------------------------------------------------------------------------
#include "mb_defines.h"
#include "mb_sercontroller.h"
#include "mb_serclient.h"
#include "mb_netclient.h"
#include "mb_broker.h"
//-----------------------------------------------------------------------------

class TMBFieldController : public TMBBroker
{
private:
	TFieldDevice FieldDevices[MaxDevices];
	//TDeviceInfo Status[MaxDevices];
	PMBSerController SerControllers[MaxChannels];
	int ControllersCount;
	PMBSerController FindSerController(const char* PortName);
protected:
	bool SendRequestRecvResponse(byte DeviceID, pbyte DataIn, word& SizeIn, pbyte DataOut, word SizeOut, PPDURecvExpected PRE);
	void SetStatus(byte DeviceID, longword Time, int JobResult);
	void ClearErrors();
	int ClearDeviceStatus(TDeviceStatus& DeviceStatus);
public:
	int Connect();
	int GetIOBufferPtr(byte DeviceID, int BufferKind, pbyte& Data);
	int GetIOBuffer(byte DeviceID, int BufferKind, pbyte Data);
	void Disconnect();
	int GetDeviceStatus(byte DeviceID, TDeviceStatus& DeviceStatus);
	int AddEthernetDevice(int Proto, byte DeviceID, const char* IP, int Port);
	int AddSerialDevice(int Format, byte DeviceID, const char* PortName, int BaudRate, char Parity, int DataBits, int Stops, int Flow);
	int SetLocalParam(byte LocalID, int ParamIndex, int Value);
	int SetRemoteDeviceParam(byte DeviceID, int ParamIndex, int Value);
	TMBFieldController();
	~TMBFieldController();
};
typedef TMBFieldController* PMBFieldController;

//-----------------------------------------------------------------------------
#endif // mb_netcontroller_h

