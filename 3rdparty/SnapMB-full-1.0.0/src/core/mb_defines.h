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
#ifndef mb_defines_h
#define mb_defines_h
//------------------------------------------------------------------------------
#include "snap_platform.h"
#include "snap_threads.h"
//------------------------------------------------------------------------------

#pragma pack(1)
//******************************************************************************
// MODBUS LIMITS - TRANSPORT AND DATA LINK LAYER
//------------------------------------------------------------------------------
// References:
//   MODBUS Application Protocol Specification V1.1b3 
//   MODBUS over serial line specification and implementation guide V1.02
//   MODBUS Messaging on TCP/IP Implementation Guide V1.0b
//******************************************************************************
//
// PDU (Protocol Data Unit)  = Function + Data
// 
// TCP/UDP/RTU    : 1 Function + 252 Data = Max 253
// ASC            : 2 Function + 504 Data = Max 506 
// 
// MBAP (TCP/UDP) : 2 Trans ID + 2 Proto ID + 2 Following Len + 1 Unit ID = 7     
// 
//==============================================================================
// 
// ADU (Application Data Unit) =  Header + PDU + Trailer
// 
// TCP : 7 MBAP + 253 PDU + 0 Trailer                         = Max 260
// UDP : 7 MBAP + 253 PDU + 0 Trailer                         = Max 260
// RTU : 1 Address + 253 PDU + 2 CRC                          = Max 256
// ASC : 1 Start Frame + 2 Address + 506 PDU + 2 LRC + 2 CRLF = Max 513
//
//******************************************************************************

#define MaxBinPDUSize   253
#define MaxAscPDUSize   506
#define MaxNetADUSize   260 // Same for TCP or UDP 
#define MaxRtuADUSize   256
#define MaxAscADUSize   513 

#define tcp_mb_protoid  0x0000 // Protocol ID for Modbus/TCP

#define mbBulkData      MaxAscADUSize // raw amount able to contain every Modbus ADU

//******************************************************************************
// MODBUS LIMITS - PROTOCOL APPLICATION LAYER
//------------------------------------------------------------------------------
// MODBUS Application Protocol Specification V1.1b3 
// MODBUS over serial line specification and implementation guide V1.02
// MODBUS Messaging on TCP/IP Implementation Guide V1.0b
//******************************************************************************

#define Reg16Size                  2   // 16 bit Modbus Register = 2 byte
#define ChannelNameSize            40  // Port Name or IP Address

#define MaxReadMultipleRegisters  125  // Max 16Bit Registers
#define MaxWriteMultipleRegisters 123  // Max 16Bit Registers
#define MaxWriteMultiReg_fun17    121  // Max 16Bit Registers for function 0x17

#define MaxWriteMultipleBitsSize  246  // 255 bytes
#define MaxWriteMultipleBits      1968 // 246*8, only Coils

#define MaxReadMultipleBitsSize   250  // 250 bytes
#define MaxReadMultipleBits       2000 // 125*8 Coils or Discrete Inputs

#define MaxRDFileRecRegsAmount    124  // (253 - 4(Header)) / 2
#define MaxWRFileRecRegsAmount    124  // (253 - 9(Header)) / 2
#define MaxWriteFileRecords       251  // = 9 byte Descriptor * 27

#define MaxDiagnosticsItems       125  // (253 - 3(Fun,SubFun))/ 2 = 125;
#define MaxCommLogEvents          245  // 253 - 8(Function, ByteCount, 2Status, 2EventCount, 2MessageCount) = 245
#define MaxServerIDSize           251  // 253 - 2(Function, ByteCount)

#define MaxReadFIFOSize           248  // 253 - 5 (see TReadFifoReqHeader) 
#define MaxReadFIFOItems          124  // MaxReadFifoSize / 2
#define MaxMEIDataSize            251  // 253 - Function - MEI_type

#define MinimumRtuPDUSize           3  // The smallest well-formed RTU PDU : function + CRC H + CRC L
#define MinimumRtuADUAnswer         5  // The smallest well-formed RTU ADU : device ID + function + exception + CRC H + CRC L

//******************************************************************************
// SnapMB LIMITS - USER APPLICATION LAYER
//******************************************************************************

#define SnapMB_MaxRegisters  32768 // Max numbers of 16bit registers (R/W)
#define SnapMB_MaxBits       65536 // Max number of Coils/Inputs (R/W)
#define MaxDevices           256   // Max number of Devices / Clients in a NetController
#define MaxConnections       1024  // Max Device concurrent connections

//******************************************************************************
// MODBUS FUNCTIONS - PROTOCOL APPLICATION LAYER
//------------------------------------------------------------------------------
// MODBUS Application Protocol Specification V1.1b3 
//******************************************************************************

// Common Serial and Ethernet
#define fun_ReadCoils                  0x01
#define fun_ReadDiscreteInputs         0x02
#define fun_ReadHoldingRegisters       0x03
#define fun_ReadInputRegisters         0x04
#define fun_WriteSingleCoil            0x05
#define fun_WriteSingleRegister        0x06
#define fun_WriteMultipleCoils         0x0F
#define fun_WriteMultipleRegisters     0x10
#define fun_MaskWriteRegister          0x16
#define fun_ReadWriteMultipleRegisters 0x17
#define fun_ReadFIFOQueue              0x18
#define fun_ReadFileRecord             0x14
#define fun_WriteFileRecord            0x15
#define fun_EncapsulatedIT             0x2B
// Serial specific
#define fun_ReadExceptionStatus        0x07
#define fun_Diagnostics                0x08
#define fun_GetCommEventCounter        0x0B
#define fun_GetCommEventLog            0x0C
#define fun_ReportServerID             0x11

//******************************************************************************
//  MODBUS FLAVOURS
//******************************************************************************

const int ProtoTCP        = 0;   // Modbus/TCP
const int ProtoUDP        = 1;   // Modbus/TCP using UDP as transport
const int ProtoRTUOverTCP = 2;   // Modbus RTU wrapped in a TCP Packet
const int ProtoRTUOverUDP = 3;   // Modbus RTU wrapped in a UDP Packet
const int FormatRTU       = 0;   // Serial RTU
const int FormatASC       = 1;   // Serial ASCII

//******************************************************************************
//  SERIAL LINE CONTROL 
//******************************************************************************

const int FlowNONE        = 0;   // No hardware flow control 
const int FlowRTSCTS      = 1;   // RTS/CTS flow control 

//******************************************************************************
//  BUFFER KIND (see GetIOBuffer/GetIOBufferPtr)
//******************************************************************************

const int BkSnd = 0;   // Buffer Sent 
const int BkRcv = 1;   // Buffer Recv 

//******************************************************************************
//  PDU 
//******************************************************************************

// Binary PDU (TCP/UDP/RTU and ASCII after the conversion)
typedef struct {
    byte Function;
    byte Data[mbBulkData];
}TMBPDU;
typedef TMBPDU* PMBPDU;

//******************************************************************************
//  ADU STRUCTS
//------------------------------------------------------------------------------
// Note :
//        Even tough the whole size is greater than the max allowed, only the
//        specification size will be transmitted/received.
//******************************************************************************

// Modbus Application Protocol Header for TCP (7 bytes) 
typedef struct {
    word TransID;       // Cyclic ID echoed by the slave
    word ProtocolID;    // 0x0000
    word PduLength;     // sizeof Payload(the PDU) + 1(the UnitID)
    byte UnitID;        // Gateway routing byte (0xff -> No routing)
} TMBAP;
typedef TMBAP* PMBAP;
const int MBAPSize = sizeof(TMBAP);

// TCP/UDP ADU 
typedef struct {
    TMBAP MBAP;
    byte PDU[mbBulkData - sizeof(TMBAP)];
}TTCPADU;
typedef TTCPADU* PTCPADU;

//******************************************************************************
//  Packets, not protocol related, but useful to encapsulate the data and 
//  its real size
//******************************************************************************

// Client/Controller related

typedef struct {
    byte ADU[mbBulkData];
    word Size; // Size to Send or Received
}TMBBulkPacket;

typedef TMBBulkPacket TMBSERPacket;
typedef TMBSERPacket* PMBSERPacket;

typedef struct {
    TTCPADU ADU;
    word Size;   // Size to Send or Received
}TMBTCPPacket;
typedef TMBTCPPacket* PMBTCPPacket;

// Device related

typedef struct {
    TMBAP MBAP;
    TMBPDU PDU;
    word Size;   // Size to Send or Received
}TMBNetDevicePacket;
typedef TMBNetDevicePacket* PMBNetDevicePacket;

typedef struct {
    byte DeviceID;
    byte PDU[mbBulkData];
    word Size;   // Size to Send or Received
}TMBRtuDevicePacket;
typedef TMBRtuDevicePacket* PMBRtuDevicePacket;

typedef TMBBulkPacket TMBAscDevicePacket;
typedef TMBAscDevicePacket* PMBAscDevicePacket;

typedef TMBBulkPacket TMBAnswerDevicePacket;
typedef TMBAnswerDevicePacket* PMBAnswerDevicePacket;

//******************************************************************************
// ERRORS
//------------------------------------------------------------------------------
// The Error is a "record" composed by some fields which are chained
//******************************************************************************
#define mbNoError 0

const int MaskObjects                = 0xF0000000;
const int MaskCategory               = 0x000F0000;
const int MaskErrNo                  = 0x0000FFFF;

// Base Objects
const int errLibrary                 = 0x10000000;
const int errSerialClient            = 0x20000000;
const int errEthernetClient          = 0x30000000;
const int errFieldController         = 0x40000000;
const int errSerialDevice            = 0x50000000;
const int errEthernetDevice          = 0x60000000;

// Library Errors
const int errCategoryLibrary         = 0x00010000;
const int errNullObject              = 0x00000001;  // Null object passed
const int errObjectInvalidMethod     = 0x00000002;  // invalid method for this object

// Serial socket Errors
const int errCategorySerialSocket    = 0x00020000; // Serial socket error
const int errPortInvalidParams       = 0x00000001;
const int errPortSettingsTimeouts    = 0x00000002;
const int errPortSettingsParams      = 0x00000003;
const int errOpeningPort             = 0x00000004;
const int errPortReadTimeout         = 0x00000005;
const int errPortWriteTimeout        = 0x00000006;
const int errPortReadError           = 0x00000007;
const int errPortWriteError          = 0x00000008;
const int errBufferOverflow          = 0x00000009;
const int errPortGetParams           = 0x0000000A;
const int errPortLocked              = 0x0000000B;
const int errInterframe              = 0x0000000C;

const int errCategoryNetSocket       = 0x00030000; // TCP-UDP Error
// TCP-UDP/IP Socket Errors          = 0x0000XXXX  // Last 2 byte will contain the Socket Layer error

// Modbus Protocol Errors 
const int errCategoryMBProtocol      = 0x00040000; // Protocol error (0x8X received)
const int errIllegalFunction         = 0x00000001; // Exception Code 0x01
const int errIllegalDataAddress      = 0x00000002; // Exception Code 0x02
const int errIllegalDataValue        = 0x00000003; // Exception Code 0x03
const int errSlaveDeviceFailure      = 0x00000004; // Exception Code 0x04
const int errAcknowledge             = 0x00000005; // Exception Code 0x05
const int errSlaveDeviceBusy         = 0x00000006; // Exception Code 0x06
const int errNegativeAcknowledge     = 0x00000007; // Exception Code 0x07
const int errMemoryParityError       = 0x00000008; // Exception Code 0x08
const int errGatewayPathUnavailable  = 0x00000010; // Exception Code 0x10
const int errGatewayTargetFailed     = 0x00000011; // Exception Code 0x11

// Process Errors
const int errCategoryProcess         = 0x00050000; // Process error
const int errInvalidBroadcastFunction= 0x00000001;
const int errInvalidParamIndex       = 0x00000002;
const int errInvalidAddress          = 0x00000003;
const int errInvalidDataAmount       = 0x00000004;
const int errInvalidADUReceived      = 0x00000005;
const int errInvalidChecksum         = 0x00000006;
const int errTimeout                 = 0x00000007;
const int errInvalidDeviceID         = 0x00000008;
const int errInvalidUserFunction     = 0x00000009;
const int errInvalidReqForThisObject = 0x0000000A;

// Field Controller errors
const int errUndefinedBroker         = 0x0000000B;
const int errUndefinedClient         = 0x0000000C;
const int errDeviceIDZero            = 0x0000000D;
const int errDeviceAlreadyExists     = 0x0000000E;
const int errUndefinedController     = 0x0000000F;
const int errSomeConnectionsError    = 0x00000010;
const int errCommParamsMismatch      = 0x00000011;

// 0x00000012..0x000000FF : available

// Device errors
const int errDevUnknownAreaID        = 0x00000100; // Unknown Area ID
const int errDevAreaZero             = 0x00000101; // Area Amount = 0
const int errDevAreaTooWide          = 0x00000102; // Area Amount too wide 
const int errDevUnknownCallbackID    = 0x00000103; // Unknown Callback ID
const int errDevInvalidParams        = 0x00000104; // Invalid param(s) supplied
const int errDevInvalidParamIndex    = 0x00000105; // Invalid param (GetDeviceParam())
const int errDevOpNotAllowed         = 0x00000106; // Cannot change because running
const int errDevTooManyPeers         = 0x00000107; // To many Peers for Deny/AcceptList (only TCP)
const int errDevCannotRebindOnRun    = 0x00000108; // Device is running, stop first

//********************************************************************************
// Functions Structs, I don't like to set anonymous byte into anonymous arrays ;) 
//********************************************************************************

//------------------------------------------------------------------------------
// Data Read - Functions 0x01, 0x02, 0x03, 0x04
//------------------------------------------------------------------------------

// Data Read Request
typedef struct {
    byte Function;
    word Address;
    word Amount;
}TDataReadReqHeader;
typedef TDataReadReqHeader* PDataReadReqHeader;
#define DataReadReqHeaderSize sizeof(TDataReadReqHeader)

// DataRead Answer
typedef struct {
    byte Function;
    byte ByteCount;
    byte Data[mbBulkData];
}TDataReadResPacket;
typedef TDataReadResPacket* PDataReadResPacket;

//------------------------------------------------------------------------------
// Data Write - Functions 0x0F, 0x10
//------------------------------------------------------------------------------

// Data Write Request
typedef struct {
    byte Function;
    word Address;
    word Amount;
    byte ByteCount;
}TDataWriteReqHeader;
typedef TDataWriteReqHeader* PDataWriteReqHeader;
#define DataWriteReqHeaderSize sizeof(TDataWriteReqHeader)

// Data write Request Packet
typedef struct {
    TDataWriteReqHeader Header;
    byte Data[mbBulkData];
}TDataWriteReqPacket;
typedef TDataWriteReqPacket* PDataWriteReqPacket;

// Data Write Answer Header
typedef struct {
    byte Function;
    word Address;
    word Amount;
}TDataWriteResHeader;
typedef TDataWriteResHeader* PDataWriteResHeader;
#define DataWriteResHeaderSize sizeof(TDataWriteResHeader)

//------------------------------------------------------------------------------
// Data Write single item - Functions 0x05, 0x06
//------------------------------------------------------------------------------

// Data Write Single Item, Request and Answer
typedef struct {
    byte Function;
    word Address;
    word Value;
}TDataSingleWritePacket;
typedef TDataSingleWritePacket* PDataSingleWritePacket;
#define DataSingleWritePacketSize sizeof(TDataSingleWritePacket)

//------------------------------------------------------------------------------
// Read/Write multiple Registers - Function  0x17
//------------------------------------------------------------------------------

typedef struct {
    byte Function;
    word RDAddress;
    word RDAmount;
    word WRAddress;
    word WRAmount;
    byte ByteCount;
}TDataReadWriteMultiRegHeader;
typedef TDataReadWriteMultiRegHeader* PDataReadWriteMultiRegHeader;
#define TDataReadWriteMultiRegHeaderSize sizeof(TDataReadWriteMultiRegHeader)

typedef struct {
    TDataReadWriteMultiRegHeader Header;
    byte Data[mbBulkData];
}TDataReadWriteMultiRegPacket;
typedef TDataReadWriteMultiRegPacket* PDataReadWriteMultiRegPacket;

//------------------------------------------------------------------------------
// Mask Write register Packet Request/Answer  - Function 0x16
//------------------------------------------------------------------------------

typedef struct {
    byte Function;
    word Address;
    word AND_Mask;
    word OR_Mask;
}TMaskRegisterPacket;
typedef TMaskRegisterPacket* PMaskRegisterPacket;
#define TMaskRegisterPacketSize sizeof(TMaskRegisterPacket)

//------------------------------------------------------------------------------
// Read/Write File Record - Functions 0x14, 0x15
//------------------------------------------------------------------------------

// File Record Definition (only one at time is managed)
typedef struct {
    byte Function;
    byte Length;         
    byte RefType;
    word FileNumber;
    word RecNumber;
    word RecLength;
}TFileRecordHeader;
typedef TFileRecordHeader* PFileRecordHeader;

#define FileRecordDefSize 7  // RefType + FileNumber + RecNumber + RecLength

// Answer Packet
typedef struct {
    byte Function;
    byte DataLength;
    byte RecLen;
    byte RefType;
    byte Data[mbBulkData];
}TReadFileRecAnswer;
typedef TReadFileRecAnswer* PReadFileRecAnswer;

// Write Record Packet (Request and Answer)
typedef struct {
    TFileRecordHeader Header;
    byte Data[mbBulkData]; 
}TWriteFileRecPacket;
typedef TWriteFileRecPacket* PWriteFileRecPacket;

//------------------------------------------------------------------------------
// Read FIFO Queue - Function 0x18      
// Note:
//   Modbus specification states that the maximum number of register must be 32
//   We accept also an higher number but *IT MUST FIT INTO MODBUS PDU*
//------------------------------------------------------------------------------

// Request
typedef struct {
    byte Function;
    word PtrAddress;
}TReadFifoReqHeader;
typedef TReadFifoReqHeader* PReadFifoReqHeader;

// Answer                      
typedef struct {
    byte Function;
    word ByteCount;
    word FifoCount;
    byte Data[MaxReadFIFOSize];                                  
}TReadFifoReqAnswer;
typedef TReadFifoReqAnswer* PReadFifoReqAnswer;

//------------------------------------------------------------------------------
// MEI - Modbus Encapsulated Interface - Function 0x2B      
//------------------------------------------------------------------------------
 
// Request                                
typedef struct {
    byte Function;
    byte MEI_Type;
    byte Data[MaxMEIDataSize];
}TMEIRequest;
typedef TMEIRequest* PMEIRequest;

// Answer                                 
typedef struct {
    byte Function;
    byte MEI_Type;
    byte Data[MaxMEIDataSize];
}TMEIAnswer;
typedef TMEIAnswer* PMEIAnswer;

//------------------------------------------------------------------------------
// Read Exception status - Function 0x07      
//------------------------------------------------------------------------------

// Request                                
typedef struct {
    byte Function;
}TReadExceptionStatusRequest;
typedef TReadExceptionStatusRequest* PReadExceptionStatusRequest;

// Answer                                 
typedef struct {
    byte Function;
    byte Data;
}TReadExceptionStatusAnswer;
typedef TReadExceptionStatusAnswer* PReadExceptionStatusAnswer;

//------------------------------------------------------------------------------
// Diagnostics - Function 0x08      
//------------------------------------------------------------------------------

// Request/Answer (same)
typedef struct {
    byte Function;
    word SubFunction;
    word Data[MaxDiagnosticsItems];
}TDiagnosticPDU;
typedef TDiagnosticPDU* PDiagnosticPDU;

//------------------------------------------------------------------------------
// Get Comm Event Counter - Function 0x0B      
//------------------------------------------------------------------------------

// Request
typedef struct {
    byte Function;
}TGetEventCounterRequest;
typedef TGetEventCounterRequest* PGetEventCounterRequest;

// Answer
typedef struct {
    byte Function;
    word Status;
    word EventCount;
}TGetEventCounterAnswer;
typedef TGetEventCounterAnswer* PGetEventCounterAnswer;

//------------------------------------------------------------------------------
// Get Comm Event Log - Function 0x0C      
//------------------------------------------------------------------------------

// Request
typedef struct {
    byte Function;
}TGetCommEventLogRequest;
typedef TGetCommEventLogRequest* PGetCommEventLogRequest;

// Answer
typedef struct {
    byte Function;
    byte ByteCount;
    word Status;
    word EventCount;
    word MessageCount;
    byte Events[MaxCommLogEvents];
}TGetCommEventLogAnswer;
typedef TGetCommEventLogAnswer* PGetCommEventLogAnswer;

//------------------------------------------------------------------------------
// Report ServerID - Function 0x11      
//------------------------------------------------------------------------------

// Request
typedef struct {
    byte Function;
}TServerIDRequest;
typedef TServerIDRequest* PServerIDRequest;

// Answer
typedef struct {
    byte Function;
    byte ByteCount;
    byte Data[MaxServerIDSize]; // It's vendor specific
}TServerIDAnswer;
typedef TServerIDAnswer* PServerIDAnswer;

//------------------------------------------------------------------------------
// User Function (request)
//------------------------------------------------------------------------------
// Request Packet
typedef struct {
    byte Function;
    byte Data[mbBulkData];
}TUsrFunctionRequest;
typedef TUsrFunctionRequest* PUsrFunctionRequest;

//------------------------------------------------------------------------------
// Error response - Common for all functions 
//------------------------------------------------------------------------------

// Answer               
typedef struct {
    byte Code;
    byte Exception;
}TErrorResponse;
typedef TErrorResponse* PErrorResponse;

//------------------------------------------------------------------------------
// PDU Receive Expected 
// This struct is used only for Serial communications, the TCP ADU already 
// contains the Size expected into MBAP
//------------------------------------------------------------------------------

typedef struct {
    byte DeviceID;     // Echo
    byte Function;     // Echo
    word Size;         // PDU size expected
}TPDURecvExpected;
typedef TPDURecvExpected* PPDURecvExpected;
//------------------------------------------------------------------------------
// Some RTU Related
//------------------------------------------------------------------------------

typedef struct {
    byte DeviceID;
    byte Function;
    byte Data[mbBulkData];
}TRTUAnswer;
typedef TRTUAnswer* PRTUAnswer;
typedef TRTUAnswer TRTURequest;
typedef TRTURequest* PRTURequest;

typedef struct {
    byte DeviceID;
    byte ErrorCode;
    byte Exception;
    word CRC16;
}TRTUError;
typedef TRTUError* PRTUError;

//------------------------------------------------------------------------------
// Register utility for Pack/Unpack
//------------------------------------------------------------------------------

// Bit array
typedef struct {
    byte BitArray[SnapMB_MaxBits];
}TMBBits;
typedef TMBBits* PMBBits;

// 16 bit register pack
typedef struct {
    word WordArray[SnapMB_MaxRegisters];
}TRegisters16;
typedef TRegisters16* PRegisters16;

// 32 bit register pack
typedef struct {
    longword DWordArray[SnapMB_MaxRegisters/2];
}TRegisters32;
typedef TRegisters32* PRegisters32;

// 64 bit register pack
typedef struct {
    uint64_t QWordArray[SnapMB_MaxRegisters / 4];
}TRegisters64;
typedef TRegisters64* PRegisters64;

//******************************************************************************
// DEVICE Specific
//******************************************************************************

#define MaxTCPPeersList  256  // TCP Device max Peer List
#define MaxUDPPeersList  256  // UDP Device max Peer List
#define plmDisabled      0
#define plmAllowList     1
#define plmBlockList     2
#define MaxEvents        1500 // Events Queue Size

const int WorkerCloseTimeout     = 3000;

// Area Selectors
const int mbAreaDiscreteInputs   = 0;
const int mbAreaCoils            = 1;
const int mbAreaInputRegisters   = 2;
const int mbAreaHoldingRegisters = 3;

// Callbacks Selectors
const int cbkDeviceEvent         = 0;
const int cbkPacketLog           = 1;
const int cbkDiscreteInputs      = 2;
const int cbkCoils               = 3;
const int cbkInputRegisters      = 4;
const int cbkHoldingRegisters    = 5;
const int cbkReadWriteRegisters  = 6;
const int cbkMaskRegister        = 7;
const int cbkFileRecord          = 8;
const int cbkExceptionStatus     = 9;
const int cbkDiagnostics         = 10;
const int cbkGetCommEventCounter = 11;
const int cbkGetCommEventLog     = 12;
const int cbkReportServerID      = 13;
const int cbkReadFIFOQueue       = 14;
const int cbkEncapsulatedIT      = 15;
const int cbkUsrFunction         = 16;
const int cbkPassthrough         = 17;

// Callbacks Actions  
const int cbActionRead           = 0;
const int cbActionWrite          = 1;

// CopyArea()
const int CopyModeRead           = 0;
const int CopyModeWrite          = 1;

const int PacketLog_NONE         = 0;
const int PacketLog_IN           = 1;
const int PacketLog_OUT          = 2;
const int PacketLog_BOTH         = 3;

// DEVICE Callbacks
extern "C"
{
    typedef int (SNAP_API* pfn_Request)(); // not directly used, it's just a function pointer to cast
    typedef void (SNAP_API* pfn_DeviceEvent)(void* usrPtr, void* PEvent, int Size);
    typedef void (SNAP_API* pfn_PacketLog)(void* usrPtr, longword Peer, int Direction, void* Data, int Size);
    typedef int (SNAP_API* pfn_DiscreteInputsRequest)(void* usrPtr, word Address, word Amount, void* Data);
    typedef int (SNAP_API* pfn_CoilsRequest)(void* usrPtr, int Action, word Address, word Amount, void* Data);
    typedef int (SNAP_API* pfn_InputRegistersRequest)(void* usrPtr, word Address, word Amount, void* Data);
    typedef int (SNAP_API* pfn_HoldingRegistersRequest)(void* usrPtr, int Action, word Address, word Amount, void* Data);
    typedef int (SNAP_API* pfn_ReadWriteMultipleRegistersRequest)(void* usrPtr, word RDAddress, word RDAmount, void* RDData, word WRAddress, word WRAmount, void* WRData);
    typedef int (SNAP_API* pfn_MaskRegisterRequest)(void* usrPtr, word Address, word AND_Mask, word OR_Mask);
    typedef int (SNAP_API* pfn_FileRecordRequest)(void* usrPtr, int Action, word RefType, word FileNumber, word RecNumber, word RegsAmount, void* Data);
    typedef int (SNAP_API* pfn_ExceptionStatusRequest)(void* usrPtr, byte& Status);
    typedef int (SNAP_API* pfn_DiagnosticsRequest)(void* usrPtr, word SubFunction, void* RxItems, void* TxItems, word ItemsSent, word& ItemsRecvd);
    typedef int (SNAP_API* pfn_GetCommEventCounterRequest)(void* usrPtr, word& Status, word& EventCount);
    typedef int (SNAP_API* pfn_GetCommEventLogRequest)(void* usrPtr, word& Status, word& EventCount, word& MessageCount, void* Data, word& EventsAmount);
    typedef int (SNAP_API* pfn_ReportServerIDRequest)(void* usrPtr, void* Data, word& DataSize);
    typedef int (SNAP_API* pfn_ReadFIFOQueueRequest)(void* usrPtr, word PtrAddress, void* FIFOValues, word& FifoCount);
    typedef int (SNAP_API* pfn_EncapsulatedIT)(void* usrPtr, byte MEI_Type, void* MEI_DataReq, word ReqDataSize, void* MEI_DataRes, word& ResDataSize);
    typedef int (SNAP_API* pfn_UsrFunctionRequest)(void* usrPtr, byte Function, void* RxPDU, word RxPDUSize, void* TxPDU, word& TxPDUSize);
    typedef int (SNAP_API* pfn_Passthrough)(void* usrPtr, byte DeviceID, void* RxPDU, word RxPDUSize, void* TxPDU, word& TxPDUSize);
}

typedef longword TPeerList[MaxTCPPeersList];
typedef TPeerList* PPeerList;

typedef struct {
    pfn_DeviceEvent OnRequest;
    PSnapCriticalSection cs;
    void* UsrPtr;
}TDeviceEventCbk;
typedef TDeviceEventCbk* PDeviceEventCbk;

typedef struct {
    pfn_PacketLog OnRequest;
    PSnapCriticalSection cs;
    void* UsrPtr;
}TPacketLog;
typedef TPacketLog* PPacketLog;

typedef struct {
    pfn_DiscreteInputsRequest OnRequest;
    PSnapCriticalSection cs;
    void* UsrPtr;
}TDiscreteInputs;
typedef TDiscreteInputs* PDiscreteInputs;

typedef struct {
    pfn_CoilsRequest OnRequest;
    PSnapCriticalSection cs;
    void* UsrPtr;
}TCoils;
typedef TCoils* PCoils;

typedef struct {
    pfn_InputRegistersRequest OnRequest;
    PSnapCriticalSection cs;
    void* UsrPtr;
}TInputRegisters;
typedef TInputRegisters* PInputRegisters;

typedef struct {
    pfn_HoldingRegistersRequest OnRequest;
    PSnapCriticalSection cs;
    void* UsrPtr;
}THoldingRegisters;
typedef THoldingRegisters* PHoldingRegisters;

typedef struct {
    pfn_ReadWriteMultipleRegistersRequest OnRequest;
    PSnapCriticalSection cs;
    void* UsrPtr;
}TReadWriteRegisters;
typedef TReadWriteRegisters* PReadWriteRegisters;

typedef struct {
    pfn_MaskRegisterRequest OnRequest;
    PSnapCriticalSection cs;
    void* UsrPtr;
}TMaskRegister;
typedef TMaskRegister* PMaskRegister;

typedef struct {
    pfn_FileRecordRequest OnRequest;
    PSnapCriticalSection cs;
    void* UsrPtr;
}TFileRecord;
typedef TFileRecord* PFileRecord;

typedef struct {
    pfn_UsrFunctionRequest OnRequest;
    PSnapCriticalSection cs;
    void* UsrPtr;
}TUsrFunction;
typedef TUsrFunction* PUsrFunction;

typedef struct {
    pfn_Passthrough OnRequest;
    PSnapCriticalSection cs;
    void* UsrPtr;
}TPassthroughEvent;
typedef TPassthroughEvent* PPassthroughEvent;

typedef struct {
    pfn_ExceptionStatusRequest OnRequest;
    PSnapCriticalSection cs;
    void* UsrPtr;
}TExceptionStatus;
typedef TExceptionStatus* PExceptionStatus;

typedef struct {
    pfn_DiagnosticsRequest OnRequest;
    PSnapCriticalSection cs;
    void* UsrPtr;
}TDiagnostics;
typedef TDiagnostics* PDiagnostics;

typedef struct {
    pfn_GetCommEventCounterRequest OnRequest;
    PSnapCriticalSection cs;
    void* UsrPtr;
}TGetCommEventCounter;
typedef TGetCommEventCounter* PGetCommEventCounter;

typedef struct {
    pfn_GetCommEventLogRequest OnRequest;
    PSnapCriticalSection cs;
    void* UsrPtr;
}TGetCommEventLog;
typedef TGetCommEventLog* PGetCommEventLog;

typedef struct {
    pfn_ReportServerIDRequest OnRequest;
    PSnapCriticalSection cs;
    void* UsrPtr;
}TReportServerID;
typedef TReportServerID* PReportServerId;

typedef struct {
    pfn_ReadFIFOQueueRequest OnRequest;
    PSnapCriticalSection cs;
    void* UsrPtr;
}TFIFORequest;
typedef TFIFORequest* PFIFORequest;

typedef struct {
    pfn_EncapsulatedIT OnRequest;
    PSnapCriticalSection cs;
    void* UsrPtr;
}TMEIRequestCbk;
typedef TMEIRequestCbk* PMEIRequestCbk;

// Shared resources

typedef struct {
    PSnapCriticalSection cs;
    void* Data;  // Must be Packed
    int Amount;  // Number of "unpacked" bits (1 bit -> 1 byte) 
}TSharedDiscreteInputs;
typedef TSharedDiscreteInputs* PSharedDiscreteInputs;

typedef struct {
    PSnapCriticalSection cs;
    void* Data;  // Must be Packed
    int Amount;  // Number of "unpacked" bits (1 bit -> 1 byte) 
}TSharedCoils;
typedef TSharedCoils* PSharedCoils;

typedef struct {
    PSnapCriticalSection cs;
    void* Data;  // Must be Packed
    int Amount;  // Number of registers (1 Register -> 2 byte) 
}TSharedInputRegisters;
typedef TSharedInputRegisters* PSharedInputRegisters;

typedef struct {
    PSnapCriticalSection cs;
    void* Data;  // Must be Packed
    int Amount;  // Number of registers (1 Register -> 2 byte) 
}TSharedHoldingRegisters;
typedef TSharedHoldingRegisters* PSharedHoldingRegisters;

//------------------------------------------------------------------------------
// Device Events
//------------------------------------------------------------------------------
#define WSAEINVALIDADDRESS   12001

// Device Base
const longword evcDeviceStarted         = 0x00000001;
const longword evcDeviceStopped         = 0x00000002;
const longword evcDeviceCannotStart     = 0x00000004;
const longword evcDevClientAdded        = 0x00000008;
const longword evcDevClientRejected     = 0x00000010;
const longword evcDevClientNoRoom       = 0x00000020;
const longword evcDevClientException    = 0x00000040;
const longword evcDevClientDisconnected = 0x00000080;
const longword evcDevClientTerminated   = 0x00000100;
const longword evcDevClientsDropped     = 0x00000200;
const longword evcDevClientRefused      = 0x00000400;
const longword evcDevClientDisTimeout   = 0x00000800;
const longword evcPortError             = 0x00001000;
const longword evcPortReset             = 0x00002000;
const longword evcInvalidADUReceived    = 0x00004000;
const longword evcNetworkError          = 0x00008000;
const longword evcCRCError              = 0x00010000;
const longword evcInvalidFunction       = 0x00020000;
// Functions     
const longword evcReadCoils             = 0x00100000;
const longword evcReadDiscrInputs       = 0x00200000; 
const longword evcReadHoldingRegs       = 0x00400000; 
const longword evcReadInputRegs         = 0x00500000; 
const longword evcWriteSingleCoil       = 0x00600000; 
const longword evcWriteSingleReg        = 0x00700000; 
const longword evcReadExcpStatus        = 0x00800000;
const longword evcDiagnostics           = 0x00900000;
const longword evcGetCommEvtCnt         = 0x00A00000;
const longword evcGetCommEvtLog         = 0x00B00000;
const longword evcWriteMultiCoils       = 0x00C00000;
const longword evcWriteMultiRegs        = 0x00D00000;
const longword evcReportServerID        = 0x00E00000;
const longword evcReadFileRecord        = 0x00F00000;
const longword evcWriteFileRecord       = 0x01000000;
const longword evcMaskWriteReg          = 0x01100000; 
const longword evcReadWriteMultiRegs    = 0x01200000; 
const longword evcReadFifoQueue         = 0x01300000;
const longword evcEncIntTransport       = 0x01400000;
const longword evcCustomFunction        = 0x01500000;
const longword evcPassthrough           = 0x01600000;

// Event Result
const word evrNoError                   = 0x0000;
// <-- From 0x0001 to 0x0011 see ModbusErrors

//******************************************************************************
// Customization Parameters index
//******************************************************************************
                                     
const int par_TCP_UDP_Port      = 1; 
const int par_DeviceID          = 2; 
const int par_TcpPersistence    = 3; 
const int par_DisconnectOnError = 4; 
const int par_SendTimeout       = 5; 
const int par_SerialFormat      = 6; 
const int par_AutoTimeout       = 7; 
const int par_AutoTimeLimitMin  = 8; 
const int par_FixedTimeout      = 9; 
const int par_BaseAddress       = 10;
const int par_DevPeerListMode   = 11;
const int par_PacketLog         = 12;
const int par_InterframeDelay   = 13;
const int par_WorkInterval      = 14;
const int par_AllowSerFunOnEth  = 15;
const int par_MaxRetries        = 16;
const int par_DisconnectTimeout = 17;
const int par_AttemptSleep      = 18;
const int par_DevicePassthrough = 19;

//******************************************************************************
//  Default Values used on objects creation                                                
//******************************************************************************

#define def_NETPort              502    // Ethernet TCP or UDP
#define def_NETPersistence       true
#define def_NETDisconnectOnError true 
#define def_SERDisconnectOnError true
#define def_SerialFormat         FormatRTU
#define def_InterframeDelay      20
#define def_AutoTimeout          true
#define def_AutoTimeLimit_Min    300
#define def_FixedTimeout_ms      3000
#define def_BaseAddressZero      false  // i.e. BaseAddress = 1
#define def_TCPSndTimeout        200
#define def_SERSndTimeout        500
#define def_AcceptBroadcast      false
#define def_WorkInterval         100
#define def_PacketLog            PacketLog_BOTH
#define def_AllowSerFunOnEth     false // Allow Serial functions callback on ethernet devices
#define def_MaxRetries_net       2
#define def_MaxRetries_rtu       1
#define def_AttemptSleep         300
#define def_DisconnectTimeout    0

// Forwards for FieldController
class TMBEthernetClient;
typedef TMBEthernetClient* PMBEthernetClient;

class TMBSerController;
typedef TMBSerController* PMBSerController;

// Broker kind

const int BrokerKind_bkUnknown       = 0;
const int BrokerKind_bkEthClient     = 1;
const int BrokerKind_bkSerController = 2;

// Device Parameters struct ------------------------------------
//                                       RANGE         SCOPE       
typedef struct {                 //-----------------------------
    int Kind;                    //   Broker Kind
    PMBEthernetClient Client;    //  Client Object    NET
    PMBSerController Controller; //  Controller idx   SER
    int SerialFormat;            //    RTU | ASCII    SER
    bool AutoTimeout;            //   true | false    NET, SER    
    longword AutoTimeLimit_Min;  //      > 300        NET, SER    
    longword FixedTimeout_ms;    //      > 300        NET, SER    
    int BaseAddress;             //      1 | 0        NET, SER   
    bool AcceptBroadcast;        //   true | false    NET
    // Runtime values, not to be set
    longword TimeMin;
    longword TimeMax;
    longword AutoTimeCalc_ms;    // Calculated timeout
}TDeviceParams;
typedef TDeviceParams* PDeviceParams;

typedef struct {                 
    int Kind;                    //  Broker Kind
    PMBEthernetClient Client;    //  Client 
    PMBSerController Controller; //  Controller 
}TFieldDevice;

#define _StatusUnknown     0
#define _StatusOk          1  // Ok
#define _StatusTimeout     2  // Timeout        
#define _StatusError       3  // Hardware error
#define _StatusProtoError  4  // Protocol error

// Device Status (used by Brokers)
typedef struct {
    int32_t LastError;
    int32_t Status;
    int32_t Connected;
    uint32_t JobTime;
}TDeviceStatus;
typedef TDeviceStatus* PDeviceStatus;

// Device Info (used by Devices)
typedef struct {
    int32_t Running;    
    int32_t ClientsCount;   // only for TCP
    int32_t ClientsBlocked; // only for TCP/UDP
    int32_t LastError;
}TDeviceInfo;
typedef TDeviceInfo* PDeviceInfo;

// Binary Nibble to ASCII
const char Ascii[16] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F' };

#pragma pack()

#define assigned(a)  (a!=NULL)


#endif // mb_defines_h

