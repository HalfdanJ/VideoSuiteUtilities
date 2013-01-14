unit TfrmMainUnit;
{ This program is a *very* simple example of interfacing with a
  Blackmagic Design Decklink card using the DSPack (DirectShow
  component package) - http://sourceforge.net/projects/dspack/
  and at www.progdigy.com

  Purpose: This program is trivial!!  Its' intention is to provide a starting
           point for Delphi developers wanting to communicate with the DL card.
           The chosen function here is the keying function of static bitmap images.
           A simple text image is generated to add a little interest, along with
           manually-connecting the pins.

           Two TFilters are pre-populated in the IDE with:
             DLFilter    = Decklink Video Renderer
             StillFilter = Decklink Still Source Filter

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
             in this directory - the BMD logo and checker image. A third image
             is created on the fly containing some text in a semi-transparent box.


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
 ,DSPack
 ,DSUtil
 ,DirectShow9
 ,Decklink
 ,ActnList
 ,Buttons
;

type
  TfrmMain = class(TForm)
    ActionList            : TActionList;
    AlphaTrackBar         : TTrackBar;
    Bevel1                : TBevel;
    Bevel3                : TBevel;
    BgColorBox            : TColorBox;
    BmpBMDAction          : TAction;
    BmpBMDRadioButton     : TRadioButton;
    BmpCheckerAction      : TAction;
    BmpCheckerRadioButton : TRadioButton;
    BmpCustomAction       : TAction;
    BmpCustomApplyAction  : TAction;
    BmpCustomButton       : TButton;
    BmpCustomRadioButton  : TRadioButton;
    DLFilter              : TFilter;
    FilterGraph           : TFilterGraph;
    ImageList             : TImageList;
    KeyRadioGroup         : TRadioGroup;
    Label1                : TLabel;
    Label2                : TLabel;
    OverlayGroupBox       : TGroupBox;
    PlayAction            : TAction;
    PrepareAction         : TAction;
    PrepareButton         : TBitBtn;
    RunButton             : TBitBtn;
    StillFilter           : TFilter;
    StopAction            : TAction;
    StopButton            : TBitBtn;
    TextEdit              : TEdit;
    TextFontAction        : TAction;
    TextFontButton        : TButton;
    TextFontDialog        : TFontDialog;
    procedure ActionListUpdate(Action: TBasicAction; var Handled: Boolean);
    procedure AlphaTrackBarChange(Sender: TObject);
    procedure BmpBMDActionExecute(Sender: TObject);
    procedure BmpCheckerActionExecute(Sender: TObject);
    procedure BmpCustomActionExecute(Sender: TObject);
    procedure BmpCustomApplyActionExecute(Sender: TObject);
    procedure FormCreate(Sender: TObject);
    procedure FormDestroy(Sender: TObject);
    procedure KeyRadioGroupClick(Sender: TObject);
    procedure PlayActionExecute(Sender: TObject);
    procedure PrepareActionExecute(Sender: TObject);
    procedure StopActionExecute(Sender: TObject);
    procedure TextFontActionExecute(Sender: TObject);
  private
    { Private declarations }
    fDecklinkKeyer: IDecklinkKeyer;
    fFileSource   : IFileSourceFilter;
    fMediaSeeking : IMediaSeeking;
    procedure Connect(AOutFilter, AInFilter: IBaseFilter);
    procedure BuildCustomBmp;
  public
    { Public declarations }
  end;

var
  frmMain: TfrmMain;

implementation
uses
{ GR32 is a suite of graphic routines that, amongst other things,
  make it easy to work with the alpha channel.
  Download from SF...
    http://sourceforge.net/projects/graphics32
  This is not essential: Comment out these (and the 'BmpCustom' action) if not installed
}
  GR32
 ,GR32_Filters
 ;


const
{ Original code set for PAL.  Reverse IMAGEHEIGHT and BMP naming for NTSC.
  Possible enhancement: Determine the metrics of the DL renderer and set
  the video type automatically. }

  IMAGEHEIGHT = 576;  // PAL
//  IMAGEHEIGHT = 486; // NTSC
  IMAGEWIDTH = 720;

// Images from Samples\DecklinkKeyer\res folder
  sOverlayBMPName1   = 'Blackmagic Design PAL.bmp';
  sOverlayBMPName2   = 'checker PAL.bmp';
//  sOverlayBMPName1   = 'Blackmagic Design NTSC.bmp';
//  sOverlayBMPName2   = 'checker NTSC.bmp';

  { sOverlayBMPName3 is an abitrary name.  It is created on the fly. }
  sOverlayBMPName3   = 'DLCustom.bmp';


{$R *.dfm}

procedure TfrmMain.FormCreate(Sender: TObject);
begin
  BmpBMDRadioButton.Caption     := sOverlayBMPName1;
  BmpCheckerRadioButton.Caption := sOverlayBMPName2;

  BgColorBox.Selected       := clBlack;

  TextFontDialog.Font.Size  := 32;
  TextFontDialog.Font.Style := [fsBold];
  TextFontDialog.Font.Color := clYellow;
end;

procedure TfrmMain.FormDestroy(Sender: TObject);
begin
{ Turning alphablend off and clearing the graph is recommended for a clean exit }
  if Assigned(fDecklinkKeyer) then
    fDecklinkKeyer.set_AlphaBlendModeOff;
  FilterGraph.ClearGraph;
  FilterGraph.Active := false;
end;

procedure TfrmMain.PrepareActionExecute(Sender: TObject);
begin
  if DLFilter.QueryInterface(IID_IDecklinkKeyer, fDecklinkKeyer) <> S_OK then
    ShowMessage('IID_IDecklinkKeyer didn''t work')
  else
  if StillFilter.QueryInterface(IID_IFileSourceFilter, fFileSource) <> S_OK then
    ShowMessage('IID_IFileSourceFilter didn''t work')
  else begin
    if not FileExists(sOverlayBMPName1) then
      ShowMessage('File not found: ' + sOverlayBMPName1)
    else
    if fFileSource.Load(StringToOleStr(sOverlayBMPName1), nil) <> S_OK then
      ShowMessage('FileSource.Load didn''t work')
  end;

  Connect((StillFilter as IBaseFilter), (DLFilter as IBaseFilter));
end;

procedure TfrmMain.PlayActionExecute(Sender: TObject);
begin
  FilterGraph.Play;
end;

procedure TfrmMain.StopActionExecute(Sender: TObject);
begin
  FilterGraph.Stop;
end;

procedure TfrmMain.BmpBMDActionExecute(Sender: TObject);
begin
  fFileSource.Load(StringToOleStr(sOverlayBMPName1), nil);
end;

procedure TfrmMain.BmpCheckerActionExecute(Sender: TObject);
begin
  fFileSource.Load(StringToOleStr(sOverlayBMPName2), nil);
end;

procedure TfrmMain.BmpCustomActionExecute(Sender: TObject);
begin
  BuildCustomBmp;
  fFileSource.Load(StringToOleStr(sOverlayBMPName3), nil);
end;

procedure TfrmMain.BmpCustomApplyActionExecute(Sender: TObject);
begin
  // Same logic as [Apply]
  BuildCustomBmp;
  fFileSource.Load(StringToOleStr(sOverlayBMPName3), nil);
end;

procedure TfrmMain.BuildCustomBmp;
var
  lBitmap32, lAlpha : TBitmap32; // Declared in GR32 (ref comments above)
  lRect : TRect;
begin
  lAlpha := TBitmap32.Create;
  try
    lBitmap32    := TBitmap32.Create;
    try
      lBitmap32.Width  := IMAGEWIDTH;
      lBitmap32.Height := IMAGEHEIGHT;
      lAlpha.Width     := IMAGEWIDTH;
      lAlpha.Height    := IMAGEHEIGHT;

      // Specify abitrary area on the screen for text box
      lRect := Rect(50,IMAGEHEIGHT-120,IMAGEWIDTH-50,IMAGEHEIGHT-50);

      lBitmap32.Canvas.Brush.Color := BgColorBox.Selected;
      lBitmap32.Canvas.FillRect(lRect);

{ lAlpha is a placeholder for the alpha channel.  We can paint anything
  here and it's ultimate destination is the alpha mask of lBitmap32.
  Don't be confused with the use of a color in lAlpha. This method is used
  because the Graphic32 function: IntensityToAlpha() provides a neat way
  of mapping one image to the alpha channel of another.
  We could of course replace this with raw addressing of the BMP canvas.

  By setting lAlpha.Canvas.Brush.Color to clGray we define the opacity of the rectangle
  around text.  Try clSilver or some other color for diferent opacity level.
  We then paint the text onto lAlpha with clWhite to arrive at 0xFF in the alpha channel
  to match the lBitmap32 text.
}
      lAlpha.Canvas.Brush.Color := clGray;
      lAlpha.Canvas.FillRect(lRect);

      lAlpha.Font.Assign(TextFontDialog.Font);
      lAlpha.Font.Color := clWhite;  // full opacity under text
      lAlpha.Textout(lRect,DT_CENTER,TextEdit.Text);

      lBitmap32.Font.Assign(TextFontDialog.Font);
      lBitmap32.Textout(lRect,DT_CENTER,TextEdit.Text);

      IntensityToAlpha(lBitmap32, lAlpha);
      lBitmap32.SaveToFile(sOverlayBMPName3);

    finally
      lBitmap32.Free;
    end;
  finally
    lAlpha.Free;
  end;
end;

procedure TfrmMain.Connect(AOutFilter, AInFilter: IBaseFilter);

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
{ This code connects the out pin of the still filter to the in pin of the DL renderer.
  It assumes no pins are currently connected *and* the first (and probably only) pin
  of each will be used.
  I'm willing to be educated if there is a better way to achieve this as I couldn't
  find one in DSPack.  Fortunately TPinList saves us performing our own enumeration. }

  lOutPinList:= TPinList.Create(AOutFilter);
  lInPinList := TPinList.Create(AInFilter);
  try
    if  (lOutPinList.Count > 0)
    and (lInPinList.Count > 0) then
      (FilterGraph as IGraphBuilder).Connect( _FirstForDirection(lOutPinList,PINDIR_OUTPUT), _FirstForDirection(lInPinList,PINDIR_INPUT) );
  finally
    lOutPinList.Free;
    lInPinList.Free;
  end;
end;

procedure TfrmMain.TextFontActionExecute(Sender: TObject);
begin
  TextFontDialog.Execute;
end;

procedure TfrmMain.KeyRadioGroupClick(Sender: TObject);
begin
  if Assigned(fDecklinkKeyer) then
    case KeyRadioGroup.ItemIndex of
      0:  fDecklinkKeyer.set_AlphaBlendModeOff;
      1:  fDecklinkKeyer.set_AlphaBlendModeOn( False );
      2:  fDecklinkKeyer.set_AlphaBlendModeOn( True );
    end;
end;

procedure TfrmMain.ActionListUpdate(Action: TBasicAction; var Handled: Boolean);
begin
  PrepareAction.Enabled := not Assigned(fDecklinkKeyer);
  PlayAction.Enabled := Assigned(fDecklinkKeyer);
  StopAction.Enabled := PlayAction.Enabled;
  OverlayGroupBox.Enabled := PlayAction.Enabled;
end;

procedure TfrmMain.AlphaTrackBarChange(Sender: TObject);
begin
  if Assigned(fDecklinkKeyer) then
    fDecklinkKeyer.set_AlphaLevel( 255 - AlphaTrackBar.Position );
end;

end.
