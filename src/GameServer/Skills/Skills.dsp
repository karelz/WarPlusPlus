# Microsoft Developer Studio Project File - Name="Skills" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=Skills - Win32 Debug Static
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Skills.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Skills.mak" CFG="Skills - Win32 Debug Static"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Skills - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Skills - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "Skills - Win32 Debug Static" (based on "Win32 (x86) Static Library")
!MESSAGE "Skills - Win32 Release No Optimizations" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/GameServer/Skills", NPAAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Skills - Win32 Release"

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

!ELSEIF  "$(CFG)" == "Skills - Win32 Debug"

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

!ELSEIF  "$(CFG)" == "Skills - Win32 Debug Static"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Skills___Win32_Debug_Static"
# PROP BASE Intermediate_Dir "Skills___Win32_Debug_Static"
# PROP BASE Target_Dir ""
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug_Static"
# PROP Intermediate_Dir "Debug_Static"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Skills - Win32 Release No Optimizations"

# PROP BASE Use_MFC 1
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Skills___Win32_Release_No_Optimizations"
# PROP BASE Intermediate_Dir "Skills___Win32_Release_No_Optimizations"
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

# Name "Skills - Win32 Release"
# Name "Skills - Win32 Debug"
# Name "Skills - Win32 Debug Static"
# Name "Skills - Win32 Release No Optimizations"
# Begin Group "SysCalls"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\SysCalls\IBagSkillSysCall.cpp
# End Source File
# Begin Source File

SOURCE=.\SysCalls\IBagSkillSysCall.h
# End Source File
# Begin Source File

SOURCE=.\SysCalls\IDPCSysCall.cpp
# End Source File
# Begin Source File

SOURCE=.\SysCalls\IDPCSysCall.h
# End Source File
# Begin Source File

SOURCE=.\SysCalls\ISkillSysCall.cpp
# End Source File
# Begin Source File

SOURCE=.\SysCalls\ISkillSysCall.h
# End Source File
# Begin Source File

SOURCE=.\SysCalls\SSysCallDPC.cpp
# End Source File
# Begin Source File

SOURCE=.\SysCalls\SSysCallDPC.h
# End Source File
# End Group
# Begin Group "ExampleSkill"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ExampleSkill\SExampleSkill.h
# End Source File
# Begin Source File

SOURCE=.\ExampleSkill\SExampleSkillType.cpp
# End Source File
# Begin Source File

SOURCE=.\ExampleSkill\SExampleSkillType.h
# End Source File
# End Group
# Begin Group "MoveSkill"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\MoveSkill\ISysCallMove.cpp
# End Source File
# Begin Source File

SOURCE=.\MoveSkill\ISysCallMove.h
# End Source File
# Begin Source File

SOURCE=.\MoveSkill\SMoveSkill.h
# End Source File
# Begin Source File

SOURCE=.\MoveSkill\SMoveSkillType.cpp
# End Source File
# Begin Source File

SOURCE=.\MoveSkill\SMoveSkillType.h
# End Source File
# Begin Source File

SOURCE=.\MoveSkill\SMoveSkillType.inl
# End Source File
# End Group
# Begin Group "SystemObjects"

# PROP Default_Filter ""
# Begin Group "Map"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\SystemObjects\Map\SMapGetUnitsInAreaDPC.cpp
# End Source File
# Begin Source File

SOURCE=.\SystemObjects\Map\SMapGetUnitsInAreaDPC.h
# End Source File
# End Group
# End Group
# Begin Group "AttackDefenseSkills"

# PROP Default_Filter ""
# Begin Group "AbstractAttackSkill"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\attackdefenseskills\abstractattackskill\ISysCallAttack.cpp
# End Source File
# Begin Source File

SOURCE=.\attackdefenseskills\abstractattackskill\ISysCallAttack.h
# End Source File
# Begin Source File

SOURCE=.\AttackDefenseSkills\AbstractAttackSkill\SAbstractAttackSkill.h
# End Source File
# Begin Source File

SOURCE=.\AttackDefenseSkills\AbstractAttackSkill\SAbstractAttackSkillType.cpp
# End Source File
# Begin Source File

SOURCE=.\AttackDefenseSkills\AbstractAttackSkill\SAbstractAttackSkillType.h
# End Source File
# Begin Source File

SOURCE=.\attackdefenseskills\abstractattackskill\SAttackableUnitsDPC.cpp
# End Source File
# Begin Source File

SOURCE=.\attackdefenseskills\abstractattackskill\SAttackableUnitsDPC.h
# End Source File
# Begin Source File

SOURCE=.\attackdefenseskills\abstractattackskill\SCanAttackUnitDPC.cpp
# End Source File
# Begin Source File

SOURCE=.\attackdefenseskills\abstractattackskill\SCanAttackUnitDPC.h
# End Source File
# End Group
# Begin Group "AbstractDefenseSkill"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\AttackDefenseSkills\AbstractDefenseSkill\SAbstractDefenseSkillType.cpp
# End Source File
# Begin Source File

SOURCE=.\AttackDefenseSkills\AbstractDefenseSkill\SAbstractDefenseSkillType.h
# End Source File
# End Group
# Begin Group "BulletDefenseSkill"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\AttackDefenseSkills\BulletDefenseSkill\SBulletDefenseSkill.h
# End Source File
# Begin Source File

SOURCE=.\AttackDefenseSkills\BulletDefenseSkill\SBulletDefenseSkillType.cpp
# End Source File
# Begin Source File

SOURCE=.\AttackDefenseSkills\BulletDefenseSkill\SBulletDefenseSkillType.h
# End Source File
# End Group
# Begin Group "BulletAttackSkill"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\AttackDefenseSkills\BulletAttackSkill\SBulletAttackSkill.h
# End Source File
# Begin Source File

SOURCE=.\AttackDefenseSkills\BulletAttackSkill\SBulletAttackSkillType.cpp
# End Source File
# Begin Source File

SOURCE=.\AttackDefenseSkills\BulletAttackSkill\SBulletAttackSkillType.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\AttackDefenseSkills\AttackDefenseInteractions.h
# End Source File
# End Group
# Begin Group "MakeSkill"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\MakeSkill\ISysCallMake.cpp
# End Source File
# Begin Source File

SOURCE=.\MakeSkill\ISysCallMake.h
# End Source File
# Begin Source File

SOURCE=.\MakeSkill\SMakeSkill.h
# End Source File
# Begin Source File

SOURCE=.\MakeSkill\SMakeSkillType.cpp
# End Source File
# Begin Source File

SOURCE=.\MakeSkill\SMakeSkillType.h
# End Source File
# End Group
# Begin Group "MineSkill"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\MineSkill\ISysCallMine.cpp
# End Source File
# Begin Source File

SOURCE=.\MineSkill\ISysCallMine.h
# End Source File
# Begin Source File

SOURCE=.\MineSkill\SMineSetModeDPC.cpp
# End Source File
# Begin Source File

SOURCE=.\MineSkill\SMineSetModeDPC.h
# End Source File
# Begin Source File

SOURCE=.\MineSkill\SMineSkill.h
# End Source File
# Begin Source File

SOURCE=.\MineSkill\SMineSkillType.cpp
# End Source File
# Begin Source File

SOURCE=.\MineSkill\SMineSkillType.h
# End Source File
# End Group
# Begin Group "StorageSkill"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\StorageSkill\ISysCallStorage.cpp
# End Source File
# Begin Source File

SOURCE=.\StorageSkill\ISysCallStorage.h
# End Source File
# Begin Source File

SOURCE=.\StorageSkill\SStorageSkill.h
# End Source File
# Begin Source File

SOURCE=.\StorageSkill\SStorageSkillType.cpp
# End Source File
# Begin Source File

SOURCE=.\StorageSkill\SStorageSkillType.h
# End Source File
# End Group
# Begin Group "ResourceSkill"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ResourceSkill\SResourceSkill.h
# End Source File
# Begin Source File

SOURCE=.\ResourceSkill\SResourceSkillType.cpp
# End Source File
# Begin Source File

SOURCE=.\ResourceSkill\SResourceSkillType.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\SDeferredProcedureCallID.txt
# End Source File
# Begin Source File

SOURCE=.\SSkillTypeID.txt
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"StdAfx.h"
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# End Target
# End Project
