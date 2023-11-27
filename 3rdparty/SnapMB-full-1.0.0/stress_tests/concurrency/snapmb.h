#ifndef snapmb_h
#define snapmb_h

//---------------------------------------------------------------------------
#if defined (_WIN32)|| defined(_WIN64)|| defined(__WIN32__) || defined(__WINDOWS__)
# define SNAP_OS_WINDOWS
#endif

// Linux, BSD and Solaris define "unix", OSX doesn't, even though it derives from BSD
#if defined(unix) || defined(__unix__) || defined(__unix)
# define PLATFORM_UNIX
#endif

#if defined(__NetBSD__) || defined (__FreeBSD__) || defined(__OpenBSD__) || defined(__bsdi__) || defined(__DragonFly__)
# define OS_BSD
#endif

// Specific Linux
#if defined (__linux__)
#define OS_LINUX
#endif

#if __APPLE__
# define OS_OSX
#endif

#if defined(PLATFORM_UNIX)
# include <unistd.h>
# include <sys/param.h>
# if defined(_POSIX_VERSION)
#   define POSIX
# endif
#endif

#ifdef OS_OSX
# include <unistd.h>
#endif

#if (!defined (SNAP_OS_WINDOWS)) && (!defined(PLATFORM_UNIX)) && (!defined(OS_BSD)) && (!defined(OS_OSX))
# error platform still unsupported (please add it yourself and report ;-)
#endif

#include <stdint.h>
#include <time.h>
#include <cstring>
#include <stdlib.h>

#ifdef SNAP_OS_WINDOWS
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
#endif

#if defined(PLATFORM_UNIX) || defined(OS_OSX)
# include <errno.h>
# include <sys/time.h>
# include <sys/socket.h>
# include <arpa/inet.h>
# include <netinet/tcp.h>
# include <netinet/in.h>
# include <sys/ioctl.h>
#endif

#if defined(PLATFORM_UNIX) || defined(OS_OSX) || defined (OS_BSD)
# include <termios.h>
# include <fcntl.h>
# include <sys/types.h>
# include <sys/stat.h>
//# include <sys/limits.h>
# include <sys/file.h>
#endif

#ifdef SNAP_OS_WINDOWS
# define SNAP_API __stdcall
#else
# define SNAP_API
#endif

// Exact length types regardless of platform/processor
// We absolute need of them, all structs have an exact size that
// must be the same across the processor used 32/64 bit

// *Use them* if you change/expand the code and avoid long, u_long and so on...

typedef uint8_t    byte;
typedef uint16_t   word;
typedef uint32_t   longword;
typedef byte* pbyte;
typedef word* pword;
typedef uintptr_t  snap_obj; // multi platform/processor object reference

#ifndef SNAP_OS_WINDOWS
# define INFINITE  0XFFFFFFFF
#endif

//---------------------------------------------------------------------------
// C++ Library
//---------------------------------------------------------------------------
#ifdef __cplusplus
#include <string>
#include <time.h>

#include <stdint.h>

extern "C" {
#endif

//---------------------------------------------------------------------------
// C exact length types
//---------------------------------------------------------------------------
#ifndef __cplusplus

#ifdef OS_BSD
#  include <stdint.h>
#  include <time.h>
#endif

#ifdef OS_OSX
#  include <stdint.h>  
#  include <time.h>
#endif

#ifdef OS_SOLARIS
#  include <stdint.h>  
#  include <time.h>
#endif

#if defined(_UINTPTR_T_DEFINED)
#  include <stdint.h>
#  include <time.h>
#endif

#if !defined(_UINTPTR_T_DEFINED) && !defined(OS_SOLARIS) && !defined(OS_BSD) && !defined(OS_OSX)
typedef unsigned char   uint8_t;  //  8 bit unsigned integer
typedef unsigned short  uint16_t; // 16 bit unsigned integer
typedef unsigned int    uint32_t; // 32 bit unsigned integer
typedef unsigned long   uintptr_t;// 64 bit unsigned integer
#endif

#endif

#pragma pack(1)

#define mbNoError 0

typedef struct {
	uintptr_t Object;
	uintptr_t Selector;
}XOBJECT;

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

//==============================================================================
//  MODBUS FLAVOURS
//==============================================================================

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

//==============================================================================
// Customization Parameters
//------------------------------------------------------------------------------
// Using the SetLocalParam() multiplexer function it's possible to change these
// parameters. This is a convenient way to add/remove params in future without 
// changing the prototypes of the functions. Moreover we avoid a huge list of 
// parameters into setup functions.
//==============================================================================

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
const int par_IsGateway         = 11;
const int par_DevPeerListMode   = 12;
const int par_PacketLog         = 13;
const int par_InterframeDelay   = 14;
const int par_WorkInterval      = 15;
const int par_AllowSerFunOnEth  = 16;
const int par_MaxRetries        = 17;
const int par_DisconnectTimeout = 18;
const int par_AttemptSleep      = 19;
const int par_DevicePassthrough = 20;

//******************************************************************************
// ERRORS
//------------------------------------------------------------------------------
// The Error is a "record" composed by some fields which are chained
//******************************************************************************
#define mbNoError 0

const int MaskObjects                 = 0xF0000000;
const int MaskCategory                = 0x000F0000;
const int MaskErrNo                   = 0x0000FFFF;

// Base Objects
const int errLibrary                  = 0x10000000;
const int errSerialClient             = 0x20000000;
const int errEthernetClient           = 0x30000000;
const int errFieldController          = 0x40000000;
const int errSerialDevice             = 0x50000000;
const int errEthernetDevice           = 0x60000000;

// Library Errors
const int errCategoryLibrary          = 0x00010000;
const int errObjectInvalidMethod      = 0x00000001;

// Serial Errors
const int errCategorySerialSocket     = 0x00020000;
const int errPortInvalidParams        = 0x00000001;
const int errPortSettingsTimeouts     = 0x00000002;
const int errPortSettingsParams       = 0x00000003;
const int errOpeningPort              = 0x00000004;
const int errPortReadTimeout          = 0x00000005;
const int errPortWriteTimeout         = 0x00000006;
const int errPortReadError            = 0x00000007;
const int errPortWriteError           = 0x00000008;
const int errBufferOverflow           = 0x00000009;
const int errPortGetParams            = 0x0000000A;
const int errPortLocked               = 0x0000000B;
const int errInterframe               = 0x0000000C;

const int errCategoryNetSocket        = 0x00030000; // TCP-UDP Error
// TCP-UDP/IP Socket Errors           = 0x0000XXXX  // Last 2 byte will contain the Socket Layer error

// Modbus Protocol Errors 
const int errCategoryMBProtocol       = 0x00040000; // Protocol error (0x8X received)
const int errIllegalFunction          = 0x00000001; // Exception Code 0x01
const int errIllegalDataAddress       = 0x00000002; // Exception Code 0x02
const int errIllegalDataValue         = 0x00000003; // Exception Code 0x03
const int errSlaveDeviceFailure       = 0x00000004; // Exception Code 0x04
const int errAcknowledge              = 0x00000005; // Exception Code 0x05
const int errSlaveDeviceBusy          = 0x00000006; // Exception Code 0x06
const int errNegativeAcknowledge      = 0x00000007; // Exception Code 0x07
const int errMemoryParityError        = 0x00000008; // Exception Code 0x08
const int errGatewayPathUnavailable   = 0x00000010; // Exception Code 0x10
const int errGatewayTargetFailed      = 0x00000011; // Exception Code 0x11

// Process Errors
const int errCategoryProcess          = 0x00050000;
const int errInvalidBroadcastFunction = 0x00000001;
const int errInvalidParamIndex        = 0x00000002;
const int errInvalidAddress           = 0x00000003;
const int errInvalidDataAmount        = 0x00000004;
const int errInvalidADUReceived       = 0x00000005;
const int errInvalidChecksum          = 0x00000006;
const int errTimeout                  = 0x00000007;
const int errInvalidDeviceID          = 0x00000008;
const int errInvalidUserFunction      = 0x00000009;
const int errInvalidReqForThisObject  = 0x0000000A;
// Field Controller errors
const int errUndefinedBroker          = 0x0000000B;
const int errUndefinedClient          = 0x0000000C;
const int errDeviceIDZero             = 0x0000000D;
const int errDeviceAlreadyExists      = 0x0000000E;
const int errUndefinedController      = 0x0000000F;
const int errSomeConnectionsError     = 0x00000010;
const int errCommParamsMismatch       = 0x00000011;

// 0x00000012..0x000000FF : available

// Device errors
const int errDevUnknownAreaID         = 0x00000100; // Unknown Area ID
const int errDevAreaZero              = 0x00000101; // Area Amount = 0
const int errDevAreaTooWide           = 0x00000102; // Area Amount too wide 
const int errDevUnknownCallbackID     = 0x00000103; // Unknown Callback ID
const int errDevInvalidParams         = 0x00000104; // Invalid param(s) supplied
const int errDevInvalidParamIndex     = 0x00000105; // Invalid param (GetDeviceParam())
const int errDevOpNotAllowed          = 0x00000106; // Cannot change because running
const int errDevTooManyPeers          = 0x00000107; // To many Peers for Deny/AcceptList (only TCP)
const int errDevCannotRebindOnRun     = 0x00000108; // Device is running, stop first

//==============================================================================
// polymorphic Broker
//==============================================================================

// Buffer Kind  
const int BkSnd = 0;   // Buffer Sent (see GetBufferSent/GetBufferSentPtr)
const int BkRcv = 1;   // Buffer Recv (see GetBufferRecv/GetBufferRecvPtr)

void SNAP_API broker_CreateFieldController(XOBJECT& Broker);
void SNAP_API broker_CreateEthernetClient(XOBJECT& Broker, int Proto, const char* Address, int Port);
void SNAP_API broker_CreateSerialClient(XOBJECT& Broker, int Format, const char* PortName, int BaudRate, char Parity, int DataBits, int Stops, int Flow);
void SNAP_API broker_Destroy(XOBJECT& Broker);
int SNAP_API broker_Connect(XOBJECT& Broker);
int SNAP_API broker_Disconnect(XOBJECT& Broker);
int SNAP_API broker_AddControllerNetDevice(XOBJECT& Broker, int Proto, byte DeviceID, const char* Address, int Port);
int SNAP_API broker_AddControllerSerDevice(XOBJECT& Broker, int Format, byte DeviceID, const char* PortName, int BaudRate, char Parity, int DataBits, int Stops, int Flow);
int SNAP_API broker_SetLocalParam(XOBJECT& Broker, byte LocalID, int ParamIndex, int Value);
int SNAP_API broker_SetRemoteDeviceParam(XOBJECT& Broker, byte DeviceID, int ParamIndex, int Value);
int SNAP_API broker_GetIOBufferPtr(XOBJECT& Broker, byte DeviceID, int BufferKind, pbyte& Data);
int SNAP_API broker_GetIOBuffer(XOBJECT& Broker, byte DeviceID, int BufferKind, pbyte Data);
int SNAP_API broker_GetDeviceStatus(XOBJECT& Broker, byte DeviceID, TDeviceStatus& DeviceStatus);
// Modbus functions
int SNAP_API broker_ReadHoldingRegisters(XOBJECT& Broker, byte DeviceID, word Address, word Amount, void* pUsrData);
int SNAP_API broker_WriteMultipleRegisters(XOBJECT& Broker, byte DeviceID, word Address, word Amount, void* pUsrData);
int SNAP_API broker_ReadCoils(XOBJECT& Broker, byte DeviceID, word Address, word Amount, void* pUsrData);
int SNAP_API broker_ReadDiscreteInputs(XOBJECT& Broker, byte DeviceID, word Address, word Amount, void* pUsrData);
int SNAP_API broker_ReadInputRegisters(XOBJECT& Broker, byte DeviceID, word Address, word Amount, void* pUsrData);
int SNAP_API broker_WriteSingleCoil(XOBJECT& Broker, byte DeviceID, word Address, word Value);
int SNAP_API broker_WriteSingleRegister(XOBJECT& Broker, byte DeviceID, word Address, word Value);
int SNAP_API broker_ReadWriteMultipleRegisters(XOBJECT& Broker, byte DeviceID, word RDAddress, word RDAmount, word WRAddress, word WRAmount, void* pRDUsrData, void* pWRUsrData);
int SNAP_API broker_WriteMultipleCoils(XOBJECT& Broker, byte DeviceID, word Address, word Amount, void* pUsrData);
int SNAP_API broker_MaskWriteRegister(XOBJECT& Broker, byte DeviceID, word Address, word AND_Mask, word OR_Mask);
int SNAP_API broker_ReadFileRecord(XOBJECT& Broker, byte DeviceID, byte RefType, word FileNumber, word RecNumber, word RegsAmount, void* RecData);
int SNAP_API broker_WriteFileRecord(XOBJECT& Broker, byte DeviceID, byte RefType, word FileNumber, word RecNumber, word RegsAmount, void* RecData);
int SNAP_API broker_ReadFIFOQueue(XOBJECT& Broker, byte DeviceID, word Address, word& FifoCount, void* FIFO);
int SNAP_API broker_ReadExceptionStatus(XOBJECT& Broker, byte DeviceID, byte& Data);
int SNAP_API broker_Diagnostics(XOBJECT& Broker, byte DeviceID, word SubFunction, void* pSendData, void* pRecvData, word ItemsToSend, word& ItemsReceived);
int SNAP_API broker_GetCommEventCounter(XOBJECT& Broker, byte DeviceID, word& Status, word& EventCount);
int SNAP_API broker_GetCommEventLog(XOBJECT& Broker, byte DeviceID, word& Status, word& EventCount, word& MessageCount, word& NumItems, void* Events);
int SNAP_API broker_ReportServerID(XOBJECT& Broker, byte DeviceID, void* pUsrData, int& DataSize);
int SNAP_API broker_ExecuteMEIFunction(XOBJECT& Broker, byte DeviceID, byte MEI_Type, void* pWRUsrData, word WRSize, void* pRDUsrData, word& RDSize);
int SNAP_API broker_CustomFunctionRequest(XOBJECT& Broker, byte DeviceID, byte UsrFunction, void* pUsrPDUWrite, word SizePDUWrite, void* pUsrPDURead, word& SizePDURead, word SizePDUExpected);
int SNAP_API broker_RawRequest(XOBJECT& Broker, byte DeviceID, void* pUsrPDUWrite, word SizePDUWrite, void* pUsrPDURead, word& SizePDURead, word SizePDUExpected);

//==============================================================================
// polymorphic Device
//==============================================================================

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

// Callbacks Actions  
const int cbActionRead           = 0;
const int cbActionWrite          = 1;

const int PacketLog_NONE         = 0;
const int PacketLog_IN           = 1;
const int PacketLog_OUT          = 2;
const int PacketLog_BOTH         = 3;

typedef struct {
	time_t EvtTime;    // Timestamp
	int EvtSender;     // Sender
	longword EvtCode;  // Event code
	word EvtRetCode;   // Event result
	word EvtParam1;    // Param 1 (if available)
	word EvtParam2;    // Param 2 (if available)
	word EvtParam3;    // Param 3 (if available)
	word EvtParam4;    // Param 4 (if available)
}TSrvEvent, * PSrvEvent;

// Device Base Events
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
// Functions events     
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

// Callbacks prototypes
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

void SNAP_API device_CreateEthernet(XOBJECT& Device, int Proto, byte DeviceID, const char* Address, int Port);
void SNAP_API device_CreateSerial(XOBJECT& Device, int Format, byte DeviceID, const char* PortName, int BaudRate, char Parity, int DataBits, int Stops, int Flow);
void SNAP_API device_Destroy(XOBJECT& Device);
int SNAP_API device_SetParam(XOBJECT& Device, int ParamIndex, int Value);
int SNAP_API device_BindEthernet(XOBJECT& Device, byte DeviceID, const char* Address, int Port);
int SNAP_API device_BindSerial(XOBJECT& Device, byte DeviceID, const char* PortName, int BaudRate, char Parity, int DataBits, int Stops, int Flow);
int SNAP_API device_GetSerialInterframe(XOBJECT& Device, int& InterframeDelay, int& MaxInterframeDetected);
int SNAP_API device_SetUserFunction(XOBJECT& Device, byte FunctionID, int Value);
int SNAP_API device_Start(XOBJECT& Device);
int SNAP_API device_Stop(XOBJECT& Device);
int SNAP_API device_AddPeer(XOBJECT& Device, const char* Address);
int SNAP_API device_RegisterArea(XOBJECT& Device, int AreaID, void* Data, int Amount);
int SNAP_API device_CopyArea(XOBJECT& Device, int AreaID, word Address, word Amount, void* Data, int CopyMode);
int SNAP_API device_LockArea(XOBJECT& Device, int AreaID);
int SNAP_API device_UnlockArea(XOBJECT& Device, int AreaID);
int SNAP_API device_RegisterCallback(XOBJECT& Device, int CallbackID, void* cbRequest, void* UsrPtr);
int SNAP_API device_PickEvent(XOBJECT& Device, void* pEvent);
int SNAP_API device_PickEventAsText(XOBJECT& Device, char* Text, int TextSize);
int SNAP_API device_GetDeviceInfo(XOBJECT& Device, TDeviceInfo& DeviceInfo);

//==============================================================================
// Misc                      
//==============================================================================
const char* SNAP_API ErrorText(int Error, char* Text, int TextSize);
const char* SNAP_API EventText(void* Event, char* Text, int TextSize);

#pragma pack()

#ifdef __cplusplus
}
#endif // __cplusplus

#ifdef __cplusplus

// classi

class TSnapMBBroker
{
private:
	XOBJECT Broker;
	void Clear();
public:
	int LastError;
	bool Connected;
	longword JobTime;
	TSnapMBBroker();
	TSnapMBBroker(int Proto, const char* Address, int Port);
	TSnapMBBroker(int Format, const char* PortName, int BaudRate, char Parity, int DataBits, int Stops, int Flow);
	virtual ~TSnapMBBroker();
	//-----------------------------------------------------------------------------
	// Dynamic recreation       
	//-----------------------------------------------------------------------------
	void ChangeTo();
	void ChangeTo(int Proto, const char* Address, int Port);
	void ChangeTo(int Format, const char* PortName, int BaudRate, char Parity, int DataBits, int Stops, int Flow);
	//-----------------------------------------------------------------------------
	// Connection and Params   
	//-----------------------------------------------------------------------------
	int Connect();
	void Disconnect(); // DeviceID used by NetController
	int SetLocalParam(byte DeviceID, int ParamIndex, int Value);
	int SetRemoteDeviceParam(byte DeviceID, int ParamIndex, int Value);
	//-----------------------------------------------------------------------------
	// Field Controller specific
	//-----------------------------------------------------------------------------
	int AddDevice(int Proto, byte DeviceID, const char* Address, int Port);
	int AddDevice(int Format, byte DeviceID, const char* PortName, int BaudRate, char Parity, int DataBits, int Stops, int Flow);
	//-----------------------------------------------------------------------------
	// Debug & Status            
	//-----------------------------------------------------------------------------
	int GetIOBufferPtr(byte DeviceID, int BufferKind, pbyte& Data);
	int GetIOBuffer(byte DeviceID, int BufferKind, pbyte Data);
	int GetDeviceStatus(byte DeviceID, TDeviceStatus& DeviceStatus);
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
	int Diagnostics(byte DeviceID, word SubFunction, void* pSendData, void* pRecvData, word ItemsToSend, word& ItemsReceived);
	int GetCommEventCounter(byte DeviceID, word& Status, word& EventCount);
	int GetCommEventLog(byte DeviceID, word& Status, word& EventCount, word& MessageCount, word& NumItems, void* Events);
	int ReportServerID(byte DeviceID, void* pUsrData, int& DataSize);
	//-----------------------------------------------------------------------------
	// Modbus Specialized Functions
	//-----------------------------------------------------------------------------
	int ExecuteMEIFunction(byte DeviceID, byte MEI_Type, void* pWRUsrData, word WRSize, void* pRDUsrData, word& RDSize);
	//-----------------------------------------------------------------------------
	// User Functions
	//-----------------------------------------------------------------------------
	int CustomFunctionRequest(byte DeviceID, byte UsrFunction, void* pUsrPDUWrite, word SizePDUWrite, void* pUsrPDURead, word& SizePDURead, word SizePDUExpected);
	int CustomFunctionRequest(byte DeviceID, byte UsrFunction, void* pUsrPDUWrite, word SizePDUWrite, void* pUsrPDURead, word& SizePDURead);
	int RawRequest(byte DeviceID, void* pUsrPDUWrite, word SizePDUWrite, void* pUsrPDURead, word& SizePDURead, word SizePDUExpected);
	int RawRequest(byte DeviceID, void* pUsrPDUWrite, word SizePDUWrite, void* pUsrPDURead, word& SizePDURead);

};
typedef TSnapMBBroker* PSnapMBBroker;

class TSnapMBDevice
{
private:
	XOBJECT Device;
public:
	TSnapMBDevice(int Proto, byte DeviceID, const char* Address, int Port);
	TSnapMBDevice(int Format, byte DeviceID, const char* PortName, int BaudRate, char Parity, int DataBits, int Stops, int Flow);
    ~TSnapMBDevice();
	int Bind(byte DeviceID, const char* Address, int Port);
	int Bind(byte DeviceID, const char* PortName, int BaudRate, char Parity, int DataBits, int Stops, int Flow);
	int RegisterArea(int AreaID, void* Data, int Amount);
	int CopyArea(int AreaID, word Address, word Amount, void* Data, int CopyMode);
	int LockArea(int AreaID);
	int UnlockArea(int AreaID);
	int RegisterCallback(int CallbackID, void* cbRequest, void* UsrPtr);
	int Start();
	int Stop();
	int SetParam(int ParamIndex, int Value);
	int SetUserFunction(byte FunctionID, bool Value);
	int AddPeer(const char* Address);
	int GetDeviceInfo(byte DeviceID, TDeviceInfo& DeviceInfo);
	bool PickEvent(void* pEvent);
	bool PickEventAsText(char* Text, int TextSize);
};
typedef TSnapMBDevice* PSnapMBDevice;


#endif



#endif  // snapmb_h
