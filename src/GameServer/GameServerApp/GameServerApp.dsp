# Microsoft Developer Studio Project File - Name="GameServerApp" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=GameServerApp - Win32 Debug Static
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "GameServerApp.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "GameServerApp.mak" CFG="GameServerApp - Win32 Debug Static"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "GameServerApp - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "GameServerApp - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "GameServerApp - Win32 Debug Static" (based on "Win32 (x86) Application")
!MESSAGE "GameServerApp - Win32 Release No Optimizations" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/GameServer/GameServerApp", UNAAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "GameServerApp - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 ws2_32.lib /nologo /subsystem:windows /machine:IX86 /nodefaultlib:"MSVCRT" /nodefaultlib:"LIBC"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "GameServerApp - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ws2_32.lib /nologo /subsystem:windows /profile /map /debug /machine:I386 /nodefaultlib:"LIBCD"

!ELSEIF  "$(CFG)" == "GameServerApp - Win32 Debug Static"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "GameServerApp___Win32_Debug_Static"
# PROP BASE Intermediate_Dir "GameServerApp___Win32_Debug_Static"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug_Static"
# PROP Intermediate_Dir "Debug_Static"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /GZ /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ws2_32.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"LIBCD" /pdbtype:sept
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 ws2_32.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"LIBCD" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "GameServerApp - Win32 Release No Optimizations"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "GameServerApp___Win32_Release_No_Optimizations"
# PROP BASE Intermediate_Dir "GameServerApp___Win32_Release_No_Optimizations"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release_No_Optimizations"
# PROP Intermediate_Dir "Release_No_Optimizations"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /Zi /Od /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x405 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ws2_32.lib /nologo /subsystem:windows /pdb:"Release/GameServ/machine:I386" /machine:IX86 /nodefaultlib:"MSVCRT"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 ws2_32.lib /nologo /subsystem:windows /debug /machine:IX86 /nodefaultlib:"MSVCRT" /nodefaultlib:"LIBC"
# SUBTRACT LINK32 /profile /map

!ENDIF 

# Begin Target

# Name "GameServerApp - Win32 Release"
# Name "GameServerApp - Win32 Debug"
# Name "GameServerApp - Win32 Debug Static"
# Name "GameServerApp - Win32 Release No Optimizations"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\CivilizationDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\ColorControl.cpp
# End Source File
# Begin Source File

SOURCE=.\EditErrorOutput.cpp
# End Source File
# Begin Source File

SOURCE=.\GameServerApp.cpp
# End Source File
# Begin Source File

SOURCE=.\GameServerApp.rc
# End Source File
# Begin Source File

SOURCE=.\GameServerAppDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\GameServerAppView.cpp
# End Source File
# Begin Source File

SOURCE=.\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\OptionsDebugPage.cpp
# End Source File
# Begin Source File

SOURCE=.\OptionsGamePage.cpp
# End Source File
# Begin Source File

SOURCE=.\OptionsNetworkPage.cpp
# End Source File
# Begin Source File

SOURCE=.\OptionsPathsPage.cpp
# End Source File
# Begin Source File

SOURCE=.\ScriptSetMakerDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ScriptSetProgressDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\SendMessageDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\UserDialog.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\CivilizationDialog.h
# End Source File
# Begin Source File

SOURCE=.\ColorControl.h
# End Source File
# Begin Source File

SOURCE=.\EditErrorOutput.h
# End Source File
# Begin Source File

SOURCE=.\GameServerApp.h
# End Source File
# Begin Source File

SOURCE=.\GameServerAppDoc.h
# End Source File
# Begin Source File

SOURCE=.\GameServerAppView.h
# End Source File
# Begin Source File

SOURCE=.\MainFrm.h
# End Source File
# Begin Source File

SOURCE=.\OptionsDebugPage.h
# End Source File
# Begin Source File

SOURCE=.\OptionsGamePage.h
# End Source File
# Begin Source File

SOURCE=.\OptionsNetworkPage.h
# End Source File
# Begin Source File

SOURCE=.\OptionsPathsPage.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\ScriptSetMakerDlg.h
# End Source File
# Begin Source File

SOURCE=.\ScriptSetProgressDlg.h
# End Source File
# Begin Source File

SOURCE=.\SendMessageDlg.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\UserDialog.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\bitmap1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Civilization.bmp
# End Source File
# Begin Source File

SOURCE=.\res\GameServerApp.ico
# End Source File
# Begin Source File

SOURCE=.\res\GameServerApp.rc2
# End Source File
# Begin Source File

SOURCE=.\res\GameServerAppDoc.ico
# End Source File
# Begin Source File

SOURCE=.\res\gamestat.ico
# End Source File
# Begin Source File

SOURCE=.\res\ico00001.ico
# End Source File
# Begin Source File

SOURCE=.\res\ico00002.ico
# End Source File
# Begin Source File

SOURCE=.\res\ico00003.ico
# End Source File
# Begin Source File

SOURCE=.\res\ico00004.ico
# End Source File
# Begin Source File

SOURCE=.\res\icon1.ico
# End Source File
# Begin Source File

SOURCE=.\res\Toolbar.bmp
# End Source File
# Begin Source File

SOURCE=.\res\ToolbarColor.bmp
# End Source File
# Begin Source File

SOURCE=.\res\ToolbarDisabled.bmp
# End Source File
# Begin Source File

SOURCE=.\res\ToolbarGray.bmp
# End Source File
# Begin Source File

SOURCE=.\res\User.bmp
# End Source File
# End Group
# Begin Source File

SOURCE=.\GameServerApp.reg
# End Source File
# End Target
# End Project
