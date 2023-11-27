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
#ifndef snap_platform_h
#define snap_platform_h
//---------------------------------------------------------------------------
#if defined (_WIN32)|| defined(_WIN64)|| defined(__WIN32__) || defined(__WINDOWS__)
# define SNAP_OS_WINDOWS
#endif

// Visual Studio needs this to use the correct time_t size
#if defined (_WIN32) && !defined(_WIN64) && !defined(_EMBEDDING_VS2013UP)
  # define _USE_32BIT_TIME_T 
#endif

// Linux, BSD and Solaris define "unix", OSX doesn't, even though it derives from BSD
#if defined(unix) || defined(__unix__) || defined(__unix)
# define PLATFORM_UNIX
#endif

#if defined(__NetBSD__) || defined (__FreeBSD__) || defined(__OpenBSD__) || defined(__bsdi__) || defined(__DragonFly__)
# define OS_BSD
#endif

// Specific Linux
#if defined (__linux__)
#define OS_LINUX
#endif

#if __APPLE__
# define OS_OSX
#endif

#if defined(PLATFORM_UNIX)
# include <unistd.h>
# include <sys/param.h>
# if defined(_POSIX_VERSION)
#   define POSIX
# endif
#endif

#ifdef OS_OSX
# include <unistd.h>
#endif

#if (!defined (SNAP_OS_WINDOWS)) && (!defined(PLATFORM_UNIX)) && (!defined(OS_BSD)) && (!defined(OS_OSX))
# error platform still unsupported (please add it yourself and report ;-)
#endif

#include <stdint.h>
#include <time.h>
#include <cstring>
#include <stdlib.h>

#ifdef SNAP_OS_WINDOWS
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
# include <winsock2.h>
# include <mmsystem.h>
#endif

#if defined(PLATFORM_UNIX) || defined(OS_OSX)
# include <errno.h>
# include <sys/time.h>
# include <sys/socket.h>
# include <arpa/inet.h>
# include <netinet/tcp.h>
# include <netinet/in.h>
# include <sys/ioctl.h>
#endif

#if defined(PLATFORM_UNIX) || defined(OS_OSX) || defined (OS_BSD)
# include <termios.h>
# include <fcntl.h>
# include <sys/types.h>
# include <sys/stat.h>
//# include <sys/limits.h>
# include <sys/file.h>
#endif

#ifdef SNAP_OS_WINDOWS
# define EXPORTSPEC extern "C" __declspec ( dllexport )
# define SNAP_API __stdcall
#else
# define EXPORTSPEC extern "C"
# define SNAP_API
#endif

// Exact length types regardless of platform/processor
// We absolute need of them, all structs have an exact size that
// must be the same across the processor used 32/64 bit

// *Use them* if you change/expand the code and avoid long, u_long and so on...

typedef uint8_t    byte;
typedef uint16_t   word;
typedef uint32_t   longword;
typedef byte       *pbyte;
typedef word       *pword;
typedef uintptr_t  snap_obj; // multi platform/processor object reference

#ifndef SNAP_OS_WINDOWS
# define INFINITE  0XFFFFFFFF
#endif


#endif // snap_platform_h
