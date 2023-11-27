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
#include "mb_sercontroller.h"
//******************************************************************************
// Modbus Serial Controller (RTU/ASCII)
//******************************************************************************
TMBSerController::TMBSerController(const char* PortName, int BaudRate, char Parity, int DataBits, int Stops, int Flow)
{
	BaseObjectError = errSerialClient;
	ChannelsManager_GetChannel(PortName, BaudRate, Parity, DataBits, Stops, Flow, sersck);
	ComSignature = sersck->ComSignature;
	strncpy(this->PortName, PortName, NameSize - 1);

	// Set default values for all Devices
	for (int c = 0; c < MaxDevices; c++)
		SetDeviceDefaults(Devices[c]);

	memset(&DeviceStat, 0, sizeof(DeviceStat));
	ClearErrors();
}
//------------------------------------------------------------------------------
TMBSerController::~TMBSerController()
{
	ChannelsManager_DelChannel(sersck);
}
//------------------------------------------------------------------------------
int TMBSerController::GetDeviceStatus(byte DeviceID, TDeviceStatus& DeviceStatus)
{
	DeviceStatus = DeviceStat[DeviceID];
	DeviceStatus.Connected = sersck->Connected;
	return 0;
}
//------------------------------------------------------------------------------
// Serial Connection (Opens the Port)
//------------------------------------------------------------------------------
int TMBSerController::Connect()
{
	memset(&DeviceStat, 0, sizeof(DeviceStat));
	int Result = sersck->Connect();
	if (!sersck->Connected)
	{
		DeviceStat[0].Status = _StatusError;
		Result = SetError(errCategorySerialSocket, sersck->LastSerError);
	}
	else
		DeviceStat[0].Status = _StatusOk;
	return Result;
}
//------------------------------------------------------------------------------
// Pointer to Device status
//------------------------------------------------------------------------------
PDeviceStatus TMBSerController::GetDeviceStatusPtr(byte DeviceID)
{
	return PDeviceStatus(&DeviceStat[DeviceID]);
}
//------------------------------------------------------------------------------
// Pointer to Device params
//------------------------------------------------------------------------------
PDeviceParams TMBSerController::GetDeviceParamsPtr(byte DeviceID)
{
	return PDeviceParams(&Devices[DeviceID]);
}
//------------------------------------------------------------------------------
void TMBSerController::SetStatus(byte DeviceID, longword Time, int JobResult)
{
	DeviceStat[DeviceID].LastError = JobResult;
	DeviceStat[DeviceID].JobTime = Time;
}



