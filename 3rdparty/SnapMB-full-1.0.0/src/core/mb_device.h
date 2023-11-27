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
#ifndef mb_device_h
#define mb_device_h
//-----------------------------------------------------------------------------
#include "snap_evtqueue.h"
#include "mb_defines.h"

typedef TSrvEvent* PDeviceEvent;

enum class TIndicationResult {
	irSkip,
	irDone,
	irSendAnswer,
	irNoAnswer,
	irDisconnect
};

class TSharedResources
{
private:
	PSnapCriticalSection cs;
public:
	TSharedDiscreteInputs DiscreteInputs;
	TSharedCoils Coils;
	TSharedInputRegisters InputRegisters;
	TSharedHoldingRegisters HoldingRegisters;
	void Lock();
	void Unlock();
	TSharedResources();
	~TSharedResources();
};

class TDeviceWorkingSet
{
private:
public:
	TDeviceEventCbk DeviceEvent;
	TPacketLog PacketLog;
	TDiscreteInputs DiscreteInputs;
	TCoils Coils;
	TInputRegisters InputRegisters;
	THoldingRegisters HoldingRegisters;
	TReadWriteRegisters ReadWriteRegisters;
	TMaskRegister MaskRegister;
	TFileRecord FileRecord;
	TExceptionStatus ExceptionStatus;
	TDiagnostics Diagnostics;
	TGetCommEventCounter CommEventCounter;
	TGetCommEventLog CommEventLog;
	TReportServerID ReportServerID;
	TFIFORequest FIFORequest;
	TMEIRequestCbk MEIRequestCbk;
	TUsrFunction UsrFunction;
	TPassthroughEvent Passthrough;

	bool IsSerialDevice;
	bool AllowSerFunOnEth;
	bool CustFunList[256];
	int RegisterArea(int AreaID, void* Data, int Amount);
	int RegisterCallback(int CallbackID, void* cbRequest, void* UsrPtr);
	int SetCustomFunction(byte FunctionID, bool Value);
	int LockArea(int AreaID);
	int UnlockArea(int AreaID);
	int CopyArea(int AreaID, word Address, word Amount, void* Data, int CopyMode);
	TDeviceWorkingSet(bool IsSerialDevice);
	~TDeviceWorkingSet();
};
typedef TDeviceWorkingSet* PDeviceWorkingSet;

class TMBDeviceExecutor
{
private:
	uint8_t Buffer[MaxReadMultipleBits]; // since it's wide enough, the same will be used for Registers (casting the pointer)
	TIndicationResult SetMBError(longword EvtCode, byte Function, byte Exception, PMBPDU TxPDU, word& TxPDUSize);
	TIndicationResult SetMBOk();
	TIndicationResult ManageReadCoils(PMBPDU RxPDU, word RxPDUSize, PMBPDU TxPDU, word& TxPDUSize);
	TIndicationResult ManageReadDiscreteInputs(PMBPDU RxPDU, word RxPDUSize, PMBPDU TxPDU, word& TxPDUSize);
	TIndicationResult ManageReadInputRegisters(PMBPDU RxPDU, word RxPDUSize, PMBPDU TxPDU, word& TxPDUSize);
	TIndicationResult ManageReadHoldingRegisters(PMBPDU RxPDU, word RxPDUSize, PMBPDU TxPDU, word& TxPDUSize);
    TIndicationResult ManageWriteSingleCoil(PMBPDU RxPDU, word RxPDUSize, PMBPDU TxPDU, word& TxPDUSize);
	TIndicationResult ManageWriteSingleRegister(PMBPDU RxPDU, word RxPDUSize, PMBPDU TxPDU, word& TxPDUSize);
	TIndicationResult ManageWriteMultipleCoils(PMBPDU RxPDU, word RxPDUSize, PMBPDU TxPDU, word& TxPDUSize);
	TIndicationResult ManageWriteMultipleRegisters(PMBPDU RxPDU, word RxPDUSize, PMBPDU TxPDU, word& TxPDUSize);
	TIndicationResult ManageMaskRegisterRequest(PMBPDU RxPDU, word RxPDUSize, PMBPDU TxPDU, word& TxPDUSize);
	TIndicationResult ManageReadWriteMultipleRegistersRequest(PMBPDU RxPDU, word RxPDUSize, PMBPDU TxPDU, word& TxPDUSize);
	TIndicationResult ManageReadFileRecordRequest(PMBPDU RxPDU, word RxPDUSize, PMBPDU TxPDU, word& TxPDUSize);
	TIndicationResult ManageWriteFileRecordRequest(PMBPDU RxPDU, word RxPDUSize, PMBPDU TxPDU, word& TxPDUSize);
	TIndicationResult ManageReadFIFOQueue(PMBPDU RxPDU, word RxPDUSize, PMBPDU TxPDU, word& TxPDUSize);
	TIndicationResult ManageEncapsulatedIT(PMBPDU RxPDU, word RxPDUSize, PMBPDU TxPDU, word& TxPDUSize);
	// Custom worker function
	TIndicationResult ManageCustomFunction(PMBPDU RxPDU, word RxPDUSize, PMBPDU TxPDU, word& TxPDUSize);
	// Serial specific worker functions
	TIndicationResult ManageReadExceptionStatus(PMBPDU RxPDU, word RxPDUSize, PMBPDU TxPDU, word& TxPDUSize);
	TIndicationResult ManageDiagnostics(PMBPDU RxPDU, word RxPDUSize, PMBPDU TxPDU, word& TxPDUSize);
	TIndicationResult ManageGetCommEventCounter(PMBPDU RxPDU, word RxPDUSize, PMBPDU TxPDU, word& TxPDUSize);
	TIndicationResult ManageGetCommEventLog(PMBPDU RxPDU, word RxPDUSize, PMBPDU TxPDU, word& TxPDUSize);
	TIndicationResult ManageReportServerID(PMBPDU RxPDU, word RxPDUSize, PMBPDU TxPDU, word& TxPDUSize);
protected:
	TSrvEvent Event;
	TIndicationResult ExecuteIndication(PMBPDU RxPDU, word RxPDUSize, PMBPDU TxPDU, word& TxPDUSize);
	void ClearEvents();
public:
	word EventsCount;
	PDeviceWorkingSet ws; // Don't Initialize/Delete it, it's Owned by the Device !!!
};
typedef TMBDeviceExecutor* PMBDeviceExecutor;


#endif // mb_device_h
