# Microsoft Developer Studio Project File - Name="Interpret" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=Interpret - Win32 Debug Static
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Interpret.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Interpret.mak" CFG="Interpret - Win32 Debug Static"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Interpret - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Interpret - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "Interpret - Win32 Debug Static" (based on "Win32 (x86) Static Library")
!MESSAGE "Interpret - Win32 Release No Optimizations" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/TripleS/Interpret", GQAAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Interpret - Win32 Release"

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

!ELSEIF  "$(CFG)" == "Interpret - Win32 Debug"

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

!ELSEIF  "$(CFG)" == "Interpret - Win32 Debug Static"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Interpret___Win32_Debug_Static"
# PROP BASE Intermediate_Dir "Interpret___Win32_Debug_Static"
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

!ELSEIF  "$(CFG)" == "Interpret - Win32 Release No Optimizations"

# PROP BASE Use_MFC 1
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Interpret___Win32_Release_No_Optimizations"
# PROP BASE Intermediate_Dir "Interpret___Win32_Release_No_Optimizations"
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

# Name "Interpret - Win32 Release"
# Name "Interpret - Win32 Debug"
# Name "Interpret - Win32 Debug Static"
# Name "Interpret - Win32 Release No Optimizations"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "SystemObjects Sources"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Src\SysObjs\ISOClient.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\SysObjs\ISOMap.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\SysObjs\ISOPath.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\SysObjs\ISOPosition.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\SysObjs\ISOResources.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\SysObjs\ISOString.cpp
# End Source File
# End Group
# Begin Group "Syscalls Sources"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Src\Syscalls\ISOClientAskForPositionSyscall.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\Syscalls\ISOClientAskForUnitSyscall.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\Src\IBag.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\ICode.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\ICodeStackItem.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\ICodeStringTable.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\IConstructor.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\IDataStack.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\IEvent.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\IGlobalFunction.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\IMember.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\IMethod.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\Interpret.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\IObjUnitStruct.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\IProcess.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\IRunEventData.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\ISendtoInformation.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\ISkillInterface.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\IStructure.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\IStructureType.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\ISyscall.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\Syscalls\ISyscallSome.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\SysObjs\ISystemGlobalFunctions.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\SysObjs\ISystemObject.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\IType.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\IUnitType.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\IWaitingInterface.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\PersistentSaveLoadWaiting.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\stdafx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "SystemObjects Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Src\SysObjs\ISOArray.h
# End Source File
# Begin Source File

SOURCE=.\Src\SysObjs\ISOArrayParent.h
# End Source File
# Begin Source File

SOURCE=.\Src\SysObjs\ISOClient.h
# End Source File
# Begin Source File

SOURCE=.\Src\SysObjs\ISOMap.h
# End Source File
# Begin Source File

SOURCE=.\Src\SysObjs\ISOPath.h
# End Source File
# Begin Source File

SOURCE=.\Src\SysObjs\ISOPosition.h
# End Source File
# Begin Source File

SOURCE=.\Src\SysObjs\ISOResources.h
# End Source File
# Begin Source File

SOURCE=.\Src\SysObjs\ISOSet.h
# End Source File
# Begin Source File

SOURCE=.\Src\SysObjs\ISOSetParent.h
# End Source File
# Begin Source File

SOURCE=.\Src\SysObjs\ISOString.h
# End Source File
# End Group
# Begin Group "Syscalls Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Src\Syscalls\ISOClientAskForPositionSyscall.h
# End Source File
# Begin Source File

SOURCE=.\Src\Syscalls\ISOClientAskForUnitSyscall.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\Src\IBag.h
# End Source File
# Begin Source File

SOURCE=.\Src\ICode.h
# End Source File
# Begin Source File

SOURCE=.\Src\ICodeStackItem.h
# End Source File
# Begin Source File

SOURCE=.\Src\ICodeStringTable.h
# End Source File
# Begin Source File

SOURCE=.\Src\IConstructor.h
# End Source File
# Begin Source File

SOURCE=.\Src\IConstructorFinishedWaiting.h
# End Source File
# Begin Source File

SOURCE=.\Src\IDataStack.h
# End Source File
# Begin Source File

SOURCE=.\Src\SysObjs\IDuplicSystemObject.h
# End Source File
# Begin Source File

SOURCE=.\Src\IEvent.h
# End Source File
# Begin Source File

SOURCE=.\Src\IGlobalFunction.h
# End Source File
# Begin Source File

SOURCE=.\Src\IMember.h
# End Source File
# Begin Source File

SOURCE=.\Src\IMethod.h
# End Source File
# Begin Source File

SOURCE=.\Interpret.h
# End Source File
# Begin Source File

SOURCE=.\Src\InterpretEnums.h
# End Source File
# Begin Source File

SOURCE=.\Src\InterpretEvents.h
# End Source File
# Begin Source File

SOURCE=.\Src\IObjUnitStruct.h
# End Source File
# Begin Source File

SOURCE=.\Src\IProcess.h
# End Source File
# Begin Source File

SOURCE=.\Src\IRunEventData.h
# End Source File
# Begin Source File

SOURCE=.\Src\IRunGlobalFunctionData.h
# End Source File
# Begin Source File

SOURCE=.\Src\ISendtoInformation.h
# End Source File
# Begin Source File

SOURCE=.\Src\ISendtoWaiting1st.h
# End Source File
# Begin Source File

SOURCE=.\Src\ISendtoWaitingAll.h
# End Source File
# Begin Source File

SOURCE=.\Src\ISkillInterface.h
# End Source File
# Begin Source File

SOURCE=.\Src\IStructure.h
# End Source File
# Begin Source File

SOURCE=.\Src\IStructureType.h
# End Source File
# Begin Source File

SOURCE=.\Src\ISyscall.h
# End Source File
# Begin Source File

SOURCE=.\Src\Syscalls\ISyscallSome.h
# End Source File
# Begin Source File

SOURCE=.\Src\SysObjs\ISystemGlobalFunctions.h
# End Source File
# Begin Source File

SOURCE=.\Src\SysObjs\ISystemObject.h
# End Source File
# Begin Source File

SOURCE=.\Src\IType.h
# End Source File
# Begin Source File

SOURCE=.\Src\IUnitType.h
# End Source File
# Begin Source File

SOURCE=.\Src\IWaitingInterface.h
# End Source File
# Begin Source File

SOURCE=.\Src\PersistentSaveLoadWaiting.h
# End Source File
# Begin Source File

SOURCE=.\Src\stdafx.h
# End Source File
# Begin Source File

SOURCE=.\Src\SysObjs\SystemObjects.h
# End Source File
# End Group
# End Target
# End Project
