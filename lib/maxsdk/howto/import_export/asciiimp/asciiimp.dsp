# Microsoft Developer Studio Project File - Name="asciiimp" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=asciiimp - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "asciiimp.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "asciiimp.mak" CFG="asciiimp - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "asciiimp - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "asciiimp - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "asciiimp - Win32 Hybrid" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "asciiimp"
# PROP Scc_LocalPath "..\..\..\.."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "asciiimp - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release"
# PROP BASE Intermediate_Dir ".\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Release"
# PROP Intermediate_Dir ".\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /G6 /MD /W3 /O2 /I "\maxsdk\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX"asciiimp.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"\3dsmax\plugins\asciiimp.dli"

!ELSEIF  "$(CFG)" == "asciiimp - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Debug"
# PROP BASE Intermediate_Dir ".\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Debug"
# PROP Intermediate_Dir ".\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /G6 /MDd /W3 /Gm /ZI /Od /I "c:\devel\3dswin\src\maxsdk\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX"asciiimp.h" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /subsystem:windows /dll /pdb:"asciiimp.pdb" /debug /machine:I386 /out:"c:\devel\3dswin\src\exe\plugins\asciiimp.dli" /libpath:"c:\devel\3dswin\src\maxsdk\lib"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "asciiimp - Win32 Hybrid"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Hybrid"
# PROP BASE Intermediate_Dir ".\Hybrid"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ""
# PROP Intermediate_Dir ""
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /G6 /MD /W3 /Gm /ZI /Od /I "$(R4SDKREL)\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX"asciiimp.h" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"$(R4EXEREL)\plugins\asciiimp.dli" /libpath:"$(R4SDKREL)\lib"

!ENDIF 

# Begin Target

# Name "asciiimp - Win32 Release"
# Name "asciiimp - Win32 Debug"
# Name "asciiimp - Win32 Hybrid"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=.\asciiimp.cpp
# End Source File
# Begin Source File

SOURCE=.\asciiimp.def
# End Source File
# Begin Source File

SOURCE=.\asciiimp.rc
# End Source File
# Begin Source File

SOURCE=.\import.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=.\asciiimp.h
# End Source File
# Begin Source File

SOURCE=.\ASCIITOK.H
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# End Group
# Begin Group "Lib"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\..\devel\3dswin\src\maxsdk\lib\zlibdll.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\..\devel\3dswin\src\maxsdk\lib\bmm.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\..\devel\3dswin\src\maxsdk\lib\client.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\..\devel\3dswin\src\maxsdk\lib\core.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\..\devel\3dswin\src\maxsdk\lib\CustDlg.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\..\devel\3dswin\src\maxsdk\lib\edmodel.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\..\devel\3dswin\src\maxsdk\lib\expr.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\..\devel\3dswin\src\maxsdk\lib\FLILIBD.LIB
# End Source File
# Begin Source File

SOURCE=..\..\..\..\devel\3dswin\src\maxsdk\lib\FLILIBH.LIB
# End Source File
# Begin Source File

SOURCE=..\..\..\..\devel\3dswin\src\maxsdk\lib\FLILIBR.LIB
# End Source File
# Begin Source File

SOURCE=..\..\..\..\devel\3dswin\src\maxsdk\lib\flt.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\..\devel\3dswin\src\maxsdk\lib\gcomm.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\..\devel\3dswin\src\maxsdk\lib\gcomm2.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\..\devel\3dswin\src\maxsdk\lib\geom.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\..\devel\3dswin\src\maxsdk\lib\gfx.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\..\devel\3dswin\src\maxsdk\lib\gup.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\..\devel\3dswin\src\maxsdk\lib\helpsys.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\..\devel\3dswin\src\maxsdk\lib\imageViewers.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\..\devel\3dswin\src\maxsdk\lib\ManipSys.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\..\devel\3dswin\src\maxsdk\lib\maxnet.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\..\devel\3dswin\src\maxsdk\lib\Maxscrpt.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\..\devel\3dswin\src\maxsdk\lib\maxutil.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\..\devel\3dswin\src\maxsdk\lib\MenuMan.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\..\devel\3dswin\src\maxsdk\lib\menus.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\..\devel\3dswin\src\maxsdk\lib\mesh.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\..\devel\3dswin\src\maxsdk\lib\MNMath.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\..\devel\3dswin\src\maxsdk\lib\Paramblk2.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\..\devel\3dswin\src\maxsdk\lib\particle.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\..\devel\3dswin\src\maxsdk\lib\Poly.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\..\devel\3dswin\src\maxsdk\lib\RenderUtil.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\..\devel\3dswin\src\maxsdk\lib\SpringSys.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\..\devel\3dswin\src\maxsdk\lib\tessint.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\..\devel\3dswin\src\maxsdk\lib\viewfile.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\..\devel\3dswin\src\maxsdk\lib\acap.lib
# End Source File
# End Group
# End Target
# End Project
