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
#include "mb_fieldcontroller.h"
//------------------------------------------------------------------------------
TMBFieldController::TMBFieldController()
{
	BaseObjectError = errFieldController;
	memset(FieldDevices, 0, sizeof(FieldDevices));
	memset(SerControllers, 0, sizeof(SerControllers));
	ControllersCount = 0;
	IsSerialBroker = true; // to allow broadcast
}
//------------------------------------------------------------------------------
TMBFieldController::~TMBFieldController()
{
	// Deletes Ethernet Clients
	for (int c = 0; c < MaxDevices; c++)
	{
		if (FieldDevices[c].Kind == BrokerKind_bkEthClient && FieldDevices[c].Client != NULL)
		{
			try {
				delete FieldDevices[c].Client;
			}
			catch (...) {}
		}
	}
	// Deletes Serial SerControllers 
	for (int c = 0; c < ControllersCount; c++)
	{
		if (SerControllers[c] != NULL)
		{
			try {
				delete SerControllers[c];
			}
			catch (...) {}
		}
	}
}
//------------------------------------------------------------------------------
PMBSerController TMBFieldController::FindSerController(const char* PortName)
{
	for (int c = 0; c < MaxChannels; c++)
		if (SerControllers[c] && strcmp(PortName, SerControllers[c]->PortName) == 0)
			return SerControllers[c];
	return NULL;
}
//------------------------------------------------------------------------------
bool TMBFieldController::SendRequestRecvResponse(byte DeviceID, pbyte DataIn, word& SizeIn, pbyte DataOut, word SizeOut, PPDURecvExpected PRE)
{
	// Broadcast (only serial Devices)
	if (DeviceID == 0)
	{
		for (int c = 0; c < ControllersCount; c++)
			SerControllers[c]->SendRequestRecvResponse(DeviceID, DataIn, SizeIn, DataOut, SizeOut, PRE);

		return true;
	}
	
	switch (FieldDevices[DeviceID].Kind)
	{
	case BrokerKind_bkEthClient:
		if (FieldDevices[DeviceID].Client != NULL)
			return FieldDevices[DeviceID].Client->SendRequestRecvResponse(DeviceID, DataIn, SizeIn, DataOut, SizeOut, PRE);
		else
			SetError(errCategoryProcess,errUndefinedClient);
		break;
	case BrokerKind_bkSerController:
		if (FieldDevices[DeviceID].Controller != NULL)
			return FieldDevices[DeviceID].Controller->SendRequestRecvResponse(DeviceID, DataIn, SizeIn, DataOut, SizeOut, PRE);
		else
			SetError(errCategoryProcess,errUndefinedController);
		break;
	default:
		SetError(errCategoryProcess,errUndefinedBroker);
	}
	return false;
}
//------------------------------------------------------------------------------
void TMBFieldController::SetStatus(byte DeviceID, longword Time, int JobResult)
{
	switch (FieldDevices[DeviceID].Kind)
	{
	case BrokerKind_bkEthClient:
		if (FieldDevices[DeviceID].Client != NULL)
			FieldDevices[DeviceID].Client->SetStatus(DeviceID, Time, JobResult);
		break;
	case BrokerKind_bkSerController:
		if (FieldDevices[DeviceID].Controller != NULL)
			FieldDevices[DeviceID].Controller->SetStatus(DeviceID, Time, JobResult);
		break;
	}
}
//------------------------------------------------------------------------------
void TMBFieldController::ClearErrors()
{
	LastError = 0;
}
//------------------------------------------------------------------------------
int TMBFieldController::ClearDeviceStatus(TDeviceStatus& DeviceStatus)
{
	DeviceStatus.Connected = 0;
	DeviceStatus.JobTime = 0;
	DeviceStatus.LastError = 0;
	DeviceStatus.Status = _StatusUnknown;
	return 0;
}
//------------------------------------------------------------------------------
int TMBFieldController::Connect()
{
	int Result = mbNoError;
	for (int c = 0; c < MaxDevices; c++)
	{
		if (FieldDevices[c].Kind == BrokerKind_bkEthClient && FieldDevices[c].Client != NULL)
			if (FieldDevices[c].Client->Connect() != mbNoError)
				Result = errSomeConnectionsError;

		if (FieldDevices[c].Kind == BrokerKind_bkSerController || FieldDevices[c].Controller != NULL)
			if (FieldDevices[c].Controller->Connect() != mbNoError)
				Result = errSomeConnectionsError;
	}
	return SetError(errCategoryProcess,Result);
}
//------------------------------------------------------------------------------
int TMBFieldController::GetIOBufferPtr(byte DeviceID, int BufferKind, pbyte& Data)
{
	if (DeviceID > 0)
	{
		switch (FieldDevices[DeviceID].Kind)
		{
		case BrokerKind_bkEthClient:
			if (FieldDevices[DeviceID].Client)
				return FieldDevices[DeviceID].Client->GetIOBufferPtr(BufferKind, Data);
			else
				SetError(errCategoryProcess,errUndefinedClient);
			break;
		case BrokerKind_bkSerController:
			if (FieldDevices[DeviceID].Controller)
				return FieldDevices[DeviceID].Controller->GetIOBufferPtr(BufferKind, Data);
			else
				SetError(errCategoryProcess,errUndefinedController);
			break;
		default:
			SetError(errCategoryProcess,errUndefinedBroker);
		}
	}
	return 0;
}
//------------------------------------------------------------------------------
int TMBFieldController::GetIOBuffer(byte DeviceID, int BufferKind, pbyte Data)
{
	if (DeviceID > 0)
	{
		switch (FieldDevices[DeviceID].Kind)
		{
		case BrokerKind_bkEthClient:
			if (FieldDevices[DeviceID].Client)
				return FieldDevices[DeviceID].Client->GetIOBuffer(BufferKind, Data);
			else
				SetError(errCategoryProcess,errUndefinedClient);
			break;
		case BrokerKind_bkSerController:
			if (FieldDevices[DeviceID].Controller)
				return FieldDevices[DeviceID].Controller->GetIOBuffer(BufferKind, Data);
			else
				SetError(errCategoryProcess,errUndefinedController);
			break;
		default:
			SetError(errCategoryProcess,errUndefinedBroker);
		}
	}
	return 0;
}
//------------------------------------------------------------------------------
void TMBFieldController::Disconnect()
{
	for (int c = 0; c < MaxDevices; c++)
	{
		if (FieldDevices[c].Kind == BrokerKind_bkEthClient && FieldDevices[c].Client != NULL)
			FieldDevices[c].Client->Disconnect();

		if (FieldDevices[c].Kind == BrokerKind_bkSerController || FieldDevices[c].Controller != NULL)
			FieldDevices[c].Controller->Disconnect();
	}
}
//------------------------------------------------------------------------------
int TMBFieldController::AddEthernetDevice(int Proto, byte DeviceID, const char* IP, int Port)
{
	if (DeviceID == 0)
		return SetError(errCategoryProcess,errDeviceIDZero);
	if (FieldDevices[DeviceID].Kind != BrokerKind_bkUnknown || FieldDevices[DeviceID].Client != NULL)
		return SetError(errCategoryProcess,errDeviceAlreadyExists);

	FieldDevices[DeviceID].Client = new TMBEthernetClient(Proto, IP, Port);
	FieldDevices[DeviceID].Kind = BrokerKind_bkEthClient;
	FieldDevices[DeviceID].Client->BaseObjectError = errFieldController; // Overrides the Base Error

	return mbNoError;
}
//------------------------------------------------------------------------------
int TMBFieldController::AddSerialDevice(int Format, byte DeviceID, const char* PortName, int BaudRate, char Parity, int DataBits, int Stops, int Flow)
{
	if (DeviceID == 0)
		return SetError(errCategoryProcess,errDeviceIDZero);
	if (FieldDevices[DeviceID].Kind != BrokerKind_bkUnknown || FieldDevices[DeviceID].Controller != NULL)
		return SetError(errCategoryProcess,errDeviceAlreadyExists);

	PMBSerController Controller = FindSerController(PortName);

	if (Controller == NULL)
	{
		Controller = new TMBSerController(PortName, BaudRate, Parity, DataBits, Stops, Flow);
		Controller->BaseObjectError = errFieldController;
		SerControllers[ControllersCount++] = Controller;
	}
	else
	{
		// Checks the signature, all parameters must be equal
		int Signature = GetComSignature(PortName, BaudRate, Parity, DataBits, Stops, Flow);
		if (Signature != Controller->ComSignature)
			return SetError(errCategoryProcess,errCommParamsMismatch);
	}

	FieldDevices[DeviceID].Kind = BrokerKind_bkSerController;
	FieldDevices[DeviceID].Controller = Controller;
	FieldDevices[DeviceID].Controller->SetRemoteDeviceParam(DeviceID, par_SerialFormat, Format);
	return mbNoError;
}
//------------------------------------------------------------------------------
int TMBFieldController::SetLocalParam(byte LocalID, int ParamIndex, int Value)
{
	if (LocalID > 0)
	{
		if (FieldDevices[LocalID].Kind == BrokerKind_bkEthClient && FieldDevices[LocalID].Client != NULL)
			return FieldDevices[LocalID].Client->SetLocalParam(ParamIndex, Value);

		if (FieldDevices[LocalID].Kind == BrokerKind_bkSerController && FieldDevices[LocalID].Controller != NULL)
			return FieldDevices[LocalID].Controller->SetLocalParam(LocalID, ParamIndex, Value);
	}
	return SetError(errCategoryProcess,errInvalidReqForThisObject);
}
//------------------------------------------------------------------------------
int TMBFieldController::SetRemoteDeviceParam(byte DeviceID, int ParamIndex, int Value)
{
	if (DeviceID > 0)
	{
		if (FieldDevices[DeviceID].Kind == BrokerKind_bkEthClient && FieldDevices[DeviceID].Client != NULL)
			return FieldDevices[DeviceID].Client->SetRemoteDeviceParam(DeviceID, ParamIndex, Value);

		if (FieldDevices[DeviceID].Kind == BrokerKind_bkSerController && FieldDevices[DeviceID].Controller != NULL)
			return FieldDevices[DeviceID].Controller->SetRemoteDeviceParam(DeviceID, ParamIndex, Value);
		
		return SetError(errCategoryProcess,errUndefinedBroker);
	}
	else
		return SetError(errCategoryProcess,errDeviceIDZero);
}
//------------------------------------------------------------------------------
int TMBFieldController::GetDeviceStatus(byte DeviceID, TDeviceStatus& DeviceStatus)
{
	ClearDeviceStatus(DeviceStatus);
	if (DeviceID > 0)
	{
		switch (FieldDevices[DeviceID].Kind)
		{
		case BrokerKind_bkEthClient:
			if (FieldDevices[DeviceID].Client != NULL)
				return FieldDevices[DeviceID].Client->GetDeviceStatus(DeviceID, DeviceStatus);
			break;
		case BrokerKind_bkSerController:
			if (FieldDevices[DeviceID].Controller != NULL)
				return FieldDevices[DeviceID].Controller->GetDeviceStatus(DeviceID, DeviceStatus);
			break;
		}
		return 0;
	}
	else
		return SetError(errCategoryProcess,errDeviceIDZero);
}
