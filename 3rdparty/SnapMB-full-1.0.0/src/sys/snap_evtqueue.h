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
#ifndef snap_evtqueue_h
#define snap_evtqueue_h
//---------------------------------------------------------------------------
#include "snap_platform.h"

#pragma pack(1)

typedef struct {
    time_t EvtTime;    // Timestamp
    int EvtSender;     // Sender
    longword EvtCode;  // Event code
    word EvtRetCode;   // Event result
    word EvtParam1;    // Param 1 (if available)
    word EvtParam2;    // Param 2 (if available)
    word EvtParam3;    // Param 3 (if available)
    word EvtParam4;    // Param 4 (if available)
}TSrvEvent, * PSrvEvent;

#pragma pack()

//---------------------------------------------------------------------------
// EVENTS QUEUE
//---------------------------------------------------------------------------
class TMsgEventQueue
{
private:
    int   IndexIn;   // <-- insert index
    int   IndexOut;  // --> extract index
    int   Max;       // Buffer upper bound [0..Max]
    int   FCapacity; // Queue capacity
    pbyte Buffer;
    int   FBlockSize;
public:
    TMsgEventQueue(const int Capacity, const int BlockSize);
    ~TMsgEventQueue();
    void Flush();
    void Insert(void* lpdata);
    bool Extract(void* lpdata);
    bool Empty();
    bool Full();
};
typedef TMsgEventQueue* PMsgEventQueue;






















#endif // snap_evtqueue_h
