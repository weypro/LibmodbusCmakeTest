using System.Reflection.Metadata;
using System.Runtime.InteropServices;
using System.Text;
using SnapModbus;

namespace WinFormsDevice
{
    public partial class MainForm : Form
    {
        static readonly int regs_amount = 32768;
        static readonly int bits_amount = 32768;

        // We Just create an "empty" Device which doesn't require params,
        // since we still don't know the type of Broker that we will want
        private SnapMBDevice Device = new SnapMBDevice();

        // Resources
        static byte[] Coils = new byte[bits_amount];
        static byte[] DiscreteInputs = new byte[bits_amount];
        static ushort[] HoldingRegisters = new ushort[regs_amount];
        static ushort[] InputRegisters = new ushort[regs_amount];

        // Callbacks
        private TDeviceEvent DeviceEvent = new TDeviceEvent(OnEvent);
        private readonly TPacketLog PacketLog = new TPacketLog(OnPacketLog);

        private static readonly object PacketLock = new object();

        GCHandle ThisHandle;
        //------------------------------------------------------------------------------
        // Callbacks
        // Remember, in a Windows Form application you can access *ONLY* static stuffs
        //------------------------------------------------------------------------------
        public static void OnEvent(IntPtr usrPtr, ref MBConsts.DeviceEvent Event, int Size)
        {
        }

        static void OnPacketLog(IntPtr UsrPtr, UInt32 Peer, int Direction, IntPtr Data, int Size)
        {
        }


        public MainForm()
        {
            InitializeComponent();
            ThisHandle = GCHandle.Alloc(this, GCHandleType.Normal);

            cbTransport.SelectedIndex = 0;
            cbFormat.SelectedIndex = 0;

            for (int i = 1; i < 65; i++)
                cbComPort.Items.Add("COM" + i.ToString());

            cbComPort.SelectedIndex = 2;
            cbBaudrate.SelectedIndex = 2;
            cbBaudrate.SelectedIndex = 2;
            cbParity.SelectedIndex = 0;
            cbDatabits.SelectedIndex = 3;
            cbStopbits.SelectedIndex = 0;

            for (int i = 1; i < 256; i++)
                cbDeviceID.Items.Add(i.ToString());
            cbDeviceID.SelectedIndex = 0;

            btnStart.Enabled = true;
            btnStop.Enabled = false;
            cbDeviceID.Enabled = true;
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
                        line[charColumn] = (b < 32 ? '·' : (char)b);
                    }
                    hexColumn += 3;
                    charColumn++;
                }
                string s = new string(line);
                Write(s);
            }
        }
        #endregion

        private void DeviceSetup()
        {
            // Share the Resources
            Device.RegisterArea(MBConsts.mbaHoldingRegisters, ref HoldingRegisters, regs_amount);
            Device.RegisterArea(MBConsts.mbaInputRegisters, ref InputRegisters, regs_amount);
            Device.RegisterArea(MBConsts.mbaDiscreteInputs, ref DiscreteInputs, bits_amount);
            Device.RegisterArea(MBConsts.mbaCoils, ref Coils, bits_amount);

            // Set Callback to receive Data Dump
            Device.RegisterCallback(MBConsts.cbkPacketLog, Marshal.GetFunctionPointerForDelegate(PacketLog), IntPtr.Zero);
            // Say to the device that we want both packet
            Device.SetParam(MBConsts.par_PacketLog, MBConsts.PacketLog_BOTH);
            // Set Log Callback
            Device.RegisterCallback(MBConsts.cbkDeviceEvent, Marshal.GetFunctionPointerForDelegate(DeviceEvent), IntPtr.Zero);
        }

        private void btnStop_Click(object sender, EventArgs e)
        {
            Device.Stop();
            PC.Enabled = true;
            btnStart.Enabled = true;
            cbDeviceID.Enabled = true;
            btnStop.Enabled = false;
        }

        private void btnStart_Click(object sender, EventArgs e)
        {
            if (PC.SelectedIndex == 0)
            {
                // We want an Ethernet Device
                Device.ChangeTo(cbTransport.SelectedIndex, cbDeviceID.SelectedIndex + 1, edAddress.Text, (int)nPort.Value);
            }
            else
            {
                // We want a Serial Device
                char[] Parities = { 'E', 'O', 'N' };
                int[] Baudrates = { 4800, 9600, 19200, 38400, 57600, 115200, 128000, 256000, 500000, 921600 };
                int[] Datas = { 5, 6, 7, 8 };
                int[] Stops = { 1, 2 };

                char Parity = Parities[cbParity.SelectedIndex];
                int Baudrate = Baudrates[cbBaudrate.SelectedIndex];
                int DataBits = Datas[cbDatabits.SelectedIndex];
                int StopBits = Stops[cbStopbits.SelectedIndex];
                              
                Device.ChangeTo(cbFormat.SelectedIndex, cbDeviceID.SelectedIndex + 1, cbComPort.Text, Baudrate, Parity, DataBits, StopBits, 0);
            }

            DeviceSetup();
            int Result = Device.Start();
            if (Result == 0)
            {
                btnStart.Enabled = false;
                btnStop.Enabled = true;
                cbDeviceID.Enabled = false;
                PC.Enabled = false;
            }
            lblError.Text=MB.ErrorText(Result);

        }

        private void timLog_Tick(object sender, EventArgs e)
        {
            String LogMessage = "";           
            if (Device.Exists && Device.PickEventAsText(ref LogMessage))
                WriteLine(LogMessage);
        }
    }
}