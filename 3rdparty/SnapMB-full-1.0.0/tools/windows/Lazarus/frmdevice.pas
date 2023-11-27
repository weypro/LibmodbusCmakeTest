unit frmDevice;

{$mode objfpc}{$H+}

interface

uses
  Classes, SysUtils, Forms, Controls, Graphics, Dialogs, ExtCtrls, Buttons,
  StdCtrls, ComCtrls, Grids, SynEdit, SynHighlighterIni,
  SynHighlighterHTML, SynHighlighterAny,
  BCButton, BGRAImageList, BCPanel, BGRATheme,
  BCComboBox, SnapMB, Types, frmDevsettings, uDataQueue, SnapMBCommon;


type

  { TMainForm }

  TMainForm = class(TForm)
    btnZeroArea: TBCButton;
    btnIncreaseFill: TBCButton;
    gridDiscrete: TStringGrid;
    ImageList1: TImageList;
    ItemIndex: TBCPanel;
    cbFormat: TBCComboBox;
    btnClearLog: TBCButton;
    btnRandomFillArea: TBCButton;
    btnZeroAll: TBCButton;
    btnRandomFillAll: TBCButton;
    btnClearDump: TBCButton;
    BCPanel1: TBCPanel;
    BCPanel2: TBCPanel;
    gridHoldingRegs: TStringGrid;
    Label1: TLabel;
    Label2: TLabel;
    Label3: TLabel;
    Label4: TLabel;
    lblLinkNameLabel1: TLabel;
    lblLinkNameLabel2: TLabel;
    lblInterframeDetected: TLabel;
    lblInterframeSet: TLabel;
    Panel8: TPanel;
    Panel9: TPanel;
    pnlChannel: TBCPanel;
    btnProperties: TBCButton;
    btnConnection: TBCButton;
    pnlChannel1: TBCPanel;
    pnlChannel2: TBCPanel;
    pnlConnection: TBCPanel;
    pnlDataLink: TBCPanel;
    BGRAImageList1: TBGRAImageList;
    lblConnection: TLabel;
    lblDeviceIDLabel: TLabel;
    lblDataLink1: TLabel;
    lblDeviceID: TLabel;
    lblDataLink: TLabel;
    lblLinkName: TLabel;
    lblParams: TLabel;
    lblFlavourLabel: TLabel;
    lblFlavour: TLabel;
    lblLinkNameLabel: TLabel;
    lblParamsLabel: TLabel;
    PCGrids: TPageControl;
    PageControl2: TPageControl;
    Panel1: TPanel;
    Panel2: TPanel;
    Panel3: TPanel;
    Panel4: TPanel;
    Panel5: TPanel;
    SB: TStatusBar;
    Splitter1: TSplitter;
    Splitter2: TSplitter;
    gridCoils: TStringGrid;
    Log: TStringGrid;
    gridInputRegs: TStringGrid;
    Dump: TSynEdit;
    SynIniSyn1: TSynIniSyn;
    TabSheet1: TTabSheet;
    TabSheet2: TTabSheet;
    TabSheet3: TTabSheet;
    TabSheet4: TTabSheet;
    TabSheet5: TTabSheet;
    TabSheet6: TTabSheet;
    timLog: TTimer;
    Tree: TTreeView;
    procedure FillAreaClick(Sender: TObject);
    procedure btnIncreaseFillClick(Sender: TObject);
    procedure btnRandomFillAllClick(Sender: TObject);
    procedure btnClearDumpClick(Sender: TObject);
    procedure btnClearLogClick(Sender: TObject);
    procedure btnConnectionClick(Sender: TObject);
    procedure btnPropertiesClick(Sender: TObject);
    procedure btnZeroAllClick(Sender: TObject);
    procedure cbFormatChange(Sender: TObject);
    procedure FormClose(Sender: TObject; var CloseAction: TCloseAction);
    procedure FormCreate(Sender: TObject);
    procedure FormDestroy(Sender: TObject);
    procedure FormShow(Sender: TObject);
    procedure gridCoilsDblClick(Sender: TObject);
    procedure gridCoilsDrawCell(Sender: TObject; aCol, aRow: Integer;
      aRect: TRect; aState: TGridDrawState);
    procedure gridCoilsKeyDown(Sender: TObject; var Key: Word;
      Shift: TShiftState);
    procedure gridCoilsSelectCell(Sender: TObject; aCol, aRow: Integer;
      var CanSelect: Boolean);
    procedure gridDiscreteDblClick(Sender: TObject);
    procedure gridDiscreteDrawCell(Sender: TObject; aCol, aRow: Integer;
      aRect: TRect; aState: TGridDrawState);
    procedure gridDiscreteKeyDown(Sender: TObject; var Key: Word;
      Shift: TShiftState);
    procedure gridHoldingRegsSelectCell(Sender: TObject; aCol, aRow: Integer;
      var CanSelect: Boolean);
    procedure gridHoldingRegsValidateEntry(sender: TObject; aCol,
      aRow: Integer; const OldValue: string; var NewValue: String);
    procedure gridInputRegsSelectCell(Sender: TObject; aCol, aRow: Integer;
      var CanSelect: Boolean);
    procedure gridInputRegsValidateEntry(sender: TObject; aCol, aRow: Integer;
      const OldValue: string; var NewValue: String);
    procedure LogDrawCell(Sender: TObject; aCol, aRow: Integer;
      aRect: TRect; aState: TGridDrawState);
    procedure Panel2Click(Sender: TObject);
    procedure PCGridsChange(Sender: TObject);
    procedure timLogTimer(Sender: TObject);
  private
    FConnected : boolean;
    doClearLog : boolean;
    doClearDump : boolean;
    SelfDestroying : boolean;
    WasConnected : boolean;
    Settings: TMBDeviceSetting;
    Device : TSnapMBDevice;
    DataQueue : TDataQueue;
    OldIfSet : integer;
    OldIfDet : integer;
    FLastInfo : TDeviceInfo;
    procedure InitControls;
    procedure AddEvent(Event : TSrvEvent);
    procedure AddEventLine(S0, S : string);
    procedure SetFConnected(AValue: boolean);
    procedure UpdateSettingsPanels;
    procedure DeviceDestroy;
    procedure DeviceCreate;
    procedure DeviceChangeTo;
    procedure DeviceMap;
    procedure ClearTree;
    procedure DumpIOData(var IOBuffer : TIOBuffer);
    procedure PushIOData(Direction : integer; Peer : longword; Data : Pointer; Size : integer);
  public
    property Connected : boolean read FConnected write SetFConnected;
  end;

var
  MainForm: TMainForm;

implementation
{$R *.lfm}


//******************************************************************************
// Note
// Callbacks *MUST* be declared outside the Device Class because they are not
// method pointer but stdcall/cdecl functions
// To call a method inside a callback we will use "usrPtr" parameter casted to
// our class.
//******************************************************************************

procedure DeviceEvent(usrPtr : Pointer; pEvent : Pointer; Size : integer);
{$IFDEF MSWINDOWS}stdcall;{$ELSE}cdecl;{$ENDIF}
begin
  // Declared but not used, we will extract the events from the Device queue
  // using PickEvent method.
end;

procedure PacketLog(usrPtr : Pointer; Peer : Longword; Direction : integer; Data : Pointer; Size : integer);
{$IFDEF MSWINDOWS}stdcall;{$ELSE}cdecl;{$ENDIF}
begin
  TMainForm(usrPtr).PushIOData(Direction, Peer, Data, Size);
end;

function DiscreteInputsRequest(usrPtr : Pointer; Address : word; Amount : word; Data : Pointer): integer;
{$IFDEF MSWINDOWS}stdcall;{$ELSE}cdecl;{$ENDIF}
begin
  Result:=mbNoError;
end;

function CoilsRequest(usrPtr : Pointer; Action : integer; Address : word; Amount : word; Data : Pointer) : integer;
{$IFDEF MSWINDOWS}stdcall;{$ELSE}cdecl;{$ENDIF}
begin
  Result:=mbNoError;
end;

function InputRegistersRequest(usrPtr : Pointer; Address : word; Amount : word; Data : Pointer) : integer;
{$IFDEF MSWINDOWS}stdcall;{$ELSE}cdecl;{$ENDIF}
begin
  Result:=mbNoError;
end;

function HoldingRegistersRequest(usrPtr : Pointer; Action : integer; Address : word; Amount : word; Data : Pointer) : integer;
{$IFDEF MSWINDOWS}stdcall;{$ELSE}cdecl;{$ENDIF}
begin
  Result:=mbNoError;
end;

function ReadWriteMultipleRegistersRequest(usrPtr : Pointer; RDAddress : word; RDAmount : word; RDData : Pointer; WRAddress : word; WRAmount : word; WRData : Pointer) : integer;
{$IFDEF MSWINDOWS}stdcall;{$ELSE}cdecl;{$ENDIF}
begin
  Result:=mbNoError;
end;

function MaskRegisterRequest(usrPtr : Pointer; Address : word; AND_Mask : word; OR_Mask : word) : integer;
{$IFDEF MSWINDOWS}stdcall;{$ELSE}cdecl;{$ENDIF}
begin
  Result:=mbNoError;
end;

function FileRecordRequest(usrPtr : Pointer; Action : integer; RefType : word; FileNumber : word; RecNumber : word; RegsAmount : word; Data : Pointer) : integer;
{$IFDEF MSWINDOWS}stdcall;{$ELSE}cdecl;{$ENDIF}
begin
  if Action = cbActionRead then
    move(HoldingRegisters, Data^, RegsAmount * 2)
  else
    move(Data^, HoldingRegisters, RegsAmount * 2);

  Result:=mbNoError;
end;

function ExceptionStatusRequest(usrPtr : Pointer; var Status : byte) : integer;
{$IFDEF MSWINDOWS}stdcall;{$ELSE}cdecl;{$ENDIF}
begin
  Result:=mbNoError;
end;

function DiagnosticsRequest(usrPtr : Pointer; SubFunction : word; RxItems : Pointer; TxItems : Pointer; ItemsSent : word; var ItemsRecvd : word) : integer;
{$IFDEF MSWINDOWS}stdcall;{$ELSE}cdecl;{$ENDIF}
begin
  Result:=mbNoError;
end;

function GetCommEventCounterRequest(usrPtr : Pointer; var Status : word; var EventCount : word) : integer;
{$IFDEF MSWINDOWS}stdcall;{$ELSE}cdecl;{$ENDIF}
begin
  Result:=mbNoError;
end;

function GetCommEventLogRequest(usrPtr : Pointer; var Status : word; var EventCount : word; var MessageCount : word; Data : Pointer; var EventsAmount : word) : integer;
{$IFDEF MSWINDOWS}stdcall;{$ELSE}cdecl;{$ENDIF}
Var
  Items : packed array[0..15] of byte;
  c : integer;
begin
  for c:=0 to 15 do
    Items[c]:=c;
  move(Items, Data^, 16);
  EventsAmount:=16;
  Result:=mbNoError;
end;

function ReportServerIDRequest(usrPtr : Pointer; Data : Pointer; var DataSize : word) : integer;
{$IFDEF MSWINDOWS}stdcall;{$ELSE}cdecl;{$ENDIF}
Var
  ServerID : AnsiString;
begin
  ServerID := 'SnapMB Device';
  DataSize := Length(ServerID);
  move(ServerID[1], Data^ ,DataSize);
  Result:=mbNoError;
end;

function ReadFIFOQueueRequest(usrPtr : Pointer; PtrAddress : word; FIFOValues : Pointer; var FifoCount : word) : integer;
{$IFDEF MSWINDOWS}stdcall;{$ELSE}cdecl;{$ENDIF}
Var
  Items : packed array[0..15] of word;
  c : integer;
begin
  for c:=0 to 15 do
    Items[c]:=c;
  move(Items, FIFOValues^, 32);
  FifoCount:=16;
  Result:=mbNoError;
end;

function EncapsulatedIT(usrPtr : Pointer; MEI_Type : byte; MEI_DataReq : pointer; ReqDataSize : word; MEI_DataRes : Pointer; var ResDataSize : word) : integer;
{$IFDEF MSWINDOWS}stdcall;{$ELSE}cdecl;{$ENDIF}
begin
  // Just Echo
  move(MEI_DataReq^, MEI_DataRes^, ReqDataSize);
  ResDataSize:=ReqDataSize;
  Result:=mbNoError;
end;

function UsrFunctionRequest(usrPtr : Pointer; UsrFunction : byte; RxPDU : Pointer; RxPDUSize : word; TxPDU : Pointer; var TxPDUSize : word) : integer;
{$IFDEF MSWINDOWS}stdcall;{$ELSE}cdecl;{$ENDIF}
begin
  Result:=mbNoError;
end;

{ TMainForm }

procedure TMainForm.btnClearLogClick(Sender: TObject);
begin
  doClearLog:=true;
end;

procedure TMainForm.btnClearDumpClick(Sender: TObject);
begin
  doClearDump:=true;
end;

procedure TMainForm.btnRandomFillAllClick(Sender: TObject);
Var
  c : integer;
begin
  case PCGrids.ActivePageIndex of
    0: begin
      for c:=0 to bits_amount-1 do
        Coils[c]:=boolean(random(4) div 2);
      gridCoils.Invalidate;
    end;
    1:begin
      for c:=0 to bits_amount-1 do
        DiscreteInputs[c]:=boolean(random(4) div 2);
      gridDiscrete.Invalidate;
    end;
    2 : begin
      for c:=0 to regs_amount-1 do
        InputRegisters[c]:=random(65535);
      InvalidateRegGrid(gridInputRegs,InputRegisters);
    end;
    3 : begin
      for c:=0 to regs_amount-1 do
        HoldingRegisters[c]:=random(65535);
      InvalidateRegGrid(gridHoldingRegs,HoldingRegisters);
    end;
  end;
end;

procedure TMainForm.btnIncreaseFillClick(Sender: TObject);
Var
  c : integer;
begin
  case PCGrids.ActivePageIndex of
    0:;
    1:;
    2 : begin
      for c:=0 to regs_amount-1 do
        InputRegisters[c]:=c;
      InvalidateRegGrid(gridInputRegs,InputRegisters);
    end;
    3 : begin
      for c:=0 to regs_amount-1 do
        HoldingRegisters[c]:=c;
      InvalidateRegGrid(gridHoldingRegs,HoldingRegisters);
    end;
  end;

end;

procedure TMainForm.FillAreaClick(Sender: TObject);
Var
  doRandom : boolean;
begin
  doRandom:=(Sender as TBCButton).Tag<>0;
  case PCGrids.ActivePageIndex of
    0 : FillBitGrid(GridCoils, Coils, doRandom);
    1 : FillBitGrid(GridDiscrete, DiscreteInputs, doRandom);
    2 : FillRegGrid(gridInputRegs, InputRegisters, doRandom);
    3 : FillRegGrid(gridHoldingRegs, HoldingRegisters, doRandom);
  end;
end;

procedure TMainForm.btnConnectionClick(Sender: TObject);
Var
  Result : integer;
begin
  SB.Panels[3].Text:='';
  if Connected then
  begin
    Device.Stop;
    Connected:=false;
    ClearTree;
  end
  else begin
    Result := Device.Start;
    SB.Panels[3].Text:=ErrorText(Result);
    Connected := Result = mbNoError;
  end;
end;

procedure TMainForm.btnPropertiesClick(Sender: TObject);
begin
  WasConnected:=Connected;
  if SettingsForm.EditSettings(Settings) then
  begin
    DeviceChangeTo;
    if Settings.ClrOnCreate then
    begin
      InitData;
      InvalidateRegGrid(GridInputRegs, InputRegisters);
      InvalidateRegGrid(GridHoldingRegs, HoldingRegisters);
      gridCoils.Invalidate;
      gridDiscrete.Invalidate;
    end;
    UpdateSettingsPanels;
    if WasConnected then
      Connected:=Device.Start=mbNoError;
  end;
end;

procedure TMainForm.btnZeroAllClick(Sender: TObject);
begin
  case PCGrids.ActivePageIndex of
    0: begin
      fillchar(Coils, SizeOf(Coils), #0);
      gridCoils.Invalidate;
    end;
    1:begin
      fillchar(DiscreteInputs, SizeOf(DiscreteInputs), #0);
      gridDiscrete.Invalidate;
    end;
    2 : begin
      fillchar(InputRegisters, SizeOf(InputRegisters), #0);
      InvalidateRegGrid(gridInputRegs,InputRegisters);
    end;
    3 : begin
      fillchar(HoldingRegisters, SizeOf(HoldingRegisters), #0);
      InvalidateRegGrid(gridHoldingRegs,HoldingRegisters);
    end;
  end;
end;

procedure TMainForm.cbFormatChange(Sender: TObject);
begin
  if PCGrids.ActivePageIndex = 2 then
  begin
    if gridInputRegs.Tag<>cbFormat.ItemIndex then
    begin
      gridInputRegs.Tag:=cbFormat.ItemIndex;
      InvalidateRegGrid(gridInputRegs, InputRegisters);
    end;
  end
  else begin
    if gridHoldingRegs.Tag<>cbFormat.ItemIndex then
    begin
      gridHoldingRegs.Tag:=cbFormat.ItemIndex;
      InvalidateRegGrid(gridHoldingRegs, HoldingRegisters);
    end;
  end;
end;

procedure TMainForm.FormClose(Sender: TObject; var CloseAction: TCloseAction);
begin
  SelfDestroying:=true;
  Device.Stop;
  timLog.Enabled:=false;
  Sleep(500);
  DeviceDestroy;
end;

procedure TMainForm.FormCreate(Sender: TObject);
begin
  SetDefaults(Settings);
  InitControls;
  InitData;
  Connected:=false;
  SelfDestroying:=false;
  UpdateSettingsPanels;
  DataQueue:=TDataQueue.Create(1024, SizeOf(TIOBuffer));
  DeviceCreate;
end;

procedure TMainForm.FormDestroy(Sender: TObject);
begin
  DataQueue.Free;
end;

procedure TMainForm.FormShow(Sender: TObject);
begin
  Position:=poScreenCenter;
end;

procedure TMainForm.gridCoilsDblClick(Sender: TObject);
Var
  Index : integer;
begin
  Index:=(gridCoils.Row-1)*32+gridCoils.Col-1;
  Coils[Index]:=not Coils[Index];
  gridCoils.Invalidate;
end;

procedure TMainForm.gridCoilsDrawCell(Sender: TObject; aCol, aRow: Integer;
  aRect: TRect; aState: TGridDrawState);
begin
  if (aCol>0) and (aRow>0) then
  begin
    if Coils[(aRow - 1) * 32 + (aCol -1)] then
      gridCoils.Canvas.Brush.Color:=$002F8AFF
    else
      gridCoils.Canvas.Brush.Color:=gridCoils.Color;
    inflateRect(aRect,-2,-2);
    gridCoils.Canvas.FillRect(aRect);
  end;
end;

procedure TMainForm.gridCoilsKeyDown(Sender: TObject; var Key: Word;
  Shift: TShiftState);
Var
  Index : integer;
begin
  if Key in [13, 32, 48, 49] then
  begin
    Index:=(gridCoils.Row-1)*32+gridCoils.Col-1;
    if (Key = 13) or (key = 32) then
      Coils[Index]:=not Coils[Index]
    else
      if Key=48 then
        Coils[Index]:=false
      else
        Coils[Index]:=true;
    gridCoils.Invalidate;
  end;
end;


procedure TMainForm.gridCoilsSelectCell(Sender: TObject; aCol, aRow: Integer;
  var CanSelect: Boolean);
begin
  ItemIndex.Caption:=IntToStr((aRow-1)*32+(aCol-1));
end;

procedure TMainForm.gridDiscreteDblClick(Sender: TObject);
Var
  Index : integer;
begin
  Index:=(gridDiscrete.Row-1)*32+gridDiscrete.Col-1;
  DiscreteInputs[Index]:=not DiscreteInputs[Index];
  gridDiscrete.Invalidate;
end;

procedure TMainForm.gridDiscreteDrawCell(Sender: TObject; aCol, aRow: Integer;
  aRect: TRect; aState: TGridDrawState);
begin
  if (aCol>0) and (aRow>0) then
  begin
    if DiscreteInputs[(aRow - 1) * 32 + (aCol -1)] then
      gridDiscrete.Canvas.Brush.Color:=$002F8AFF
    else
      gridDiscrete.Canvas.Brush.Color:=gridCoils.Color;
    inflateRect(aRect,-2,-2);
    gridDiscrete.Canvas.FillRect(aRect);
  end;
end;

procedure TMainForm.gridDiscreteKeyDown(Sender: TObject; var Key: Word;
  Shift: TShiftState);
Var
  Index : integer;
begin
  if Key in [13, 32, 48, 49] then
  begin
    Index:=(gridDiscrete.Row-1)*32+gridDiscrete.Col-1;
    if (Key = 13) or (key = 32) then
      DiscreteInputs[Index]:=not DiscreteInputs[Index]
    else
      if Key=48 then
        DiscreteInputs[Index]:=false
      else
        DiscreteInputs[Index]:=true;
    gridDiscrete.Invalidate;
  end;
end;

procedure TMainForm.gridHoldingRegsSelectCell(Sender: TObject; aCol,
  aRow: Integer; var CanSelect: Boolean);
begin
  ItemIndex.Caption:=IntToStr((aRow-1)*16+(aCol-1));
end;

procedure TMainForm.gridHoldingRegsValidateEntry(sender: TObject; aCol,
  aRow: Integer; const OldValue: string; var NewValue: String);
Var
  Value : word;
begin
  if not IsValidInt16(NewValue, gridHoldingRegs.tag=1, Value) then
  begin
    messageDlg(format('%s is not a valid value',[NewValue]),mtError, [mbok], 0);
    NewValue:=OldValue;
  end
  else begin
    if gridHoldingRegs.tag=1 then
      NewValue:=IntToHex(Value, 4);
    HoldingRegisters[(aRow-1) * 16 + (aCol-1)]:=Value;
  end;
end;

procedure TMainForm.gridInputRegsSelectCell(Sender: TObject; aCol, aRow: Integer;
  var CanSelect: Boolean);
begin
  ItemIndex.Caption:=IntToStr((aRow-1)*16+(aCol-1));
end;

procedure TMainForm.gridInputRegsValidateEntry(sender: TObject; aCol,
  aRow: Integer; const OldValue: string; var NewValue: String);
Var
  Value : word;
begin
  if not IsValidInt16(NewValue, gridInputRegs.tag=1, Value) then
  begin
    messageDlg(format('%s is not a valid value',[NewValue]),mtError, [mbok], 0);
    NewValue:=OldValue;
  end
  else begin
    if gridInputRegs.tag=1 then
      NewValue:=IntToHex(Value, 4);
    InputRegisters[(aRow-1) * 16 + (aCol-1)]:=Value;
  end;
end;

procedure TMainForm.LogDrawCell(Sender: TObject; aCol, aRow: Integer;
  aRect: TRect; aState: TGridDrawState);
begin
  if (ACol = 0) and (AROw > 0) then
  begin
    Log.Canvas.Brush.Color := clWhite;
    Log.Canvas.FillRect(aRect);
    InflateRect(aRect, -2, -2);
    if Log.Cells[0,ARow]='' then
      Log.Canvas.Brush.Color := $004CC16C
    else
      Log.Canvas.Brush.Color := $004F53D9;
    Log.Canvas.FillRect(aRect);
  end;
end;

procedure TMainForm.Panel2Click(Sender: TObject);
begin

end;

procedure TMainForm.PCGridsChange(Sender: TObject);
begin
  cbFormat.Visible:=PCGrids.ActivePageIndex>1;
  btnIncreaseFill.Visible:=PCGrids.ActivePageIndex>1;
  case PCGrids.ActivePageIndex of
    0:begin
      with gridCoils.Selection.TopLeft do
        ItemIndex.Caption:=IntToStr((Y-1)*32+(X-1));
    end;
    1:begin
      with gridDiscrete.Selection.TopLeft do
        ItemIndex.Caption:=IntToStr((Y-1)*32+(X-1));
    end;
    2:begin
      cbFormat.ItemIndex:=gridInputRegs.Tag;
      with gridInputRegs.Selection.TopLeft do
        ItemIndex.Caption:=IntToStr((Y-1)*16+(X-1));
    end;
    3:begin
      cbFormat.ItemIndex:=GridHoldingRegs.Tag;
      with gridHoldingRegs.Selection.TopLeft do
        ItemIndex.Caption:=IntToStr((Y-1)*16+(X-1));
    end;
  end;
end;

procedure TMainForm.timLogTimer(Sender: TObject);
Var
  Event : TSrvEvent;
  IOBuffer : TIOBuffer;
  IfSet, IfDet : integer;
  Info : TDeviceInfo;
begin
  if SelfDestroying then
    exit;

  if doClearLog then
  begin
    Log.RowCount:=2;
    Log.Cells[0,1]:='';
    Log.Cells[1,1]:='';
    doClearLog:=false;
  end;

  if DoClearDump then
  begin
    Dump.Clear;
    DoClearDump:=false;
  end;

  while DataQueue.Extract(@IoBuffer) do
    DumpIOData(IoBuffer);

  if Device.PickEvent(Event) then // check if there is at least an event
  begin
    Log.BeginUpdate;
    try
      AddEvent(Event);
      while Device.PickEvent(Event) do
        AddEvent(Event);
    finally
      Log.Row:=Log.RowCount-1;
      Log.EndUpdate;
    end;
  end;

  Device.GetSerialInterframe(IfSet, IfDet);

  if IfSet<>OldIFSet then
  begin
    lblInterframeSet.Caption:=IntToStr(IfSet);
    OldIFSet:=IfSet;
  end;

  if IfDet<>OldIfDet then
  begin
    lblInterframeDetected.Caption:=IntToStr(IfDet);
    OldIfDet:=IfDet;
  end;

  Device.GetDeviceInfo(Info);
  if (Info.ClientsBlocked<>FLastInfo.ClientsBlocked) or
     (Info.ClientsCount<>FLastInfo.ClientsCount) or
     (Info.LastError<>FLastInfo.LastError) or
     (Info.Running<>FLastInfo.Running) then
   begin
     if Info.Running then
       SB.Panels[0].Text:='Running'
     else
       SB.Panels[0].Text:='Stopped';
     SB.Panels[1].Text:=Format('Clients : %d', [Info.ClientsCount]);
     SB.Panels[2].Text:=Format('Clients blocked: %d', [Info.ClientsBlocked]);
     FLastInfo:=Info;
   end;
end;

procedure TMainForm.InitControls;
Var
  c : integer;
begin
  PCGrids.ActivePageIndex:=0;
  cbFormat.Visible:=false;
  btnIncreaseFill.Visible:=false;
  for c:=1 to gridDiscrete.RowCount -1 do
    gridDiscrete.Cells[0, c]:=inttostr((c-1)*32);
  for c:=1 to gridCoils.RowCount -1 do
    gridCoils.Cells[0, c]:=inttostr((c-1)*32);
  for c:=1 to gridInputRegs.RowCount -1 do
    gridInputRegs.Cells[0, c]:=inttostr((c-1)*16);
  for c:=1 to gridHoldingRegs.RowCount -1 do
    gridHoldingRegs.Cells[0, c]:=inttostr((c-1)*16);
  InvalidateRegGrid(GridInputRegs, InputRegisters);
  InvalidateRegGrid(GridHoldingRegs, HoldingRegisters);
  ClearTree;
end;

procedure TMainForm.AddEvent(Event: TSrvEvent);
Var
  S : string;
  Node : TTreeNode;
begin
  // Regresh grids
  case Event.EvtCode of
    evcWriteSingleCoil,
    evcWriteMultiCoils: begin
      gridCoils.Invalidate;
    end;
    evcWriteSingleReg,
    evcWriteMultiRegs,
    evcMaskWriteReg,
    evcReadWriteMultiRegs,
    evcWriteFileRecord: begin
      InvalidateRegGrid(gridHoldingRegs, HoldingRegisters);
    end;
    evcDevClientAdded : begin
      Node:=Tree.Items.AddChild(Tree.Items[0],IPAddress(Event.EvtSender));
      Node.ImageIndex:=1;
      Node.SelectedIndex:=1;
    end;
    evcDevClientDisconnected,
    evcDevClientDisTimeout,
    evcDevClientTerminated: begin
      Node:=Tree.Items.FindNodeWithText(IPAddress(Event.EvtSender));
      if Assigned(Node) then
        Tree.Items.Delete(Node);
    end;
  end;

  S := EventText(Event);
  if Event.EvtRetCode<>0 then
    AddEventLine('.', S)
  else
    AddEventLine('', S);
end;

procedure TMainForm.AddEventLine(S0, S: string);
begin
  if (Log.RowCount>2) or (Log.Cells[1,1]<>'') then
    Log.RowCount:=Log.RowCount+1;

  Log.Cells[0,Log.RowCount-1]:=S0;
  Log.Cells[1,Log.RowCount-1]:=S;
end;

procedure TMainForm.UpdateSettingsPanels;
begin
  if Settings.DataLink = dlEthernet then
  begin
    lblDataLink.Caption:='Ethernet';
    lblFlavourLabel.Caption:='Protocol';
    lblFlavour.Caption:=NetProtocols[Settings.NetProto];
    lblLinkNameLabel.Caption:='Address';
    lblLinkName.Caption:=Settings.EthParams.Address;
    lblParamsLabel.Caption:='Port';
    lblParams.Caption:=IntToStr(Settings.EthParams.Port);
  end
  else begin
    lblDataLink.Caption:='Serial';
    lblFlavourLabel.Caption:='Format';
    lblFlavour.Caption:=SerialFormats[Settings.SerFormat];

    lblLinkNameLabel.Caption:='Port';
    lblLinkName.Caption:=Settings.SerParams.Port;
    lblParamsLabel.Caption:='Params';
    with Settings.SerParams do
      lblParams.Caption:=SysUtils.Format('%d, %s, %d, %d',[BaudRate, Parity, DataBits, StopBits]);
  end;
  lblDeviceID.Caption:=IntToStr(Settings.DeviceID);
end;

procedure TMainForm.DeviceDestroy;
begin
  Device.Free;
end;

procedure TMainForm.DeviceCreate;
begin
  Device:=TSnapMBDevice.Create(Settings.NetProto, Settings.DeviceID, Settings.EthParams.Address, Settings.EthParams.Port);
  Device.SetParam(par_DisconnectTimeout, Settings.EthParams.DisTimeout);
  Device.SetParam(par_AllowSerFunOnEth, integer(Settings.SerialOnEth));
  AddEventLine('','Default Ethernet device created');
  DeviceMap;
  timLog.Enabled:=true;
end;

procedure TMainForm.DeviceChangeTo;
begin
  timLog.Enabled:=false;
  Sleep(500);
  if Settings.DataLink = dlEthernet then
  begin
    Device.ChangeTo(Settings.NetProto, Settings.DeviceID, Settings.EthParams.Address, Settings.EthParams.Port);
    Device.SetParam(par_DisconnectTimeout, Settings.EthParams.DisTimeout);
    Device.SetParam(par_AllowSerFunOnEth, integer(Settings.SerialOnEth));
    AddEventLine('','Device changed to Ethernet');
  end
  else begin
    with Settings do
      Device.ChangeTo(SerFormat, DeviceID, SerParams.Port, SerParams.BaudRate, SerParams.Parity, SerParams.DataBits, SerParams.StopBits, SerParams.Flow);
    Device.SetParam(par_InterframeDelay, Settings.SerIframe);
    AddEventLine('','Device changed to Serial');
  end;
  DeviceMap;
  timLog.Enabled:=true;
end;

procedure TMainForm.DeviceMap;
begin
  fillchar(FLastInfo, SizeOf(TDeviceInfo), #0);
  FLastInfo.ClientsBlocked:=-1; // to force the refresh
  // Map resources
  Device.RegisterArea(mbAreaDiscreteInputs, @DiscreteInputs, bits_amount);
  Device.RegisterArea(mbAreaCoils, @Coils, bits_amount);
  Device.RegisterArea(mbAreaInputRegisters, @InputRegisters, regs_amount);
  Device.RegisterArea(mbAreaHoldingRegisters, @HoldingRegisters, regs_amount);
  // Map Callbacks

  //
  // Next 6 callbacks are commented because we are using the shared resources
  // Nevertheless they can work contemporary to the resources
  //
(*
  Device.RegisterCallback(cbkDiscreteInputs, @DiscreteInputsRequest, Self);
  Device.RegisterCallback(cbkCoils, @CoilsRequest, Self);
  Device.RegisterCallback(cbkInputRegisters, @InputRegistersRequest, Self);
  Device.RegisterCallback(cbkHoldingRegisters,@HoldingRegistersRequest, Self);
  Device.RegisterCallback(cbkReadWriteRegisters, @ReadWriteMultipleRegistersRequest, Self);
  Device.RegisterCallback(cbkMaskRegister, @MaskRegisterRequest, Self);
*)
  Device.RegisterCallback(cbkPacketLog, @PacketLog, Self);
  Device.RegisterCallback(cbkFileRecord, @FileRecordRequest, Self);
  Device.RegisterCallback(cbkExceptionStatus, @ExceptionStatusRequest, Self);
  Device.RegisterCallback(cbkDiagnostics, @DiagnosticsRequest, Self);
  Device.RegisterCallback(cbkGetCommEventCounter, @GetCommEventCounterRequest, Self);
  Device.RegisterCallback(cbkGetCommEventLog, @GetCommEventLogRequest, Self);
  Device.RegisterCallback(cbkReportServerID, @ReportServerIDRequest, Self);
  Device.RegisterCallback(cbkReadFIFOQueue, @ReadFIFOQueueRequest, Self);
  Device.RegisterCallback(cbkEncapsulatedIT, @EncapsulatedIT, Self);
  Device.RegisterCallback(cbkUsrFunction, @UsrFunctionRequest, Self);
end;

procedure TMainForm.ClearTree;
Var
  Root : TTreeNode;
begin
  Tree.Items.Clear;
  Root:=Tree.Items.AddChild(nil,'Clients');
  Root.ImageIndex:=0;
  Root.SelectedIndex:=0;
  OldIfSet :=0;
  OldIfDet :=0;
end;

procedure TMainForm.DumpIOData(var IOBuffer: TIOBuffer);
Var
  sHeader : string;
  SHex, SChr : string;
  Ch : AnsiChar;
  c, cnt : integer;
begin
  if IOBuffer.Direction = PacketLog_IN then // Indication
    sHeader:=format('[Indication from %s]', [IPAddress(IOBuffer.Peer)])
  else // Response
    sHeader:=format('[Response to %s]', [IPAddress(IOBuffer.Peer)]);

  if IOBuffer.Size > 0 then
  begin
    SHex:='';SChr:='';cnt:=0;
    Dump.Lines.BeginUpdate;
    try
      Dump.Lines.Add(sHeader);
      for c := 0 to IOBuffer.Size - 1 do
      begin
        SHex:=SHex+IntToHex(IOBuffer.Data[c],2)+' ';
        Ch:=AnsiChar(IOBuffer.Data[c]);
        if not (Ch in ['a'..'z','A'..'Z','0'..'9','_','$','-',#32]) then
          Ch:='.';
        SChr:=SChr+String(Ch);
        inc(cnt);
        if cnt=16 then
        begin
          Dump.Lines.Add(SHex+'  '+SChr);
          SHex:='';SChr:='';
          cnt:=0;
        end;
      end;
      // Dump remainder
      if cnt>0 then
      begin
        while Length(SHex)<48 do
          SHex:=SHex+' ';
        Dump.Lines.Add(SHex+'  '+SChr);
      end;
    finally
      Dump.Lines.EndUpdate;
    end;
    Dump.CaretY:=MaxInt;
  end;
end;

procedure TMainForm.PushIOData(Direction: integer; Peer : longword; Data: Pointer; Size: integer);
Var
  IOBuffer : TIOBuffer;
begin
  if Size > IoDataSize then
    Size:=IoDataSize;
  move(pbyte(Data)[0], IoBuffer.Data, Size);
  IoBuffer.Direction:=word(Direction);
  IoBuffer.Size:=word(Size);
  IoBuffer.Peer:=Peer;
  DataQueue.Insert(@IoBuffer);
end;

procedure TMainForm.SetFConnected(AValue: boolean);
begin
  FConnected:=AValue;
  if FConnected then
  begin
    pnlConnection.BackGround.Color:=color_Connected;
    lblConnection.Caption:='Running';
    pnlDataLink.BackGround.Color:=color_DataLinkActive;
    pnlChannel.BackGround.Color:=color_ChannelActive;
    btnConnection.ImageIndex:=1;
    btnConnection.Hint:='Stop Device';
  end
  else begin
    pnlConnection.BackGround.Color:=color_Disconnected;
    lblConnection.Caption:='Stopped';
    pnlDataLink.BackGround.Color:=color_DataLinkOff;
    pnlChannel.BackGround.Color:=color_ChannelOff;
    btnConnection.ImageIndex:=0;
    btnConnection.Hint:='Start Device';
  end;
end;

end.

