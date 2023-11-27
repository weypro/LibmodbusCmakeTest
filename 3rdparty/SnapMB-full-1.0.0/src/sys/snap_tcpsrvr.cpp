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
#include "snap_tcpsrvr.h"
//---------------------------------------------------------------------------
// WORKER THREAD
//---------------------------------------------------------------------------
TMsgWorkerThread::TMsgWorkerThread(TTcpSocket *Socket, TCustomTcpServer *Server)
{
    Index = 0;
    FreeOnTerminate = true;
    WorkerSocket = Socket;
    FServer = Server;
}
//---------------------------------------------------------------------------
void TMsgWorkerThread::Execute()
{
    bool Exception = false;
    bool SelfClose = false;
    int WorkResult = WorkerContinue;
    // Working loop
    while (!Terminated && !SelfClose && !Exception && !FServer->Destroying)
    {
        try
        {
            WorkResult = WorkerSocket->Execute();
        } catch (...)
        {
            Exception = true;
        }
        SelfClose = WorkResult > 0;
    };
    if (!FServer->Destroying)
    {
        // Exception detected during Worker activity
        if (Exception)
        {
            WorkerSocket->ForceClose();
            FServer->DoEvent(WorkerSocket->ClientHandle, evcClientException, 0, 0, 0, 0, 0);
        }
        else
        {
            if (SelfClose)
            {
                switch (WorkResult)
                {
                case WorkerTerminate:
                    FServer->DoEvent(WorkerSocket->ClientHandle, evcClientDisconnected, 0, 0, 0, 0, 0);
                    break;
                case WorkerTimeout:
                    FServer->DoEvent(WorkerSocket->ClientHandle, evcClientDisTimeout, 0, 0, 0, 0, 0);
                    break;
                default:
                    FServer->DoEvent(WorkerSocket->ClientHandle, evcClientTerminated, 0, 0, 0, 0, 0);
                }
            }
            else // Terminated
                FServer->DoEvent(WorkerSocket->ClientHandle, evcClientTerminated, 0, 0, 0, 0, 0);
        }            
    }
    delete WorkerSocket;
    // Delete reference from list
    FServer->Delete(Index);
}
//---------------------------------------------------------------------------
// LISTENER THREAD
//---------------------------------------------------------------------------

TMsgListenerThread::TMsgListenerThread(TTcpSocket *Listener, TCustomTcpServer *Server)
{
    FServer = Server;
    FListener = Listener;
    FreeOnTerminate = false;
}
//---------------------------------------------------------------------------

void TMsgListenerThread::Execute()
{
    socket_t Sock;
    bool Valid;

    while (!Terminated)
    {
        if (FListener->CanRead(FListener->WorkInterval))
        {
            Sock = FListener->SckAccept(); // in any case we must accept
            Valid = Sock != INVALID_SOCKET;
            // check if we are not destroying
            if ((!Terminated) && (!FServer->Destroying))
            {
                if (Valid)
                    FServer->Incoming(Sock);
            }
            else
                if (Valid)
                    Msg_CloseSocket(Sock);
        };
    }
}
//---------------------------------------------------------------------------
// TCP SERVER
//---------------------------------------------------------------------------
TCustomTcpServer::TCustomTcpServer()
{
    strncpy(FLocalAddress, "0.0.0.0", 16);
    CSList = new TSnapCriticalSection();
    csEvent = new TSnapCriticalSection();
    FEventQueue = new TMsgEventQueue(MaxEvents, sizeof (TSrvEvent));
    memset(Workers, 0, sizeof (Workers));
    for (int i = 0; i < MaxWorkers; i++)
        Workers[i] = NULL;
    Status = SrvStopped;
    EventMask = 0xFFFFFFFF;
    LogMask = 0xFFFFFFFF;
    Destroying = false;
    FLastError = 0;
    ClientsCount = 0;
    LocalBind = 0;
    MaxClients = MaxWorkers;
    OnEvent = NULL;
    FUsrPtr = NULL;
}
//---------------------------------------------------------------------------
TCustomTcpServer::~TCustomTcpServer()
{
    Destroying = true;
    Stop();
    OnEvent = NULL;
    delete CSList;
    delete csEvent;
    delete FEventQueue;
}
//---------------------------------------------------------------------------
void TCustomTcpServer::LockList()
{
    CSList->Enter();
}
//---------------------------------------------------------------------------
void TCustomTcpServer::UnlockList()
{
    CSList->Leave();
}
//---------------------------------------------------------------------------
int TCustomTcpServer::FirstFree()
{
    int i;
    for (i = 0; i < MaxWorkers; i++)
    {
        if (Workers[i] == 0)
            return i;
    }
    return -1;
}
//---------------------------------------------------------------------------

int TCustomTcpServer::StartListener()
{
    int Result;
    // Creates the listener
    SockListener = new TTcpSocket();
    strncpy(SockListener->LocalAddress, FLocalAddress, 16);
    SockListener->LocalPort = LocalPort;
    // Binds
    Result = SockListener->SckBind();
    if (Result == 0)
    {
        LocalBind = SockListener->LocalBind;
        // Listen
        Result = SockListener->SckListen();
        if (Result == 0)
        {
            // Creates the Listener thread
            ServerThread = new TMsgListenerThread(SockListener, this);
            ServerThread->Start();
        }
        else
            delete SockListener;
    }
    else
        delete SockListener;

    return Result;
}
//---------------------------------------------------------------------------
void TCustomTcpServer::TerminateAll()
{
    int c;
    longword Elapsed;
    bool Timeout;

    if (ClientsCount > 0)
    {
        for (c = 0; c < MaxWorkers; c++)
        {
            if (Workers[c] != 0)
                PMsgWorkerThread(Workers[c])->Terminate();
        }
        // Wait for closing
        Elapsed = SysGetTick();
        Timeout = false;
        while (!Timeout && (ClientsCount > 0))
        {
            Timeout = DeltaTime(Elapsed) > WkTimeout;
            if (!Timeout)
                SysSleep(100);
        };
        if (ClientsCount > 0)
            KillAll(); // one o more threads are hanged
        ClientsCount = 0;
    }
}
//---------------------------------------------------------------------------
void TCustomTcpServer::KillAll()
{
    int c, cnt = 0;
    LockList();
    for (c = 0; c < MaxWorkers; c++)
    {
        if (Workers[c] != 0)
            try
            {
                PMsgWorkerThread(Workers[c])->Kill();
                PMsgWorkerThread(Workers[c])->WorkerSocket->ForceClose();
                delete PMsgWorkerThread(Workers[c]);
                Workers[c] = 0;
                cnt++;
            } catch (...)
            {
            };
    }
    UnlockList();
    DoEvent(0, evcClientsDropped, 0, word(cnt), 0, 0, 0);
}
//---------------------------------------------------------------------------
bool TCustomTcpServer::CanAccept(socket_t Socket)
{
    return ((MaxClients == 0) || (ClientsCount < MaxClients));
}
//---------------------------------------------------------------------------
void TCustomTcpServer::DoEvent(int Sender, longword Code, word RetCode, word Param1, word Param2, word Param3, word Param4)
{
    TSrvEvent SrvEvent;
    bool GoLog = (Code & LogMask) != 0;
    bool GoEvent = (Code & EventMask) != 0;

    if (!Destroying && (GoLog || GoEvent))
    {
        csEvent->Enter();

        time(&SrvEvent.EvtTime);
        SrvEvent.EvtSender = Sender;
        SrvEvent.EvtCode = Code;
        SrvEvent.EvtRetCode = RetCode;
        SrvEvent.EvtParam1 = Param1;
        SrvEvent.EvtParam2 = Param2;
        SrvEvent.EvtParam3 = Param3;
        SrvEvent.EvtParam4 = Param4;

        if (GoEvent && (OnEvent != NULL))
            try
            { // callback is outside here, we have to shield it
                OnEvent(FUsrPtr, &SrvEvent, sizeof (TSrvEvent));
            } catch (...)
            {
            };

        if (GoLog)
            FEventQueue->Insert(&SrvEvent);

        csEvent->Leave();
    };
}
//---------------------------------------------------------------------------
void TCustomTcpServer::DoEvent(PSrvEvent Event)
{
    DoEvent(Event->EvtSender, Event->EvtCode, Event->EvtRetCode, Event->EvtParam1, Event->EvtParam2, Event->EvtParam3, Event->EvtParam4);
}
//---------------------------------------------------------------------------
void TCustomTcpServer::Delete(int Index)
{
    LockList();
    Workers[Index] = 0;
    ClientsCount--;
    UnlockList();
}
//---------------------------------------------------------------------------
void TCustomTcpServer::Incoming(socket_t Sock)
{
    int idx;
    PWorkerSocket WorkerSocket;
    longword ClientHandle = Msg_GetSockAddr(Sock);

    if (CanAccept(Sock))
    {
        LockList();
        // First position available in the thread buffer
        idx = FirstFree();
        if (idx >= 0)
        {
            // Creates the Worker and assigns it the connected socket
            WorkerSocket = CreateWorkerSocket(Sock);
            // Creates the Worker thread
            Workers[idx] = new TMsgWorkerThread(WorkerSocket, this);
            PMsgWorkerThread(Workers[idx])->Index = idx;
            // Update the number
            ClientsCount++;
            // And Starts the worker
            PMsgWorkerThread(Workers[idx])->Start();
            DoEvent(WorkerSocket->ClientHandle, evcClientAdded, 0, 0, 0, 0, 0);
        }
        else
        {
            DoEvent(ClientHandle, evcClientNoRoom, 0, 0, 0, 0, 0);
            Msg_CloseSocket(Sock);
        }
        UnlockList();
    }
    else
    {
        Msg_CloseSocket(Sock);
        DoEvent(ClientHandle, evcClientRejected, 0, 0, 0, 0, 0);
    };
}
//---------------------------------------------------------------------------
int TCustomTcpServer::Start()
{
    int Result = 0;
    if (Status != SrvRunning)
    {
        Result = StartListener();
        if (Result != 0)
        {
            DoEvent(0, evcListenerCannotStart, word(Result), 0, 0, 0, 0);
            Status = SrvError;
        }
        else
        {
            DoEvent(LocalBind, evcServerStarted, 0, LocalPort, 0, 0, 0);
            Status = SrvRunning;
        };
    };
    FLastError = Result;
    return Result;
}
//---------------------------------------------------------------------------
int TCustomTcpServer::StartTo(const char *Address, word Port)
{
    strncpy(FLocalAddress, Address, 15);
    LocalPort = Port;
    return Start();
}
//---------------------------------------------------------------------------
void TCustomTcpServer::Stop()
{
    if (Status == SrvRunning)
    {
        // Kills the listener thread
        ServerThread->Terminate();
        if (ServerThread->WaitFor(ThTimeout) != WAIT_OBJECT_0)
            ServerThread->Kill();
        delete ServerThread;
        // Kills the listener
        delete SockListener;

        // Terminate all client threads
        TerminateAll();

        Status = SrvStopped;
        LocalBind = 0;
        DoEvent(0, evcServerStopped, 0, 0, 0, 0, 0);
    };
    FLastError = 0;
}
//---------------------------------------------------------------------------
int TCustomTcpServer::SetEventsCallBack(void* PCallBack, void *UsrPtr)
{
    OnEvent = pfn_SrvCallBack(PCallBack);
    FUsrPtr = UsrPtr;
    return 0;
}
//---------------------------------------------------------------------------
bool TCustomTcpServer::PickEvent(void *pEvent)
{
    try
    {
        return FEventQueue->Extract(pEvent);
    } catch (...)
    {
        return false;
    };
}
//---------------------------------------------------------------------------
bool TCustomTcpServer::EventEmpty()
{
    return FEventQueue->Empty();
}
//---------------------------------------------------------------------------
void TCustomTcpServer::EventsFlush()
{
    csEvent->Enter();
    FEventQueue->Flush();
    csEvent->Leave();
}
//---------------------------------------------------------------------------


