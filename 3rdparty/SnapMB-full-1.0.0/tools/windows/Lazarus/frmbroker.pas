unit frmBroker;

{$mode objfpc}{$H+}

interface

uses
  Classes, SysUtils, Forms, Controls, Graphics, Dialogs, ExtCtrls, Buttons,
  StdCtrls, ComCtrls, Grids, SynEdit, SynHighlighterIni, LMessages,
  LCLIntf, SynHighlighterHTML, SynHighlighterAny, SnapMBCommon,
  BCButton, BGRAImageList, BCPanel,
  BCComboBox, BCListBox, SnapMB, Types, frmBrokerSettings;

const
  WM_EXECUTE_FUN = LM_USER + $101;

type

  TFunDescriptor = record
    Name : string;
    Info : string;
    EdLabels : array[1..5] of string;
    EdEnable : array[1..5] of boolean;
  end;


  { TMainForm }

  TMainForm = class(TForm)
    BCPanel18: TBCPanel;
    btnClients: TBCButton;
    btnFunction: TBCButton;
    BCPanel1: TBCPanel;
    btnClearHistory: TBCButton;
    FunExecuted: TSynEdit;
    edUsrFunction: TEdit;
    gridPDUOut: TStringGrid;
    gridPDUIn: TStringGrid;
    PDUOutItemIndex: TBCPanel;
    Panel13: TPanel;
    Panel14: TPanel;
    Panel15: TPanel;
    Panel16: TPanel;
    Panel5: TPanel;
    SpecialFunDump: TSynEdit;
    FunPanel_1: TBCPanel;
    FunPanel_3: TBCPanel;
    FunPanel_4: TBCPanel;
    FunPanel_2: TBCPanel;
    FunPanel_5: TBCPanel;
    BCPanel15: TBCPanel;
    BCPanel16: TBCPanel;
    BCPanel17: TBCPanel;
    BCPanel2: TBCPanel;
    BGRAImageList1: TBGRAImageList;
    btnClearDump: TBCButton;
    btnConnection: TBCButton;
    btnIncreaseFill: TBCButton;
    btnProperties: TBCButton;
    btnRandomFillAll: TBCButton;
    btnRandomFillArea: TBCButton;
    btnZeroAll: TBCButton;
    btnClrPDUs: TBCButton;
    btnZeroArea: TBCButton;
    cbFormat: TBCComboBox;
    cbFormatPDU: TBCComboBox;
    cbDeviceID: TBCComboBox;
    cbFunction: TBCComboBox;
    Dump: TSynEdit;
    edRDAddress: TEdit;
    edRDAmount: TEdit;
    edValue: TEdit;
    edWRAddress: TEdit;
    edWRAmount: TEdit;
    gridCoils: TStringGrid;
    gridDiscrete: TStringGrid;
    gridHoldingRegs: TStringGrid;
    gridInputRegs: TStringGrid;
    ImageList1: TImageList;
    ItemIndex: TBCPanel;
    ItemIndex1: TBCPanel;
    Label1: TLabel;
    Label13: TLabel;
    Label2: TLabel;
    Label3: TLabel;
    Label4: TLabel;
    lblFunInfo: TLabel;
    lblConnection: TLabel;
    lblDataLink: TLabel;
    lblDataLink1: TLabel;
    lblBrokerType: TLabel;
    Bus: TLabel;
    lblDeviceIDLabel: TLabel;
    lblFlavour: TLabel;
    lblFlavourLabel: TLabel;
    lblLinkName: TLabel;
    lblLinkNameLabel: TLabel;
    lblLinkNameLabel1: TLabel;
    lblParams: TLabel;
    lblParamsLabel: TLabel;
    Panel10: TPanel;
    PC: TPageControl;
    Panel1: TPanel;
    Panel11: TPanel;
    Panel12: TPanel;
    Panel2: TPanel;
    Panel3: TPanel;
    Panel4: TPanel;
    Panel6: TPanel;
    Panel7: TPanel;
    Panel8: TPanel;
    Panel9: TPanel;
    PCGrids: TPageControl;
    pnlChannel: TBCPanel;
    pnlNetController: TBCPanel;
    pnlConnection: TBCPanel;
    pnlDataLink: TBCPanel;
    SB: TStatusBar;
    Splitter1: TSplitter;
    Splitter2: TSplitter;
    Splitter3: TSplitter;
    SynIniSyn1: TSynIniSyn;
    TabSheet1: TTabSheet;
    TabSheet10: TTabSheet;
    TabSheet2: TTabSheet;
    TabSheet3: TTabSheet;
    TabSheet4: TTabSheet;
    TabSheet5: TTabSheet;
    TabSheet6: TTabSheet;
    timLog: TTimer;
    procedure btnBroadcastClick(Sender: TObject);
    procedure btnClearDumpClick(Sender: TObject);
    procedure btnClearHistoryClick(Sender: TObject);
    procedure btnClrPDUsClick(Sender: TObject);
    procedure btnConnectionClick(Sender: TObject);
    procedure btnFunctionClick(Sender: TObject);
    procedure btnIncreaseFill1Click(Sender: TObject);
    procedure btnIncreaseFillClick(Sender: TObject);
    procedure btnPropertiesClick(Sender: TObject);
    procedure btnRandomFillAllClick(Sender: TObject);
    procedure btnRandomFillAreaClick(Sender: TObject);
    procedure btnZeroAllClick(Sender: TObject);
    procedure btnZeroAreaClick(Sender: TObject);
    procedure cbDeviceIDChange(Sender: TObject);
    procedure cbFormatChange(Sender: TObject);
    procedure cbFunctionChange(Sender: TObject);
    procedure FillAreaClick(Sender: TObject);
    procedure FormCreate(Sender: TObject);
    procedure FormDestroy(Sender: TObject);
    procedure gridCoilsDblClick(Sender: TObject);
    procedure gridCoilsDrawCell(Sender: TObject; aCol, aRow: Integer;
      aRect: TRect; aState: TGridDrawState);
    procedure gridCoilsKeyDown(Sender: TObject; var Key: Word;
      Shift: TShiftState);
    procedure gridBitsSelectCell(Sender: TObject; aCol, aRow: Integer;
      var CanSelect: Boolean);
    procedure gridDiscreteDblClick(Sender: TObject);
    procedure gridDiscreteDrawCell(Sender: TObject; aCol, aRow: Integer;
      aRect: TRect; aState: TGridDrawState);
    procedure gridDiscreteKeyDown(Sender: TObject; var Key: Word;
      Shift: TShiftState);
    procedure gridHoldingRegsValidateEntry(sender: TObject; aCol,
      aRow: Integer; const OldValue: string; var NewValue: String);
    procedure gridInputRegsValidateEntry(sender: TObject; aCol, aRow: Integer;
      const OldValue: string; var NewValue: String);
    procedure gridPDUOutSelectCell(Sender: TObject; aCol, aRow: Integer;
      var CanSelect: Boolean);
    procedure GridRegsSelectCell(Sender: TObject; aCol, aRow: Integer;
      var CanSelect: Boolean);
    procedure PCGridsChange(Sender: TObject);
  private
    Fun : array[0..19] of TFunDescriptor;
    FunPanels : array[1..5] of TBCPanel;
    FunEdits : array[1..5] of TEdit;
    FConnected : boolean;
    doClearDump : boolean;
    WasConnected : boolean;
    Settings: TMBBrokerSetting;
    Broker : TSnapMBBroker;
    procedure ActionInfo(funidx : integer);
    procedure InitControls;
    procedure InitFunctions;
    procedure SetFConnected(AValue: boolean);
    procedure UpdateSettingsPanels;
    procedure UpdateEdits;
    procedure BrokerChangeTo;
    procedure BrokerDestroy;
    function WordOf(Edit : TEdit) : word;
    procedure DumpIOData(var IOBuffer : TIOBuffer);
    procedure ClrPDUOut;
    procedure ClrPDUIn;
    function funReadCoils(DeviceID : byte) : integer;
    function funReadDiscreteInputs(DeviceID : byte) : integer;
    function funReadInputRegisters(DeviceID : byte) : integer;
    function funReadHoldingRegisters(DeviceID : byte) : integer;
    function funWriteSingleCoil(DeviceID : byte) : integer;
    function funWriteSingleRegister(DeviceID : byte) : integer;
    function funWriteMultipleCoils(DeviceID : byte) : integer;
    function funWriteMultipleRegisters(DeviceID : byte) : integer;
    function funMaskWriteRegisters(DeviceID : byte) : integer;
    function funReadWriteMultipleRegisters(DeviceID : byte) : integer;
    function funReadFileRecord(DeviceID : byte) : integer;
    function funWriteFileRecord(DeviceID : byte) : integer;
    function funReadExceptionStatus(DeviceID : byte) : integer;
    function funDiagnostics(DeviceID : byte) : integer;
    function funGetCommEventCounter(DeviceID : byte) : integer;
    function funGetCommEventLog(DeviceID : byte) : integer;
    function funReportServerID(DeviceID : byte) : integer;
    function funReadFIFOQueue(DeviceID : byte) : integer;
    function funEncapsulatedInterfaceTransport(DeviceID : byte) : integer;
    function funUserFunction(DeviceID : byte) : integer;
  public
    procedure WmExecuteFun(var Msg: TLMessage) message WM_EXECUTE_FUN;
    property Connected : boolean read FConnected write SetFConnected;
  end;

var
  MainForm: TMainForm;

implementation

{$R *.lfm}

{ TMainForm }


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

procedure TMainForm.btnRandomFillAreaClick(Sender: TObject);
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

procedure TMainForm.btnConnectionClick(Sender: TObject);
begin
  SB.Panels[1].Text:='';
  if Connected then
  begin
    Broker.Disconnect;
    Connected:=false;
  end
  else
    Connected := Broker.Connect = mbNoError;
  ActionInfo(-1);
end;

procedure TMainForm.btnClrPDUsClick(Sender: TObject);
begin
  ClrPDUOut;
  ClrPDUIn;
end;

procedure TMainForm.btnClearDumpClick(Sender: TObject);
begin
  Dump.Clear;
  SpecialFunDump.Clear;
end;

procedure TMainForm.btnBroadcastClick(Sender: TObject);
begin

end;

procedure TMainForm.btnClearHistoryClick(Sender: TObject);
begin
  FunExecuted.Clear;
end;

procedure TMainForm.btnFunctionClick(Sender: TObject);
begin
  PostMessage(Self.Handle, WM_EXECUTE_FUN, cbFunction.ItemIndex, cbDeviceID.ItemIndex);
end;

procedure TMainForm.btnIncreaseFill1Click(Sender: TObject);
begin

end;

procedure TMainForm.btnPropertiesClick(Sender: TObject);
begin
  WasConnected:=Connected;
  if SettingsForm.EditSettings(Settings) then
  begin
    BrokerChangeTo;
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
      Connected:=Broker.Connect=mbNoError;
  end;
end;

procedure TMainForm.btnZeroAreaClick(Sender: TObject);
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

procedure TMainForm.cbDeviceIDChange(Sender: TObject);
begin
  UpdateEdits;
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

procedure TMainForm.cbFunctionChange(Sender: TObject);
begin
  UpdateEdits;
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

procedure TMainForm.FormCreate(Sender: TObject);
begin
  SetDefaults(Settings);
  InitFunctions;
  InitControls;
  InitData;
  Connected:=false;
  UpdateSettingsPanels;
  with Settings do
    Broker := TSnapMBBroker.Create(settings.CliNetProto, Settings.CliEthParams.Address, Settings.CliEthParams.Port);
end;

procedure TMainForm.FormDestroy(Sender: TObject);
begin
  BrokerDestroy;
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

procedure TMainForm.gridBitsSelectCell(Sender: TObject; aCol, aRow: Integer;
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

procedure TMainForm.gridPDUOutSelectCell(Sender: TObject; aCol, aRow: Integer;
  var CanSelect: Boolean);
begin
  PDUOutItemIndex.Caption:=IntToStr((aRow-1)*32+(aCol-1));
end;

procedure TMainForm.GridRegsSelectCell(Sender: TObject; aCol, aRow: Integer;
  var CanSelect: Boolean);
begin
  ItemIndex.Caption:=IntToStr((aRow-1)*16+(aCol-1));
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

procedure TMainForm.ActionInfo(funidx : integer);
Var
  Status : TDeviceStatus;
begin
  Status:=Broker.GetDeviceStatus;
  SB.Panels[0].Text:=format('Job time : %d ms',[Status.Time]) ;
  SB.Panels[1].Text:=ErrorText(Status.LastError);
  Connected:=Status.Connected;
  if funidx>-1 then
    FunExecuted.Append(format('%s : [%s]',[Fun[funidx].Name, ErrorText(Status.LastError)]));
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
  ClrPDUOut;
  ClrPDUIn;
end;

procedure TMainForm.InitFunctions;
Var
  c : integer;
begin
  FunPanels[1]:=FunPanel_1;
  FunPanels[2]:=FunPanel_2;
  FunPanels[3]:=FunPanel_3;
  FunPanels[4]:=FunPanel_4;
  FunPanels[5]:=FunPanel_5;
  FunEdits[1]:=edRDAddress;
  FunEdits[2]:=edRDAmount;
  FunEdits[3]:=edWRAddress;
  FunEdits[4]:=edWRAmount;
  FunEdits[5]:=edValue;

  for c:=0 to 18 do
  with fun[c] do
  begin
    Info:='';
    EdLabels[1]:='Read Address';
    EdLabels[2]:='Read Amount';
    EdLabels[3]:='Write Address';
    EdLabels[4]:='Write Amount';
    EdLabels[5]:='Value';
    EdEnable[1]:=true;
    EdEnable[2]:=true;
    EdEnable[3]:=true;
    EdEnable[4]:=true;
    EdEnable[5]:=false;
  end;

  fun[0].Name:='Read Coils';
  with fun[0] do
  begin
    EdEnable[1]:=true;
    EdEnable[2]:=true;
    EdEnable[3]:=false;
    EdEnable[4]:=false;
  end;
  fun[1].Name:='Read Discrete Inputs';
  with fun[1] do
  begin
    EdEnable[1]:=true;
    EdEnable[2]:=true;
    EdEnable[3]:=false;
    EdEnable[4]:=false;
  end;
  fun[2].Name:='Read Holding Registers';
  with fun[2] do
  begin
    EdEnable[1]:=true;
    EdEnable[2]:=true;
    EdEnable[3]:=false;
    EdEnable[4]:=false;
  end;
  fun[3].Name:='Read Input Registers';
  with fun[3] do
  begin
    EdEnable[1]:=true;
    EdEnable[2]:=true;
    EdEnable[3]:=false;
    EdEnable[4]:=false;
  end;
  fun[4].Name:='Write Single Coil';
  with fun[4] do
  begin
    EdEnable[1]:=false;
    EdEnable[2]:=false;
    EdEnable[3]:=true;
    EdEnable[4]:=false;
    EdEnable[5]:=true;
  end;
  fun[5].Name:='Write Single Register';
  with fun[5] do
  begin
    EdEnable[1]:=false;
    EdEnable[2]:=false;
    EdEnable[3]:=true;
    EdEnable[4]:=false;
    EdEnable[5]:=true;
  end;
  fun[6].Name:='Write Multiple Coils';
  with fun[6] do
  begin
    EdEnable[1]:=false;
    EdEnable[2]:=false;
    EdEnable[3]:=true;
    EdEnable[4]:=true;
  end;
  fun[7].Name:='Write Multiple Registers';
  with fun[7] do
  begin
    EdEnable[1]:=false;
    EdEnable[2]:=false;
    EdEnable[3]:=true;
    EdEnable[4]:=true;
  end;
  fun[8].Name:='Mask Write Register';
  with fun[8] do
  begin
    EdEnable[1]:=false;
    EdEnable[2]:=false;
    EdEnable[3]:=true;
    EdEnable[4]:=true;
    EdEnable[5]:=true;
    EdLabels[4]:='AND Mask';
    EdLabels[5]:='OR Mask';
  end;
  fun[9].Name:='Read/Write Multiple Registers';
  with fun[9] do
  begin
    EdEnable[1]:=true;
    EdEnable[2]:=true;
    EdEnable[3]:=true;
    EdEnable[4]:=true;
  end;
  fun[10].Name:='Read File Record';
  with fun[10] do
  begin
    Info:='The registers received by the "Read File Record" function will be stored into the Holding Registers starting from offset 0 ';
    EdEnable[1]:=true;
    EdEnable[2]:=true;
    EdEnable[3]:=true;
    EdEnable[4]:=true;
    EdLabels[1]:='Ref. Type';
    EdLabels[2]:='File Number';
    EdLabels[3]:='Rec. Number';
    EdLabels[4]:='Regs Amount';
  end;
  fun[11].Name:='Write File Record';
  with fun[11] do
  begin
    Info:='The registers sent by the "Write File Record" function will be taken from the Holding Registers starting from offset 0 ';
    EdEnable[1]:=true;
    EdEnable[2]:=true;
    EdEnable[3]:=true;
    EdEnable[4]:=true;
    EdLabels[1]:='Ref. Type';
    EdLabels[2]:='File Number';
    EdLabels[3]:='Rec. Number';
    EdLabels[4]:='Regs Amount';
  end;

  fun[12].Name :='Read Exception Status';
  with fun[12] do
  begin
    EdEnable[1]:=false;
    EdEnable[2]:=false;
    EdEnable[3]:=false;
    EdEnable[4]:=false;
    EdEnable[5]:=false;
    EdLabels[1]:='';
    EdLabels[2]:='';
    EdLabels[3]:='';
    EdLabels[4]:='';
    EdLabels[5]:='';
  end;

  fun[13].Name :='Diagnostics';
  with fun[13] do
  begin
    Info:='The data received by the "Diagnostic" function will be stored into the Holding Registers starting from offset 0 ';
    EdEnable[1]:=true;
    EdEnable[2]:=true;
    EdEnable[3]:=false;
    EdEnable[4]:=false;
    EdEnable[5]:=false;
    EdLabels[1]:='Sub Fun';
    EdLabels[2]:='Data Out';
    EdLabels[3]:='';
    EdLabels[4]:='';
    EdLabels[5]:='';
  end;

  fun[14].Name:='Get Comm Event Counter';
  with fun[14] do
  begin
    EdEnable[1]:=false;
    EdEnable[2]:=false;
    EdEnable[3]:=false;
    EdEnable[4]:=false;
    EdEnable[5]:=false;
    EdLabels[1]:='';
    EdLabels[2]:='';
    EdLabels[3]:='';
    EdLabels[4]:='';
    EdLabels[5]:='';
  end;

  fun[15].Name:='Get Comm Event Log';
  with fun[15] do
  begin
    EdEnable[1]:=false;
    EdEnable[2]:=false;
    EdEnable[3]:=false;
    EdEnable[4]:=false;
    EdEnable[5]:=false;
    EdLabels[1]:='';
    EdLabels[2]:='';
    EdLabels[3]:='';
    EdLabels[4]:='';
    EdLabels[5]:='';
  end;

  fun[16].Name:='Report Server ID';
  with fun[16] do
  begin
    EdEnable[1]:=false;
    EdEnable[2]:=false;
    EdEnable[3]:=false;
    EdEnable[4]:=false;
    EdEnable[5]:=false;
    EdLabels[1]:='';
    EdLabels[2]:='';
    EdLabels[3]:='';
    EdLabels[4]:='';
    EdLabels[5]:='';
  end;

  fun[17].Name:='Read FIFO Queue';
  with fun[17] do
  begin
    Info:='The data received by the "Read FIFO Queue" function will be stored into the Holding Registers starting from offset 0 ';
    EdEnable[1]:=true;
    EdEnable[2]:=false;
    EdEnable[3]:=false;
    EdEnable[4]:=false;
    EdEnable[5]:=false;
    EdLabels[1]:='Address';
    EdLabels[2]:='';
    EdLabels[3]:='';
    EdLabels[4]:='';
    EdLabels[5]:='';
  end;

  fun[18].Name:='Encapsulated Interface Transport';
  with fun[18] do
  begin
    Info:='For more flexibility, consider using the "User Function" by passing 0x2B as the function number ';
    EdEnable[1]:=true;
    EdEnable[2]:=true;
    EdEnable[3]:=false;
    EdEnable[4]:=false;
    EdEnable[5]:=false;
    EdLabels[1]:='MEI Type';
    EdLabels[2]:='Data';
    EdLabels[3]:='';
    EdLabels[4]:='';
    EdLabels[5]:='';
  end;

  fun[19].Name:='User Function';
  with fun[19] do
  begin
    Info:='Setup your own PDU into "User Function" Tab ';
    EdEnable[1]:=false;
    EdEnable[2]:=false;
    EdEnable[3]:=false;
    EdEnable[4]:=false;
    EdEnable[5]:=false;
    EdLabels[1]:='';
    EdLabels[2]:='';
    EdLabels[3]:='';
    EdLabels[4]:='';
    EdLabels[5]:='';
  end;

  cbFunction.Items.clear;
  for c:=0 to 19 do
    cbFunction.Items.Add(fun[c].Name);
  cbFunction.ItemIndex:=0;

  cbDeviceID.Items.Clear;
  for c:=0 to 255 do
    cbDeviceID.Items.Add(IntToStr(c));
  cbDeviceID.ItemIndex:=1;

  UpdateEdits;
end;

procedure TMainForm.SetFConnected(AValue: boolean);
begin
  FConnected:=AValue;
  if FConnected then
  begin
    pnlConnection.BackGround.Color:=color_Connected;
    lblConnection.Caption:='Connected';
    pnlDataLink.BackGround.Color:=color_DataLinkActive;
    pnlChannel.BackGround.Color:=color_ChannelActive;
    btnConnection.ImageIndex:=1;
    btnConnection.Hint:='Disconnect';
  end
  else begin
    pnlConnection.BackGround.Color:=color_Disconnected;
    lblConnection.Caption:='Not Connected';
    pnlDataLink.BackGround.Color:=color_DataLinkOff;
    pnlChannel.BackGround.Color:=color_ChannelOff;
    btnConnection.ImageIndex:=0;
    btnConnection.Hint:='Connect';
  end;
end;

procedure TMainForm.UpdateSettingsPanels;
begin
  lblBrokerType.Caption:=Brokers[Settings.BrokerType];
  case Settings.BrokerType of
    btNetClient: begin
      pnlNetController.Visible:=false;
      pnlChannel.Visible:=true;
      lblDataLink.Caption:='Ethernet';
      lblFlavourLabel.Caption:='Protocol';
      lblFlavour.Caption:=NetProtocols[Settings.CliNetProto];
      lblLinkNameLabel.Caption:='Address';
      lblLinkName.Caption:=Settings.CliEthParams.Address;
      lblParamsLabel.Caption:='Port';
      lblParams.Caption:=IntToStr(Settings.CliEthParams.Port);
    end;
    btSerController: begin
      pnlNetController.Visible:=false;
      pnlChannel.Visible:=true;
      lblDataLink.Caption:='Serial';
      lblFlavourLabel.Caption:='Format';
      lblFlavour.Caption:='RTU/ASCII';
      lblLinkNameLabel.Caption:='Port';
      lblLinkName.Caption:=Settings.SerParams.Port;
      lblParamsLabel.Caption:='Params';
      with Settings.SerParams do
        lblParams.Caption:=SysUtils.Format('%d, %s, %d, %d',[BaudRate, Parity, DataBits, StopBits]);
    end;
  end;
end;

procedure TMainForm.UpdateEdits;
Var
  idx, c : integer;
begin
  idx:=cbFunction.itemIndex;
  for c:=1 to 5 do
  begin
    FunPanels[c].Caption:=fun[idx].EdLabels[c];
    if fun[idx].EdEnable[c] then
    begin
      FunEdits[c].Font.Color:=clBlack;
      FunEdits[c].Color:=clWhite;
    end
    else begin
      FunEdits[c].Font.Color:=clSilver;
      FunEdits[c].Color:=clSilver;
    end;
    FunEdits[c].Enabled:=fun[idx].EdEnable[c];
  end;
  lblFunInfo.Caption:=fun[idx].Info;

  if idx = 19 then
    PC.ActivePageIndex:=2;

  if cbDeviceID.ItemIndex = 0 then
    btnFunction.Enabled:= (Settings.BrokerType=btNetClient) or (idx in[4, 5, 6, 7, 11])
  else
    btnFunction.Enabled:= true;
end;

procedure TMainForm.BrokerChangeTo;
begin
  case Settings.BrokerType of
    btNetClient: begin
      Broker.ChangeTo(Settings.CliNetProto, Settings.CliEthParams.Address, Settings.CliEthParams.Port);
      Broker.SetLocalParam(par_DisconnectOnError, integer(Settings.CliEthParams.DisOnError));
      Broker.SetLocalParam(par_BaseAddressZero,integer(Settings.BaseAddressZero));
      Broker.SetLocalParam(par_MaxRetries, Settings.MaxRetries);
    end;
    btSerController: begin
      with Settings do
        Broker.ChangeTo(sfRTU, SerParams.Port, SerParams.BaudRate, SerParams.Parity, SerParams.DataBits, SerParams.StopBits, SerParams.Flow);
      Broker.SetLocalParam(par_BaseAddressZero,integer(Settings.BaseAddressZero));
      Broker.SetLocalParam(par_MaxRetries, Settings.MaxRetries);
    end;
  end;
  PC.ActivePageIndex:=0;
  UpdateEdits;
end;

procedure TMainForm.BrokerDestroy;
begin
  Broker.Disconnect;
  Broker.Free;
end;

function TMainForm.WordOf(Edit: TEdit): word;
Var
  MinVal, Value : Longint;
begin
  if Settings.BaseAddressZero then
    MinVal := 0
  else
    MinVal := 1;

  Value := StrToIntDef(Edit.Text, -1);
  if (Value>=MinVal) and (Value<$FFFF) then
    exit(Value);
  if Value<MinVal then Value := MinVal;
  if Value>$FFFF then Value:=$FFFF;
  Edit.Text:=IntToStr(Value);
  Result:=value;
end;

procedure TMainForm.DumpIOData(var IOBuffer: TIOBuffer);
Var
  sHeader : string;
  SHex, SChr : string;
  Ch : AnsiChar;
  c, cnt : integer;
begin
  if IOBuffer.Direction = PacketLog_IN then // Indication
    sHeader:=format('[Confirmation from Device %d]', [IOBuffer.Peer])
  else // Response
    sHeader:=format('[Request to Device %d]', [IOBuffer.Peer]);

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

procedure TMainForm.ClrPDUOut;
Var
  x,y: integer;
begin
  for y:=1 to gridPDUOut.RowCount-1 do
  begin
    gridPDUOut.Cells[0, y]:=inttostr((y-1)*32);
    for x:=1 to gridPDUOut.ColCount-1 do
      gridPDUOut.Cells[x,y]:='';
  end;
end;

procedure TMainForm.ClrPDUIn;
Var
  x,y: integer;
begin
  for y:=1 to gridPDUIn.RowCount-1 do
  begin
    gridPDUIn.Cells[0, y]:=inttostr((y-1)*32);
    for x:=1 to gridPDUIn.ColCount-1 do
      gridPDUIn.Cells[x,y]:='';
  end;
end;

function TMainForm.funReadCoils(DeviceID : byte): integer;
Var
  Address, AddrIn, Amount : word;
begin
  Address:=WordOf(edRDAddress);
  Amount :=WordOf(edRDAmount);

  if Settings.BaseAddressZero then
    AddrIn := Address
  else
    AddrIn := Address - 1;

  Result:=Broker.ReadCoils(DeviceID, Address, Amount, @Coils[AddrIn]);
  if Result = 0 then
    gridCoils.Invalidate;
end;

function TMainForm.funReadDiscreteInputs(DeviceID : byte): integer;
Var
  Address, AddrIn, Amount : word;
begin
  Address:=WordOf(edRDAddress);
  Amount :=WordOf(edRDAmount);

  if Settings.BaseAddressZero then
    AddrIn := Address
  else
    AddrIn := Address - 1;

  Result:=Broker.ReadDiscreteInputs(DeviceID, Address, Amount, @DiscreteInputs[AddrIn]);
  if Result = 0 then
    gridDiscrete.Invalidate;
end;

function TMainForm.funReadInputRegisters(DeviceID : byte): integer;
Var
  Address, AddrIn, Amount : word;
begin
  Address:=WordOf(edRDAddress);
  Amount :=WordOf(edRDAmount);

  if Settings.BaseAddressZero then
    AddrIn := Address
  else
    AddrIn := Address - 1;

  Result:=Broker.ReadInputRegisters(DeviceID, Address, Amount, @InputRegisters[AddrIn]);
  if Result = 0 then
    InvalidateRegGrid(gridInputRegs,InputRegisters);
end;

function TMainForm.funReadHoldingRegisters(DeviceID : byte): integer;
Var
  Address, AddrIn, Amount : word;
begin
  Address:=WordOf(edRDAddress);
  Amount :=WordOf(edRDAmount);

  if Settings.BaseAddressZero then
    AddrIn := Address
  else
    AddrIn := Address - 1;

  Result:=Broker.ReadHoldingRegisters(DeviceID, Address, Amount, @HoldingRegisters[AddrIn]);
  if Result = 0 then
    InvalidateRegGrid(gridHoldingRegs,HoldingRegisters);
end;

function TMainForm.funWriteSingleCoil(DeviceID : byte): integer;
Var
  Address, Value  : word;
begin
  Address:=wordOf(edWRAddress);
  Value:=StrToIntDef(edValue.Text, 0);
  Result:=Broker.WriteSingleCoil(DeviceID, Address, Value);
end;

function TMainForm.funWriteSingleRegister(DeviceID : byte): integer;
Var
  Address, Value  : word;
begin
  Address:=wordOf(edWRAddress);
  Value:=StrToIntDef(edValue.Text, 0);
  Result:=Broker.WriteSingleRegister(DeviceID, Address, Value);
end;

function TMainForm.funWriteMultipleCoils(DeviceID : byte): integer;
Var
  Address, AddrOut, Amount : word;
begin
  Address:=wordOf(edWRAddress);
  Amount :=wordOf(edWRAmount);

  if Settings.BaseAddressZero then
    AddrOut := Address
  else
    AddrOut := Address - 1;

  Result:=Broker.WriteMultipleCoils(DeviceID,Address,Amount,@Coils[AddrOut]);
end;

function TMainForm.funWriteMultipleRegisters(DeviceID : byte): integer;
Var
  Address, AddrOut, Amount : word;
begin
  Address:=wordOf(edWRAddress);
  Amount :=wordOf(edWRAmount);

  if Settings.BaseAddressZero then
    AddrOut := Address
  else
    AddrOut := Address - 1;

  Result:=Broker.WriteMultipleRegisters(DeviceID,Address,Amount,@HoldingRegisters[AddrOut]);
end;

function TMainForm.funMaskWriteRegisters(DeviceID : byte): integer;
Var
  Address, AND_Mask, OR_Mask : word;
begin
  Address :=wordOf(edWRAddress);
  AND_Mask:=StrToIntDef(FunEdits[4].Text, 0);
  OR_Mask :=StrToIntDef(FunEdits[5].Text, 0);
  Result:=Broker.MaskWriteRegister(DeviceID, Address, AND_Mask, OR_Mask);
end;

function TMainForm.funReadWriteMultipleRegisters(DeviceID : byte): integer;
Var
  RDAddress, WRAddress, RDAmount, WRAmount : word;
  AddrIn, AddrOut : word;
begin
  RDAddress:=wordOf(edRDAddress);
  RDAmount :=wordOf(edRDAmount);
  WRAddress:=wordOf(edWRAddress);
  WRAmount :=wordOf(edWRAmount);

  if Settings.BaseAddressZero then
  begin
    AddrIn  := RDAddress;
    AddrOut := WRAddress;
  end
  else begin
    AddrIn  := RDAddress - 1;
    AddrOut := WRAddress - 1;
  end;

  Result:=Broker.ReadWriteMultipleRegisters(DeviceID, RDAddress, RDAmount, WRAddress, WRAmount, @HoldingRegisters[AddrIn], @HoldingRegisters[AddrOut]);
  if Result = 0 then
    InvalidateRegGrid(gridHoldingRegs,HoldingRegisters);
end;

function TMainForm.funReadFileRecord(DeviceID : byte): integer;
Var
  RefType  : byte;
  FileNumber, RecNumber, RegsAmount : word;
begin
  RefType   :=StrToIntDef(FunEdits[1].Text, 0);
  FileNumber:=StrToIntDef(FunEdits[2].Text, 0);
  RecNumber :=StrToIntDef(FunEdits[3].Text, 0);
  RegsAmount:=StrToIntDef(FunEdits[4].Text, 0);

  Result:=Broker.ReadFileRecord(DeviceID, RefType, FileNumber, RecNumber, RegsAmount, @HoldingRegisters[0]);
  if Result = 0 then
    InvalidateRegGrid(gridHoldingRegs,HoldingRegisters);
end;

function TMainForm.funWriteFileRecord(DeviceID : byte): integer;
Var
  RefType  : byte;
  FileNumber, RecNumber, RegsAmount : word;
begin
  RefType   :=StrToIntDef(FunEdits[1].Text, 0);
  FileNumber:=StrToIntDef(FunEdits[2].Text, 0);
  RecNumber :=StrToIntDef(FunEdits[3].Text, 0);
  RegsAmount:=StrToIntDef(FunEdits[4].Text, 0);

  Result:=Broker.WriteFileRecord(DeviceID, RefType, FileNumber, RecNumber, RegsAmount, @HoldingRegisters[0]);
end;

function TMainForm.funReadExceptionStatus(DeviceID : byte): integer;
Var
  Data : byte;
begin
  Result:=Broker.ReadExceptionStatus(DeviceID, Data);
  if Result = 0 then
  begin
    SpecialFunDump.Lines.Add('[Function Read Exception Status]');
    SpecialFunDump.Lines.Add(format('Return Value : 0x%s',[IntToHex(Data,2)]));
    PC.ActivePageIndex:=1;
  end;
end;

function TMainForm.funDiagnostics(DeviceID : byte): integer;
Var
  SubFun, Data : word;
  Received : word;
begin
  SubFun   :=StrToIntDef(FunEdits[1].Text, 0);
  Data     :=StrToIntDef(FunEdits[2].Text, 0);

  Result:=Broker.Diagnostics(DeviceID, SubFun, @Data, @HoldingRegisters[0], 1, Received);
  if Result = 0 then
  begin
    InvalidateRegGrid(gridHoldingRegs,HoldingRegisters);
    SpecialFunDump.Lines.Add('[Function Diagnostics]');
    SpecialFunDump.Lines.Add(format('SubFun : 0x%s, Data : 0x%s',[IntToHex(SubFun,2),IntToHex(Data,4)]));
    SpecialFunDump.Lines.Add(format('Received %d items stored @HoldingRegisters[0]',[Received]));
    PC.ActivePageIndex:=1;
  end;
end;

function TMainForm.funGetCommEventCounter(DeviceID : byte): integer;
Var
  Status, EventCount : word;
begin
  Result:=Broker.GetCommEventCounter(DeviceID, Status, EventCount);
  if Result = 0 then
  begin
    SpecialFunDump.Lines.Add('[Function Get Comm Event Counter]');
    SpecialFunDump.Lines.Add(format('Received - Status : 0x%s, Event Count : 0x%s',[IntToHex(Status,4),IntToHex(EventCount,4)]));
    PC.ActivePageIndex:=1;
  end;
end;

function TMainForm.funGetCommEventLog(DeviceID : byte): integer;
Var
  Status : word;
  EventCount : word;
  MessageCount : word;
  NumItems : word;
  Events : packed array[0..255] of byte;
  c : integer;
begin
  Result:=Broker.GetCommEventLog(DeviceID, Status, EventCount, MessageCount, NumItems, @Events);
  if Result = 0 then
  begin
    SpecialFunDump.Lines.Add('[Function Get Comm Event Log]');
    SpecialFunDump.Lines.Add(format('Received - Status : 0x%s, Evt Count : %d, Msg Count : %d, Num Items : %d',[IntToHex(Status,4),EventCount, MessageCount, NumItems]));
    SpecialFunDump.Lines.Add('Items[]');
    if NumItems>256 then NumItems:=256;
    for c:=0 to NumItems-1 do
      SpecialFunDump.Lines.Add(' (%3d) : 0x%s',[c, IntToHex(Events[c])]);
    PC.ActivePageIndex:=1;
  end;
end;

function TMainForm.funReportServerID(DeviceID : byte): integer;
Var
  DataSize : integer;
  c : integer;
  ServerID : packed array[0..255] of AnsiChar;
  S : string;
begin
  Result:=Broker.ReportServerID(DeviceID, @ServerID, DataSize);
  if Result = 0 then
  begin
    SpecialFunDump.Lines.Add('[Function Report Server ID]');
    SpecialFunDump.Lines.Add('%d byte received',[DataSize]);
    S:='';
    for c:=0 to DataSize-1 do
      if ServerID[c] in ['0'..'9','a'..'z','A'..'Z',' ','_'] then
        S:=S+ServerID[c]
      else
        S:=S+Format('(0x%s)',[IntToHex(ord(ServerID[c]),2)]);
    SpecialFunDump.Lines.Add(S);
    PC.ActivePageIndex:=1;
  end;
end;

function TMainForm.funReadFIFOQueue(DeviceID : byte): integer;
Var
  Address : word;
  FifoCount : word;
begin
  Address :=wordOf(edWRAddress);
  Result:=Broker.ReadFIFOQueue(DeviceID, Address, FifoCount, @HoldingRegisters[0]);
  if Result = 0 then
  begin
    SpecialFunDump.Lines.Add('[Function Read FIFO Queue]');
    SpecialFunDump.Lines.Add('%d Items received, stored starting from HoldingRegisters[0]',[FifoCount]);
    PC.ActivePageIndex:=1;
  end;
end;

function TMainForm.funEncapsulatedInterfaceTransport(DeviceID : byte): integer;
Var
  MEI_Type : byte;
  DataOut  : word;
  RdSize   : word;
  c        : integer;
  S        : string;
  DataIN   : packed array[0..255] of AnsiChar;
begin
  MEI_Type:=StrToIntDef(FunEdits[1].Text, 0);
  DataOut :=StrToIntDef(FunEdits[2].Text, 0);
  Result:=Broker.ExecuteMEIFunction(DeviceID, MEI_Type, @DataOut, 2, @DataIn, RDSize);
  if Result = 0 then
  begin
    SpecialFunDump.Lines.Add('[Function Encapsulated Interface Transport (MEI)]');
    SpecialFunDump.Lines.Add('%d byte received',[RDSize]);
    S:='';
    for c:=0 to RDSize-1 do
      if DataIN[c] in ['0'..'9','a'..'z','A'..'Z',' ','_'] then
        S:=S+DataIN[c]
      else
        S:=S+Format('(0x%s)',[IntToHex(ord(DataIN[c]),2)]);
    SpecialFunDump.Lines.Add(S);
    PC.ActivePageIndex:=1;
  end;
end;

function TMainForm.funUserFunction(DeviceID : byte): integer;
Var
  UsrFunction : byte;
  SizePDUWrite : integer;
  SizePDURead : word;
  PDUOut, PDUIn : packed array[0..255] of byte;


  function GetValue(x,y : integer) : integer;
  begin
    if cbFormatPDU.ItemIndex=0 then
      Result:=StrToIntDef(gridPDUOut.Cells[x,y],-1)
    else
      Result:=StrToIntDef('$'+gridPDUOut.Cells[x,y],-1);

    if (Result<0) or (Result>255) then
      Result:=-1;
  end;

  function FillPDUOut : integer;
  Var
    c,x,y,val : integer;
  begin
    c:=-1;
    for y:=1 to gridPDUOut.ColCount-1 do
    begin
      for x:=1 to gridPDUOut.RowCount-1 do
      begin
        if trim(gridPDUOut.Cells[x,y])='' then
          exit(c+1);

        val:=GetValue(x,y);
        if val=-1 then
        begin
          gridPDUOut.Col:=x;
          gridPDUOut.Row:=y;
          messageDlg('Invalid byte value', mtError, [mbOk], 0);
          exit(-1);
        end;

        inc(c);
        PDUOut[c]:=val;

      end;
    end;
    Result:=c+1;
  end;

  procedure PutValue(x,y,value : integer);
  begin
    if cbFormatPDU.ItemIndex=0 then
      gridPDUIn.Cells[x,y]:=IntToStr(value)
    else
      gridPDUIn.Cells[x,y]:=IntToHex(value,2);
  end;

  procedure FillPDUIn(SizeRead : integer);
  Var
    c, x, y : integer;
  begin
    for c:=0 to SizeRead-1 do
    begin
      x:=(c mod 32)+1;
      y:=(c div 32)+1;
      PutValue(x,y,PDUIn[c]);
    end;
  end;

begin
  Result:=0;

  UsrFunction:=byte(StrToIntDef(edUsrFunction.Text, 0));
  edUsrFunction.Text:=IntToStr(UsrFunction);
  if (UsrFunction<=0) then
  begin
    messageDlg('Invalid Function Number (Must be in [1..255])', mtError, [mbOk], 0);
    exit;
  end;

  SizePDUWrite:=FillPDUOut;
  if SizePDUWrite=-1 then
    exit;
  if SizePDUWrite=0 then
  begin
    messageDlg('PDU is Empty', mtError, [mbOk], 0);
    exit;
  end;
  if SizePDUWrite>MaxBinPDUSize then
  begin
    messageDlg('PDU Size exceeds 253 byte', mtError, [mbOk], 0);
    exit;
  end;
  ClrPDUIn;

  Result:=Broker.CustomFunctionRequest(DeviceID, UsrFunction, @PDUOut, SizePDUWrite, @PDUIn, SizePDURead,0);
  if Result = 0 then
  begin
    if SizePDURead>256 then SizePDURead:=256;
    FillPDUIn(SizePDURead);
  end;

end;

procedure TMainForm.WmExecuteFun(var Msg: TLMessage);
var
  IOBuffer: TIOBuffer;
  Size : integer;
  DeviceID : byte;
begin
  DeviceID:=byte(Msg.lParam);

  SB.Panels[0].Text:='';
  SB.Panels[1].Text:='';
  case Msg.wParam of
    0: funReadCoils(DeviceID);
    1: funReadDiscreteInputs(DeviceID);
    2: funReadHoldingRegisters(DeviceID);
    3: funReadInputRegisters(DeviceID);
    4: funWriteSingleCoil(DeviceID);
    5: funWriteSingleRegister(DeviceID);
    6: funWriteMultipleCoils(DeviceID);
    7: funWriteMultipleRegisters(DeviceID);
    8: funMaskWriteRegisters(DeviceID);
    9: funReadWriteMultipleRegisters(DeviceID);
    10:funReadFileRecord(DeviceID);
    11:funWriteFileRecord(DeviceID);
    12:funReadExceptionStatus(DeviceID);
    13:funDiagnostics(DeviceID);
    14:funGetCommEventCounter(DeviceID);
    15:funGetCommEventLog(DeviceID);
    16:funReportServerID(DeviceID);
    17:funReadFIFOQueue(DeviceID);
    18:funEncapsulatedInterfaceTransport(DeviceID);
    19:funUserFunction(DeviceID);
  end;
  ActionInfo(Msg.wParam);
  if Broker.GetBufferSent(@IOBuffer.Data, Size) then
  begin
    IOBuffer.Size:=Size;
    IOBuffer.Direction:=PacketLog_OUT;
    IoBuffer.Peer:=DeviceID;
    DumpIOData(IOBuffer);
  end;
  if (DeviceID>0) and Broker.GetBufferRecv(@IOBuffer.Data, Size) then
  begin
    IOBuffer.Size:=Size;
    IOBuffer.Direction:=PacketLog_IN;
    IoBuffer.Peer:=DeviceID;
    DumpIOData(IOBuffer);
  end;
end;

end.

