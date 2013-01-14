VERSION 5.00
Begin VB.Form frmMain 
   Caption         =   "GMFBridge Preview in VB"
   ClientHeight    =   2715
   ClientLeft      =   60
   ClientTop       =   450
   ClientWidth     =   4710
   LinkTopic       =   "Form1"
   ScaleHeight     =   2715
   ScaleWidth      =   4710
   StartUpPosition =   3  'Windows Default
   Begin VB.CommandButton btnStop 
      Caption         =   "&Stop"
      Height          =   375
      Left            =   3480
      TabIndex        =   3
      Top             =   1440
      Width           =   975
   End
   Begin VB.CommandButton btnCapture 
      Caption         =   "&Capture"
      Height          =   375
      Left            =   3480
      TabIndex        =   2
      Top             =   840
      Width           =   975
   End
   Begin VB.ListBox List1 
      Height          =   2400
      Left            =   120
      TabIndex        =   1
      Top             =   120
      Width           =   3135
   End
   Begin VB.CommandButton btnDevice 
      Caption         =   "&Device"
      Height          =   375
      Left            =   3480
      TabIndex        =   0
      Top             =   240
      Width           =   975
   End
End
Attribute VB_Name = "frmMain"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Dim SourceGraph As IMediaControl
Dim SourceGraphSinkFilter
Dim SinkGraph As IMediaControl
Dim SinkGraphSourceFilter

Dim Bridge As GMFBridgeController

' string constants representing standard GUIDs
Const typeVideo As String = "{73646976-0000-0010-8000-00AA00389B71}"
Const typeAudio As String = "{73647561-0000-0010-8000-00AA00389B71}"
Const catPreview As String = "{fb6c4282-0353-11d1-905f-0000c0cc16ba}"
Const catCapture As String = "{fb6c4281-0353-11d1-905f-0000c0cc16ba}"
Const SubtypeAVI As String = "{e436eb88-524f-11ce-9f53-0020af0ba770}"
Const SubtypeASF As String = "{3DB80F90-9412-11d1-ADED-0000F8754B99}"

Dim FileIndex As Long

' to check if a file exists
Private Declare Function OpenFile Lib "kernel32" (ByVal lpFileName As String, _
   lpReOpenBuff As OFSTRUCT, ByVal wStyle As Long) As Long

Const OFS_MAXPATHNAME As Integer = 128

Private Type OFSTRUCT
    cBytes As Byte
    fFixedDisk As Byte
    nErrCode As Integer
    Reserved1 As Integer
    Reserved2 As Integer
    szPathName(OFS_MAXPATHNAME) As Byte
End Type
   
   



Private Sub btnCapture_Click()
    If SinkGraph Is Nothing Then
        btnDevice_Click
    End If
    SinkGraph.Run
    Bridge.BridgeGraphs SourceGraphSinkFilter, SinkGraphSourceFilter
    List1.AddItem "Capture started"
End Sub

Private Sub btnDevice_Click()
    
    ' user selection of device by friendly name
    Set SourceGraph = New FilgraphManager
    Set frmDevice.Graph = SourceGraph
    frmDevice.Show 1
    If frmDevice.SourceFilter Is Nothing Then Exit Sub
    List1.AddItem "Created filter: " + frmDevice.FilterName
    
    ' initialise bridge and source graph
    Set Bridge = New GMFBridgeController
    Bridge.AddStream True, eMuxInputs, True
    Set SourceGraphSinkFilter = Bridge.InsertSinkFilter(SourceGraph)
    
    ' use capture graph builder to render preview stream
    Dim builder As VBCaptureGraphHelper
    Set builder = New VBCaptureGraphHelper
    builder.Graph = SourceGraph
    builder.RenderStream catPreview, typeVideo, frmDevice.SourceFilter, Nothing, Nothing
    
    ' connect capture stream to bridge
    builder.RenderStream catCapture, typeVideo, frmDevice.SourceFilter, Nothing, SourceGraphSinkFilter
    builder.Graph = Nothing
        
    SourceGraph.Run
    List1.AddItem "Source graph running"
   
    SetOutputFile
End Sub

Private Sub btnStop_Click()
    If Not SinkGraph Is Nothing Then
        Bridge.BridgeGraphs Nothing, Nothing
        SinkGraph.Stop
        List1.AddItem "capture completed"
        
        SetOutputFile
    End If
End Sub

Private Sub Form_Load()
    FileIndex = 1
End Sub

Private Function FileExists(FileName As String) As Boolean
    Dim RetCode As Integer
    Dim OpenFileStructure As OFSTRUCT
    Const OF_EXIST = &H4000
    Const FILE_NOT_FOUND = 2
    RetCode = OpenFile(FileName$, OpenFileStructure, OF_EXIST)
    If OpenFileStructure.nErrCode = FILE_NOT_FOUND Then
        FileExists = False
    Else
        FileExists = True
    End If
End Function


Private Sub SetOutputFile()

    ' create a unique capture file name
    Dim FileName As String
    Do
        FileName = "c:\capture" + Str$(FileIndex) + ".avi"
        FileIndex = FileIndex + 1
    Loop While FileExists(FileName)
    
    Set SinkGraph = New FilgraphManager
    Set SinkGraphSourceFilter = Bridge.InsertSourceFilter(SourceGraphSinkFilter, SinkGraph)
    
    ' connect up mux and file writer using capture graph builder
    Dim builder As VBCaptureGraphHelper
    Set builder = New VBCaptureGraphHelper
    builder.Graph = SinkGraph
    Dim fMux As IFilterInfo
    Dim fSink As IFilterInfo
    builder.SetOutputFileName SubtypeAVI, FileName, fMux, fSink
    builder.RenderStream vbNullString, vbNullString, SinkGraphSourceFilter, Nothing, fMux
    List1.AddItem "Initialized output for " + FileName
End Sub
