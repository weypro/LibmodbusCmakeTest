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
// Transport agnostic Device
#include "snap_threads.h"
#include "snap_sysutils.h"
#include "mb_device.h"
#include "mb_utils.h"

//------------------------------------------------------------------------------

static TSharedResources Shared;

//------------------------------------------------------------------------------
TIndicationResult TMBDeviceExecutor::SetMBError(longword EvtCode, byte Function, byte Exception, PMBPDU TxPDU, word& TxPDUSize)
{
    TxPDU->Function = 0x80 + Function;
    TxPDU->Data[0] = Exception;
    TxPDUSize = 2;
    Event.EvtCode = EvtCode;
    Event.EvtRetCode = Exception;
    return TIndicationResult::irSendAnswer;
}
//------------------------------------------------------------------------------
TIndicationResult TMBDeviceExecutor::SetMBOk()
{
    EventsCount++;
    return TIndicationResult::irSendAnswer;
}
//------------------------------------------------------------------------------
TIndicationResult TMBDeviceExecutor::ManageWriteMultipleCoils(PMBPDU RxPDU, word RxPDUSize, PMBPDU TxPDU, word& TxPDUSize)
{
    byte ExceptionCode = 0;
    int Action = cbActionWrite;
    int RxPDUSizeExpected;
    longword EventCode;
    EventCode = evcWriteMultiCoils;
    Event.EvtCode = EventCode;

    // Check the function
    if (Shared.Coils.Data == NULL && ws->Coils.OnRequest == NULL)  // neither shared data ,nor callback
        return SetMBError(EventCode, RxPDU->Function, errIllegalFunction, TxPDU, TxPDUSize);

    PDataWriteReqPacket Request = PDataWriteReqPacket(RxPDU);
    PDataWriteResHeader Answer = PDataWriteResHeader(TxPDU);

    word Address = SwapWord(Request->Header.Address);
    word Amount = SwapWord(Request->Header.Amount);

    // Set Event Info
    Event.EvtParam3 = Address;
    Event.EvtParam4 = Amount;


    int ByteCount = Request->Header.ByteCount;
    int Payload = RoundToNextByte(Amount);
    RxPDUSizeExpected = Payload + DataWriteReqHeaderSize;
    // check the telegram
    if (RxPDUSizeExpected != RxPDUSize || ByteCount != Payload)
    {
        Event.EvtCode = evcInvalidADUReceived;
        return TIndicationResult::irSkip;
    }
    // Check the sizes
    if (Amount == 0 || Amount > MaxWriteMultipleBits)
        return SetMBError(EventCode, RxPDU->Function, errIllegalDataValue, TxPDU, TxPDUSize);

    // Check Bounds if we are using internal area, otherwise it must be done inside the user callback
    int EndAddress = Shared.Coils.Amount - 1;
    if (Shared.Coils.Data != NULL && (Address > EndAddress || EndAddress - Address < Amount))
        return SetMBError(EventCode, RxPDU->Function, errIllegalDataAddress, TxPDU, TxPDUSize);

    Answer->Function = Request->Header.Function;
    Answer->Address = Request->Header.Address;
    Answer->Amount = Request->Header.Amount;
    TxPDUSize = DataWriteResHeaderSize;

    // Use the Area, if shared
    if (Shared.Coils.Data != NULL)
    {
        pbyte Start;
        // Calc Source start point
        Start = pbyte(Shared.Coils.Data) + uintptr_t(Address);

        Shared.Coils.cs->Enter(); // Lock access
        try
        {
            UnpackBits(Start, Request->Data, Amount);
        }
        catch (...) {
            ExceptionCode = errSlaveDeviceFailure;
        }
        Shared.Coils.cs->Leave();
    }

    // use the Callback if assigned
    if (ws->Coils.OnRequest != NULL && ExceptionCode == 0)
    {
        UnpackBits(&Buffer[0], Request->Data, Amount);      
        ws->Coils.cs->Enter(); // Locks the callback
        try
        {
            ExceptionCode = byte(ws->Coils.OnRequest(ws->Coils.UsrPtr, Action, Address, Amount, &Buffer));
        }
        catch (...) {
            ExceptionCode = errSlaveDeviceFailure;
        }
        ws->Coils.cs->Leave();
    }

    if (ExceptionCode != 0)
        return SetMBError(EventCode, RxPDU->Function, ExceptionCode, TxPDU, TxPDUSize);
    else
        return SetMBOk();
}
//------------------------------------------------------------------------------
TIndicationResult TMBDeviceExecutor::ManageWriteMultipleRegisters(PMBPDU RxPDU, word RxPDUSize, PMBPDU TxPDU, word& TxPDUSize)
{
    byte ExceptionCode = 0;
    int Action = cbActionWrite;
    int RxPDUSizeExpected;
    longword EventCode;

    EventCode = evcWriteMultiRegs;

    Event.EvtCode = EventCode;

    // Check the function
    if (Shared.HoldingRegisters.Data == NULL && ws->HoldingRegisters.OnRequest == NULL)  // neither shared data ,nor callback
        return SetMBError(EventCode, RxPDU->Function, errIllegalFunction, TxPDU, TxPDUSize);

    PDataWriteReqPacket Request = PDataWriteReqPacket(RxPDU);
    PDataWriteResHeader Answer = PDataWriteResHeader(TxPDU);

    word Address = SwapWord(Request->Header.Address);
    word Amount = SwapWord(Request->Header.Amount);

    // Set Event Info
    Event.EvtParam3 = Address;
    Event.EvtParam4 = Amount;

    int ByteCount = Request->Header.ByteCount;
    int Payload = Amount * Reg16Size;
    RxPDUSizeExpected = Payload + DataWriteReqHeaderSize;
    if (RxPDUSizeExpected != RxPDUSize || ByteCount != Payload)
    {
        Event.EvtCode = evcInvalidADUReceived;
        return TIndicationResult::irSkip;
    }
    // Check the sizes
    if (Amount == 0 || Amount > MaxWriteMultipleRegisters)
        return SetMBError(EventCode, RxPDU->Function, errIllegalDataValue, TxPDU, TxPDUSize);

    // Check Bounds if we are using internal area, otherwise it must be done inside the user callback
    int EndAddress = Shared.HoldingRegisters.Amount - 1;
    if (Shared.HoldingRegisters.Data != NULL && (Address > EndAddress || EndAddress - Address < Amount))
        return SetMBError(EventCode, RxPDU->Function, errIllegalDataAddress, TxPDU, TxPDUSize);

    Answer->Function = Request->Header.Function;
    Answer->Address = Request->Header.Address;
    Answer->Amount = Request->Header.Amount;
    TxPDUSize = DataWriteResHeaderSize;

    // Use the Area, if shared
    if (Shared.HoldingRegisters.Data != NULL)
    {
        pbyte Start;
        // Calc Source start point 
        Start = pbyte(Shared.HoldingRegisters.Data) + uintptr_t(Address * uintptr_t(Reg16Size));

        Shared.HoldingRegisters.cs->Enter(); // Lock access
        try
        {
            CopyAndSwapReg16Pack(PRegisters16(Start), PRegisters16(Request->Data), Amount);
        }
        catch (...) {
            ExceptionCode = errSlaveDeviceFailure;
        }
        Shared.HoldingRegisters.cs->Leave();

    }

    // use the Callback if assigned
    if (ws->HoldingRegisters.OnRequest != NULL && ExceptionCode == 0)
    {
        ws->Coils.cs->Enter(); // Locks the callback
        try
        {
            CopyAndSwapReg16Pack(PRegisters16(&Buffer), PRegisters16(Request->Data), Amount);
            ExceptionCode = byte(ws->HoldingRegisters.OnRequest(ws->HoldingRegisters.UsrPtr, Action, Address, Amount, &Buffer));
        }
        catch (...) {
            ExceptionCode = errSlaveDeviceFailure;
        }
        ws->Coils.cs->Leave(); 
    }

    if (ExceptionCode != 0)
        return SetMBError(EventCode, RxPDU->Function, ExceptionCode, TxPDU, TxPDUSize);
    else
        return SetMBOk();
}
//------------------------------------------------------------------------------
TIndicationResult TMBDeviceExecutor::ManageWriteSingleCoil(PMBPDU RxPDU, word RxPDUSize, PMBPDU TxPDU, word& TxPDUSize)
{
    byte ExceptionCode = 0;
    int Action = cbActionWrite;
    longword EventCode;
    word RxPDUSizeExpected = DataSingleWritePacketSize;
    if (RxPDUSizeExpected != RxPDUSize)
    {
        Event.EvtCode = evcInvalidADUReceived;
        return TIndicationResult::irSkip;
    }

    EventCode = evcWriteSingleCoil;
    Event.EvtCode = EventCode;

    // Check the function
    if (Shared.Coils.Data == NULL && ws->Coils.OnRequest == NULL) // neither shared data ,nor callback
        return SetMBError(EventCode, RxPDU->Function, errIllegalFunction, TxPDU, TxPDUSize);

    PDataSingleWritePacket Request = PDataSingleWritePacket(RxPDU);
    PDataSingleWritePacket Answer = PDataSingleWritePacket(TxPDU);

    word Address = SwapWord(Request->Address);
    word Value = SwapWord(Request->Value);
    byte bval = bool(Value);

    Event.EvtParam3 = Address;
    Event.EvtParam4 = Value;

    // Check Bounds if we are using internal area, otherwise it must be done inside the user callback
    if (Shared.Coils.Data != NULL && Address > Shared.Coils.Amount - 1)
        return SetMBError(EventCode, RxPDU->Function, errIllegalDataAddress, TxPDU, TxPDUSize);

    Answer->Function = Request->Function;
    Answer->Address = Request->Address;
    Answer->Value = Request->Value;
    TxPDUSize = DataSingleWritePacketSize;

    // Use the Area, if shared
    if (Shared.Coils.Data != NULL)
    {
        // Pack Bits
        Shared.Coils.cs->Enter(); // Lock access
        try
        {
            PMBBits(Shared.Coils.Data)->BitArray[Address] = bval; // we don't know how a Controller could encode it, so if == 0 ->false, otherwise true
        }
        catch (...) {
            ExceptionCode = errSlaveDeviceFailure;
        }
        Shared.Coils.cs->Leave();
    }

    // use the Callback if assigned
    if (ws->Coils.OnRequest != NULL && ExceptionCode == 0)
    {
        ws->Coils.cs->Enter(); // Locks the callback
        try
        {
            ExceptionCode = byte(ws->Coils.OnRequest(ws->Coils.UsrPtr, Action, Address, 1, &bval));
        }
        catch (...) {
            ExceptionCode = errSlaveDeviceFailure;
        }
        ws->Coils.cs->Leave();
    }

    if (ExceptionCode != 0)
        return SetMBError(EventCode, RxPDU->Function, ExceptionCode, TxPDU, TxPDUSize);
    else
        return SetMBOk();
}
//------------------------------------------------------------------------------
TIndicationResult TMBDeviceExecutor::ManageWriteSingleRegister(PMBPDU RxPDU, word RxPDUSize, PMBPDU TxPDU, word& TxPDUSize)
{
    byte ExceptionCode = 0;
    int Action = cbActionWrite;
    longword EventCode;
    word RxPDUSizeExpected = DataSingleWritePacketSize;
    if (RxPDUSizeExpected != RxPDUSize)
    {
        Event.EvtCode = evcInvalidADUReceived;
        return TIndicationResult::irSkip;
    }

    EventCode = evcWriteSingleReg;
    Event.EvtCode = EventCode;

    // Check the function
    if (Shared.HoldingRegisters.Data == NULL && ws->HoldingRegisters.OnRequest == NULL) // neither shared data ,nor callback
        return SetMBError(EventCode, RxPDU->Function, errIllegalFunction, TxPDU, TxPDUSize);

    PDataSingleWritePacket Request = PDataSingleWritePacket(RxPDU);
    PDataSingleWritePacket Answer = PDataSingleWritePacket(TxPDU);

    word Address = SwapWord(Request->Address);
    word Value = SwapWord(Request->Value);

    Event.EvtParam3 = Address;
    Event.EvtParam4 = Value;

    // Check Bounds if we are using internal area, otherwise it must be done inside the user callback
    if (Shared.HoldingRegisters.Data != NULL && Address > Shared.HoldingRegisters.Amount - 1)
        return SetMBError(EventCode, RxPDU->Function, errIllegalDataAddress, TxPDU, TxPDUSize);

    Answer->Function = Request->Function;
    Answer->Address = Request->Address;
    Answer->Value = Request->Value;
    TxPDUSize = DataSingleWritePacketSize;

    // Use the Area, if shared
    if (Shared.HoldingRegisters.Data != NULL)
    {
        // Pack Bits
        Shared.HoldingRegisters.cs->Enter(); // Lock access
        try
        {
            PRegisters16(Shared.HoldingRegisters.Data)->WordArray[Address] = Value;
        }
        catch (...) {
            ExceptionCode = errSlaveDeviceFailure;
        }
        Shared.HoldingRegisters.cs->Leave();
    }

    // use the Callback if assigned
    if (ws->HoldingRegisters.OnRequest != NULL && ExceptionCode == 0)
    {
        ws->HoldingRegisters.cs->Enter(); // Locks the callback
        try
        {
            ExceptionCode = byte(ws->HoldingRegisters.OnRequest(ws->HoldingRegisters.UsrPtr, Action, Address, 1, &Value));
        }
        catch (...) {
            ExceptionCode = errSlaveDeviceFailure;
        }
        ws->HoldingRegisters.cs->Leave(); 
    }

    if (ExceptionCode != 0)
        return SetMBError(EventCode, RxPDU->Function, ExceptionCode, TxPDU, TxPDUSize);
    else
        return SetMBOk();
}
//------------------------------------------------------------------------------
TIndicationResult TMBDeviceExecutor::ManageReadCoils(PMBPDU RxPDU, word RxPDUSize, PMBPDU TxPDU, word& TxPDUSize)
{
    byte ExceptionCode = 0;
    word RxPDUSizeExpected = DataReadReqHeaderSize;
    longword EventCode;
    // check the telegram
    if (RxPDUSizeExpected != RxPDUSize)
    {
        Event.EvtCode = evcInvalidADUReceived;
        return TIndicationResult::irSkip;
    }
    int Action = cbActionRead;
    EventCode = evcReadCoils;
    Event.EvtCode = EventCode;

    // Check the function
    if (Shared.Coils.Data == NULL && ws->Coils.OnRequest == NULL) // neither shared data ,nor callback
        return SetMBError(EventCode, RxPDU->Function, errIllegalFunction, TxPDU, TxPDUSize);

    PDataReadReqHeader Request = PDataReadReqHeader(RxPDU);
    PDataReadResPacket Answer = PDataReadResPacket(TxPDU);

    word Address = SwapWord(Request->Address);
    word Amount = SwapWord(Request->Amount);

    // Set Event Info
    Event.EvtParam3 = Address;
    Event.EvtParam4 = Amount;

    // check the Amount
    if (Amount == 0 || Amount > MaxReadMultipleBits)
        return SetMBError(EventCode, RxPDU->Function, errIllegalDataValue, TxPDU, TxPDUSize);

    // Check Bounds if we are using internal area, otherwise it must be done inside the user callback
    int EndAddress = Shared.Coils.Amount - 1;
    if (Shared.Coils.Data != NULL && (Address > EndAddress || EndAddress - Address < Amount))
        return SetMBError(EventCode, RxPDU->Function, errIllegalDataAddress, TxPDU, TxPDUSize);

    Answer->Function = Request->Function;
    Answer->ByteCount = byte(RoundToNextByte(Amount));
    TxPDUSize = Answer->ByteCount + 2; // Adding Function and the ByteCount itself
    
    // use the Callback if assigned
    if (ws->Coils.OnRequest != NULL)
    {
        memset(&Buffer, 0, sizeof(Buffer));
        ws->Coils.cs->Enter(); // Locks the callback
        try
        {
            ExceptionCode = byte(ws->Coils.OnRequest(ws->Coils.UsrPtr, Action, Address, Amount, &Buffer));
        }
        catch (...) {
            ExceptionCode = errSlaveDeviceFailure;
        }
        ws->Coils.cs->Leave(); 

        if (ExceptionCode == 0)
            PackBits(Answer->Data, &Buffer[0], Amount);
    }

    // Use the Area, if shared
    if (Shared.Coils.Data != NULL && ExceptionCode == 0)
    {
        // Calc Source start point and Rounded Amount
        pbyte Start = pbyte(Shared.Coils.Data) + uintptr_t(Address);

        // Pack Bits
        Shared.Coils.cs->Enter();
        try
        {
            PackBits(Answer->Data, Start, Amount);
        }
        catch (...) {
            ExceptionCode = errSlaveDeviceFailure;
        }
        Shared.Coils.cs->Leave();
    }

    if (ExceptionCode != 0)
        return SetMBError(EventCode, RxPDU->Function, ExceptionCode, TxPDU, TxPDUSize);
    else
        return SetMBOk();
}
//------------------------------------------------------------------------------
TIndicationResult TMBDeviceExecutor::ManageReadDiscreteInputs(PMBPDU RxPDU, word RxPDUSize, PMBPDU TxPDU, word& TxPDUSize)
{
    byte ExceptionCode = 0;
    word RxPDUSizeExpected = DataReadReqHeaderSize;
    longword EventCode;
    // check the telegram
    if (RxPDUSizeExpected != RxPDUSize)
    {
        Event.EvtCode = evcInvalidADUReceived;
        return TIndicationResult::irSkip;
    }
    EventCode = evcReadDiscrInputs;
    Event.EvtCode = EventCode;

    // Check the function
    if (Shared.DiscreteInputs.Data == NULL && ws->DiscreteInputs.OnRequest == NULL) // neither shared data ,nor callback
        return SetMBError(EventCode, RxPDU->Function, errIllegalFunction, TxPDU, TxPDUSize);

    PDataReadReqHeader Request = PDataReadReqHeader(RxPDU);
    PDataReadResPacket Answer = PDataReadResPacket(TxPDU);

    word Address = SwapWord(Request->Address);
    word Amount = SwapWord(Request->Amount);

    // Set Event Info
    Event.EvtParam3 = Address;
    Event.EvtParam4 = Amount;

    // check the Amount
    if (Amount == 0 || Amount > MaxReadMultipleBits)
        return SetMBError(EventCode, RxPDU->Function, errIllegalDataValue, TxPDU, TxPDUSize);

    // Check Bounds if we are using internal area, otherwise it must be done inside the user callback
    int EndAddress = Shared.DiscreteInputs.Amount - 1;
    if (Shared.DiscreteInputs.Data != NULL && (Address > EndAddress || EndAddress - Address < Amount))
        return SetMBError(EventCode, RxPDU->Function, errIllegalDataAddress, TxPDU, TxPDUSize);

    Answer->Function = Request->Function;
    Answer->ByteCount = byte(RoundToNextByte(Amount));
    TxPDUSize = Answer->ByteCount + 2; // Adding Function and the ByteCount itself

    // use the Callback if assigned
    if (ws->DiscreteInputs.OnRequest != NULL)
    {
        ws->DiscreteInputs.cs->Enter(); // Locks the callback
        try
        {
            memset(&Buffer, 0, sizeof(Buffer));
            ExceptionCode = byte(ws->DiscreteInputs.OnRequest(ws->DiscreteInputs.UsrPtr, Address, Amount, &Buffer));
        }
        catch (...) {
            ExceptionCode = errSlaveDeviceFailure;
        }
        ws->DiscreteInputs.cs->Leave();

        if (ExceptionCode == 0)
            PackBits(Answer->Data, &Buffer[0], Amount);
    }

    // Use the Area, if shared
    if (Shared.DiscreteInputs.Data != NULL && ExceptionCode == 0)
    {
        // Calc Source start point and Rounded Amount
        pbyte Start = pbyte(Shared.DiscreteInputs.Data) + uintptr_t(Address);

        // Pack Bits
        Shared.DiscreteInputs.cs->Enter();
        try
        {
            PackBits(Answer->Data, Start, Amount);
        }
        catch (...) {
            ExceptionCode = errSlaveDeviceFailure;
        }
        Shared.DiscreteInputs.cs->Leave();
    }

    if (ExceptionCode != 0)
        return SetMBError(EventCode, RxPDU->Function, ExceptionCode, TxPDU, TxPDUSize);
    else
        return SetMBOk();
}
//------------------------------------------------------------------------------
TIndicationResult TMBDeviceExecutor::ManageReadInputRegisters(PMBPDU RxPDU, word RxPDUSize, PMBPDU TxPDU, word& TxPDUSize)
{
    byte ExceptionCode = 0;
    word RxPDUSizeExpected = DataReadReqHeaderSize;
    longword EventCode;
    // check the telegram
    if (RxPDUSizeExpected != RxPDUSize)
    {
        Event.EvtCode = evcInvalidADUReceived;
        return TIndicationResult::irSkip;
    }

    EventCode = evcReadInputRegs;
    Event.EvtCode = EventCode;

    // Check the function
    if (Shared.InputRegisters.Data == NULL && ws->InputRegisters.OnRequest == NULL) // neither shared data ,nor callback
        return SetMBError(EventCode, RxPDU->Function, errIllegalFunction, TxPDU, TxPDUSize);

    PDataReadReqHeader Request = PDataReadReqHeader(RxPDU);
    PDataReadResPacket Answer = PDataReadResPacket(TxPDU);

    word Address = SwapWord(Request->Address);
    word Amount = SwapWord(Request->Amount);

    // Set Event Info
    Event.EvtParam3 = Address;
    Event.EvtParam4 = Amount;

    // check the Amount
    if (Amount == 0 || Amount > MaxReadMultipleRegisters)
        return SetMBError(EventCode, RxPDU->Function, errIllegalDataValue, TxPDU, TxPDUSize);

    // Check Bounds if we are using internal area, otherwise it must be done inside the user callback
    int EndAddress = Shared.InputRegisters.Amount - 1;
    if (Shared.InputRegisters.Data != NULL && (Address > EndAddress || EndAddress - Address < Amount))
        return SetMBError(EventCode, RxPDU->Function, errIllegalDataAddress, TxPDU, TxPDUSize);

    Answer->Function = Request->Function;
    Answer->ByteCount = byte(Amount * Reg16Size);

    TxPDUSize = Answer->ByteCount + 2; // Adding Function and the ByteCount itself

    // use the Callback if assigned
    if (ws->InputRegisters.OnRequest != NULL)
    {
        ws->InputRegisters.cs->Enter(); // Locks the callback
        try
        {
            memset(&Buffer, 0, sizeof(Buffer));
            ExceptionCode = byte(ws->InputRegisters.OnRequest(ws->InputRegisters.UsrPtr, Address, Amount, &Buffer));
        }
        catch (...) {
            ExceptionCode = errSlaveDeviceFailure;
        }
        ws->InputRegisters.cs->Leave();

        if (ExceptionCode == 0)
            CopyAndSwapReg16Pack(PRegisters16(Answer->Data), PRegisters16(&Buffer), Amount);
    }

    // Use the Area, if shared
    if (Shared.InputRegisters.Data != NULL && ExceptionCode == 0)
    {
        // Calc Source start point 
        pbyte Start = pbyte(Shared.InputRegisters.Data) + uintptr_t(Address * uintptr_t(Reg16Size));

        // Swap regs
        Shared.InputRegisters.cs->Enter();
        try
        {
            CopyAndSwapReg16Pack(PRegisters16(Answer->Data), PRegisters16(Start), Amount);
        }
        catch (...) {
            ExceptionCode = errSlaveDeviceFailure;
        }
        Shared.InputRegisters.cs->Leave();
    }

    if (ExceptionCode != 0)
        return SetMBError(EventCode, RxPDU->Function, ExceptionCode, TxPDU, TxPDUSize);
    else
        return SetMBOk();
}
//------------------------------------------------------------------------------
TIndicationResult TMBDeviceExecutor::ManageReadHoldingRegisters(PMBPDU RxPDU, word RxPDUSize, PMBPDU TxPDU, word& TxPDUSize)
{
    byte ExceptionCode = 0;
    word RxPDUSizeExpected = DataReadReqHeaderSize;
    longword EventCode;
    // check the telegram
    if (RxPDUSizeExpected != RxPDUSize)
    {
        Event.EvtCode = evcInvalidADUReceived;
        return TIndicationResult::irSkip;
    }

    int Action = cbActionRead;
    EventCode = evcReadHoldingRegs;
    Event.EvtCode = EventCode;

    // Check the function
    if (Shared.HoldingRegisters.Data == NULL && ws->HoldingRegisters.OnRequest == NULL) // neither shared data ,nor callback
        return SetMBError(EventCode, RxPDU->Function, errIllegalFunction, TxPDU, TxPDUSize);

    PDataReadReqHeader Request = PDataReadReqHeader(RxPDU);
    PDataReadResPacket Answer = PDataReadResPacket(TxPDU);

    word Address = SwapWord(Request->Address);
    word Amount = SwapWord(Request->Amount);

    // Set Event Info
    Event.EvtParam3 = Address;
    Event.EvtParam4 = Amount;

    // check the Amount
    if (Amount == 0 || Amount > MaxReadMultipleRegisters)
        return SetMBError(EventCode, RxPDU->Function, errIllegalDataValue, TxPDU, TxPDUSize);

    // Check Bounds if we are using internal area, otherwise it must be done inside the user callback
    int EndAddress = Shared.HoldingRegisters.Amount - 1;
    if (Shared.HoldingRegisters.Data != NULL && (Address > EndAddress || EndAddress - Address < Amount))
        return SetMBError(EventCode, RxPDU->Function, errIllegalDataAddress, TxPDU, TxPDUSize);

    Answer->Function = Request->Function;
    Answer->ByteCount = byte(Amount * Reg16Size);

    TxPDUSize = Answer->ByteCount + 2; // Adding Function and the ByteCount itself

    // use the Callback if assigned
    if (ws->HoldingRegisters.OnRequest != NULL)
    {
        ws->HoldingRegisters.cs->Enter(); // Locks the callback
        try
        {
            memset(&Buffer, 0, sizeof(Buffer));
            ExceptionCode = byte(ws->HoldingRegisters.OnRequest(ws->HoldingRegisters.UsrPtr, Action, Address, Amount, &Buffer));
        }
        catch (...) {
            ExceptionCode = errSlaveDeviceFailure;
        }
        ws->HoldingRegisters.cs->Leave();

        if (ExceptionCode == 0)
            CopyAndSwapReg16Pack(PRegisters16(Answer->Data), PRegisters16(&Buffer), Amount);
    }

    // Use the Area, if shared
    if (Shared.HoldingRegisters.Data != NULL && ExceptionCode == 0)
    {
        // Calc Source start point 
        pbyte Start = pbyte(Shared.HoldingRegisters.Data) + uintptr_t(Address * uintptr_t(Reg16Size));

        // Swap regs
        Shared.HoldingRegisters.cs->Enter();
        try
        {
            CopyAndSwapReg16Pack(PRegisters16(Answer->Data), PRegisters16(Start), Amount);
        }
        catch (...) {
            ExceptionCode = errSlaveDeviceFailure;
        }
        Shared.HoldingRegisters.cs->Leave();
    }

    if (ExceptionCode != 0)
        return SetMBError(EventCode, RxPDU->Function, ExceptionCode, TxPDU, TxPDUSize);
    else
        return SetMBOk();
}
//------------------------------------------------------------------------------
TIndicationResult TMBDeviceExecutor::ManageReadWriteMultipleRegistersRequest(PMBPDU RxPDU, word RxPDUSize, PMBPDU TxPDU, word& TxPDUSize)
{
    byte ExceptionCode = 0;
    Event.EvtCode = evcReadWriteMultiRegs;

    // Check the function : 
    if (Shared.HoldingRegisters.Data == NULL && ws->ReadWriteRegisters.OnRequest == NULL) // neither shared data ,nor callback
        return SetMBError(Event.EvtCode, RxPDU->Function, errIllegalFunction, TxPDU, TxPDUSize);

    PDataReadWriteMultiRegPacket Request = PDataReadWriteMultiRegPacket(RxPDU);
    PDataReadResPacket Answer = PDataReadResPacket(TxPDU);

    word RDAddress = SwapWord(Request->Header.RDAddress);
    word RDAmount = SwapWord(Request->Header.RDAmount);
    word WRAddress = SwapWord(Request->Header.WRAddress);
    word WRAmount = SwapWord(Request->Header.WRAmount);
    byte ByteCount = Request->Header.ByteCount;

    Event.EvtParam1 = RDAddress;
    Event.EvtParam2 = RDAmount;
    Event.EvtParam3 = WRAddress;
    Event.EvtParam4 = WRAmount;

    int RxPDUSizeExpected = ByteCount + TDataReadWriteMultiRegHeaderSize;
    if (RxPDUSize != RxPDUSizeExpected)
    {
        Event.EvtCode = evcInvalidADUReceived;
        return TIndicationResult::irSkip;
    }

    // Check the amount
    if (RDAmount == 0 || RDAmount > MaxReadMultipleRegisters || WRAmount == 0 || WRAmount > MaxWriteMultiReg_fun17)
        return SetMBError(Event.EvtCode, RxPDU->Function, errIllegalDataValue, TxPDU, TxPDUSize);
  
    // if we use the shared area, we can check the bounds too
    if (Shared.HoldingRegisters.Data != NULL)
    {
        int EndAddress = Shared.HoldingRegisters.Amount - 1;
        if (RDAddress > EndAddress || EndAddress - RDAddress < RDAmount || WRAddress > EndAddress || EndAddress - WRAddress < WRAmount)
            return SetMBError(Event.EvtCode, RxPDU->Function, errIllegalDataAddress, TxPDU, TxPDUSize);
    }

    Answer->Function = Request->Header.Function;
    Answer->ByteCount = byte(RDAmount * Reg16Size);
    TxPDUSize = Answer->ByteCount + 2; // Adding Function and the ByteCount itself

    // Use the shared Area
    if (Shared.HoldingRegisters.Data != NULL && ExceptionCode == 0)
    {
        pbyte RDStart = pbyte(Shared.HoldingRegisters.Data) + uintptr_t(RDAddress * uintptr_t(Reg16Size));
        pbyte WRStart = pbyte(Shared.HoldingRegisters.Data) + uintptr_t(WRAddress * uintptr_t(Reg16Size));

        Shared.HoldingRegisters.cs->Enter(); // Lock access
        try
        {
            // First write then read, in accord to the specification
            CopyAndSwapReg16Pack(PRegisters16(WRStart), PRegisters16(Request->Data), WRAmount);
            CopyAndSwapReg16Pack(PRegisters16(Answer->Data), PRegisters16(RDStart), RDAmount);
        }
        catch (...) {
            ExceptionCode = errSlaveDeviceFailure;
        }
        Shared.HoldingRegisters.cs->Leave();
    }

    // Use the callback if assigned
    if (ws->ReadWriteRegisters.OnRequest != NULL)
    {
        // Prepares the registers to write
        SwapReg16Pack(PRegisters16(Request->Data), WRAmount);
        ws->ReadWriteRegisters.cs->Enter(); // Locks the callback
        try
        {
            ExceptionCode = byte(ws->ReadWriteRegisters.OnRequest(ws->ReadWriteRegisters.UsrPtr, RDAddress, RDAmount, Answer->Data, WRAddress, WRAmount, Request->Data));
        }
        catch (...) {
            ExceptionCode = errSlaveDeviceFailure;
        }
        ws->ReadWriteRegisters.cs->Leave();

        if (ExceptionCode == 0)
            // Adjusts the registers read
            SwapReg16Pack(PRegisters16(Answer->Data), RDAmount);
    }

    if (ExceptionCode == 0)
        return TIndicationResult::irSendAnswer;
    else
        return SetMBOk();
}
//------------------------------------------------------------------------------
TIndicationResult TMBDeviceExecutor::ManageMaskRegisterRequest(PMBPDU RxPDU, word RxPDUSize, PMBPDU TxPDU, word& TxPDUSize)
{
    byte ExceptionCode = 0;
    Event.EvtCode = evcMaskWriteReg;

    // Check the function
    if (Shared.HoldingRegisters.Data == NULL && ws->HoldingRegisters.OnRequest == NULL) // neither shared data ,nor callback
        return SetMBError(Event.EvtCode, RxPDU->Function, errIllegalFunction, TxPDU, TxPDUSize);

    // Check the telegram
    if (RxPDUSize != TMaskRegisterPacketSize)
        return SetMBError(Event.EvtCode, RxPDU->Function, errIllegalDataValue, TxPDU, TxPDUSize);

    PMaskRegisterPacket Request = PMaskRegisterPacket(RxPDU);

    word Address = SwapWord(Request->Address);
    word AND_Mask = SwapWord(Request->AND_Mask);
    word OR_Mask = SwapWord(Request->OR_Mask);

    memcpy(TxPDU, RxPDU, TMaskRegisterPacketSize);
    TxPDUSize = RxPDUSize;

    Event.EvtParam1 = Address;
    Event.EvtParam2 = AND_Mask;
    Event.EvtParam3 = OR_Mask;

    // Check bounds
    if (Shared.HoldingRegisters.Data != NULL && Address > Shared.HoldingRegisters.Amount - 1)
        return SetMBError(Event.EvtCode, RxPDU->Function, errIllegalDataAddress, TxPDU, TxPDUSize);

    // Use the Area if assigned
    if (Shared.HoldingRegisters.Data != NULL)
    {
        Shared.HoldingRegisters.cs->Enter();
        try
        {
            word Value = PRegisters16(Shared.HoldingRegisters.Data)->WordArray[Address];
            PRegisters16(Shared.HoldingRegisters.Data)->WordArray[Address] = (Value & AND_Mask) | (OR_Mask & ~AND_Mask);
        }
        catch (...) {
            ExceptionCode = errSlaveDeviceFailure;
        }
        Shared.HoldingRegisters.cs->Leave();
    }
    
    // Use the callback if assigned
    if (ws->HoldingRegisters.OnRequest != NULL && ExceptionCode == 0)
    {
        ws->MaskRegister.cs->Enter(); // Locks the callback       
        try
        {
            ExceptionCode = byte(ws->MaskRegister.OnRequest(ws->MaskRegister.UsrPtr, Address, AND_Mask, OR_Mask));
        }
        catch (...) {
            ExceptionCode = errSlaveDeviceFailure;
        }
        ws->MaskRegister.cs->Leave();
    }

    if (ExceptionCode == 0)
        return TIndicationResult::irSendAnswer;
    else
        return SetMBOk();
}
//------------------------------------------------------------------------------
TIndicationResult TMBDeviceExecutor::ManageReadFileRecordRequest(PMBPDU RxPDU, word RxPDUSize, PMBPDU TxPDU, word& TxPDUSize)
{
    /*

     RxPDU (received from a client)

        byte Function;     // 0x14
        byte Length;       // The "byte" sum of remaining fields, always 7 because we read only 1 File record
        byte RefType;      // should be = 6 but we don't check it, the user will decide if a number !=6 is good enough
        word FileNumber;   // file num identifier
        word RecNumber;    // rec num identifier
        word RecLength;    // Registers amount, i.e. how many regs we want to read (*)

     TxPDU (transmitted to the client)

        byte Function;     // 0x14
        byte DataLength;   // The "byte" sum of remaining fields
        byte RecLen;       // The "byte" sum of remaining fields
        byte RefType;      // Reference type echo
        byte Data[xx];     // registers read (*)


        (*) The whole Modbus PDU size cannot exceed 253 bytes, so the maximum number of registers that we can read
            is (253 - 4) / 2 = 124.  Where 4 is the size of the first TxPDU four fields

    */
    byte ExceptionCode = 0;
    Event.EvtCode = evcReadFileRecord;

    // Check the Callback assignment
    if (ws->FileRecord.OnRequest == NULL)
        return SetMBError(Event.EvtCode, RxPDU->Function, errIllegalFunction, TxPDU, TxPDUSize);

    PFileRecordHeader Request = PFileRecordHeader(RxPDU);
    PReadFileRecAnswer Answer = PReadFileRecAnswer(TxPDU);

    word RefType = Request->RefType;
    word FileNumber = SwapWord(Request->FileNumber);
    word RecNumber = SwapWord(Request->RecNumber);
    word RegsAmount = SwapWord(Request->RecLength);

    Event.EvtParam1 = RefType;
    Event.EvtParam2 = FileNumber;
    Event.EvtParam3 = RecNumber;
    Event.EvtParam4 = RegsAmount;

    int RxPDUSizeExpected = Request->Length + 2;
    if (RxPDUSizeExpected != RxPDUSize)
    {
        Event.EvtCode = evcInvalidADUReceived;
        return TIndicationResult::irSkip;
    }

    if (RegsAmount == 0 || RegsAmount > MaxRDFileRecRegsAmount)
        return SetMBError(Event.EvtCode, RxPDU->Function, errIllegalDataValue, TxPDU, TxPDUSize);

    // Call the Callback 
    ws->FileRecord.cs->Enter();
    try
    {
        ExceptionCode = byte(ws->FileRecord.OnRequest(ws->FileRecord.UsrPtr, cbActionRead, RefType, FileNumber, RecNumber, RegsAmount, &Answer->Data));
    }
    catch (...) {
        ExceptionCode = errSlaveDeviceFailure;
    }
    ws->FileRecord.cs->Leave();

    if (ExceptionCode == 0)
    {
        SwapReg16Pack(PRegisters16(&Answer->Data), RegsAmount);
        Answer->Function = Request->Function;
        Answer->RefType = Request->RefType;
        Answer->RecLen = byte(RegsAmount * Reg16Size + 1); // adding RefType and Reclen itself
        Answer->DataLength = Answer->RecLen + 1;
        TxPDUSize = Answer->DataLength + 2;
    }

    if (ExceptionCode != 0)
        return SetMBError(Event.EvtCode, RxPDU->Function, ExceptionCode, TxPDU, TxPDUSize);
    else
        return SetMBOk();
}
//------------------------------------------------------------------------------
TIndicationResult TMBDeviceExecutor::ManageWriteFileRecordRequest(PMBPDU RxPDU, word RxPDUSize, PMBPDU TxPDU, word& TxPDUSize)
{
    /*

        RxPDU (received from a client)

        byte Function;     // 0x14
        byte Length;       // The "byte" sum of remaining fields
        byte RefType;      // should be = 6 but we don't check it, the user will decide if a number !=6 is good enough
        word FileNumber;   // file num identifier
        word RecNumber;    // rec num identifier
        word RecLength;    // Registers amount, i.e. how many regs we want to read (*)
        byte Data[xx]

        TxPDU (transmitted to the client) is the echo of RxPDU


        (*) The whole Modbus PDU size cannot exceed 253 bytes, so the maximum number of registers that we can write
            is (253 - 9) / 2 = 122.  Where 9 is the size of the first six fields

    */
    byte ExceptionCode = 0;
    Event.EvtCode = evcWriteFileRecord;

    // Check the Callback assignment
    if (ws->FileRecord.OnRequest == NULL)
        return SetMBError(Event.EvtCode, RxPDU->Function, errIllegalFunction, TxPDU, TxPDUSize);

    PWriteFileRecPacket Request = PWriteFileRecPacket(RxPDU);

    word RefType = Request->Header.RefType;
    word FileNumber = SwapWord(Request->Header.FileNumber);
    word RecNumber = SwapWord(Request->Header.RecNumber);
    word RegsAmount = SwapWord(Request->Header.RecLength);

    Event.EvtParam1 = RefType;
    Event.EvtParam2 = FileNumber;
    Event.EvtParam3 = RecNumber;
    Event.EvtParam4 = RegsAmount;

    int RxPDUSizeExpected = Request->Header.Length + 2;
    if (RxPDUSizeExpected != RxPDUSize)
    {
        Event.EvtCode = evcInvalidADUReceived;
        return TIndicationResult::irSkip;
    }

    if (RegsAmount == 0 || RegsAmount > MaxWRFileRecRegsAmount)
        return SetMBError(Event.EvtCode, RxPDU->Function, errIllegalDataValue, TxPDU, TxPDUSize);

    // The Answer is the Echo of the Request
    memcpy(TxPDU, RxPDU, RxPDUSize); // Prepare the answer since we are going to modify the Request->Data
    TxPDUSize = RxPDUSize;

    SwapReg16Pack(PRegisters16(&Request->Data), RegsAmount);

    ws->FileRecord.cs->Enter();
    try
    {
        ExceptionCode = byte(ws->FileRecord.OnRequest(ws->FileRecord.UsrPtr, cbActionWrite, RefType, FileNumber, RecNumber, RegsAmount, &Request->Data));
    }
    catch (...) {
        ExceptionCode = errSlaveDeviceFailure;
    }
    ws->FileRecord.cs->Leave();

    if (ExceptionCode != 0)
        return SetMBError(Event.EvtCode, RxPDU->Function, ExceptionCode, TxPDU, TxPDUSize);
    else
        return SetMBOk();
}
//------------------------------------------------------------------------------
TIndicationResult TMBDeviceExecutor::ManageReadFIFOQueue(PMBPDU RxPDU, word RxPDUSize, PMBPDU TxPDU, word& TxPDUSize)
{
    byte ExceptionCode = 0;
    Event.EvtCode = evcReadFifoQueue;

    // Check the Callback assignment
    if (ws->FIFORequest.OnRequest == NULL)
        return SetMBError(Event.EvtCode, RxPDU->Function, errIllegalFunction, TxPDU, TxPDUSize);

    PReadFifoReqHeader Request = PReadFifoReqHeader(RxPDU);
    PReadFifoReqAnswer Answer = PReadFifoReqAnswer(TxPDU);

    word FifoCount = 0;
    word PtrAddress = SwapWord(Request->PtrAddress);
    Event.EvtParam1 = PtrAddress;

    ws->FIFORequest.cs->Enter();
    try
    {
        ExceptionCode = byte(ws->FIFORequest.OnRequest(ws->FIFORequest.UsrPtr, PtrAddress, Answer->Data, FifoCount));
    }
    catch (...) {
        ExceptionCode = errSlaveDeviceFailure;
    }
    ws->FIFORequest.cs->Leave();

    if (ExceptionCode == 0)
    {
        if (FifoCount > MaxReadFIFOItems)
            FifoCount = MaxReadFIFOItems;
        // Prepares the Answer
        Answer->Function = Request->Function;
        Answer->ByteCount = SwapWord(word(FifoCount)*Reg16Size + 2); 
        Answer->FifoCount = SwapWord(word(FifoCount));
        TxPDUSize = FifoCount * Reg16Size + 5;
        // Reverse the Answer
        SwapReg16Pack(PRegisters16(Answer->Data), FifoCount);
    }

    if (ExceptionCode != 0)
        return SetMBError(Event.EvtCode, RxPDU->Function, ExceptionCode, TxPDU, TxPDUSize);
    else
        return SetMBOk();
}
//------------------------------------------------------------------------------
TIndicationResult TMBDeviceExecutor::ManageEncapsulatedIT(PMBPDU RxPDU, word RxPDUSize, PMBPDU TxPDU, word& TxPDUSize)
{
    byte ExceptionCode = 0;
    Event.EvtCode = evcEncIntTransport;
    
    // Check the Callback assignment
    if (ws->MEIRequestCbk.OnRequest == NULL)
        return SetMBError(Event.EvtCode, RxPDU->Function, errIllegalFunction, TxPDU, TxPDUSize);

    PMEIRequest Request = PMEIRequest(RxPDU);
    PMEIAnswer Answer = PMEIAnswer(TxPDU); 
    Event.EvtParam1 = Request->MEI_Type;

    word ReqDataSize = RxPDUSize - 2; // 2 = Function + MEI type
    word ResDataSize = 0;

    ws->MEIRequestCbk.cs->Enter();
    try
    {
        ExceptionCode = byte(ws->MEIRequestCbk.OnRequest(ws->MEIRequestCbk.UsrPtr, Request->MEI_Type, Request->Data, ReqDataSize, Answer->Data, ResDataSize));
    }
    catch (...) {
        ExceptionCode = errSlaveDeviceFailure;
    }
    ws->MEIRequestCbk.cs->Leave();

    if (ExceptionCode == 0)
    {
        if (ResDataSize > MaxMEIDataSize)
            ResDataSize = MaxMEIDataSize;
        Answer->Function = Request->Function;
        Answer->MEI_Type = Request->MEI_Type;
        TxPDUSize = ResDataSize + 2; // Adding Function and MEI_Type
    }

    if (ExceptionCode != 0)
        return SetMBError(Event.EvtCode, RxPDU->Function, ExceptionCode, TxPDU, TxPDUSize);
    else
        return SetMBOk();
}
//------------------------------------------------------------------------------
TIndicationResult TMBDeviceExecutor::ManageReadExceptionStatus(PMBPDU RxPDU, word RxPDUSize, PMBPDU TxPDU, word& TxPDUSize)
{
    byte ExceptionCode = 0;
    Event.EvtCode = evcReadExcpStatus;

    // Check the Callback assignment
    if (ws->ExceptionStatus.OnRequest == NULL)
        return SetMBError(Event.EvtCode, RxPDU->Function, errIllegalFunction, TxPDU, TxPDUSize);

    PReadExceptionStatusRequest Request = PReadExceptionStatusRequest(RxPDU);
    PReadExceptionStatusAnswer Answer = PReadExceptionStatusAnswer(TxPDU);

    Answer->Data = 0;
    ws->ExceptionStatus.cs->Enter();
    try
    {
        ExceptionCode = byte(ws->ExceptionStatus.OnRequest(ws->UsrFunction.UsrPtr, Answer->Data));
    }
    catch (...) {
        ExceptionCode = errSlaveDeviceFailure;
    }
    ws->ExceptionStatus.cs->Leave();

    if (ExceptionCode == 0)
    {
        TxPDUSize = 2; // (1)Function + (1)Status
        Answer->Function = Request->Function;
        Event.EvtParam1 = Answer->Data;
        return SetMBOk();
    }
    else
        return SetMBError(Event.EvtCode, RxPDU->Function, ExceptionCode, TxPDU, TxPDUSize);
}
//------------------------------------------------------------------------------
TIndicationResult TMBDeviceExecutor::ManageDiagnostics(PMBPDU RxPDU, word RxPDUSize, PMBPDU TxPDU, word& TxPDUSize)
{
    byte ExceptionCode = 0;
    Event.EvtCode = evcDiagnostics;
    PDiagnosticPDU Request = PDiagnosticPDU(RxPDU);
    PDiagnosticPDU Answer = PDiagnosticPDU(TxPDU);
    if (ws->Diagnostics.OnRequest == NULL)
        return SetMBError(Event.EvtCode, RxPDU->Function, errIllegalFunction, TxPDU, TxPDUSize);

    // The cases known from the modbus specifications foresee only 1 item, but it is possible that, 
    // due to vendor implementations, there may be more items. Wee need to know how many they are
    word ItemsSent = (RxPDUSize - 3) / Reg16Size; // 3 = (1)Function + (2)SubFunction
    word ItemsRecvd = ItemsSent; // by default

    // Reverse the Request
    SwapReg16Pack(PRegisters16(Request->Data), ItemsSent);

    word SubFunction = SwapWord(Request->SubFunction);
    
    // Check for Loopback request 
    if (SubFunction==0)
        memcpy(TxPDU, RxPDU, RxPDUSize);
    
    Event.EvtParam1 = SubFunction;

    ws->Diagnostics.cs->Enter();
    try
    {
        ExceptionCode = byte(ws->Diagnostics.OnRequest(ws->UsrFunction.UsrPtr, SubFunction, &Request->Data, &Answer->Data, ItemsSent, ItemsRecvd));
    }
    catch (...) {
        ExceptionCode = errSlaveDeviceFailure;
    }
    ws->Diagnostics.cs->Leave();

    if (ExceptionCode == 0) 
    {
        Answer->Function = Request->Function;
        Answer->SubFunction = Request->SubFunction;
        // Reverse the Answer
        SwapReg16Pack(PRegisters16(Answer->Data), ItemsRecvd);
        TxPDUSize = ItemsRecvd * Reg16Size + 3;
    }

    if (ExceptionCode != 0)
        return SetMBError(Event.EvtCode, RxPDU->Function, ExceptionCode, TxPDU, TxPDUSize);
    else
        return SetMBOk();
}
//------------------------------------------------------------------------------
TIndicationResult TMBDeviceExecutor::ManageGetCommEventCounter(PMBPDU RxPDU, word RxPDUSize, PMBPDU TxPDU, word& TxPDUSize)
{
    byte ExceptionCode = 0;
    Event.EvtCode = evcGetCommEvtCnt;
    PGetEventCounterAnswer Answer = PGetEventCounterAnswer(TxPDU);
    word Status = 0;
    word EventCount = EventsCount;

    if (ws->CommEventCounter.OnRequest == NULL)
        return SetMBError(Event.EvtCode, RxPDU->Function, errIllegalFunction, TxPDU, TxPDUSize);

    ws->CommEventCounter.cs->Enter();
    try
    {
        ExceptionCode = byte(ws->CommEventCounter.OnRequest(ws->UsrFunction.UsrPtr, Status, EventCount));
    }
    catch (...) {
        ExceptionCode = errSlaveDeviceFailure;
    }
    ws->CommEventCounter.cs->Leave();

    if (ExceptionCode == 0)
    {
        Answer->Function = RxPDU->Function;
        Answer->Status = SwapWord(Status);
        Answer->EventCount = SwapWord(EventCount);
        TxPDUSize = sizeof(TGetEventCounterAnswer);
    }

    if (ExceptionCode != 0)
        return SetMBError(Event.EvtCode, RxPDU->Function, ExceptionCode, TxPDU, TxPDUSize);
    else
        return SetMBOk();
}
//------------------------------------------------------------------------------
TIndicationResult TMBDeviceExecutor::ManageGetCommEventLog(PMBPDU RxPDU, word RxPDUSize, PMBPDU TxPDU, word& TxPDUSize)
{
    byte ExceptionCode = 0;
    Event.EvtCode = evcGetCommEvtLog;
    PGetCommEventLogAnswer Answer = PGetCommEventLogAnswer(TxPDU);
    if (ws->CommEventLog.OnRequest == NULL)
        return SetMBError(Event.EvtCode, RxPDU->Function, errIllegalFunction, TxPDU, TxPDUSize);

    word Status = 0;
    word EventCount = EventsCount;
    word MessageCount = 0;
    word EventsAmount = 0;

    ws->CommEventLog.cs->Enter();
    try
    {
        ExceptionCode = byte(ws->CommEventLog.OnRequest(ws->UsrFunction.UsrPtr, Status, EventCount, MessageCount, Answer->Events, EventsAmount));
    }
    catch (...) {
        ExceptionCode = errSlaveDeviceFailure;
    }
    ws->CommEventLog.cs->Leave();

    if (ExceptionCode == 0)
    {
        if (EventsAmount > MaxCommLogEvents)
            EventsAmount = MaxCommLogEvents;
        Event.EvtParam1 = EventsAmount;
        // Prepares the Answer
        Answer->Function = RxPDU->Function;
        Answer->ByteCount = byte(EventsAmount + 6); // 6 = (2)Status + (2)EventCount + (2)MessageCount
        Answer->Status = SwapWord(Status);
        Answer->EventCount = SwapWord(EventCount);
        Answer->MessageCount = SwapWord(MessageCount);
        TxPDUSize = EventsAmount + 8; // Adding (1)Function + (1)ByteCount
    }

    if (ExceptionCode != 0)
        return SetMBError(Event.EvtCode, RxPDU->Function, ExceptionCode, TxPDU, TxPDUSize);
    else
        return SetMBOk();
}
//------------------------------------------------------------------------------
TIndicationResult TMBDeviceExecutor::ManageReportServerID(PMBPDU RxPDU, word RxPDUSize, PMBPDU TxPDU, word& TxPDUSize)
{
    byte ExceptionCode = 0;
    Event.EvtCode = evcReportServerID;
    PServerIDAnswer Answer = PServerIDAnswer(TxPDU);

    if (ws->ReportServerID.OnRequest == NULL)
        return SetMBError(Event.EvtCode, RxPDU->Function, errIllegalFunction, TxPDU, TxPDUSize);

    word DataSize = 0;
    ws->ReportServerID.cs->Enter();
    try
    {
        ExceptionCode = byte(ws->ReportServerID.OnRequest(ws->ReportServerID.UsrPtr, Answer->Data, DataSize));
    }
    catch (...) {
        ExceptionCode = errSlaveDeviceFailure;
    }
    ws->ReportServerID.cs->Leave();

    if (ExceptionCode == 0)
    {
        // The Data is passed without preprocess
        if (DataSize > MaxServerIDSize)
            DataSize = MaxServerIDSize;
        Event.EvtParam1 = DataSize;
        Answer->Function = RxPDU->Function;
        Answer->ByteCount = byte(DataSize);
        TxPDUSize = DataSize + 2; // 2 = (1)Function + (1)ByteCount
    }

    if (ExceptionCode != 0)
        return SetMBError(Event.EvtCode, RxPDU->Function, ExceptionCode, TxPDU, TxPDUSize);
    else
        return SetMBOk();
}
//------------------------------------------------------------------------------
TIndicationResult TMBDeviceExecutor::ManageCustomFunction(PMBPDU RxPDU, word RxPDUSize, PMBPDU TxPDU, word& TxPDUSize)
{
    byte ExceptionCode = 0;
    Event.EvtCode = evcCustomFunction;

    // Check the Callback assignment
    if (ws->UsrFunction.OnRequest == NULL)
        return SetMBError(Event.EvtCode, RxPDU->Function, errIllegalFunction, TxPDU, TxPDUSize);

    Event.EvtParam1 = RxPDU->Function;

    ws->UsrFunction.cs->Enter();
    try
    {
        ExceptionCode = byte(ws->UsrFunction.OnRequest(ws->UsrFunction.UsrPtr, RxPDU->Function, RxPDU, RxPDUSize, TxPDU, TxPDUSize));
    }
    catch (...) {
        ExceptionCode = errSlaveDeviceFailure;
    }
    ws->UsrFunction.cs->Leave();

    if (ExceptionCode != 0)
        return SetMBError(Event.EvtCode, RxPDU->Function, ExceptionCode, TxPDU, TxPDUSize);
    else
        return SetMBOk();
}
//------------------------------------------------------------------------------
TIndicationResult TMBDeviceExecutor::ExecuteIndication(PMBPDU RxPDU, word RxPDUSize, PMBPDU TxPDU, word& TxPDUSize)
{
    switch (RxPDU->Function)
    {
    // Serial and Ethernet
    case fun_ReadCoils:
        return ManageReadCoils(RxPDU, RxPDUSize, TxPDU, TxPDUSize);
    case fun_ReadDiscreteInputs:
        return ManageReadDiscreteInputs(RxPDU, RxPDUSize, TxPDU, TxPDUSize);
    case fun_ReadHoldingRegisters:
        return ManageReadHoldingRegisters(RxPDU, RxPDUSize, TxPDU, TxPDUSize);
    case fun_ReadInputRegisters:  
        return ManageReadInputRegisters(RxPDU, RxPDUSize, TxPDU, TxPDUSize);
    case fun_WriteSingleCoil:
        return ManageWriteSingleCoil(RxPDU, RxPDUSize, TxPDU, TxPDUSize);
    case fun_WriteSingleRegister:       
        return ManageWriteSingleRegister(RxPDU, RxPDUSize, TxPDU, TxPDUSize);
    case fun_WriteMultipleCoils:
        return ManageWriteMultipleCoils(RxPDU, RxPDUSize, TxPDU, TxPDUSize);
    case fun_WriteMultipleRegisters:
        return ManageWriteMultipleRegisters(RxPDU, RxPDUSize, TxPDU, TxPDUSize);
    case fun_MaskWriteRegister:
        return ManageMaskRegisterRequest(RxPDU, RxPDUSize, TxPDU, TxPDUSize);
    case fun_ReadWriteMultipleRegisters: 
        return ManageReadWriteMultipleRegistersRequest(RxPDU, RxPDUSize, TxPDU, TxPDUSize);
    case fun_ReadFileRecord:
        return ManageReadFileRecordRequest(RxPDU, RxPDUSize, TxPDU, TxPDUSize);
    case fun_WriteFileRecord:            
        return ManageWriteFileRecordRequest(RxPDU, RxPDUSize, TxPDU, TxPDUSize);
    case fun_ReadFIFOQueue:
        return ManageReadFIFOQueue(RxPDU, RxPDUSize, TxPDU, TxPDUSize);
    case fun_EncapsulatedIT:
        return ManageEncapsulatedIT(RxPDU, RxPDUSize, TxPDU, TxPDUSize);
    // Serial specific
    case fun_ReadExceptionStatus:
        if (ws->IsSerialDevice || ws->AllowSerFunOnEth)
            return ManageReadExceptionStatus(RxPDU, RxPDUSize, TxPDU, TxPDUSize);
        else
            return SetMBError(evcReadExcpStatus, RxPDU->Function, errIllegalFunction, TxPDU, TxPDUSize);
    case fun_Diagnostics:
        if (ws->IsSerialDevice || ws->AllowSerFunOnEth)
            return ManageDiagnostics(RxPDU, RxPDUSize, TxPDU, TxPDUSize);
        else
            return SetMBError(evcDiagnostics, RxPDU->Function, errIllegalFunction, TxPDU, TxPDUSize);
    case fun_GetCommEventCounter:
        if (ws->IsSerialDevice || ws->AllowSerFunOnEth)
            return ManageGetCommEventCounter(RxPDU, RxPDUSize, TxPDU, TxPDUSize);
        else
            return SetMBError(evcGetCommEvtCnt, RxPDU->Function, errIllegalFunction, TxPDU, TxPDUSize);
    case fun_GetCommEventLog:
        if (ws->IsSerialDevice || ws->AllowSerFunOnEth)
            return ManageGetCommEventLog(RxPDU, RxPDUSize, TxPDU, TxPDUSize);
        else
            return SetMBError(evcGetCommEvtLog, RxPDU->Function, errIllegalFunction, TxPDU, TxPDUSize);
    case fun_ReportServerID:
        if (ws->IsSerialDevice || ws->AllowSerFunOnEth)
            return ManageReportServerID(RxPDU, RxPDUSize, TxPDU, TxPDUSize);
        else
            return SetMBError(evcReportServerID, RxPDU->Function, errIllegalFunction, TxPDU, TxPDUSize);
    }
    // No known function found : let's check for a custom one, (in any case it must not have Bit 7 set)
    if ((RxPDU->Function & 0x80) == 0) 
    {
        if (ws->CustFunList[RxPDU->Function])
            return ManageCustomFunction(RxPDU, RxPDUSize, TxPDU, TxPDUSize);
    }
    // this hand went bad... 
    Event.EvtCode = evcInvalidFunction;
    Event.EvtParam1 = RxPDU->Function;
    return SetMBError(Event.EvtCode, RxPDU->Function, errIllegalFunction, TxPDU, TxPDUSize);
}
//------------------------------------------------------------------------------
void TMBDeviceExecutor::ClearEvents()
{
    memset(&Event, 0, sizeof(Event));
}
//------------------------------------------------------------------------------
// Get the Pointer and creates the CriticalSection if Data is not null
// A subsequent Registering will change only the pointer
//------------------------------------------------------------------------------
int TDeviceWorkingSet::RegisterArea(int AreaID, void* Data, int Amount)
{
    if (Amount == 0)
        return errDevAreaZero;

    switch (AreaID)
    {
    case mbAreaDiscreteInputs:
        Shared.Lock();
        if (Amount > SnapMB_MaxBits)
            return errDevAreaTooWide;
        Shared.DiscreteInputs.Data =Data;
        Shared.DiscreteInputs.Amount = Amount;
        if (Data != NULL && Shared.DiscreteInputs.cs == NULL)
            Shared.DiscreteInputs.cs = new TSnapCriticalSection();
        Shared.Unlock();
        break;
    case mbAreaCoils:
        Shared.Lock();
        if (Amount > SnapMB_MaxBits)
            return errDevAreaTooWide;
        Shared.Coils.Data = Data;
        Shared.Coils.Amount = Amount;
        if (Data != NULL && Shared.Coils.cs == NULL)
            Shared.Coils.cs = new TSnapCriticalSection();
        Shared.Unlock();
        break;
    case mbAreaInputRegisters:
        Shared.Lock();
        if (Amount > SnapMB_MaxRegisters)
            return errDevAreaTooWide;
        Shared.InputRegisters.Data = Data;
        Shared.InputRegisters.Amount = Amount;
        if (Data != NULL && Shared.InputRegisters.cs == NULL)
            Shared.InputRegisters.cs = new TSnapCriticalSection();
        Shared.Unlock();
        break;
    case mbAreaHoldingRegisters:
        Shared.Lock();
        if (Amount > SnapMB_MaxRegisters)
            return errDevAreaTooWide;
        Shared.HoldingRegisters.Data = Data;
        Shared.HoldingRegisters.Amount = Amount;
        if (Data != NULL && Shared.HoldingRegisters.cs == NULL)
            Shared.HoldingRegisters.cs = new TSnapCriticalSection();
        Shared.Unlock();
        break;
    default:
        return errDevUnknownAreaID;
    }
    return mbNoError;
}
//------------------------------------------------------------------------------
// Get the Pointer and creates the CriticalSection if cbRequest is not null
// A subsequent Registering will change only the pointer
//------------------------------------------------------------------------------
int TDeviceWorkingSet::RegisterCallback(int CallbackID, void* cbRequest, void* UsrPtr)
{
    int Result = mbNoError;
    switch (CallbackID)
    {
    case cbkDeviceEvent:
        DeviceEvent.OnRequest = pfn_DeviceEvent(cbRequest);
        DeviceEvent.UsrPtr = UsrPtr;
        if (cbRequest != NULL && DeviceEvent.cs == NULL)
            DeviceEvent.cs = new TSnapCriticalSection();
        break;
    case cbkPacketLog:
        PacketLog.OnRequest = pfn_PacketLog(cbRequest);
        PacketLog.UsrPtr = UsrPtr;
        if (cbRequest != NULL && PacketLog.cs == NULL)
            PacketLog.cs = new TSnapCriticalSection();
        break;
    case cbkDiscreteInputs:
        DiscreteInputs.OnRequest = pfn_DiscreteInputsRequest(cbRequest);
        DiscreteInputs.UsrPtr = UsrPtr;
        Shared.Lock();
        if (cbRequest != NULL && Shared.DiscreteInputs.cs == NULL)
            Shared.DiscreteInputs.cs = new TSnapCriticalSection();
        Shared.Unlock();
        if (cbRequest != NULL && DiscreteInputs.cs == NULL)
            DiscreteInputs.cs = new TSnapCriticalSection();
        break;
    case cbkCoils:
        Coils.OnRequest = pfn_CoilsRequest(cbRequest);
        Coils.UsrPtr = UsrPtr;
        Shared.Lock();
        if (cbRequest != NULL && Shared.Coils.cs == NULL)
            Shared.Coils.cs = new TSnapCriticalSection();
        Shared.Unlock();
        if (cbRequest != NULL && Coils.cs == NULL)
            Coils.cs = new TSnapCriticalSection();
        break;
    case cbkInputRegisters:
        InputRegisters.OnRequest = pfn_InputRegistersRequest(cbRequest);
        InputRegisters.UsrPtr = UsrPtr;
        Shared.Lock();
        if (cbRequest != NULL && Shared.InputRegisters.cs == NULL)
            Shared.InputRegisters.cs = new TSnapCriticalSection();
        Shared.Unlock();
        if (cbRequest != NULL && InputRegisters.cs == NULL)
            InputRegisters.cs = new TSnapCriticalSection();
        break;
    case cbkHoldingRegisters:
        HoldingRegisters.OnRequest = pfn_HoldingRegistersRequest(cbRequest);
        HoldingRegisters.UsrPtr = UsrPtr;
        Shared.Lock();
        if (cbRequest != NULL && Shared.HoldingRegisters.cs == NULL)
            Shared.HoldingRegisters.cs = new TSnapCriticalSection();
        Shared.Unlock();
        if (cbRequest != NULL && HoldingRegisters.cs == NULL)
            HoldingRegisters.cs = new TSnapCriticalSection();
        break;
    case cbkMaskRegister:
        MaskRegister.OnRequest = pfn_MaskRegisterRequest(cbRequest);
        MaskRegister.UsrPtr = UsrPtr;
        Shared.Lock();
        if (cbRequest != NULL && Shared.HoldingRegisters.cs == NULL)
            Shared.HoldingRegisters.cs = new TSnapCriticalSection();
        Shared.Unlock();
        if (cbRequest != NULL && MaskRegister.cs == NULL)
            MaskRegister.cs = new TSnapCriticalSection();
        break;
    case cbkFileRecord:
        FileRecord.OnRequest = pfn_FileRecordRequest(cbRequest);
        FileRecord.UsrPtr = UsrPtr;
        if (cbRequest != NULL && FileRecord.cs == NULL)
            FileRecord.cs = new TSnapCriticalSection();
        break;
    case cbkExceptionStatus:
        ExceptionStatus.OnRequest = pfn_ExceptionStatusRequest(cbRequest);
        ExceptionStatus.UsrPtr = UsrPtr;
        if (cbRequest != NULL && ExceptionStatus.cs == NULL)
            ExceptionStatus.cs = new TSnapCriticalSection();
        break;
    case cbkDiagnostics:
        Diagnostics.OnRequest = pfn_DiagnosticsRequest(cbRequest);
        Diagnostics.UsrPtr = UsrPtr;
        if (cbRequest != NULL && Diagnostics.cs == NULL)
            Diagnostics.cs = new TSnapCriticalSection();
        break;
    case cbkGetCommEventCounter:
        CommEventCounter.OnRequest = pfn_GetCommEventCounterRequest(cbRequest);
        CommEventCounter.UsrPtr = UsrPtr;
        if (cbRequest != NULL && CommEventCounter.cs == NULL)
            CommEventCounter.cs = new TSnapCriticalSection();
        break;
    case cbkGetCommEventLog:
        CommEventLog.OnRequest = pfn_GetCommEventLogRequest(cbRequest);
        CommEventLog.UsrPtr = UsrPtr;
        if (cbRequest != NULL && CommEventLog.cs == NULL)
            CommEventLog.cs = new TSnapCriticalSection();
        break;
    case cbkReportServerID:
        ReportServerID.OnRequest = pfn_ReportServerIDRequest(cbRequest);
        ReportServerID.UsrPtr = UsrPtr;
        if (cbRequest != NULL && ReportServerID.cs == NULL)
            ReportServerID.cs = new TSnapCriticalSection();
        break;
    case cbkReadFIFOQueue:
        FIFORequest.OnRequest = pfn_ReadFIFOQueueRequest(cbRequest);
        FIFORequest.UsrPtr = UsrPtr;
        if (cbRequest != NULL && FIFORequest.cs == NULL)
            FIFORequest.cs = new TSnapCriticalSection();
        break;
    case cbkEncapsulatedIT:
        MEIRequestCbk.OnRequest = pfn_EncapsulatedIT(cbRequest);
        MEIRequestCbk.UsrPtr = UsrPtr;
        if (cbRequest != NULL && MEIRequestCbk.cs == NULL)
            MEIRequestCbk.cs = new TSnapCriticalSection();
        break;
    case cbkUsrFunction:
        UsrFunction.OnRequest = pfn_UsrFunctionRequest(cbRequest);
        UsrFunction.UsrPtr = UsrPtr;
        if (cbRequest != NULL && UsrFunction.cs == NULL)
            UsrFunction.cs = new TSnapCriticalSection();
        break;
    case cbkPassthrough:
        Passthrough.OnRequest = pfn_Passthrough(cbRequest);
        Passthrough.UsrPtr = UsrPtr;
        if (cbRequest != NULL && Passthrough.cs == NULL)
            Passthrough.cs = new TSnapCriticalSection();
        break;
    default:
        Result = errDevUnknownCallbackID;
    }
    return Result;
}
//------------------------------------------------------------------------------
int TDeviceWorkingSet::SetCustomFunction(byte FunctionID, bool Value)
{
    if ((FunctionID > 64 && FunctionID < 73) || (FunctionID > 99 && FunctionID < 111))
    {
        CustFunList[FunctionID] = Value;
        return 0;
    }
    else
        return errInvalidUserFunction;
}
//------------------------------------------------------------------------------
int TDeviceWorkingSet::LockArea(int AreaID)
{
    switch (AreaID)
    {
    case mbAreaDiscreteInputs:
        if (Shared.DiscreteInputs.cs != NULL)
        {
            Shared.DiscreteInputs.cs->Enter();
            return 0;
        }
        else
            return errDevAreaZero;
    case mbAreaCoils:
        if (Shared.Coils.cs != NULL)
        {
            Shared.Coils.cs->Enter();
            return 0;
        }
        else
            return errDevAreaZero;
    case mbAreaInputRegisters:
        if (Shared.InputRegisters.cs != NULL)
        {
            Shared.InputRegisters.cs->Enter();
            return 0;
        }
        else
            return errDevAreaZero;
    case mbAreaHoldingRegisters:
        if (Shared.HoldingRegisters.cs != NULL)
        {
            Shared.HoldingRegisters.cs->Enter();
            return 0;
        }
        else
            return errDevAreaZero;
    }
    return errDevUnknownAreaID;
}
//------------------------------------------------------------------------------
int TDeviceWorkingSet::UnlockArea(int AreaID)
{
    switch (AreaID)
    {
    case mbAreaDiscreteInputs:
        if (Shared.DiscreteInputs.cs != NULL)
        {
            Shared.DiscreteInputs.cs->Leave();
            return 0;
        }
        else
            return errDevAreaZero;
    case mbAreaCoils:
        if (Shared.Coils.cs != NULL)
        {
            Shared.Coils.cs->Leave();
            return 0;
        }
        else
            return errDevAreaZero;
    case mbAreaInputRegisters:
        if (Shared.InputRegisters.cs != NULL)
        {
            Shared.InputRegisters.cs->Leave();
            return 0;
        }
        else
            return errDevAreaZero;
    case mbAreaHoldingRegisters:
        if (Shared.HoldingRegisters.cs != NULL)
        {
            Shared.HoldingRegisters.cs->Leave();
            return 0;
        }
        else
            return errDevAreaZero;
    }
    return errDevUnknownAreaID;
}
//------------------------------------------------------------------------------
int TDeviceWorkingSet::CopyArea(int AreaID, word Address, word Amount, void* Data, int CopyMode)
{
    int SizeToCopy;
    pbyte pStart;

    switch (AreaID)
    {
    case mbAreaDiscreteInputs:
        if (Shared.DiscreteInputs.cs != NULL)
        {
    
            if (Address + Amount > Shared.DiscreteInputs.Amount)
                SizeToCopy = Shared.DiscreteInputs.Amount - Address;
            else
                SizeToCopy = Amount;

            if (SizeToCopy > 0)
            {
                int64_t offset = Address - 1;
                if (offset < 0)
                    offset = 0;
                pStart = pbyte(Shared.DiscreteInputs.Data) + offset;
                Shared.DiscreteInputs.cs->Enter();
                try
                {
                    if (CopyMode) // Write
                        memcpy(pStart, Data, SizeToCopy);
                    else
                        memcpy(Data, pStart, SizeToCopy);
                }catch(...){}
                Shared.DiscreteInputs.cs->Leave();

                return mbNoError;
            }
            else
                return errDevInvalidParams;
        }
        else
            return errDevAreaZero;

    case mbAreaCoils:
        if (Shared.Coils.cs != NULL)
        {

            if (Address + Amount > Shared.Coils.Amount)
                SizeToCopy = Shared.Coils.Amount - Address;
            else
                SizeToCopy = Amount;

            if (SizeToCopy > 0)
            {
                int64_t offset = Address - 1;
                if (offset < 0)
                    offset = 0;
                pStart = pbyte(Shared.Coils.Data) + offset;;
                Shared.Coils.cs->Enter();
                try
                {
                    if (CopyMode) // Write
                        memcpy(pStart, Data, SizeToCopy);
                    else
                        memcpy(Data, pStart, SizeToCopy);
                }
                catch (...) {}
                Shared.Coils.cs->Leave();

                return mbNoError;
            }
            else
                return errDevInvalidParams;
        }
        else
            return errDevAreaZero;

    case mbAreaHoldingRegisters:
        if (Shared.HoldingRegisters.cs != NULL)
        {           
            if (Address + Amount > Shared.HoldingRegisters.Amount)
                SizeToCopy = (Shared.HoldingRegisters.Amount - Address) * 2;
            else
                SizeToCopy = Amount * 2;

            if (SizeToCopy > 0)
            {
                int64_t offset = (int64_t(Address) - 1) * 2;
                if (offset < 0)
                    offset = 0;
                pStart = pbyte(Shared.HoldingRegisters.Data) + offset;
                Shared.HoldingRegisters.cs->Enter();
                try
                {
                    if (CopyMode) // write
                        memcpy(pStart, Data, SizeToCopy);
                    else
                        memcpy(Data, pStart, SizeToCopy);
                }
                catch (...) {}
                Shared.HoldingRegisters.cs->Leave();

                return mbNoError;
            }
            else
                return errDevInvalidParams;
        }
        else
            return errDevAreaZero;

    case mbAreaInputRegisters:
        if (Shared.InputRegisters.cs != NULL)
        {
            if (Address + Amount > Shared.InputRegisters.Amount)
                SizeToCopy = (Shared.InputRegisters.Amount - Address) * 2;
            else
                SizeToCopy = Amount * 2;

            if (SizeToCopy > 0)
            {
                int64_t offset = (int64_t(Address) - 1) * 2;
                if (offset < 0)
                    offset = 0;

                pStart = pbyte(Shared.InputRegisters.Data) + offset;
                Shared.InputRegisters.cs->Enter();
                try
                {
                    if (CopyMode) // Write
                        memcpy(pStart, Data, SizeToCopy);
                    else
                        memcpy(Data, pStart, SizeToCopy);
                }
                catch (...) {}
                Shared.InputRegisters.cs->Leave();

                return mbNoError;
            }
            else
                return errDevInvalidParams;
        }
        else
            return errDevAreaZero;
    default:
        return errDevUnknownAreaID;
    }
}
//------------------------------------------------------------------------------
TDeviceWorkingSet::TDeviceWorkingSet(bool IsSerialDevice)
{
    this->IsSerialDevice = IsSerialDevice; // to filter some function not available for TCP / UDP
    AllowSerFunOnEth = def_AllowSerFunOnEth;
    // Resources initialization
    memset(&DeviceEvent, 0, sizeof(DeviceEvent));
    memset(&PacketLog, 0, sizeof(PacketLog));
    memset(&DiscreteInputs, 0, sizeof(DiscreteInputs));
    memset(&Coils, 0, sizeof(Coils));
    memset(&InputRegisters, 0, sizeof(InputRegisters));
    memset(&HoldingRegisters, 0, sizeof(HoldingRegisters));
    memset(&ReadWriteRegisters, 0, sizeof(ReadWriteRegisters));
    memset(&MaskRegister, 0, sizeof(MaskRegister));
    memset(&FileRecord, 0, sizeof(FileRecord));
    memset(&ExceptionStatus, 0, sizeof(ExceptionStatus));
    memset(&Diagnostics, 0, sizeof(Diagnostics));
    memset(&CommEventCounter, 0, sizeof(CommEventCounter));
    memset(&CommEventLog, 0, sizeof(CommEventLog));
    memset(&ReportServerID, 0, sizeof(ReportServerID));
    memset(&FIFORequest, 0, sizeof(FIFORequest));
    memset(&MEIRequestCbk, 0, sizeof(MEIRequestCbk));
    memset(&UsrFunction, 0, sizeof(UsrFunction));
    memset(&Passthrough, 0, sizeof(Passthrough));
    // Custom Functions List
    memset(&CustFunList, 0, sizeof(CustFunList));
}

//------------------------------------------------------------------------------
TDeviceWorkingSet::~TDeviceWorkingSet()
{
    if (DeviceEvent.cs != NULL)
        delete DeviceEvent.cs;
    if (PacketLog.cs != NULL)
        delete PacketLog.cs;
    if (Coils.cs != NULL)
        delete Coils.cs;
    if (DiscreteInputs.cs != NULL)
        delete DiscreteInputs.cs;
    if (InputRegisters.cs != NULL)
        delete InputRegisters.cs;
    if (HoldingRegisters.cs != NULL)
        delete HoldingRegisters.cs;
    if (ReadWriteRegisters.cs != NULL)
        delete ReadWriteRegisters.cs;
    if (MaskRegister.cs != NULL)
        delete MaskRegister.cs;
    if (FileRecord.cs != NULL)
        delete FileRecord.cs;
    if (ExceptionStatus.cs != NULL)
        delete ExceptionStatus.cs;
    if (Diagnostics.cs != NULL)
        delete Diagnostics.cs;
    if (CommEventCounter.cs != NULL)
        delete CommEventCounter.cs;
    if (CommEventLog.cs != NULL)
        delete CommEventLog.cs;
    if (ReportServerID.cs != NULL)
        delete ReportServerID.cs;
    if (FIFORequest.cs != NULL)
        delete FIFORequest.cs;
    if (MEIRequestCbk.cs != NULL)
        delete MEIRequestCbk.cs;   
    if (UsrFunction.cs != NULL)
        delete UsrFunction.cs;
    if (Passthrough.cs != NULL)
        delete Passthrough.cs;
}

//------------------------------------------------------------------------------
TSharedResources::TSharedResources()
{
    cs = new TSnapCriticalSection();
    memset(&DiscreteInputs, 0, sizeof(DiscreteInputs));
    memset(&Coils, 0, sizeof(Coils));
    memset(&InputRegisters, 0, sizeof(InputRegisters));
    memset(&HoldingRegisters, 0, sizeof(HoldingRegisters));
}

//------------------------------------------------------------------------------
TSharedResources::~TSharedResources()
{
    if (DiscreteInputs.cs != NULL)
        delete DiscreteInputs.cs;
    if (Coils.cs != NULL)
        delete Coils.cs;
    if (InputRegisters.cs != NULL)
        delete InputRegisters.cs;
    if (HoldingRegisters.cs != NULL)
        delete HoldingRegisters.cs;
    delete cs;
}

//------------------------------------------------------------------------------
void TSharedResources::Lock()
{
    cs->Enter();
}

//------------------------------------------------------------------------------
void TSharedResources::Unlock()
{
    cs->Leave();
}

