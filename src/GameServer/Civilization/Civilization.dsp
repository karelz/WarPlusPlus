# Microsoft Developer Studio Project File - Name="Civilization" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=Civilization - Win32 Debug Static
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Civilization.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Civilization.mak" CFG="Civilization - Win32 Debug Static"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Civilization - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Civilization - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "Civilization - Win32 Debug Static" (based on "Win32 (x86) Static Library")
!MESSAGE "Civilization - Win32 Release No Optimizations" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Civilization", BCAAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Civilization - Win32 Release"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Civilization - Win32 Debug"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "_WINDOWS" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Civilization - Win32 Debug Static"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Civilization___Win32_Debug_Static"
# PROP BASE Intermediate_Dir "Civilization___Win32_Debug_Static"
# PROP BASE Target_Dir ""
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug_Static"
# PROP Intermediate_Dir "Debug_Static"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "_WINDOWS" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "_WINDOWS" /D "WIN32" /D "_DEBUG" /D "_MBCS" /Yu"stdafx.h" /FD /GZ /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Civilization - Win32 Release No Optimizations"

# PROP BASE Use_MFC 1
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Civilization___Win32_Release_No_Optimizations"
# PROP BASE Intermediate_Dir "Civilization___Win32_Release_No_Optimizations"
# PROP BASE Target_Dir ""
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release_No_Optimizations"
# PROP Intermediate_Dir "Release_No_Optimizations"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /Zi /Od /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x405 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "Civilization - Win32 Release"
# Name "Civilization - Win32 Debug"
# Name "Civilization - Win32 Debug Static"
# Name "Civilization - Win32 Release No Optimizations"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Src\LogFile\LogFile.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\MiniMap\MiniMapServer.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\stdafx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\Src\ZCivilization.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\ClientCommunication\ZClientUnitInfoSender.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\ZConnectedClient.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\ZLogOutput.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\MapDownload\ZMapDownload.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\MiniMap\ZMiniMapRequest.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\ClientCommunication\ZRequestVirtualConnection.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\ZScriptCompilerErrorOutput.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\ZScriptEditor.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\ZScriptErrorOutput.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\ClientCommunication\ZTimestampedVirtualConnection.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\ZUnit.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\ZUnitCommandQueueItem.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\ZUnitNotifyQueueItem.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\ClientCommunication\ZWatchedRectangle.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\Src\LogFile\LogFile.h
# End Source File
# Begin Source File

SOURCE=.\Src\MiniMap\MiniMapServer.h
# End Source File
# Begin Source File

SOURCE=.\SkillTypeName.h
# End Source File
# Begin Source File

SOURCE=.\Src\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\ZCivilization.h
# End Source File
# Begin Source File

SOURCE=.\Src\ClientCommunication\ZClientUnitInfoSender.h
# End Source File
# Begin Source File

SOURCE=.\Src\ZConnectedClient.h
# End Source File
# Begin Source File

SOURCE=.\Src\ZLogOutput.h
# End Source File
# Begin Source File

SOURCE=.\Src\MapDownload\ZMapDownload.h
# End Source File
# Begin Source File

SOURCE=.\Src\MiniMap\ZMiniMapRequest.h
# End Source File
# Begin Source File

SOURCE=.\Src\ClientCommunication\ZRequestVirtualConnection.h
# End Source File
# Begin Source File

SOURCE=.\Src\ZScriptCompilerErrorOutput.h
# End Source File
# Begin Source File

SOURCE=.\Src\ZScriptEditor.h
# End Source File
# Begin Source File

SOURCE=.\Src\ZScriptErrorOutput.h
# End Source File
# Begin Source File

SOURCE=.\Src\ClientCommunication\ZTimestampedVirtualConnection.h
# End Source File
# Begin Source File

SOURCE=.\Src\ZUnit.h
# End Source File
# Begin Source File

SOURCE=.\Src\ZUnitCommandQueueItem.h
# End Source File
# Begin Source File

SOURCE=.\Src\ZUnitNotifyQueueItem.h
# End Source File
# Begin Source File

SOURCE=.\Src\ClientCommunication\ZWatchedRectangle.h
# End Source File
# End Group
# End Target
# End Project
