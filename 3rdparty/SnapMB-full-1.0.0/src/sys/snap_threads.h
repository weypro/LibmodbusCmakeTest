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
#ifndef snap_threads_h
#define snap_threads_h
//---------------------------------------------------------------------------
#include "snap_platform.h"

#ifdef SNAP_OS_WINDOWS
# include "win_threads.h"
#endif
#if defined(POSIX) && (!defined(OS_SOLARIS_NATIVE_THREADS))
# include "unix_threads.h"
#endif
#if defined(OS_OSX)
# include "unix_threads.h"
#endif

//---------------------------------------------------------------------------
#endif // snap_threads_h
