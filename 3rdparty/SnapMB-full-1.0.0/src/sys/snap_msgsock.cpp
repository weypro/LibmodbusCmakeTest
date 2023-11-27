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

#include "snap_msgsock.h"

//---------------------------------------------------------------------------

static SocketsLayer SocketsLayerInitializer;

//---------------------------------------------------------------------------
//  Base class endian aware
//---------------------------------------------------------------------------
TSnapBase::TSnapBase()
{
    int x = 1;
    LittleEndian=*(char *)&x == 1;
}
//---------------------------------------------------------------------------
word TSnapBase::SwapWord(word Value)
{
    if (LittleEndian)
        return  ((Value >> 8) & 0xFF) | ((Value << 8) & 0xFF00);
    else
        return Value;
}
//---------------------------------------------------------------------------
longword TSnapBase::SwapDWord(longword Value)
{
    if (LittleEndian)
        return (Value >> 24) | ((Value << 8) & 0x00FF0000) | ((Value >> 8) & 0x0000FF00) | (Value << 24);
    else
        return Value;
}
//---------------------------------------------------------------------------
int TSnapSocket::SetSin(sockaddr_in& sin, char* Address, u_short Port)
{
    uint32_t in_addr;
    in_addr = inet_addr(Address);
    memset(&sin, 0, sizeof(sin));

    if (in_addr != INADDR_NONE)
    {
        sin.sin_addr.s_addr = in_addr;
        sin.sin_family = AF_INET;
        sin.sin_port = htons(Port);
        return 0;
    }
    else
        return WSAEINVALIDADDRESS;
}
//---------------------------------------------------------------------------
void TSnapSocket::GetSin(sockaddr_in sin, char* Address, u_short& Port)
{
    strcpy(Address, inet_ntoa(sin.sin_addr));
    Port = htons(sin.sin_port);
}
//---------------------------------------------------------------------------
int TSnapSocket::SockCheck(int SockResult)
{
    if (SockResult == (int)(SOCKET_ERROR))
        LastNetError = GetLastSocketError();

    return LastNetError;
}
//---------------------------------------------------------------------------
void TSnapSocket::GetLocal()
{
#ifdef SNAP_OS_WINDOWS
    int namelen = sizeof(LocalSin);
#else
    uint32_t namelen = sizeof(LocalSin);
#endif
    if (getsockname(FSocket, (struct sockaddr*)&LocalSin, &namelen) == 0)
        GetSin(LocalSin, LocalAddress, LocalPort);
}
//---------------------------------------------------------------------------
void TSnapSocket::GetRemote()
{
#ifdef SNAP_OS_WINDOWS
    int namelen = sizeof(RemoteSin);
#else
    uint32_t namelen = sizeof(RemoteSin);
#endif
    if (getpeername(FSocket, (struct sockaddr*)&RemoteSin, &namelen) == 0)
        GetSin(RemoteSin, RemoteAddress, RemotePort);
}
//---------------------------------------------------------------------------
int TSnapSocket::GetLastSocketError()
{
#ifdef SNAP_OS_WINDOWS
    return WSAGetLastError();
#else
    return errno;
#endif
}
//---------------------------------------------------------------------------
bool TSnapSocket::CanRead(int Timeout)
{
    timeval TimeV = { 0, 0 };
    int64_t x;
    fd_set FDset;

    if (FSocket == INVALID_SOCKET)
        return false;

    TimeV.tv_usec = (Timeout % 1000) * 1000;
    TimeV.tv_sec = Timeout / 1000;

    FD_ZERO(&FDset);
    FD_SET(FSocket, &FDset);

    x = select(int(FSocket + 1), &FDset, NULL, NULL, &TimeV); //<-Ignore this warning in 64bit Visual Studio
    if (x == (int)SOCKET_ERROR)
    {
        LastNetError = GetLastSocketError();
        x = 0;
    }
    return (x > 0);
}
//---------------------------------------------------------------------------
void TSnapSocket::DestroySocket()
{
    // to be inherited
}
//---------------------------------------------------------------------------
bool TSnapSocket::CanWrite(int Timeout)
{
    timeval TimeV = { 0, 0 };
    int64_t x;
    fd_set FDset;

    if (FSocket == INVALID_SOCKET)
        return false;

    TimeV.tv_usec = (Timeout % 1000) * 1000;
    TimeV.tv_sec = Timeout / 1000;

    FD_ZERO(&FDset);
    FD_SET(FSocket, &FDset);

    x = select(int(FSocket + 1), NULL, &FDset, NULL, &TimeV); //<-Ignore this warning in 64bit Visual Studio
    if (x == (int)SOCKET_ERROR)
    {
        LastNetError = GetLastSocketError();
        x = 0;
    }
    return (x > 0);
}
//---------------------------------------------------------------------------
int TSnapSocket::SetError(int error)
{
    LastNetError = error;
    return error;
}
//---------------------------------------------------------------------------
int TSnapSocket::SendPacket(void* Data, int Size)
{
    return 0;
}
//---------------------------------------------------------------------------
void TSnapSocket::SckDisconnect()
{
    DestroySocket();
    Connected = false;
}
//---------------------------------------------------------------------------
TSnapSocket::~TSnapSocket()
{
    DestroySocket();
}
//---------------------------------------------------------------------------
TSnapSocket::TSnapSocket()
{
    // Set Defaults
    strncpy(LocalAddress, "0.0.0.0", 16);
    LocalPort = 0;
    strncpy(RemoteAddress, "127.0.0.1", 16);
    memset(&LocalSin, 0, sizeof(LocalSin));
    memset(&RemoteSin, 0, sizeof(RemoteSin));
    ClientHandle = 0;
    RemotePort = 0;
    WorkInterval = 100;
    RecvTimeout = 500;
    SendTimeout = 10;
    PingTimeout = 750;
    Connected = false;
    FSocket = INVALID_SOCKET;
    LastNetError = 0;
    LocalBind = 0;
}

//---------------------------------------------------------------------------
void Msg_CloseSocket(socket_t FSocket)
{
    #ifdef SNAP_OS_WINDOWS
    closesocket(FSocket);
    #else
    close(FSocket);
    #endif
}
//---------------------------------------------------------------------------
longword Msg_GetSockAddr(socket_t FSocket)
{
    sockaddr_in RemoteSin;
    #ifdef SNAP_OS_WINDOWS
    int namelen = sizeof(RemoteSin);
    #else
    uint32_t namelen = sizeof(RemoteSin);
    #endif
    namelen=sizeof(sockaddr_in);
    if (getpeername(FSocket,(struct sockaddr*)&RemoteSin, &namelen)==0)
        return RemoteSin.sin_addr.s_addr;
    else
        return 0;
}
//---------------------------------------------------------------------------
TTcpSocket::TTcpSocket()
{
    Pinger = new TPinger();
}
//---------------------------------------------------------------------------
TTcpSocket::~TTcpSocket()
{
    DestroySocket();
    delete Pinger;
}
//---------------------------------------------------------------------------
void TTcpSocket::Purge()
{
    // small buffer to empty the socket
    char Trash[512];
    int Read;
    if (LastNetError!=WSAECONNRESET)
    {
        if (CanRead(0)) {
           do
           {
               Read=recv(FSocket, Trash, 512, MSG_NOSIGNAL );
           } while(Read==512);
        }
    }
}
//---------------------------------------------------------------------------
void TTcpSocket::CreateSocket()
{
    DestroySocket();
    LastNetError=0;
    FSocket =socket(AF_INET, SOCK_STREAM, IPPROTO_TCP );
    if (FSocket!=INVALID_SOCKET)
        SetSocketOptions();
    else
        LastNetError =GetLastSocketError();
}
//---------------------------------------------------------------------------
void TTcpSocket::GotSocket()
{
    ClientHandle=RemoteSin.sin_addr.s_addr;
    // could be inherited it if wee need further actions on the socket
}
//---------------------------------------------------------------------------
void TTcpSocket::SetSocket(socket_t s)
{
    FSocket=s;
    if (FSocket!=INVALID_SOCKET)
    {
        SetSocketOptions();
        GetLocal();
        GetRemote();
        GotSocket();
    }
    Connected=FSocket!=INVALID_SOCKET;
}
//---------------------------------------------------------------------------
void TTcpSocket::DestroySocket()
{
    if(FSocket != INVALID_SOCKET)
    {
        if (shutdown(FSocket, SD_SEND)==0)
            Purge();
    #ifdef SNAP_OS_WINDOWS
        closesocket(FSocket);
    #else
        close(FSocket);
    #endif
        FSocket=INVALID_SOCKET;
    }
    LastNetError=0;
}
//---------------------------------------------------------------------------
int TTcpSocket::WaitingData()
{
    int result = 0;
    u_long x = 0;
#ifdef SNAP_OS_WINDOWS
    if (ioctlsocket(FSocket, FIONREAD, &x) == 0)
        result = x;
#else
    if (ioctl(FSocket, FIONREAD, &x) == 0)
        result = x;
#endif
    if (result>MaxPacketSize)
        result = MaxPacketSize;
    return result;
}
//---------------------------------------------------------------------------
int TTcpSocket::WaitForData(int Size, int Timeout)
{
    longword Elapsed;

    // Check for connection active
    if (CanRead(0) && (WaitingData()==0))
        LastNetError=WSAECONNRESET;
    else
        LastNetError=0;

    // Enter main loop
    if (LastNetError==0)
    {
        Elapsed =SysGetTick();
        while((WaitingData()<Size) && (LastNetError==0))
        {
            // Checks timeout
            if (DeltaTime(Elapsed)>=(longword)(Timeout))
                LastNetError =WSAETIMEDOUT;
            else
                SysSleep(1);
        }
    }
    if(LastNetError==WSAECONNRESET)
        Connected =false;

    return LastNetError;
}
//---------------------------------------------------------------------------
void TTcpSocket::SetSocketOptions()
{
    int NoDelay = 1;
    int KeepAlive = 1;
    LastNetError=0;
    SockCheck(setsockopt(FSocket, IPPROTO_TCP, TCP_NODELAY,(char*)&NoDelay, sizeof(NoDelay)));

    if (LastNetError==0)
        SockCheck(setsockopt(FSocket, SOL_SOCKET, SO_KEEPALIVE,(char*)&KeepAlive, sizeof(KeepAlive)));
}
//---------------------------------------------------------------------------
#ifdef NON_BLOCKING_CONNECT
//
// Non blocking connection (UNIX) Thanks to Rolf Stalder
//
int TTcpSocket::SckConnect()
{
    int n, flags, err;
    socklen_t len;
    fd_set rset, wset;
    struct timeval tval;

    LastNetError = SetSin(RemoteSin, RemoteAddress, RemotePort);

    if (LastNetError == 0)
    {
        CreateSocket();
        if (LastNetError == 0)
        {
            flags = fcntl(FSocket, F_GETFL, 0);
            if (flags >= 0)
            {
                if (fcntl(FSocket, F_SETFL, flags | O_NONBLOCK)  != -1)
                {
                    n = connect(FSocket, (struct sockaddr*)&RemoteSin, sizeof(RemoteSin));
                    if (n < 0)
                    {
                        if (errno != EINPROGRESS)
                        {
                            LastNetError = GetLastSocketError();
                        }
                        else {
                            // still connecting ...
                            FD_ZERO(&rset);
                            FD_SET(FSocket, &rset);
                            wset = rset;
                            tval.tv_sec = PingTimeout / 1000;
                            tval.tv_usec = (PingTimeout % 1000) * 1000;

                            n = select(FSocket+1, &rset, &wset, NULL,
                                       (PingTimeout ? &tval : NULL));
                            if (n == 0)
                            {
                                // timeout
                                LastNetError = WSAEHOSTUNREACH;
                            }
                            else
                            {
                                if (FD_ISSET(FSocket, &rset) || FD_ISSET(FSocket, &wset))
                                {
                                    err = 0;
                                    len = sizeof(err);
                                    if (getsockopt(FSocket, SOL_SOCKET, SO_ERROR, &err, &len) == 0)
                                    {
                                        if (err)
                                        {
                                             LastNetError = err;
                                        }
                                        else
                                        {
                                            if (fcntl(FSocket, F_SETFL, flags) != -1)
                                            {
                                                GetLocal();
                                                ClientHandle = LocalSin.sin_addr.s_addr;
                                            }
                                            else
                                            {
                                                LastNetError = GetLastSocketError();
                                            }
                                        }
                                    }
                                    else
                                    {
                                        LastNetError = GetLastSocketError();
                                    }
                                }
                                else
                                {
                                    LastNetError = -1;
                                }
                            }
                        } // still connecting
                    }
                    else if (n == 0)
                    {
                        // connected immediatly
                        GetLocal();
                        ClientHandle = LocalSin.sin_addr.s_addr;
                    }
                }
                else
                {
                    LastNetError = GetLastSocketError();
                } // fcntl(F_SETFL)
            }
            else
            {
                LastNetError = GetLastSocketError();
            } // fcntl(F_GETFL)
        } //valid socket
    } // LastNetError==0
    Connected=LastNetError==0;
    return LastNetError;
}
#else
//
// Regular connection (Windows)
//
int TTcpSocket::SckConnect()
{
    int Result;
    LastNetError = SetSin(RemoteSin, RemoteAddress, RemotePort);
    if (LastNetError==0)
    {
        if (Ping(RemoteSin))
        {
            CreateSocket();
            if (LastNetError==0)
            {
                Result=connect(FSocket, (struct sockaddr*)&RemoteSin, sizeof(RemoteSin));
                if (SockCheck(Result)==0)
                {
                    GetLocal();
                    // Client handle is self_address (here the connection is ACTIVE)
                    ClientHandle=LocalSin.sin_addr.s_addr;
                }
            }
        }
        else
            LastNetError=WSAEHOSTUNREACH;
    }
    Connected=LastNetError==0;
    return LastNetError;
}
#endif
//---------------------------------------------------------------------------
void TTcpSocket::ForceClose()
{
    if(FSocket != INVALID_SOCKET)
    {
        try {
            #ifdef SNAP_OS_WINDOWS
            closesocket(FSocket);
            #else
            close(FSocket);
            #endif
        } catch (...) {
        }
        FSocket=INVALID_SOCKET;
    }
    LastNetError=0;
}
//---------------------------------------------------------------------------
int TTcpSocket::SckBind()
{
    int Res;
    int Opt=1;
    LastNetError = SetSin(LocalSin, LocalAddress, LocalPort);
    if (LastNetError==0)
    {
        CreateSocket();
        if (LastNetError==0)
        {
            setsockopt(FSocket ,SOL_SOCKET, SO_REUSEADDR, (const char *)&Opt, sizeof(int));
            Res =bind(FSocket, (struct sockaddr*)&LocalSin, sizeof(sockaddr_in));
            SockCheck(Res);
            if (Res==0)
            {
                LocalBind=LocalSin.sin_addr.s_addr;
            }
        }
    }
    else
        LastNetError=WSAEINVALIDADDRESS;

    return LastNetError;
}
//---------------------------------------------------------------------------
int TTcpSocket::SckListen()
{
    LastNetError=0;
    SockCheck(listen(FSocket ,SOMAXCONN));
    return LastNetError;
}
//---------------------------------------------------------------------------
bool TTcpSocket::Ping(char *Host)
{
    return Pinger->Ping(Host, PingTimeout);
}
//---------------------------------------------------------------------------
bool TTcpSocket::Ping(sockaddr_in Addr)
{
    if (PingTimeout == 0)
        return true;
    else
        return Pinger->Ping(Addr.sin_addr.s_addr, PingTimeout);
}
//---------------------------------------------------------------------------
socket_t TTcpSocket::SckAccept()
{
    socket_t result;
    LastNetError=0;
    result = accept(FSocket, NULL, NULL);
    if(result==INVALID_SOCKET)
        LastNetError =GetLastSocketError();
    return result;
}
//---------------------------------------------------------------------------
int TTcpSocket::SendPacket(void *Data, int Size)
{
    int Result;

    LastNetError=0;
    if (SendTimeout>0)
    {
        if (!CanWrite(SendTimeout))
        {
            LastNetError = WSAETIMEDOUT;
            return LastNetError;
        }
    }

    if (send(FSocket, (char*)Data, Size, MSG_NOSIGNAL)==Size)
        return 0;
    else
        Result =SOCKET_ERROR;
  
    return SockCheck(Result);
}
//---------------------------------------------------------------------------
bool TTcpSocket::PacketReady(int Size)
{
    return (WaitingData()>=Size);
}
//---------------------------------------------------------------------------
int TTcpSocket::Receive(void *Data, int BufSize, int &SizeRecvd)
{
    LastNetError=0;
    if (CanRead(RecvTimeout))
    {
        SizeRecvd=recv(FSocket ,(char*)Data ,BufSize ,MSG_NOSIGNAL );

        if (SizeRecvd>0) // something read (default case)
           LastNetError=0;
        else
            if (SizeRecvd==0)
                LastNetError = WSAECONNRESET;  // Connection reset by Peer
            else
                LastNetError=GetLastSocketError(); // we need to know what happened
    }
    else
        LastNetError = WSAETIMEDOUT;

    if (LastNetError==WSAECONNRESET)
        Connected = false;

    return LastNetError;
}
//---------------------------------------------------------------------------
int TTcpSocket::RecvPacket(void *Data, int Size)
{
    int BytesRead;
    WaitForData(Size, RecvTimeout);
    if (LastNetError==0)
    {
        BytesRead=recv(FSocket, (char*)Data, Size, MSG_NOSIGNAL);
        if (BytesRead==0)
            LastNetError = WSAECONNRESET;  // Connection reset by Peer
        else
            if (BytesRead<0)
                LastNetError = GetLastSocketError();
    }
    else // After the timeout the bytes waiting were less then we expected
        if (LastNetError==WSAETIMEDOUT)
            Purge();

    if (LastNetError==WSAECONNRESET)
        Connected =false;

    return LastNetError;
}
//---------------------------------------------------------------------------
int TTcpSocket::PeekPacket(void *Data, int Size)
{
    int BytesRead;
    WaitForData(Size, RecvTimeout);
    if (LastNetError==0)
    {
        BytesRead=recv(FSocket, (char*)Data, Size, MSG_PEEK | MSG_NOSIGNAL );
        if (BytesRead==0)
            LastNetError = WSAECONNRESET;  // Connection reset by Peer
        else
            if (BytesRead<0)
                LastNetError = GetLastSocketError();
    }
    else // After the timeout the bytes waiting were less then we expected
        if (LastNetError==WSAETIMEDOUT)
            Purge();

    if (LastNetError==WSAECONNRESET)
        Connected =false;

    return LastNetError;
}
//==============================================================================
// PING
//==============================================================================

static int PingKind;
static int WSStartupResult;

#ifdef SNAP_OS_WINDOWS
// iphlpapi, is loaded dinamically because if this fails we can still try
// to use raw sockets

static char const *iphlpapi = "\\iphlpapi.dll";
#pragma pack(1)

//typedef byte TTxBuffer[40];
typedef byte TTxBuffer[32];
#pragma pack()

typedef HANDLE (__stdcall *pfn_IcmpCreateFile)();
typedef bool (__stdcall *pfn_IcmpCloseHandle)(HANDLE PingHandle);

typedef int (__stdcall *pfn_IcmpSendEcho2)(
    HANDLE PingHandle,
    void *Event,
    void *AcpRoutine,
    void *AcpContext,
    unsigned long DestinationAddress,
    void *RequestData,
    int RequestSize,
    void *not_used,  //should be *IP_OPTION_INFORMATION but we don't use it
    void *ReplyBuffer,
    int ReplySize,
    int Timeout
);

static pfn_IcmpCreateFile IcmpCreateFile;
static pfn_IcmpCloseHandle IcmpCloseHandle;
static pfn_IcmpSendEcho2 IcmpSendEcho2;
static HINSTANCE IcmpDllHandle = 0;
static bool IcmpAvail = false;

bool IcmpInit()
{
    char iphlppath[MAX_PATH+12];

    int PathLen = GetSystemDirectoryA(iphlppath, MAX_PATH);
    if (PathLen != 0)
    {
        strcat(iphlppath, iphlpapi);
        IcmpDllHandle = LoadLibraryA(iphlppath);
    }
    else
        IcmpDllHandle = 0;

    if (IcmpDllHandle != 0)
    {
        IcmpCreateFile=(pfn_IcmpCreateFile)GetProcAddress(IcmpDllHandle,"IcmpCreateFile");
        IcmpCloseHandle=(pfn_IcmpCloseHandle)GetProcAddress(IcmpDllHandle,"IcmpCloseHandle");
        IcmpSendEcho2=(pfn_IcmpSendEcho2)GetProcAddress(IcmpDllHandle,"IcmpSendEcho2");
        return (IcmpCreateFile!=NULL) && (IcmpCloseHandle!=NULL) && (IcmpSendEcho2!=NULL);
    }
    else
        return false;
}

void IcmpDone()
{
    if (IcmpDllHandle!=0)
       FreeLibrary(IcmpDllHandle);
    IcmpAvail=false;
}
#endif

//---------------------------------------------------------------------------
//  RAW Socket Pinger
//---------------------------------------------------------------------------
TRawSocketPinger::TRawSocketPinger()
{
    FSocket =socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    FId  =word(size_t(this));
    memset(IcmpBuffer, 0, sizeof(IcmpBuffer));
    FSeq =0;
}
//---------------------------------------------------------------------------
TRawSocketPinger::~TRawSocketPinger()
{
    if (FSocket!=INVALID_SOCKET)
    {
      #ifdef SNAP_OS_WINDOWS
          closesocket(FSocket);
      #else
          close(FSocket);
      #endif
      FSocket=INVALID_SOCKET;
    };
}
//---------------------------------------------------------------------------
void TRawSocketPinger::InitPacket()
{
    memset(&IcmpBuffer,0,ICmpBufferSize);
    FSeq++;

    SendPacket=PIcmpPacket(pbyte(&IcmpBuffer)+sizeof(TIPHeader));
    SendPacket->Header.ic_type=ICMP_ECHORQ;
    SendPacket->Header.ic_code=0;
    SendPacket->Header.ic_cksum=0;
    SendPacket->Header.ic_id=FId;
    SendPacket->Header.ic_seq=FSeq;

    memset(&SendPacket->Data,0,sizeof(SendPacket->Data));
    SendPacket->Header.ic_cksum=PacketChecksum();
}
//---------------------------------------------------------------------------
word TRawSocketPinger::PacketChecksum()
{
    word *P = (word*)(SendPacket);
    longword Sum = 0;
    int c;
    for (c = 0; c < int(sizeof(TIcmpPacket) / 2); c++) {
        Sum+=*P;
        P++;
    }
    Sum=(Sum >> 16) + (Sum & 0xFFFF);
    Sum=Sum+(Sum >> 16);
    return word(~Sum);
}
//---------------------------------------------------------------------------
bool TRawSocketPinger::CanRead(int Timeout)
{
    timeval TimeV;
    int64_t x;
    fd_set FDset;

    TimeV.tv_usec = (Timeout % 1000) * 1000;
    TimeV.tv_sec = Timeout / 1000;

    FD_ZERO(&FDset);
    FD_SET(FSocket, &FDset);

    x = select(int(FSocket + 1), &FDset, NULL, NULL, &TimeV); //<-Ignore this warning in 64bit Visual Studio
    if (x==(int)(SOCKET_ERROR))
       x=0;
    return (x > 0);
}
//---------------------------------------------------------------------------
bool TRawSocketPinger::Ping(longword ip_addr, int Timeout)
{
    sockaddr_in LSockAddr;
    sockaddr_in RSockAddr;
    PIcmpReply Reply;

    if (FSocket==INVALID_SOCKET)
      return true;

    // Init packet
    InitPacket();
    Reply=PIcmpReply(&IcmpBuffer);
    // Init Remote and Local Addresses struct
    RSockAddr.sin_family=AF_INET;
    RSockAddr.sin_port=0;
    RSockAddr.sin_addr.s_addr=ip_addr;

    LSockAddr.sin_family=AF_INET;
    LSockAddr.sin_port=0;
    LSockAddr.sin_addr.s_addr=inet_addr("0.0.0.0");

    // Bind to local
    if (bind(FSocket, (struct sockaddr*)&LSockAddr, sizeof(sockaddr_in))!=0)
        return false;
    // Connect to remote (not a really TCP connection, only to setup the socket)
    if (connect(FSocket, (struct sockaddr*)&RSockAddr, sizeof(sockaddr_in))!=0)
        return false;
    // Send ICMP packet
    if (send(FSocket, (char*)SendPacket, sizeof(TIcmpPacket), MSG_NOSIGNAL)!=int(sizeof(TIcmpPacket)))
        return false;
    // Wait for a reply
    if (!CanRead(Timeout))
        return false;// time expired
    // Get the answer
    if (recv(FSocket, (char*)&IcmpBuffer, ICmpBufferSize, MSG_NOSIGNAL)<int(sizeof(TIcmpReply)))
        return false;
    // Check the answer
    return (Reply->IPH.ip_src==RSockAddr.sin_addr.s_addr) &&  // the peer is what we are looking for
           (Reply->ICmpReply.Header.ic_type==ICMP_ECHORP);    // type = reply
}
//---------------------------------------------------------------------------
//  Pinger
//---------------------------------------------------------------------------
TPinger::TPinger()
{
    RawAvail = false;
    RawPinger = NULL;
}
//---------------------------------------------------------------------------
TPinger::~TPinger()
{
}
//---------------------------------------------------------------------------
bool TPinger::RawPing(longword ip_addr, int Timeout)
{
    RawPinger = new TRawSocketPinger();
    bool Result;
    Result=RawPinger->Ping(ip_addr, Timeout);
    delete RawPinger;
    return Result;
}
//---------------------------------------------------------------------------
#ifdef SNAP_OS_WINDOWS
bool TPinger::WinPing(longword ip_addr, int Timeout)
{
    HANDLE PingHandle;
    TTxBuffer TxBuffer;
    TIcmpBuffer IcmpBuffer;
    bool Result;

    PingHandle = IcmpCreateFile();
    if (PingHandle != INVALID_HANDLE_VALUE)
    {
        memset(&TxBuffer,'\55',sizeof(TTxBuffer));
        Result=(IcmpSendEcho2(PingHandle, NULL, NULL, NULL, ip_addr,
            &TxBuffer, sizeof(TxBuffer), NULL, &IcmpBuffer, ICmpBufferSize, Timeout))>0;
        IcmpCloseHandle(PingHandle);
        return Result;
    }
    else
        return false;
}
#endif
//---------------------------------------------------------------------------
bool TPinger::Ping(char *Host, int Timeout)
{
    longword Addr;
    Addr=inet_addr(Host);
    return Ping(Addr, Timeout);
}
//---------------------------------------------------------------------------
bool TPinger::Ping(longword ip_addr, int Timeout)
{
#ifdef SNAP_OS_WINDOWS
    if (PingKind==pkWinHelper)
        return WinPing(ip_addr, Timeout);
    else
#endif
    if (PingKind==pkRawSocket)
        return RawPing(ip_addr, Timeout);
    else
        return true; // we still need to continue
}
//---------------------------------------------------------------------------
// Checks if raw sockets are allowed
//---------------------------------------------------------------------------
bool RawSocketsCheck()
{
    socket_t RawSocket;
    bool Result;
    RawSocket = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);

    Result=RawSocket != INVALID_SOCKET;
    if (Result)
    #ifdef SNAP_OS_WINDOWS
        closesocket(RawSocket);
    #else
        close(RawSocket);
    #endif

    return Result;
}
//---------------------------------------------------------------------------
// Sockets init
// - Winsock Startup (Windows)
// - ICMP Helper Load (Windows)
// - Check for raw socket (Unix or Windows if ICMP load failed)
//---------------------------------------------------------------------------
SocketsLayer::SocketsLayer()
{
#ifdef SNAP_OS_WINDOWS
 //   timeBeginPeriod(1); // Moved to LibInit
    WSStartupResult = WSAStartup(0x202,&wsaData);
    if (IcmpInit())
       PingKind=pkWinHelper;
    else
#endif
    if (RawSocketsCheck())
        PingKind=pkRawSocket;
    else
        PingKind=pkCannotPing;
}

SocketsLayer::~SocketsLayer()
{
#ifdef SNAP_OS_WINDOWS
    IcmpDone();
    WSACleanup();
//    timeEndPeriod(1); // Moved to LibDone
#endif
}

//---------------------------------------------------------------------------
// UDP Socket
//---------------------------------------------------------------------------
void TUdpSocket::CreateSocket()
{
    DestroySocket();
    LastNetError = 0;
    FSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (FSocket != INVALID_SOCKET)
        SetSocketOptions();
    else
        LastNetError = GetLastSocketError();
}
//---------------------------------------------------------------------------
void TUdpSocket::DestroySocket()
{
    if (FSocket != INVALID_SOCKET)
    {
#ifdef SNAP_OS_WINDOWS
        closesocket(FSocket);
#else
        close(FSocket);
#endif
        FSocket = INVALID_SOCKET;
    }
    LastNetError = 0;
}
//---------------------------------------------------------------------------
void TUdpSocket::SetSocketOptions()
{
    char broadcast = 1;
    setsockopt(FSocket, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)); // give a try
}
//---------------------------------------------------------------------------
int TUdpSocket::WaitingData()
{
    int result = 0;
    u_long x = 0;
#ifdef SNAP_OS_WINDOWS
    if (ioctlsocket(FSocket, FIONREAD, &x) == 0)
        result = int(x);
#else
    if (ioctl(FSocket, FIONREAD, &x) == 0)
        result = x;
#endif
    if (result > MaxPacketSize)
        result = MaxPacketSize;
    return result;
}
//---------------------------------------------------------------------------
int TUdpSocket::WaitForData(int Size, int Timeout)
{
    longword Elapsed;
    LastNetError = 0;

    // Enter main loop
    if (LastNetError == 0)
    {
        Elapsed = SysGetTick();
        while ((WaitingData() < Size) && (LastNetError == 0))
        {
            // Checks timeout
            if (DeltaTime(Elapsed) >= (longword)(Timeout))
                LastNetError = WSAETIMEDOUT;
            else
                SysSleep(1);
        }
    }
    return LastNetError;
}
//---------------------------------------------------------------------------
int TUdpSocket::SckConnect()
{
    LastNetError = SetSin(RemoteSin, RemoteAddress, RemotePort);

    if (LastNetError == 0)
        CreateSocket();

    if (LastNetError==0)
        SockCheck(connect(FSocket, (struct sockaddr*)&RemoteSin, sizeof(RemoteSin)));
    
    Connected = LastNetError == 0;
    return LastNetError;
}
//---------------------------------------------------------------------------
int TUdpSocket::SendPacket(void* Data, int Size)
{
    LastNetError = 0;
    if (CanWrite(SendTimeout))
    {
        int SizeWritten = send(FSocket, (char*)Data, Size, 0);
        if (SizeWritten != Size)
        {
            if (SizeWritten < 0)
                LastNetError = GetLastSocketError();
            else
                LastNetError = WSAETIMEDOUT;
        }
    }
    else
        if (LastNetError==0)
            LastNetError = WSAETIMEDOUT;

    return LastNetError;
}
//---------------------------------------------------------------------------
int TUdpSocket::RecvPacket(void* Data, int Size)
{
    LastNetError = 0;
    int BytesRead = 0;
   
    if (CanRead(RecvTimeout))
    {
        BytesRead = recv(FSocket, (char*)Data, Size, MSG_NOSIGNAL);
        if (BytesRead <= 0)
            LastNetError = GetLastSocketError();
    }
    else
        if (LastNetError==0)
            LastNetError = WSAETIMEDOUT;

    return BytesRead;
}
//---------------------------------------------------------------------------
int TUdpSocket::RecvPacketFrom(void* Data, int Size, sockaddr_in& ClientSin)
{
    LastNetError = 0;
    int BytesRead = 0;
    int SinLen = sizeof(RemoteSin);

    if (CanRead(RecvTimeout))
    {
#ifdef SNAP_OS_WINDOWS
        BytesRead = recvfrom(FSocket, (char*)Data, Size, 0, (struct sockaddr*)&ClientSin, &SinLen);
#else
        BytesRead = recvfrom(FSocket, (char*)Data, Size, 0, (struct sockaddr*)&ClientSin, (socklen_t*)&SinLen);
#endif        
        if (BytesRead <= 0)
            LastNetError = GetLastSocketError();
    }
    else
        if (LastNetError == 0)
            LastNetError = WSAETIMEDOUT;

    return BytesRead;
}

//---------------------------------------------------------------------------
int TUdpSocket::SendPacketTo(void* Data, int Size, sockaddr_in& ClientSin)
{
    LastNetError = 0;
    if (CanWrite(SendTimeout))
    {
        int SizeWritten = sendto(FSocket, (char*)Data, Size, 0, (const struct sockaddr*)&ClientSin, sizeof(ClientSin));

        if (SizeWritten != Size)
        {
            if (SizeWritten < 0)
                LastNetError = GetLastSocketError();
            else
                LastNetError = WSAETIMEDOUT;
        }
    }
    else
        if (LastNetError == 0)
            LastNetError = WSAETIMEDOUT;

    return LastNetError;
}

//---------------------------------------------------------------------------
int TUdpSocket::AnswerPacketTo(void* Data, int Size, sockaddr_in& ClientSin)
{
#ifndef MSG_CONFIRM
#define MSG_CONFIRM 0
#endif
    LastNetError = 0;
    if (CanWrite(SendTimeout))
    {
        int SizeWritten = sendto(FSocket, (char*)Data, Size, MSG_CONFIRM, (const struct sockaddr*)&ClientSin, sizeof(ClientSin));

        if (SizeWritten != Size)
        {
            if (SizeWritten < 0)
                LastNetError = GetLastSocketError();
            else
                LastNetError = WSAETIMEDOUT;
        }
    }
    else
        if (LastNetError == 0)
            LastNetError = WSAETIMEDOUT;

    return LastNetError;
}
//---------------------------------------------------------------------------
int TUdpSocket::SckBind()
{
    int Res;
    LastNetError = SetSin(LocalSin, LocalAddress, LocalPort);
    if (LastNetError == 0)
    {
        CreateSocket();
        if (LastNetError == 0)
        {
            Res = bind(FSocket, (struct sockaddr*)&LocalSin, sizeof(sockaddr_in));
            SockCheck(Res);
            if (Res == 0)
            {
                LocalBind = LocalSin.sin_addr.s_addr;
            }
        }
    }
    else
        LastNetError = WSAEINVALIDADDRESS;

    return LastNetError;
}

TUdpSocket::~TUdpSocket()
{
    DestroySocket();
}
