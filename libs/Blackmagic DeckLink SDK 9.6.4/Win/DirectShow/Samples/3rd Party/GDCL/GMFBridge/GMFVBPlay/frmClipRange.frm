VERSION 5.00
Begin VB.Form frmClipRange 
   Caption         =   "Set Clip Range"
   ClientHeight    =   2745
   ClientLeft      =   60
   ClientTop       =   450
   ClientWidth     =   5865
   LinkTopic       =   "Form2"
   ScaleHeight     =   2745
   ScaleWidth      =   5865
   StartUpPosition =   3  'Windows Default
   Begin VB.TextBox txtStop 
      Height          =   495
      Left            =   1920
      TabIndex        =   4
      Text            =   "0"
      Top             =   1680
      Width           =   1455
   End
   Begin VB.TextBox txtStart 
      Height          =   495
      Left            =   1920
      TabIndex        =   2
      Text            =   "0"
      Top             =   960
      Width           =   1455
   End
   Begin VB.CommandButton btnCancel 
      Cancel          =   -1  'True
      Caption         =   "&Cancel"
      Height          =   375
      Left            =   4680
      TabIndex        =   6
      Top             =   600
      Width           =   1095
   End
   Begin VB.CommandButton btnOK 
      Caption         =   "&OK"
      Default         =   -1  'True
      Height          =   375
      Left            =   4680
      TabIndex        =   5
      Top             =   120
      Width           =   1095
   End
   Begin VB.Label lblStop 
      Caption         =   "Sto&p"
      Height          =   255
      Left            =   120
      TabIndex        =   3
      Top             =   1800
      Width           =   1575
   End
   Begin VB.Label lblStart 
      Caption         =   "S&tart"
      Height          =   255
      Left            =   120
      TabIndex        =   1
      Top             =   1080
      Width           =   1575
   End
   Begin VB.Label lblDuration 
      Caption         =   "Duration: Unknown"
      Height          =   375
      Left            =   120
      TabIndex        =   0
      Top             =   240
      Width           =   4335
   End
End
Attribute VB_Name = "frmClipRange"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Public duration As Double
Public tStart As Double
Public tStop As Double

Private Sub btnCancel_Click()
    tStart = 0
    tStop = 0
    Unload Me
End Sub

Private Sub btnOK_Click()
    tStart = Val(txtStart.Text)
    tStop = Val(txtStop.Text)
    Unload Me
End Sub

Private Sub Form_Load()
    tStart = 0
    tStop = 0
    lblDuration.Caption = "Duration: " + Str$(Round(duration, 1))
    txtStart.Text = 0
    txtStop.Text = 0
End Sub
