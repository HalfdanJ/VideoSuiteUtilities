VERSION 5.00
Begin VB.Form frmDevice 
   Caption         =   "Video Capture Devices"
   ClientHeight    =   3495
   ClientLeft      =   60
   ClientTop       =   450
   ClientWidth     =   6330
   LinkTopic       =   "Form2"
   ScaleHeight     =   3495
   ScaleWidth      =   6330
   StartUpPosition =   3  'Windows Default
   Visible         =   0   'False
   Begin VB.CommandButton btnCancel 
      Cancel          =   -1  'True
      Caption         =   "Cancel"
      Height          =   375
      Left            =   5280
      TabIndex        =   2
      Top             =   840
      Width           =   975
   End
   Begin VB.CommandButton btnOK 
      Caption         =   "OK"
      Default         =   -1  'True
      Height          =   375
      Left            =   5280
      TabIndex        =   1
      Top             =   240
      Width           =   975
   End
   Begin VB.ListBox List1 
      Height          =   2790
      Left            =   120
      TabIndex        =   0
      Top             =   120
      Width           =   4695
   End
End
Attribute VB_Name = "frmDevice"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Public SourceFilter As IFilterInfo
Public FilterName As String
Public Graph As IMediaControl
Private catlist As IVBCollection

Private Sub btnCancel_Click()
    Set SourceFilter = Nothing
    Unload Me
End Sub

Private Sub btnOK_Click()
    Set SourceFilter = Nothing
    If catlist Is Nothing Then Exit Sub
    If List1.ListIndex < 0 Then Exit Sub
    
    Dim f As IFilterClass
    Set f = catlist.Item(List1.ListIndex)
    FilterName = f.Name
    Set SourceFilter = f.Create(Graph)
    Unload Me
End Sub

Private Sub Form_Load()
    RefreshList
End Sub


Public Sub RefreshList()
    Dim catidVideoCapture As String
    catidVideoCapture = "{860BB310-5D01-11d0-BD3B-00A0C911CE86}"
    
    Dim fce As FilterCatEnumerator
    Set fce = New FilterCatEnumerator
    Set catlist = fce.Category(catidVideoCapture)
    
    List1.Clear
    Dim f As IFilterClass
    For Each f In catlist
        List1.AddItem f.Name
    Next f
End Sub

Private Sub List1_DblClick()
    btnOK_Click
End Sub
