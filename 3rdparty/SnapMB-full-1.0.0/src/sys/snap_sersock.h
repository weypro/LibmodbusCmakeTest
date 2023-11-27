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
#ifndef snap_sersock_h
#define snap_sersock_h
//-----------------------------------------------------------------------------
#include "snap_platform.h"
#include "snap_threads.h"
#include "snap_sysutils.h"
//-----------------------------------------------------------------------------

#define NameSize 64
#define MaxPorts 64

#define EPARAMS      0x01
#define ETIMESET     0x02
#define EPORTSET     0x03
#define EPORTOPEN    0x04
#define ERDTOUT      0x05
#define EWRTOUT      0x06
#define EPORTRD      0x07
#define EPORTWR      0x08
#define EPORTGET     0x09
#define EINTERFRAME  0X0A

#define RX  0
#define TX  1
#define RTX 2

#pragma pack(1)

typedef struct {
    char PortName[NameSize];
    int BaudRate;
    char Parity;
    int DataBits;
    int Stops;
    int Flow;
}TComParams;

#pragma pack()

#define FlushTime 150 // Ensures to flush 256 byte @ 19200 bps

class TSerialSocket
{
private:
#ifdef SNAP_OS_WINDOWS
    HANDLE Port;
    DCB cur_settings;
    int LastSizeExpected;
    longword LastTimeout;
#else
    int Port;
    struct termios cur_settings;
    bool CanWrite(int Timeout);
#endif
    PSnapCriticalSection cs;
    int Open();
    void Close();
    int SetError(int Error);
public:
    TComParams ComParams;
    int ComSignature;
    int SendTimeout;
    int RecvTimeout;
    int InterframeDelay;
    int LastSerError;
    bool Connected;
    int MaxInterframeDetected;
#ifndef SNAP_OS_WINDOWS
    bool CanRead(int Timeout);
#endif
#ifdef SNAP_OS_WINDOWS
    int SetTimeout(int SizeExpected, longword Timeout);
#endif
    void Lock();
    void Unlock();
    int SetComParams(const char* PortName, int BaudRate, char Parity, int DataBits, int Stops, int Flow);
    int Connect();
    bool CharReady(char &ch, int Timeout);
    void Disconnect();
    void Flush(int Mode);
    int TelegramTime(int TelegramSize);
    int SendPacket(void* Data, int Size);
    int RecvPacket(void* Data, int Size);
    int RecvPacket(void* Data, int MaxSize, int& Size);
    int RecvPacket(void* Data, int MaxSize, int& Size, int Timeout);
    int RecvInterframedPacket(void* Data, int MaxSize, int& Size);

    TSerialSocket();
    ~TSerialSocket();
};
typedef TSerialSocket* PSerialSocket;


void BuildComParams(const char* PortName, int BaudRate, char Parity, int DataBits, int Stops, int Flow, TComParams & ComParams);
int GetComSignature(TComParams* ComParams);
int GetComSignature(const char* PortName, int BaudRate, char Parity, int DataBits, int Stops, int Flow);


//-----------------------------------------------------------------------------
#endif // snap_sersock_h
