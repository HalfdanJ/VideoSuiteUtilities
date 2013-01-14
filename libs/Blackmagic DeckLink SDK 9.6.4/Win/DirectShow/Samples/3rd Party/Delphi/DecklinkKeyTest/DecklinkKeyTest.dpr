program DecklinkKeyTest;

uses
  Forms,
  TfrmMainRawUnit in 'TfrmMainRawUnit.pas' {frmMainRaw};

{$R *.res}

begin
  Application.Initialize;
  Application.CreateForm(TfrmMainRaw, frmMainRaw);
  Application.Run;
end.
