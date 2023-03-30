# Microsoft Developer Studio Project File - Name="RT_StaticLib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=RT_StaticLib - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "RT_StaticLib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "RT_StaticLib.mak" CFG="RT_StaticLib - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "RT_StaticLib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "RT_StaticLib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "RT_StaticLib - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "RT_StaticLib___Win32_OpenGL_Release"
# PROP BASE Intermediate_Dir "RT_StaticLib___Win32_OpenGL_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release VC 6.0"
# PROP Intermediate_Dir "Release VC 6.0"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "SPEEDTREE_DATA_FORMAT_NETIMMERSE" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /FR /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"NetImmerse Release\SpeedTreeRT_NetImmerse.lib"
# ADD LIB32 /nologo /out:"../../lib/VC 6.0/Release/SpeedTreeRT_Static.lib"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=Copying header file
PostBuild_Cmds=copy "..\SourceCode\SpeedTreeRT.h" ..\..\include
# End Special Build Tool

!ELSEIF  "$(CFG)" == "RT_StaticLib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "RT_StaticLib___Win32_OpenGL_Debug"
# PROP BASE Intermediate_Dir "RT_StaticLib___Win32_OpenGL_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug VC 6.0"
# PROP Intermediate_Dir "Debug VC 6.0"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "SPEEDTREE_DATA_FORMAT_NETIMMERSE" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FR /YX /FD /GZ /c
# SUBTRACT CPP /WX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"NetImmerse Debug\SpeedTreeRT_NetImmerse_d.lib"
# ADD LIB32 /nologo /out:"../../lib/VC 6.0/Debug/SpeedTreeRT_Static_d.lib"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=Copying header file
PostBuild_Cmds=copy "..\SourceCode\SpeedTreeRT.h" ..\..\include
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "RT_StaticLib - Win32 Release"
# Name "RT_StaticLib - Win32 Debug"
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

SOURCE=..\SourceCode\IdvFastMath.h
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

SOURCE=..\SourceCode\WindEngine.h
# End Source File
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
