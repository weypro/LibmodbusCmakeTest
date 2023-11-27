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
#include "mb_text.h"
//------------------------------------------------------------------------------
const char* SocketTextOf(int Error)
{
	switch (Error)
	{
	case 0:                   return "\0";
	case WSAEINTR:            return "TCP/UDP: Interrupted system call\0";
	case WSAEBADF:            return "TCP/UDP: Bad file number\0";
	case WSAEACCES:           return "TCP/UDP: Permission denied\0";
	case WSAEFAULT:           return "TCP/UDP: Bad address\0";
	case WSAEINVAL:           return "TCP/UDP: Invalid argument\0";
	case WSAEMFILE:           return "TCP/UDP: Too many open files\0";
	case WSAEWOULDBLOCK:      return "TCP/UDP: Operation would block\0";
	case WSAEINPROGRESS:      return "TCP/UDP: Operation now in progress\0";
	case WSAEALREADY:         return "TCP/UDP: Operation already in progress\0";
	case WSAENOTSOCK:         return "TCP/UDP: Socket operation on non socket\0";
	case WSAEDESTADDRREQ:     return "TCP/UDP: Destination address required\0";
	case WSAEMSGSIZE:         return "TCP/UDP: Message too long\0";
	case WSAEPROTOTYPE:       return "TCP/UDP: Protocol wrong type for Socket\0";
	case WSAENOPROTOOPT:      return "TCP/UDP: Protocol not available\0";
	case WSAEPROTONOSUPPORT:  return "TCP/UDP: Protocol not supported\0";
	case WSAESOCKTNOSUPPORT:  return "TCP/UDP: Socket not supported\0";
	case WSAEOPNOTSUPP:       return "TCP/UDP: Operation not supported on Socket\0";
	case WSAEPFNOSUPPORT:     return "TCP/UDP: Protocol family not supported\0";
	case WSAEAFNOSUPPORT:     return "TCP/UDP: Address family not supported\0";
	case WSAEADDRINUSE:       return "TCP/UDP: Address already in use\0";
	case WSAEADDRNOTAVAIL:    return "TCP/UDP: Can't assign requested address\0";
	case WSAENETDOWN:         return "TCP/UDP: Network is down\0";
	case WSAENETUNREACH:      return "TCP/UDP: Network is unreachable\0";
	case WSAENETRESET:        return "TCP/UDP: Network dropped connection on reset\0";
	case WSAECONNABORTED:     return "TCP/UDP: Software caused connection abort\0";
	case WSAECONNRESET:       return "TCP/UDP: Connection reset by peer\0";
	case WSAENOBUFS:          return "TCP/UDP: No Buffer space available\0";
	case WSAEISCONN:          return "TCP/UDP: Socket is already connected\0";
	case WSAENOTCONN:         return "TCP/UDP: Socket is not connected\0";
	case WSAESHUTDOWN:        return "TCP/UDP: Can't send after Socket shutdown\0";
	case WSAETOOMANYREFS:     return "TCP/UDP: Too many references:can't splice\0";
	case WSAETIMEDOUT:        return "TCP/UDP: Connection timed out\0";
	case WSAECONNREFUSED:     return "TCP/UDP: Connection refused\0";
	case WSAELOOP:            return "TCP/UDP: Too many levels of symbolic links\0";
	case WSAENAMETOOLONG:     return "TCP/UDP: File name is too long\0";
	case WSAEHOSTDOWN:        return "TCP/UDP: Host is down\0";
	case WSAEHOSTUNREACH:     return "TCP/UDP: Unreachable peer\0";
	case WSAENOTEMPTY:        return "TCP/UDP: Directory is not empty\0";
	case WSAEUSERS:           return "TCP/UDP: Too many users\0";
	case WSAEDQUOT:           return "TCP/UDP: Disk quota exceeded\0";
	case WSAESTALE:           return "TCP/UDP: Stale NFS file handle\0";
	case WSAEREMOTE:          return "TCP/UDP: Too many levels of remote in path\0";
#ifdef SNAP_OS_WINDOWS
	case WSAEPROCLIM:         return "TCP/UDP: Too many processes\0";
	case WSASYSNOTREADY:      return "TCP/UDP: Network subsystem is unusable\0";
	case WSAVERNOTSUPPORTED:  return "TCP/UDP: Winsock DLL cannot support this application\0";
	case WSANOTINITIALISED:   return "TCP/UDP: Winsock not initialized\0";
	case WSAEDISCON:          return "TCP/UDP: Disconnect\0";
	case WSAHOST_NOT_FOUND:   return "TCP/UDP: Host not found\0";
	case WSATRY_AGAIN:        return "TCP/UDP: Non authoritative - host not found\0";
	case WSANO_RECOVERY:      return "TCP/UDP: Non recoverable error\0";
	case WSANO_DATA:          return "TCP/UDP: Valid name, no data record of requested type\0";
#endif
	case WSAEINVALIDADDRESS:  return "TCP/UDP: Invalid address\0";
	default:
		return "TCP/UDP: Other Socket Error ";
	}
}
//---------------------------------------------------------------------------
const char* TxtModbusError(int Exception)
{
	switch (Exception)
	{
	case evrNoError: return "OK";		
	case errIllegalFunction: return "ERR 0x01 Illegal Function";		
	case errIllegalDataAddress: return "ERR 0x02 Illegal Data Address";		
	case errIllegalDataValue: return "ERR 0x03 Illegal Data Value";		
	case errSlaveDeviceFailure: return "ERR 0x04 Device Failure";		
	case errAcknowledge: return "ERR 0x05 Acknowledge";		
	case errSlaveDeviceBusy: return "ERR 0x06 Device Busy";		
	case errNegativeAcknowledge: return "ERR 0x07 Negative Acknowledge";		
	case errMemoryParityError: return "ERR 0x08 Memory Parity Error";		
	case errGatewayPathUnavailable: return "ERR 0x0A Gateway Path Unavailable";		
	case errGatewayTargetFailed: return "ERR 0x0B Gateway Target Device Resp.Failure";		
	default:
		return "Unknown Exception Code";
	};
}
//---------------------------------------------------------------------------
const char* SerialTextOf(int Error)
{
	switch (Error)
	{
	case 0: return "\0";
	case errPortInvalidParams:    return "SER : Invalid Port params supplied";
	case errOpeningPort:          return "SER : Error opening the Port (does not exists or it is already in use)";
	case errPortSettingsParams:   return "SER : Error setting Port params (check the values supplied)";
	case errPortSettingsTimeouts: return "SER : Error setting Port timeout (check the values supplied)";
	case errPortReadTimeout:      return "SER : Port Read Timeout";
	case errPortWriteTimeout:     return "SER : Error setting Port Write-timeout (check the values supplied)";
	case errPortReadError:        return "SER : Port Read error";
	case errPortWriteError:       return "SER : Port Write error";
    case errBufferOverflow:       return "SER : Read Buffer Overflow";
	case errPortGetParams:        return "SER : Port Get Params";
	case errPortLocked:           return "SER : Port Locked by another Process";
	case errInterframe:           return "SER : Cannot determine Interframe";
	default:
		return "";
	}
}

const char* LibraryTextOf(int Error)
{
	switch (Error)
	{
	case 0: return "\0";
	case errNullObject: return "The Object is null";
	case errObjectInvalidMethod: return "Invalid method for this Object";
	default:
		return "Unknown Library Error";
	}
}

const char* ProcessTextOf(int Error)
{
	switch (Error)
	{
	case errInvalidBroadcastFunction:
		return "Invalid Broadcast function (it's not a write function)";
	case errInvalidParamIndex:
		return "Invalid Param Index";
	case errInvalidAddress:
		return "Invalid Address supplied";
	case errInvalidDataAmount:
		return "Invalid Data amount";
	case errInvalidADUReceived:
		return "Invalid ADU received (malformed ADU or fields mismatch)";
	case errInvalidChecksum:
		return "Invalid Checksum received (CRC or LRC)";
	case errTimeout:
		return "Device Timeout";
	case errInvalidDeviceID:
		return "Invalid Device ID";
	case errInvalidUserFunction:
		return "Invalid User Function (must be smaller than 0x80)";
	case errInvalidReqForThisObject:
		return "Invalid Request for this Object";
	case errUndefinedBroker:
		return "Undefined Broker to handle this Device";
	case errUndefinedClient:
		return "Undefined Client (NULL Reference)";
	case errDeviceIDZero:
		return "Invalid DeviceID Index (must be >0)";
	case errDeviceAlreadyExists:
		return "A Client with this index was already defined";
	case errUndefinedController:
		return "Undefined Controller (NULL Reference)";
	case errSomeConnectionsError:
		return "Some connections error (get the status for detail)";
	case errCommParamsMismatch:
		return "Comm params mismatch (Same port but different params)";
	case errDevUnknownAreaID:
		return "Unknown Area ID";
	case errDevAreaZero:
		return "Area Amount = 0";
	case errDevAreaTooWide:
		return "Area Amount too wide";
	case errDevUnknownCallbackID:
		return "Unknown Callback ID";
	case errDevInvalidParams:
		return "Invalid Param supplied";
	case errDevInvalidParamIndex:
		return "Invalid Param Index";
	case errDevOpNotAllowed:
		return "Operation not allowed while Device is running";
	case errDevTooManyPeers:
		return "Too many peers for the Allow/Deny List";
	case errDevCannotRebindOnRun:
		return "Device is running : cannot Rebind, stop it first";
	default:
		return "Unknown Process Error";
	}
}

//******************************************************************************
// Error Text
//******************************************************************************
const char* _ErrorText(int Error, char* Text, int TextLen)
{
	char S[512];
	char Buf[128];
	memset(S, 0, 512);

	if (Error == 0)
	{
		strncpy(Text, "OK", TextLen);
		return Text;
	}

	int ErrObject = Error & MaskObjects;
	int Category = Error & MaskCategory;
	int ErrNo  = Error & MaskErrNo;

	switch (ErrObject)
	{
	case errLibrary: 
		strcpy(S, "LIB : ");
		break;
	case errSerialClient:
		strcpy(S, "SERCLI : ");
		break;
	case errEthernetClient:
		strcpy(S, "ETHCLI : ");
		break;
	case errFieldController:
		strcpy(S, "ETHCTRL : ");
		break;
	case errSerialDevice:
		strcpy(S, "SERDEV : ");
		break;
	case errEthernetDevice:
		strcpy(S, "ETHDEV : ");
		break;
	default:
		strcpy(S, "OBJ??");
	}

	switch (Category)
	{
	// Library Errors
	case errCategoryLibrary:
		strcat(S, LibraryTextOf(ErrNo));
		break;
	// Serial Socket Errors
	case errCategorySerialSocket:
		strcat(S, SerialTextOf(ErrNo));
		break;
	case errCategoryNetSocket:
		strcat(S, SocketTextOf(ErrNo));
		break;
	case errCategoryMBProtocol:
		strcat(S, TxtModbusError(ErrNo));
		break;
	case errCategoryProcess:
		strcat(S, ProcessTextOf(ErrNo));
		break;
	default:
		snprintf(Buf, sizeof(Buf), "Unknown Error category (0x%X)", Category);
		strcat(S, Buf);
	}

	strncpy(Text, S, TextLen);
	return Text;
}

//******************************************************************************
// Event Text
//******************************************************************************
const char* IpAddress(int IP)
{
	in_addr Addr;
	Addr.s_addr = IP;
	return inet_ntoa(Addr);
}
//------------------------------------------------------------------------------
const char* ExceptionText(word RetCode)
{
	switch (RetCode)
	{
	case evrNoError:
		return " --> OK";
	case errIllegalFunction:
		return " --> ERR 0x01 Illegal Function";
	case errIllegalDataAddress:
		return " --> ERR 0x02 Illegal Data Address";
	case errIllegalDataValue:
		return " --> ERR 0x03 Illegal Data Value";
	case errSlaveDeviceFailure:
		return " --> ERR 0x04 Device Failure";
	case errAcknowledge:
		return " --> ERR 0x05 Acknowledge";
	case errSlaveDeviceBusy:
		return " --> ERR 0x06 Device Busy";
	case errNegativeAcknowledge:
		return " --> ERR 0x07 Negative Acknowledge";
	case errMemoryParityError:
		return " --> ERR 0x08 Memory Parity Error";
	case errGatewayPathUnavailable:
		return " --> ERR 0x0A Gateway Path Unavailable";
	case errGatewayTargetFailed:
		return " --> ERR 0x0B Gateway Target Device Resp.Failure";
	default:
		return " --> Unknown Exception Code";
	};
}

//------------------------------------------------------------------------------
const char* PTResult(word RetCode)
{
	switch (TIndicationResult(RetCode))
	{
	case TIndicationResult::irSkip:
		return "irSkip";
	case TIndicationResult::irDone:
		return "irDone";
	case TIndicationResult::irSendAnswer:
		return "irSendAnswer";
	case TIndicationResult::irNoAnswer:
		return "irNoAnswer";
	case TIndicationResult::irDisconnect:
		return "irDisconnect";
	default:
		return "Unknown Result";
	}
}
//------------------------------------------------------------------------------
const char* _EventText(void* pEvent, char* Text, int TextSize)
{
	PDeviceEvent Event = PDeviceEvent(pEvent);
	char Timestamp[24];

	struct tm* DateTime = localtime(&Event->EvtTime);

	if (DateTime != NULL)
		strftime(Timestamp, 23, "%Y-%m-%d %H:%M:%S -", DateTime);
	else
		Timestamp[0] = '\0';

	switch (Event->EvtCode)
	{
		// Device Base
	case evcDeviceStarted:
		if (Event->EvtSender == 0 && Event->EvtParam1 == 0)
			snprintf(Text, TextSize, "%s Device Started", Timestamp);
		else
			snprintf(Text, TextSize, "%s Device Started @ %s:%d", Timestamp, IpAddress(Event->EvtSender), Event->EvtParam1);
		break;
	case evcDeviceStopped:
		snprintf(Text, TextSize, "%s Device Stopped", Timestamp);
		break;
	case evcDeviceCannotStart:
		snprintf(Text, TextSize, "%s Device CANNOT start", Timestamp);
		break;
	case evcDevClientAdded:
		snprintf(Text, TextSize, "%s [%s] Client Added", Timestamp, IpAddress(Event->EvtSender));
		break;
	case evcDevClientRejected:
	case evcDevClientRefused:
		snprintf(Text, TextSize, "%s [%s] Client Rejected due to Allow/Block List policies", Timestamp, IpAddress(Event->EvtSender));
		break;
	case evcDevClientDisTimeout:
		snprintf(Text, TextSize, "%s [%s] Client disconnected due to Inactivity timeout", Timestamp, IpAddress(Event->EvtSender));
		break;
	case evcDevClientNoRoom:
		snprintf(Text, TextSize, "%s A client was refused due to maximum connections number", Timestamp);
		break;
	case evcDevClientException:
		snprintf(Text, TextSize, "%s [%s] Client Exception", Timestamp, IpAddress(Event->EvtSender));
		break;
	case evcDevClientDisconnected:
		snprintf(Text, TextSize, "%s [%s] Client disconnected by peer", Timestamp, IpAddress(Event->EvtSender));
		break;
	case evcDevClientTerminated:
		snprintf(Text, TextSize, "%s [%s] Client terminated", Timestamp, IpAddress(Event->EvtSender));
		break;
	case evcDevClientsDropped:
		snprintf(Text, TextSize, "%s %d Clients have been dropped beacuse unresponsive", Timestamp, Event->EvtParam1);
		break;
	case evcPortError:
		snprintf(Text, TextSize, "%s Com Port error %d (%s)", Timestamp, Event->EvtRetCode, SerialTextOf(Event->EvtRetCode));
		break;
	case evcPortReset:
		snprintf(Text, TextSize, "%s Com Port was reset due too many errors", Timestamp);
		break;
	case evcInvalidADUReceived:
		if (Event->EvtSender == 0)
			snprintf(Text, TextSize, "%s Invalid ADU received from the Controller", Timestamp);
		else
			snprintf(Text, TextSize, "%s [%s] Invalid ADU received", Timestamp, IpAddress(Event->EvtSender));
		break;
	case evcNetworkError:
		if (Event->EvtRetCode == WSAECONNRESET)
			snprintf(Text, TextSize, "%s [%s] Client disconnected by peer", Timestamp, IpAddress(Event->EvtSender));
		else
			snprintf(Text, TextSize, "%s [%s] Socket Error : %s", IpAddress(Event->EvtSender), Timestamp, SocketTextOf(Event->EvtRetCode));
		break;
	case evcCRCError:
		if (Event->EvtSender == 0)
			snprintf(Text, TextSize, "%s Invalid Packet CRC received from the Controller", Timestamp);
		else
			snprintf(Text, TextSize, "%s [%s] Invalid Packet CRC received", Timestamp, IpAddress(Event->EvtSender));
		break;
	case evcInvalidFunction:
		if (Event->EvtSender == 0)
			snprintf(Text, TextSize, "%s Invalid function received from the Controller [0x%02X]", Timestamp, Event->EvtParam1);
		else
			snprintf(Text, TextSize, "%s [%s] Invalid function received [0x%02X]", Timestamp, IpAddress(Event->EvtSender), Event->EvtParam1);
		break;
		// Functions
	case evcReadCoils:
		if (Event->EvtSender == 0)
			snprintf(Text, TextSize, "%s 0x01 - Read Coils - Addr : 0x%04X, Amount : %d %s", Timestamp, Event->EvtParam3, Event->EvtParam4, ExceptionText(Event->EvtRetCode));
		else
			snprintf(Text, TextSize, "%s [%s] 0x01 - Read Coils - Addr : 0x%04X, Amount : %d %s", Timestamp, IpAddress(Event->EvtSender), Event->EvtParam3, Event->EvtParam4, ExceptionText(Event->EvtRetCode));
		break;
	case evcReadDiscrInputs:
		if (Event->EvtSender == 0)
			snprintf(Text, TextSize, "%s 0x02 - Read Discrete Inputs - Addr : 0x%04X, Amount : %d %s", Timestamp, Event->EvtParam3, Event->EvtParam4, ExceptionText(Event->EvtRetCode));
		else
			snprintf(Text, TextSize, "%s [%s] 0x02 - Read Discrete Inputs - Addr : 0x%04X, Amount : %d %s", Timestamp, IpAddress(Event->EvtSender), Event->EvtParam3, Event->EvtParam4, ExceptionText(Event->EvtRetCode));
		break;
	case evcReadHoldingRegs:
		if (Event->EvtSender == 0)
			snprintf(Text, TextSize, "%s 0x03 - Read Holding Registers - Addr : 0x%04X, Amount : %d %s", Timestamp, Event->EvtParam3, Event->EvtParam4, ExceptionText(Event->EvtRetCode));
		else
			snprintf(Text, TextSize, "%s [%s] 0x03 - Read Holding Registers - Addr : 0x%04X, Amount : %d %s", Timestamp, IpAddress(Event->EvtSender), Event->EvtParam3, Event->EvtParam4, ExceptionText(Event->EvtRetCode));
		break;
	case evcReadInputRegs:
		if (Event->EvtSender == 0)
			snprintf(Text, TextSize, "%s 0x04 - Read Input Registers - Addr : 0x%04X, Amount : %d %s", Timestamp, Event->EvtParam3, Event->EvtParam4, ExceptionText(Event->EvtRetCode));
		else
			snprintf(Text, TextSize, "%s [%s] 0x04 - Read Input Registers - Addr : 0x%04X, Amount : %d %s", Timestamp, IpAddress(Event->EvtSender), Event->EvtParam3, Event->EvtParam4, ExceptionText(Event->EvtRetCode));
		break;
	case evcWriteSingleCoil:
		if (Event->EvtSender == 0)
			snprintf(Text, TextSize, "%s 0x05 - Write Single Coil - Addr : 0x%04X, Value : 0x%04X %s", Timestamp, Event->EvtParam3, Event->EvtParam4, ExceptionText(Event->EvtRetCode));
		else
			snprintf(Text, TextSize, "%s [%s] 0x05 - Write Single Coil - Addr : 0x%04X, Value : 0x%04X %s", Timestamp, IpAddress(Event->EvtSender), Event->EvtParam3, Event->EvtParam4, ExceptionText(Event->EvtRetCode));
		break;
	case evcWriteSingleReg:
		if (Event->EvtSender == 0)
			snprintf(Text, TextSize, "%s 0x06 - Write Single Register - Addr : 0x%04X, Value : 0x%04X %s", Timestamp, Event->EvtParam3, Event->EvtParam4, ExceptionText(Event->EvtRetCode));
		else
			snprintf(Text, TextSize, "%s [%s] 0x06 - Write Single Register - Addr : 0x%04X, Value : 0x%04X %s", Timestamp, IpAddress(Event->EvtSender), Event->EvtParam3, Event->EvtParam4, ExceptionText(Event->EvtRetCode));
		break;
	case evcReadExcpStatus:
		if (Event->EvtSender == 0)
			snprintf(Text, TextSize, "%s 0x07 - Read Exception Status %s", Timestamp, ExceptionText(Event->EvtRetCode));
		else
			snprintf(Text, TextSize, "%s [%s] 0x07 - Read Exception Status %s", Timestamp, IpAddress(Event->EvtSender), ExceptionText(Event->EvtRetCode));
		break;
	case evcDiagnostics:
		if (Event->EvtSender == 0)
			snprintf(Text, TextSize, "%s 0x08 - Diagnostics - Sub Fun : 0x%02X %s", Timestamp, Event->EvtParam1, ExceptionText(Event->EvtRetCode));
		else
			snprintf(Text, TextSize, "%s [%s] 0x08 - Diagnostics - Sub Fun : 0x%02X %s", Timestamp, IpAddress(Event->EvtSender), Event->EvtParam1, ExceptionText(Event->EvtRetCode));
		break;
	case evcGetCommEvtCnt:
		if (Event->EvtSender == 0)
			snprintf(Text, TextSize, "%s 0x0B - Get Comm Event Counter %s", Timestamp, ExceptionText(Event->EvtRetCode));
		else
			snprintf(Text, TextSize, "%s [%s] 0x0B - Get Comm Event Counter %s", Timestamp, IpAddress(Event->EvtSender), ExceptionText(Event->EvtRetCode));
		break;
	case evcGetCommEvtLog:
		if (Event->EvtSender == 0)
			snprintf(Text, TextSize, "%s 0x0C - Get Comm Event Log %s", Timestamp, ExceptionText(Event->EvtRetCode));
		else
			snprintf(Text, TextSize, "%s [%s] 0x0C - Get Comm Event Log %s", Timestamp, IpAddress(Event->EvtSender), ExceptionText(Event->EvtRetCode));
		break;
	case evcWriteMultiCoils:
		if (Event->EvtSender == 0)
			snprintf(Text, TextSize, "%s 0x0F - Write Multiple Coils - Addr : 0x%04X, Amount : %d %s", Timestamp, Event->EvtParam3, Event->EvtParam4, ExceptionText(Event->EvtRetCode));
		else
			snprintf(Text, TextSize, "%s [%s] 0x0F - Write Multiple Coils - Addr : 0x%04X, Amount : %d %s", Timestamp, IpAddress(Event->EvtSender), Event->EvtParam3, Event->EvtParam4, ExceptionText(Event->EvtRetCode));
		break;
	case evcWriteMultiRegs:
		if (Event->EvtSender == 0)
			snprintf(Text, TextSize, "%s 0x0F - Write Multiple Registers - Addr : 0x%04X, Amount : %d %s", Timestamp, Event->EvtParam3, Event->EvtParam4, ExceptionText(Event->EvtRetCode));
		else
			snprintf(Text, TextSize, "%s [%s] 0x0F - Write Multiple Registers - Addr : 0x%04X, Amount : %d %s", Timestamp, IpAddress(Event->EvtSender), Event->EvtParam3, Event->EvtParam4, ExceptionText(Event->EvtRetCode));
		break;
	case evcReportServerID:
		if (Event->EvtSender == 0)
			snprintf(Text, TextSize, "%s 0x11 - Report Server ID %s", Timestamp, ExceptionText(Event->EvtRetCode));
		else
			snprintf(Text, TextSize, "%s [%s] 0x11 - Report Server ID %s", Timestamp, IpAddress(Event->EvtSender), ExceptionText(Event->EvtRetCode));
		break;
	case evcReadFileRecord:
		if (Event->EvtSender == 0)
			snprintf(Text, TextSize, "%s 0x14 - Read File Record - Ref : %d, FileNum : %d, RecNum : %d, RecLen : %d %s", Timestamp, Event->EvtParam1, Event->EvtParam2, Event->EvtParam3, Event->EvtParam4, ExceptionText(Event->EvtRetCode));
		else
			snprintf(Text, TextSize, "%s [%s] 0x14 - Read File Record - Ref : %d, FileNum : %d, RecNum : %d, RecLen : %d %s", Timestamp, IpAddress(Event->EvtSender), Event->EvtParam1, Event->EvtParam2, Event->EvtParam3, Event->EvtParam4, ExceptionText(Event->EvtRetCode));
		break;
	case evcWriteFileRecord:
		if (Event->EvtSender == 0)
			snprintf(Text, TextSize, "%s 0x15 - Write File Record - Ref : %d, FileNum : %d, RecNum : %d, RecLen : %d %s", Timestamp, Event->EvtParam1, Event->EvtParam2, Event->EvtParam3, Event->EvtParam4, ExceptionText(Event->EvtRetCode));
		else
			snprintf(Text, TextSize, "%s [%s] 0x15 - Write File Record - Ref : %d, FileNum : %d, RecNum : %d, RecLen : %d %s", Timestamp, IpAddress(Event->EvtSender), Event->EvtParam1, Event->EvtParam2, Event->EvtParam3, Event->EvtParam4, ExceptionText(Event->EvtRetCode));
		break;
	case evcMaskWriteReg:
		if (Event->EvtSender == 0)
			snprintf(Text, TextSize, "%s 0x16 - Mask Write Register - Addr : 0x%04X, AND_Mask : 0x%04X, OR_Mask : 0x%04X %s", Timestamp, Event->EvtParam1, Event->EvtParam2, Event->EvtParam3, ExceptionText(Event->EvtRetCode));
		else
			snprintf(Text, TextSize, "%s [%s] 0x16 - Mask Write Register - Addr : 0x%04X, AND_Mask : 0x%04X, OR_Mask : 0x%04X %s", Timestamp, IpAddress(Event->EvtSender), Event->EvtParam1, Event->EvtParam2, Event->EvtParam3, ExceptionText(Event->EvtRetCode));
		break;
	case evcReadWriteMultiRegs:
		if (Event->EvtSender == 0)
			snprintf(Text, TextSize, "%s 0x17 - Read/Write Multiple Registers - RDStart : 0x%04X, RDSize : %d, WRStart : 0x%04X, WRSize : %d %s", Timestamp, Event->EvtParam1, Event->EvtParam2, Event->EvtParam3, Event->EvtParam4, ExceptionText(Event->EvtRetCode));
		else
			snprintf(Text, TextSize, "%s [%s] 0x17 - Read/Write Multiple Registers - RDStart : 0x%04X, RDSize : %d, WRStart : 0x%04X, WRSize : %d %s", Timestamp, IpAddress(Event->EvtSender), Event->EvtParam1, Event->EvtParam2, Event->EvtParam3, Event->EvtParam4, ExceptionText(Event->EvtRetCode));
		break;
	case evcReadFifoQueue:
		if (Event->EvtSender == 0)
			snprintf(Text, TextSize, "%s 0x18 - Read FIFO Queue - Addr : 0x%04X %s", Timestamp, Event->EvtParam3, ExceptionText(Event->EvtRetCode));
		else
			snprintf(Text, TextSize, "%s [%s] 0x18 - Read FIFO Queue - Addr : 0x%04X %s", Timestamp, IpAddress(Event->EvtSender), Event->EvtParam3, ExceptionText(Event->EvtRetCode));
		break;
	case evcEncIntTransport:
		if (Event->EvtSender == 0)
			snprintf(Text, TextSize, "%s 0x2B - Encapsulated Interface Transport - MEI Type : 0x%02X %s", Timestamp, Event->EvtParam1, ExceptionText(Event->EvtRetCode));
		else
			snprintf(Text, TextSize, "%s [%s] 0x2B - Encapsulated Interface Transport - MEI Type : 0x%02X %s", Timestamp, IpAddress(Event->EvtSender), Event->EvtParam1, ExceptionText(Event->EvtRetCode));
		break;
	case evcCustomFunction:
		if (Event->EvtSender == 0)
			snprintf(Text, TextSize, "%s Custom Function (0x%02X) %s", Timestamp, Event->EvtParam1, ExceptionText(Event->EvtRetCode));
		else
			snprintf(Text, TextSize, "%s [%s] - Custom Function (0x%02X) %s", Timestamp, IpAddress(Event->EvtSender), Event->EvtParam1, ExceptionText(Event->EvtRetCode));
		break;
	case evcPassthrough:
		snprintf(Text, TextSize, "%s [%s] Message relayed DeviceID=%d, Fun=0x%02X, Sent=%d, Recvd=%d, IResult=%s", Timestamp, IpAddress(Event->EvtSender), Event->EvtParam1, Event->EvtParam2, Event->EvtParam3, Event->EvtParam4, PTResult(Event->EvtRetCode));
		break;
	default:
		snprintf(Text, TextSize, "%s Unknown event (0x%08X)", Timestamp, Event->EvtCode);
		break;
	}
	return Text;
}




