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
#ifndef mb_netclient_h
#define mb_netclient_h
//-----------------------------------------------------------------------------
#include "snap_msgsock.h"
#include "mb_defines.h"
#include "mb_utils.h"
#include "mb_broker.h"
//-----------------------------------------------------------------------------

class TMBEthernetClient : public TMBBroker
{
private:
	int Proto;
	PTcpSocket tcpsck;      // TCP Socket
	PUdpSocket udpsck;      // UDP Socket
	PSnapSocket netsck;     // cast for common functions
	word LastTransID;
	byte CurrentFun;
	TMBTCPPacket SndPacket; // Packet to Send 
	TMBTCPPacket RcvPacket; // Packet to Recv
	TDeviceParams Params;
	TDeviceStatus DeviceStat;
	longword AttemptSleep;
	bool Persistent;
	void ClearBuffers();
	word NewTransID();
	bool ValidateConfirmationHeader();
	int TCPRecvTCPPacket(void* Data, word& PDUSizeIn);
	int TCPRecvRTUPacket(void* Data, word& PDUSizeIn);
	int UDPRecvTCPPacket(void* Data, word& PDUSizeIn);
	int UDPRecvRTUPacket(void* Data, word& PDUSizeIn);
	void PrepareTCPRequest(byte DeviceID, pbyte DataOut, word SizeOut);
	void PrepareRTURequest(byte DeviceID, pbyte DataOut, word SizeOut);
	bool SendRequest(byte DeviceID);
	bool RecvResponse(byte DeviceID, pbyte DataIn, word& PDUSizeIn, PPDURecvExpected PRE);
protected:
	void ClearErrors();
	void SetStatus(byte DeviceID, longword Time, int JobResult);
	void PrepareRequest(byte DeviceID, pbyte DataOut, word SizeOut);
	bool SendRequestRecvResponse(byte DeviceID, pbyte DataIn, word& SizeIn, pbyte DataOut, word SizeOut, PPDURecvExpected PRE);
public:
	int Connect();
	int SetRemoteDeviceParam(byte DeviceID, int ParamIndex, int Value);
	int SetLocalParam(int ParamIndex, int Value);
	void Disconnect();
	int GetIOBufferPtr(int BufferKind, pbyte& Data);
	int GetIOBuffer(int BufferKind, pbyte Data);
	int GetDeviceStatus(byte DeviceID, TDeviceStatus& DeviceStatus);
	TMBEthernetClient(int Proto, const char* IP, int Port);
	~TMBEthernetClient();
	friend class TMBFieldController;
};
typedef TMBEthernetClient* PMBEthernetClient;

#endif
