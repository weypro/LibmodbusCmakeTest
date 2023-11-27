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
#include "snap_evtqueue.h"

//---------------------------------------------------------------------------
// EVENTS QUEUE
//---------------------------------------------------------------------------

TMsgEventQueue::TMsgEventQueue(const int Capacity, const int BlockSize)
{
    FCapacity = Capacity;
    Max = FCapacity - 1;
    FBlockSize = BlockSize;
    Buffer = new byte[uintptr_t(FCapacity) * uintptr_t(FBlockSize)];
    Flush();
}
//---------------------------------------------------------------------------
TMsgEventQueue::~TMsgEventQueue()
{
    delete[] Buffer;
}
//---------------------------------------------------------------------------
void TMsgEventQueue::Flush()
{
    IndexIn = 0;
    IndexOut = 0;
}
//---------------------------------------------------------------------------
void TMsgEventQueue::Insert(void* lpdata)
{
    pbyte PBlock;
    if (!Full())
    {
        // Calc offset
        if (IndexIn < Max) IndexIn++;
        else IndexIn = 0;
        PBlock = Buffer + (uintptr_t(IndexIn) * uintptr_t(FBlockSize));
        memcpy(PBlock, lpdata, FBlockSize);
    };
}
//---------------------------------------------------------------------------
bool TMsgEventQueue::Extract(void* lpdata)
{
    int IdxOut;
    pbyte PBlock;

    if (!Empty())
    {
        // stores IndexOut
        IdxOut = IndexOut;
        if (IdxOut < Max) IdxOut++;
        else IdxOut = 0;
        PBlock = Buffer + (uintptr_t(IdxOut) * uintptr_t(FBlockSize));
        // moves data
        memcpy(lpdata, PBlock, FBlockSize);
        // Updates IndexOut
        IndexOut = IdxOut;
        return true;
    }
    else
        return false;
}
//---------------------------------------------------------------------------
bool TMsgEventQueue::Empty()
{
    return (IndexIn == IndexOut);
}
//---------------------------------------------------------------------------
bool TMsgEventQueue::Full()
{
    int IdxOut = IndexOut; // To avoid troubles if IndexOut changes during next line
    return ((IdxOut == IndexIn + 1) || ((IndexIn == Max) && (IdxOut == 0)));
}
