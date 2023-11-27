#include "snapmb_e.h"

//*****************************************************************************
// BROKER                          
//*****************************************************************************
void TSnapMBBroker::Clear()
{
	Connected = false;
	JobTime = 0;
	LastError = 0;
}
//-----------------------------------------------------------------------------
// FieldController 
//-----------------------------------------------------------------------------
TSnapMBBroker::TSnapMBBroker()
{
	broker_CreateFieldController(Broker);
	Clear();
}
//-----------------------------------------------------------------------------
// Ethernet Client 
//-----------------------------------------------------------------------------
TSnapMBBroker::TSnapMBBroker(int Proto, const char* Address, int Port)
{
	broker_CreateEthernetClient(Broker, Proto, Address, Port);
	Clear();
}
//-----------------------------------------------------------------------------
// Serial Client 
//-----------------------------------------------------------------------------
TSnapMBBroker::TSnapMBBroker(int Format, const char* PortName, int BaudRate, char Parity, int DataBits, int Stops, int Flow)
{
	broker_CreateSerialClient(Broker, Format, PortName, BaudRate, Parity, DataBits, Stops, Flow);
	Clear();
}
//-----------------------------------------------------------------------------
TSnapMBBroker::~TSnapMBBroker()
{
	broker_Destroy(Broker);
}
//-----------------------------------------------------------------------------
void TSnapMBBroker::ChangeTo()
{
	if (Broker.Object)
		broker_Destroy(Broker);
	broker_CreateFieldController(Broker);
	Clear();
}
//-----------------------------------------------------------------------------
void TSnapMBBroker::ChangeTo(int Proto, const char* Address, int Port)
{
	if (Broker.Object)
		broker_Destroy(Broker);
	broker_CreateEthernetClient(Broker, Proto, Address, Port);
	Clear();
}
//-----------------------------------------------------------------------------
void TSnapMBBroker::ChangeTo(int Format, const char* PortName, int BaudRate, char Parity, int DataBits, int Stops, int Flow)
{
	if (Broker.Object)
		broker_Destroy(Broker);
	broker_CreateSerialClient(Broker, Format, PortName, BaudRate, Parity, DataBits, Stops, Flow);
	Clear();
}
//-----------------------------------------------------------------------------
int TSnapMBBroker::Connect()
{
	return broker_Connect(Broker);
}
//-----------------------------------------------------------------------------
void TSnapMBBroker::Disconnect()
{
	broker_Disconnect(Broker);
}
//-----------------------------------------------------------------------------
int TSnapMBBroker::SetLocalParam(byte DeviceID, int ParamIndex, int Value)
{
	return broker_SetLocalParam(Broker, DeviceID, ParamIndex, Value);
}
//-----------------------------------------------------------------------------
int TSnapMBBroker::SetRemoteDeviceParam(byte DeviceID, int ParamIndex, int Value)
{
	return broker_SetRemoteDeviceParam(Broker, DeviceID, ParamIndex, Value);
}
//-----------------------------------------------------------------------------
int TSnapMBBroker::AddDevice(int Proto, byte DeviceID, const char* Address, int Port)
{
	return broker_AddControllerNetDevice(Broker, Proto, DeviceID, Address, Port);
}
//-----------------------------------------------------------------------------
int TSnapMBBroker::AddDevice(int Format, byte DeviceID, const char* PortName, int BaudRate, char Parity, int DataBits, int Stops, int Flow)
{
	return broker_AddControllerSerDevice(Broker, Format, DeviceID, PortName, BaudRate, Parity, DataBits, Stops, Flow);
}
//-----------------------------------------------------------------------------
int TSnapMBBroker::GetIOBufferPtr(byte DeviceID, int BufferKind, pbyte& Data)
{
	return broker_GetIOBufferPtr(Broker, DeviceID, BufferKind, Data);
}
//-----------------------------------------------------------------------------
int TSnapMBBroker::GetIOBuffer(byte DeviceID, int BufferKind, pbyte Data)
{
	return broker_GetIOBuffer(Broker, DeviceID, BufferKind, Data);
}
//-----------------------------------------------------------------------------
int TSnapMBBroker::GetDeviceStatus(byte DeviceID, TDeviceStatus& DeviceStatus)
{
	return broker_GetDeviceStatus(Broker, DeviceID, DeviceStatus);
}
//-----------------------------------------------------------------------------
int TSnapMBBroker::ReadHoldingRegisters(byte DeviceID, word Address, word Amount, void* pUsrData)
{
	return broker_ReadHoldingRegisters(Broker, DeviceID, Address, Amount, pUsrData);
}
//-----------------------------------------------------------------------------
int TSnapMBBroker::WriteMultipleRegisters(byte DeviceID, word Address, word Amount, void* pUsrData)
{
	return broker_WriteMultipleRegisters(Broker, DeviceID, Address, Amount, pUsrData);
}
//-----------------------------------------------------------------------------
int TSnapMBBroker::ReadCoils(byte DeviceID, word Address, word Amount, void* pUsrData)
{
	return broker_ReadCoils(Broker, DeviceID, Address, Amount, pUsrData);
}
//-----------------------------------------------------------------------------
int TSnapMBBroker::ReadDiscreteInputs(byte DeviceID, word Address, word Amount, void* pUsrData)
{
	return broker_ReadDiscreteInputs(Broker, DeviceID, Address, Amount, pUsrData);
}
//-----------------------------------------------------------------------------
int TSnapMBBroker::ReadInputRegisters(byte DeviceID, word Address, word Amount, void* pUsrData)
{
	return broker_ReadInputRegisters(Broker, DeviceID, Address, Amount, pUsrData);
}
//-----------------------------------------------------------------------------
int TSnapMBBroker::WriteSingleCoil(byte DeviceID, word Address, word Value)
{
	return broker_WriteSingleCoil(Broker, DeviceID, Address, Value);
}
//-----------------------------------------------------------------------------
int TSnapMBBroker::WriteSingleRegister(byte DeviceID, word Address, word Value)
{
	return broker_WriteSingleRegister(Broker, DeviceID, Address, Value);
}
//-----------------------------------------------------------------------------
int TSnapMBBroker::ReadWriteMultipleRegisters(byte DeviceID, word RDAddress, word RDAmount, word WRAddress, word WRAmount, void* pRDUsrData, void* pWRUsrData)
{
	return broker_ReadWriteMultipleRegisters(Broker, DeviceID, RDAddress, RDAmount, WRAddress, WRAmount, pRDUsrData, pWRUsrData);
}
//-----------------------------------------------------------------------------
int TSnapMBBroker::WriteMultipleCoils(byte DeviceID, word Address, word Amount, void* pUsrData)
{
	return broker_WriteMultipleCoils(Broker, DeviceID, Address, Amount, pUsrData);
}
//-----------------------------------------------------------------------------
int TSnapMBBroker::MaskWriteRegister(byte DeviceID, word Address, word AND_Mask, word OR_Mask)
{
	return broker_MaskWriteRegister(Broker, DeviceID, Address, AND_Mask, OR_Mask);
}
//-----------------------------------------------------------------------------
int TSnapMBBroker::ReadFileRecord(byte DeviceID, byte RefType, word FileNumber, word RecNumber, word RegsAmount, void* RecData)
{
	return broker_ReadFileRecord(Broker, DeviceID, RefType, FileNumber, RecNumber, RegsAmount, RecData);
}
//-----------------------------------------------------------------------------
int TSnapMBBroker::WriteFileRecord(byte DeviceID, byte RefType, word FileNumber, word RecNumber, word RegsAmount, void* RecData)
{
	return broker_WriteFileRecord(Broker, DeviceID, RefType, FileNumber, RecNumber, RegsAmount, RecData);
}
//-----------------------------------------------------------------------------
int TSnapMBBroker::ReadFIFOQueue(byte DeviceID, word Address, word& FifoCount, void* FIFO)
{
	return broker_ReadFIFOQueue(Broker, DeviceID, Address, FifoCount, FIFO);
}
//-----------------------------------------------------------------------------
int TSnapMBBroker::ReadExceptionStatus(byte DeviceID, byte& Data)
{
	return broker_ReadExceptionStatus(Broker, DeviceID, Data);
}
//-----------------------------------------------------------------------------
int TSnapMBBroker::Diagnostics(byte DeviceID, word SubFunction, void* pSendData, void* pRecvData, word ItemsToSend, word& ItemsReceived)
{
	return broker_Diagnostics(Broker, DeviceID, SubFunction, pSendData, pRecvData, ItemsToSend, ItemsReceived);
}
//-----------------------------------------------------------------------------
int TSnapMBBroker::GetCommEventCounter(byte DeviceID, word& Status, word& EventCount)
{
	return broker_GetCommEventCounter(Broker, DeviceID, Status, EventCount);
}
//-----------------------------------------------------------------------------
int TSnapMBBroker::GetCommEventLog(byte DeviceID, word& Status, word& EventCount, word& MessageCount, word& NumItems, void* Events)
{
	return broker_GetCommEventLog(Broker, DeviceID, Status, EventCount, MessageCount, NumItems, Events);
}
//-----------------------------------------------------------------------------
int TSnapMBBroker::ReportServerID(byte DeviceID, void* pUsrData, int& DataSize)
{
	return broker_ReportServerID(Broker, DeviceID, pUsrData, DataSize);
}
//-----------------------------------------------------------------------------
int TSnapMBBroker::ExecuteMEIFunction(byte DeviceID, byte MEI_Type, void* pWRUsrData, word WRSize, void* pRDUsrData, word& RDSize)
{
	return broker_ExecuteMEIFunction(Broker, DeviceID, MEI_Type, pWRUsrData, WRSize, pRDUsrData, RDSize);
}
//-----------------------------------------------------------------------------
int TSnapMBBroker::CustomFunctionRequest(byte DeviceID, byte UsrFunction, void* pUsrPDUWrite, word SizePDUWrite, void* pUsrPDURead, word& SizePDURead, word SizePDUExpected)
{
	return broker_CustomFunctionRequest(Broker, DeviceID, UsrFunction, pUsrPDUWrite, SizePDUWrite, pUsrPDURead, SizePDURead, SizePDUExpected);
}
//-----------------------------------------------------------------------------
int TSnapMBBroker::CustomFunctionRequest(byte DeviceID, byte UsrFunction, void* pUsrPDUWrite, word SizePDUWrite, void* pUsrPDURead, word& SizePDURead)
{
	return broker_CustomFunctionRequest(Broker, DeviceID, UsrFunction, pUsrPDUWrite, SizePDUWrite, pUsrPDURead, SizePDURead, 0);
}
//-----------------------------------------------------------------------------
int TSnapMBBroker::RawRequest(byte DeviceID, void* pUsrPDUWrite, word SizePDUWrite, void* pUsrPDURead, word& SizePDURead, word SizePDUExpected)
{
	return broker_RawRequest(Broker, DeviceID, pUsrPDUWrite, SizePDUWrite, pUsrPDURead, SizePDURead, SizePDUExpected);
}
//-----------------------------------------------------------------------------
int TSnapMBBroker::RawRequest(byte DeviceID, void* pUsrPDUWrite, word SizePDUWrite, void* pUsrPDURead, word& SizePDURead)
{
	return broker_RawRequest(Broker, DeviceID, pUsrPDUWrite, SizePDUWrite, pUsrPDURead, SizePDURead, 0);
}
//*****************************************************************************
// DEVICE                          
//*****************************************************************************

TSnapMBDevice::TSnapMBDevice(int Proto, byte DeviceID, const char* Address, int Port)
{
	device_CreateEthernet(Device, Proto, DeviceID, Address, Port);
}
//-----------------------------------------------------------------------------
TSnapMBDevice::TSnapMBDevice(int Format, byte DeviceID, const char* PortName, int BaudRate, char Parity, int DataBits, int Stops, int Flow)
{
	device_CreateSerial(Device, Format, DeviceID, PortName, BaudRate, Parity, DataBits, Stops, Flow);
}
//-----------------------------------------------------------------------------
TSnapMBDevice::~TSnapMBDevice()
{
	device_Destroy(Device);
}
//-----------------------------------------------------------------------------
int TSnapMBDevice::Bind(byte DeviceID, const char* Address, int Port)
{
	return device_BindEthernet(Device, DeviceID, Address, Port);
}
//-----------------------------------------------------------------------------
int TSnapMBDevice::Bind(byte DeviceID, const char* PortName, int BaudRate, char Parity, int DataBits, int Stops, int Flow)
{
	return device_BindSerial(Device, DeviceID, PortName, BaudRate, Parity, DataBits, Stops, Flow);
}
//-----------------------------------------------------------------------------
int TSnapMBDevice::RegisterArea(int AreaID, void* Data, int Amount)
{
	return device_RegisterArea(Device, AreaID, Data, Amount);
}
//-----------------------------------------------------------------------------
int TSnapMBDevice::CopyArea(int AreaID, word Address, word Amount, void* Data, int CopyMode)
{
	return device_CopyArea(Device, AreaID, Address, Amount, Data, CopyMode);
}
//-----------------------------------------------------------------------------
int TSnapMBDevice::LockArea(int AreaID)
{
	return device_LockArea(Device, AreaID);
}
//-----------------------------------------------------------------------------
int TSnapMBDevice::UnlockArea(int AreaID)
{
	return device_UnlockArea(Device, AreaID);
}
//-----------------------------------------------------------------------------
int TSnapMBDevice::RegisterCallback(int CallbackID, void* cbRequest, void* UsrPtr)
{
	return device_RegisterCallback(Device, CallbackID, cbRequest, UsrPtr);
}
//-----------------------------------------------------------------------------
int TSnapMBDevice::Start()
{
	return device_Start(Device);
}
//-----------------------------------------------------------------------------
int TSnapMBDevice::Stop()
{
	return device_Stop(Device);	
}
//-----------------------------------------------------------------------------
int TSnapMBDevice::SetParam(int ParamIndex, int Value)
{
	return device_SetParam(Device, ParamIndex, Value);
}
//-----------------------------------------------------------------------------
int TSnapMBDevice::SetCustomFunction(byte FunctionID, bool Value)
{
	return device_SetUserFunction(Device, FunctionID, Value);
}
//-----------------------------------------------------------------------------
int TSnapMBDevice::AddPeer(const char* Address)
{
	return device_AddPeer(Device, Address);
}
//-----------------------------------------------------------------------------
int TSnapMBDevice::GetDeviceInfo(TDeviceInfo& DeviceInfo)
{
	return device_GetDeviceInfo(Device, DeviceInfo);
}
//-----------------------------------------------------------------------------
bool TSnapMBDevice::PickEvent(void* pEvent)
{
	return device_PickEvent(Device, pEvent);
}
//-----------------------------------------------------------------------------
bool TSnapMBDevice::PickEventAsText(char* Text, int TextSize)
{
	return device_PickEventAsText(Device, Text, TextSize);
}
//-----------------------------------------------------------------------------
