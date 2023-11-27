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
#include "snap_sersock.h"
//-----------------------------------------------------------------------------

#ifndef SNAP_OS_WINDOWS
#define INVALID_HANDLE_VALUE -1
#endif

#if defined(OS_LINUX)
# include <linux/serial.h>
#endif // OS_LINUX

//#include <stdio.h>

//-----------------------------------------------------------------------------
TSerialSocket::TSerialSocket()
{
#ifdef SNAP_OS_WINDOWS
    Port = INVALID_HANDLE_VALUE;
    LastSizeExpected = -1;
    LastTimeout = 0;
#else
    Port = -1;
#endif
    memset(&ComParams, 0, sizeof(TComParams));
    cs = new TSnapCriticalSection();
    RecvTimeout = 3000;
    SendTimeout = 3000;
    InterframeDelay = 20;
    MaxInterframeDetected = 0;
    ComSignature = 0;
}
//-----------------------------------------------------------------------------
TSerialSocket::~TSerialSocket()
{
    Close();
    delete cs;
}
//-----------------------------------------------------------------------------
int TSerialSocket::Open()
{
    LastSerError = 0;
#ifdef SNAP_OS_WINDOWS
    DCB c_settings;

    // Check Params (before opening the port to avoid closing the handle in case of some incorrect param)

    DWORD br = 0;      // Baudrate
    byte  db = 0;      // Data Bits
    byte  cpar = 0;      // Parity
    byte  sb = 0;      // Stop bits
    DWORD fpar = 0;      // Parity Flag

    // Baudrate
    switch (ComParams.BaudRate)
    {
    case     110:
    case     300:
    case     600:
    case    1200:
    case    2400:
    case    4800:
    case    9600:
    case   19200:
    case   38400:
    case   57600:
    case  115200:
    case  128000:
    case  256000:
    case  500000:
    case  921600:
    case 1000000:
    case 1500000:
    case 2000000:
    case 3000000:
        br = ComParams.BaudRate;
        break;
    default:
        return SetError(EPARAMS);
    }

    // Parity
    switch (ComParams.Parity)
    {
    case 'n':
    case 'N':
        cpar = NOPARITY;
        fpar = 0;
        break;
    case 'o':
    case 'O':
        cpar = ODDPARITY;
        fpar = 1;
        break;
    case 'e':
    case 'E':
        cpar = EVENPARITY;
        fpar = 1;
        break;
    default:
        return SetError(EPARAMS);
    }

    // Data bits
    switch (ComParams.DataBits)
    {
    case 5:
    case 6:
    case 7:
    case 8:
        db = byte(ComParams.DataBits);
        break;
    default:
        return SetError(EPARAMS);
    }

    // Stop bits
    switch (ComParams.Stops)
    {
    case 1:
        sb = ONESTOPBIT;
        break;
    case 2:
        sb = TWOSTOPBITS;
        break;
    default:
        return SetError(EPARAMS);
    }

    // Try to Open the Port
    Port = CreateFileA(ComParams.PortName,
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);

    if (Port == INVALID_HANDLE_VALUE)
        return SetError(EPORTOPEN);

    // Stores current settings
    cur_settings.DCBlength = sizeof(DCB);
    if (!GetCommState(Port, &cur_settings))
    {
        CloseHandle(Port);
        Port = INVALID_HANDLE_VALUE;
        return SetError(EPORTGET);
    }

    // Copies into c_settings
    c_settings = cur_settings;

    // Set our Params
    c_settings.fBinary = 1;
    c_settings.fTXContinueOnXoff = 1;
    c_settings.BaudRate = br;
    c_settings.Parity = cpar;
    c_settings.fParity = fpar;
    c_settings.ByteSize = db;
    c_settings.StopBits = sb;
    c_settings.fOutX = 0;      // No XON/XOFF out
    c_settings.fInX = 0;       // No XON/XOFF in

    
    c_settings.fOutxDsrFlow = 0;
    c_settings.fDsrSensitivity = 0;
    c_settings.fDtrControl = RTS_CONTROL_ENABLE;
    c_settings.fAbortOnError = false; 

    if (ComParams.Flow)
    {
        c_settings.fOutxCtsFlow = TRUE;
        c_settings.fRtsControl = RTS_CONTROL_HANDSHAKE;
    }
    else 
    {
        c_settings.fOutxCtsFlow = 0;
        c_settings.fRtsControl = RTS_CONTROL_DISABLE;
    }


    // Set Com Params
    if (!SetCommState(Port, &c_settings))
    {
        CloseHandle(Port);
        Port = INVALID_HANDLE_VALUE;
        return SetError(EPORTSET);
    }

    if (!SetupComm(Port, 2048, 2048))
    {
        CloseHandle(Port);
        Port = INVALID_HANDLE_VALUE;
        return SetError(EPORTSET);
    }

    return 0;
#else
    int br = 0;         // Baudrate
    int db = 0;         // Data Bits
    int cpar = 0;       // Parity
    int ipar = IGNPAR;  // Ignore characters with Parity error
    int sb = 0;         // Stop bits
    int PortFlags = 0;
    struct termios c_settings;

    switch (ComParams.BaudRate)
    {
    case      50: br = B50; break;
    case      75: br = B75; break;
    case     110: br = B110; break;
    case     134: br = B134; break;
    case     150: br = B150; break;
    case     200: br = B200; break;
    case     300: br = B300; break;
    case     600: br = B600; break;
    case    1200: br = B1200; break;
    case    1800: br = B1800; break;
    case    2400: br = B2400; break;
    case    4800: br = B4800; break;
    case    9600: br = B9600; break;
    case   19200: br = B19200; break;
    case   38400: br = B38400; break;
    case   57600: br = B57600; break;
    case  115200: br = B115200; break;
    case  230400: br = B230400; break;
#if defined(OS_LINUX)
    case  460800: br = B460800; break;
    case  500000: br = B500000; break;
    case  576000: br = B576000; break;
    case  921600: br = B921600; break;
    case 1000000: br = B1000000; break;
    case 1152000: br = B1152000; break;
    case 1500000: br = B1500000; break;
    case 2000000: br = B2000000; break;
    case 2500000: br = B2500000; break;
    case 3000000: br = B3000000; break;
    case 3500000: br = B3500000; break;
    case 4000000: br = B4000000; break;
#endif
    default:
        return SetError(EPARAMS);
    }

    switch (ComParams.Parity)
    {
    case 'N':
    case 'n':
        cpar = 0;
        ipar = IGNPAR;
        break;
    case 'E':
    case 'e':
        cpar = PARENB;
        ipar = INPCK;
        break;
    case 'O':
    case 'o':
        cpar = (PARENB | PARODD);
        ipar = INPCK;
        break;
    default:
        return SetError(EPARAMS);
    }

    switch (ComParams.DataBits)
    {
    case 5: db = CS5; break;
    case 6: db = CS6; break;
    case 7: db = CS7; break;
    case 8: db = CS8; break;
    default:
        return SetError(EPARAMS);
    }

    switch (ComParams.Stops)
    {
    case 1: sb = 0; break;
    case 2: sb = CSTOPB; break;
    default:
        return SetError(EPARAMS);
    }

    PortFlags = O_RDWR | O_NOCTTY | O_NDELAY | O_EXCL;
#ifdef O_CLOEXEC
    PortFlags |= O_CLOEXEC;
#endif

    // Try to Open the Port
    Port = open(ComParams.PortName, PortFlags);
    if (Port == INVALID_HANDLE_VALUE)
        return SetError(EPORTOPEN);

    // Stores current settings
    if (tcgetattr(Port, &cur_settings) == -1)
    {
        close(Port);
        Port = INVALID_HANDLE_VALUE;
        return SetError(EPORTGET);
    }
    memset(&c_settings, 0, sizeof(struct termios));

    // Try to Lock the Resource
    if (flock(Port, LOCK_EX | LOCK_NB) != 0)
    {
        close(Port);
        Port = INVALID_HANDLE_VALUE;
        return SetError(EPORTOPEN);
    }

    //-------------------------------------------------------------- c_cflag

    c_settings.c_cflag = db | cpar | sb | CLOCAL | CREAD;
    
    if (ComParams.Flow)
    {
        c_settings.c_cflag |= CRTSCTS;
    }

    // Try to set input and output Port Baudrate
    if ((cfsetispeed(&c_settings, br) < 0) || (cfsetospeed(&c_settings, br) < 0))
    {
        flock(Port, LOCK_UN); // Unlock the Port
        close(Port);
        Port = INVALID_HANDLE_VALUE;
        return SetError(EPORTSET);
    }

    //-------------------------------------------------------------- c_iflag

    c_settings.c_iflag = ipar;

    if (ComParams.Parity == 'N' || ComParams.Parity == 'n')
        c_settings.c_iflag &= ~INPCK;
    else
        c_settings.c_iflag |= INPCK;

    // Disable flow control
    c_settings.c_iflag &= ~(IXON | IXOFF | IXANY);

    //-------------------------------------------------------------- c_lflag

    c_settings.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

    //-------------------------------------------------------------- c_oflag

    c_settings.c_oflag &= ~OPOST;

    c_settings.c_cc[VMIN] = 0;
    c_settings.c_cc[VTIME] = 0;

    int error = tcsetattr(Port, TCSANOW, &c_settings);
    if (error == -1)
    {
        tcsetattr(Port, TCSANOW, &cur_settings);
        flock(Port, LOCK_UN); // Unlock the Port
        close(Port);
        Port = INVALID_HANDLE_VALUE;
        return SetError(EPORTSET);
    }

#ifdef OS_LINUX
    // We don't report the error for this set, since
    // it works only with some chipset (FTDI FT232 is one of them)
    struct serial_struct serial;
    ioctl(Port, TIOCGSERIAL, &serial);
    serial.flags |= ASYNC_LOW_LATENCY;
    ioctl(Port, TIOCSSERIAL, &serial);
#endif


    return 0;
#endif
}
//-----------------------------------------------------------------------------
void TSerialSocket::Close()
{
#ifdef SNAP_OS_WINDOWS
    if (Port != INVALID_HANDLE_VALUE)
    {
        SetCommState(Port, &cur_settings); // Restores previous Params
        CloseHandle(Port);
        Port = INVALID_HANDLE_VALUE;
    }
#else
    if (Port != INVALID_HANDLE_VALUE)
    {
        flock(Port, LOCK_UN); // Unlock the Port
        //tcsetattr(Port, TCSANOW, &cur_settings);
        close(Port);
        Port = INVALID_HANDLE_VALUE;
    }
#endif
}
//-----------------------------------------------------------------------------
int TSerialSocket::SetComParams(const char* PortName, int BaudRate, char Parity, int DataBits, int Stops, int Flow)
{
    BuildComParams(PortName, BaudRate, Parity, DataBits,Stops, Flow, ComParams);
    ComSignature = GetComSignature(&ComParams);
    return 0;
}
//-----------------------------------------------------------------------------
int TSerialSocket::Connect()
{
    Lock();
    try {
        LastSerError = 0;
        if (!Connected)
            Connected = Open() == 0;
    }
    catch (...) {};
    Unlock();
    return LastSerError;
}
//-----------------------------------------------------------------------------
void TSerialSocket::Disconnect()
{
    Lock();
    if (Connected)
    {
        Close();
        Connected = false;
    }
    Unlock();
}
//-----------------------------------------------------------------------------
void TSerialSocket::Flush(int Mode)
{
#ifdef SNAP_OS_WINDOWS
    switch (Mode)
    {
    case RX:
        SysSleep(FlushTime);
        PurgeComm(Port, PURGE_RXABORT | PURGE_RXCLEAR);
        break;
    case TX:
        PurgeComm(Port, PURGE_TXABORT | PURGE_TXCLEAR);
        break;
    case RTX:
        SysSleep(FlushTime);
        PurgeComm(Port, PURGE_RXABORT | PURGE_RXCLEAR | PURGE_TXABORT | PURGE_TXCLEAR);
        break;
    }
#else
    switch (Mode)
    {
    case RX:
        SysSleep(FlushTime);
        tcflush(Port, TCIFLUSH);
        break;
    case TX:
        tcflush(Port, TCOFLUSH);
        break;
    case RTX:
        SysSleep(FlushTime);
        tcflush(Port, TCIOFLUSH);
        break;
    }
#endif
}
//-----------------------------------------------------------------------------
bool TSerialSocket::CharReady(char &ch, int Timeout)
{
// Windows does not have a Select() for serial communications. To know if a char
// is ready we need to get it (which is extracted from the queue).
// So to unify the behaviour Win/Linux for a Serial server we use this function
// Which return a char if it's ready or 0

    LastSerError = 0;

#ifdef SNAP_OS_WINDOWS
    DWORD SizeRead = 0;
    SetError(SetTimeout(1, Timeout));
    if (LastSerError == 0)
    {
        if (!ReadFile(Port, &ch, 1, &SizeRead, NULL))
        {
            DWORD ce;
            ClearCommError(Port, &ce, NULL);
            SetError(EPORTRD);
        }
    }
#else
    int SizeRead = 0;
    if (CanRead(Timeout))
    {
        SizeRead = read(Port, &ch, 1);
        if (SizeRead < 0 && errno != EAGAIN)
            SetError(EPORTRD);
    }
#endif
    return SizeRead > 0;
}
//-----------------------------------------------------------------------------
#ifdef SNAP_OS_WINDOWS
int TSerialSocket::SetTimeout(int SizeExpected, longword Timeout)
{
    int Result = 0;
    if (SizeExpected == 0)
        SizeExpected = 1;  // To avoid division by zero

    // we avoid calling a device function unnecessarily
    if (LastSizeExpected == SizeExpected && LastTimeout == Timeout)
        return Result;

    COMMTIMEOUTS c_tout;

    if (Timeout != 0)
    {
        if (Timeout == 1)
        {
            c_tout.ReadIntervalTimeout = MAXDWORD;
            c_tout.ReadTotalTimeoutMultiplier = 0;
            c_tout.ReadTotalTimeoutConstant = 1;
        }
        else {
            c_tout.ReadIntervalTimeout = 0;
            c_tout.ReadTotalTimeoutMultiplier = Timeout / SizeExpected;
            c_tout.ReadTotalTimeoutConstant = Timeout % SizeExpected;
        }
    }
    else
    {
        c_tout.ReadIntervalTimeout = MAXDWORD;
        c_tout.ReadTotalTimeoutMultiplier = 0;
        c_tout.ReadTotalTimeoutConstant = 0;
    }

    c_tout.WriteTotalTimeoutMultiplier = 60;
    c_tout.WriteTotalTimeoutConstant = SendTimeout;
    if (!SetCommTimeouts(Port, &c_tout))
        Result = ETIMESET;

    // Store these values on success
    if (Result == 0)
    {
        LastSizeExpected = SizeExpected;
        LastTimeout = Timeout;
    }

    return Result;
}
#endif // SNAP_OS_WINDOWS
//-----------------------------------------------------------------------------
int TSerialSocket::SetError(int Error)
{
    LastSerError = Error;
    return LastSerError;
}
//-----------------------------------------------------------------------------
int TSerialSocket::TelegramTime(int TelegramSize)
{
/*
                               FrameBits
TelegramTime (ms) = NBytes *  -----------  * 1000
                               BaudRate

FrameBits = The bits needed to transmit/receive a Char
          = Startbit + DataBits + StopBits + ParityBits

Example :
Size to Send/Receive = 256 byte
Baudrate = 19200
Parity   = Even
DataBits = 8
StopBits = 1

FrameBits = 1 + 8 + 1 + 1 = 11

TelegramTime (ms) = (256 * 1000 * 11) / 19200 = 146 ms

*/

    if (ComParams.BaudRate > 0)
    {
        int CharBits = 1 + ComParams.DataBits + ComParams.Stops;

        if (ComParams.Parity != 'N' && ComParams.Parity != 'n')
            CharBits++;

        return (TelegramSize * 1000 * CharBits) / ComParams.BaudRate;
    }
    else
        return 0;
}
//-----------------------------------------------------------------------------
int TSerialSocket::SendPacket(void* Data, int Size)
{
    int Result = 0;
    LastSerError = 0;

#ifdef SNAP_OS_WINDOWS
    DWORD Written;
    if (WriteFile(Port, Data, Size, &Written, NULL))
    {
        if (int(Written) < Size)
            Result = EWRTOUT;
    }
    else
        Result = EPORTWR;

    // Check Error to do something..
    if (Result == EPORTWR)
    {
        DWORD ce;
        ClearCommError(Port, &ce, NULL);
    }
    else
        if (Result == EWRTOUT && Written > 0)
            PurgeComm(Port, PURGE_TXCLEAR);

#else
    int Written = write(Port, Data, Size);
    if (Written < 0)
        Result = EPORTWR;
    else
        if (Written < Size)
        {
            Result = EWRTOUT;
            Flush(TX);
        }
#endif
    return SetError(Result);
}
//-----------------------------------------------------------------------------
#ifndef SNAP_OS_WINDOWS
bool TSerialSocket::CanRead(int FirstCharTimeout)
{
    LastSerError = 0;
    timeval TV = { FirstCharTimeout / 1000, (FirstCharTimeout % 1000) * 1000 };
    fd_set Read_fds;
    int x;
    FD_ZERO(&Read_fds);
    FD_SET(Port, &Read_fds);
    x = select(Port + 1, &Read_fds, NULL, NULL, &TV);

    if (x < 0)
        return SetError(EPORTRD) == 0;  // set LastSerError to EPORTRD and returns false

    return (x > 0);
}
//-----------------------------------------------------------------------------
bool TSerialSocket::CanWrite(int FirstCharTimeout)
{
    LastSerError = 0;
    timeval TV = { FirstCharTimeout / 1000, (FirstCharTimeout % 1000) * 1000 };
    fd_set Write_fds;
    int x;
    FD_ZERO(&Write_fds);
    FD_SET(Port, &Write_fds);
    x = select(Port + 1, NULL , &Write_fds , NULL, &TV);

    if (x < 0)
        return SetError(EPORTWR) == 0;  // set LastSerError to EPORTRD and returns false

    return (x > 0);
}
#endif
//-----------------------------------------------------------------------------
// Receive a packet of exact size
//-----------------------------------------------------------------------------
int TSerialSocket::RecvPacket(void* Data, int Size)
{
    int Result = 0;
    LastSerError = 0;
    if (Size <= 0)
        return LastSerError;

#ifdef SNAP_OS_WINDOWS
    DWORD SizeRead;

    SetTimeout(Size, RecvTimeout);

    if (ReadFile(Port, Data, Size, &SizeRead, NULL))
    {
        if (int(SizeRead) != Size) // We didn't read what we expected
            Result = ERDTOUT;
    }
    else
        Result = EPORTRD;

    // Check Error to do something..
    if (Result == EPORTRD)
    {
        DWORD ce;
        ClearCommError(Port, &ce, NULL);
    }
    else
        if (Result == ERDTOUT && int(SizeRead) > 0)
            Flush(RX);
#else
    pbyte Buffer = pbyte(Data);
    int x;
    bool ComError = false;
    bool Expired = false;

    if (CanRead(RecvTimeout))
    {
        longword Elapsed = SysGetTick();
        int L = 0;
        while (!Expired && !ComError && L < Size)
        {
            x = read(Port, &Buffer[L], Size);
            ComError = (x < 0) && (errno != EAGAIN);
            if (!ComError)
            {
                L += x;
                if (L < Size)
                    SysSleep(1);
            }
            Expired = int(DeltaTime(Elapsed)) >= RecvTimeout;
        }
        if (Expired)
        {
            Result = ERDTOUT;
            if (L > 0)
                Flush(RX);
        }
        else
            if (ComError)
                Result = EPORTRD;
    }
    else
        if (LastSerError != EPORTRD)
            Result = ERDTOUT;


#endif
    return SetError(Result);
}

//-----------------------------------------------------------------------------
// Receive a packet of unknown size, using InterframeDelay for End Of Packet
//-----------------------------------------------------------------------------
int TSerialSocket::RecvPacket(void* Data, int MaxSize, int& Size, int Timeout)
{
    int Result;
    pbyte Buffer = pbyte(Data);
    char ch;

    if (CharReady(ch,  Timeout))
    {
        Buffer[0] = ch;
        if (MaxSize > 1)
        {
            Result = RecvInterframedPacket(&Buffer[1], MaxSize - 1, Size);
            if (Result == 0)
                Size++;
        }
        else {
            Size = 1;
            Result = 0;
        }
    }
    else
        Result = ERDTOUT;

    return Result;
}

//-----------------------------------------------------------------------------
int TSerialSocket::RecvPacket(void* Data, int MaxSize, int& Size)
{
    return RecvPacket(Data, MaxSize, Size, RecvTimeout);
}

//-----------------------------------------------------------------------------
// Receives an unknown-sized Packet. The Packed is ready when Interframe time
// expired without receiving further chars or MaxSize is reached.
// Function used for Modbus or similar protocols.
// This function should be called after CharReady(), i.e. when we know that
// there is something in the queue
//-----------------------------------------------------------------------------
int TSerialSocket::RecvInterframedPacket(void* Data, int MaxSize, int& Size)
{
    int L = 0;
    int SizeToRead = MaxSize;
    int cnt_if = 0;
    LastSerError = 0;
    MaxInterframeDetected = 0;
    bool ComError = false;
    pbyte Buffer = pbyte(Data);
    Size = 0;

    if (MaxSize <= 0)
        return LastSerError;

#ifdef SNAP_OS_WINDOWS
    DWORD SizeRead;
    SetTimeout(1, 1); // 1 byte and 1 ms
#else
    int SizeRead;
#endif

    while (cnt_if < InterframeDelay && !ComError && SizeToRead > 0)
    {
#ifdef SNAP_OS_WINDOWS
        ComError = !ReadFile(Port, &Buffer[L], SizeToRead, &SizeRead, NULL);
#else
        if (CanRead(1)) // 1 ms
        {
            SizeRead = read(Port, &Buffer[L], SizeToRead);
            if (SizeRead < 0)
            {
                if (errno != EAGAIN)
                    ComError = true;
                else
                    SizeRead = 0;
            }
        }
        else
            SizeRead = 0;

#endif
        if (!ComError)
        {
            if (SizeRead > 0) // Something read
            {
                L += SizeRead;
                SizeToRead -= SizeRead;
                if (MaxInterframeDetected < cnt_if)
                    MaxInterframeDetected = cnt_if;
                cnt_if = 0;  // Reset interframe counter
            }
            else
                cnt_if++;  // no data : increment interframe counter
            // dump interchar, <stdio.h> must be included
            //printf("cnt_if:%d SizeRead:%d\n", cnt_if, SizeRead);
        }
    }
    if (ComError)
    {
#ifdef SNAP_OS_WINDOWS
        DWORD ce;
        ClearCommError(Port, &ce, NULL);
#endif
        if (L > 0)
            Flush(RX);
        return SetError(EPORTRD);
    }
    if (SizeToRead == 0) // We reached the End of Buffer
    {
        // Maybe the buffer is too small or there is Jamming
        // Most likely this Packet will be discarded by the caller
        Flush(RX);
        return SetError(EINTERFRAME);
    }
    // We don't set ERDTOUT error if L == 0, if needed, the caller will do it
    Size = L;
    return 0;
}
//-----------------------------------------------------------------------------
void TSerialSocket::Lock()
{
    cs->Enter();
}
//-----------------------------------------------------------------------------
void TSerialSocket::Unlock()
{
    cs->Leave();
}
//-----------------------------------------------------------------------------
void BuildComParams(const char* PortName, int BaudRate, char Parity, int DataBits, int Stops, int Flow, TComParams& ComParams)
{
#ifdef SNAP_OS_WINDOWS
    strncpy(ComParams.PortName, "\\\\.\\", NameSize - 1); // This allow us to use "COM1", "COM2", ...
    strncat(ComParams.PortName, PortName, NameSize - 1);
#else
    strncpy(ComParams.PortName, PortName, NameSize - 1);
#endif
    ComParams.BaudRate = BaudRate;
    ComParams.Parity = Parity;
    ComParams.DataBits = DataBits;
    ComParams.Stops = Stops;
    ComParams.Flow = Flow;
}
//-----------------------------------------------------------------------------
int GetComSignature(TComParams* ComParams)
{
    int Size = sizeof(TComParams);
    int Result = 0;
    uint8_t* Buf = (uint8_t*)ComParams;

    while (Size--)
        Result += *Buf++;   
    
    return Result;
}
//-----------------------------------------------------------------------------
int GetComSignature(const char* PortName, int BaudRate, char Parity, int DataBits, int Stops, int Flow)
{
    TComParams ComParams;
    BuildComParams(PortName, BaudRate, Parity, DataBits, Stops, Flow, ComParams);
    return GetComSignature(&ComParams);
}
