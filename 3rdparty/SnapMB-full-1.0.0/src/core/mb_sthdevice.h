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
#ifndef mb_sth_device_h
#define mb_sth_device_h
//------------------------------------------------------------------------------
#include "snap_threads.h"
#include "snap_evtqueue.h"
#include "mb_defines.h"
#include "mb_device.h"
#include "mb_text.h"
//------------------------------------------------------------------------------

//const longword WkTimeout = 3000; // Workers termination timeout
const int DevStopped = 0;
const int DevRunning = 1;

class TMBSingleThreadDevice
{
protected:
	PSnapCriticalSection csEvent;
	PMsgEventQueue FEventQueue;
	PDeviceWorkingSet ws;
	TDeviceInfo DeviceStat;
	int BaseObjectError;
	bool Destroying;
	void* FUsrPtr;
	void DoEvent(int Sender, longword Code, word RetCode, word Param1, word Param2, word Param3, word Param4);
	void DoEvent(PSrvEvent Event);
	int SetError(int Category, int ErrNo);
public:
	int LastError;
	int Status;
	int PacketLog;
	bool Passthrough;
	TMBSingleThreadDevice();
	virtual ~TMBSingleThreadDevice();
	bool PickEvent(void* pEvent);
	bool PickEventAsText(char* Text, int TextSize);
	bool EventEmpty();
	void EventsFlush();
	int RegisterArea(int AreaID, void* Data, int Amount);
	int LockArea(int AreaID);
	int UnlockArea(int AreaID);
	int CopyArea(int AreaID, word Address, word Amount, void* Data, int CopyMode);

	int RegisterCallback(int CallbackID, void* cbRequest, void* UsrPtr);
};


















#endif // !mb_sth_device_h

