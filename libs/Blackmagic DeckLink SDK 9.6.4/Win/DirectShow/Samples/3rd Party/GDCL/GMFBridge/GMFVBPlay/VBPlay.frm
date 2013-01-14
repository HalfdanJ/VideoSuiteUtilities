VERSION 5.00
Object = "{F9043C88-F6F2-101A-A3C9-08002B2F49FB}#1.2#0"; "comdlg32.ocx"
Begin VB.Form Form1 
   Caption         =   "GMF Play VB Demo"
   ClientHeight    =   4185
   ClientLeft      =   60
   ClientTop       =   450
   ClientWidth     =   5910
   LinkTopic       =   "Form1"
   ScaleHeight     =   4185
   ScaleWidth      =   5910
   StartUpPosition =   3  'Windows Default
   Begin MSComDlg.CommonDialog CommonDialog1 
      Left            =   5400
      Top             =   3600
      _ExtentX        =   847
      _ExtentY        =   847
      _Version        =   393216
   End
   Begin VB.CommandButton btnPlay 
      Caption         =   "&Play"
      Height          =   375
      Left            =   4680
      TabIndex        =   3
      Top             =   1080
      Width           =   1095
   End
   Begin VB.CommandButton btnClear 
      Caption         =   "&Clear"
      Height          =   375
      Left            =   4680
      TabIndex        =   2
      Top             =   600
      Width           =   1095
   End
   Begin VB.CommandButton btnAdd 
      Caption         =   "&Add"
      Height          =   375
      Left            =   4680
      TabIndex        =   1
      Top             =   120
      Width           =   1095
   End
   Begin VB.ListBox List1 
      Height          =   3960
      Left            =   120
      TabIndex        =   0
      Top             =   0
      Width           =   4335
   End
End
Attribute VB_Name = "Form1"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Dim sourcegraph() As IMediaControl
Dim SourceGraphSinkFilter()
Dim SourceStart() As Double
Dim SourceStop() As Double
Dim SourceIndex As Long

Dim SourceCount As Long

Dim RenderGraph As IMediaControl
Dim RenderGraphSourceFilter
Dim WithEvents Bridge As GMFBridgeController
Attribute Bridge.VB_VarHelpID = -1

Private Sub Bridge_OnEndOfSegment()
    SourceIndex = SourceIndex + 1
    If SourceIndex = SourceCount Then
        SourceIndex = 0
        Rewind
    End If
    Bridge.BridgeGraphs SourceGraphSinkFilter(SourceIndex), RenderGraphSourceFilter
End Sub

Private Sub btnAdd_Click()
    CommonDialog1.ShowOpen
    
    If Bridge Is Nothing Then
        Set Bridge = New GMFBridgeController
        Bridge.AddStream True, eUncompressed, False
        Bridge.AddStream False, eUncompressed, False
        Bridge.SetBufferMinimum 200
    End If
    ReDim Preserve sourcegraph(SourceCount)
    ReDim Preserve SourceGraphSinkFilter(SourceCount)
    ReDim Preserve SourceStart(SourceCount)
    ReDim Preserve SourceStop(SourceCount)
    
    Set sourcegraph(SourceCount) = New FilgraphManager
    Set SourceGraphSinkFilter(SourceCount) = Bridge.CreateSourceGraph(CommonDialog1.FileName, sourcegraph(SourceCount))
       
    Dim mp As IMediaPosition
    Set mp = sourcegraph(SourceCount)
    frmClipRange.duration = mp.duration
    frmClipRange.Show 1
    SourceStart(SourceCount) = frmClipRange.tStart
    SourceStop(SourceCount) = frmClipRange.tStop
    Dim title As String
    title = CommonDialog1.FileName
    If (SourceStart(SourceCount) <> 0) Or (SourceStop(SourceCount) <> 0) Then
        title = title + " [" + Str(SourceStart(SourceCount)) + ".." + Str(SourceStop(SourceCount)) + "]"
    End If
    
    
    SourceCount = SourceCount + 1
    If SourceCount = 1 Then
        Set RenderGraph = New FilgraphManager
        Set RenderGraphSourceFilter = Bridge.CreateRenderGraph(SourceGraphSinkFilter(0), RenderGraph)
    End If
    List1.AddItem title
End Sub


Private Sub btnClear_Click()
    If Not RenderGraph Is Nothing Then
        RenderGraph.Stop
        Bridge.BridgeGraphs Nothing, Nothing
        Dim sg As Variant
        For Each sg In sourcegraph
            Dim mc As IMediaControl
            Set mc = sg
            mc.Stop
        Next sg
    End If
    
    ReDim sourcegraph(0)
    ReDim SourceGraphSinkFilter(0)
    ReDim SourceStart(0)
    ReDim SourceStop(0)
    
    Set RenderGraph = Nothing
    Set RenderGraphSourceFilter = Nothing
    Set Bridge = Nothing
    SourceCount = 0
    List1.Clear
End Sub

Private Sub btnPlay_Click()
    SourceIndex = 0
    Bridge.BridgeGraphs Nothing, Nothing
    RenderGraph.Pause
    Rewind
    Bridge.BridgeGraphs SourceGraphSinkFilter(SourceIndex), RenderGraphSourceFilter
    Dim sg As Variant
    For Each sg In sourcegraph
        Dim mc As IMediaControl
        Set mc = sg
        mc.Run
    Next sg
    RenderGraph.Run
End Sub

Private Sub Form_Load()
    SourceCount = 0
End Sub

Private Sub Form_Unload(Cancel As Integer)
    If Not Bridge Is Nothing Then
        Bridge.BridgeGraphs Nothing, Nothing
        RenderGraph.Stop
        Dim sg As Variant
        For Each sg In sourcegraph
            Dim mc As IMediaControl
            Set mc = sg
            mc.Stop
        Next sg
    End If
End Sub

Private Sub Rewind()
    Dim idx As Long
    For idx = 0 To SourceCount - 1
        Dim mp As IMediaPosition
        Set mp = sourcegraph(idx)
        If SourceStop(idx) <> 0 Then
            mp.StopTime = SourceStop(idx)
        End If
        mp.CurrentPosition = SourceStart(idx)
    Next idx
End Sub
