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
#ifndef snap_tcpsrvr_h
#define snap_tcpsrvr_h
//---------------------------------------------------------------------------
#include "snap_msgsock.h"
#include "snap_threads.h"
#include "snap_evtqueue.h"
//---------------------------------------------------------------------------

#define MaxWorkers 1024
#define MaxEvents  1500

const int SrvStopped = 0;
const int SrvRunning = 1;
const int SrvError   = 2;

const longword evcServerStarted       = 0x00000001;
const longword evcServerStopped       = 0x00000002;
const longword evcListenerCannotStart = 0x00000004;
const longword evcClientAdded         = 0x00000008;
const longword evcClientRejected      = 0x00000010;
const longword evcClientNoRoom        = 0x00000020;
const longword evcClientException     = 0x00000040;
const longword evcClientDisconnected  = 0x00000080;
const longword evcClientTerminated    = 0x00000100;
const longword evcClientsDropped      = 0x00000200;
const longword evcClientRefused       = 0x00000400;
const longword evcClientDisTimeout    = 0x00000800;
const longword evcReserved_00001000   = 0x00001000;
const longword evcReserved_00002000   = 0x00002000;
const longword evcReserved_00004000   = 0x00004000;
const longword evcReserved_00008000   = 0x00008000;

// Server Interface errors
const longword errSrvBase             = 0x0000FFFF;
const longword errSrvMask             = 0xFFFF0000;
const longword errSrvCannotStart      = 0x00100000;

const longword ThTimeout   = 2000; // Thread timeout
const longword WkTimeout   = 3000; // Workers termination timeout

const int WorkerContinue  = 0;
const int WorkerTerminate = 1;
const int WorkerTimeout   = 2;

extern "C"
{
typedef void (SNAP_API *pfn_SrvCallBack)(void * usrPtr, void* PEvent, int Size);
}

//---------------------------------------------------------------------------
// WORKER THREAD
//---------------------------------------------------------------------------
class TCustomTcpServer; // forward declaration

// It's created when connection is accepted, it will interface with the client.
class TMsgWorkerThread : public TSnapThread
{
private:
        TCustomTcpServer *FServer;
protected:
        TTcpSocket *WorkerSocket;
public:
        int Index;
        friend class TCustomTcpServer;
        TMsgWorkerThread(TTcpSocket *Socket, TCustomTcpServer *Server);
        void Execute();
};
typedef TMsgWorkerThread *PMsgWorkerThread;

//---------------------------------------------------------------------------
// LISTENER THREAD
//---------------------------------------------------------------------------
// It listens for incoming connection.
class TMsgListenerThread : public TSnapThread
{
private:
        TTcpSocket *FListener;
        TCustomTcpServer *FServer;
public:
        TMsgListenerThread(TTcpSocket *Listener, TCustomTcpServer *Server);
        void Execute();
};
typedef TMsgListenerThread *PMsgListenerThread;

//---------------------------------------------------------------------------
// TCP SERVER
//---------------------------------------------------------------------------
typedef TTcpSocket *PWorkerSocket;

class TCustomTcpServer
{
private:
        int FLastError;
        // Socket listener
        PTcpSocket SockListener;
        // Server listener
        PMsgListenerThread ServerThread;
        // Critical section to lock Workers list activities
        PSnapCriticalSection CSList;
        // Event queue
        PMsgEventQueue FEventQueue;
        // Callback related
        pfn_SrvCallBack OnEvent;
        void *FUsrPtr;
        // private methods
        int StartListener();
        void LockList();
        void UnlockList();
        int FirstFree();
protected:
        char FLocalAddress[16];
        bool Destroying;
        // Critical section to lock Event activities
        PSnapCriticalSection csEvent;
	    // Workers list
        void *Workers[MaxWorkers];
        // Terminates all worker threads
        virtual void TerminateAll();
        // Kills all worker threads that are unresponsive
        void KillAll();
        // if (true the connection is accepted, otherwise the connection
        // is closed gracefully
        virtual bool CanAccept(socket_t Socket);
        // Returns the class of the worker socket, override it for real servers
        virtual PWorkerSocket CreateWorkerSocket(socket_t Sock) = 0;
        // Handles the event
        virtual void DoEvent(int Sender, longword Code, word RetCode, word Param1, word Param2, word Param3, word Param4);
        virtual void DoEvent(PSrvEvent Event);
        // Delete the worker from the list (It's invoked by Worker Thread)
        void Delete(int Index);
        // Incoming connection (It's invoked by ServerThread, the listener)
        virtual void Incoming(socket_t Sock);
public:
        friend class TMsgWorkerThread;
        friend class TMsgListenerThread;
        word LocalPort;
        longword LocalBind;
        longword LogMask;
        longword EventMask;
        int Status;
        int ClientsCount;
        int MaxClients;
        TCustomTcpServer();
        virtual ~TCustomTcpServer();
        // Starts the server
        int Start();
        int StartTo(const char *Address, word Port);
        // Stops the server
        void Stop();
        // Sets Event callback
        int SetEventsCallBack(void* PCallBack, void *UsrPtr);
        // Pick an event from the circular queue
        bool PickEvent(void *pEvent);
        // Returns true if the Event queue is empty
        bool EventEmpty();
        // Flushes Event queue
        void EventsFlush();
};

//---------------------------------------------------------------------------
// TCP WORKER
//---------------------------------------------------------------------------
class TEcoTcpWorker : public TTcpSocket
{
public:
	int Execute()
	{
        return 1;
	};
};

//---------------------------------------------------------------------------
#endif // snap_tcpsrvr_h
