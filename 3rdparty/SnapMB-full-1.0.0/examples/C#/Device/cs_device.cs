using System;
using System.Net;
using System.Runtime.InteropServices;
using System.Text;
using SnapModbus;

class DeviceDemo
{
    static readonly int regs_amount = 32768;
    static readonly int bits_amount = 65536;

    enum DeviceType
    {
        null_Device,
        tcp_Device,
        udp_Device,
        rtu_overTCP_Device,
        rtu_overUDP_Device,
        rtu_Device,
        asc_Device
    }
    static DeviceType dt = DeviceType.null_Device;

    enum Option
    {
        opt_log,
        opt_dump
    }
    static Option opt = Option.opt_log;

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

    // Our Polimorphic Device
    static SnapMBDevice Device;

    // Callbacks
    static readonly TDeviceEvent DeviceEvent = new TDeviceEvent(OnEvent);
    static readonly TPacketLog PacketLog = new TPacketLog(OnPacketLog);
    static readonly TUsrFunctionRequest UsrFunctionRequest = new TUsrFunctionRequest(OnUserFunction);

    // Resources
    static ushort[] HoldingRegisters = new ushort[regs_amount];
    static ushort[] InputRegisters = new ushort[regs_amount];
    static byte[] Coils = new byte[bits_amount];
    static byte[] DiscreteInputs = new byte[bits_amount];

    //------------------------------------------------------------------------------
    // Callbacks
    //------------------------------------------------------------------------------
    static void OnEvent(IntPtr usrPtr, ref MBConsts.DeviceEvent Event, int Size)
    {
        Console.WriteLine(MB.EventText(ref Event));
    }


    static int OnUserFunction(IntPtr usrPtr, byte Function, IntPtr RxPDU, int RxPDUSize, IntPtr TxPDU, ref ushort TxPDUSize)
    {
        if (Function == 0x41)
        {
            // Do something
        }              
        return 0;
    }

    static void OnPacketLog(IntPtr UsrPtr, UInt32 Peer, int Direction, IntPtr Data, int Size)
    {
        if (Direction == MBConsts.PacketLog_IN)
            Console.WriteLine("Indication received ("+Convert.ToInt32(Size)+" byte)");
        else
            Console.WriteLine("Confirmation sent (" + Convert.ToInt32(Size) + " byte)");

        Console.WriteLine("------------------------------------------------------------------------------");

        // Note : The marshaller doesn't know how big is Intptr (i.e. the byte array) so we cannot
        // use byte[] as input parameter. 
        // We need an IntPtr then copy it into a local byte[] array knowing the size.
        byte[] Packet = new byte[Size];
        Marshal.Copy(Data, Packet, 0, Size);
        HexDump(Packet, Size);
        Console.WriteLine("------------------------------------------------------------------------------");

        if (Direction == MBConsts.PacketLog_OUT)
            Console.WriteLine("");
    }

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
        Console.WriteLine(result.ToString());
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

    static DeviceType GetDeviceType(string DeviceName)
    {
        string upDeviceName = DeviceName.ToUpper();

        if (upDeviceName == "TCP")
            return DeviceType.tcp_Device;
        if (upDeviceName == "UDP")
            return DeviceType.udp_Device;
        if (upDeviceName == "RTUOVERTCP")
            return DeviceType.rtu_overTCP_Device;
        if (upDeviceName == "RTUOVERUDP")
            return DeviceType.rtu_overUDP_Device;
        if (upDeviceName == "RTU")
            return DeviceType.rtu_Device;
        if (upDeviceName == "ASC")
            return DeviceType.asc_Device;

        return DeviceType.null_Device;
    }

    static bool ParseCmdline(string[] args)
    {
        string Param = "";

        if (args.Length < 1)
            return false;
        
        dt = GetDeviceType(args[0]);
        if (dt==DeviceType.null_Device) 
            return false;

        if (dt== DeviceType.rtu_Device || dt == DeviceType.asc_Device)
        {
            if (Find("-I", ref Param, args.Length, args))
                DeviceID = Convert.ToByte(Param);
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

        if (Find("-G", ref Param, args.Length, args))
        {
            if (Param=="D" || Param=="d")
                opt = Option.opt_dump;
        }
        return true;
    }

    static SnapMBDevice CreateDevice(DeviceType Type)
    {
        switch (Type)
        {
            case DeviceType.tcp_Device:
                return new SnapMBDevice(MBConsts.ProtoTCP, DeviceID, Address, Port);
            case DeviceType.udp_Device:
                return new SnapMBDevice(MBConsts.ProtoUDP, DeviceID, Address, Port);
            case DeviceType.rtu_overTCP_Device:
                return new SnapMBDevice(MBConsts.ProtoRTUOverTCP, DeviceID, Address, Port);
            case DeviceType.rtu_overUDP_Device:
                return new SnapMBDevice(MBConsts.ProtoRTUOverUDP, DeviceID, Address, Port);
            case DeviceType.rtu_Device:
                return new SnapMBDevice(MBConsts.FormatRTU, DeviceID, ComPort, BaudRate, Parity, DataBits, StopBits, 0);
            case DeviceType.asc_Device:
                return new SnapMBDevice(MBConsts.FormatASC, DeviceID, ComPort, BaudRate, Parity, DataBits, StopBits, 0);
            default:
                return new SnapMBDevice(MBConsts.ProtoTCP, DeviceID, Address, Port);
        }
    }

    static void Legenda()
    {
        Console.WriteLine("");
        Console.WriteLine("Usage:");
        Console.WriteLine("Device <DeviceType> [<DeviceParams>] [<Options>]");
        Console.WriteLine("");
        Console.WriteLine("DeviceType (Mandatory, no case sensitive):");
        Console.WriteLine("  tcp             : Modbus/TCP Device");
        Console.WriteLine("  udp             : Modbus/UDP Device");
        Console.WriteLine("  RtuOverTcp      : RTU Telegram Over Modbus/TCP Device");
        Console.WriteLine("  RtuOverUdp      : RTU Telegram Over Modbus/UDP Device");
        Console.WriteLine("  rtu             : RTU (serial) Device");
        Console.WriteLine("  asc             : ASCII (serial) Device");
        Console.WriteLine("");
        Console.WriteLine("Ethernet Device Params (Optionals):");
        Console.WriteLine("  -a <Address>    : IP Address (default 127.0.0.1)");
        Console.WriteLine("  -p <Port>       : IP Port (default 502)");
        Console.WriteLine("");
        Console.WriteLine("Serial Device Params (Optionals):");
        Console.WriteLine("  -i <DeviceID>   : Device ID, 1..255 (Default 1)");
        Console.WriteLine("  -c <ComPort>    : ComPort Name (Default COM5");
        Console.WriteLine("  -b <BaudRate>   : BaudRate (Default 19200)");
        Console.WriteLine("  -y <Parity>     : Parity, N or E or O (Default E)");
        Console.WriteLine("  -d <DataBits>   : DataBits, 7 or 8 (Default 8)");
        Console.WriteLine("  -s <StopBits>   : StopBits, 1 or 2 (Default 1)");
        Console.WriteLine("");
        Console.WriteLine("Options:");
        Console.WriteLine("  -g <LogOption>  : L (Activity Log), D (Telegram Dump) (Default L)");
        Console.WriteLine("");
        Console.WriteLine("Examples:");
        Console.WriteLine("  Device tcp -a 192.168.0.15 -p 5020");
        Console.WriteLine("  Device rtu -c COM7 -b 115200 -y N");
        Console.WriteLine("");
    }

    static void Main(string[] args)
    {
        if (!ParseCmdline(args))
        {
            Legenda();
            return;
        }   

        Device = CreateDevice(dt);

        // Share the Resources
        Device.RegisterArea(MBConsts.mbaHoldingRegisters, ref HoldingRegisters, regs_amount);
        Device.RegisterArea(MBConsts.mbaInputRegisters, ref InputRegisters, regs_amount);
        Device.RegisterArea(MBConsts.mbaDiscreteInputs, ref DiscreteInputs, bits_amount);
        Device.RegisterArea(MBConsts.mbaCoils, ref Coils, bits_amount);

        if (opt==Option.opt_dump)
        {
            // Set Callback to receive Data Dump
            Device.RegisterCallback(MBConsts.cbkPacketLog, Marshal.GetFunctionPointerForDelegate(PacketLog), IntPtr.Zero);
            // Say to the device that we want both packet
            Device.SetParam(MBConsts.par_PacketLog, MBConsts.PacketLog_BOTH);   
        }
        else // Log
            Device.RegisterCallback(MBConsts.cbkDeviceEvent, Marshal.GetFunctionPointerForDelegate(DeviceEvent), IntPtr.Zero);

        Device.SetUserFunction(0x41, true);
        Device.SetUserFunction(0x42, true);
        Device.SetUserFunction(0x43, true);
        Device.RegisterCallback(MBConsts.cbkUsrFunction, Marshal.GetFunctionPointerForDelegate(UsrFunctionRequest), IntPtr.Zero);



        int Result = Device.Start();
        if (Result == 0) 
        {
            Console.WriteLine("Connected");
            Console.WriteLine("Press any key to terminate...");
            Console.WriteLine("");
            // Now the Device is running ... 
            Console.ReadKey();
        }
        else
            Console.WriteLine(MB.ErrorText(Result));

    }
}
