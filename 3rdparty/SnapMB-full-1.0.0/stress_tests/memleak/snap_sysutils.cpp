/*=============================================================================|
|  PROJECT SNAP7                                                         1.3.0 |
|==============================================================================|
|  Copyright (C) 2013, 2015 Davide Nardella                                    |
|  All rights reserved.                                                        |
|==============================================================================|
|  SNAP7 is free software: you can redistribute it and/or modify               |
|  it under the terms of the Lesser GNU General Public License as published by |
|  the Free Software Foundation, either version 3 of the License, or           |
|  (at your option) any later version.                                         |
|                                                                              |
|  It means that you can distribute your commercial software linked with       |
|  SNAP7 without the requirement to distribute the source code of your         |
|  application and without the requirement that your application be itself     |
|  distributed under LGPL.                                                     |
|                                                                              |
|  SNAP7 is distributed in the hope that it will be useful,                    |
|  but WITHOUT ANY WARRANTY; without even the implied warranty of              |
|  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the               |
|  Lesser GNU General Public License for more details.                         |
|                                                                              |
|  You should have received a copy of the GNU General Public License and a     |
|  copy of Lesser GNU General Public License along with Snap7.                 |
|  If not, see  http://www.gnu.org/licenses/                                   |
|=============================================================================*/

#include "snap_sysutils.h"

#ifdef OS_OSX
int clock_gettime(int clk_id, struct timespec* t)
{
    struct timeval now;
    int rv = gettimeofday(&now, NULL);
    if (rv) return rv;
    t->tv_sec  = now.tv_sec;
    t->tv_nsec = now.tv_usec * 1000;
    return 0;
}
#endif

//---------------------------------------------------------------------------
longword SysGetTick()
{
#ifdef SNAP_OS_WINDOWS
    return timeGetTime();

    // GetTickCount64();
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (longword) (ts.tv_sec * 1000) + (ts.tv_nsec / 1000000);
#endif
}
//---------------------------------------------------------------------------
void SysSleep(longword Delay_ms)
{
#ifdef SNAP_OS_WINDOWS
	Sleep(Delay_ms);
#else
    struct timespec ts;
    ts.tv_sec = (time_t)(Delay_ms / 1000);
    ts.tv_nsec =(long)((Delay_ms - ts.tv_sec) * 1000000);
    nanosleep(&ts, (struct timespec *)0);
#endif
}
//---------------------------------------------------------------------------
longword DeltaTime(longword &Elapsed)
{
    longword TheTime;
    TheTime=SysGetTick();
    // Checks for rollover
    if (TheTime >= Elapsed)
        return TheTime - Elapsed;
    else
        return (0xFFFFFFFF - Elapsed) + TheTime;
}
//---------------------------------------------------------------------------
word SwapWord(word Value)
{
    return ((Value >> 8) & 0xFF) | ((Value << 8) & 0xFF00);
}
//---------------------------------------------------------------------------
longword SwapDWord(longword Value)
{
    return (Value >> 24) | ((Value << 8) & 0x00FF0000) | ((Value >> 8) & 0x0000FF00) | (Value << 24);
}

