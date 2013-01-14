# Microsoft Developer Studio Project File - Name="GMFBridge" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=GMFBridge - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "GMFBridge.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "GMFBridge.mak" CFG="GMFBridge - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "GMFBridge - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "GMFBridge - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "GMFBridge - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /I "..\..\..\BaseClasses" /ZI /W3 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "GMFBRIDGE_EXPORTS" /D "_MBCS" /Gm /Yu"stdafx.h" /Gz /GZ /c /GX 
# ADD CPP /nologo /MDd /I "..\..\..\BaseClasses" /ZI /W3 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "GMFBRIDGE_EXPORTS" /D "_MBCS" /Gm /Yu"stdafx.h" /Gz /GZ /c /GX 
# ADD BASE MTL /nologo /win32 
# ADD MTL /nologo /win32 
# ADD BASE RSC /l 1033 
# ADD RSC /l 1033 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo 
# ADD BSC32 /nologo 
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ..\..\..\lib\DecklinkInterface.lib winmm.lib /nologo /dll /out:"Debug\GMFBridge.dll" /incremental:yes /def:"GMFBridge.def" /debug /pdb:"Debug\GMFBridge.pdb" /pdbtype:sept /subsystem:windows /implib:"$(OutDir)/GMFBridge.lib" /machine:ix86 
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ..\..\..\lib\DecklinkInterface.lib winmm.lib /nologo /dll /out:"Debug\GMFBridge.dll" /incremental:yes /def:"GMFBridge.def" /debug /pdb:"Debug\GMFBridge.pdb" /pdbtype:sept /subsystem:windows /implib:"$(OutDir)/GMFBridge.lib" /machine:ix86 
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=if not exist "..\..\..\include" mkdir "..\..\..\include"	copy $(TargetName)_h.h "..\..\..\include"	if not exist "..\..\..\bin" mkdir "..\..\..\bin"	copy $(OutDir)\$(TargetFileName) "..\..\..\bin"	echo regsvr32 $(TargetFileName) > "..\..\..\bin\register.bat"	echo regsvr32 /u $(TargetFileName) > "..\..\..\bin\unregister.bat"	
# End Special Build Tool

!ELSEIF  "$(CFG)" == "GMFBridge - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /I "..\..\..\BaseClasses" /Zi /W3 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "GMFBRIDGE_EXPORTS" /D "_MBCS" /Yu"stdafx.h" /Gz /c /GX 
# ADD CPP /nologo /MD /I "..\..\..\BaseClasses" /Zi /W3 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "GMFBRIDGE_EXPORTS" /D "_MBCS" /Yu"stdafx.h" /Gz /c /GX 
# ADD BASE MTL /nologo /win32 
# ADD MTL /nologo /win32 
# ADD BASE RSC /l 1033 
# ADD RSC /l 1033 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo 
# ADD BSC32 /nologo 
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ..\..\..\lib\DecklinkInterface.lib winmm.lib /nologo /dll /out:"Release\GMFBridge.dll" /incremental:no /def:"GMFBridge.def" /debug /pdbtype:sept /subsystem:windows /opt:ref /opt:icf /implib:"$(OutDir)/GMFBridge.lib" /machine:ix86 
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ..\..\..\lib\DecklinkInterface.lib winmm.lib /nologo /dll /out:"Release\GMFBridge.dll" /incremental:no /def:"GMFBridge.def" /debug /pdbtype:sept /subsystem:windows /opt:ref /opt:icf /implib:"$(OutDir)/GMFBridge.lib" /machine:ix86 
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=if not exist "..\..\..\include" mkdir "..\..\..\include"	copy $(TargetName)_h.h "..\..\..\include"	if not exist "..\..\..\bin" mkdir "..\..\..\bin"	copy $(OutDir)\$(TargetFileName) "..\..\..\bin"	echo regsvr32 $(TargetFileName) > "..\..\..\bin\register.bat"	echo regsvr32 /u $(TargetFileName) > "..\..\..\bin\unregister.bat"	
# End Special Build Tool

!ENDIF

# Begin Target

# Name "GMFBridge - Win32 Debug"
# Name "GMFBridge - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;def;odl;idl;hpj;bat;asm;asmx"
# Begin Source File

SOURCE=.\Bridge.cpp
# End Source File
# Begin Source File

SOURCE=.\CP.cpp
# End Source File
# Begin Source File

SOURCE=.\GMFBridge.cpp
# End Source File
# Begin Source File

SOURCE=.\GMFBridge.def
# End Source File
# Begin Source File

SOURCE=.\GMFBridge.idl

!IF  "$(CFG)" == "GMFBridge - Win32 Debug"

# ADD MTL /nologo /tlb"$(ProjectDir)\$(ProjectName).tlb" /win32 
!ELSEIF  "$(CFG)" == "GMFBridge - Win32 Release"

# ADD MTL /nologo /tlb"$(ProjectDir)\$(ProjectName).tlb" /win32 
!ENDIF

# End Source File
# Begin Source File

SOURCE=.\sink.cpp
# End Source File
# Begin Source File

SOURCE=.\source.cpp
# End Source File
# Begin Source File

SOURCE=.\stdafx.cpp

!IF  "$(CFG)" == "GMFBridge - Win32 Debug"

# ADD CPP /nologo /Yc"stdafx.h" /GZ /GX 
!ELSEIF  "$(CFG)" == "GMFBridge - Win32 Release"

# ADD CPP /nologo /Yc"stdafx.h" /GX 
!ENDIF

# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;inc;xsd"
# Begin Source File

SOURCE=.\Bridge.h
# End Source File
# Begin Source File

SOURCE=.\CP.h
# End Source File
# Begin Source File

SOURCE=.\dispatch.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\sink.h
# End Source File
# Begin Source File

SOURCE=.\SmartPtr.h
# End Source File
# Begin Source File

SOURCE=.\source.h
# End Source File
# Begin Source File

SOURCE=.\stdafx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "rc;ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe;resx"
# Begin Source File

SOURCE=.\GMFBridge.rc
# End Source File
# End Group
# Begin Source File

SOURCE=.\Changes.txt
# End Source File
# End Target
# End Project

