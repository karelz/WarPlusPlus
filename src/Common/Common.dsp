# Microsoft Developer Studio Project File - Name="Common" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=Common - Win32 Debug Static
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Common.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Common.mak" CFG="Common - Win32 Debug Static"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Common - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Common - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "Common - Win32 Debug Static" (based on "Win32 (x86) Static Library")
!MESSAGE "Common - Win32 Release No Optimizations" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Common", JWAAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Common - Win32 Release"

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
# ADD BASE RSC /l 0x405 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x405 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Common - Win32 Debug"

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
# ADD BASE RSC /l 0x405 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x405 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Common - Win32 Debug Static"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Common___Win32_Debug_Static"
# PROP BASE Intermediate_Dir "Common___Win32_Debug_Static"
# PROP BASE Target_Dir ""
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug_Static"
# PROP Intermediate_Dir "Debug_Static"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /GZ /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x405 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x405 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Common - Win32 Release No Optimizations"

# PROP BASE Use_MFC 1
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Common___Win32_Release_No_Optimizations"
# PROP BASE Intermediate_Dir "Common___Win32_Release_No_Optimizations"
# PROP BASE Target_Dir ""
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release_No_Optimizations"
# PROP Intermediate_Dir "Release_No_Optimizations"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /Zi /Od /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD BASE RSC /l 0x405 /d "NDEBUG"
# ADD RSC /l 0x405 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "Common - Win32 Release"
# Name "Common - Win32 Debug"
# Name "Common - Win32 Debug Static"
# Name "Common - Win32 Release No Optimizations"
# Begin Group "AfxDebugPlus"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\AfxDebugPlus\AfxDebugPlus.cpp

!IF  "$(CFG)" == "Common - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Common - Win32 Debug"

!ELSEIF  "$(CFG)" == "Common - Win32 Debug Static"

!ELSEIF  "$(CFG)" == "Common - Win32 Release No Optimizations"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\AfxDebugPlus\AfxDebugPlus.doc
# End Source File
# Begin Source File

SOURCE=.\AfxDebugPlus\AfxDebugPlus.h
# End Source File
# End Group
# Begin Group "MemoryPool"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\MemoryPool\Array2DOnDemand.h
# End Source File
# Begin Source File

SOURCE=.\MemoryPool\MemoryPool.cpp
# End Source File
# Begin Source File

SOURCE=.\MemoryPool\MemoryPool.h
# End Source File
# Begin Source File

SOURCE=.\MemoryPool\MultiThreadMemoryPool.h
# End Source File
# Begin Source File

SOURCE=.\MemoryPool\MultiThreadTypedMemoryPool.h
# End Source File
# Begin Source File

SOURCE=.\MemoryPool\PooledList.h
# End Source File
# Begin Source File

SOURCE=.\MemoryPool\PooledQueue.h
# End Source File
# Begin Source File

SOURCE=.\MemoryPool\SelfPooledList.h
# End Source File
# Begin Source File

SOURCE=.\MemoryPool\SelfPooledPriorityQueue.h
# End Source File
# Begin Source File

SOURCE=.\MemoryPool\SelfPooledQueue.h
# End Source File
# Begin Source File

SOURCE=.\MemoryPool\SelfPooledStack.h
# End Source File
# Begin Source File

SOURCE=.\MemoryPool\SmartPriorityQueue.h
# End Source File
# Begin Source File

SOURCE=.\MemoryPool\TypedMemoryPool.h
# End Source File
# End Group
# Begin Group "ConfigFile"

# PROP Default_Filter ""
# Begin Group "ConfigFile Header Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ConfigFile\ConfigFile\ConfigFile.h
# End Source File
# Begin Source File

SOURCE=.\ConfigFile\ConfigFile\ConfigFileException.h
# End Source File
# Begin Source File

SOURCE=.\ConfigFile\ConfigFile\ConfigFileSection.h
# End Source File
# Begin Source File

SOURCE=.\ConfigFile\ConfigFile\ConfigFileSection.inl
# End Source File
# Begin Source File

SOURCE=.\ConfigFile\ConfigFile\ConfigFileSectionInfo.h
# End Source File
# Begin Source File

SOURCE=.\ConfigFile\ConfigFile\ConfigFileSectionInfo.inl
# End Source File
# End Group
# Begin Group "ConfigFile Source Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ConfigFile\ConfigFile\ConfigFile.cpp
# End Source File
# Begin Source File

SOURCE=.\ConfigFile\ConfigFile\ConfigFileException.cpp
# End Source File
# Begin Source File

SOURCE=.\ConfigFile\ConfigFile\ConfigFileSection.cpp
# End Source File
# Begin Source File

SOURCE=.\ConfigFile\ConfigFile\ConfigFileSectionInfo.cpp
# End Source File
# End Group
# End Group
# Begin Group "ConfigFileLex"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ConfigFile\ConfigFileLex\ConfigFileLex.cpp
# End Source File
# Begin Source File

SOURCE=.\ConfigFile\ConfigFileLex\ConfigFileLex.h
# End Source File
# Begin Source File

SOURCE=.\ConfigFile\ConfigFileLex\ConfigFileLexGen.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\ConfigFile\ConfigFileLex\FlexLexer.h
# End Source File
# End Group
# Begin Group "PersistentStorage"

# PROP Default_Filter ""
# Begin Group "PersistentStorage Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\PersistentStorage\PersistentLoadException.h
# End Source File
# Begin Source File

SOURCE=.\PersistentStorage\PersistentObject.h
# End Source File
# Begin Source File

SOURCE=.\PersistentStorage\PersistentStorage.h
# End Source File
# End Group
# Begin Group "PersistentStorage Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\PersistentStorage\PersistentObject.cpp
# End Source File
# Begin Source File

SOURCE=.\PersistentStorage\PersistentStorage.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=".\PersistentStorage\Persistent objects.doc"
# End Source File
# End Group
# Begin Group "ReadWriteLock"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ReadWriteLock\ReadWriteLock.cpp
# End Source File
# Begin Source File

SOURCE=.\ReadWriteLock\ReadWriteLock.h
# End Source File
# End Group
# Begin Group "Map"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Map\MAbstractAttackSkillType.h
# End Source File
# Begin Source File

SOURCE=.\Map\Map.h
# End Source File
# Begin Source File

SOURCE=.\Map\MapDataTypes.h
# End Source File
# Begin Source File

SOURCE=.\Map\MBulletAttackSkillType.h
# End Source File
# Begin Source File

SOURCE=.\Map\MBulletDefenseSkillType.h
# End Source File
# Begin Source File

SOURCE=.\Map\MFindPathGraphs.h
# End Source File
# Begin Source File

SOURCE=.\Map\mmakeskilltype.h
# End Source File
# Begin Source File

SOURCE=.\Map\MMineSkillType.h
# End Source File
# Begin Source File

SOURCE=.\Map\MMoveSkillType.h
# End Source File
# Begin Source File

SOURCE=.\Map\MResourceSkillType.h
# End Source File
# Begin Source File

SOURCE=.\Map\MStorageSkillType.h
# End Source File
# End Group
# Begin Group "ServerClient"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ServerClient\CommunicationInfoStructures.h
# End Source File
# Begin Source File

SOURCE=.\ServerClient\ControlConnection.h
# End Source File
# Begin Source File

SOURCE=.\ServerClient\LogOutput.h
# End Source File
# Begin Source File

SOURCE=.\ServerClient\MapDownloadStructures.h
# End Source File
# Begin Source File

SOURCE=.\ServerClient\MapViewConnection.h
# End Source File
# Begin Source File

SOURCE=.\ServerClient\MiniMapStructures.h
# End Source File
# Begin Source File

SOURCE=.\ServerClient\ScriptEditor.h
# End Source File
# Begin Source File

SOURCE=.\ServerClient\UnitHierarchyConnection.h
# End Source File
# Begin Source File

SOURCE=.\ServerClient\UnitInfoStructures.h
# End Source File
# Begin Source File

SOURCE=.\ServerClient\VirtualConnectionIDs.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\Time.cpp
# End Source File
# End Target
# End Project
