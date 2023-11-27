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
#ifndef mb_client_h
#define mb_client_h
//-----------------------------------------------------------------------------
#include "snap_sysutils.h"
#include "mb_defines.h"
#include "mb_utils.h"
//-----------------------------------------------------------------------------
enum class TDataType {
	dtBit,
	dtWord
};
//-----------------------------------------------------------------------------
// Ancestor Class for Modbus function/PDU handling, the low-level communication
// is performed bye the inherited classes (TCP/UDP and RTU)
//-----------------------------------------------------------------------------
class TMBBroker
{
private:
	PSnapCriticalSection cs;
	void Lock();
	void Unlock();
	// Checks if the request can be broadcast
	bool ValidBroadcast(byte Function);
	// Reads a group of registers/bits in accord to Modbus specifications : Amount must be max 250 byte
	int RequestDataRead(byte Function, byte DeviceID, word Address, word Amount, TDataType DataType, void* pUsrData);
	// Writes a group of registers/bits in accord to Modbus specifications : Amount must be max 246 byte (reg) or 255 byte (bits)
	int RequestDataWrite(byte Function, byte DeviceID, word Address, word Amount, TDataType DataType, void* pUsrData);
	// Writes a single Item register o Coil
	int RequestSingleDataWrite(byte Function, byte DeviceID, word Address, word Value);
	// Reads an arbitrary group of bitpack using several subsequent requests (RequestDataRead) if Amount > 2000
	int ReadBits(byte Function, byte DeviceID, word Address, word Amount, void* pUsrData);
	// Writes an arbitrary group of bitpack using several subsequent requests (RequestDataRead) if Amount > 2040
	int WriteBits(byte Function, byte DeviceID, word Address, word Amount, void *pUsrData);
	// Writes an arbitrary group of registers using several subsequent requests (RequestDataWrite) if Amount > 123
	int ReadRegisters(byte Function, byte DeviceID, word Address, word Amount, void* pUsrData);
	// Reads an arbitrary group of registers/bits using several subsequent requests (RequestDataRead) if Amount > 125 byte
	int WriteRegisters(byte Function, byte DeviceID, word Address, word Amount, void* pUsrData);
protected:
	byte TxPDU[mbBulkData];
	byte RxPDU[mbBulkData];
	PErrorResponse Error;
	bool BaseZero;
	//bool IsSerialController;
	int BaseObjectError;
	longword JobStart;
	int MaxRetries;
	bool DisconnectOnError;
	virtual void ClearErrors() = 0;
	// Abstract method which are inherited by specialized clients
	virtual bool SendRequestRecvResponse(byte DeviceID, pbyte DataIn, word& SizeIn, pbyte DataOut, word SizeOut, PPDURecvExpected PRE) = 0;
	virtual int Transaction(byte DeviceID, void* DataOut, void* DataIn, word SizeOut, word& PDUSizeIn, PPDURecvExpected PRE);
	virtual void SetStatus(byte DeviceID, longword Time, int JobResult) = 0;
	int SetError(int Category, int ErrNo);
public:
	int LastError;
	bool Connected;
	bool IsSerialBroker;
    //longword JobTime;
	TMBBroker();
	virtual ~TMBBroker();
	// Abstract method which are inherited by specialized clients
	virtual int Connect() = 0;
	virtual void Disconnect() = 0; // DeviceID used by NetController
	//-----------------------------------------------------------------------------
	// Modbus Class 0 Functions
	//-----------------------------------------------------------------------------
	int ReadHoldingRegisters(byte DeviceID, word Address, word Amount, void* pUsrData);
	int WriteMultipleRegisters(byte DeviceID, word Address, word Amount, void* pUsrData);
	//-----------------------------------------------------------------------------
	// Modbus Class 1 Functions
	//-----------------------------------------------------------------------------
	int ReadCoils(byte DeviceID, word Address, word Amount, void* pUsrData);
	int ReadDiscreteInputs(byte DeviceID, word Address, word Amount, void* pUsrData);
	int ReadInputRegisters(byte DeviceID, word Address, word Amount, void* pUsrData);
	int WriteSingleCoil(byte DeviceID, word Address, word Value);
	int WriteSingleRegister(byte DeviceID, word Address, word Value);
	//-----------------------------------------------------------------------------
	// Modbus Class 2 Functions
	//-----------------------------------------------------------------------------
    int ReadWriteMultipleRegisters(byte DeviceID, word RDAddress, word RDAmount, word WRAddress, word WRAmount, void* pRDUsrData, void* pWRUsrData);
	int WriteMultipleCoils(byte DeviceID, word Address, word Amount, void* pUsrData);
    int MaskWriteRegister(byte DeviceID, word Address, word AND_Mask, word OR_Mask);
    int ReadFileRecord(byte DeviceID, byte RefType, word FileNumber, word RecNumber, word RegsAmount, void* RecData);
    int WriteFileRecord(byte DeviceID, byte RefType, word FileNumber, word RecNumber, word RegsAmount, void* RecData);
	int ReadFIFOQueue(byte DeviceID, word Address, word& FifoCount, void* FIFO);
	//-----------------------------------------------------------------------------
	// Serial Only Functions
	//-----------------------------------------------------------------------------
	int ReadExceptionStatus(byte DeviceID, byte& Data);
	int Diagnostics(byte DeviceID, word SubFunction, void* pSendData, void* pRecvData, word ItemsToSend, word &ItemsReceived);
	int GetCommEventCounter(byte DeviceID, word& Status, word& EventCount);
	int GetCommEventLog(byte DeviceID, word& Status, word& EventCount, word& MessageCount, word &NumItems, void* Events);
	int ReportServerID(byte DeviceID, void* pUsrData, int &DataSize);
	// SERVERID
	//-----------------------------------------------------------------------------
	// Modbus Specialized Functions
	//-----------------------------------------------------------------------------
	int ExecuteMEIFunction(byte DeviceID, byte MEI_Type, void* pWRUsrData, word WRSize, void* pRDUsrData, word &RDSize);
	//-----------------------------------------------------------------------------
	// User Function
	//-----------------------------------------------------------------------------
	int CustomFunctionRequest(byte DeviceID, byte UsrFunction, void* pUsrPDUWrite, word SizePDUWrite, void* pUsrPDURead, word &SizePDURead, word SizePDUExpected);
	//-----------------------------------------------------------------------------
	// Raw (User debug or Gateway)
	//-----------------------------------------------------------------------------
	int RawRequest(byte DeviceID, void* pUsrPDUWrite, word SizePDUWrite, void* pUsrPDURead, word& SizePDURead, word SizePDUExpected);

};
typedef TMBBroker* PMBBroker;

//-----------------------------------------------------------------------------
#endif // mb_client_h


