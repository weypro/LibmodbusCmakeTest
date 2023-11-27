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
#ifndef mb_sercontroller_h
#define mb_sercontroller_h
//-----------------------------------------------------------------------------
#include "snap_sersock.h"
#include "mb_defines.h"
#include "mb_utils.h"
#include "mb_serbroker.h"
#include "mb_serchannels.h"
//------------------------------------------------------------------------------
// Serial (RTU or ASCII) Controller (aka Master): Inherited Modbus Broker which 
// performs serial data exchange
//------------------------------------------------------------------------------
class TMBSerController : public TMBSerBroker
{
private:
	TDeviceParams Devices[MaxDevices];
	TDeviceStatus DeviceStat[MaxDevices];
	friend class TMBFieldController;
protected:
	PDeviceStatus GetDeviceStatusPtr(byte DeviceID);
	PDeviceParams GetDeviceParamsPtr(byte DeviceID);
	void SetStatus(byte DeviceID, longword Time, int JobResult);
public:
	int Connect();
	int GetDeviceStatus(byte DeviceID, TDeviceStatus& DeviceStatus);
	TMBSerController(const char* PortName, int BaudRate, char Parity, int DataBits, int Stops, int Flow); 
	~TMBSerController();
};
typedef TMBSerController* PMBSerController;

//-----------------------------------------------------------------------------
#endif // mb_sercontroller_h

