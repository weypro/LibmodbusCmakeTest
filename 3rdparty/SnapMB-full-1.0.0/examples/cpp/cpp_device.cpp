#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "snapmb.h"

#define regs_amount 32768
#define bits_amount 65536

typedef enum {
    null_Device,
    tcp_Device,
    udp_Device,
    rtu_overTCP_Device,
    rtu_overUDP_Device,
    rtu_Device,
    asc_Device
}DeviceType;

typedef enum {
    opt_log,
    opt_dump
}Option;
Option opt = opt_log;

// This is our Device, regardless of the Transport/Protocol
PSnapMBDevice Device;

// These are shared Areas
word InputRegisters[regs_amount];
word HoldingRegisters[regs_amount];
byte Coils[bits_amount];
byte DiscreteInputs[bits_amount];

char Text[256] = "";

DeviceType dt = null_Device;
#define tcp "TCP"
#define udp "UDP"
#define RtuOverTCP "RTUOVERTCP"
#define RtuOverUDP "RTUOVERUDP"
#define rtu "RTU"
#define asc "ASC"

#define DefaultMBPort 502

// NET Params with defaults
char Address[16] = "127.0.0.1"; // -a
int Port = DefaultMBPort;     // -p

// SER Params with defaults
byte DeviceID = 1;
#ifdef SNAP_OS_WINDOWS
char ComPort[] = "COM5";      // -c
#else
char ComPort[] = "/dev/ttyUSB0";
#endif
int BaudRate = 19200;         // -b
char Parity = 'E';            // -y
int DataBits = 8;             // -d
int StopBits = 1;             // -s

//------------------------------------------------------------------------------
#ifndef SNAP_OS_WINDOWS
char* strupr(char* text)
{
    char* s = text;
    for (; *s; ++s)
        *s = toupper((unsigned char)*s);
    return text;
}
#endif
//------------------------------------------------------------------------------
// Find a Param into the Cmd Line
bool Find(const char* ParamName, char* Param, int argc, char* argv[])
{
    Param[0] = '\0';

    for (int c = 0; c < argc; c++)
    {
        if (strcmp(ParamName, strupr(argv[c])) == 0)
        {
            if (c < argc - 1)
            {
                strcpy(Param, argv[c + 1]);
                return true;
            }
            else
                return false;
        }
    }
    return false;
}

//------------------------------------------------------------------------------
// Get the Device Type
DeviceType GetDeviceType(char* DeviceName)
{
    if (strcmp(tcp, DeviceName) == 0)
        return tcp_Device;
    if (strcmp(udp, DeviceName) == 0)
        return udp_Device;
    if (strcmp(RtuOverTCP, DeviceName) == 0)
        return rtu_overTCP_Device;
    if (strcmp(RtuOverUDP, DeviceName) == 0)
        return rtu_overUDP_Device;
    if (strcmp(rtu, DeviceName) == 0)
        return rtu_Device;
    if (strcmp(asc, DeviceName) == 0)
        return asc_Device;
    return null_Device;
}

// Parses the Cmd Line
bool ParseCmdLine(int argc, char* argv[])
{
    char Param[32];

    if (argc < 2)
        return false;

    // Get Device Type (Mandatory)
    dt = GetDeviceType(strupr(argv[1]));
    if (dt == null_Device)
        return false;

    if (dt == rtu_Device || dt == asc_Device)
    {
        if (Find("-I", Param, argc, argv))
            DeviceID = atoi(Param);
        if (Find("-C", Param, argc, argv))
            strcpy(ComPort, Param);
        if (Find("-B", Param, argc, argv))
            BaudRate = atoi(Param);
        if (Find("-Y", Param, argc, argv))
            Parity = Param[0];
        if (Find("-D", Param, argc, argv))
            DataBits = atoi(Param);
        if (Find("-S", Param, argc, argv))
            StopBits = atoi(Param);
    }
    else // is Net Device
    {
        if (Find("-A", Param, argc, argv))
            strcpy(Address, Param);
        if (Find("-P", Param, argc, argv))
            Port = atoi(Param);
    }

    // Get Option
    if (Find("-G", Param, argc, argv))
    {
        if (strcmp("D", strupr(Param)) == 0)
            opt = opt_dump;
    }

    return true;
}

//------------------------------------------------------------------------------
// hexdump, a very nice function, it's not mine.
// I found it on the net somewhere some time ago... thanks to the author ;-)
//------------------------------------------------------------------------------
#ifndef HEXDUMP_COLS
#define HEXDUMP_COLS 16
#endif
void hexdump(void* mem, unsigned int len)
{
    unsigned int i, j;

    for (i = 0; i < len + ((len % HEXDUMP_COLS) ? (HEXDUMP_COLS - len % HEXDUMP_COLS) : 0); i++)
    {
        /* print offset */
        if (i % HEXDUMP_COLS == 0)
        {
            printf("0x%04x: ", i);
        }

        /* print hex data */
        if (i < len)
        {
            printf("%02x ", 0xFF & ((char*)mem)[i]);
        }
        else /* end of block, just aligning for ASCII dump */
        {
            printf("   ");
        }

        /* print ASCII dump */
        if (i % HEXDUMP_COLS == (HEXDUMP_COLS - 1))
        {
            for (j = i - (HEXDUMP_COLS - 1); j <= i; j++)
            {
                if (j >= len) /* end of block, not really printing */
                {
                    putchar(' ');
                }
                else if (isprint((((char*)mem)[j] & 0x7F))) /* printable char */
                {
                    putchar(0xFF & ((char*)mem)[j]);
                }
                else /* other char */
                {
                    putchar('.');
                }
            }
            putchar('\n');
        }
    }
}

//***********************************************************************************
// CALLBACKS
// ----------------------------------------------------------------------------------
// When the Device receives a request it needs to know how to manage it. For each
// function (or group of them) it calls an user function : the "callback"; inside
// that we will perform the oparations needed.
// If a function has not a callback registered, the Device will answer an error code
// 0x01 : illegal function called, i.e. "function not implemented into this device".
//***********************************************************************************

// Events Callback. Here we use it, for semplicity, to show the event messages
// Beware that it's called during the modbus transaction, so, if you don't need of
// an "immediate" reaction, you should use PickEvent() or PickEventAsText() that
// they use a circular queue. Look at no-console demos to see how to use them.
void SNAP_API EventCallBack(void* usrPtr, void* Event, int Size)
{
    // print the event
    if (opt == opt_log)
        printf("%s\n", EventText(Event, Text, 255));
}
//------------------------------------------------------------------------------
void SNAP_API PacketLog(void* UsrPtr, longword Peer, int Direction, void* Data, int Size)
{
    if (opt == opt_dump)
    {
        if (Direction == PacketLog_IN)
            printf("Indication received (%d Byte)\n", Size);
        else
            printf("Confirmation sent (%d Byte)\n", Size);

        printf("------------------------------------------------------------------------\n");
        hexdump(Data, Size);
        printf("------------------------------------------------------------------------\n");

        if (Direction == PacketLog_OUT)
            printf("\n");
    }
}
//------------------------------------------------------------------------------
int SNAP_API DiscreteInputsRequest(void* usrPtr, word Address, word Amount, void* Data)
{
    return 0;
}
//------------------------------------------------------------------------------
int SNAP_API CoilsRequest(void* usrPtr, int Action, word Address, word Amount, void* Data)
{
    return 0;
}
//------------------------------------------------------------------------------
int SNAP_API InputRegistersRequest(void* usrPtr, word Address, word Amount, void* Data)
{
    return 0;
}
//------------------------------------------------------------------------------
int SNAP_API HoldingRegistersRequest(void* usrPtr, int Action, word Address, word Amount, void* Data)
{
    return 0;
}
//------------------------------------------------------------------------------
int SNAP_API ReadWriteMultipleRegistersRequest(void* usrPtr, word RDAddress, word RDAmount, void* RDData, word WRAddress, word WRAmount, void* WRData)
{
    return 0;
}
//------------------------------------------------------------------------------
int SNAP_API MaskRegisterRequest(void* usrPtr, word Address, word AND_Mask, word OR_Mask)
{
    return 0;
}
//------------------------------------------------------------------------------
int SNAP_API FileRecordRequest(void* usrPtr, int Action, word RefType, word FileNumber, word RecNumber, word RegsAmount, void* Data)
{
    if (Action == cbActionRead)
        memcpy(Data, HoldingRegisters, RegsAmount * 2);
    else
        memcpy(HoldingRegisters, Data, RegsAmount * 2);
    return 0;
}
//------------------------------------------------------------------------------
int SNAP_API ExceptionStatusRequest(void* usrPtr, byte& Status)
{
    return 0;
}
//------------------------------------------------------------------------------
int SNAP_API DiagnosticsRequest(void* usrPtr, word SubFunction, void* RxItems, void* TxItems, uint16_t ItemsSent, uint16_t& ItemsRecvd)
{
    return 0;
}
//------------------------------------------------------------------------------
int SNAP_API GetCommEventCounterRequest(void* usrPtr, word& Status, word& EventCount)
{
    return 0;
}
//------------------------------------------------------------------------------
int SNAP_API GetCommEventLogRequest(void* usrPtr, word& Status, word& EventCount, word& MessageCount, void* Data, uint16_t& EventsAmount)
{
    uint8_t Items[16];
    for (int c = 0; c < 16; c++)
        Items[c] = uint8_t(c);
    memcpy(Data, &Items, 16);
    EventsAmount = 16;

    return 0;
}
//------------------------------------------------------------------------------
int SNAP_API ReportServerIDRequest(void* usrPtr, void* Data, uint16_t& DataSize)
{

    char ServerID[] = "SnapMB Device";
    DataSize = sizeof(ServerID);
    memcpy(Data, &ServerID, DataSize);

    return 0;
}
//------------------------------------------------------------------------------
int SNAP_API ReadFIFOQueueRequest(void* usrPtr, word PtrAddress, void* FIFOValues, uint16_t& FifoCount)
{
    uint16_t Items[16];
    for (int c = 0; c < 16; c++)
        Items[c] = uint16_t(c);
    memcpy(FIFOValues, &Items, 32);
    FifoCount = 16;

    return 0;
}
//------------------------------------------------------------------------------
int SNAP_API EncapsulatedIT(void* usrPtr, byte MEI_Type, void* MEI_DataReq, uint16_t ReqDataSize, void* MEI_DataRes, uint16_t& ResDataSize)
{
    memcpy(MEI_DataRes, MEI_DataReq, ReqDataSize);
    return 0;
}
//------------------------------------------------------------------------------
int SNAP_API UsrFunctionRequest(void* usrPtr, byte Function, void* RxPDU, uint16_t RxPDUSize, void* TxPDU, uint16_t& TxPDUSize)
{
    if (Function == 0x41)
    {
        // Do something
    }
    if (Function == 0x42)
    {
        // Do something else
    }
    return 0;
}
//------------------------------------------------------------------------------
void Legenda()
{
    printf("\n");
    printf("Usage:\n");
    printf("Device <DeviceType> [<DeviceParams>] [<Options>]\n");
    printf("\n");
    printf("DeviceType (Mandatory, no case sensitive):\n");
    printf("  tcp             : Modbus/TCP Device\n");
    printf("  udp             : Modbus/UDP Device\n");
    printf("  RtuOverTcp      : RTU Telegram Over Modbus/TCP Device\n");
    printf("  RtuOverUdp      : RTU Telegram Over Modbus/UDP Device\n");
    printf("  rtu             : RTU (serial) Device\n");
    printf("  asc             : ASCII (serial) Device\n");
    printf("\n");
    printf("Ethernet Device Params (Optionals):\n");
    printf("  -a <Address>    : IP Address (default 127.0.0.1)\n");
    printf("  -p <Port>       : IP Port (default 502) (*)\n");
    printf("\n");
    printf("Serial Device Params (Optionals):\n");
    printf("  -i <DeviceID>   : Device ID, 1..255 (Default 1)\n");
    printf("  -c <ComPort>    : ComPort Name (Default Windows = COM5, Linux = \\dev\\ttyUSB0)\n");
    printf("  -b <BaudRate>   : BaudRate (Default 19200)\n");
    printf("  -y <Parity>     : Parity, N or E or O (Default E)\n");
    printf("  -d <DataBits>   : DataBits, 7 or 8 (Default 8)\n");
    printf("  -s <StopBits>   : StopBits, 1 or 2 (Default 1)\n");
    printf("\n");
    printf("Options:\n");
    printf("  -g <LogOption>  : L (Activity Log), D (Telegram Dump) (Default L)\n");
    printf("\n");
    printf("Examples:\n");
    printf("  Device tcp -a 192.168.0.15 -p 5020\n");
    printf("  Device rtu -c COM7 -b 115200 -y N\n");
    printf("\n");
    printf("(*) to use a Low port number under Linux you must run the program as root\n");
    printf("    i.e.  sudo Device <DeviceType> [<DeviceParams>] [<Options>]\n");
    printf("\n");
}

// Device Creation
void CreateDevice(DeviceType Type)
{
    switch (Type)
    {
    case tcp_Device:
        Device = new TSnapMBDevice(ProtoTCP, 1, Address, Port);
        break;
    case udp_Device:
        Device = new TSnapMBDevice(ProtoUDP, 1, Address, Port);
        break;
    case rtu_overTCP_Device:
        Device = new TSnapMBDevice(ProtoRTUOverTCP, 1, Address, Port);
        break;
    case rtu_overUDP_Device:
        Device = new TSnapMBDevice(ProtoRTUOverUDP, 1, Address, Port);
        break;
    case rtu_Device:
        Device = new TSnapMBDevice(FormatRTU, DeviceID, ComPort, BaudRate, Parity, DataBits, StopBits, 0);
        break;
    case asc_Device:
        Device = new TSnapMBDevice(FormatASC, DeviceID, ComPort, BaudRate, Parity, DataBits, StopBits, 0);
        break;
    default:
        Device = new TSnapMBDevice(ProtoTCP, 1, Address, Port);
    }

    printf("\n");
    printf("Device created\n");
    printf("--------------\n");
    if (dt == rtu_Device || dt == asc_Device)
    {
        printf("Device ID : %d\n", DeviceID);
        printf("DataLink  : Serial\n");
        printf("Params    : %s, %d, %c, %d, %d\n", ComPort, BaudRate, Parity, DataBits, StopBits);
    }
    else
    {
        printf("Device ID : %d\n", DeviceID);
        printf("DataLink  : Ethernet\n");
        printf("Params    : %s : %d\n", Address, Port);
    }
    printf("\n");
}

int main(int argc, char* argv[])
{
    if (!ParseCmdLine(argc, argv))
    {
        Legenda();
        getchar();
        return 1;
    }
    
    CreateDevice(dt);

    // Here we register the Areas. i.e. we say to the Device that when there is an operation
    // of read/write, he must use these structs

    Device->RegisterArea(mbAreaCoils, &Coils, bits_amount);
    Device->RegisterArea(mbAreaDiscreteInputs, &DiscreteInputs, bits_amount);
    Device->RegisterArea(mbAreaInputRegisters, &InputRegisters, regs_amount);
    Device->RegisterArea(mbAreaHoldingRegisters, &HoldingRegisters, regs_amount);

    // Register callbacks
    Device->RegisterCallback(cbkDeviceEvent, (void*)EventCallBack, NULL);
    Device->RegisterCallback(cbkPacketLog, (void*)PacketLog, NULL);
    Device->RegisterCallback(cbkDiscreteInputs, (void*)DiscreteInputsRequest, NULL);
    Device->RegisterCallback(cbkCoils, (void*)CoilsRequest, NULL);
    Device->RegisterCallback(cbkInputRegisters, (void*)InputRegistersRequest, NULL);
    Device->RegisterCallback(cbkHoldingRegisters, (void*)HoldingRegistersRequest, NULL);
    Device->RegisterCallback(cbkReadWriteRegisters, (void*)ReadWriteMultipleRegistersRequest, NULL);
    Device->RegisterCallback(cbkMaskRegister, (void*)MaskRegisterRequest, NULL);
    Device->RegisterCallback(cbkFileRecord, (void*)FileRecordRequest, NULL);
    Device->RegisterCallback(cbkExceptionStatus, (void*)ExceptionStatusRequest, NULL);
    Device->RegisterCallback(cbkDiagnostics, (void*)DiagnosticsRequest, NULL);
    Device->RegisterCallback(cbkGetCommEventCounter, (void*)GetCommEventCounterRequest, NULL);
    Device->RegisterCallback(cbkGetCommEventLog, (void*)GetCommEventLogRequest, NULL);
    Device->RegisterCallback(cbkReportServerID, (void*)ReportServerIDRequest, NULL);
    Device->RegisterCallback(cbkReadFIFOQueue, (void*)ReadFIFOQueueRequest, NULL);
    Device->RegisterCallback(cbkEncapsulatedIT, (void*)EncapsulatedIT, NULL);
    Device->RegisterCallback(cbkUsrFunction, (void*)UsrFunctionRequest, NULL);

    Device->SetParam(par_AllowSerFunOnEth, 1);

    // Device Start
    int Result = Device->Start();
    if (Result == 0)
    {
        printf("Connected\n");
        printf("Enter any char to terminate...\n");
        printf("\n");
        // Now the Device is running ... press a key (and Enter) to terminate
        getchar();
    }
    else
        printf("%s\n", ErrorText(Result, Text, 255));

    delete Device;
}
