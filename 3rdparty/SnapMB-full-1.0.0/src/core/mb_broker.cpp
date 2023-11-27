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
#include "mb_broker.h"
//******************************************************************************
// Modbus Client Ancestor
//******************************************************************************
TMBBroker::TMBBroker()
{
	LastError = 0;
    JobStart = 0;
	MaxRetries = def_MaxRetries_net;
	memset(&TxPDU, 0, sizeof(TxPDU));
	memset(&RxPDU, 0, sizeof(RxPDU));
	Error = PErrorResponse(&RxPDU);
	BaseZero = def_BaseAddressZero;
	IsSerialBroker = false;
	cs = new TSnapCriticalSection();
}
//------------------------------------------------------------------------------
TMBBroker::~TMBBroker()
{
	delete cs;
}
//------------------------------------------------------------------------------
inline void TMBBroker::Lock()
{
	cs->Enter();
}
//------------------------------------------------------------------------------

inline void TMBBroker::Unlock()
{
	cs->Leave();
}
//------------------------------------------------------------------------------
bool TMBBroker::ValidBroadcast(byte Function)
{
	// Only write functions can be broadcast
	return (Function == fun_WriteSingleCoil) ||
		(Function == fun_WriteSingleRegister) ||
		(Function == fun_WriteMultipleCoils) ||
		(Function == fun_WriteMultipleRegisters) ||
		(Function == fun_WriteFileRecord);
}
//------------------------------------------------------------------------------
int TMBBroker::Transaction(byte DeviceID, void* DataOut, void* DataIn, word SizeOut, word& PDUSizeIn, PPDURecvExpected PRE)
{
	// Check for invalid Broadcast. Not all functions can be broadcast
	if (DeviceID == 0 && IsSerialBroker && !ValidBroadcast(PRE->Function))
		return SetError(errCategoryProcess, errInvalidBroadcastFunction);
	
	// SendRequest and ReceiveResponse must be in the same function due to error recovery.
	// TCP/UDP and Serial Trasmission cannot detect low-level errors, the only way is into
	// the data receive.

	if (SendRequestRecvResponse(DeviceID, pbyte(DataIn), PDUSizeIn, pbyte(DataOut), SizeOut, PRE))
		return mbNoError;
	else
		return LastError;
}
//------------------------------------------------------------------------------
int TMBBroker::SetError(int Category, int ErrNo)
{
	if (ErrNo)
		LastError = BaseObjectError | Category | ErrNo;
	else
		LastError = 0;
	return LastError;
}
//------------------------------------------------------------------------------
// Data Read Request, common for Bits and Registers
//------------------------------------------------------------------------------
int TMBBroker::RequestDataRead(byte Function, byte DeviceID, word Address, word Amount, TDataType DataType, void* pUsrData)
{
	int Result = 0;
	word SizeIn;
	word SizeExpected; // Byte expected
	TPDURecvExpected PRE = {DeviceID, Function, 0};

	PDataReadReqHeader Request = PDataReadReqHeader(&TxPDU);
	PDataReadResPacket Answer = PDataReadResPacket(&RxPDU);

	ClearErrors();
	Request->Function = Function;
	Request->Address = SwapWord(Address); // Registers/bits number are 1-based, the protocol PDU needs 0-based
	Request->Amount = SwapWord(Amount);

	if (DataType == TDataType::dtBit)
		SizeExpected = word(RoundToNextByte(Amount) + 2); // Adding Function and byteCount
	else 
		SizeExpected = Amount * Reg16Size + 2; // Adding Function and byteCount

	PRE.Size = SizeExpected; 

	Result = Transaction(DeviceID, Request, Answer, sizeof(TDataReadReqHeader), SizeIn, &PRE);

	if (Result == mbNoError)
	{
		// Check Return
		if (Answer->Function == Function)
		{
			// Check the sizes
			if (SizeIn == SizeExpected && Answer->ByteCount + 2 == SizeIn)
			{
				memcpy(pUsrData, Answer->Data, Answer->ByteCount);
			}
			else
				SetError(errCategoryProcess, errInvalidADUReceived);
		}
		else
			Result = SetError(errCategoryMBProtocol, Error->Exception);

	}
	return Result;
}
//------------------------------------------------------------------------------
// Data Write Request, common for Bits and Registers
//------------------------------------------------------------------------------
int TMBBroker::RequestDataWrite(byte Function, byte DeviceID, word Address, word Amount, TDataType DataType, void* pUsrData)
{
	int Result = 0;
	word SizeIn;
	TPDURecvExpected PRE = { DeviceID, Function, sizeof(TDataWriteResHeader)}; // 5 = Function + 2 Start Address + 2 Quantity
	PDataWriteReqPacket Request = PDataWriteReqPacket(&TxPDU);
	PDataWriteResHeader Answer = PDataWriteResHeader(&RxPDU);

	ClearErrors();
	Request->Header.Function = Function;
	Request->Header.Address = SwapWord(Address); 
	Request->Header.Amount = SwapWord(Amount);

	if (DataType == TDataType::dtWord)
	{
		Request->Header.ByteCount = byte(Amount * Reg16Size);
		CopyAndSwapReg16Pack(PRegisters16(Request->Data), PRegisters16(pUsrData), Amount);
	}
	else // Data is ready into Request->Data
		Request->Header.ByteCount = byte(RoundToNextByte(Amount));

	Result = Transaction(DeviceID, Request, Answer, sizeof(TDataWriteReqHeader) + Request->Header.ByteCount, SizeIn, &PRE);
	if (Result == 0)
	{
		if (!IsSerialBroker || DeviceID != 0)
		{

			// Check Return
			if (Answer->Function == Function)
			{
				if (Answer->Address != Request->Header.Address || Answer->Amount != Request->Header.Amount || SizeIn != sizeof(TDataWriteResHeader))
					Result = SetError(errCategoryProcess, errInvalidADUReceived);
			}
			else
				Result = SetError(errCategoryMBProtocol, Error->Exception);
		}

	}
	return Result;
}
//-----------------------------------------------------------------------------------
// Request of single item write (Bit or Register)
//-----------------------------------------------------------------------------------
int TMBBroker::RequestSingleDataWrite(byte Function, byte DeviceID, word Address, word Value)
{
    JobStart = SysGetTick();
	int Result = mbNoError;
	word SizeIn;
	TPDURecvExpected PRE = { DeviceID, Function, sizeof(TDataSingleWritePacket) }; // 5 = Function + 2 Address + 2 Value
	PDataSingleWritePacket Request = PDataSingleWritePacket(&TxPDU);
	PDataSingleWritePacket Answer = PDataSingleWritePacket(&RxPDU);

	ClearErrors();
	Request->Function = Function;
	Request->Address = SwapWord(Address); 
	Request->Value = SwapWord(Value);

	Result = Transaction(DeviceID, Request, Answer, sizeof(TDataSingleWritePacket), SizeIn, &PRE);
	if (Result == mbNoError)
	{
		if (!IsSerialBroker || DeviceID != 0)
		{
			// Check Return
			if (Answer->Function == Function)
			{
				// Check Answer consistence
				if (Answer->Address != Request->Address || SizeIn != sizeof(TDataSingleWritePacket))
					Result = SetError(errCategoryProcess, errInvalidADUReceived);
			}
			else
				Result = SetError(errCategoryMBProtocol, Error->Exception);
		}

	}
	SetStatus(DeviceID, SysGetTick() - JobStart, Result);
	return Result;
}
//-----------------------------------------------------------------------------------
// Request of Bits read
//-----------------------------------------------------------------------------------
int TMBBroker::ReadBits(byte Function, byte DeviceID, word Address, word Amount, void *pUsrData)
{
    JobStart = SysGetTick();
	int Result = 0;
	word BitSliceAmount;
	word StartAddress = Address;
	pbyte Target = pbyte(&RxPDU);
	pbyte Packed = pbyte(pUsrData);

	ClearErrors();

	if (Amount < 1)
    {
		SetError(errCategoryProcess, errInvalidDataAmount);	
		SetStatus(DeviceID, 0, LastError);
		return LastError;
    }

	word TotalAmount = Amount;

	while ((Result == 0) && (TotalAmount > 0))
	{
		BitSliceAmount = TotalAmount;
		if (BitSliceAmount > MaxReadMultipleBits)
			BitSliceAmount = MaxReadMultipleBits;

		Result = RequestDataRead(Function, DeviceID, StartAddress, BitSliceAmount, TDataType::dtBit, Target);

		if (Result == 0)
		{
			UnpackBits(Packed, Target, BitSliceAmount);
			// Set Pointers for the next step
			TotalAmount -= BitSliceAmount;
			StartAddress += BitSliceAmount;
			Target += uintptr_t(MaxReadMultipleBitsSize);
			Packed += uintptr_t(MaxReadMultipleBitsSize);
		}
	}
	SetStatus(DeviceID, SysGetTick() - JobStart, Result);
	return Result;
}
//-----------------------------------------------------------------------------------
// Request of Bits write
//-----------------------------------------------------------------------------------
int TMBBroker::WriteBits(byte Function, byte DeviceID, word Address, word Amount, void *pUsrData)
{
    JobStart = SysGetTick();
	int Result = 0;
	word BitSliceAmount;
	word TotalAmount;
	word StartAddress = Address;
	pbyte Source = pbyte(pUsrData);
	PDataWriteReqPacket Request = PDataWriteReqPacket(&TxPDU);

	ClearErrors();

	if (Amount < 1)
	{
		SetError(errCategoryProcess, errInvalidDataAmount);
		SetStatus(DeviceID, 0, LastError);
		return LastError;
    }

	TotalAmount = Amount;
	while ((Result == 0) && (TotalAmount > 0))
	{
		BitSliceAmount = TotalAmount;
		if (BitSliceAmount > MaxWriteMultipleBits)
			BitSliceAmount = MaxWriteMultipleBits;

		PackBits(Request->Data, Source, BitSliceAmount);

		Result = RequestDataWrite(Function, DeviceID, StartAddress, BitSliceAmount, TDataType::dtBit, Request->Data);

		if (Result == 0)
		{
			// Set Pointers for the next step
			TotalAmount -= BitSliceAmount;
			StartAddress += BitSliceAmount;
			Source += uintptr_t(MaxWriteMultipleBitsSize);
		}
	}
	SetStatus(DeviceID, SysGetTick() - JobStart, Result);
	return Result;
}
//-----------------------------------------------------------------------------------
// Read registers with Amount greater than 125, Max 32768 (64 Kbyte)
//-----------------------------------------------------------------------------------
int TMBBroker::ReadRegisters(byte Function, byte DeviceID, word Address, word Amount, void* pUsrData)
{
    JobStart = SysGetTick();
	int Result = 0;
	word SliceAmount;
	word TotalAmount;
	word SliceStart = Address;
	pbyte Target = pbyte(pUsrData);

	ClearErrors();

	if (Amount < 1 || Amount > SnapMB_MaxRegisters){
		SetError(errCategoryProcess, errInvalidDataAmount);
		SetStatus(DeviceID, 0, LastError);
		return LastError;
	}

	TotalAmount = Amount;
	while ((Result == 0) && (TotalAmount > 0))
	{
		SliceAmount = TotalAmount;
		if (SliceAmount > MaxReadMultipleRegisters)
			SliceAmount = MaxReadMultipleRegisters;

		Result = RequestDataRead(Function, DeviceID, SliceStart, SliceAmount, TDataType::dtWord, Target);

		if (Result==0)
		{
			// Set Pointers for the next step
			TotalAmount -= SliceAmount;
			SliceStart += SliceAmount;
			Target += uintptr_t(SliceAmount * uintptr_t(Reg16Size));
		}
	}

	if (Result == 0)
		SwapReg16Pack(PRegisters16(pUsrData), Amount);

	SetStatus(DeviceID, SysGetTick() - JobStart, Result);
	return Result;
}
//-----------------------------------------------------------------------------------
// Write Registers with Amount greater than 123, Max 32768 (64 Kbyte)
//-----------------------------------------------------------------------------------
int TMBBroker::WriteRegisters(byte Function, byte DeviceID, word Address, word Amount, void* pUsrData)
{
    JobStart = SysGetTick();
	int Result = 0;
	word SliceAmount;
	word TotalAmount;
	word SliceStart = Address;
	pbyte Source = pbyte(pUsrData);

	ClearErrors();

	if (Amount < 1 || Amount > SnapMB_MaxRegisters){
		SetError(errCategoryProcess, errInvalidDataAmount);
		SetStatus(DeviceID, 0, LastError);
		return LastError;
	}

	TotalAmount = Amount;
	while ((Result == 0) && (TotalAmount > 0))
	{
		SliceAmount = TotalAmount;
		if (SliceAmount > MaxWriteMultipleRegisters)
			SliceAmount = MaxWriteMultipleRegisters;

		Result = RequestDataWrite(Function, DeviceID, SliceStart, SliceAmount, TDataType::dtWord, Source);

		if (Result==0)
		{
			// Set Pointers for the next step
			TotalAmount -= SliceAmount;
			SliceStart += SliceAmount;
			Source += uintptr_t(SliceAmount * uintptr_t(Reg16Size));
		}
	}
	SetStatus(DeviceID, SysGetTick() - JobStart, Result);
	return Result;
}
//-----------------------------------------------------------------------------------
int TMBBroker::ReadInputRegisters(byte DeviceID, word Address, word Amount, void* pUsrData)
{
	Lock();
    int Result = ReadRegisters(fun_ReadInputRegisters, DeviceID, (BaseZero ? Address : --Address), Amount, pUsrData);
	Unlock();
	return Result;
}
//-----------------------------------------------------------------------------------
int TMBBroker::ReadHoldingRegisters(byte DeviceID, word Address, word Amount, void* pUsrData)
{
	Lock();
	int Result = ReadRegisters(fun_ReadHoldingRegisters, DeviceID, (BaseZero ? Address : --Address), Amount, pUsrData);
	Unlock();
	return Result;
}
//-----------------------------------------------------------------------------------
int TMBBroker::WriteSingleRegister(byte DeviceID, word Address, word Value)
{
	Lock();
	int Result = RequestSingleDataWrite(fun_WriteSingleRegister, DeviceID, (BaseZero ? Address : --Address), Value);
	Unlock();
	return Result;
}
//-----------------------------------------------------------------------------------
int TMBBroker::WriteMultipleRegisters(byte DeviceID, word Address, word Amount, void* pUsrData)
{
	Lock();
	int Result = WriteRegisters(fun_WriteMultipleRegisters, DeviceID, (BaseZero ? Address : --Address), Amount, pUsrData);
	Unlock();
	return Result;
}
//-----------------------------------------------------------------------------------
int TMBBroker::ReadCoils(byte DeviceID, word Address, word Amount, void* pUsrData)
{
	Lock();
	int Result = ReadBits(fun_ReadCoils, DeviceID, (BaseZero ? Address : --Address), Amount, pUsrData);
	Unlock();
	return Result;
}
//-----------------------------------------------------------------------------------
int TMBBroker::ReadDiscreteInputs(byte DeviceID, word Address, word Amount, void* pUsrData)
{
	Lock();
	int Result = ReadBits(fun_ReadDiscreteInputs, DeviceID, (BaseZero ? Address : --Address), Amount, pUsrData);
	Unlock();
	return Result;
}
//-----------------------------------------------------------------------------------
int TMBBroker::WriteMultipleCoils(byte DeviceID, word Address, word Amount, void* pUsrData)
{
	Lock();
	int Result = WriteBits(fun_WriteMultipleCoils, DeviceID, (BaseZero ? Address : --Address), Amount, pUsrData);
	Unlock();
	return Result;
}
//-----------------------------------------------------------------------------------
int TMBBroker::WriteSingleCoil(byte DeviceID, word Address, word Value)
{
	Lock();
	int Result = RequestSingleDataWrite(fun_WriteSingleCoil, DeviceID, (BaseZero ? Address : --Address), (Value!=0 ? 0xFF00:0x0000));
	Unlock();
	return Result;
}
//-----------------------------------------------------------------------------------
int TMBBroker::MaskWriteRegister(byte DeviceID, word Address, word AND_Mask, word OR_Mask)
{
	Lock();
	JobStart = SysGetTick();
    int Result = mbNoError;
	word SizeIn;
	TPDURecvExpected PRE = { DeviceID, fun_MaskWriteRegister, sizeof(TMaskRegisterPacket) }; // 7 = Function + 2 Address + 2 AND mask + 2 OR mask
	PMaskRegisterPacket Request = PMaskRegisterPacket(&TxPDU);
    PMaskRegisterPacket Answer = PMaskRegisterPacket(&RxPDU);

	ClearErrors();
	Request->Function = fun_MaskWriteRegister;
	Request->Address = BaseZero? SwapWord(Address) : SwapWord(Address - 1); 
	Request->AND_Mask = SwapWord(AND_Mask);
	Request->OR_Mask = SwapWord(OR_Mask);

	Result = Transaction(DeviceID, Request, Answer, sizeof(TMaskRegisterPacket), SizeIn, &PRE);
    if (Result == mbNoError)
    {
		// Check Return
		if (Answer->Function == fun_MaskWriteRegister)
		{
            // Check Answer consistence
            if (Answer->Address != Request->Address || Answer->AND_Mask != Request->AND_Mask || Answer->OR_Mask != Request->OR_Mask || SizeIn != sizeof(TMaskRegisterPacket))
				Result = SetError(errCategoryProcess, errInvalidADUReceived);
        }
		else
			Result = SetError(errCategoryMBProtocol, Error->Exception);

    }
	SetStatus(DeviceID, SysGetTick() - JobStart, Result);
	Unlock();
	return Result;
}
//-----------------------------------------------------------------------------------
int TMBBroker::ReadWriteMultipleRegisters(byte DeviceID, word RDAddress, word RDAmount, word WRAddress, word WRAmount, void* pRDUsrData, void* pWRUsrData)
{
	Lock();
	JobStart = SysGetTick();
    int Result = mbNoError;
	word SizeIn;
	word SizeExpected = RDAmount * Reg16Size;
	TPDURecvExpected PRE = {DeviceID, fun_ReadWriteMultipleRegisters, 0};
	PDataReadWriteMultiRegPacket Request = PDataReadWriteMultiRegPacket(&TxPDU);
    PDataReadResPacket Answer = PDataReadResPacket(&RxPDU);
	ClearErrors();

	if (RDAmount < 1 || RDAmount > MaxReadMultipleRegisters || WRAmount < 1 || WRAmount > MaxWriteMultiReg_fun17)
	{
		SetError(errCategoryProcess, errInvalidDataAmount);
		SetStatus(DeviceID, 0, LastError);
		Result = LastError;
	}

	if (Result == mbNoError)
	{
		Request->Header.Function = fun_ReadWriteMultipleRegisters;
		Request->Header.RDAddress = BaseZero ? SwapWord(RDAddress) : SwapWord(RDAddress - 1);
		Request->Header.RDAmount = SwapWord(RDAmount);
		Request->Header.WRAddress = BaseZero ? SwapWord(WRAddress) : SwapWord(WRAddress - 1);
		Request->Header.WRAmount = SwapWord(WRAmount);
		Request->Header.ByteCount = byte(WRAmount * Reg16Size);
		// Prepares Data
		CopyAndSwapReg16Pack(PRegisters16(Request->Data), PRegisters16(pWRUsrData), WRAmount);

		PRE.Size = 2 + RDAmount * Reg16Size; // Function + Bytecount + N * Data16

		Result = Transaction(DeviceID, Request, Answer, sizeof(TDataReadWriteMultiRegHeader) + Request->Header.ByteCount, SizeIn, &PRE);

		if (Result == mbNoError)
		{
			// Check Return
			if (Answer->Function == fun_ReadWriteMultipleRegisters)
			{
				int SizeRead = Answer->ByteCount;
				if (SizeRead == SizeExpected && SizeIn == PRE.Size) // Check that the payload size is exactly what we expected
					CopyAndSwapReg16Pack(PRegisters16(pRDUsrData), PRegisters16(Answer->Data), RDAmount);
				else
					Result = SetError(errCategoryProcess, errInvalidADUReceived);
			}
			else
				Result = SetError(errCategoryMBProtocol, Error->Exception);

		}
		SetStatus(DeviceID, SysGetTick() - JobStart, Result);
	}
	Unlock();
	return Result;
}

//-----------------------------------------------------------------------------------
int TMBBroker::ReadFileRecord(byte DeviceID, byte RefType, word FileNumber, word RecNumber, word RegsAmount, void* RecData)
{
	Lock();
	JobStart = SysGetTick();
    int Result = 0;
	word SizeIn;
	TPDURecvExpected PRE = { DeviceID, fun_ReadFileRecord, 0 };

	PFileRecordHeader Request = PFileRecordHeader(&TxPDU);
    PReadFileRecAnswer Answer = PReadFileRecAnswer(&RxPDU);

	ClearErrors();
    if (RegsAmount<1 || RegsAmount>MaxRDFileRecRegsAmount)
    {
		SetError(errCategoryProcess, errInvalidDataAmount);
		SetStatus(DeviceID, 0, LastError);
		Result = LastError;
	}
	if (Result == mbNoError)
	{
		Request->Function = fun_ReadFileRecord;
		Request->Length = FileRecordDefSize;
		Request->RefType = RefType;
		Request->FileNumber = SwapWord(FileNumber);
		Request->RecNumber = SwapWord(RecNumber);
		Request->RecLength = SwapWord(RegsAmount);

		word SizeExpected = RegsAmount * Reg16Size + 2;

		PRE.Size = SizeExpected + 2; // Add Function and DataLength field

		Result = Transaction(DeviceID, Request, Answer, Request->Length + 2, SizeIn, &PRE);
		if (Result == mbNoError)
		{
			// Check Return
			if (Answer->Function == fun_ReadFileRecord)
			{
				if (Answer->DataLength == SizeExpected && SizeIn == PRE.Size) // Check that the payload size is exactly what we expected
				{
					CopyAndSwapReg16Pack(PRegisters16(RecData), PRegisters16(Answer->Data), RegsAmount);
				}
				else
					Result = SetError(errCategoryProcess, errInvalidADUReceived);
			}
			else
				Result = SetError(errCategoryMBProtocol, Error->Exception);

		};
	};
	SetStatus(DeviceID, SysGetTick() - JobStart, Result);
	Unlock();
	return Result;
}
//-----------------------------------------------------------------------------
int TMBBroker::WriteFileRecord(byte DeviceID, byte RefType, word FileNumber, word RecNumber, word RegsAmount, void* RecData)
{
	Lock();
	JobStart = SysGetTick();
    int Result = 0;
	word SizeIn;
	TPDURecvExpected PRE = { DeviceID, fun_WriteFileRecord, 0 };

    PWriteFileRecPacket Request = PWriteFileRecPacket(&TxPDU);
    PWriteFileRecPacket Answer = PWriteFileRecPacket(&RxPDU);

    ClearErrors();
    if (RegsAmount<1 || RegsAmount > MaxWRFileRecRegsAmount) 
    {
		SetError(errCategoryProcess, errInvalidDataAmount);
		SetStatus(DeviceID, 0, LastError);
		Result = LastError;
	}
	if (Result == mbNoError)
	{
		Request->Header.Function = fun_WriteFileRecord;
		Request->Header.Length = byte((RegsAmount * Reg16Size) + 7);
		Request->Header.RefType = RefType;
		Request->Header.FileNumber = SwapWord(FileNumber);
		Request->Header.RecNumber = SwapWord(RecNumber);
		Request->Header.RecLength = SwapWord(RegsAmount);
		CopyAndSwapReg16Pack(PRegisters16(Request->Data), PRegisters16(RecData), RegsAmount);

		word PacketSize = Request->Header.Length + 2;
		PRE.Size = PacketSize;

		Result = Transaction(DeviceID, Request, Answer, PacketSize, SizeIn, &PRE);

		if (Result == mbNoError)
		{
			if (!IsSerialBroker || DeviceID != 0)
			{
				if (Answer->Header.Function == fun_WriteFileRecord)
				{
					// Request and Answer must be equal
					if (memcmp(TxPDU, RxPDU, PacketSize) != 0 || SizeIn != PRE.Size)
						Result = SetError(errCategoryProcess, errInvalidADUReceived);
				}
				else
					Result = SetError(errCategoryMBProtocol, Error->Exception);
			}

		}
	}
	SetStatus(DeviceID, SysGetTick() - JobStart, Result);
	Unlock();
	return Result;
}
//-----------------------------------------------------------------------------
int TMBBroker::ReadFIFOQueue(byte DeviceID, word Address, word& FifoCount, void* FIFO)
{
	Lock();
	JobStart = SysGetTick();
	int Result;
	word SizeIn;
	word SizeOut = sizeof(TReadFifoReqHeader);
	TPDURecvExpected PRE = { DeviceID, fun_ReadFIFOQueue, 0 }; // we don't know in advance how many byte to expect
	PReadFifoReqHeader Request = PReadFifoReqHeader(&TxPDU);
	PReadFifoReqAnswer Answer = PReadFifoReqAnswer(&RxPDU);

    Request->Function = fun_ReadFIFOQueue;
    Request->PtrAddress = SwapWord(Address);

	Result = Transaction(DeviceID, Request, Answer, SizeOut, SizeIn, &PRE);
	if (Result == mbNoError)
	{
		if (Answer->Function == fun_ReadFIFOQueue)
        {
            int SizeExpected = SwapWord(Answer->ByteCount) + 3; // see TReadFifoReqAnswer
            if (SizeExpected == SizeIn)
            {
                // Further Check on Fifo Items
                FifoCount = SwapWord(Answer->FifoCount);
                if (FifoCount * Reg16Size + 5 == SizeIn)
                {
                    // Everything seems OK
                    CopyAndSwapReg16Pack(PRegisters16(FIFO), PRegisters16(Answer->Data), FifoCount);
                }
                else
                    Result = SetError(errCategoryProcess, errInvalidADUReceived);
            }
            else
                Result = SetError(errCategoryProcess, errInvalidADUReceived);
        }
        else
			Result = SetError(errCategoryMBProtocol, Error->Exception);

	}
	SetStatus(DeviceID, SysGetTick() - JobStart, Result);
	Unlock();
	return Result;
}
//-----------------------------------------------------------------------------
int TMBBroker::ExecuteMEIFunction(byte DeviceID, byte MEI_Type, void* pWRUsrData, word WRSize, void* pRDUsrData, word& RDSize)
{
	Lock();
	JobStart = SysGetTick();
    int Result = 0;
	word SizeIn;
	TPDURecvExpected PRE = { DeviceID, fun_EncapsulatedIT, 0 };
	PMEIRequest Request = PMEIRequest(&TxPDU);
    PMEIAnswer Answer = PMEIAnswer(&RxPDU);

    ClearErrors();
    if (WRSize + 2 > MaxBinPDUSize) // 2 = (1)Function + (1)MEI_Type
    {
		SetError(errCategoryProcess, errInvalidDataAmount);
		SetStatus(DeviceID, 0, LastError);
		Result = LastError;
	}

	if (Result == mbNoError)
	{

		Request->Function = fun_EncapsulatedIT;
		Request->MEI_Type = MEI_Type;

		memcpy(Request->Data, pWRUsrData, WRSize);

		Result = Transaction(DeviceID, Request, Answer, WRSize + 2, SizeIn, &PRE);

		if (Result == mbNoError)
		{
			if (Answer->Function == fun_EncapsulatedIT)
			{
				if (Answer->MEI_Type == Request->MEI_Type)
				{
					RDSize = SizeIn - 2;
					memcpy(pRDUsrData, Answer->Data, RDSize);
				}
				else
					Result = SetError(errCategoryProcess, errInvalidADUReceived);
			}
			else
				Result = SetError(errCategoryMBProtocol, Error->Exception);

		}
	}
	SetStatus(DeviceID, SysGetTick() - JobStart, Result);
	Unlock();
	return Result;
}
//-----------------------------------------------------------------------------
int TMBBroker::ReadExceptionStatus(byte DeviceID, byte& Data)
{
	Lock();
	JobStart = SysGetTick();
	int Result;
	word SizeIn;
	word SizeOut = 1; // Only Function
	TPDURecvExpected PRE = { DeviceID, fun_ReadExceptionStatus, 2 };
	PReadExceptionStatusRequest Request = PReadExceptionStatusRequest(&TxPDU);
	PReadExceptionStatusAnswer Answer = PReadExceptionStatusAnswer(&RxPDU);

	Request->Function = fun_ReadExceptionStatus;
	Result = Transaction(DeviceID, Request, Answer, SizeOut, SizeIn, &PRE);

	if (Result == mbNoError)
	{
		// Check Return
		if (Answer->Function == fun_ReadExceptionStatus)
		{
			if (SizeIn == 2) // Check that the payload size is exactly what we expected
				Data = Answer->Data;
			else
				Result = SetError(errCategoryProcess, errInvalidADUReceived);
		}
		else
			Result = SetError(errCategoryMBProtocol, Error->Exception);
	}

	SetStatus(DeviceID, SysGetTick() - JobStart, Result);
	Unlock();
	return Result;
}
//-----------------------------------------------------------------------------------
int TMBBroker::Diagnostics(byte DeviceID, word SubFunction, void* pSendData, void* pRecvData, word ItemsToSend, word& ItemsReceived)
{
	Lock();
	JobStart = SysGetTick();
	int Result;
	word SizeOut = (ItemsToSend * Reg16Size) + 3; // 3 = (1)Function) + (2)SubFunction
	word SizeIn = 0;
	TPDURecvExpected PRE = { DeviceID, fun_Diagnostics, SizeOut };

	PDiagnosticPDU Request = PDiagnosticPDU(&TxPDU);
	PDiagnosticPDU Answer = PDiagnosticPDU(&RxPDU);

	if (ItemsToSend > 0 && ItemsToSend <= MaxDiagnosticsItems)
	{
		Request->Function = fun_Diagnostics;
		Request->SubFunction = SwapWord(SubFunction);
		CopyAndSwapReg16Pack(PRegisters16(Request->Data), PRegisters16(pSendData), ItemsToSend);

		Result = Transaction(DeviceID, Request, Answer, SizeOut, SizeIn, &PRE);
		if (Result == mbNoError)
		{
			// Check Return
			if (Answer->Function == fun_Diagnostics)
			{
                if (Answer->SubFunction == Request->SubFunction)
                {
                    // Diagnostics is very Vendor specific, we don't know in advance,
                    // for a specific sub-function sent to a sepcific hardware, how
                    // many bytes we have to receive. We just check that they fit into
                    // the PDU
					ItemsReceived = (SizeIn - 3) / Reg16Size;			
					if (ItemsReceived <= MaxDiagnosticsItems)
                    {
                        CopyAndSwapReg16Pack(PRegisters16(pRecvData), PRegisters16(Answer->Data), ItemsReceived);
                        // ritornare gli items ricevuti
                    }
                    else
                        Result = SetError(errCategoryProcess, errInvalidADUReceived);
                }
				else
					Result = SetError(errCategoryProcess, errInvalidADUReceived);
			}
			else
				Result = SetError(errCategoryMBProtocol, Error->Exception);

		}
	}
	else
		Result = SetError(errCategoryProcess, errInvalidDataAmount);

	SetStatus(DeviceID, SysGetTick() - JobStart, Result);
	Unlock();
	return Result;
}
//-----------------------------------------------------------------------------------
int TMBBroker::GetCommEventCounter(byte DeviceID, word& Status, word& EventCount)
{
	Lock();
	JobStart = SysGetTick();
	int Result;
	word SizeOut = 1; // Only Function
	word SizeIn = 0;
	TPDURecvExpected PRE = { DeviceID, fun_GetCommEventCounter, sizeof(TGetEventCounterAnswer)};
	PGetEventCounterRequest Request = PGetEventCounterRequest(&TxPDU);
	PGetEventCounterAnswer Answer = PGetEventCounterAnswer(&RxPDU);

	Request->Function = fun_GetCommEventCounter;

	Result = Transaction(DeviceID, Request, Answer, SizeOut, SizeIn, &PRE);
	if (Result == mbNoError)
	{
        if (Answer->Function == fun_GetCommEventCounter)
        {
            if (SizeIn == sizeof(TGetEventCounterAnswer))
            {
                    Status = SwapWord(Answer->Status);
                    EventCount = SwapWord(Answer->EventCount);
            }
            else
                Result = SetError(errCategoryProcess, errInvalidADUReceived);
        }
        else
			Result = SetError(errCategoryMBProtocol, Error->Exception);

	}

	SetStatus(DeviceID, SysGetTick() - JobStart, Result);
	Unlock();
	return Result;
}
//-----------------------------------------------------------------------------------
int TMBBroker::GetCommEventLog(byte DeviceID, word& Status, word& EventCount, word& MessageCount, word& NumItems, void* Events)
{
	Lock();
	JobStart = SysGetTick();
	int Result;
	word SizeOut = 1; // Only Function
	word SizeIn = 0;
	TPDURecvExpected PRE = { DeviceID, fun_GetCommEventLog, 0 }; // we don't know in advance how many byte to expect
	PGetCommEventLogRequest Request = PGetCommEventLogRequest(&TxPDU);
	PGetCommEventLogAnswer Answer = PGetCommEventLogAnswer(&RxPDU);

	Request->Function = fun_GetCommEventLog;

	Result = Transaction(DeviceID, Request, Answer, SizeOut, SizeIn, &PRE);
	if (Result == mbNoError)
	{
        if (Answer->Function == fun_GetCommEventLog)
        {
            int SizeExpected = Answer->ByteCount + 2; // adding Function and Bytecont itself
            if (SizeExpected == SizeIn)
            {
                NumItems = SizeIn - 8; // skip the first 8 bytes, see TReadGetCommEventLogAnswer
                Status = SwapWord(Answer->Status);
                EventCount = SwapWord(Answer->EventCount);
                MessageCount = SwapWord(Answer->MessageCount);
                // Copy the frame
				if (NumItems > 0)
					memcpy(Events, Answer->Events, NumItems);
            }
            else
                Result = SetError(errCategoryProcess, errInvalidADUReceived);
        }
		else
			Result = SetError(errCategoryMBProtocol, Error->Exception);

	}
	SetStatus(DeviceID, SysGetTick() - JobStart, Result);
	Unlock();
	return Result;
}
//-----------------------------------------------------------------------------------
int TMBBroker::ReportServerID(byte DeviceID, void* pUsrData, int& DataSize)
{
	Lock();
	JobStart = SysGetTick();
	int Result;
	word SizeOut = 1; // Only Function
	word SizeIn = 0;
	TPDURecvExpected PRE = { DeviceID, fun_ReportServerID, 0 }; // we don't know in advance how many byte to expect
	PServerIDRequest Request = PServerIDRequest(&TxPDU);
	PServerIDAnswer Answer = PServerIDAnswer(&RxPDU);

	Request->Function = fun_ReportServerID;
	Result = Transaction(DeviceID, Request, Answer, SizeOut, SizeIn, &PRE);
	if (Result == mbNoError)
	{
        if (Answer->Function == fun_ReportServerID)
        {
            int SizeExpected = Answer->ByteCount + 2; // adding Function and Bytecount itself
            if (SizeExpected == SizeIn)
            {
                    DataSize = Answer->ByteCount; 
					if (DataSize > 0) 
						memcpy(pUsrData, Answer->Data, DataSize);
            }
            else
                Result = SetError(errCategoryProcess, errInvalidADUReceived);
        }
        else
			Result = SetError(errCategoryMBProtocol, Error->Exception);

	}
	SetStatus(DeviceID, SysGetTick() - JobStart, Result);
	Unlock();
	return Result;
}
//-----------------------------------------------------------------------------------
int TMBBroker::CustomFunctionRequest(byte DeviceID, byte UsrFunction, void* pUsrPDUWrite, word SizePDUWrite, void* pUsrPDURead, word& SizePDURead, word SizePDUExpected)
{
	Lock();
	JobStart = SysGetTick();
	int Result;
	word TotalWRSize = SizePDUWrite + 1; // Add the Function Number
	TPDURecvExpected PRE = { DeviceID, UsrFunction, SizePDUExpected }; // SizePDUExpected can be 0 if we don't know it in advance
	TUsrFunctionRequest Request;

	if (SizePDUWrite > 0 && TotalWRSize <= MaxBinPDUSize) // Check the Modbus max PDU Size
	{
		if ((UsrFunction & 0x80) == 0x00) // The function cannot be an error answer
		{
			Request.Function = UsrFunction;
			memcpy(&Request.Data, pUsrPDUWrite, SizePDUWrite);
			
			Result = Transaction(DeviceID, &Request, pUsrPDURead, TotalWRSize, SizePDURead, &PRE);
			if (Result == mbNoError)
			{
				if (pbyte(pUsrPDURead)[0] != UsrFunction )
					Result = SetError(errCategoryMBProtocol, pbyte(pUsrPDURead)[1]);
			}
		}
		else
			Result = SetError(errCategoryProcess, errInvalidUserFunction);
	}
	else
		Result = SetError(errCategoryProcess, errInvalidDataAmount);

	SetStatus(DeviceID, SysGetTick() - JobStart, Result);
	Unlock();
	return Result;
}
//-----------------------------------------------------------------------------------
int TMBBroker::RawRequest(byte DeviceID, void* pUsrPDUWrite, word SizePDUWrite, void* pUsrPDURead, word& SizePDURead, word SizePDUExpected)
{
	Lock();
	JobStart = SysGetTick();
	int Result;
	TPDURecvExpected PRE = { DeviceID, pbyte(pUsrPDUWrite)[0], SizePDUExpected }; // SizePDUExpected can be 0 if we don't know it in advance
	SizePDURead = 0;
	if (SizePDUWrite > 0 && SizePDUWrite <= MaxBinPDUSize) // Check the Modbus max PDU Size
	{
		Result = Transaction(DeviceID, pUsrPDUWrite, pUsrPDURead, SizePDUWrite, SizePDURead, &PRE);
	}
	else
		Result = SetError(errCategoryProcess, errInvalidDataAmount);

	SetStatus(DeviceID, SysGetTick() - JobStart, Result);
	Unlock();
	return Result;
}
