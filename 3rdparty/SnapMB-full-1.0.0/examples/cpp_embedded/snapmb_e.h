#ifndef snapmb_e_h
#define snapmb_e_h

#include "mb_libinterface.h"

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
	int SetCustomFunction(byte FunctionID, bool Value);
	int AddPeer(const char* Address);
	int GetDeviceInfo(TDeviceInfo& DeviceInfo);
	bool PickEvent(void* pEvent);
	bool PickEventAsText(char* Text, int TextSize);
};
typedef TSnapMBDevice* PSnapMBDevice;

#endif // __cplusplus


#endif  // snapmb_e_h
