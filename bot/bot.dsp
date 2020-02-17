# Microsoft Developer Studio Project File - Name="bot" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=bot - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "bot.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "bot.mak" CFG="bot - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "bot - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "bot - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "Perforce Project"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "bot - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "BOT_EXPORTS" /YX /FD /c
# ADD CPP /nologo /W3 /O2 /Ob2 /I "src/cpp" /I "src/cpp/extern/STLport/stlport" /I "../pathematics/src/cpp" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "BOT_EXPORTS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386 /nodefaultlib:"LIBCMT" /def:"src/cpp/linkage/bot.def" /out:"Release/whichbot.dll" /libpath:"lib"
# SUBTRACT LINK32 /nodefaultlib
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy Release\whichbot.dll f:\games\half-life\ns\addons\whichbot\dlls
# End Special Build Tool

!ELSEIF  "$(CFG)" == "bot - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "BOT_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "src/cpp/extern/STLport/stlport" /I "../pathematics/src/cpp" /I "src/cpp" /I "../bot/src/cpp/extern/STLport/stlport" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "BOT_EXPORTS" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /nodefaultlib:"LIBCMT" /nodefaultlib:"LIBCMTD" /def:"src/cpp/linkage/bot.def" /out:"Debug/whichbot.dll" /pdbtype:sept /libpath:"lib"
# SUBTRACT LINK32 /pdb:none /incremental:no
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy .\Debug\whichbot.dll g:\games\steam\steamapps\handdeath_ps@yahoo.com\Half-Life\ns\addons\whichbot\dlls	copy conf\*.pot g:\games\Steam\SteamApps\handdeath_ps@yahoo.com\Half-Life\ns\addons\whichbot\conf	copy conf\*.txt g:\games\Steam\SteamApps\handdeath_ps@yahoo.com\Half-Life\ns\addons\whichbot\conf	copy .\Debug\whichbot.dll g:\games\HLServer\ns\addons\whichbot\dlls	copy conf\*.pot g:\games\HLServer\ns\addons\whichbot\conf	copy conf\*.txt g:\games\HLServer\ns\addons\whichbot\conf
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "bot - Win32 Release"
# Name "bot - Win32 Debug"
# Begin Group "extern"

# PROP Default_Filter ""
# Begin Group "metamod"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\cpp\extern\metamod\dllapi.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\extern\metamod\dllapi_post.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\extern\metamod\dummy.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\extern\metamod\engine_api.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\extern\metamod\h_export.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cpp\extern\metamod\h_export.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\extern\metamod\info_name.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\extern\metamod\log_meta.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\extern\metamod\meta_api.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\extern\metamod\mhook.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\extern\metamod\mreg.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\extern\metamod\mutil.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\extern\metamod\osdep.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\extern\metamod\plinfo.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\extern\metamod\types_meta.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\extern\metamod\vdate.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\extern\metamod\vers_meta.h
# End Source File
# End Group
# Begin Group "halflifesdk"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\cpp\extern\halflifesdk\activity.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\extern\halflifesdk\basemonster.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\extern\halflifesdk\cbase.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\extern\halflifesdk\cdll_dll.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\extern\halflifesdk\const.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\extern\halflifesdk\crc.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\extern\halflifesdk\custom.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\extern\halflifesdk\cvardef.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\extern\halflifesdk\edict.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\extern\halflifesdk\effects.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\extern\halflifesdk\eiface.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\extern\halflifesdk\engine.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\extern\halflifesdk\enginecallback.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\extern\halflifesdk\entity_state.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\extern\halflifesdk\event_args.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\extern\halflifesdk\event_flags.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\extern\halflifesdk\extdll.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\extern\halflifesdk\in_buttons.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\extern\halflifesdk\monsterevent.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\extern\halflifesdk\player.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\extern\halflifesdk\pm_materials.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\extern\halflifesdk\progdefs.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\extern\halflifesdk\saverestore.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\extern\halflifesdk\schedule.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\extern\halflifesdk\usercmd.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\extern\halflifesdk\util.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cpp\extern\halflifesdk\util.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\extern\halflifesdk\vector.h
# End Source File
# End Group
# Begin Group "getpot"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\cpp\extern\getpot\GetPot.h
# End Source File
# End Group
# End Group
# Begin Group "combat"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\cpp\combat\CombatNavMethod.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cpp\combat\CombatNavMethod.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\combat\CombatStrategy.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cpp\combat\CombatStrategy.h
# End Source File
# End Group
# Begin Group "sensory"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\cpp\sensory\AuditoryManager.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cpp\sensory\AuditoryManager.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\sensory\AuditoryTrace.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cpp\sensory\AuditoryTrace.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\sensory\BotSensor.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cpp\sensory\BotSensor.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\sensory\Target.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cpp\sensory\Target.h
# End Source File
# End Group
# Begin Group "strategy"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\cpp\strategy\AmbushStrategy.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cpp\strategy\AmbushStrategy.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\strategy\AttackStrategy.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cpp\strategy\AttackStrategy.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\strategy\BuilderStrategy.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cpp\strategy\BuilderStrategy.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\strategy\FleeStrategy.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cpp\strategy\FleeStrategy.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\strategy\HiveMind.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cpp\strategy\HiveMind.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\strategy\PackFollowerStrategy.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cpp\strategy\PackFollowerStrategy.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\strategy\PackInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cpp\strategy\PackInfo.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\strategy\PackLeaderStrategy.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cpp\strategy\PackLeaderStrategy.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\strategy\PackManager.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cpp\strategy\PackManager.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\strategy\RescueStrategy.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cpp\strategy\RescueStrategy.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\strategy\ScoutStrategy.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cpp\strategy\ScoutStrategy.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\strategy\Strategy.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cpp\strategy\Strategy.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\strategy\StrategyManager.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cpp\strategy\StrategyManager.h
# End Source File
# End Group
# Begin Group "navigation"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\cpp\navigation\BotMovement.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cpp\navigation\BotMovement.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\navigation\BuildNavMethod.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cpp\navigation\BuildNavMethod.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\navigation\NavigationEngine.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cpp\navigation\NavigationEngine.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\navigation\NavigationMethod.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cpp\navigation\NavigationMethod.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\navigation\WaypointDebugger.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cpp\navigation\WaypointDebugger.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\navigation\WaypointManager.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cpp\navigation\WaypointManager.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\navigation\WaypointNavMethod.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cpp\navigation\WaypointNavMethod.h
# End Source File
# End Group
# Begin Group "linkage"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\cpp\linkage\dllapi.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cpp\linkage\dllapi.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\linkage\engine_api.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cpp\linkage\engine_intercepts.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cpp\linkage\engine_intercepts.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\linkage\linkfunc.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cpp\linkage\linkfunc.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\linkage\meta_api.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cpp\linkage\plugin.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cpp\linkage\version.h
# End Source File
# End Group
# Begin Group "message"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\cpp\message\ClientCommandDispatcher.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cpp\message\ClientCommandDispatcher.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\message\ClientCommandHandlers.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cpp\message\NetMessage.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cpp\message\NetMessage.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\message\NetMessageDispatcher.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cpp\message\NetMessageDispatcher.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\message\NetMessageElement.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cpp\message\NetMessageElement.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\message\NetMessageHandlers.cpp
# End Source File
# End Group
# Begin Group "framework"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\cpp\framework\Log.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cpp\framework\Log.h
# End Source File
# End Group
# Begin Group "config"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\cpp\config\Config.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cpp\config\Config.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\config\EntityInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cpp\config\EntityInfo.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\config\TranslationManager.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cpp\config\TranslationManager.h
# End Source File
# End Group
# Begin Group "worldstate"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\cpp\worldstate\AreaInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cpp\worldstate\AreaInfo.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\worldstate\AreaManager.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cpp\worldstate\AreaManager.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\worldstate\EntityReference.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cpp\worldstate\EntityReference.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\worldstate\HiveInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cpp\worldstate\HiveInfo.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\worldstate\HiveManager.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cpp\worldstate\HiveManager.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\worldstate\WorldStateUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cpp\worldstate\WorldStateUtil.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\src\cpp\Bot.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cpp\Bot.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\BotManager.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cpp\BotManager.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\BotTypedefs.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\NSConstants.h
# End Source File
# Begin Source File

SOURCE=.\src\cpp\StatusReporter.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cpp\StatusReporter.h
# End Source File
# End Target
# End Project
