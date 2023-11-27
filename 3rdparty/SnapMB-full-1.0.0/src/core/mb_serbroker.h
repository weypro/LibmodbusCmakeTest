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
//-----------------------------------------------------------------------------
#ifndef mb_serbroker_h
#define mb_serbroker_h
//-----------------------------------------------------------------------------
#include "snap_sersock.h"
#include "mb_defines.h"
#include "mb_utils.h"
#include "mb_broker.h"
//-----------------------------------------------------------------------------

class TMBSerBroker : public TMBBroker
{
private:
protected:
	PSerialSocket sersck;
	TMBSERPacket SndPacket; // Packet to Send
	TMBSERPacket RcvPacket; // Packet to Send
	byte CurrentFun;
	longword AttemptSleep;
	virtual PDeviceStatus GetDeviceStatusPtr(byte DeviceID) = 0;
	virtual PDeviceParams GetDeviceParamsPtr(byte DeviceID) = 0;
	void SetRecvTimeout(byte DeviceID);
	void ClearBuffers();
	void ClearErrors();
	void UpdateDeviceStatus(PDeviceStatus DeviceStat, longword LastSerError, bool SameFun);
	void PrepareRTURequest(byte DeviceID, pbyte DataOut, word SizeOut);
	void PrepareASCIIRequest(byte DeviceID, pbyte DataOut, word SizeOut);
	int RecvUnknownRTUResponse(byte DeviceID, pbyte DataIn, word& SizeIn, PPDURecvExpected PRE);
	int RecvRTUResponse(byte DeviceID, pbyte DataIn, word& SizeIn, PPDURecvExpected PRE);
	int RecvASCIIResponse(byte DeviceID, pbyte DataIn, word& SizeIn, PPDURecvExpected PRE);
	void PrepareRequest(byte DeviceID, pbyte DataOut, word SizeOut);
	bool SendRequest(byte DeviceID);
	bool RecvResponse(byte DeviceID, pbyte DataIn, word& SizeIn, PPDURecvExpected PRE);
	bool SendRequestRecvResponse(byte DeviceID, pbyte DataIn, word& SizeIn, pbyte DataOut, word SizeOut, PPDURecvExpected PRE);
public:
	int ComSignature;
	char PortName[NameSize];
	void Disconnect();
	int SetLocalParam(byte LocalID, int ParamIndex, int Value);
	int SetRemoteDeviceParam(byte DeviceID, int ParamIndex, int Value);
	int GetIOBufferPtr(int BufferKind, pbyte& Data);
	int GetIOBuffer(int BufferKind, pbyte Data);
	TMBSerBroker();
	virtual ~TMBSerBroker();
};

#endif