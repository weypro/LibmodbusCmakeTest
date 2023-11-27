#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "snapmb_e.h"

#define regs_amount 32768
#define bits_amount 65536

typedef enum {
    null_Client,
    tcp_Client,
    udp_Client,
    rtu_overTCP_Client,
    rtu_overUDP_Client,
    rtu_Client,
    asc_Client
}ClientType;

typedef enum {
    dump_light,
    dump_full
}DumpOption;
DumpOption opt = dump_full;

typedef enum {
    mode_Interactive,
    mode_Batch
}ModeOption;
ModeOption mode = mode_Interactive;

// This is our Client, regardless of the Transport/Protocol
PSnapMBBroker Client;

// Resources
uint16_t Regs[regs_amount];
uint8_t Bits[bits_amount];

char Text[256] = "";

ClientType ct = null_Client;
#define tcp "TCP"
#define udp "UDP"
#define RtuOverTCP "RTUOVERTCP"
#define RtuOverUDP "RTUOVERUDP"
#define rtu "RTU"
#define asc "ASC"
#define DefaultMBPort 502

byte DeviceID = 1;
// NET Params with defaults
char Address[16] = "127.0.0.1"; // -a
int Port = DefaultMBPort;       // -p

// SER Params with defaults
#ifdef SNAP_OS_WINDOWS
char ComPort[] = "COM5";       // -c
#else
char ComPort[] = "/dev/ttyUSB0";
#endif
int BaudRate = 19200;          // -b
char Parity = 'E';             // -y
int DataBits = 8;              // -d
int StopBits = 1;              // -s

int16_t Amount = 8;
int TestPassed = 0;
int TotalTime = 0;

#define _StatusUnknown     0
#define _StatusOk          1  // Ok
#define _StatusTimeout     2  // Timeout        
#define _StatusError       3  // Hardware error
#define _StatusProtoError  4  // Protocol error

const char* DeviceStatus[5] = { "Unknown", "Ok", "Timeout", "Hardware error", "Protocol error" };
const char* ConnectionStatus[2] = { "NOT Connected", "Connected" };

typedef void(*pfn_ModbusFunction)();

//==============================================================================
// Utilities (Results and Dump)
//==============================================================================

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
//------------------------------------------------------------------------------
// Dumps Registers
//------------------------------------------------------------------------------
void RegsDump(int Count)
{
    printf("+---------------------------------------------------------------------------+\n");
    printf("Registers Dump (%d)\n", Count);
    for (int c = 0; c < Count; c++)
    {
        printf("%04x ", Regs[c]);
        if (((c + 1) % 8) == 0 && c != Count - 1)
            printf("\n");
    }
    printf("\n");
}
//------------------------------------------------------------------------------
// Dumps Bits      
//------------------------------------------------------------------------------
void BitsDump(int Count)
{
    printf("+---------------------------------------------------------------------------+\n");
    printf("Bits Dump (%d)\n", Count);
    for (int c = 0; c < Count; c++)
    {
        printf("%d ", Bits[c]);
        if (((c + 1) % 16) == 0 && c != Count - 1)
            printf("\n");
    }
    printf("\n");
}
//------------------------------------------------------------------------------
// Dumps Telegrams
//------------------------------------------------------------------------------
void TelegramsDump()
{
    pbyte pData = NULL;
    int Size;
    Size = Client->GetIOBufferPtr(DeviceID, BkSnd, pData);
    if (Size > 0)
    {
        printf("+---------------------------------------------------------------------------+\n");
        printf("Request Dump (%d)\n", Size);
        hexdump(pData, Size);
    }
    Size = Client->GetIOBufferPtr(DeviceID, BkRcv, pData);
    if (Size > 0)
    {
        printf("+---------------------------------------------------------------------------+\n");
        printf("Response Dump (%d)\n", Size);
        hexdump(pData, Size);
    }
}
//------------------------------------------------------------------------------
// Print Result
//------------------------------------------------------------------------------
void PrintStatus(const char* Info)
{
    TDeviceStatus Status;
    printf("\n");
    printf("+---------------------------------------------------------------------------+\n");
    printf(" %s\n", Info);
    printf("+---------------------------------------------------------------------------+\n");
    Client->GetDeviceStatus(DeviceID, Status);
    printf("Function Result   : %s\n", ErrorText(Status.LastError, Text, 255));
    printf("Device Status     : %s\n", DeviceStatus[Status.Status]);
    printf("Job Time (ms)     : %d\n", Status.JobTime);
    printf("Connection Status : %s\n", ConnectionStatus[Status.Connected]);
    if (Status.LastError == mbNoError)
        TestPassed++;
    TotalTime += Status.JobTime;
}

//==============================================================================
// User program interfaces (legenda, menu, parameters parse...)
//==============================================================================
void Legenda()
{
    printf("\n");
    printf("Usage:\n");
    printf("Client <ClientType> [<ClientParams>] [<Options>]\n");
    printf("\n");
    printf("ClientType (Mandatory, no case sensitive):\n");
    printf("  tcp             : Modbus/TCP Client\n");
    printf("  udp             : Modbus/UDP Client\n");
    printf("  RtuOverTcp      : RTU Telegram Over Modbus/TCP Client\n");
    printf("  RtuOverUdp      : RTU Telegram Over Modbus/UDP Client\n");
    printf("  rtu             : RTU (serial) Client\n");
    printf("  asc             : ASCII (serial) Client\n");
    printf("\n");
    printf("Ethernet Client Params (Optionals):\n");
    printf("  -a <Address>    : IP Address (default 127.0.0.1)\n");
    printf("  -p <Port>       : IP Port (default 502) (*)\n");
    printf("\n");
    printf("Serial Client Params (Optionals):\n");
    printf("  -c <ComPort>    : ComPort Name (Default Windows = COM5, Linux = \\dev\\ttyUSB0)\n");
    printf("  -b <BaudRate>   : BaudRate (Default 19200)\n");
    printf("  -y <Parity>     : Parity, N or E or O (Default E)\n");
    printf("  -d <DataBits>   : DataBits, 7 or 8 (Default 8)\n");
    printf("  -s <StopBits>   : StopBits, 1 or 2 (Default 1)\n");
    printf("\n");
    printf("Options:\n");
    printf("  -g <DumpOption> : L (Light Dump), F (Full Dump) (Default F)\n");
    printf("  -m <ModeOption> : I (Interactive), A (All tests) (Default I)\n");
    printf("\n");
    printf("Examples:\n");
    printf("  Client tcp -a 192.168.0.15 -p 5020\n");
    printf("  Client rtu -c COM7 -b 115200 -y N\n");
    printf("\n");
    printf("(*) to use a Low port number under Linux you must run the program as root\n");
    printf("    i.e.  sudo Client <ClientType> [<ClientParams>] [<Options>]\n");
    printf("\n");
}
//------------------------------------------------------------------------------
void Menu()
{
    printf("+---------------------------------------------------------------------------+\n");
    printf("|                          MODBUS Functions available                       |\n");
    printf("+---------------------------------------------------------------------------+\n");
    printf("| 1 - (0x01) Read Coils               11 - (0x0F) Write Multiple Coils      |\n");
    printf("| 2 - (0x02) Read Discrete Inputs     12 - (0x10) Write Multiple Registers  |\n");
    printf("| 3 - (0x03) Read Holding Registers   13 - (0x11) Report Server ID          |\n");
    printf("| 4 - (0x04) Read Input Registers     14 - (0x14) Read File Record          |\n");
    printf("| 5 - (0x05) Write Single Coil        15 - (0x15) Write File Record         |\n");
    printf("| 6 - (0x06) Write Single Register    16 - (0x16) Mask Write Register       |\n");
    printf("| 7 - (0x07) Read Exception Status    17 - (0x17) Read/Write Multi Registers|\n");
    printf("| 8 - (0x08) Diagnostics              18 - (0x18) Read FIFO Queue           |\n");
    printf("| 9 - (0x0B) Get Comm Event Counter   19 - (0x2B) Encapsulated Intf.Transp. |\n");
    printf("|10 - (0x0C) Get Comm Event Log       20 - Execute All                      |\n");
    printf("+---------------------------------------------------------------------------+\n");
    printf("|Options                                                                    |\n");
    printf("+---------------------------------------------------------------------------+\n");
    printf("|21 - Overrides the number of Regs/Coils to Read/Write (Current is %5d)   |\n", Amount);
    printf("|22 - Overrides the target Device ID (Default is %3d)                       |\n", DeviceID);
    printf("+---------------------------------------------------------------------------+\n");
    printf("|0  - Exit Program                                                          |\n");
    printf("+---------------------------------------------------------------------------+\n");
}
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
//------------------------------------------------------------------------------
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
// Get the Client Type
//------------------------------------------------------------------------------
ClientType GetClientType(char* ClientName)
{
    if (strcmp(tcp, ClientName) == 0)
        return tcp_Client;
    if (strcmp(udp, ClientName) == 0)
        return udp_Client;
    if (strcmp(RtuOverTCP, ClientName) == 0)
        return rtu_overTCP_Client;
    if (strcmp(RtuOverUDP, ClientName) == 0)
        return rtu_overUDP_Client;
    if (strcmp(rtu, ClientName) == 0)
        return rtu_Client;
    if (strcmp(asc, ClientName) == 0)
        return asc_Client;
    return null_Client;
}
//------------------------------------------------------------------------------
// Parses the Cmd Line
//------------------------------------------------------------------------------
bool ParseCmdLine(int argc, char* argv[])
{
    char Param[32];

    if (argc < 2)
        return false;

    // Get Device Type (Mandatory)
    ct = GetClientType(strupr(argv[1]));
    if (ct == null_Client)
        return false;

    if (ct == rtu_Client || ct == asc_Client)
    {
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

    // Get Dump Option
    if (Find("-G", Param, argc, argv))
    {
        if (strcmp("L", strupr(Param)) == 0)
            opt = dump_light;
    }

    // Get Mode Option
    if (Find("-M", Param, argc, argv))
    {
        if (strcmp("A", strupr(Param)) == 0)
            mode = mode_Batch;
    }

    return true;
}
//------------------------------------------------------------------------------
// Client Creation
//------------------------------------------------------------------------------
bool CreateClient(ClientType Type)
{
    switch (Type)
    {
    case tcp_Client:
        Client = new TSnapMBBroker(ProtoTCP, Address, Port);
        break;
    case udp_Client:
        Client = new TSnapMBBroker(ProtoUDP, Address, Port);
        break;
    case rtu_overTCP_Client:
        Client = new TSnapMBBroker(ProtoRTUOverTCP, Address, Port);
        break;
    case rtu_overUDP_Client:
        Client = new TSnapMBBroker(ProtoRTUOverUDP, Address, Port);
        break;
    case rtu_Client:
        Client = new TSnapMBBroker(FormatRTU, ComPort, BaudRate, Parity, DataBits, StopBits, 0);
        break;
    case asc_Client:
        Client = new TSnapMBBroker(FormatASC, ComPort, BaudRate, Parity, DataBits, StopBits, 0);
        break;
    default:
        Client = new TSnapMBBroker(ProtoTCP, Address, Port);
    }

    bool IsSerial = ct == rtu_Client || ct == asc_Client;

    printf("\n");
    printf("Client created\n");
    printf("-----------------------------------------------------------------------------\n");

    if (IsSerial)
    {
        IsSerial = true;
        printf("DataLink  : Serial\n");
        if (Type==rtu_Client)
            printf("Format    : RTU\n");
        else
            printf("Format    : ASCII\n");
        printf("Params    : %s, %d, %c, %d, %d\n", ComPort, BaudRate, Parity, DataBits, StopBits);
    }
    else
    {
        printf("DataLink  : Ethernet\n");
        switch (Type)
        {
        case tcp_Client:
            printf("Transport : TCP (Modbus/TCP)\n");
            break;
        case udp_Client:
            printf("Transport : UDP (Modbus/UDP)\n");
            break;
        case rtu_overTCP_Client:
            printf("Transport : TCP (RTU over TCP)\n");
            break;
        case rtu_overUDP_Client:
            printf("Transport : UDP (RTU over UDP)\n");
            break;
        }
        printf("Params    : %s : %d\n", Address, Port);
    }

    // Connection
    int Result = Client->Connect();

    if (Result == mbNoError)
    {
        printf("Status    : Connected\n");
        return true;
    }
    else {
        printf("Status    : NOT Connected (exiting)\n");
        printf("Error     : %s\n", ErrorText(Result, Text, 255));
        printf("-----------------------------------------------------------------------------\n");
        delete Client;
        return false;
    }
}
//------------------------------------------------------------------------------
void GetOption(int& Option)
{
    printf("Select > ");
    if (scanf("%2d", &Option) != 1)
    {
        Option = -1;
        scanf("%255s", Text); // Flush stdin
    }
}
//------------------------------------------------------------------------------
void GetAmount()
{
    int NewAmount;
    printf("Enter new Amount > ");
    if (scanf("%d", &NewAmount) != 1)
    {
        scanf("%255s", Text); // Flush stdin
        return;
    }
    if (NewAmount > 0)
        Amount = NewAmount;
}
//------------------------------------------------------------------------------
void GetDeviceID()
{
    int NewDeviceID;
    printf("Enter new DeviceID > ");
    if (scanf("%d", &NewDeviceID) != 1)
    {
        scanf("%255s", Text); // Flush stdin
        return;
    }
    if (NewDeviceID > 0 && NewDeviceID < 256)
        DeviceID = NewDeviceID;
}
//==============================================================================
// MODBUS FUNCTIONS
//==============================================================================
void func_ReadCoils()
{
    int Result = Client->ReadCoils(DeviceID, 1, Amount, Bits);
    PrintStatus("(0x01) Read Coils");
    if (Result == mbNoError)
    {
        BitsDump(Amount);
        if (opt == dump_full)
            TelegramsDump();
    }
    printf("\n");
}
//------------------------------------------------------------------------------
void func_ReadDiscreteInputs()
{
    int Result = Client->ReadDiscreteInputs(DeviceID, 1, Amount, Bits);
    PrintStatus("(0x02) Read Discrete Inputs");
    if (Result == mbNoError)
    {
        BitsDump(Amount);
        if (opt == dump_full)
            TelegramsDump();
    }
    printf("\n");
}
//------------------------------------------------------------------------------
void func_ReadHoldingRegisters()
{
    int Result = Client->ReadHoldingRegisters(DeviceID, 1, Amount, Regs);
    PrintStatus("(0x03) Read Holding Registers");
    if (Result == mbNoError)
    {
        RegsDump(Amount);
        if (opt == dump_full)
            TelegramsDump();
    }
    printf("\n");
}
//------------------------------------------------------------------------------
void func_ReadInputRegisters()
{
    int Result = Client->ReadInputRegisters(DeviceID, 1, Amount, Regs);
    PrintStatus("(0x04) Read Input Registers");
    if (Result == mbNoError)
    {
        RegsDump(Amount);
        if (opt == dump_full)
            TelegramsDump();
    }
    printf("\n");
}
//------------------------------------------------------------------------------
void func_WriteSingleCoil()
{
    int Result = Client->WriteSingleCoil(DeviceID, 1, 1);
    PrintStatus("(0x05) Write Single Coil");
    if (Result == mbNoError)
    {
        if (opt == dump_full)
            TelegramsDump();
    }
    printf("\n");
}
//------------------------------------------------------------------------------
void func_WriteSingleRegister()
{
    Regs[0] = 0x0123;
    int Result = Client->WriteSingleRegister(DeviceID, 1, Regs[0]);
    PrintStatus("(0x06) Write Single Register");
    if (Result == mbNoError)
    {
        RegsDump(1);
        if (opt == dump_full)
            TelegramsDump();
    }
    printf("\n");
}
//------------------------------------------------------------------------------
void func_ReadExceptionStatus()
{
    byte Status = 0;
    int Result = Client->ReadExceptionStatus(DeviceID, Status);
    PrintStatus("(0x07) Read Exception Status");
    if (Result == mbNoError)
    {
        printf("-----------------------------\n");
        printf("Exception Status  : 0x%02x\n", Status);
        if (opt == dump_full)
            TelegramsDump();
    }
    printf("\n");
}
//------------------------------------------------------------------------------
void func_Diagnostics()
{
    uint16_t ItemsReceived;
    Regs[0] = 0x5555;
    uint16_t SubFun = 0;
    int Result = Client->Diagnostics(DeviceID, SubFun, &Regs, &Regs, 1, ItemsReceived);
    PrintStatus("(0x08) Diagnostics");
    if (Result == mbNoError)
    {
        printf("-----------------------------\n");
        printf("SubFun sent       : 0x0000 (Echo)\n");
        printf("Items sent (1)    : 0x5555\n");
        printf("Items recv (%d)    : 0x%04x\n", ItemsReceived, Regs[0]);
        if (opt == dump_full)
            TelegramsDump();
    }
    printf("\n");
}
//------------------------------------------------------------------------------
void func_GetCommEventCounter()
{
    uint16_t Status = 0;
    uint16_t EventsCount = 0;
    int Result = Client->GetCommEventCounter(DeviceID, Status, EventsCount);
    PrintStatus("(0x0B) Get Comm Event Counter");

    if (Result == mbNoError)
    {
        printf("-----------------------------\n");
        printf("Status            : 0x%04x\n", Status);
        printf("Events Count      : 0x%04x\n", EventsCount);

        if (opt == dump_full)
            TelegramsDump();
    }
    printf("\n");
}
//------------------------------------------------------------------------------
void func_GetCommEventLog()
{
    uint16_t Status = 0;
    uint16_t EventsCount = 0;
    uint16_t MessageCount = 0;
    uint16_t NumItems = 0;
    uint8_t Items[256];

    int Result = Client->GetCommEventLog(DeviceID, Status, EventsCount, MessageCount, NumItems, &Items);
    PrintStatus("(0x0C) Get Comm Event Log");

    if (Result == mbNoError)
    {
        if (MessageCount > 256)
            MessageCount = 256;
        printf("-----------------------------\n");
        printf("Status            : 0x%04x\n", Status);
        printf("Events Count      : 0x%04x\n", EventsCount);
        printf("Message Count     : 0x%04x\n", MessageCount);
        printf("Num Items         : 0x%04x\n", NumItems);
        printf("Items[]\n");
        for (int c = 0; c < NumItems; c++)
            printf("  (%3d) : 0x%02x\n", c, Items[c]);
        if (opt == dump_full)
            TelegramsDump();
    }
    printf("\n");
}
//------------------------------------------------------------------------------
void func_WriteMultipleCoils()
{
    for (int c = 0; c < Amount; c++)
        Bits[c] = uint8_t(c % 2);  // Alternates 0 and 1

    int Result = Client->WriteMultipleCoils(DeviceID, 1, Amount, Bits);
    PrintStatus("(0x0F) Write Multiple Coils");
    if (Result == mbNoError)
    {
        BitsDump(Amount);
        if (opt == dump_full)
            TelegramsDump();
    }
    printf("\n");
}
//------------------------------------------------------------------------------
void func_WriteMultipleRegisters()
{
    for (int c = 0; c < Amount; c++)
        Regs[c] = c;

    int Result = Client->WriteMultipleRegisters(DeviceID, 1, Amount, Regs);
    PrintStatus("(0x10) Write Multiple Registers");
    if (Result == mbNoError)
    {
        RegsDump(Amount);
        if (opt == dump_full)
            TelegramsDump();
    }
    printf("\n");
}
//------------------------------------------------------------------------------
void func_ReportServerID()
{
    int DataSize = 0;
    int Result = Client->ReportServerID(DeviceID, Regs, DataSize); // We use Regs[] as generic Buffer
    PrintStatus("(0x11) Report Server ID");
    if (Result == mbNoError)
    {
        printf("+---------------------------------------------------------------------------+\n");
        printf("Data Recvd (%d)\n", DataSize);
        hexdump(Regs, DataSize);

        if (opt == dump_full)
            TelegramsDump();
    }
    printf("\n");
}
//------------------------------------------------------------------------------
void func_ReadFileRecord()
{
    int Result = Client->ReadFileRecord(DeviceID,
        6, // RefType *must* be 6
        1, // FileNumber
        1, // RecNumber
        Amount, // Regs to Read
        Regs);  // we use Regs[] as read buffer
    PrintStatus("(0x14) Read File Record");
    if (Result == mbNoError)
    {
        RegsDump(Amount);
        if (opt == dump_full)
            TelegramsDump();
    }
    printf("\n");
}
//------------------------------------------------------------------------------
void func_WriteFileRecord()
{
    for (int c = 0; c < Amount; c++)
        Regs[c] = c;
    int Result = Client->WriteFileRecord(DeviceID,
        6, // RefType *must* be 6
        1, // FileNumber
        1, // RecNumber
        Amount, // Regs to Write
        Regs);  // we use Regs[] as write buffer
    PrintStatus("(0x15) Write File Record");
    if (Result == mbNoError)
    {
        RegsDump(Amount);
        if (opt == dump_full)
            TelegramsDump();
    }
    printf("\n");
}
//------------------------------------------------------------------------------
void func_MaskWriteRegister()
{
    // use the same example parameters of Modbus_Application_Protocol_V1_1b3.pdf
    // AND_Mask = 0x00F2
    // OR_Mask = 0x0025;
    // If you set Reg 1 = 0x0012, the result should be 0x0017
    uint16_t Reg_1_before;
    uint16_t Reg_1_after;
    uint16_t AND_Mask = 0x00F2;
    uint16_t OR_Mask = 0x0025;

    bool ReadOk = (Client->ReadHoldingRegisters(DeviceID, 1, 1, &Reg_1_before) == mbNoError);    

    int Result = Client->MaskWriteRegister(DeviceID, 1, AND_Mask, OR_Mask);
    PrintStatus("(0x16) Mask Write Register");

    if (Result == mbNoError)
    {
        if (ReadOk && (Client->ReadHoldingRegisters(DeviceID, 1, 1, &Reg_1_after) == mbNoError))
        {
            printf("-----------------------------\n");
            printf("Reg 1 Value before: 0x%04x\n", Reg_1_before);
            printf("AND Mask          : 0x%04x\n", AND_Mask);
            printf("OR Mask           : 0x%04x\n", OR_Mask);
            printf("Reg 1 Value after : 0x%04x\n", Reg_1_after);
        }
        if (opt == dump_full)
            TelegramsDump();
    }
    printf("\n");
}
//------------------------------------------------------------------------------
void func_ReadWriteMultiRegisters()
{
    for (int c = 0; c < Amount; c++)
        Regs[c] = c;

    int Result = Client->ReadWriteMultipleRegisters(DeviceID, 1, Amount, 1, Amount, Regs, Regs);
    PrintStatus("(0x10) Read/Write Multiple Registers");
    if (Result == mbNoError)
    {
        RegsDump(Amount);
        if (opt == dump_full)
            TelegramsDump();
    }
    printf("\n");
}
//------------------------------------------------------------------------------
void func_ReadFIFOQueue()
{
    uint16_t FifoCount = 0;
    uint16_t Fifo[256];

    int Result = Client->ReadFIFOQueue(DeviceID, 1, FifoCount, &Fifo);
    PrintStatus("(0x18) Read FIFO Queue");

    if (Result == mbNoError)
    {
        if (FifoCount > 256)
            FifoCount = 256;
        printf("-----------------------------\n");
        printf("Fifo Count        : 0x%04x\n", FifoCount);
        printf("Items[]\n");
        for (int c = 0; c < FifoCount; c++)
            printf("  (%3d) : 0x%04x\n", c, Fifo[c]);
        if (opt == dump_full)
            TelegramsDump();
    }
    printf("\n");
}
//------------------------------------------------------------------------------
void func_EncapsulatedInterfaceTransport()
{   
    uint8_t MEIDataOut[16] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 };
    uint8_t MEIDataIn[256];
    uint16_t WRSize = 16;
    uint16_t RDSize = 0;

    int Result = Client->ExecuteMEIFunction(DeviceID, 1, &MEIDataOut, WRSize, &MEIDataIn, RDSize);
    PrintStatus("(0x2B) Encapsulated Interface Transport (MEI)");
    if (Result == mbNoError)
    {
        if (RDSize > 256)
            RDSize = 256;
        printf("-----------------------------\n");
        printf("Data Dump (%d byte received):\n", RDSize);
        hexdump(MEIDataIn, RDSize);

        if (opt == dump_full)
            TelegramsDump();
    }
    printf("\n");
}
//------------------------------------------------------------------------------
void ExecuteAllFunctions(); //forward
//------------------------------------------------------------------------------
pfn_ModbusFunction ProgramFunctions[22] = {
    func_ReadCoils,
    func_ReadDiscreteInputs,
    func_ReadHoldingRegisters,
    func_ReadInputRegisters,
    func_WriteSingleCoil,
    func_WriteSingleRegister,
    func_ReadExceptionStatus,
    func_Diagnostics,
    func_GetCommEventCounter,
    func_GetCommEventLog,
    func_WriteMultipleCoils,
    func_WriteMultipleRegisters,
    func_ReportServerID,
    func_ReadFileRecord,
    func_WriteFileRecord,
    func_MaskWriteRegister,
    func_ReadWriteMultiRegisters,
    func_ReadFIFOQueue,
    func_EncapsulatedInterfaceTransport,
    ExecuteAllFunctions,
    GetAmount,
    GetDeviceID
};
//------------------------------------------------------------------------------
void ExecuteAllFunctions()
{
    TotalTime = 0;
    TestPassed = 0;
    for (int c = 0; c < 19; c++)
        ProgramFunctions[c]();
    printf("+---------------------------------------------------------------------------+\n");
    printf(" Test executed   : 19\n");
    printf(" Passed          : %d\n", TestPassed);
    printf(" Failed          : %d\n", 19 - TestPassed);
    printf(" Total time (ms) : %d\n", TotalTime);
    printf("+---------------------------------------------------------------------------+\n");
}
//------------------------------------------------------------------------------
void ExecuteOption(int Option)
{
    if (Option == 0)
        return;
    if (Option > 0 && Option < 23)
        ProgramFunctions[Option - 1]();
    else
        printf("Invalid Option\n");
}

//------------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    if (!ParseCmdLine(argc, argv))
    {
        Legenda();
        getchar();
        return 1;
    }

    if (!CreateClient(ct))
        return 1;

    if (mode == mode_Interactive)
    {
        int Option = 0;
        do
        {
            Menu();
            GetOption(Option);
            ExecuteOption(Option);
        } while (Option);
    }
    else
        ExecuteAllFunctions();

    delete Client;
}