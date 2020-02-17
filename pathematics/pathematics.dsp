# Microsoft Developer Studio Project File - Name="pathematics" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=pathematics - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "pathematics.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "pathematics.mak" CFG="pathematics - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "pathematics - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "pathematics - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "pathematics - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "pathematics___Win32_Release"
# PROP BASE Intermediate_Dir "pathematics___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "pathematics___Win32_Release"
# PROP Intermediate_Dir "pathematics___Win32_Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "src/cpp" /I "../bot/src/cpp/extern/STLport/stlport" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# SUBTRACT CPP /Z<none>
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "pathematics - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "pathematics___Win32_Debug"
# PROP BASE Intermediate_Dir "pathematics___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "pathematics___Win32_Debug"
# PROP Intermediate_Dir "pathematics___Win32_Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "src/cpp" /I "../bot/src/cpp/extern/STLport/stlport" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "pathematics - Win32 Release"
# Name "pathematics - Win32 Debug"
# Begin Group "engine"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\cpp\engine\NodeIdBag.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cpp\engine\NodeIdBag.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\engine\PathData.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cpp\engine\PathData.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\engine\PathManager.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cpp\engine\PathManager.h
# End Source File
# End Group
# Begin Group "model"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\cpp\model\Digraph.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cpp\model\Digraph.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\model\Edge.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cpp\model\Edge.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\model\Node.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cpp\model\Node.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\engine\Reward.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\model\TerrainData.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cpp\model\TerrainData.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\model\Vector3D.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\src\cpp\PathematicsTypedefs.h
# End Source File
# End Target
# End Project
