namespace WinFormsClient
{
    partial class Form1
    {
        /// <summary>
        ///  Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        ///  Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        ///  Required method for Designer support - do not modify
        ///  the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.panel1 = new System.Windows.Forms.Panel();
            this.btnClear = new System.Windows.Forms.Button();
            this.btnExecutaAll = new System.Windows.Forms.Button();
            this.label15 = new System.Windows.Forms.Label();
            this.btnExecute = new System.Windows.Forms.Button();
            this.cbFunction = new System.Windows.Forms.ComboBox();
            this.label9 = new System.Windows.Forms.Label();
            this.cbAttempts = new System.Windows.Forms.ComboBox();
            this.label10 = new System.Windows.Forms.Label();
            this.cbDeviceID = new System.Windows.Forms.ComboBox();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.nWRAmount = new System.Windows.Forms.NumericUpDown();
            this.nWRStart = new System.Windows.Forms.NumericUpDown();
            this.nRDAmount = new System.Windows.Forms.NumericUpDown();
            this.nRDStart = new System.Windows.Forms.NumericUpDown();
            this.label14 = new System.Windows.Forms.Label();
            this.label13 = new System.Windows.Forms.Label();
            this.label12 = new System.Windows.Forms.Label();
            this.label11 = new System.Windows.Forms.Label();
            this.btnDisconnect = new System.Windows.Forms.Button();
            this.btnConnect = new System.Windows.Forms.Button();
            this.PC = new System.Windows.Forms.TabControl();
            this.tabPage1 = new System.Windows.Forms.TabPage();
            this.nPort = new System.Windows.Forms.NumericUpDown();
            this.label3 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.edAddress = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.cbTransport = new System.Windows.Forms.ComboBox();
            this.tabPage2 = new System.Windows.Forms.TabPage();
            this.label16 = new System.Windows.Forms.Label();
            this.cbFormat = new System.Windows.Forms.ComboBox();
            this.label8 = new System.Windows.Forms.Label();
            this.label7 = new System.Windows.Forms.Label();
            this.label6 = new System.Windows.Forms.Label();
            this.cbBaudrate = new System.Windows.Forms.ComboBox();
            this.cbStopbits = new System.Windows.Forms.ComboBox();
            this.cbDatabits = new System.Windows.Forms.ComboBox();
            this.cbParity = new System.Windows.Forms.ComboBox();
            this.label5 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.cbComPort = new System.Windows.Forms.ComboBox();
            this.statusStrip1 = new System.Windows.Forms.StatusStrip();
            this.lblError = new System.Windows.Forms.ToolStripStatusLabel();
            this.Log = new System.Windows.Forms.TextBox();
            this.panel1.SuspendLayout();
            this.groupBox2.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.nWRAmount)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.nWRStart)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.nRDAmount)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.nRDStart)).BeginInit();
            this.PC.SuspendLayout();
            this.tabPage1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.nPort)).BeginInit();
            this.tabPage2.SuspendLayout();
            this.statusStrip1.SuspendLayout();
            this.SuspendLayout();
            // 
            // panel1
            // 
            this.panel1.Controls.Add(this.btnClear);
            this.panel1.Controls.Add(this.btnExecutaAll);
            this.panel1.Controls.Add(this.label15);
            this.panel1.Controls.Add(this.btnExecute);
            this.panel1.Controls.Add(this.cbFunction);
            this.panel1.Controls.Add(this.label9);
            this.panel1.Controls.Add(this.cbAttempts);
            this.panel1.Controls.Add(this.label10);
            this.panel1.Controls.Add(this.cbDeviceID);
            this.panel1.Controls.Add(this.groupBox2);
            this.panel1.Controls.Add(this.btnDisconnect);
            this.panel1.Controls.Add(this.btnConnect);
            this.panel1.Controls.Add(this.PC);
            this.panel1.Dock = System.Windows.Forms.DockStyle.Top;
            this.panel1.Location = new System.Drawing.Point(0, 0);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(723, 250);
            this.panel1.TabIndex = 1;
            // 
            // btnClear
            // 
            this.btnClear.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.btnClear.Location = new System.Drawing.Point(626, 220);
            this.btnClear.Name = "btnClear";
            this.btnClear.Size = new System.Drawing.Size(75, 23);
            this.btnClear.TabIndex = 15;
            this.btnClear.Text = "Clear";
            this.btnClear.UseVisualStyleBackColor = true;
            this.btnClear.Click += new System.EventHandler(this.btnClear_Click);
            // 
            // btnExecutaAll
            // 
            this.btnExecutaAll.Location = new System.Drawing.Point(419, 203);
            this.btnExecutaAll.Name = "btnExecutaAll";
            this.btnExecutaAll.Size = new System.Drawing.Size(114, 34);
            this.btnExecutaAll.TabIndex = 14;
            this.btnExecutaAll.Text = "Execute All";
            this.btnExecutaAll.UseVisualStyleBackColor = true;
            this.btnExecutaAll.Click += new System.EventHandler(this.btnExecutaAll_Click);
            // 
            // label15
            // 
            this.label15.AutoSize = true;
            this.label15.Location = new System.Drawing.Point(295, 157);
            this.label15.Name = "label15";
            this.label15.Size = new System.Drawing.Size(59, 15);
            this.label15.TabIndex = 13;
            this.label15.Text = "Functions";
            // 
            // btnExecute
            // 
            this.btnExecute.Location = new System.Drawing.Point(295, 203);
            this.btnExecute.Name = "btnExecute";
            this.btnExecute.Size = new System.Drawing.Size(114, 34);
            this.btnExecute.TabIndex = 12;
            this.btnExecute.Text = "Execute Selected";
            this.btnExecute.UseVisualStyleBackColor = true;
            this.btnExecute.Click += new System.EventHandler(this.btnExecute_Click);
            // 
            // cbFunction
            // 
            this.cbFunction.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbFunction.FormattingEnabled = true;
            this.cbFunction.Items.AddRange(new object[] {
            "(0x01) Read Coils",
            "(0x02) Read Discrete Inputs",
            "(0x03) Read Holding Registers",
            "(0x04) Read Input Registers",
            "(0x05) Write Single Coil",
            "(0x06) Write Single Register",
            "(0x07) Read Exception Status",
            "(0x08) Diagnostics",
            "(0x0B) Get Comm Event Counter",
            "(0x0C) Get Comm Event Log",
            "(0x0F) Write Multiple Coils",
            "(0x10) Write Multiple Registers",
            "(0x11) Report Server ID",
            "(0x14) Read File Record",
            "(0x15) Write File Record",
            "(0x16) Mask Write Register",
            "(0x17) Read/Write Multi Registers",
            "(0x18) Read FIFO Queue",
            "(0x2B) Encapsulated Intface Transport"});
            this.cbFunction.Location = new System.Drawing.Point(295, 174);
            this.cbFunction.Name = "cbFunction";
            this.cbFunction.Size = new System.Drawing.Size(238, 23);
            this.cbFunction.TabIndex = 11;
            // 
            // label9
            // 
            this.label9.AutoSize = true;
            this.label9.Location = new System.Drawing.Point(607, 24);
            this.label9.Name = "label9";
            this.label9.Size = new System.Drawing.Size(56, 15);
            this.label9.TabIndex = 10;
            this.label9.Text = "Attempts";
            // 
            // cbAttempts
            // 
            this.cbAttempts.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbAttempts.FormattingEnabled = true;
            this.cbAttempts.Location = new System.Drawing.Point(607, 42);
            this.cbAttempts.Name = "cbAttempts";
            this.cbAttempts.Size = new System.Drawing.Size(100, 23);
            this.cbAttempts.TabIndex = 9;
            this.cbAttempts.SelectedIndexChanged += new System.EventHandler(this.cbAttempts_SelectedIndexChanged);
            // 
            // label10
            // 
            this.label10.AutoSize = true;
            this.label10.Location = new System.Drawing.Point(525, 24);
            this.label10.Name = "label10";
            this.label10.Size = new System.Drawing.Size(56, 15);
            this.label10.TabIndex = 8;
            this.label10.Text = "Device ID";
            // 
            // cbDeviceID
            // 
            this.cbDeviceID.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbDeviceID.FormattingEnabled = true;
            this.cbDeviceID.Location = new System.Drawing.Point(525, 42);
            this.cbDeviceID.Name = "cbDeviceID";
            this.cbDeviceID.Size = new System.Drawing.Size(78, 23);
            this.cbDeviceID.TabIndex = 7;
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add(this.nWRAmount);
            this.groupBox2.Controls.Add(this.nWRStart);
            this.groupBox2.Controls.Add(this.nRDAmount);
            this.groupBox2.Controls.Add(this.nRDStart);
            this.groupBox2.Controls.Add(this.label14);
            this.groupBox2.Controls.Add(this.label13);
            this.groupBox2.Controls.Add(this.label12);
            this.groupBox2.Controls.Add(this.label11);
            this.groupBox2.Location = new System.Drawing.Point(289, 72);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(426, 79);
            this.groupBox2.TabIndex = 6;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "Read/Write";
            // 
            // nWRAmount
            // 
            this.nWRAmount.Location = new System.Drawing.Point(318, 46);
            this.nWRAmount.Maximum = new decimal(new int[] {
            65535,
            0,
            0,
            0});
            this.nWRAmount.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
            this.nWRAmount.Name = "nWRAmount";
            this.nWRAmount.Size = new System.Drawing.Size(98, 23);
            this.nWRAmount.TabIndex = 11;
            this.nWRAmount.Value = new decimal(new int[] {
            8,
            0,
            0,
            0});
            // 
            // nWRStart
            // 
            this.nWRStart.Location = new System.Drawing.Point(214, 46);
            this.nWRStart.Maximum = new decimal(new int[] {
            65535,
            0,
            0,
            0});
            this.nWRStart.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
            this.nWRStart.Name = "nWRStart";
            this.nWRStart.Size = new System.Drawing.Size(98, 23);
            this.nWRStart.TabIndex = 10;
            this.nWRStart.Value = new decimal(new int[] {
            1,
            0,
            0,
            0});
            // 
            // nRDAmount
            // 
            this.nRDAmount.Location = new System.Drawing.Point(110, 45);
            this.nRDAmount.Maximum = new decimal(new int[] {
            65535,
            0,
            0,
            0});
            this.nRDAmount.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
            this.nRDAmount.Name = "nRDAmount";
            this.nRDAmount.Size = new System.Drawing.Size(98, 23);
            this.nRDAmount.TabIndex = 9;
            this.nRDAmount.Value = new decimal(new int[] {
            8,
            0,
            0,
            0});
            // 
            // nRDStart
            // 
            this.nRDStart.Location = new System.Drawing.Point(6, 45);
            this.nRDStart.Maximum = new decimal(new int[] {
            65535,
            0,
            0,
            0});
            this.nRDStart.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
            this.nRDStart.Name = "nRDStart";
            this.nRDStart.Size = new System.Drawing.Size(98, 23);
            this.nRDStart.TabIndex = 8;
            this.nRDStart.Value = new decimal(new int[] {
            1,
            0,
            0,
            0});
            // 
            // label14
            // 
            this.label14.AutoSize = true;
            this.label14.Location = new System.Drawing.Point(318, 27);
            this.label14.Name = "label14";
            this.label14.Size = new System.Drawing.Size(82, 15);
            this.label14.TabIndex = 7;
            this.label14.Text = "Write Amount";
            // 
            // label13
            // 
            this.label13.AutoSize = true;
            this.label13.Location = new System.Drawing.Point(214, 27);
            this.label13.Name = "label13";
            this.label13.Size = new System.Drawing.Size(62, 15);
            this.label13.TabIndex = 6;
            this.label13.Text = "Write Start";
            // 
            // label12
            // 
            this.label12.AutoSize = true;
            this.label12.Location = new System.Drawing.Point(110, 27);
            this.label12.Name = "label12";
            this.label12.Size = new System.Drawing.Size(80, 15);
            this.label12.TabIndex = 3;
            this.label12.Text = "Read Amount";
            // 
            // label11
            // 
            this.label11.AutoSize = true;
            this.label11.Location = new System.Drawing.Point(6, 27);
            this.label11.Name = "label11";
            this.label11.Size = new System.Drawing.Size(60, 15);
            this.label11.TabIndex = 1;
            this.label11.Text = "Read Start";
            // 
            // btnDisconnect
            // 
            this.btnDisconnect.Location = new System.Drawing.Point(407, 30);
            this.btnDisconnect.Name = "btnDisconnect";
            this.btnDisconnect.Size = new System.Drawing.Size(112, 36);
            this.btnDisconnect.TabIndex = 5;
            this.btnDisconnect.Text = "Disconnect";
            this.btnDisconnect.UseVisualStyleBackColor = true;
            this.btnDisconnect.Click += new System.EventHandler(this.btnDisconnect_Click);
            // 
            // btnConnect
            // 
            this.btnConnect.Location = new System.Drawing.Point(289, 30);
            this.btnConnect.Name = "btnConnect";
            this.btnConnect.Size = new System.Drawing.Size(112, 36);
            this.btnConnect.TabIndex = 4;
            this.btnConnect.Text = "Connect";
            this.btnConnect.UseVisualStyleBackColor = true;
            this.btnConnect.Click += new System.EventHandler(this.btnConnect_Click);
            // 
            // PC
            // 
            this.PC.Controls.Add(this.tabPage1);
            this.PC.Controls.Add(this.tabPage2);
            this.PC.Location = new System.Drawing.Point(3, 3);
            this.PC.Name = "PC";
            this.PC.SelectedIndex = 0;
            this.PC.Size = new System.Drawing.Size(280, 244);
            this.PC.TabIndex = 0;
            // 
            // tabPage1
            // 
            this.tabPage1.Controls.Add(this.nPort);
            this.tabPage1.Controls.Add(this.label3);
            this.tabPage1.Controls.Add(this.label2);
            this.tabPage1.Controls.Add(this.edAddress);
            this.tabPage1.Controls.Add(this.label1);
            this.tabPage1.Controls.Add(this.cbTransport);
            this.tabPage1.Location = new System.Drawing.Point(4, 24);
            this.tabPage1.Name = "tabPage1";
            this.tabPage1.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage1.Size = new System.Drawing.Size(272, 216);
            this.tabPage1.TabIndex = 0;
            this.tabPage1.Text = "Ethernet";
            this.tabPage1.UseVisualStyleBackColor = true;
            // 
            // nPort
            // 
            this.nPort.Location = new System.Drawing.Point(136, 80);
            this.nPort.Maximum = new decimal(new int[] {
            65534,
            0,
            0,
            0});
            this.nPort.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
            this.nPort.Name = "nPort";
            this.nPort.Size = new System.Drawing.Size(100, 23);
            this.nPort.TabIndex = 15;
            this.nPort.Value = new decimal(new int[] {
            502,
            0,
            0,
            0});
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(136, 62);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(29, 15);
            this.label3.TabIndex = 5;
            this.label3.Text = "Port";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(136, 14);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(49, 15);
            this.label2.TabIndex = 3;
            this.label2.Text = "Address";
            // 
            // edAddress
            // 
            this.edAddress.Location = new System.Drawing.Point(136, 32);
            this.edAddress.Name = "edAddress";
            this.edAddress.Size = new System.Drawing.Size(100, 23);
            this.edAddress.TabIndex = 2;
            this.edAddress.Text = "127.0.0.1";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(21, 14);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(56, 15);
            this.label1.TabIndex = 1;
            this.label1.Text = "Transport";
            // 
            // cbTransport
            // 
            this.cbTransport.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbTransport.FormattingEnabled = true;
            this.cbTransport.Items.AddRange(new object[] {
            "TCP",
            "UDP",
            "RTU Over TCP",
            "RTU Over UDP"});
            this.cbTransport.Location = new System.Drawing.Point(21, 32);
            this.cbTransport.Name = "cbTransport";
            this.cbTransport.Size = new System.Drawing.Size(100, 23);
            this.cbTransport.TabIndex = 0;
            // 
            // tabPage2
            // 
            this.tabPage2.Controls.Add(this.label16);
            this.tabPage2.Controls.Add(this.cbFormat);
            this.tabPage2.Controls.Add(this.label8);
            this.tabPage2.Controls.Add(this.label7);
            this.tabPage2.Controls.Add(this.label6);
            this.tabPage2.Controls.Add(this.cbBaudrate);
            this.tabPage2.Controls.Add(this.cbStopbits);
            this.tabPage2.Controls.Add(this.cbDatabits);
            this.tabPage2.Controls.Add(this.cbParity);
            this.tabPage2.Controls.Add(this.label5);
            this.tabPage2.Controls.Add(this.label4);
            this.tabPage2.Controls.Add(this.cbComPort);
            this.tabPage2.Location = new System.Drawing.Point(4, 24);
            this.tabPage2.Name = "tabPage2";
            this.tabPage2.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage2.Size = new System.Drawing.Size(272, 216);
            this.tabPage2.TabIndex = 1;
            this.tabPage2.Text = "Serial";
            this.tabPage2.UseVisualStyleBackColor = true;
            // 
            // label16
            // 
            this.label16.AutoSize = true;
            this.label16.Location = new System.Drawing.Point(21, 62);
            this.label16.Name = "label16";
            this.label16.Size = new System.Drawing.Size(45, 15);
            this.label16.TabIndex = 15;
            this.label16.Text = "Format";
            // 
            // cbFormat
            // 
            this.cbFormat.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbFormat.FormattingEnabled = true;
            this.cbFormat.Items.AddRange(new object[] {
            "RTU",
            "ASCII"});
            this.cbFormat.Location = new System.Drawing.Point(21, 80);
            this.cbFormat.Name = "cbFormat";
            this.cbFormat.Size = new System.Drawing.Size(100, 23);
            this.cbFormat.TabIndex = 14;
            // 
            // label8
            // 
            this.label8.AutoSize = true;
            this.label8.Location = new System.Drawing.Point(136, 158);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(53, 15);
            this.label8.TabIndex = 13;
            this.label8.Text = "Stop Bits";
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Location = new System.Drawing.Point(136, 110);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(53, 15);
            this.label7.TabIndex = 12;
            this.label7.Text = "Data Bits";
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(136, 62);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(37, 15);
            this.label6.TabIndex = 11;
            this.label6.Text = "Parity";
            // 
            // cbBaudrate
            // 
            this.cbBaudrate.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbBaudrate.FormattingEnabled = true;
            this.cbBaudrate.Items.AddRange(new object[] {
            "4800",
            "9600",
            "19200",
            "38400",
            "57600",
            "115200",
            "128000",
            "256000",
            "500000",
            "921600"});
            this.cbBaudrate.Location = new System.Drawing.Point(136, 32);
            this.cbBaudrate.Name = "cbBaudrate";
            this.cbBaudrate.Size = new System.Drawing.Size(100, 23);
            this.cbBaudrate.TabIndex = 10;
            // 
            // cbStopbits
            // 
            this.cbStopbits.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbStopbits.FormattingEnabled = true;
            this.cbStopbits.Items.AddRange(new object[] {
            "1",
            "2"});
            this.cbStopbits.Location = new System.Drawing.Point(136, 176);
            this.cbStopbits.Name = "cbStopbits";
            this.cbStopbits.Size = new System.Drawing.Size(100, 23);
            this.cbStopbits.TabIndex = 9;
            // 
            // cbDatabits
            // 
            this.cbDatabits.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbDatabits.FormattingEnabled = true;
            this.cbDatabits.Items.AddRange(new object[] {
            "5",
            "6",
            "7",
            "8"});
            this.cbDatabits.Location = new System.Drawing.Point(136, 128);
            this.cbDatabits.Name = "cbDatabits";
            this.cbDatabits.Size = new System.Drawing.Size(100, 23);
            this.cbDatabits.TabIndex = 8;
            // 
            // cbParity
            // 
            this.cbParity.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbParity.FormattingEnabled = true;
            this.cbParity.Items.AddRange(new object[] {
            "E",
            "O",
            "N"});
            this.cbParity.Location = new System.Drawing.Point(136, 80);
            this.cbParity.Name = "cbParity";
            this.cbParity.Size = new System.Drawing.Size(100, 23);
            this.cbParity.TabIndex = 7;
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(136, 14);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(54, 15);
            this.label5.TabIndex = 6;
            this.label5.Text = "Baudrate";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(21, 14);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(58, 15);
            this.label4.TabIndex = 3;
            this.label4.Text = "Com Port";
            // 
            // cbComPort
            // 
            this.cbComPort.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbComPort.FormattingEnabled = true;
            this.cbComPort.Location = new System.Drawing.Point(21, 32);
            this.cbComPort.Name = "cbComPort";
            this.cbComPort.Size = new System.Drawing.Size(100, 23);
            this.cbComPort.TabIndex = 2;
            // 
            // statusStrip1
            // 
            this.statusStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.lblError});
            this.statusStrip1.Location = new System.Drawing.Point(0, 507);
            this.statusStrip1.Name = "statusStrip1";
            this.statusStrip1.Size = new System.Drawing.Size(723, 22);
            this.statusStrip1.TabIndex = 1;
            this.statusStrip1.Text = "statusStrip1";
            // 
            // lblError
            // 
            this.lblError.Name = "lblError";
            this.lblError.Size = new System.Drawing.Size(22, 17);
            this.lblError.Text = "Ok";
            this.lblError.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // Log
            // 
            this.Log.BackColor = System.Drawing.Color.Black;
            this.Log.Dock = System.Windows.Forms.DockStyle.Fill;
            this.Log.Font = new System.Drawing.Font("Consolas", 11.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point);
            this.Log.ForeColor = System.Drawing.Color.Lime;
            this.Log.Location = new System.Drawing.Point(0, 250);
            this.Log.Multiline = true;
            this.Log.Name = "Log";
            this.Log.ReadOnly = true;
            this.Log.ScrollBars = System.Windows.Forms.ScrollBars.Both;
            this.Log.Size = new System.Drawing.Size(723, 257);
            this.Log.TabIndex = 15;
            this.Log.WordWrap = false;
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(7F, 15F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(723, 529);
            this.Controls.Add(this.Log);
            this.Controls.Add(this.statusStrip1);
            this.Controls.Add(this.panel1);
            this.MinimumSize = new System.Drawing.Size(739, 400);
            this.Name = "Form1";
            this.Text = "C# (Minimal) Client Example";
            this.panel1.ResumeLayout(false);
            this.panel1.PerformLayout();
            this.groupBox2.ResumeLayout(false);
            this.groupBox2.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.nWRAmount)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.nWRStart)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.nRDAmount)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.nRDStart)).EndInit();
            this.PC.ResumeLayout(false);
            this.tabPage1.ResumeLayout(false);
            this.tabPage1.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.nPort)).EndInit();
            this.tabPage2.ResumeLayout(false);
            this.tabPage2.PerformLayout();
            this.statusStrip1.ResumeLayout(false);
            this.statusStrip1.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion
        private Panel panel1;
        private TabControl PC;
        private TabPage tabPage1;
        private Label label1;
        private ComboBox cbTransport;
        private TabPage tabPage2;
        private StatusStrip statusStrip1;
        private ToolStripStatusLabel lblError;
        private Label label3;
        private Label label2;
        private TextBox edAddress;
        private Label label4;
        private ComboBox cbComPort;
        private Label label8;
        private Label label7;
        private Label label6;
        private ComboBox cbBaudrate;
        private ComboBox cbStopbits;
        private ComboBox cbDatabits;
        private ComboBox cbParity;
        private Label label5;
        private GroupBox groupBox2;
        private Label label14;
        private Label label13;
        private Label label12;
        private Label label11;
        private Button btnDisconnect;
        private Button btnConnect;
        private Label label15;
        private Button btnExecute;
        private ComboBox cbFunction;
        private Label label9;
        private ComboBox cbAttempts;
        private Label label10;
        private ComboBox cbDeviceID;
        private NumericUpDown nPort;
        private NumericUpDown nWRAmount;
        private NumericUpDown nWRStart;
        private NumericUpDown nRDAmount;
        private NumericUpDown nRDStart;
        private TextBox Log;
        private Button btnExecutaAll;
        private Button btnClear;
        private ComboBox cbFormat;
        private Label label16;
    }
}