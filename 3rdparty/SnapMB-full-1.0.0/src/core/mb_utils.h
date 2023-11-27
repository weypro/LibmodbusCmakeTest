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
#ifndef mb_utils_h
#define mb_utils_h
//-----------------------------------------------------------------------------
#include "mb_defines.h"

void CopyAndSwapReg16Pack(PRegisters16 Dst, PRegisters16 Src, int Amount);
void SwapReg16Pack(PRegisters16 RegPack, int Amount);
void SwapReg32Pack(PRegisters32 RegPack, int Amount);
void SwapReg64Pack(PRegisters64 RegPack, int Amount);

int RoundToNextByte(int Amount);

void UnpackBits(uint8_t* Array, uint8_t* Pack, int Amount);
void PackBits(uint8_t* Pack, uint8_t *Array, int Amount);

void CalcTimeout(longword Elapsed, PDeviceParams Params);
void ClearTimeout(PDeviceParams Params);
void SetDeviceDefaults(TDeviceParams& Params);
uint16_t CalcCRC(uint8_t* Buf, uint16_t Length);
bool CheckCRC16(pbyte Data, word Size);
byte nibble(char V);

//-----------------------------------------------------------------------------
#endif // mb_utils_h
