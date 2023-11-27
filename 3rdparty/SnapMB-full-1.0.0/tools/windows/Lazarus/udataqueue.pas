unit uDataQueue;
{$MODE DELPHI}

interface

uses
  Classes, SysUtils;

type

TByteArray = packed array[0..0] of byte;
PByteArray = ^TByteArray;

TDataQueue = class(TObject)
private
  IndexIn    : integer;   // <-- insert index
  IndexOut   : integer;   // --> extract index
  Max        : integer;   // Buffer upper bound [0..Max]
  FCapacity  : integer;   // Queue capacity
  Buffer     : PByteArray;
  FBlockSize : integer;
public
  constructor Create(const Capacity: integer; BlockSize : integer);
  destructor Destroy; override;
  procedure Flush;
  procedure Insert(lpdata : pointer);
  function Extract(lpdata : pointer) : boolean;
  function Empty : boolean;
end;


implementation

{ TDataQueue }

constructor TDataQueue.Create(const Capacity: integer; BlockSize : integer);
begin
  inherited Create;
  FCapacity:=Capacity;
  Max :=FCapacity-1;
  FBlockSize:=BlockSize;
  GetMem(Buffer,FCapacity*FBlockSize);
end;

destructor TDataQueue.Destroy;
begin
  FreeMem(Buffer,FCapacity*FBlockSize);
  inherited;
end;

function TDataQueue.Empty: boolean;
begin
  Result:=IndexIn=IndexOut;
end;

function TDataQueue.Extract(lpdata : pointer): boolean;
Var
  Offset : integer;
  IdxOut : integer;
begin
  Result:=not Empty;
  if Result then
  begin
    // Calc offset
    IdxOut:=indexOut;
    if IdxOut<Max then inc(IdxOut) else IdxOut:=0;
    Offset:=IdxOut*FBlockSize;
    // moves data
    {$R-}
    move(Buffer^[Offset],lpData^,FBlockSize);
    {$R+}
    // Updates IndexOut
    IndexOut:=IdxOut;
  end;
end;

procedure TDataQueue.Flush;
begin
  IndexIn :=0;
  IndexOut:=0;
end;

procedure TDataQueue.Insert(lpdata : pointer);
Var
  idxOut : integer;
  Offset : integer;
begin
  idxOut:=IndexOut; // To avoid that indexout may change during next line
  if not ((IdxOut=IndexIn+1) or ((IndexIn=Max) and (IdxOut=0))) then // if not full
  begin
    // Calc offset
    if IndexIn<Max then inc(IndexIn) else IndexIn:=0;
    Offset:=IndexIn*FBlockSize;
    {$R-}
    move(lpData^,Buffer^[Offset],FBlockSize);
    {$R+}
  end;
end;


end.

