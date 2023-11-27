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
#include "mb_sthdevice.h"

//------------------------------------------------------------------------------
void TMBSingleThreadDevice::DoEvent(int Sender, longword Code, word RetCode, word Param1, word Param2, word Param3, word Param4)
{
	TSrvEvent SrvEvent;

	if (!Destroying)
	{
		csEvent->Enter();

		time(&SrvEvent.EvtTime);
		SrvEvent.EvtSender = Sender;
		SrvEvent.EvtCode = Code;
		SrvEvent.EvtRetCode = RetCode;
		SrvEvent.EvtParam1 = Param1;
		SrvEvent.EvtParam2 = Param2;
		SrvEvent.EvtParam3 = Param3;
		SrvEvent.EvtParam4 = Param4;

		if (ws->DeviceEvent.OnRequest != NULL)
		try
		{ // callback is outside here, we have to shield it
			ws->DeviceEvent.OnRequest(FUsrPtr, &SrvEvent, sizeof(TSrvEvent));
		}
		catch (...)	{};

		FEventQueue->Insert(&SrvEvent);

		csEvent->Leave();
	};
}
//------------------------------------------------------------------------------
void TMBSingleThreadDevice::DoEvent(PSrvEvent Event)
{
	DoEvent(Event->EvtSender, Event->EvtCode, Event->EvtRetCode, Event->EvtParam1, Event->EvtParam2, Event->EvtParam3, Event->EvtParam4);
}
//------------------------------------------------------------------------------
int TMBSingleThreadDevice::SetError(int Category, int ErrNo)
{
	if (ErrNo)
		LastError = BaseObjectError | Category | ErrNo;
	else
		LastError = 0;
	return LastError;
}
//------------------------------------------------------------------------------
TMBSingleThreadDevice::TMBSingleThreadDevice()
{
	Status = DevStopped;
	LastError = 0;
	PacketLog = def_PacketLog;
	Destroying = false;
	Passthrough = false;
	FUsrPtr = NULL;
	BaseObjectError = 0;
	csEvent = new TSnapCriticalSection();
	FEventQueue = new TMsgEventQueue(MaxEvents, sizeof(TSrvEvent));
	ws = new TDeviceWorkingSet(false);
	memset(&DeviceStat, 0, sizeof(DeviceStat));
}
//------------------------------------------------------------------------------
TMBSingleThreadDevice::~TMBSingleThreadDevice()
{
	delete ws;
	delete FEventQueue;
	delete csEvent;
}
//------------------------------------------------------------------------------
bool TMBSingleThreadDevice::PickEvent(void* pEvent)
{
	try
	{
		return FEventQueue->Extract(pEvent);
	}
	catch (...)
	{
		return false;
	};
}
//------------------------------------------------------------------------------
bool TMBSingleThreadDevice::PickEventAsText(char* Text, int TextSize)
{
	TSrvEvent Event;
	bool Result = PickEvent(&Event);
	if (Result)
		_EventText(PDeviceEvent(&Event), Text, TextSize);
	return Result;
}
//------------------------------------------------------------------------------
bool TMBSingleThreadDevice::EventEmpty()
{
	return FEventQueue->Empty();
}
//------------------------------------------------------------------------------
void TMBSingleThreadDevice::EventsFlush()
{
	csEvent->Enter();
	FEventQueue->Flush();
	csEvent->Leave();
}
//------------------------------------------------------------------------------
int TMBSingleThreadDevice::RegisterArea(int AreaID, void* Data, int Amount)
{
	if (Status != DevRunning)
		return SetError(errCategoryProcess, ws->RegisterArea(AreaID, Data, Amount));
	else
		return SetError(errCategoryProcess, errDevOpNotAllowed);
}
//------------------------------------------------------------------------------
int TMBSingleThreadDevice::LockArea(int AreaID)
{
	return SetError(errCategoryProcess, ws->LockArea(AreaID));
}
//------------------------------------------------------------------------------
int TMBSingleThreadDevice::UnlockArea(int AreaID)
{
	return SetError(errCategoryProcess, ws->UnlockArea(AreaID));
}
//------------------------------------------------------------------------------
int TMBSingleThreadDevice::CopyArea(int AreaID, word Address, word Amount, void* Data, int CopyMode)
{
	return SetError(errCategoryProcess, ws->CopyArea(AreaID, Address, Amount, Data, CopyMode));
}
//------------------------------------------------------------------------------
int TMBSingleThreadDevice::RegisterCallback(int CallbackID, void* cbRequest, void* UsrPtr)
{
	if (Status != DevRunning)
		return SetError(errCategoryProcess, ws->RegisterCallback(CallbackID, cbRequest, UsrPtr));
	else
		return SetError(errCategoryProcess, errDevOpNotAllowed);
}
