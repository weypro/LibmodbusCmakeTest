using SnapModbus;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Text;

namespace WinFormsClient
{
    public partial class Form1 : Form
    {
        static readonly int regs_amount = 32768;
        static readonly int bits_amount = 65536;
        // We create an Field controller which doesn't require params,
        // since we still don't know the type of Broker that we will want
        private SnapMBBroker Client = new SnapMBBroker();
        // Resources
        static ushort[] Regs = new ushort[regs_amount];
        static byte[] Bits = new byte[bits_amount];
        int TestPassed = 0;
        uint TotalTime = 0;

        public Form1()
        {
            InitializeComponent();
            cbTransport.SelectedIndex = 0;

            for (int i = 1; i < 65; i++)
                cbComPort.Items.Add("COM" + i.ToString());

            cbFormat.SelectedIndex = 0;
            cbComPort.SelectedIndex = 2;
            cbBaudrate.SelectedIndex = 2;
            cbBaudrate.SelectedIndex = 2;
            cbParity.SelectedIndex = 0;
            cbDatabits.SelectedIndex = 3;
            cbStopbits.SelectedIndex = 0;

            for (int i = 1; i < 256; i++)
                cbDeviceID.Items.Add(i.ToString());
            cbDeviceID.SelectedIndex = 0;

            for (int i = 1; i < 33; i++)
                cbAttempts.Items.Add(i.ToString());
            cbAttempts.SelectedIndex = 0;

            cbFunction.SelectedIndex = 0;

            btnConnect.Enabled = true;
            btnDisconnect.Enabled = false;
            cbFunction.Enabled = false;
            btnExecute.Enabled = false;
        }

        #region [Dump Functions]

        private void WriteLine()
        {
            Log.AppendText(Environment.NewLine);
        }

        private void WriteLine(string Message)
        {
            Log.AppendText(Message + Environment.NewLine);
        }

        private void WriteLine(string Text, params object[] args)
        {
            Log.AppendText(String.Format(Text, args) + Environment.NewLine);
        }

        private void Write(string Message)
        {
            Log.AppendText(Message);
        }

        private void Write(string Message, params object[] args)
        {
            Log.AppendText(String.Format(Message, args));
        }

        //------------------------------------------------------------------------------
        // Print Result
        //------------------------------------------------------------------------------
        private void HexDump(byte[] bytes, int Size)
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
                        line[charColumn] = (b < 32 ? '?' : (char)b);
                    }
                    hexColumn += 3;
                    charColumn++;
                }
                string s = new string(line);
                Write(s);
            }
        }

        //------------------------------------------------------------------------------
        // Print Result
        //------------------------------------------------------------------------------
        private void PrintStatus(String Info)
        {
            String[] DeviceStatus = { "Unknown", "Ok", "Timeout", "Hardware error", "Protocol error" };
            String[] ConnectionStatus = { "NOT Connected", "Connected" };

            MBConsts.DeviceStatus Status = new MBConsts.DeviceStatus { LastError = 0, Connected = 0, Status = 0, JobTime = 0 };
            WriteLine("===============================================================================");
            WriteLine(" {0:s}", Info);
            WriteLine("===============================================================================");
            Client.GetDeviceStatus(DeviceID, ref Status);
            WriteLine("Function Result   : {0:s}", MB.ErrorText(Status.LastError));
            WriteLine("Device Status     : {0:s}", DeviceStatus[Status.Status]);
            WriteLine("Job Time (ms)     : {0:d}", Status.JobTime);
            WriteLine("Connection Status : {0:s}", ConnectionStatus[Status.Connected]);
            if (Status.LastError == 0)
                TestPassed++;
            TotalTime += Status.JobTime;
        }
        //------------------------------------------------------------------------------
        // Dumps Telegrams
        //------------------------------------------------------------------------------
        private void TelegramsDump()
        {
            byte[] Data = new byte[1024];
            int Size;
            Size = Client.GetIOBuffer(Data, 0);
            if (Size > 0)
            {
                WriteLine("+-----------------------------------------------------------------------------+");
                WriteLine("Request Dump ({0:d})", Size);
                HexDump(Data, Size);
            }
            Size = Client.GetIOBuffer(Data, 1);
            if (Size > 0)
            {
                WriteLine("+-----------------------------------------------------------------------------+");
                WriteLine("Response Dump ({0:d})", Size);
                HexDump(Data, Size);
            }


        }
        //------------------------------------------------------------------------------
        // Dumps Bits      
        //------------------------------------------------------------------------------
        private void BitsDump(int Count)
        {
            WriteLine("+-----------------------------------------------------------------------------+");
            WriteLine("Bits Dump ({0:d})", Count);
            for (int c = 0; c < Count; c++)
            {
                Write("{0:d} ", Bits[c]);
                if (((c + 1) % 16) == 0 && c != Count - 1)
                    WriteLine();
            }
            WriteLine();
        }

        //------------------------------------------------------------------------------
        // Dumps Registers
        //------------------------------------------------------------------------------
        private void RegsDump(int Count)
        {
            WriteLine("+-----------------------------------------------------------------------------+");
            WriteLine("Registers Dump ({0:d})", Count);
            for (int c = 0; c < Count; c++)
            {
                Write("{0:x4} ", Regs[c]);
                if (((c + 1) % 8) == 0 && c != Count - 1)
                    WriteLine();
            }
            WriteLine();
        }
        #endregion

        #region [Modbus Functions]
        //------------------------------------------------------------------------------
        // Fn 0x01 : if Amount is greater than the specification, SnapMB automatically
        //           splits the function into subsequent calls
        //------------------------------------------------------------------------------
        private void func_ReadCoils()
        {
            int Result = Client.ReadCoils(DeviceID, RDStart, RDAmount, Bits);
            PrintStatus("(0x01) Read Coils");
            if (Result == MBConsts.mbNoError)
            {
                BitsDump(RDAmount);
                TelegramsDump();
            }
            WriteLine();
        }
        //------------------------------------------------------------------------------
        // Fn 0x02 : if Amount is greater than the specification, SnapMB automatically
        //           splits the function into subsequent calls
        //------------------------------------------------------------------------------
        private void func_ReadDiscreteInputs()
        {
            int Result = Client.ReadDiscreteInputs(DeviceID, RDStart, RDAmount, Bits);
            PrintStatus("(0x01) Read Discrete Inputs");
            if (Result == MBConsts.mbNoError)
            {
                BitsDump(RDAmount);
                TelegramsDump();
            }
            WriteLine();
        }
        //------------------------------------------------------------------------------
        // Fn 0x03 : if Amount is greater than the specification, SnapMB automatically
        //           splits the function into subsequent calls
        //------------------------------------------------------------------------------
        private void func_ReadHoldingRegisters()
        {
            int Result = Client.ReadHoldingRegisters(DeviceID, RDStart, RDAmount, Regs);

            PrintStatus("(0x03) Read Holding Registers");
            if (Result == MBConsts.mbNoError)
            {
                RegsDump(RDAmount);
                TelegramsDump();
            }
            WriteLine();
        }
        //------------------------------------------------------------------------------
        // Fn 0x04 : if Amount is greater than the specification, SnapMB automatically
        //           splits the function into subsequent calls
        //------------------------------------------------------------------------------
        private void func_ReadInputRegisters()
        {
            int Result = Client.ReadInputRegisters(DeviceID, RDStart, RDAmount, Regs);

            PrintStatus("(0x04) Read Input Registers");
            if (Result == MBConsts.mbNoError)
            {
                RegsDump(RDAmount);
                TelegramsDump();
            }
            WriteLine();
        }
        //------------------------------------------------------------------------------
        // Fn 0x05 : Nothing special to remark
        //------------------------------------------------------------------------------
        private void func_WriteSingleCoil()
        {
            int Result = Client.WriteSingleCoil(DeviceID, WRStart, true);
            PrintStatus("(0x05) Write Single Coil");
            if (Result == MBConsts.mbNoError)
                TelegramsDump();
            WriteLine();
        }
        //------------------------------------------------------------------------------
        // Fn 0x06 : Nothing special to remark
        //------------------------------------------------------------------------------
        private void func_WriteSingleRegister()
        {
            Regs[0] = 0x0123;
            int Result = Client.WriteSingleRegister(DeviceID, WRStart, Regs[0]);
            PrintStatus("(0x06) Write Single Register");
            if (Result == MBConsts.mbNoError)
            {
                RegsDump(1);
                TelegramsDump();
            }
            WriteLine();
        }
        //------------------------------------------------------------------------------
        // Fn 0x07 : Nothing special to remark
        //------------------------------------------------------------------------------
        private void func_ReadExceptionStatus()
        {
            byte Status = 0;
            int Result = Client.ReadExceptionStatus(DeviceID, ref Status);
            PrintStatus("(0x07) Read Exception Status");
            if (Result == MBConsts.mbNoError)
            {
                WriteLine("-----------------------------");
                WriteLine("Exception Status  : 0x{0:x2}", Status);
                TelegramsDump();
            }
            WriteLine();
        }
        //------------------------------------------------------------------------------
        // Fn 0x08 : Here we sent an arbitrary 0x5555 just for test 
        //------------------------------------------------------------------------------
        private void func_Diagnostics()
        {
            ushort ItemsReceived = 0;
            Regs[0] = 0x5555;
            ushort SubFun = 0;
            int Result = Client.Diagnostics(DeviceID, SubFun, Regs, Regs, 1, ref ItemsReceived);
            PrintStatus("(0x08) Diagnostics");
            if (Result == MBConsts.mbNoError)
            {
                WriteLine("-----------------------------");
                WriteLine("SubFun sent       : 0x0000 (Echo)");
                WriteLine("Items sent (1)    : 0x5555");
                WriteLine("Items recv (%d)    : 0x{0:x4}", ItemsReceived, Regs[0]);
                TelegramsDump();
            }
            WriteLine();
        }
        //------------------------------------------------------------------------------
        // Fn 0x09 : Nothing special to remark
        //------------------------------------------------------------------------------
        private void func_GetCommEventCounter()
        {
            ushort Status = 0;
            ushort EventsCount = 0;
            int Result = Client.GetCommEventCounter(DeviceID, ref Status, ref EventsCount);
            PrintStatus("(0x0B) Get Comm Event Counter");

            if (Result == MBConsts.mbNoError)
            {
                WriteLine("-----------------------------");
                WriteLine("Status            : 0x{0:x4}", Status);
                WriteLine("Events Count      : 0x{0:x4}", EventsCount);
                TelegramsDump();
            }
            WriteLine();
        }
        //------------------------------------------------------------------------------
        // Fn 0x0C : Nothing special to remark
        //------------------------------------------------------------------------------
        private void func_GetCommEventLog()
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
                WriteLine("-----------------------------");
                WriteLine("Status            : 0x{0:x4}", Status);
                WriteLine("Events Count      : 0x{0:x4}", EventsCount);
                WriteLine("Message Count     : 0x{0:x4}", MessageCount);
                WriteLine("Num Items         : 0x{0:x4}", NumItems);
                WriteLine("Items[]");
                for (int c = 0; c < NumItems; c++)
                    WriteLine("  ({0:d}) : 0x{1:x2}", c, Items[c]);
                TelegramsDump();
            }
            WriteLine();
        }
        //------------------------------------------------------------------------------
        // Fn 0x0F : Here we supply an alternate 0/1 pattern
        //------------------------------------------------------------------------------
        private void func_WriteMultipleCoils()
        {
            for (int c = 0; c < WRAmount; c++)
                Bits[c] = Convert.ToByte(c % 2);  // Alternates 0 and 1

            int Result = Client.WriteMultipleCoils(DeviceID, WRStart, WRAmount, Bits);
            PrintStatus("(0x0F) Write Multiple Coils");
            if (Result == MBConsts.mbNoError)
            {
                BitsDump(WRAmount);
                TelegramsDump();
            }
            WriteLine();
        }
        //------------------------------------------------------------------------------
        // Fn 0x10 : Here we fill Regs with increasing values
        //------------------------------------------------------------------------------
        private void func_WriteMultipleRegisters()
        {
            for (ushort c = 0; c < WRAmount; c++)
                Regs[c] = c;

            int Result = Client.WriteMultipleRegisters(DeviceID, WRStart, WRAmount, Regs);
            PrintStatus("(0x10) Write Multiple Registers");
            if (Result == MBConsts.mbNoError)
            {
                RegsDump(WRAmount);
            }
            WriteLine();
        }
        //------------------------------------------------------------------------------
        // Fn 0x11 : Here we use Bits[] as generic byte buffer   
        //------------------------------------------------------------------------------
        private void func_ReportServerID()
        {
            int DataSize = 0;
            int Result = Client.ReportServerID(DeviceID, Bits, ref DataSize);
            PrintStatus("(0x11) Report Server ID");
            if (Result == MBConsts.mbNoError)
            {
                WriteLine("+---------------------------------------------------------------------------+");
                WriteLine("Data Recvd ({0:d})", DataSize);
                HexDump(Bits, DataSize);
                TelegramsDump();
            }
            WriteLine();
        }
        //------------------------------------------------------------------------------
        // Fn 0x14 : Here we use Regs[] as generic ushort buffer   
        //------------------------------------------------------------------------------
        private void func_ReadFileRecord()
        {
            int Result = Client.ReadFileRecord(DeviceID,
                6, // RefType *must* be 6
                1, // FileNumber
                1, // RecNumber
                RDAmount, // Regs to Read
                Regs);  // we use Regs[] as read buffer
            PrintStatus("(0x14) Read File Record");
            if (Result == MBConsts.mbNoError)
            {
                RegsDump(RDAmount);
                TelegramsDump();
            }
            WriteLine();
        }
        //------------------------------------------------------------------------------
        // Fn 0x15 : Here we use Regs[] as generic ushort buffer which we fill   
        //------------------------------------------------------------------------------
        private void func_WriteFileRecord()
        {
            for (ushort c = 0; c < WRAmount; c++)
                Regs[c] = c;
            int Result = Client.WriteFileRecord(DeviceID,
                6, // RefType *must* be 6
                1, // FileNumber
                1, // RecNumber
                WRAmount, // Regs to Read
                Regs);  // we use Regs[] as write buffer
            PrintStatus("(0x15) Write File Record");
            if (Result == MBConsts.mbNoError)
            {
                RegsDump(WRAmount);
                TelegramsDump();
            }
            WriteLine();
        }
        //------------------------------------------------------------------------------
        // Fn 0x16 :
        //  - Here we use the same example parameters of Modbus_Application_Protocol_V1_1b3.pdf
        //  - AND_Mask = 0x00F2
        //  - OR_Mask = 0x0025;
        //  - If you set Reg 1 = 0x0012, the result should be 0x0017
        //------------------------------------------------------------------------------
        private void func_MaskWriteRegister()
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
                    WriteLine("-----------------------------");
                    WriteLine("Reg 1 Value before: 0x{0:x4}", Reg_1_before[0]);
                    WriteLine("AND Mask          : 0x{0:x4}", AND_Mask);
                    WriteLine("OR Mask           : 0x{0:x4}", OR_Mask);
                    WriteLine("Reg 1 Value after : 0x{0:x4}", Reg_1_after[0]);
                }
                TelegramsDump();
            }
            WriteLine();
        }
        //------------------------------------------------------------------------------
        // Fn 0x10 : We fill Regs in advance with increasing values
        //           This function is very nice if the device supports it, because
        //           we can Write and Read multiple Registers with ony one call
        //------------------------------------------------------------------------------
        private void func_ReadWriteMultiRegisters()
        {
            for (ushort c = 0; c < WRAmount; c++)
                Regs[c] = c;

            int Result = Client.ReadWriteMultipleRegisters(DeviceID, RDStart, RDAmount, WRStart, WRAmount, Regs, Regs);
            PrintStatus("(0x10) Read/Write Multiple Registers");
            if (Result == MBConsts.mbNoError)
            {
                RegsDump(RDAmount);
                TelegramsDump();
            }
            WriteLine();
        }
        //------------------------------------------------------------------------------
        // Fn 0x18 : Nothing special to remark
        //------------------------------------------------------------------------------
        private void func_ReadFIFOQueue()
        {
            ushort FifoCount = 0;
            ushort[] Fifo = new ushort[256];

            int Result = Client.ReadFIFOQueue(DeviceID, 1, ref FifoCount, Fifo);
            PrintStatus("(0x18) Read FIFO Queue");

            if (Result == MBConsts.mbNoError)
            {
                if (FifoCount > 256)
                    FifoCount = 256;
                WriteLine("-----------------------------");
                WriteLine("Fifo Count        : 0x%04x", FifoCount);
                WriteLine("Items[]");
                for (int c = 0; c < FifoCount; c++)
                    WriteLine("  ({0:d3}) : 0x{1:x4}", c, Fifo[c]);
                TelegramsDump();
            }
            WriteLine();
        }
        //------------------------------------------------------------------------------
        // Fn 0x2B : Nothing special to remark
        //------------------------------------------------------------------------------
        private void func_EncapsulatedInterfaceTransport()
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
                WriteLine("-----------------------------");
                WriteLine("Data Dump ({0:d} byte received):", RDSize);
                HexDump(MEIDataIn, RDSize);
                TelegramsDump();
            }
            WriteLine();
        }

        #endregion



        private void btnExecute_Click(object sender, EventArgs e)
        {

            switch (cbFunction.SelectedIndex + 1)
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
                default:
                    break;
            }
        }

        private void btnConnect_Click(object sender, EventArgs e)
        {
            if (PC.SelectedIndex == 0)
            {
                // We want an Ethernet Client
                Client.ChangeTo(cbTransport.SelectedIndex, edAddress.Text, (int)nPort.Value);
            }
            else
            {
                // We want a Serial Client
                char[] Parities = { 'E', 'O', 'N' };
                int[] Baudrates = { 4800, 9600, 19200, 38400, 57600, 115200, 128000, 256000, 500000, 921600 };
                int[] Datas = { 5, 6, 7, 8 };
                int[] Stops = { 1, 2 };

                char Parity = Parities[cbParity.SelectedIndex];
                int Baudrate = Baudrates[cbBaudrate.SelectedIndex];
                int DataBits = Datas[cbDatabits.SelectedIndex];
                int StopBits = Stops[cbStopbits.SelectedIndex];

                Client.ChangeTo(cbFormat.SelectedIndex, cbComPort.Text, Baudrate, Parity, DataBits, StopBits, 0);
            }

            int Result = Client.Connect();
            if (Result == MBConsts.mbNoError)
            {
                btnConnect.Enabled = false;
                btnDisconnect.Enabled = true;
                cbFunction.Enabled = true;
                btnExecute.Enabled = true;
                PC.Enabled = false;
            }

            lblError.Text = MB.ErrorText(Result);

        }

        private void btnDisconnect_Click(object sender, EventArgs e)
        {
            Client.Disconnect();
            PC.Enabled = true;
            btnConnect.Enabled = true;
            btnDisconnect.Enabled = false;
            cbFunction.Enabled = false;
            btnExecute.Enabled = false;
        }

        private void btnExecutaAll_Click(object sender, EventArgs e)
        {
            Log.Clear();
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
            WriteLine("+-----------------------------------------------------------------------------+");
            WriteLine(" Test executed   : 19");
            WriteLine(" Passed          : {0:d}", TestPassed);
            WriteLine(" Failed          : {0:d}", 19 - TestPassed);
            WriteLine(" Total time (ms) : {0:d}", TotalTime);
            WriteLine("+-----------------------------------------------------------------------------+");
        }

        private void cbAttempts_SelectedIndexChanged(object sender, EventArgs e)
        {
            Client.SetLocalParam(DeviceID, MBConsts.par_MaxRetries, cbAttempts.SelectedIndex + 1);
        }

        private void btnClear_Click(object sender, EventArgs e)
        {
            Log.Clear();
        }

        byte DeviceID
        {
            get
            {
                return (byte)(cbDeviceID.SelectedIndex + 1);
            }
        }

        ushort RDAmount
        {
            get
            {
                return (ushort)nRDAmount.Value;
            }
        }

        ushort WRAmount
        {
            get
            {
                return (ushort)nWRAmount.Value;
            }
        }

        ushort RDStart
        {
            get
            {
                return (ushort)nRDStart.Value;
            }
        }

        ushort WRStart
        {
            get
            {
                return (ushort)nWRStart.Value;
            }
        }
    }
}