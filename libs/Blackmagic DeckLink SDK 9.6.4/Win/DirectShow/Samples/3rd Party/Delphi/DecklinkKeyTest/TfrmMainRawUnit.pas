unit TfrmMainRawUnit;
{ This program is a *very* simple example of interfacing with a
  Blackmagic Design Decklink card without using DSPack... almost.
  DSUtil.pas is referenced for the TPin* classes.  Apart from that,
  CoCreateInstance() is used to build the graph.
  Generally, it would make sense to use DSPack (sourceforge.net/projects/dspack/
  and www.progdigy.com) since that encapsulates many DS internals.
  eg. Message events.  The sister program to this uses DSPack and
  is perhaps a better model to start with.  It also gives an example
  of creating a custom bitmap with alpha channel.


  Purpose: This program is trivial!!  Its' intention is to provide a starting
           point for Delphi developers wanting to communicate with the DL card.
           The chosen function here is the keying function of static bitmap images.

           The graph:

             Decklink Still Source Filter -> Decklink Video Renderer

           (I did say it was trivial!)



  Operation: 1) Check the Decklink.dll is registered
                If not, run register.bat in Samples\bin folder
             2) Copy bmps from Samples\DecklinkKey\res to this EXE's folder
             3) Run program
                3.1 Click [Prepare], then [Play]
                3.2 Select an image to overlay.
                3.3 Key option (eg. Internal)
                    NB External option not relevant to this sample.
                3.4 Change key slider
                 
                The [Stop] button is optional. It stops the graph
                which would be noticed if we had an AVI running.
                As it is now, the image will appear frozen (but can still be keyed)
                until [Play] is pressed again.

             The two BMPs supplied with the Decklink Keyer app should be located
             in this directory - the BMD logo and checker image.


  Desirable enhancements:
             1) Incorporate AVI mixing (similar to the Decklink Keyer sample)
             2) Dynamic bitmap update (ie. not creating a BMP file)
             3) Selection of DL hardware (if multiple cards installed)
             4) More complexe graph/  E.g An Infinite Tee filter and preview VMR.
             5) Loading filters using monikers for a more flexible design
             6) Interlaced scrolling text


  Platform:  Original code developed on BDS2006 and tested with a Decklink SP card (SDI).
             Should be backward compatible with Delphi 7.

  Code donated to Blackmagic Design by Ian Krigsman, www.discoverysystems.com.au

  Change History:
             May 05, 2007 - Initial release using BMD drivers v6.0 and SDK v5.8
}


interface

uses
  Windows
 ,Messages
 ,SysUtils
 ,Variants
 ,Classes
 ,Graphics
 ,Controls
 ,Forms
 ,Dialogs
 ,StdCtrls
 ,ExtCtrls
 ,ImgList
 ,ComCtrls
 ,ToolWin
 ,DSUtil
 ,DirectShow9
 ,Decklink
 ,ActnList
 ,Buttons
;

type
  TfrmMainRaw = class(TForm)
    ActionList            : TActionList;
    AlphaTrackBar         : TTrackBar;
    Bevel1                : TBevel;
    BmpBMDAction          : TAction;
    BmpBMDRadioButton     : TRadioButton;
    BmpCheckerAction      : TAction;
    BmpCheckerRadioButton : TRadioButton;
    ImageList             : TImageList;
    KeyRadioGroup         : TRadioGroup;
    Label2                : TLabel;
    OverlayGroupBox       : TGroupBox;
    PlayAction            : TAction;
    PrepareAction         : TAction;
    PrepareButton         : TBitBtn;
    RunButton             : TBitBtn;
    StopAction            : TAction;
    StopButton            : TBitBtn;
    procedure KeyRadioGroupClick(Sender: TObject);
    procedure FormDestroy(Sender: TObject);
    procedure PrepareActionExecute(Sender: TObject);
    procedure PlayActionExecute(Sender: TObject);
    procedure ActionListUpdate(Action: TBasicAction; var Handled: Boolean);
    procedure AlphaTrackBarChange(Sender: TObject);
    procedure StopActionExecute(Sender: TObject);
    procedure BmpBMDActionExecute(Sender: TObject);
    procedure BmpCheckerActionExecute(Sender: TObject);
  private
    { Private declarations }
    fFilterGraph  : IGraphBuilder;
    fDecklinkKeyer: IDecklinkKeyer;
    fDecklinkStillSource : IBaseFilter;
//    fCaptureGraph : ICaptureGraphBuilder2;
//    fInfinitePin  : IBaseFilter;
    fDecklinkVR   : IBaseFilter;
    fMediaControl : IMediaControl;
    fFileSource   : IFileSourceFilter;
    procedure Connect(AOutFilter, AInFilter: IBaseFilter);
public
    { Public declarations }
  end;

var
  frmMainRaw: TfrmMainRaw;

const
{ Original code set for PAL.  Reverse IMAGEHEIGHT assignment for NTSC.
  Possible enhancement: Determine the metrics of the DL renderer and set
  the video type automatically. }

// Images from Samples\DecklinkKeyer\res folder (copy to this EXE folder)
  sOverlayBMPName1   = 'Blackmagic Design PAL.bmp';
  sOverlayBMPName2   = 'checker PAL.bmp';
//  sOverlayBMPName1   = 'Blackmagic Design NTSC.bmp';
//  sOverlayBMPName2   = 'checker NTSC.bmp';

  sDLStillSourceName       = 'Decklink Checker Source Filter';
  sInfiniteTeeName         = 'Infinite Pin Tee Filter';
  sDLVideoRenderFilterName = 'Decklink Video Renderer';


implementation
uses
  ActiveX
 ,DecklinkFilters
 ;

{$R *.dfm}

procedure TfrmMainRaw.FormDestroy(Sender: TObject);
var
  lPinList: TPinList;
  i : integer;
begin
  if Assigned(fDecklinkKeyer) then
    fDecklinkKeyer.set_AlphaBlendModeOff;

  if Assigned(fMediaControl) then
    fMediaControl.Stop;

  if Assigned(fDecklinkVR) then begin
    lPinList:= TPinList.Create(fDecklinkVR);
    for i := 0 to lPinList.Count - 1 do
      lPinList[i].Disconnect;
    end;


  fDecklinkKeyer       := nil;
  fDecklinkStillSource := nil;
//  fCaptureGraph        := nil;
//  fInfinitePin         := nil;
  fDecklinkVR          := nil;
  fMediaControl        := nil;
  fFileSource          := nil;
  fFilterGraph         := nil;
end;

procedure TfrmMainRaw.ActionListUpdate(Action: TBasicAction; var Handled: Boolean);
begin
  PrepareAction.Enabled := not Assigned(fDecklinkKeyer);
  PlayAction.Enabled := Assigned(fDecklinkKeyer);
  StopAction.Enabled := PlayAction.Enabled;
  OverlayGroupBox.Enabled := PlayAction.Enabled;
end;

procedure TfrmMainRaw.AlphaTrackBarChange(Sender: TObject);
begin
  if Assigned(fDecklinkKeyer) then
    fDecklinkKeyer.set_AlphaLevel( 255 - AlphaTrackBar.Position );
end;

procedure TfrmMainRaw.BmpBMDActionExecute(Sender: TObject);
begin
  fFileSource.Load(StringToOleStr(sOverlayBMPName1), nil);
end;

procedure TfrmMainRaw.BmpCheckerActionExecute(Sender: TObject);
begin
  fFileSource.Load(StringToOleStr(sOverlayBMPName2), nil);
end;

procedure TfrmMainRaw.Connect(AOutFilter, AInFilter: IBaseFilter);

  function _FirstForDirection( const APinList : TPinList; const APinDirection : TPinDirection): IPin;
  var
    i : integer;
    lPinDir : TPinDirection;
  begin
    Result := nil;
    for i := 0 to APinList.Count - 1 do
      if not APinList.Connected[i] then begin
        APinList[i].QueryDirection(lPinDir);
        if lPinDir = APinDirection then begin
          Result := APinList[i];
          Break; //==========================>
          end;
      end;
  end;

var
  lInPinList, lOutPinList: TPinList;
begin
  lOutPinList:= TPinList.Create(AOutFilter);
  lInPinList := TPinList.Create(AInFilter);
  try
    if  (lOutPinList.Count > 0)
    and (lInPinList.Count > 0) then
      fFilterGraph.Connect( _FirstForDirection(lOutPinList,PINDIR_OUTPUT), _FirstForDirection(lInPinList,PINDIR_INPUT) );
  finally
    lOutPinList.Free;
    lInPinList.Free;
  end;
end;

procedure TfrmMainRaw.KeyRadioGroupClick(Sender: TObject);
begin
  if Assigned(fDecklinkKeyer) then
    case KeyRadioGroup.ItemIndex of
      0:  fDecklinkKeyer.set_AlphaBlendModeOff;
      1:  fDecklinkKeyer.set_AlphaBlendModeOn( False );
      2:  fDecklinkKeyer.set_AlphaBlendModeOn( True );
    end;
end;

procedure TfrmMainRaw.PlayActionExecute(Sender: TObject);
begin
  if Succeeded(fFilterGraph.QueryInterface(IMediaControl, fMediaControl)) then
    fMediaControl.Run;
  KeyRadioGroup.ItemIndex := 0; // set_AlphaBlendModeOff
end;

procedure TfrmMainRaw.StopActionExecute(Sender: TObject);
begin
  if Succeeded(fFilterGraph.QueryInterface(IMediaControl, fMediaControl)) then
    fMediaControl.Stop;
end;

procedure TfrmMainRaw.PrepareActionExecute(Sender: TObject);
var
  HR: HRESULT;
begin
  HR := CoCreateInstance(CLSID_FilterGraph              , nil, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, fFilterGraph);

  HR := CoCreateInstance(CLSID_DecklinkStillSource      , nil, CLSCTX_INPROC_SERVER, IID_IBaseFilter  , fDecklinkStillSource);
  if Succeeded(HR) then
    fFilterGraph.AddFilter(fDecklinkStillSource, StringToOleStr(sDLStillSourceName));

  HR := CoCreateInstance(CLSID_DecklinkVideoRenderFilter      , nil, CLSCTX_INPROC_SERVER, IID_IBaseFilter  , fDecklinkVR);
  if Succeeded(HR) then
    fFilterGraph.AddFilter(fDecklinkVR, StringToOleStr(sDLVideoRenderFilterName));

  HR := fDecklinkVR.QueryInterface(IID_IDecklinkKeyer, fDecklinkKeyer);

  HR := fDecklinkStillSource.QueryInterface(IID_IFileSourceFilter, fFileSource);
  if Succeeded(HR) then
    fFileSource.Load(StringToOleStr(sOverlayBMPName1), nil);

  Connect(fDecklinkStillSource, fDecklinkVR);

// Started to experiment with Infinite Tee Pin Filter...
//  HR := CoCreateInstance(CLSID_InfTee                   , nil, CLSCTX_INPROC_SERVER, IID_IBaseFilter  , fInfinitePin);
//  if Succeeded(HR) then
//    fFilterGraph.AddFilter(fInfinitePin, StringToOleStr(sInfiniteTeeName));

//  Connect(fInfinitePin, fDecklinkVR);


end;

end.
