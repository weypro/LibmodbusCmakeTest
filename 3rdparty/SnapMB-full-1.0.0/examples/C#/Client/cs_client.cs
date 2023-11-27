using System;
using System.Net;
using System.Runtime.InteropServices;
using System.Text;
using SnapModbus;

class ClientDemo
{
    static readonly int regs_amount = 32768;
    static readonly int bits_amount = 65536;

    enum ClientType
    {
        null_Client,
        tcp_Client,
        udp_Client,
        rtu_overTCP_Client,
        rtu_overUDP_Client,
        rtu_Client,
        asc_Client
    }
    static ClientType ct = ClientType.null_Client;

    enum DumpOption{
        dump_light,
        dump_full
    }
    static DumpOption opt = DumpOption.dump_full;

    enum ModeOption
    {
        mode_Interactive,
        mode_Batch
    }
    static ModeOption mode = ModeOption.mode_Interactive;

    const int DefaultMBPort = 502;

    // NET Params with defaults
    static string Address = "127.0.0.1"; // -a
    static int Port = DefaultMBPort;     // -p

    // SER Params with defaults
    static byte DeviceID = 1;            // -i
    static string ComPort = "COM5";      // -c
    static int BaudRate = 19200;         // -b
    static char Parity = 'E';            // -y
    static int DataBits = 8;             // -d
    static int StopBits = 1;             // -s

    static UInt16 Amount = 8;
    static int TestPassed = 0;
    static uint TotalTime = 0;

    // Our Polimorphic Client
    static SnapMBBroker Client;

    // Resources
    static ushort[] Regs = new ushort[regs_amount];
    static byte[] Bits = new byte[bits_amount];
    //------------------------------------------------------------------------------
    // Dumps Hex 
    //------------------------------------------------------------------------------
    static void HexDump(byte[] bytes, int Size)
    {
        if (bytes == null)
            return;
        int bytesLength = Size;
        int bytesPerLine = 16;

        char[] HexChars = "0123456789ABCDEF".ToCharArray();

        int firstHexColumn =
              8                   // 8 characters for the address
            + 3;                  // 3 spaces

        int firstCharColumn = firstHexColumn
            + bytesPerLine * 3       // - 2 digit for the hexadecimal value and 1 space
            + (bytesPerLine - 1) / 8 // - 1 extra space every 8 characters from the 9th
            + 2;                  // 2 spaces 

        int lineLength = firstCharColumn
            + bytesPerLine           // - characters to show the ascii value
            + Environment.NewLine.Length; // Carriage return and line feed (should normally be 2)

        char[] line = (new String(' ', lineLength - 2) + Environment.NewLine).ToCharArray();
        int expectedLines = (bytesLength + bytesPerLine - 1) / bytesPerLine;
        StringBuilder result = new StringBuilder(expectedLines * lineLength);

        for (int i = 0; i < bytesLength; i += bytesPerLine)
        {
            line[0] = HexChars[(i >> 28) & 0xF];
            line[1] = HexChars[(i >> 24) & 0xF];
            line[2] = HexChars[(i >> 20) & 0xF];
            line[3] = HexChars[(i >> 16) & 0xF];
            line[4] = HexChars[(i >> 12) & 0xF];
            line[5] = HexChars[(i >> 8) & 0xF];
            line[6] = HexChars[(i >> 4) & 0xF];
            line[7] = HexChars[(i >> 0) & 0xF];

            int hexColumn = firstHexColumn;
            int charColumn = firstCharColumn;

            for (int j = 0; j < bytesPerLine; j++)
            {
                if (j > 0 && (j & 7) == 0) hexColumn++;
                if (i + j >= bytesLength)
                {
                    line[hexColumn] = ' ';
                    line[hexColumn + 1] = ' ';
                    line[charColumn] = ' ';
                }
                else
                {
                    byte b = bytes[i + j];
                    line[hexColumn] = HexChars[(b >> 4) & 0xF];
                    line[hexColumn + 1] = HexChars[b & 0xF];
                    line[charColumn] = (b < 32 ? '·' : (char)b);
                }
                hexColumn += 3;
                charColumn++;
            }
            result.Append(line);
        }
        Console.Write(result.ToString());
    }
    //------------------------------------------------------------------------------
    // Dumps Registers
    //------------------------------------------------------------------------------
    static void RegsDump(int Count)
    {
        Console.WriteLine("+-----------------------------------------------------------------------------+");
        Console.WriteLine("Registers Dump ({0:d})", Count);
        for (int c = 0; c < Count; c++)
        {
            Console.Write("{0:x4} ", Regs[c]);
            if (((c + 1) % 8) == 0 && c != Count - 1)
                Console.WriteLine();
        }
        Console.WriteLine();
    }
    //------------------------------------------------------------------------------
    // Dumps Bits      
    //------------------------------------------------------------------------------
    static void BitsDump(int Count)
    {
        Console.WriteLine("+-----------------------------------------------------------------------------+");
        Console.WriteLine("Bits Dump ({0:d})", Count);
        for (int c = 0; c < Count; c++)
        {
            Console.Write("{0:d} ", Bits[c]);
            if (((c + 1) % 16) == 0 && c != Count - 1)
                Console.WriteLine();
        }
        Console.WriteLine();
    }
    //------------------------------------------------------------------------------
    // Dumps Telegrams
    //------------------------------------------------------------------------------
    static void TelegramsDump()
    {
        if (opt != DumpOption.dump_full)
            return;
        byte[] Data = new byte[1024];
        int Size;
        Size = Client.GetIOBuffer(Data, 0);
        if (Size > 0)
        {
            Console.WriteLine("+-----------------------------------------------------------------------------+");
            Console.WriteLine("Request Dump ({0:d})", Size);
            HexDump(Data, Size);
        }
        Size = Client.GetIOBuffer(Data, 1);
        if (Size > 0)
        {
            Console.WriteLine("+-----------------------------------------------------------------------------+");
            Console.WriteLine("Response Dump ({0:d})", Size);
            HexDump(Data, Size);
        }
    }

    //------------------------------------------------------------------------------
    // Print Result
    //------------------------------------------------------------------------------
    static void PrintStatus(String Info)
    {
        String[] DeviceStatus = { "Unknown", "Ok", "Timeout", "Hardware error", "Protocol error" };
        String[] ConnectionStatus = { "NOT Connected", "Connected" };

        MBConsts.DeviceStatus Status = new MBConsts.DeviceStatus { LastError = 0, Connected = 0, Status = 0, JobTime = 0 };
        Console.WriteLine();
        Console.WriteLine("+-----------------------------------------------------------------------------+");
        Console.WriteLine(" {0:s}", Info);
        Console.WriteLine("+-----------------------------------------------------------------------------+");
        Client.GetDeviceStatus(DeviceID, ref Status);
        Console.WriteLine("Function Result   : {0:s}", MB.ErrorText(Status.LastError));
        Console.WriteLine("Device Status     : {0:s}", DeviceStatus[Status.Status]);
        Console.WriteLine("Job Time (ms)     : {0:d}", Status.JobTime);
        Console.WriteLine("Connection Status : {0:s}", ConnectionStatus[Status.Connected]);
        if (Status.LastError == MBConsts.mbNoError)
            TestPassed++;
        TotalTime += Status.JobTime;
    }
    //------------------------------------------------------------------------------
    // Program Usage   
    //------------------------------------------------------------------------------
    static void Legenda()
    {
        Console.WriteLine("");
        Console.WriteLine("Usage:");
        Console.WriteLine("Client <ClientType> [<ClientParams>] [<Options>]");
        Console.WriteLine("");
        Console.WriteLine("ClientType (Mandatory, no case sensitive):");
        Console.WriteLine("  tcp             : Modbus/TCP Client");
        Console.WriteLine("  udp             : Modbus/UDP Client");
        Console.WriteLine("  RtuOverTcp      : RTU Telegram Over Modbus/TCP Client");
        Console.WriteLine("  RtuOverUdp      : RTU Telegram Over Modbus/UDP Client");
        Console.WriteLine("  rtu             : RTU (serial) Client");
        Console.WriteLine("  asc             : ASCII (serial) Client");
        Console.WriteLine("");
        Console.WriteLine("Ethernet Client Params (Optionals):");
        Console.WriteLine("  -a <Address>    : IP Address (default 127.0.0.1)");
        Console.WriteLine("  -p <Port>       : IP Port (default 502)");
        Console.WriteLine("");
        Console.WriteLine("Serial Client Params (Optionals):");
        Console.WriteLine("  -c <ComPort>    : ComPort Name (Default COM5");
        Console.WriteLine("  -b <BaudRate>   : BaudRate (Default 19200)");
        Console.WriteLine("  -y <Parity>     : Parity, N or E or O (Default E)");
        Console.WriteLine("  -d <DataBits>   : DataBits, 7 or 8 (Default 8)");
        Console.WriteLine("  -s <StopBits>   : StopBits, 1 or 2 (Default 1)");
        Console.WriteLine("");
        Console.WriteLine("Options:");
        Console.WriteLine("  -g <DumpOption> : L (Light Dump), F (Full Dump) (Default F)");
        Console.WriteLine("  -m <ModeOption> : I (Interactive), A (All tests) (Default I)");
        Console.WriteLine("");
        Console.WriteLine("Examples:");
        Console.WriteLine("  Client tcp -a 192.168.0.15 -p 5020");
        Console.WriteLine("  Client rtu -c COM7 -b 115200 -y N");
        Console.WriteLine("");
    }
    
    static void Menu()
    {
        Console.WriteLine("+-----------------------------------------------------------------------------+");
        Console.WriteLine("|                           MODBUS Functions available                        |");
        Console.WriteLine("+-----------------------------------------------------------------------------+");
        Console.WriteLine("| 1 - (0x01) Read Coils                11 - (0x0F) Write Multiple Coils       |");
        Console.WriteLine("| 2 - (0x02) Read Discrete Inputs      12 - (0x10) Write Multiple Registers   |");
        Console.WriteLine("| 3 - (0x03) Read Holding Registers    13 - (0x11) Report Server ID           |");
        Console.WriteLine("| 4 - (0x04) Read Input Registers      14 - (0x14) Read File Record           |");
        Console.WriteLine("| 5 - (0x05) Write Single Coil         15 - (0x15) Write File Record          |");
        Console.WriteLine("| 6 - (0x06) Write Single Register     16 - (0x16) Mask Write Register        |");
        Console.WriteLine("| 7 - (0x07) Read Exception Status     17 - (0x17) Read/Write Multi Registers |");
        Console.WriteLine("| 8 - (0x08) Diagnostics               18 - (0x18) Read FIFO Queue            |");
        Console.WriteLine("| 9 - (0x0B) Get Comm Event Counter    19 - (0x2B) Encapsulated Intf.Transp.  |");
        Console.WriteLine("|10 - (0x0C) Get Comm Event Log        20 - Execute All                       |");
        Console.WriteLine("+-----------------------------------------------------------------------------+");
        Console.WriteLine("|Options                                                                      |");
        Console.WriteLine("+-----------------------------------------------------------------------------+");
        Console.WriteLine("|20 - Overrides the number of Regs/Coils to Read/Write (Current is {0:d5})     |", Amount);
        Console.WriteLine("|21 - Overrides the target Device ID (Default is {0:d3})                         |", DeviceID);
        Console.WriteLine("+-----------------------------------------------------------------------------+");
        Console.WriteLine("|0  - Exit program                                                            |");
        Console.WriteLine("+-----------------------------------------------------------------------------+");
    }

    static bool Find(string ParamName, ref string Param, int argc, string[] args)
    {
        for (int c = 0; c < argc; c++)
        {
            string s = args[c].ToUpper();
            if (ParamName == args[c].ToUpper())
            {
                if (c < argc - 1)
                {
                    Param = args[c + 1];
                    return true;
                }
                else
                    return false;
            }
        }
        return false;
    }

    static ClientType GetClientType(string ClientName)
    {
        string upClientName = ClientName.ToUpper();

        if (upClientName == "TCP")
            return ClientType.tcp_Client;
        if (upClientName == "UDP")
            return ClientType.udp_Client;
        if (upClientName == "RTUOVERTCP")
            return ClientType.rtu_overTCP_Client;
        if (upClientName == "RTUOVERUDP")
            return ClientType.rtu_overUDP_Client;
        if (upClientName == "RTU")
            return ClientType.rtu_Client;
        if (upClientName == "ASC")
            return ClientType.asc_Client;

        return ClientType.null_Client;
    }

    static bool ParseCmdline(string[] args)
    {
        string Param = "";

        if (args.Length < 1)
            return false;

        ct = GetClientType(args[0]);
        if (ct == ClientType.null_Client)
            return false;

        if (ct == ClientType.rtu_Client || ct == ClientType.asc_Client)
        {
            if (Find("-C", ref Param, args.Length, args))
                ComPort = Param;
            if (Find("-B", ref Param, args.Length, args))
                BaudRate = Convert.ToInt32(Param);
            if (Find("-Y", ref Param, args.Length, args))
                Parity = Param[0];
            if (Find("-D", ref Param, args.Length, args))
                DataBits = Convert.ToInt32(Param);
            if (Find("-S", ref Param, args.Length, args))
                StopBits = Convert.ToInt32(Param);
        }
        else
        {
            if (Find("-A", ref Param, args.Length, args))
                Address = Param;
            if (Find("-P", ref Param, args.Length, args))
                Port = Convert.ToInt32(Param);
        }

        // Get Dump Option
        if (Find("-G", ref Param, args.Length, args))
        {
            if (Param == "L" || Param == "l")
                opt = DumpOption.dump_light;
        }

        // Get Mode Option
        if (Find("-M", ref Param, args.Length, args))
        {
            if (Param == "A" || Param == "a")
                mode = ModeOption.mode_Batch;
        }
        
        return true;
    }

    static bool CreateClient(ClientType Type)
    {
        switch (Type)
        {
            case ClientType.tcp_Client:
                Client = new SnapMBBroker(MBConsts.ProtoTCP, Address, Port);
                break;
            case ClientType.udp_Client:
                Client = new SnapMBBroker(MBConsts.ProtoUDP, Address, Port);
                break;
            case ClientType.rtu_overTCP_Client:
                Client = new SnapMBBroker(MBConsts.ProtoRTUOverTCP, Address, Port);
                break;
            case ClientType.rtu_overUDP_Client:
                Client = new SnapMBBroker(MBConsts.ProtoRTUOverUDP, Address, Port);
                break;
            case ClientType.rtu_Client:
                Client = new SnapMBBroker(MBConsts.FormatRTU, ComPort, BaudRate, Parity, DataBits, StopBits, 0);
                break;
            case ClientType.asc_Client:
                Client = new SnapMBBroker(MBConsts.FormatASC, ComPort, BaudRate, Parity, DataBits, StopBits, 0);
                break;
            default:
                Client = new SnapMBBroker(MBConsts.ProtoTCP, Address, Port);
                break;
        }

        bool IsSerial = ct == ClientType.rtu_Client || ct == ClientType.asc_Client;

        Console.WriteLine("");
        Console.WriteLine("Client created");
        Console.WriteLine("===============================================================================");

        if (IsSerial)
        {
            IsSerial = true;
            Console.WriteLine("DataLink  : Serial");
            if (Type == ClientType.rtu_Client)
                Console.WriteLine("Format    : RTU");
            else
                Console.WriteLine("Format    : ASCII");

            Console.WriteLine("Params    : {0:s}, {1:d}, {2:s}, {3:d}, {4:d}", ComPort, BaudRate, Parity, DataBits, StopBits);
        }
        else
        {
            Console.WriteLine("DataLink  : Ethernet");
            switch (Type)
            {
                case ClientType.tcp_Client:
                    Console.WriteLine("Transport : TCP (Modbus/TCP)");
                    break;
                case ClientType.udp_Client:
                    Console.WriteLine("Transport : UDP (Modbus/UDP)");
                    break;
                case ClientType.rtu_overTCP_Client:
                    Console.WriteLine("Transport : TCP (RTU over TCP)");
                    break;
                case ClientType.rtu_overUDP_Client:
                    Console.WriteLine("Transport : UDP (RTU over UDP)");
                    break;
            }
            Console.WriteLine("Params    : {0:s} : {1:d}", Address, Port);
        }

        // Connection
        int Result = Client.Connect();

        if (Result == MBConsts.mbNoError)
        {
            Console.WriteLine("Status    : Connected");
            return true;
        }
        else
        {
            Console.WriteLine("Status    : NOT Connected (exiting)");
            Console.WriteLine("Error     : {0:s}", MB.ErrorText(Result));
            Console.WriteLine("-------------------------------------------------------------------------------");
            return false;
        }

    }

    static void GetAmount()
    {
        UInt16 NewAmount;
        Console.Write("Enter new Amount > ");
        string s = Console.ReadLine();
        try
        {
            NewAmount = Convert.ToUInt16(s);
        }
        catch (Exception)
        {
            NewAmount = 0;
        }

        if (NewAmount > 0)
            Amount = NewAmount;
    }

    static void GetDeviceID()
    {
        byte NewDeviceID;
        Console.Write("Enter new DeviceID > ");
        string s = Console.ReadLine();
        try
        {
            NewDeviceID = Convert.ToByte(s);
        }
        catch (Exception)
        {
            NewDeviceID = 0;
        }

        if (NewDeviceID > 0) 
            DeviceID = NewDeviceID;
    }

    static void GetOption(ref int Option)
    {
        Console.Write("Select > ");
        string s = Console.ReadLine();
        try
        {
            Option = Convert.ToInt32(s);
        }
        catch (Exception)
        {
            Option = -1;
        }
    }

    //==============================================================================
    // Modbus Functions 
    //==============================================================================
    //------------------------------------------------------------------------------
    // Fn 0x01 : if Amount is greater than the specification, SnapMB automatically
    //           splits the function into subsequent calls
    //------------------------------------------------------------------------------
    static void func_ReadCoils()
    {
        int Result = Client.ReadCoils(DeviceID, 1, Amount, Bits);
        PrintStatus("(0x01) Read Coils");
        if (Result == MBConsts.mbNoError)
        {
            BitsDump(Amount);
            TelegramsDump();
        }
        Console.WriteLine();
    }
    //------------------------------------------------------------------------------
    // Fn 0x02 : if Amount is greater than the specification, SnapMB automatically
    //           splits the function into subsequent calls
    //------------------------------------------------------------------------------
    static void func_ReadDiscreteInputs()
    {
        int Result = Client.ReadDiscreteInputs(DeviceID, 1, Amount, Bits);
        PrintStatus("(0x01) Read Discrete Inputs");
        if (Result == MBConsts.mbNoError)
        {
            BitsDump(Amount);
            TelegramsDump();
        }
        Console.WriteLine();
    }
    //------------------------------------------------------------------------------
    // Fn 0x03 : if Amount is greater than the specification, SnapMB automatically
    //           splits the function into subsequent calls
    //------------------------------------------------------------------------------
    static void func_ReadHoldingRegisters()
    {
        int Result = Client.ReadHoldingRegisters(DeviceID, 1, Amount, Regs);

        PrintStatus("(0x03) Read Holding Registers");
        if (Result == MBConsts.mbNoError)
        {
            RegsDump(Amount);
            TelegramsDump();
        }
        Console.WriteLine();
    }
    //------------------------------------------------------------------------------
    // Fn 0x04 : if Amount is greater than the specification, SnapMB automatically
    //           splits the function into subsequent calls
    //------------------------------------------------------------------------------
    static void func_ReadInputRegisters()
    {
        int Result = Client.ReadInputRegisters(DeviceID, 1, Amount, Regs);

        PrintStatus("(0x04) Read Input Registers");
        if (Result == MBConsts.mbNoError)
        {
            RegsDump(Amount);
            TelegramsDump();
        }
        Console.WriteLine();
    }
    //------------------------------------------------------------------------------
    // Fn 0x05 : Nothing special to remark
    //------------------------------------------------------------------------------
    static void func_WriteSingleCoil()
    {
        int Result = Client.WriteSingleCoil(DeviceID, 1, true);
        PrintStatus("(0x05) Write Single Coil");
        if (Result == MBConsts.mbNoError)
            TelegramsDump();
        Console.WriteLine();
    }
    //------------------------------------------------------------------------------
    // Fn 0x06 : Nothing special to remark
    //------------------------------------------------------------------------------
    static void func_WriteSingleRegister()
    {
        Regs[0] = 0x0123;
        int Result = Client.WriteSingleRegister(DeviceID, 1, Regs[0]);
        PrintStatus("(0x06) Write Single Register");
        if (Result == MBConsts.mbNoError)
        {
            RegsDump(1);
            TelegramsDump();
        }
        Console.WriteLine();
    }
    //------------------------------------------------------------------------------
    // Fn 0x07 : Nothing special to remark
    //------------------------------------------------------------------------------
    static void func_ReadExceptionStatus()
    {
        byte Status = 0;
        int Result = Client.ReadExceptionStatus(DeviceID, ref Status);
        PrintStatus("(0x07) Read Exception Status");
        if (Result == MBConsts.mbNoError)
        {
            Console.WriteLine("-----------------------------");
            Console.WriteLine("Exception Status  : 0x{0:x2}", Status);
            TelegramsDump();
        }
        Console.WriteLine();
    }
    //------------------------------------------------------------------------------
    // Fn 0x08 : Here we sent an arbitrary 0x5555 just for test 
    //------------------------------------------------------------------------------
    static void func_Diagnostics()
    {
        ushort ItemsReceived = 0;
        Regs[0] = 0x5555;
        ushort SubFun = 0;
        int Result = Client.Diagnostics(DeviceID, SubFun, Regs, Regs, 1, ref ItemsReceived);
        PrintStatus("(0x08) Diagnostics");
        if (Result == MBConsts.mbNoError)
        {
            Console.WriteLine("-----------------------------");
            Console.WriteLine("SubFun sent       : 0x0000 (Echo)");
            Console.WriteLine("Items sent (1)    : 0x5555");
            Console.WriteLine("Items recv ({0:d}) : 0x{0:x4}", ItemsReceived, Regs[0]);
            TelegramsDump();
        }
        Console.WriteLine();
    }
    //------------------------------------------------------------------------------
    // Fn 0x09 : Nothing special to remark
    //------------------------------------------------------------------------------
    static void func_GetCommEventCounter()
    {
        ushort Status = 0;
        ushort EventsCount = 0;
        int Result = Client.GetCommEventCounter(DeviceID, ref Status, ref EventsCount);
        PrintStatus("(0x0B) Get Comm Event Counter");

        if (Result == MBConsts.mbNoError)
        {
            Console.WriteLine("-----------------------------");
            Console.WriteLine("Status            : 0x{0:x4}", Status);
            Console.WriteLine("Events Count      : 0x{0:x4}", EventsCount);
            TelegramsDump();
        }
        Console.WriteLine();
    }
    //------------------------------------------------------------------------------
    // Fn 0x0C : Nothing special to remark
    //------------------------------------------------------------------------------
    static void func_GetCommEventLog()
    {
        ushort Status = 0;
        ushort EventsCount = 0;
        ushort MessageCount = 0;
        ushort NumItems = 0;
        byte[] Items = new byte[256];

        int Result = Client.GetCommEventLog(DeviceID, ref Status, ref EventsCount, ref MessageCount, ref NumItems, Items);
        PrintStatus("(0x0C) Get Comm Event Log");

        if (Result == MBConsts.mbNoError)
        {
            if (MessageCount > 256)
                MessageCount = 256;
            Console.WriteLine("-----------------------------");
            Console.WriteLine("Status            : 0x{0:x4}", Status);
            Console.WriteLine("Events Count      : 0x{0:x4}", EventsCount);
            Console.WriteLine("Message Count     : 0x{0:x4}", MessageCount);
            Console.WriteLine("Num Items         : 0x{0:x4}", NumItems);
            Console.WriteLine("Items[]");
            for (int c = 0; c < NumItems; c++)
                Console.WriteLine("  ({0:d}) : 0x{1:x2}", c, Items[c]);
            TelegramsDump();
        }
        Console.WriteLine();
    }
    //------------------------------------------------------------------------------
    // Fn 0x0F : Here we supply an alternate 0/1 pattern
    //------------------------------------------------------------------------------
    static void func_WriteMultipleCoils()
    {
        for (int c = 0; c < Amount; c++)
            Bits[c] = Convert.ToByte(c % 2);  // Alternates 0 and 1

        int Result = Client.WriteMultipleCoils(DeviceID, 1, Amount, Bits);
        PrintStatus("(0x0F) Write Multiple Coils");
        if (Result == MBConsts.mbNoError)
        {
            BitsDump(Amount);
            TelegramsDump();
        }
        Console.WriteLine();
    }
    //------------------------------------------------------------------------------
    // Fn 0x10 : Here we fill Regs with increasing values
    //------------------------------------------------------------------------------
    static void func_WriteMultipleRegisters()
    {
        for (ushort c = 0; c < Amount; c++)
            Regs[c] = c;

        int Result = Client.WriteMultipleRegisters(DeviceID, 1, Amount, Regs);
        PrintStatus("(0x10) Write Multiple Registers");
        if (Result == MBConsts.mbNoError)
        {
            RegsDump(Amount);
        }
        Console.WriteLine();
    }
    //------------------------------------------------------------------------------
    // Fn 0x11 : Here we use Bits[] as generic byte buffer   
    //------------------------------------------------------------------------------
    static void func_ReportServerID()
    {
        int DataSize = 0;
        int Result = Client.ReportServerID(DeviceID, Bits, ref DataSize); 
        PrintStatus("(0x11) Report Server ID");
        if (Result == MBConsts.mbNoError)
        {
            Console.WriteLine("+---------------------------------------------------------------------------+");
            Console.WriteLine("Data Recvd ({0:d})", DataSize);
            HexDump(Bits, DataSize);
            TelegramsDump();
        }
        Console.WriteLine();
    }
    //------------------------------------------------------------------------------
    // Fn 0x14 : Here we use Regs[] as generic ushort buffer   
    //------------------------------------------------------------------------------
    static void func_ReadFileRecord()
    {
        int Result = Client.ReadFileRecord(DeviceID,
            6, // RefType *must* be 6
            1, // FileNumber
            1, // RecNumber
            Amount, // Regs to Read
            Regs);  // we use Regs[] as read buffer
        PrintStatus("(0x14) Read File Record");
        if (Result == MBConsts.mbNoError)
        {
            RegsDump(Amount);
            TelegramsDump();
        }
        Console.WriteLine();
    }
    //------------------------------------------------------------------------------
    // Fn 0x15 : Here we use Regs[] as generic ushort buffer which we fill   
    //------------------------------------------------------------------------------
    static void func_WriteFileRecord()
    {
        for (ushort c = 0; c < Amount; c++)
            Regs[c] = c;
        int Result = Client.WriteFileRecord(DeviceID,
            6, // RefType *must* be 6
            1, // FileNumber
            1, // RecNumber
            Amount, // Regs to Read
            Regs);  // we use Regs[] as write buffer
        PrintStatus("(0x15) Write File Record");
        if (Result == MBConsts.mbNoError)
        {
            RegsDump(Amount);
            TelegramsDump();
        }
        Console.WriteLine();
    }
    //------------------------------------------------------------------------------
    // Fn 0x16 :
    //  - Here we use the same example parameters of Modbus_Application_Protocol_V1_1b3.pdf
    //  - AND_Mask = 0x00F2
    //  - OR_Mask = 0x0025;
    //  - If you set Reg 1 = 0x0012, the result should be 0x0017
    //------------------------------------------------------------------------------
    static void func_MaskWriteRegister()
    {
        ushort[] Reg_1_before = new ushort[1];
        ushort[] Reg_1_after = new ushort[1];
        ushort AND_Mask = 0x00F2;
        ushort OR_Mask = 0x0025;

        // Read the value before (this is only for debug purpose)
        bool ReadOk = (Client.ReadHoldingRegisters(DeviceID, 1, 1, Reg_1_before) == MBConsts.mbNoError);

        int Result = Client.MaskWriteRegister(DeviceID, 1, AND_Mask, OR_Mask);
        PrintStatus("(0x16) Mask Write Register");

        if (Result == MBConsts.mbNoError)
        {
            // Read the value after
            if (ReadOk && (Client.ReadHoldingRegisters(DeviceID, 1, 1, Reg_1_after) == MBConsts.mbNoError))
            {
                Console.WriteLine("-----------------------------");
                Console.WriteLine("Reg 1 Value before: 0x{0:x4}", Reg_1_before[0]);
                Console.WriteLine("AND Mask          : 0x{0:x4}", AND_Mask);
                Console.WriteLine("OR Mask           : 0x{0:x4}", OR_Mask);
                Console.WriteLine("Reg 1 Value after : 0x{0:x4}", Reg_1_after[0]);
            }
            TelegramsDump();
        }
        Console.WriteLine();
    }
    //------------------------------------------------------------------------------
    // Fn 0x10 : We fill Regs in advance with increasing values
    //           This function is very nice if the device supports it, because
    //           we can Write and Read multiple Registers with ony one call
    //------------------------------------------------------------------------------
    static void func_ReadWriteMultiRegisters()
    {
        for (ushort c = 0; c < Amount; c++)
            Regs[c] = c;

        int Result = Client.ReadWriteMultipleRegisters(DeviceID, 1, Amount, 1, Amount, Regs, Regs);
        PrintStatus("(0x10) Read/Write Multiple Registers");
        if (Result == MBConsts.mbNoError)
        {
            RegsDump(Amount);
            TelegramsDump();
        }
        Console.WriteLine();
    }
    //------------------------------------------------------------------------------
    // Fn 0x18 : Nothing special to remark
    //------------------------------------------------------------------------------
    static void func_ReadFIFOQueue()
    {
        ushort FifoCount = 0;
        ushort[] Fifo = new ushort[256];

        int Result = Client.ReadFIFOQueue(DeviceID, 1, ref FifoCount, Fifo);
        PrintStatus("(0x18) Read FIFO Queue");

        if (Result == MBConsts.mbNoError)
        {
            if (FifoCount > 256)
                FifoCount = 256;
            Console.WriteLine("-----------------------------");
            Console.WriteLine("Fifo Count        : 0x%04x", FifoCount);
            Console.WriteLine("Items[]");
            for (int c = 0; c < FifoCount; c++)
                Console.WriteLine("  ({0:d3}) : 0x{1:x4}", c, Fifo[c]);
            TelegramsDump();
        }
        Console.WriteLine();
    }
    //------------------------------------------------------------------------------
    // Fn 0x2B : Nothing special to remark
    //------------------------------------------------------------------------------
    static void func_EncapsulatedInterfaceTransport()
    {
        byte[] MEIDataOut = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
        byte[] MEIDataIn = new byte[256];
        ushort WRSize = 16;
        ushort RDSize = 0;

        int Result = Client.ExecuteMEIFunction(DeviceID, 1, MEIDataOut, WRSize, MEIDataIn, ref RDSize);
        PrintStatus("(0x2B) Encapsulated Interface Transport (MEI)");
        if (Result == MBConsts.mbNoError)
        {
            if (RDSize > 256)
                RDSize = 256;
            Console.WriteLine("-----------------------------");
            Console.WriteLine("Data Dump ({0:d} byte received):", RDSize);
            HexDump(MEIDataIn, RDSize);
            TelegramsDump();
        }
        Console.WriteLine();
    }
    //------------------------------------------------------------------------------
    // Executes the Console Option
    //------------------------------------------------------------------------------
    static void ExecuteOption(int Option)
    {
        switch(Option)
        {
            case 0:
                break;
            case 1:
                func_ReadCoils();
                break;
            case 2:
                func_ReadDiscreteInputs();
                break;
            case 3:
                func_ReadHoldingRegisters();
                break;
            case 4:
                func_ReadInputRegisters();
                break;
            case 5:
                func_WriteSingleCoil();
                break;
            case 6:
                func_WriteSingleRegister();
                break;
            case 7:
                func_ReadExceptionStatus();
                break;
            case 8:
                func_Diagnostics();
                break;
            case 9:
                func_GetCommEventCounter();
                break;
            case 10:
                func_GetCommEventLog();
                break;
            case 11:
                func_WriteMultipleCoils();
                break;
            case 12:
                func_WriteMultipleRegisters();
                break;
            case 13:
                func_ReportServerID();
                break;
            case 14:
                func_ReadFileRecord();
                break;
            case 15:
                func_WriteFileRecord();
                break;
            case 16:
                func_MaskWriteRegister();
                break;
            case 17:
                func_ReadWriteMultiRegisters();
                break;
            case 18:
                func_ReadFIFOQueue();
                break;
            case 19:
                func_EncapsulatedInterfaceTransport();
                break;
            case 20:
                ExecuteAllFunctions();
                break;
            case 21:
                GetAmount();
                break;
            case 22:
                GetDeviceID();
                break;
            default:
                Console.WriteLine("Invalid Option");
                break;
        }

    }
    //------------------------------------------------------------------------------
    // Executes all the functions (useful for debug purpose)
    //------------------------------------------------------------------------------
    static void ExecuteAllFunctions()
    {
        TestPassed = 0;
        TotalTime = 0;
        func_ReadCoils();
        func_ReadDiscreteInputs();
        func_ReadHoldingRegisters();
        func_ReadInputRegisters();
        func_WriteSingleCoil();
        func_WriteSingleRegister();
        func_ReadExceptionStatus();
        func_Diagnostics();
        func_GetCommEventCounter();
        func_GetCommEventLog();
        func_WriteMultipleCoils();
        func_WriteMultipleRegisters();
        func_ReportServerID();
        func_ReadFileRecord();
        func_WriteFileRecord();
        func_MaskWriteRegister();
        func_ReadWriteMultiRegisters();
        func_ReadFIFOQueue();
        func_EncapsulatedInterfaceTransport();
        // Show the report
        Console.WriteLine("+-----------------------------------------------------------------------------+");
        Console.WriteLine(" Test executed   : 19");
        Console.WriteLine(" Passed          : {0:d}", TestPassed);
        Console.WriteLine(" Failed          : {0:d}", 19 - TestPassed);
        Console.WriteLine(" Total time (ms) : {0:d}", TotalTime);
        Console.WriteLine("+-----------------------------------------------------------------------------+");
    }
    //------------------------------------------------------------------------------
    // Main
    //------------------------------------------------------------------------------
    static void Main(string[] args)
    {

        if (!ParseCmdline(args))
        {
            Legenda();
            return;
        }

        if (!CreateClient(ct))
            return;

        if (mode == ModeOption.mode_Interactive)
        {
            int Option = 0;
            do
            {
                Menu();
                GetOption(ref Option);
                ExecuteOption(Option);
            } while (Option!=0);
        }
        else
            ExecuteAllFunctions();
    }

}