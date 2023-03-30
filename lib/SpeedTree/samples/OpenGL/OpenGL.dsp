# Microsoft Developer Studio Project File - Name="OpenGL" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=OpenGL - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "OpenGL.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "OpenGL.mak" CFG="OpenGL - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "OpenGL - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "OpenGL - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "OpenGL - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release VC 6.0"
# PROP Intermediate_Dir "Release VC 6.0"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "..\Common Source\\" /I "..\Common Source\SpeedWind\\" /I "." /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /FR /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib SpeedTreeRT.lib /nologo /subsystem:console /machine:I386 /out:"..\..\bin\VC 6.0\Release\SpeedTreeRT OpenGL.exe" /libpath:"..\..\lib\VC 6.0\Release"

!ELSEIF  "$(CFG)" == "OpenGL - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "OpenGL___Win32_Debug0"
# PROP BASE Intermediate_Dir "OpenGL___Win32_Debug0"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Debug VC 6.0"
# PROP Intermediate_Dir "Debug VC 6.0"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /I "../Common Source/" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /FR /YX /FD /c
# ADD CPP /nologo /MDd /W3 /GX /Zi /Od /I "..\Common Source\\" /I "..\Common Source\SpeedWind\\" /I "." /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /FAs /FR /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib SpeedTreeRT_OpenGL.lib /nologo /subsystem:console /machine:I386 /out:"..\..\bin\Release.VS6\SpeedTreeRT OpenGL.exe" /libpath:"..\..\lib\Release.VS6"
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib SpeedTreeRT.lib /nologo /subsystem:console /map /debug /machine:I386 /out:"..\..\bin\VC 6.0\Debug\SpeedTreeRT OpenGL.exe" /libpath:"..\..\lib\VC 6.0\Release"

!ENDIF 

# Begin Target

# Name "OpenGL - Win32 Release"
# Name "OpenGL - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE="..\Common Source\extgl.c"
# End Source File
# Begin Source File

SOURCE="..\Common Source\IdvVertexBuffer.cpp"
# End Source File
# Begin Source File

SOURCE=.\main.cpp
# End Source File
# Begin Source File

SOURCE="..\Common Source\Navigation.cpp"
# End Source File
# Begin Source File

SOURCE="..\Common Source\nv_dds.cpp"
# End Source File
# Begin Source File

SOURCE="..\Common Source\SpeedTreeForest.cpp"
# End Source File
# Begin Source File

SOURCE=.\SpeedTreeForestOpenGL.cpp
# End Source File
# Begin Source File

SOURCE=.\SpeedTreeWrapper.cpp
# End Source File
# Begin Source File

SOURCE="..\Common Source\Transform.cpp"
# End Source File
# Begin Source File

SOURCE="..\Common Source\VertexProgram.cpp"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE="..\Common Source\extgl.h"
# End Source File
# Begin Source File

SOURCE="..\Common Source\Filename.h"
# End Source File
# Begin Source File

SOURCE="..\Common Source\glut.h"
# End Source File
# Begin Source File

SOURCE="..\Common Source\IdvVertexBuffer.h"
# End Source File
# Begin Source File

SOURCE="..\Common Source\nv_dds.h"
# End Source File
# Begin Source File

SOURCE="..\Common Source\Random.h"
# End Source File
# Begin Source File

SOURCE=.\SpeedTreeConfig.h
# End Source File
# Begin Source File

SOURCE="..\Common Source\SpeedTreeForest.h"
# End Source File
# Begin Source File

SOURCE=.\SpeedTreeForestOpenGL.h
# End Source File
# Begin Source File

SOURCE=.\SpeedTreeMaterial.h
# End Source File
# Begin Source File

SOURCE=.\SpeedTreeWrapper.h
# End Source File
# Begin Source File

SOURCE="..\Common Source\Transform.h"
# End Source File
# Begin Source File

SOURCE="..\Common Source\Vector.h"
# End Source File
# Begin Source File

SOURCE="..\Common Source\VertexProgram.h"
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Group "Vertex Programs"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\VP_Basic_DynamicLighting_GpuWind.h
# End Source File
# Begin Source File

SOURCE=.\VP_Basic_StaticLighting_GpuWind.h
# End Source File
# Begin Source File

SOURCE=.\VP_Leaves_DynamicLighting_GpuPlacement_CpuWind.h
# End Source File
# Begin Source File

SOURCE=.\VP_Leaves_DynamicLighting_GpuPlacement_GpuWind.h
# End Source File
# Begin Source File

SOURCE=.\VP_Leaves_StaticLighting_GpuPlacement_CpuWind.h
# End Source File
# Begin Source File

SOURCE=.\VP_Leaves_StaticLighting_GpuPlacement_GpuWind.h
# End Source File
# End Group
# Begin Group "SpeedWind"

# PROP Default_Filter ""
# Begin Source File

SOURCE="..\Common Source\SpeedWind\PIDController.h"
# End Source File
# Begin Source File

SOURCE="..\Common Source\SpeedWind\SpeedWind.cpp"
# End Source File
# Begin Source File

SOURCE="..\Common Source\SpeedWind\SpeedWind.h"
# End Source File
# Begin Source File

SOURCE="..\Common Source\SpeedWind\SpeedWindParser.cpp"
# End Source File
# Begin Source File

SOURCE="..\Common Source\SpeedWind\SpeedWindParser.h"
# End Source File
# End Group
# End Target
# End Project
