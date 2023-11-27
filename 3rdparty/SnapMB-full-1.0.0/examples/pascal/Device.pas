// Common part for Delphi and Lazarus

{$APPTYPE CONSOLE}
{$IFDEF FPC}
  {$MODE DELPHI}
{$ENDIF}

Uses
   SysUtils,  SnapMB;

Const
  regs_amount = 32768;
  bits_amount = 65536;
  DefaultMBPort = 502;

Type
  DeviceType = (
    null_Device,
    tcp_Device,
    udp_Device,
    rtu_overTCP_Device,
    rtu_overUDP_Device,
    rtu_Device,
    asc_Device
  );

  Option = (
    opt_log,
    opt_dump
  );

Var
  dt : DeviceType = null_Device;
  opt : Option = opt_log;

// NET Params with defaults
  Address : AnsiString = '127.0.0.1';  // -a
  Port : integer = DefaultMBPort;      // -p

// SER Params with defaults
  DeviceID : byte = 1;                 // -i
{$IFDEF MSWINDOWS}
  ComPort  : AnsiString = 'COM5';      // -c
{$ELSE}
  ComPort  : AnsiString = '/dev/ttyUSB0';
{$ENDIF}
  BaudRate : integer = 19200;          // -b
  Parity   : AnsiChar = 'E';           // -y
  DataBits : integer = 8;              // -d
  StopBits : integer = 1;              // -s

// Device Object
  Device : TSnapMBDevice;

// Shared Resurces
  HoldingRegisters : packed array[0..regs_amount-1] of word;
  InputRegisters   : packed array[0..regs_amount-1] of word;
  Coils            : packed array[0..bits_amount-1] of bytebool;
  DiscreteInputs   : packed array[0..bits_amount-1] of bytebool;

// Dump
procedure hexdump(mem : pointer; count : integer);
type
  TBuffer = packed array[0..$FFFF] of byte;
  PBuffer = ^TBuffer;
Var
  P : PBuffer;
  SHex, SChr : string;
  Ch : AnsiChar;
  c, cnt : integer;
begin
  P:=PBuffer(mem);
  SHex:='';SChr:='';cnt:=0;
  for c := 0 to Count - 1 do
  begin
    SHex:=SHex+IntToHex(P^[c],2)+' ';
    Ch:=AnsiChar(P^[c]);
    if not (Ch in ['a'..'z','A'..'Z','0'..'9','_','$','-',#32]) then
      Ch:='.';
    SChr:=SChr+String(Ch);
    inc(cnt);
    if cnt=16 then
    begin
      Writeln(SHex+'  '+SChr);
      SHex:='';SChr:='';
      cnt:=0;
    end;
  end;
  // Dump remainder
  if cnt>0 then
  begin
    while Length(SHex)<48 do
      SHex:=SHex+' ';
    Writeln(SHex+'  '+SChr);
  end;
end;

// Callbacks
procedure DeviceEvent(usrPtr : Pointer; var Event : TSrvEvent; Size : integer);
{$IFDEF MSWINDOWS}stdcall;{$ELSE}cdecl;{$ENDIF}
begin
  Writeln(EventText(Event));
end;

procedure PacketLog(usrPtr : Pointer; Peer : Longword; Direction : integer; Data : Pointer; Size : integer);
{$IFDEF MSWINDOWS}stdcall;{$ELSE}cdecl;{$ENDIF}
begin
  if (Direction = PacketLog_IN) then
    Writeln(Format('Indication received (%d byte)', [Size]))
  else
    Writeln(Format('Confirmation sent (%d byte)', [Size]));
  Writeln('------------------------------------------------------------------');
  HexDump(Data, Size);
  Writeln('------------------------------------------------------------------');
  if (Direction = PacketLog_OUT) then
    Writeln;
end;

function OnUserFunction(usrPtr : Pointer; UsrFunction : byte; RxPDU : Pointer;
  RxPDUSize : word; TxPDU : Pointer; var TxPDUSize : word) : integer;
{$IFDEF MSWINDOWS}stdcall;{$ELSE}cdecl;{$ENDIF}
begin
  if UsrFunction = $41 then
  begin
    // Do something
  end;
  if UsrFunction = $42 then
  begin
    // Do something else...
  end;
  Result := 0;
end;

procedure Legenda();
begin
    Writeln('');
    Writeln('Usage:');
    Writeln('Device <DeviceType> [<DeviceParams>] [<Options>]');
    Writeln('');
    Writeln('DeviceType (Mandatory, no case sensitive):');
    Writeln('  tcp             : Modbus/TCP Device');
    Writeln('  udp             : Modbus/UDP Device');
    Writeln('  RtuOverTcp      : RTU Telegram Over Modbus/TCP Device');
    Writeln('  RtuOverUdp      : RTU Telegram Over Modbus/UDP Device');
    Writeln('  rtu             : RTU (serial) Device');
    Writeln('  asc             : ASCII (serial) Device');
    Writeln('');
    Writeln('Ethernet Device Params (Optionals):');
    Writeln('  -a <Address>    : IP Address (default 127.0.0.1)');
    Writeln('  -p <Port>       : IP Port (default 502) (*)');
    Writeln('');
    Writeln('Serial Device Params (Optionals):');
    Writeln('  -i <DeviceID>   : Device ID, 1..255 (Default 1)');
    Writeln('  -c <ComPort>    : ComPort Name (Default COM5');
    Writeln('  -b <BaudRate>   : BaudRate (Default 19200)');
    Writeln('  -y <Parity>     : Parity, N or E or O (Default E)');
    Writeln('  -d <DataBits>   : DataBits, 7 or 8 (Default 8)');
    Writeln('  -s <StopBits>   : StopBits, 1 or 2 (Default 1)');
    Writeln('');
    Writeln('Options:');
    Writeln('  -g <LogOption>  : L (Activity Log), D (Telegram Dump) (Default L)');
    Writeln('');
    Writeln('Examples:');
    Writeln('  Device tcp -a 192.168.0.15 -p 5020');
    Writeln('  Device rtu -c COM7 -b 115200 -y N');
    Writeln('');
    Writeln('(*) to use a Low port number under Linux you must run the program as root');
    Writeln('    i.e.  sudo Device <DeviceType> [<DeviceParams>] [<Options>]');
    Writeln('');
end;


function Find(const ParamName : string; var Param : string) : boolean;
Var
  c : integer;
begin
  for c:=2 to ParamCount -1 do
  begin
    if SameText(ParamName, ParamStr(c)) then
    begin
      if c < ParamCount then
      begin
        Param := ParamStr(c + 1);
        exit(true);
      end
      else
        exit(false);
    end;
  end;
  Result := false;
end;

function GetDeviceType(DeviceName : string) : DeviceType;
begin
  if SameText(DeviceName, 'tcp') then
    exit(tcp_Device);
  if SameText(DeviceName, 'udp') then
    exit(udp_Device);
  if SameText(DeviceName, 'RtuOverTCP') then
    exit(rtu_overTCP_Device);
  if SameText(DeviceName, 'RtuOverUDP') then
    exit(rtu_overUDP_Device);
  if SameText(DeviceName, 'rtu') then
    exit(rtu_Device);
  if SameText(DeviceName, 'asc') then
    exit(asc_Device);

  Result:=null_Device;
end;

function ParseCmdLine() : boolean;
Var
  Param : string;
begin
  Param := '';
  if ParamCount<1 then
    exit(false);

// Get Device Type (Mandatory)

  dt:=GetDeviceType(ParamStr(1));
  if dt = null_device then
    exit(false);

// Get Params (Optionals)

  if (dt = rtu_Device) or (dt = asc_Device) then
  begin
    if Find('-i', Param) then
      DeviceID:=StrToIntDef(Param, DeviceID);
    if Find('-c', Param) then
      ComPort:=AnsiString(Param);
    if Find('-b', Param) then
      BaudRate:=StrToIntDef(Param, BaudRate);
    if Find('-y', Param) then
      Parity:=AnsiChar(Param[1]);
    if Find('-d', Param) then
      DataBits:=StrToIntDef(Param, DataBits);
    if Find('-s', Param) then
      StopBits:=StrToIntDef(Param, StopBits);
  end
  else begin
    if Find('-a', Param) then
      Address:=AnsiString(Param);
    if Find('-p', Param) then
      Port:=StrToIntDef(Param, Port);
  end;

  if Find('-g', Param) and (UpperCase(Param)='D') then
    opt := opt_dump;

  Result:=true;
end;


function CreateDevice(const dt : DeviceType) : TSnapMBDevice;
begin
  case dt of
    tcp_Device:
        Result:=TSnapMBDevice.Create(mbTCP, DeviceID, Address, Port);
    udp_Device:
        Result:=TSnapMBDevice.Create(mbUDP, DeviceID, Address, Port);
    rtu_overTCP_Device:
        Result:=TSnapMBDevice.Create(mbRTUOverTCP, DeviceID, Address, Port);
    rtu_overUDP_Device:
        Result:=TSnapMBDevice.Create(mbRTUOverUDP, DeviceID, Address, Port);
    rtu_Device:
      Result:=TSnapMBDevice.Create(sfRTU, DeviceID, ComPort, BaudRate, Parity, DataBits, StopBits, flowNone);
    asc_Device:
      Result:=TSnapMBDevice.Create(sfASCII, DeviceID, ComPort, BaudRate, Parity, DataBits, StopBits, flowNone);
  else
      Result:=TSnapMBDevice.Create(mbTCP, DeviceID, Address, Port);
  end;
end;

Var
  Result : integer;
begin
  if not ParseCmdLine() then
  begin
    Legenda();
    exit;
  end;

  Device:=CreateDevice(dt);

  // Share the resources
  Device.RegisterArea(mbAreaDiscreteInputs, @DiscreteInputs, bits_amount);
  Device.RegisterArea(mbAreaCoils, @Coils, bits_amount);
  Device.RegisterArea(mbAreaInputRegisters, @InputRegisters, regs_amount);
  Device.RegisterArea(mbAreaHoldingRegisters, @HoldingRegisters, regs_amount);

  // Set the Option
  if opt = opt_dump then
  begin
    Device.RegisterCallback(cbkPacketLog, @PacketLog, nil);
    Device.SetParam(par_PacketLog, PacketLog_BOTH);
  end
  else
    Device.RegisterCallback(cbkDeviceEvent, @DeviceEvent, nil);

  Device.SetUserFunction($41, true);
  Device.SetUserFunction($42, true);
  Device.RegisterCallback(cbkUsrFunction, @OnUserFunction, nil);

  // Let's Start

  Result:=Device.Start();
  if Result = 0 then
  begin
    Writeln('Press any key to terminate...');
    ReadLn;
  end
  else
    Writeln(ErrorText(Result));

  Device.Free;

end.
