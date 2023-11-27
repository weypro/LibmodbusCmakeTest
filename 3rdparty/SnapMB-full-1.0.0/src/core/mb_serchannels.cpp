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
#include "mb_serchannels.h"
#include "mb_defines.h"
//-----------------------------------------------------------------------------

static TChannelsManager ChannelsManager;

//-----------------------------------------------------------------------------
TChannelsManager::TChannelsManager()
{
	memset(&Channels, 0, sizeof(Channels));
	cs = new TSnapCriticalSection();
}
//-----------------------------------------------------------------------------
TChannelsManager::~TChannelsManager()
{
	delete cs;
}
//-----------------------------------------------------------------------------
void TChannelsManager::Lock()
{
	cs->Enter();
}
//-----------------------------------------------------------------------------
void TChannelsManager::Unlock()
{
	cs->Leave();
}
//-----------------------------------------------------------------------------
int TChannelsManager::FindFirst()
{
	for (int c = 0; c < MaxChannels; c++)
	{
		if (Channels[c].SerialSocket == NULL)
			return c;
	}
	return MaxChannels - 1; // This will never happen since 256 is the maximum OS Comports
}
//-----------------------------------------------------------------------------
int TChannelsManager::ComIndexOf(int Signature)
{
	for (int c = 0; c < MaxChannels; c++)
	{
		if (Channels[c].SerialSocket && Channels[c].SerialSocket->ComSignature==Signature)
			return c;
	}
	return -1;
}
//-----------------------------------------------------------------------------
int TChannelsManager::SckIndexOf(PSerialSocket SerialSocket)
{
	for (int c = 0; c < MaxChannels; c++)
	{
		if (Channels[c].SerialSocket == SerialSocket)
			return c;
	}
	return -1;
}
//-----------------------------------------------------------------------------
void TChannelsManager::NewChannel(const char* PortName, int BaudRate, char Parity, int DataBits, int Stops, int Flow, PSerialSocket& SerialSocket)
{
	int idx = FindFirst();
	SerialSocket = new TSerialSocket();
	SerialSocket->SetComParams(PortName, BaudRate, Parity, DataBits, Stops, Flow);
	SerialSocket->SendTimeout = def_SERSndTimeout;
	SerialSocket->InterframeDelay = def_InterframeDelay;

	Channels[idx].SerialSocket = SerialSocket;
	Channels[idx].RefCount = 1;
}
//-----------------------------------------------------------------------------
void TChannelsManager::GetChannel(const char* PortName, int BaudRate, char Parity, int DataBits, int Stops, int Flow, PSerialSocket& SerialSocket)
{
	Lock();
	try {
		int idx = ComIndexOf(GetComSignature(PortName, BaudRate, Parity, DataBits, Stops, Flow));
		if (idx > -1)
		{
			Channels[idx].RefCount++;
			SerialSocket = Channels[idx].SerialSocket;
		}
		else
			NewChannel(PortName, BaudRate, Parity, DataBits, Stops, Flow, SerialSocket);
	}
	catch (...) {
	}
	Unlock();
}
//-----------------------------------------------------------------------------
void TChannelsManager::DelChannel(PSerialSocket& SerialSocket)
{
	Lock();
	try 
	{
		int idx = SckIndexOf(SerialSocket);
		if (idx > -1)
		{
			if (--Channels[idx].RefCount == 0)
			{
				Channels[idx].SerialSocket = NULL;
				delete SerialSocket;
				SerialSocket = NULL;
			}
		}
	}
	catch(...){}
	Unlock();
}
//-----------------------------------------------------------------------------
void ChannelsManager_GetChannel(const char* PortName, int BaudRate, char Parity, int DataBits, int Stops, int Flow, PSerialSocket& SerialSocket)
{
	ChannelsManager.GetChannel(PortName, BaudRate, Parity, DataBits, Stops, Flow, SerialSocket);
}
//-----------------------------------------------------------------------------
void ChannelsManager_DelChannel(PSerialSocket& SerialSocket)
{
	ChannelsManager.DelChannel(SerialSocket);
}
