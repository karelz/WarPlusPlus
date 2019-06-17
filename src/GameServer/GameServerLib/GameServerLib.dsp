# Microsoft Developer Studio Project File - Name="GameServerLib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=GameServerLib - Win32 Debug Static
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "GameServerLib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "GameServerLib.mak" CFG="GameServerLib - Win32 Debug Static"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "GameServerLib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "GameServerLib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "GameServerLib - Win32 Debug Static" (based on "Win32 (x86) Static Library")
!MESSAGE "GameServerLib - Win32 Release No Optimizations" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/GameServer/GameServerLib", RDAAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "GameServerLib - Win32 Release"

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

!ELSEIF  "$(CFG)" == "GameServerLib - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "GameServerLib - Win32 Debug Static"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "GameServerLib___Win32_Debug_Static"
# PROP BASE Intermediate_Dir "GameServerLib___Win32_Debug_Static"
# PROP BASE Target_Dir ""
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug_Static"
# PROP Intermediate_Dir "Debug_Static"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /GZ /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "GameServerLib - Win32 Release No Optimizations"

# PROP BASE Use_MFC 1
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "GameServerLib___Win32_Release_No_Optimizations"
# PROP BASE Intermediate_Dir "GameServerLib___Win32_Release_No_Optimizations"
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

# Name "GameServerLib - Win32 Release"
# Name "GameServerLib - Win32 Debug"
# Name "GameServerLib - Win32 Debug Static"
# Name "GameServerLib - Win32 Release No Optimizations"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "Containers Source Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\SCheckPointPositionList.cpp
# End Source File
# Begin Source File

SOURCE=.\SDeferredProcedureCallQueue.cpp
# End Source File
# Begin Source File

SOURCE=.\SFindPathRequestQueue.cpp
# End Source File
# Begin Source File

SOURCE=.\SRegisteredClientList.cpp
# End Source File
# Begin Source File

SOURCE=.\SUnitList.cpp
# End Source File
# Begin Source File

SOURCE=.\SWatchedMapSquareArray.cpp
# End Source File
# Begin Source File

SOURCE=.\SWatchedUnitList.cpp
# End Source File
# Begin Source File

SOURCE=.\SWatchingMapSquareRequestQueue.cpp
# End Source File
# Begin Source File

SOURCE=.\SWatchingUnitRequestQueue.cpp
# End Source File
# End Group
# Begin Group "Abstract Data Classes Source F"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\AbstractDataClasses\PrivilegedThreadReadWriteLock.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\SCheckPoint.cpp
# End Source File
# Begin Source File

SOURCE=.\SCivilization.cpp
# End Source File
# Begin Source File

SOURCE=.\SDeferredProcedureCall.cpp
# End Source File
# Begin Source File

SOURCE=.\SDeferredProcedureCalls.cpp
# End Source File
# Begin Source File

SOURCE=.\SFileManager.cpp
# End Source File
# Begin Source File

SOURCE=.\SFindPathGraph.cpp
# End Source File
# Begin Source File

SOURCE=.\SMap.cpp
# End Source File
# Begin Source File

SOURCE=.\SMapSquare.cpp
# End Source File
# Begin Source File

SOURCE=.\SNotifications.cpp
# End Source File
# Begin Source File

SOURCE=.\SOneInstance.cpp
# End Source File
# Begin Source File

SOURCE=.\SPath.cpp
# End Source File
# Begin Source File

SOURCE=.\SSkillType.cpp
# End Source File
# Begin Source File

SOURCE=.\SSkillTypes.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\SUnit.cpp
# End Source File
# Begin Source File

SOURCE=.\SUnitType.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "Abstract Data Classes Header Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\AbstractDataClasses\256BitArray.h
# End Source File
# Begin Source File

SOURCE=.\AbstractDataClasses\BitArray.h
# End Source File
# Begin Source File

SOURCE=.\AbstractDataClasses\PrivilegedThreadReadWriteLock.h
# End Source File
# Begin Source File

SOURCE=.\AbstractDataClasses\RunTimeIDCreation.h
# End Source File
# Begin Source File

SOURCE=.\AbstractDataClasses\RunTimeIDNameCreation.h
# End Source File
# Begin Source File

SOURCE=.\AbstractDataClasses\StaticBitArray.h
# End Source File
# End Group
# Begin Group "General Classes Header Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\GeneralClasses\GeneralClasses.h
# End Source File
# Begin Source File

SOURCE=.\GeneralClasses\PointDW.h
# End Source File
# Begin Source File

SOURCE=.\GeneralClasses\PointDW.inl
# End Source File
# Begin Source File

SOURCE=.\GeneralClasses\SizeDW.h
# End Source File
# Begin Source File

SOURCE=.\GeneralClasses\SizeDW.inl
# End Source File
# End Group
# Begin Group "Containers Header Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\SCheckPointPositionList.h
# End Source File
# Begin Source File

SOURCE=.\SCivilizationBitArray.h
# End Source File
# Begin Source File

SOURCE=.\SDeferredProcedureCallQueue.h
# End Source File
# Begin Source File

SOURCE=.\SFindPathRequestQueue.h
# End Source File
# Begin Source File

SOURCE=.\SRegisteredClientList.h
# End Source File
# Begin Source File

SOURCE=.\SUnitList.h
# End Source File
# Begin Source File

SOURCE=.\SWatchedMapSquareArray.h
# End Source File
# Begin Source File

SOURCE=.\SWatchedUnitList.h
# End Source File
# Begin Source File

SOURCE=.\SWatchingMapSquareRequestQueue.h
# End Source File
# Begin Source File

SOURCE=.\SWatchingUnitRequestQueue.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\GameServerCommon.h
# End Source File
# Begin Source File

SOURCE=.\GameServerCompileSettings.h
# End Source File
# Begin Source File

SOURCE=.\SCheckPoint.h
# End Source File
# Begin Source File

SOURCE=.\SCivilization.h
# End Source File
# Begin Source File

SOURCE=.\SDeferredProcedureCall.h
# End Source File
# Begin Source File

SOURCE=.\SDeferredProcedureCall.inl
# End Source File
# Begin Source File

SOURCE=.\SFileManager.h
# End Source File
# Begin Source File

SOURCE=.\SFindPathGraph.h
# End Source File
# Begin Source File

SOURCE=.\SFindPathGraph.inl
# End Source File
# Begin Source File

SOURCE=.\SMap.h
# End Source File
# Begin Source File

SOURCE=.\SMapSquare.h
# End Source File
# Begin Source File

SOURCE=.\SNotifications.h
# End Source File
# Begin Source File

SOURCE=.\SOneInstance.h
# End Source File
# Begin Source File

SOURCE=.\SPath.h
# End Source File
# Begin Source File

SOURCE=.\SPosition.h
# End Source File
# Begin Source File

SOURCE=.\SResources.h
# End Source File
# Begin Source File

SOURCE=.\SSkillType.h
# End Source File
# Begin Source File

SOURCE=.\SSkillTypeMethods.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\SUnit.h
# End Source File
# Begin Source File

SOURCE=.\SUnit.inl
# End Source File
# Begin Source File

SOURCE=.\SUnitType.h
# End Source File
# Begin Source File

SOURCE=.\SUnitType.inl
# End Source File
# End Group
# Begin Source File

SOURCE=.\GeneralClasses\GeneralClasses.txt
# End Source File
# End Target
# End Project
