unit SnapMBCommon;

{$MODE DELPHI}

interface

uses
  Classes, SysUtils, Graphics, Grids, SnapMB;

const
  color_Connected      = $004CC16C;
  color_Disconnected   = $004F53D9;

  color_DataLinkActive = $00F8AE26;
  color_DataLinkOff    = clGray;

  color_ChannelActive  = $002F8AFF;
  color_ChannelOff     = clGray;

const
  bits_amount = 65536;
  regs_amount = 32768;
  IoDataSize  = 520;

  Brokers : array[TMBBrokerType] of string = ('NET Client', 'SER Controller');
  SerialFormats : array[TMBSerialFormat] of string = ('RTU', 'ASCII');
  NetProtocols : array[TMBNetProto] of string = ('TCP', 'UDP', 'RTU-TCP', 'RTU-UDP');

type
  TRegistersArea = packed array[0..regs_amount-1] of word;
  TBitArea       = packed array[0..bits_amount-1] of boolean;

  TIOBuffer = packed record // 528 byte to contail ASCII packet and to be multiple of 8
    Direction : word;
    Size      : word;
    Peer      : longword;
    Data      : packed array [0..IoDataSize-1] of byte;
  end;

//******************************************************************************
// Note
// Resources shoud be declared inside the classes
// Here we like to switch between Ethernet and Serial, so, to avoid
// to loose their content, they are declared outside the class.
//******************************************************************************
var
  Coils            : TBitArea;
  DiscreteInputs   : TBitArea;
  HoldingRegisters : TRegistersArea;
  InputRegisters   : TRegistersArea;

procedure InitData;
procedure FillRegGrid(Grid: TStringGrid; Regs: TRegistersArea; IdxStart, IdxEnd : integer); overload;
procedure FillRegGrid(Grid: TStringGrid; var Regs: TRegistersArea; FillRandom: boolean); overload;
procedure FillBitGrid(Grid: TStringGrid; var Bits: TBitArea; FillRandom: boolean);
procedure InvalidateRegGrid(Grid: TStringGrid; Regs: TRegistersArea);
function IsValidInt16(S: string; Hex : boolean; var Value : word): boolean;
function IPAddress(Address: Longword): string;


implementation

procedure InitData;
begin
  fillchar(Coils, SizeOf(Coils), #0);
  fillchar(DiscreteInputs, SizeOf(DiscreteInputs), #0);
  fillchar(InputRegisters, SizeOf(InputRegisters), #0);
  fillchar(HoldingRegisters, SizeOf(HoldingRegisters), #0);
end;

procedure FillRegGrid(Grid: TStringGrid; Regs: TRegistersArea; IdxStart, IdxEnd : integer);
var
  c, Col, Row : integer;
  GridWidth : integer;
begin
  GridWidth:=Grid.ColCount - 1;
  Grid.BeginUpdate;
  try
    for c:=IdxStart to IdxEnd do
    begin
      Row:=(c div GridWidth) + 1;
      Col:=(c mod GridWidth) + 1;
      if Grid.Tag=0 then
        Grid.Cells[Col,Row]:=IntToStr(Regs[c])
      else
        Grid.Cells[Col,Row]:=IntToHex(Regs[c],4);
    end;
  finally
    Grid.EndUpdate(true);
  end;

end;

procedure FillRegGrid(Grid: TStringGrid; var Regs: TRegistersArea; FillRandom: boolean);
Var
  X1, Y1, X2, Y2 : integer;
  X, Y : integer;
begin
  X1:=Grid.Selection.TopLeft.X-1;
  Y1:=Grid.Selection.TopLeft.Y-1;
  X2:=Grid.Selection.BottomRight.X-1;
  Y2:=Grid.Selection.BottomRight.Y-1;

  for Y:=Y1 to Y2 do
    for X:=X1 to X2 do
       if FillRandom then
         Regs[Y*16+X]:=random(65535)
       else
         Regs[Y*16+X]:=0;
  InvalidateRegGrid(Grid, Regs);
end;

procedure FillBitGrid(Grid: TStringGrid; var Bits: TBitArea;
  FillRandom: boolean);
Var
  X1, Y1, X2, Y2 : integer;
  X, Y : integer;
begin
  X1:=Grid.Selection.TopLeft.X-1;
  Y1:=Grid.Selection.TopLeft.Y-1;
  X2:=Grid.Selection.BottomRight.X-1;
  Y2:=Grid.Selection.BottomRight.Y-1;

  for Y:=Y1 to Y2 do
    for X:=X1 to X2 do
       if FillRandom then
         Bits[Y*32+X]:=boolean(random(4) div 2)
       else
         Bits[Y*32+X]:=false;
  grid.Invalidate;
end;

procedure InvalidateRegGrid(Grid: TStringGrid; Regs: TRegistersArea);
begin
  FillRegGrid(Grid, Regs, 0, regs_amount-1);
end;

function IsValidInt16(S: string; Hex : boolean; var Value : word): boolean;
var
  aValue : int64;
begin
  Result:=true;
  if Hex then
    S:='$'+S;
  // Check Number
  Try
    aValue:=StrToInt(S);
  except
    On E : EConvertError do
      Result:=false;
  end;
  // Check bounds
  if Result then
  begin
    if (aValue>=0) and (aValue<=$FFFF) then
      Value:=aValue
    else
      Result:=false;
  end;
end;

function IPAddress(Address: Longword): string;
Var
  X : packed array[0..3] of byte absolute Address;
begin
  Result:=format('%d.%d.%d.%d',[X[0],X[1],X[2],X[3]]);
end;

end.

