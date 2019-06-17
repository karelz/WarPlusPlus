# Microsoft Developer Studio Project File - Name="Archive" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=Archive - Win32 Debug Static
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Archive.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Archive.mak" CFG="Archive - Win32 Debug Static"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Archive - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Archive - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "Archive - Win32 Debug Static" (based on "Win32 (x86) Static Library")
!MESSAGE "Archive - Win32 Release No Optimizations" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Archive/Archive", FHCAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Archive - Win32 Release"

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
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD BASE RSC /l 0x405 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x405 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Archive - Win32 Debug"

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
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x405 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x405 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Archive - Win32 Debug Static"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Archive___Win32_Debug_Static"
# PROP BASE Intermediate_Dir "Archive___Win32_Debug_Static"
# PROP BASE Target_Dir ""
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug_Static"
# PROP Intermediate_Dir "Debug_Static"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_AFXDLL" /FR /Yu"stdafx.h" /FD /GZ /c
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

!ELSEIF  "$(CFG)" == "Archive - Win32 Release No Optimizations"

# PROP BASE Use_MFC 1
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Archive___Win32_Release_No_Optimizations"
# PROP BASE Intermediate_Dir "Archive___Win32_Release_No_Optimizations"
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

# Name "Archive - Win32 Release"
# Name "Archive - Win32 Debug"
# Name "Archive - Win32 Debug Static"
# Name "Archive - Win32 Release No Optimizations"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\ArchiveFile.cpp
# End Source File
# Begin Source File

SOURCE=.\ArchiveFileCompressed.cpp
# End Source File
# Begin Source File

SOURCE=.\ArchiveFileInFile.cpp
# End Source File
# Begin Source File

SOURCE=.\ArchiveFileInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\ArchiveFilePhysical.cpp
# End Source File
# Begin Source File

SOURCE=.\ArchiveUtils.cpp
# End Source File
# Begin Source File

SOURCE=.\CompressDataSinkArchiveFile.cpp
# End Source File
# Begin Source File

SOURCE=.\CompressDataSourceArchiveFile.cpp
# End Source File
# Begin Source File

SOURCE=.\DataArchive.cpp
# End Source File
# Begin Source File

SOURCE=.\DataArchiveDirContents.cpp
# End Source File
# Begin Source File

SOURCE=.\DataArchiveDirectory.cpp
# End Source File
# Begin Source File

SOURCE=.\DataArchiveException.cpp
# End Source File
# Begin Source File

SOURCE=.\DataArchiveFile.cpp
# End Source File
# Begin Source File

SOURCE=.\DataArchiveFileCommon.cpp
# End Source File
# Begin Source File

SOURCE=.\DataArchiveFileDataList.cpp
# End Source File
# Begin Source File

SOURCE=.\DataArchiveFileDirectory.cpp
# End Source File
# Begin Source File

SOURCE=.\DataArchiveFileDirectoryCache.cpp
# End Source File
# Begin Source File

SOURCE=.\DataArchiveFileDirectoryInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\DataArchiveFileDirectoryItem.cpp
# End Source File
# Begin Source File

SOURCE=.\DataArchiveFileMainHeader.cpp
# End Source File
# Begin Source File

SOURCE=.\DataArchiveFileManager.cpp
# End Source File
# Begin Source File

SOURCE=.\DataArchiveFilePrefixed.cpp
# End Source File
# Begin Source File

SOURCE=.\DataArchiveInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\MappedFile.cpp
# End Source File
# Begin Source File

SOURCE=.\stdafx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\archive.h
# End Source File
# Begin Source File

SOURCE=.\ArchiveFile.h
# End Source File
# Begin Source File

SOURCE=.\ArchiveFileCompressed.h
# End Source File
# Begin Source File

SOURCE=.\ArchiveFileInFile.h
# End Source File
# Begin Source File

SOURCE=.\ArchiveFileInfo.h
# End Source File
# Begin Source File

SOURCE=.\ArchiveFilePhysical.h
# End Source File
# Begin Source File

SOURCE=.\ArchiveUtils.h
# End Source File
# Begin Source File

SOURCE=.\CompressDataSinkArchiveFile.h
# End Source File
# Begin Source File

SOURCE=.\CompressDataSourceArchiveFile.h
# End Source File
# Begin Source File

SOURCE=.\DataArchive.h
# End Source File
# Begin Source File

SOURCE=.\DataArchiveDirContents.h
# End Source File
# Begin Source File

SOURCE=.\DataArchiveDirectory.h
# End Source File
# Begin Source File

SOURCE=.\DataArchiveException.h
# End Source File
# Begin Source File

SOURCE=.\DataArchiveFile.h
# End Source File
# Begin Source File

SOURCE=.\DataArchiveFileCommon.h
# End Source File
# Begin Source File

SOURCE=.\DataArchiveFileDataList.h
# End Source File
# Begin Source File

SOURCE=.\DataArchiveFileDirectory.h
# End Source File
# Begin Source File

SOURCE=.\DataArchiveFileDirectoryCache.h
# End Source File
# Begin Source File

SOURCE=.\DataArchiveFileDirectoryInfo.h
# End Source File
# Begin Source File

SOURCE=.\DataArchiveFileDirectoryItem.h
# End Source File
# Begin Source File

SOURCE=.\DataArchiveFileMainHeader.h
# End Source File
# Begin Source File

SOURCE=.\DataArchiveFileManager.h
# End Source File
# Begin Source File

SOURCE=.\DataArchiveFilePrefixed.h
# End Source File
# Begin Source File

SOURCE=.\DataArchiveInfo.h
# End Source File
# Begin Source File

SOURCE=.\Definitions.h
# End Source File
# Begin Source File

SOURCE=.\EAppendModes.h
# End Source File
# Begin Source File

SOURCE=.\MappedFile.h
# End Source File
# Begin Source File

SOURCE=.\stdafx.h
# End Source File
# End Group
# End Target
# End Project
