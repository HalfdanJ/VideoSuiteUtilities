(***************************************************************)
(*  This is an import unit for DECKLINKINTERFACE.H             *)
(*  Translated by Vassil Nazarov                               *)
(*  email: vassil@engineer.bg                                  *)
(*                                                             *)
(*  This file has been translated from DECKLINKINTERFACE.H     *)
(*  and provides all the DeckLink GUIDs.                       *)
(*                                                             *)
(*  To use, add this file to your "Uses" clause and implement  *)
(*  as shown below:                                            *)
(*                                                             *)
(*                                                             *)
(*  Procedure SampleProc;                                      *)
(*  Var                                                        *)
(*   HR: HRESULT;                                              *)
(*   FVidRen: IBaseFilter;                                     *)
(*  Begin                                                      *)
(*                                                             *)
(*   HR:=CoCreateInstance(CLSID_DecklinkVideoRenderFilter,     *)
(*                          Nil,                               *)
(*                          CLSCTX_INPROC,                     *)
(*                          IID_IBaseFilter,                   *)
(*                          FVidRen);                          *)
(*                                                             *)
(*   //Build your hraph etc ...                                *)
(*                                                             *)
(*   With FVidRen AS IDecklinkKeyer DO Begin                   *)
(*     ...                                                     *)
(*     HR:=set_AlphaBlendModeOn(True);                         *)
(*     ...                                                     *)
(*     End;                                                    *)
(*                                                             *)
(*  End;                                                       *)
(*                                                             *)
(***************************************************************)

Unit DeckLink;
Interface
Uses Windows, DirectShow9;

Const
  CLSID_DecklinkVideoRenderFilter : TGUID='{CEB13CC8-3591-45A5-BA0F-20E9A1D72F76}';
  CLSID_DecklinkVideoRenderFilter2: TGUID='{16A2E3A5-0C3E-4484-9E78-3ABF2FBE3ACE}';
  CLSID_DecklinkVideoRenderFilter3: TGUID='{4354ED19-BDE1-4083-9BE7-991AF1F0B527}';
  CLSID_DecklinkVideoRenderFilter4: TGUID='{4546201B-E2B2-4e78-9B1C-DE6406C3BEFA}';
  CLSID_DecklinkVideoRenderFilter5: TGUID='{4A5FCAED-CAA7-456e-B8EA-608F35A68A0D}';
  CLSID_DecklinkVideoRenderFilter6: TGUID='{8128230E-8FC1-4af6-BBF1-6C6B6E7C1F16}';
  CLSID_DecklinkVideoRenderFilter7: TGUID='{6919722B-7329-4c5f-9F68-BA2FE3CF1C77}';
  CLSID_DecklinkVideoRenderFilter8: TGUID='{4C17A259-854D-433f-B087-9AC89238180A}';
  CLSID_DecklinkVideoRenderProperties: TGUID='{98C36C7C-5985-46C4-909C-0EB7BD0C60F7}';

  CLSID_DecklinkAudioRenderFilter : TGUID='{19FA8CC3-56CE-46AB-825D-5CE1A39B137A}';
  CLSID_DecklinkAudioRenderFilter2: TGUID='{8D09D460-D361-40bd-A2D0-7E094B4D56FA}';
  CLSID_DecklinkAudioRenderFilter3: TGUID='{F34B54D3-15B2-4779-8913-64D6E6C67873}';
  CLSID_DecklinkAudioRenderFilter4: TGUID='{98FC338D-9524-457c-916A-14016AC483A8}';
  CLSID_DecklinkAudioRenderFilter5: TGUID='{1A06C310-DCD3-4bc3-8BA3-47F8273A509C}';
  CLSID_DecklinkAudioRenderFilter6: TGUID='{2C7D2EF4-2054-485b-8310-5EE8305955C9}';
  CLSID_DecklinkAudioRenderFilter7: TGUID='{EF87B072-BCD3-4f4e-A17C-234C73592631}';
  CLSID_DecklinkAudioRenderFilter8: TGUID='{F8CFB51B-ADE0-4fae-B9A6-38E9F0BB2919}';
  CLSID_DecklinkAudioRenderProperties: TGUID='{222A4295-E98B-4af2-9063-340E91BE7E68}';

  CLSID_DecklinkVideoCaptureFilter : TGUID='{44A8B5C7-13B6-4211-BD40-35B629D9E6DF}';
  CLSID_DecklinkVideoCaptureFilter2: TGUID='{CE3FF814-04C1-4827-9F18-426203E9B1B3}';
  CLSID_DecklinkVideoCaptureFilter3: TGUID='{20722FC4-9EBC-47b9-80B4-6A2ED4D27ECF}';
  CLSID_DecklinkVideoCaptureFilter4: TGUID='{B3980D18-10D4-4045-9AF7-91EF330AEBF2}';
  CLSID_DecklinkVideoCaptureFilter5: TGUID='{929CE8E6-96DB-4fe0-980A-83DAB2D2AF31}';
  CLSID_DecklinkVideoCaptureFilter6: TGUID='{681439FF-EB76-45dd-BF51-80D7C60F5727}';
  CLSID_DecklinkVideoCaptureFilter7: TGUID='{DE5D8755-421A-43f2-8847-E5A9ACB44D04}';
  CLSID_DecklinkVideoCaptureFilter8: TGUID='{976AC924-E89A-4a20-B692-482346E24C71}';
  CLSID_DecklinkVideoCaptureProperties: TGUID='{50D29FCF-70ED-4155-9B2A-91F2CE9A86BA}';
  CLSID_DecklinkVideoCaptureProperties2: TGUID='{6296A3EB-08FF-421e-B5AC-6BC834CF4DB6}';

  CLSID_DecklinkAudioCaptureFilter : TGUID='{AAA22F7E-5AA0-49d9-8C8D-B52B1AA92EB7}';
  CLSID_DecklinkAudioCaptureFilter2: TGUID='{1BCC3EF4-724F-4a45-B61D-8D4BBF32C5BF}';
  CLSID_DecklinkAudioCaptureFilter3: TGUID='{BC90EE8C-F8DC-4776-8DCF-24929C13F9D2}';
  CLSID_DecklinkAudioCaptureFilter4: TGUID='{1EF51542-B849-4337-B7AE-C980C13F86BF}';
  CLSID_DecklinkAudioCaptureFilter5: TGUID='{DE6DB48D-66EE-477a-BE4B-522B2253CD95}';
  CLSID_DecklinkAudioCaptureFilter6: TGUID='{B8E0C8A1-777D-4727-A542-6DF0DF1E1D1C}';
  CLSID_DecklinkAudioCaptureFilter7: TGUID='{ED1441F5-A132-44e7-9027-14DB5674149B}';
  CLSID_DecklinkAudioCaptureFilter8: TGUID='{37BD18AA-F5BD-434c-B0C5-7AE570C377A5}';
  CLSID_DecklinkAudioCaptureProperties: TGUID='{ED4418E7-582D-4759-AE07-8CA7F771427F}';
  CLSID_DecklinkAudioCaptureProperties2: TGUID='{8869832C-FDE3-468e-B0D2-53BF2D59C17A}';

  CLSID_DecklinkUpsampleFilter: TGUID='{F5C45F6D-E4DD-469d-B397-7341D602C403}';

  CLSID_DecklinkEffectsFilter: TGUID='{BFA26F43-FB18-40d9-BD58-5A6CE0F42469}';
  CLSID_DecklinkEffectsProperties: TGUID='{EA131320-64CC-4f3f-B79D-41A383A65EDE}';

  CLSID_DecklinkMJPEGEncoderFilter: TGUID='{1E003B41-B606-4ae4-B2BB-C35E133575A5}';
  CLSID_DecklinkMJPEGEncoderProperties: TGUID='{B21009AE-27A7-4e32-B6B8-39723FD5D35D}';

  CLSID_DecklinkMJPEGDecoderFilter: TGUID='{979520BE-4C52-4ba7-A534-B145EDCFDF21}';
  CLSID_DecklinkMJPEGDecoderProperties: TGUID='{EFAD70C1-0CA9-4754-A36B-FE2EC634E140}';

  CLSID_DecklinkCaptureFilter: TGUID='{472BB322-7639-412e-AF90-F86F1AD6A22F}';
  CLSID_DecklinkRenderFilter: TGUID='{189B7800-82A0-4e92-A2E9-2C8E4A15C3E3}';

  CLSID_Decklink_Decoder_DMO: TGUID='{7E7E7215-6A31-486C-AA6F-1DB916E1E022}';

  IID_MEDIASUBTYPE_v210: TGUID ='{30313276-B0B0-4dd3-8E8C-572692D526F6}';
  IID_MEDIASUBTYPE_v210a: TGUID ='{30313276-0000-0010-8000-00AA00389B71}';
  IID_MEDIASUBTYPE_r210: TGUID ='{30313272-0000-0010-8000-00AA00389B71}';
  IID_MEDIASUBTYPE_HDYC: TGUID ='{43594448-0000-0010-8000-00AA00389B71}';

  IID_IDecklinkKeyer: TGUID = '{9D63ADFC-8D1A-451D-958E-12FA4B1EFD2F}';
  IID_IDecklinkRawDeviceControl: TGUID = '{72D62DE6-010F-48e6-A251-78CA285BDFE0}';
  IID_IDecklinkStatus: TGUID ='{15BE165D-BFF5-47f8-8E71-DE4657ABEBE5}';
  IID_IDecklinkCaptureBanner: TGUID ='{26D02C91-B25F-40ff-9B39-63B3FABCC518}';
  IID_IDecklinkIOControl: TGUID ='{60F58A81-A387-4922-AAAC-998BD9FBE1AA}';
  IID_IDecklinkMediaSample: TGUID ='{4CAEF6E0-714A-4b4c-902D-BC53AAB2C423}';

Type
  DECKLINK_VIDEO_FORMAT=(DECKLINK_OF_NTSC,
                         DECKLINK_OF_PAL,
                         DECKLINK_OF_HD720,
                         DECKLINK_OF_HD1080,
                         DECKLINK_OF_MAX);

  DECKLINK_PIXEL_FORMAT=(DECKLINK_PF_8BIT,
                         DECKLINK_PF_10BIT,
                         DECKLINK_PF_BGRA32BIT,
                         DECKLINK_PF_ARGB32BIT,
                         DECKLINK_PF_RGB10BIT,
                         DECKLINK_PF_MAX);

  DECKLINK_PIXEL_4CC=(DECKLINK_XF_8BIT=$32767579,
                      DECKLINK_XF_10BIT=$76323130,
                      DECKLINK_XF_BGRA32BIT=$42475241,
                      DECKLINK_XF_ARGB32BIT=$20);

  DECKLINK_FRAME_RATE=(DECKLINK_FR_2400,
                       DECKLINK_FR_2500,
                       DECKLINK_FR_2997,
                       DECKLINK_FR_3000,
                       DECKLINK_FR_5994,
                       DECKLINK_FR_6000);

  SYNTH_OUTPUT_FORMAT=(SYNTH_OF_PCM,
                       SYNTH_OF_MS_ADPCM);

  IDecklinkKeyer=Interface(IUnknown)
    ['{9D63ADFC-8D1A-451D-958E-12FA4B1EFD2F}']
    Function set_AlphaBlendModeOn(isExternalKey: BOOL): HRESULT; StdCall;
    Function set_AlphaBlendModeOff: HRESULT; StdCall;
    Function set_AlphaLevel(alphaLevel: Cardinal): HRESULT; StdCall;
    Function set_AlphaAutoBlendSettings(rampFrames: Cardinal;
                                        onFrames: Cardinal;
                                        offFrames: Cardinal;
                                        blendProcessRepeats: Cardinal): HRESULT; StdCall;
    Function do_AlphaRampOn(framesDuration: Cardinal): HRESULT; StdCall;
    Function do_AlphaRampOff(framesDuration: Cardinal): HRESULT; StdCall;
    Function get_DeviceSupportsKeying: HRESULT; StdCall;
    Function get_DeviceSupportsExternalKeying: HRESULT; StdCall;
    Function set_DefaultTimebase(Timebase: Integer): HRESULT; StdCall;
    End;

  TDecklinkRawCommandAsync=Packed Record
    Command: PByte;
    lenCommand: Cardinal;
    asyncResult: Integer;
    response: PByte;
    lenResponse: Cardinal;
    commandComplete: Integer;
    End;

  IDecklinkRawDeviceControl=Interface(IUnknown)
    ['{72D62DE6-010F-48e6-A251-78CA285BDFE0}']
    Function SendRawCommandSync(Const Command: Pointer;
                                lenCommand: Cardinal;
                                Var response: Pointer;
                                lenResponse: Cardinal): HRESULT; StdCall;
    Function SendRawCommandAsync(Var Command: TDecklinkRawCommandAsync): HRESULT; StdCall;
    End;

  DECKLINK_INPUT=(DECKLINK_INPUT_NONE,
	                DECKLINK_INPUT_PRESENT,
	                DECKLINK_INPUT_MAX);
  TDL_VideoStatus=DECKLINK_INPUT;

  DECKLINK_GENLOCK=(DECKLINK_GENLOCK_NOTSUPPORTED,
	                  DECKLINK_GENLOCK_NOTCONNECTED,
	                  DECKLINK_GENLOCK_LOCKED,
	                  DECKLINK_GENLOCK_NOTLOCKED,
	                  DECKLINK_GENLOCK_MAX);
  TDL_GenLockStatus=DECKLINK_GENLOCK;

  IDecklinkStatus=Interface(IUnknown)
    ['{15BE165D-BFF5-47f8-8E71-DE4657ABEBE5}']
    Function GetVideoInputStatus(Var videoStatus: DECKLINK_INPUT;
                                 Var genlockStatus: DECKLINK_GENLOCK): HRESULT; StdCall;
    Function RegisterVideoStatusChangeEvent(HEvent: THandle): HRESULT; StdCall;
    End;

  IDecklinkCaptureBanner=Interface(IUnknown)
    ['{26D02C91-B25F-40ff-9B39-63B3FABCC518}']
    Function GetNoInputFrame(Var frame: Pointer): HRESULT; StdCall;
    Function BlackVideo(Const bmiH: Pointer;
                        Const Frame: Pointer): HRESULT; StdCall;
    End;

  DECKLINK_BLACKINCAPTURE=(DECKLINK_BLACKINCAPTURE_NONE,
	                         DECKLINK_BLACKINCAPTURE_DIGITAL,
	                         DECKLINK_BLACKINCAPTURE_ANALOGUE,
	                         DECKLINK_BLACKINCAPTURE_MAX);
  TDL_BlackInCapture=DECKLINK_BLACKINCAPTURE;

  DECKLINK_HDDOWNCONVERSION=(DECKLINK_HDDOWNCONVERSION_OFF,
	                           DECKLINK_HDDOWNCONVERSION_LB16X9,
	                           DECKLINK_HDDOWNCONVERSION_ANA,
                             DECKLINK_HDDOWNCONVERSION_CENTER,
	                           DECKLINK_HDDOWNCONVERSION_MAX);
  TDL_HDDownConversion=DECKLINK_HDDOWNCONVERSION;

  DECKLINK_AUDIOINPUTSOURCE=(DECKLINK_AUDIOINPUTSOURCE_EMBEDDED,
	                           DECKLINK_AUDIOINPUTSOURCE_AESEBU,
                             DECKLINK_AUDIOINPUTSOURCE_ANALOGUE,
	                           DECKLINK_AUDIOINPUTSOURCE_MAX,
							   (* Synonyms *)
							   DECKLINK_AUDIOINPUTSOURCE_SDI = DECKLINK_AUDIOINPUTSOURCE_EMBEDDED);
  TDL_AudioInputSrc=DECKLINK_AUDIOINPUTSOURCE;

  DECKLINK_IOFEATURES=(DECKLINK_IOFEATURES_SUPPORTSINTERNALKEY	    = 1 SHL  4,
                       DECKLINK_IOFEATURES_SUPPORTSEXTERNALKEY	    = 1 SHL  5,
                       DECKLINK_IOFEATURES_HASCOMPONENTVIDEOOUTPUT	= 1 SHL  6,
                       DECKLINK_IOFEATURES_HASCOMPOSITEVIDEOOUTPUT	= 1 SHL  7,
                       DECKLINK_IOFEATURES_HASDIGITALVIDEOOUTPUT	  = 1 SHL  8,
                       DECKLINK_IOFEATURES_HASDVIVIDEOOUTPUT	      = 1 SHL  9,
                       DECKLINK_IOFEATURES_HASCOMPONENTVIDEOINPUT	  = 1 SHL 10,
                       DECKLINK_IOFEATURES_HASCOMPOSITEVIDEOINPUT	  = 1 SHL 11,
                       DECKLINK_IOFEATURES_HASDIGITALVIDEOINPUT	    = 1 SHL 12,
                       DECKLINK_IOFEATURES_HASDUALLINKOUTPUT	      = 1 SHL 13,
                       DECKLINK_IOFEATURES_HASDUALLINKINPUT	        = 1 SHL 14,
                       DECKLINK_IOFEATURES_SUPPORTSHD	              = 1 SHL 15,
                       DECKLINK_IOFEATURES_SUPPORTS2KOUTPUT	        = 1 SHL 16,
                       DECKLINK_IOFEATURES_SUPPORTSHDDOWNCONVERSION	= 1 SHL 17,
                       DECKLINK_IOFEATURES_HASAESAUDIOINPUT	        = 1 SHL 18,
                       DECKLINK_IOFEATURES_HASANALOGUEAUDIOINPUT	  = 1 SHL 19,
                       DECKLINK_IOFEATURES_HASSVIDEOINPUT	          = 1 SHL 20,
                       DECKLINK_IOFEATURES_HASSVIDEOOUTPUT	        = 1 SHL 21,
                       DECKLINK_IOFEATURES_SUPPORTSMULTICAMERAINPUT	= 1 SHL 22,
                       DECKLINK_IOFEATURES_HASRS422SERIALPORT	      = 1 SHL 23,
                       DECKLINK_IOFEATURES_HASHDMIINPUT	            = 1 SHL 24,
		                   DECKLINK_IOFEATURES_HASHDMIOUTPUT	          = 1 SHL 25,
                       DECKLINK_IOFEATURES_MAX = DECKLINK_IOFEATURES_SUPPORTSMULTICAMERAINPUT + 1);
  TDL_IOFeatures=DECKLINK_IOFEATURES;

  DECKLINK_TIMECODESOURCE=(DECKLINK_TIMECODESOURCE_VITC,
	                         DECKLINK_TIMECODESOURCE_HANC,
	                         DECKLINK_TIMECODESOURCE_MAX);
  TDL_TimecodeSource=DECKLINK_TIMECODESOURCE;

  DECKLINK_VIDEOINPUT=(DECKLINK_VIDEOINPUT_SDI,
	                     DECKLINK_VIDEOINPUT_COMPONENT,
	                     DECKLINK_VIDEOINPUT_COMPOSITE,
	                     DECKLINK_VIDEOINPUT_SVIDEO,
						 DECKLINK_VIDEOINPUT_HDMI,
	                     DECKLINK_VIDEOINPUT_MAX);
  TDL_VideoInput=DECKLINK_VIDEOINPUT;

  DECKLINK_VIDEOOUTPUT=(DECKLINK_VIDEOOUTPUT_COMPONENT,
	                      DECKLINK_VIDEOOUTPUT_COMPOSITE,
	                      DECKLINK_VIDEOOUTPUT_SVIDEO,
	                      DECKLINK_VIDEOOUTPUT_MAX);
  TDL_VideoOutput=DECKLINK_VIDEOOUTPUT;

  IDecklinkIOControl=Interface(IUnknown)
    ['{60F58A81-A387-4922-AAAC-998BD9FBE1AA}']
    Function GetIOFeatures(Var Features: Cardinal): HRESULT; StdCall;
    Function SetAnalogueOutput(IsComponent: BOOL; SetupIs75: BOOL): HRESULT; StdCall;
    Function SetVideoInput(InputIsDigital: BOOL;
                           IsComponent: BOOL;
                           SetupIs75: BOOL): HRESULT; StdCall;
    Function SetDualLinkOutput(EnableDualLinkOutput: BOOL): HRESULT; StdCall;
    Function SetSingleFieldOutputForSynchronousFrames(SingleFieldOutput: BOOL): HRESULT; StdCall;
    Function SetHDTVPulldownOnOutput(EnableHDTV32PulldownOnOutput: BOOL): HRESULT; StdCall;
    Function SetBlackToDeckInCapture(BlackToDeckSetting: TDL_BlackInCapture): HRESULT; StdCall;
    Function SetAFrameReference(aFrameReference: Cardinal): HRESULT; StdCall;
    Function SetCaptureVANCLines(VancLine1, VancLine2, VancLine3: Cardinal): HRESULT; StdCall;
    Function SetVideoOutputDownconversionMode(DownconversionMode: TDL_HDDownConversion): HRESULT; StdCall;
    Function SetAudioInputSource(AudioInputSource: TDL_AudioInputSrc): HRESULT; StdCall;
    Function SetGenlockTiming(TimingOffset: Integer): HRESULT; StdCall;
    Function SetVideoOutputDownconversionMode2(DownconversionMode: TDL_HDDownConversion;
                                               DownconvertToAnalogOutput: BOOL): HRESULT; StdCall;
    Function SetCaptureTimecodeSource(TimecodeSource: TDL_TimecodeSource): HRESULT; StdCall;
    Function SetVideoInput2(VideoSource: TDL_VideoInput;
                            SetupIs75: BOOL;
                            ComponentLevelsSMPTE: BOOL): HRESULT; StdCall;
    Function SetAnalogueOutput2(VideoOutput: TDL_VideoOutput;
                                SetupIs75: BOOL;
                                ComponentLevelsSMPTE: BOOL): HRESULT; StdCall;
    End;

  IDecklinkMediaSample=Interface(IUnknown)
    ['{4CAEF6E0-714A-4b4c-902D-BC53AAB2C423}']
    Function GetVANCBuffer(Var ppBuffer: Pointer): HRESULT; StdCall;
    End;

  IDecklinkReferenceClock=Interface(IUnknown)
    ['{E2ED66BF-C926-42C0-84BB-9A830F805DDB}']
    Function GetFrameTime(Var rtFrame: REFERENCE_TIME): HRESULT; StdCall;
    End;

  MEDIASUBTYPE_V210=Interface
    ['{30313276-B0B0-4dd3-8E8C-572692D526F6}']
    End;

  MEDIASUBTYPE_r210=Interface
    ['{30313272-0000-0010-8000-00AA00389B71}']
    End;

  MEDIASUBTYPE_HDYC=Interface
    ['{43594448-0000-0010-8000-00AA00389B71}']
    End;






IMPLEMENTATION

END.
