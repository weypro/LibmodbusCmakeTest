unit frmDevsettings;

{$mode ObjFPC}{$H+}

interface

uses
  Classes, SysUtils, Forms, Controls, Graphics, Dialogs, ExtCtrls, ComCtrls,
  StdCtrls, BCButton, SnapMB;

const
  ParityChar : array [0..2] of char = ('N','E','O');

  LinuxPorts : array[0..37] of string=(
  '/dev/ttyS0','/dev/ttyS1','/dev/ttyS2','/dev/ttyS3','/dev/ttyS4','/dev/ttyS5',
  '/dev/ttyS6','/dev/ttyS7','/dev/ttyS8','/dev/ttyS9','/dev/ttyS10','/dev/ttyS11',
  '/dev/ttyS12','/dev/ttyS13','/dev/ttyS14','/dev/ttyS15','/dev/ttyUSB0',
  '/dev/ttyUSB1','/dev/ttyUSB2','/dev/ttyUSB3','/dev/ttyUSB4','/dev/ttyUSB5',
  '/dev/ttyAMA0','/dev/ttyAMA1','/dev/ttyACM0','/dev/ttyACM1',
  '/dev/rfcomm0','/dev/rfcomm1','/dev/ircomm0','/dev/ircomm1',
  '/dev/cuau0','/dev/cuau1','/dev/cuau2','/dev/cuau3',
  '/dev/cuaU0','/dev/cuaU1','/dev/cuaU2','/dev/cuaU3');


type

  TMBDeviceSetting = record
     DeviceID    : byte;
     DataLink    : TMBDataLink;
     NetProto    : TMBNetProto;
     SerFormat   : TMBSerialFormat;
     SerIframe   : integer;
     EthParams   : TMBEthParams;
     SerParams   : TMBSerParams;
     ClrOnCreate : boolean;
     SerialOnEth : boolean;
  end;

  { TSettingsForm }

  TSettingsForm = class(TForm)
    btnCancel: TBCButton;
    btnAccept: TBCButton;
    btnBack: TBCButton;
    cbBaudRate: TComboBox;
    cbDataBits: TComboBox;
    cbDataLink: TComboBox;
    cbDeviceID: TComboBox;
    cbProtocol: TComboBox;
    cbFlow: TComboBox;
    cbParity: TComboBox;
    cbPort: TComboBox;
    cbFormat: TComboBox;
    cbStopBits: TComboBox;
    chkClrResources: TCheckBox;
    chkSerialOnEth: TCheckBox;
    edAddress: TEdit;
    edIFrame: TEdit;
    edPort: TEdit;
    edDisTimeout: TEdit;
    gbSerial: TGroupBox;
    gbEthernet: TGroupBox;
    Label1: TLabel;
    Label10: TLabel;
    Label11: TLabel;
    Label14: TLabel;
    Label15: TLabel;
    Label16: TLabel;
    lblError: TLabel;
    Label12: TLabel;
    Label13: TLabel;
    Label2: TLabel;
    Label3: TLabel;
    Label4: TLabel;
    Label5: TLabel;
    Label6: TLabel;
    Label7: TLabel;
    Label8: TLabel;
    Label9: TLabel;
    PC: TPageControl;
    Panel1: TPanel;
    Panel2: TPanel;
    pnlDrop: TPanel;
    TabSheet1: TTabSheet;
    TabSheet2: TTabSheet;
    procedure btnAcceptClick(Sender: TObject);
    procedure btnBackClick(Sender: TObject);
    procedure btnCancelClick(Sender: TObject);
    procedure cbDataLinkCloseUp(Sender: TObject);
    procedure cbProtocolCloseUp(Sender: TObject);
    procedure FormCreate(Sender: TObject);
    procedure FormShow(Sender: TObject);
    procedure gbSerialClick(Sender: TObject);
    procedure PageControl2Change(Sender: TObject);
  private
    procedure InitComboBoxes;
    procedure UpdateGroupBoxes;
  public
    function EditSettings(var Settings : TMBDeviceSetting) : boolean;
    function CheckSettings : boolean;
    procedure SettingsToForm(var Settings : TMBDeviceSetting);
    procedure FormToSettings(var Settings : TMBDeviceSetting);
  end;

var
  SettingsForm: TSettingsForm;

procedure SetDefaults(var Settings : TMBDeviceSetting);

implementation

{$R *.lfm}

procedure SetDefaults(var Settings : TMBDeviceSetting);
begin
  Settings.DataLink:=dlEthernet;
  Settings.DeviceID:=1;
  Settings.EthParams.Address:='127.0.0.1';
  Settings.EthParams.Port:=502;
  Settings.EthParams.DisTimeout:=0;
  Settings.NetProto:=mbTCP;
  Settings.SerParams.Port:='COM1';
  Settings.SerParams.BaudRate:=19200;
  Settings.SerParams.Parity:='E';
  Settings.SerParams.DataBits:=8;
  Settings.SerParams.StopBits:=1;
  Settings.SerParams.Flow:=flowNone;
  Settings.SerFormat:=sfRTU;
  Settings.SerIframe:=50;
  Settings.ClrOnCreate:=false;
  Settings.SerialOnEth:=true;
end;

{ TSettingsForm }

procedure TSettingsForm.PageControl2Change(Sender: TObject);
begin

end;

procedure TSettingsForm.btnAcceptClick(Sender: TObject);
begin
  if CheckSettings then
    ModalResult:=mrOK;
end;

procedure TSettingsForm.btnBackClick(Sender: TObject);
begin
  ModalResult:=mrCancel;
end;

procedure TSettingsForm.btnCancelClick(Sender: TObject);
begin
  ModalResult:=mrCancel;
end;

procedure TSettingsForm.cbDataLinkCloseUp(Sender: TObject);
begin
  UpdateGroupBoxes;
end;

procedure TSettingsForm.cbProtocolCloseUp(Sender: TObject);
begin
  pnlDrop.Visible:=cbProtocol.ItemIndex in [0,2];
end;

procedure TSettingsForm.FormCreate(Sender: TObject);
begin
  InitComboBoxes;
end;

procedure TSettingsForm.FormShow(Sender: TObject);
begin
  lblError.Caption:='';
  Position:=poScreenCenter;
  PC.ActivePageIndex:=0;
end;

procedure TSettingsForm.gbSerialClick(Sender: TObject);
begin

end;

procedure TSettingsForm.InitComboBoxes;
Var
  c : integer;
begin
  cbDeviceID.Items.Clear;
  for c:=0 to 255 do
    cbDeviceID.Items.Add(IntToStr(c));
  cbDeviceID.ItemIndex:=1;
  cbPort.Items.Clear;
  {$IFDEF MSWINDOWS}
  for c:=1 to 64 do
    cbPort.Items.Add('COM'+IntToStr(c));
  cbPort.ItemIndex:=0;
  {$ELSE}
  for c:=0 to 37 do
    cbPort.Items.Add(LinuxPorts[C]);
  {$ENDIF}
  cbPort.ItemIndex:=0;
end;

procedure TSettingsForm.UpdateGroupBoxes;
begin
  if cbDataLink.ItemIndex=0 then
  begin
    gbSerial.Visible:=false;
    gbEthernet.Visible:=true;
  end
  else begin
    gbEthernet.Visible:=false;
    gbSerial.Visible:=true;
  end;
end;

function TSettingsForm.EditSettings(var Settings: TMBDeviceSetting): boolean;
begin
  SettingsToForm(Settings);
  if ShowModal = mrOk then
  begin
    FormToSettings(Settings);
    Result:=true;
  end
  else
    Result:=false;
end;

function TSettingsForm.CheckSettings: boolean;
Var
  port : integer;
  deviceid : integer;
begin
  Result:=false;
  if cbDataLink.ItemIndex = -1 then
  begin
    lblError.Caption:='Invalid Data Link';
    exit;
  end;
  if cbDeviceID.ItemIndex = -1 then
  begin
    lblError.Caption:='Invalid Device ID';
    exit;
  end;
  if trim(edAddress.Text) = '' then
  begin
    lblError.Caption:='Invalid IP Address';
    exit;
  end;
  port := StrToIntDef(edPort.Text, -1);
  if (port < 1) or (port > $FFFF) then
  begin
    lblError.Caption:='Invalid IP Port (see Ethernet settings)';
    exit;
  end;
  if cbProtocol.ItemIndex = -1 then
  begin
    lblError.Caption:='Invalid Protocol (see Ethernet settings)';
    exit;
  end;
  if (cbPort.ItemIndex=-1) or
     (cbBaudRate.ItemIndex=-1) or
     (cbParity.ItemIndex=-1) or
     (cbDataBits.ItemIndex=-1) or
     (cbStopBits.ItemIndex=-1) or
     (cbFlow.ItemIndex=-1) or
     (cbFormat.ItemIndex=-1) then
  begin
    lblError.Caption:='Invalid Serial settings';
    exit;
  end;
  Result:=true;
end;

procedure TSettingsForm.SettingsToForm(var Settings: TMBDeviceSetting);
Var
  ch : char;
begin
  cbDeviceID.ItemIndex:=Settings.DeviceID;
  cbDataLink.ItemIndex:=ord(Settings.DataLink);
  // Ethernet
  edAddress.Text:=Settings.EthParams.Address;
  edPort.Text:=IntToStr(Settings.EthParams.Port);
  cbProtocol.ItemIndex:=ord(Settings.NetProto);
  edDisTimeout.Text:=IntToStr(Settings.EthParams.DisTimeout);
  // Serial
  cbPort.ItemIndex:=cbPort.Items.IndexOf(Settings.SerParams.Port);
  cbBaudRate.ItemIndex:=cbBaudRate.Items.IndexOf(IntToStr(Settings.SerParams.BaudRate));
  edIFrame.text:=IntToStr(Settings.SerIframe);

  ch:=UpCase(Settings.SerParams.Parity);
  if ch='N' then cbParity.ItemIndex:=0 else
  if ch='E' then cbParity.ItemIndex:=1 else
  if ch='O' then cbParity.ItemIndex:=2 else
    cbParity.ItemIndex:=0;

  cbDataBits.ItemIndex:=cbDataBits.Items.IndexOf(IntToStr(Settings.SerParams.DataBits));
  cbStopBits.ItemIndex:=cbStopBits.Items.IndexOf(IntToStr(Settings.SerParams.StopBits));
  cbFlow.ItemIndex:=ord(Settings.SerParams.Flow);
  cbFormat.ItemIndex:=Ord(Settings.SerFormat);
  pnlDrop.Visible:=Settings.NetProto in[mbTCP, mbRTUOverTCP];

  chkClrResources.Checked:=Settings.ClrOnCreate;
  chkSerialOnEth.Checked:=Settings.SerialOnEth;

  UpdateGroupBoxes;
end;

procedure TSettingsForm.FormToSettings(var Settings: TMBDeviceSetting);
begin
  Settings.DeviceID:=cbDeviceID.ItemIndex;
  Settings.DataLink:=TMBDataLink(cbDataLink.ItemIndex);
  // Ethernet
  Settings.EthParams.Address:=edAddress.Text;
  Settings.EthParams.Port:=StrToIntDef(edPort.Text, 502);
  Settings.EthParams.DisTimeout:=StrToIntDef(edDisTimeout.Text, 0);
  Settings.NetProto:=TMBNetProto(cbProtocol.ItemIndex);
  // Serial
  Settings.SerParams.Port:=cbPort.Text;
  Settings.SerParams.BaudRate:=StrToIntDef(cbBaudRate.Text,19200);
  Settings.SerParams.Parity:=ParityChar[cbParity.ItemIndex];
  Settings.SerParams.DataBits:=StrToIntDef(cbDataBits.Text,8);
  Settings.SerParams.StopBits:=StrToIntDef(cbStopBits.Text,1);
  Settings.SerParams.Flow:=TMBSerialFlow(cbFlow.ItemIndex);
  Settings.SerFormat:=TMBSerialFormat(cbFormat.ItemIndex);
  Settings.SerIframe:=StrToIntDef(edIFrame.text, 20);
  Settings.ClrOnCreate:=chkClrResources.Checked;
  Settings.SerialOnEth:=chkSerialOnEth.Checked;
end;

end.

