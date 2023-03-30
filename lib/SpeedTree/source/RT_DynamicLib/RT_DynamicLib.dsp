# Microsoft Developer Studio Project File - Name="RT_DynamicLib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=RT_DynamicLib - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "RT_DynamicLib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "RT_DynamicLib.mak" CFG="RT_DynamicLib - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "RT_DynamicLib - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "RT_DynamicLib - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "RT_DynamicLib - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release VC 6.0"
# PROP Intermediate_Dir "Release VC 6.0"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "IDV_SPEEDTREERT_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "IDV_SPEEDTREERT_EXPORTS" /D "SPEEDTREERT_DYNAMIC_LIB" /D "SPEEDTREE_EVAL" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386 /out:"Release/SpeedTreeRT.dll"
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386 /out:"../../bin/VC 6.0/Release/SpeedTreeRT.dll" /implib:"../../lib/VC 6.0/Release/SpeedTreeRT.lib"
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=Copying header file
PostBuild_Cmds=copy "..\SourceCode\SpeedTreeRT.h" ..\..\include
# End Special Build Tool

!ELSEIF  "$(CFG)" == "RT_DynamicLib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug VC 6.0"
# PROP Intermediate_Dir "Debug VC 6.0"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "RT_DYNAMICLIB_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "IDV_SPEEDTREERT_EXPORTS" /D "SPEEDTREERT_DYNAMIC_LIB" /D "SPEEDTREE_EVAL" /FAs /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /map /debug /machine:I386 /out:"../../bin/VC 6.0/Debug/SpeedTreeRT_d.dll" /implib:"../../lib/VC 6.0/Debug/SpeedTreeRT_d.lib" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=Copying header file
PostBuild_Cmds=copy "..\SourceCode\SpeedTreeRT.h" ..\..\include
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "RT_DynamicLib - Win32 Release"
# Name "RT_DynamicLib - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\SourceCode\BillboardLeaf.cpp
# End Source File
# Begin Source File

SOURCE=..\SourceCode\Branch.cpp
# End Source File
# Begin Source File

SOURCE=..\SourceCode\BranchInfo.cpp
# End Source File
# Begin Source File

SOURCE=..\SourceCode\Camera.cpp
# End Source File
# Begin Source File

SOURCE=..\SourceCode\FileAccess.cpp
# End Source File
# Begin Source File

SOURCE=..\SourceCode\FrondEngine.cpp
# End Source File
# Begin Source File

SOURCE=..\SourceCode\IndexedGeometry.cpp
# End Source File
# Begin Source File

SOURCE=..\SourceCode\LeafGeometry.cpp
# End Source File
# Begin Source File

SOURCE=..\SourceCode\LeafInfo.cpp
# End Source File
# Begin Source File

SOURCE=..\SourceCode\LeafLod.cpp
# End Source File
# Begin Source File

SOURCE=..\SourceCode\LightingEngine.cpp
# End Source File
# Begin Source File

SOURCE=..\SourceCode\ProjectedShadow.cpp
# End Source File
# Begin Source File

SOURCE=..\SourceCode\SimpleBillboard.cpp
# End Source File
# Begin Source File

SOURCE=..\SourceCode\SpeedTreeRT.cpp
# End Source File
# Begin Source File

SOURCE=..\SourceCode\TreeEngine.cpp
# End Source File
# Begin Source File

SOURCE=..\SourceCode\TreeInfo.cpp
# End Source File
# Begin Source File

SOURCE=..\SourceCode\WindEngine.cpp
# End Source File
# Begin Source File

SOURCE=..\SourceCode\WindInfo.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\SourceCode\BillboardLeaf.h
# End Source File
# Begin Source File

SOURCE=..\SourceCode\Debug.h
# End Source File
# Begin Source File

SOURCE=..\SourceCode\Endian.h
# End Source File
# Begin Source File

SOURCE=..\SourceCode\FileAccess.h
# End Source File
# Begin Source File

SOURCE=..\SourceCode\FrondEngine.h
# End Source File
# Begin Source File

SOURCE=..\SourceCode\IndexedGeometry.h
# End Source File
# Begin Source File

SOURCE=..\SourceCode\Instances.h
# End Source File
# Begin Source File

SOURCE=..\SourceCode\LeafGeometry.h
# End Source File
# Begin Source File

SOURCE=..\SourceCode\LeafLod.h
# End Source File
# Begin Source File

SOURCE=..\SourceCode\LightingEngine.h
# End Source File
# Begin Source File

SOURCE=..\SourceCode\ProjectedShadow.h
# End Source File
# Begin Source File

SOURCE=..\SourceCode\SimpleBillboard.h
# End Source File
# Begin Source File

SOURCE=..\SourceCode\SpeedTreeRT.h
# End Source File
# Begin Source File

SOURCE=..\SourceCode\StructsInfo.h
# End Source File
# Begin Source File

SOURCE=..\SourceCode\StructsSupport.h
# End Source File
# Begin Source File

SOURCE=..\SourceCode\TreeEngine.h
# End Source File
# Begin Source File

SOURCE=..\SourceCode\UpVector.h
# End Source File
# Begin Source File

SOURCE=..\SourceCode\WindEngine.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Group "LibSpline_Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\SourceCode\LibSpline_Source\IdvSpline.cpp
# End Source File
# Begin Source File

SOURCE=..\SourceCode\LibSpline_Source\IdvSpline.h
# End Source File
# End Group
# Begin Group "LibRandom_Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\SourceCode\LibRandom_Source\ExtendedReal.cpp
# End Source File
# Begin Source File

SOURCE=..\SourceCode\LibRandom_Source\ExtendedReal.h
# End Source File
# Begin Source File

SOURCE=..\SourceCode\LibRandom_Source\IdvRandom.cpp
# End Source File
# Begin Source File

SOURCE=..\SourceCode\LibRandom_Source\IdvRandom.h
# End Source File
# Begin Source File

SOURCE=..\SourceCode\LibRandom_Source\RobertDavies_Random.cpp
# End Source File
# Begin Source File

SOURCE=..\SourceCode\LibRandom_Source\RobertDavies_Random.h
# End Source File
# End Group
# Begin Group "LibGlobals_Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\SourceCode\LibGlobals_Source\IdvGlobals.h
# End Source File
# End Group
# Begin Group "LibFilename_Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\SourceCode\LibFilename_Source\IdvFilename.h
# End Source File
# End Group
# Begin Group "LibVector_Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\SourceCode\LibVector_Source\IdvFastMath.h
# End Source File
# Begin Source File

SOURCE=..\SourceCode\LibVector_Source\IdvVector.h
# End Source File
# Begin Source File

SOURCE=..\SourceCode\LibVector_Source\Region.cpp
# End Source File
# Begin Source File

SOURCE=..\SourceCode\LibVector_Source\RotTransform.cpp
# End Source File
# Begin Source File

SOURCE=..\SourceCode\LibVector_Source\Transform.cpp
# End Source File
# Begin Source File

SOURCE=..\SourceCode\LibVector_Source\Vec.cpp
# End Source File
# Begin Source File

SOURCE=..\SourceCode\LibVector_Source\Vec3.cpp
# End Source File
# End Group
# End Target
# End Project
