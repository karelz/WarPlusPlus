# Microsoft Developer Studio Project File - Name="DirectX" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=DirectX - Win32 Debug Static
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "DirectX.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "DirectX.mak" CFG="DirectX - Win32 Debug Static"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "DirectX - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "DirectX - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "DirectX - Win32 Debug Static" (based on "Win32 (x86) Static Library")
!MESSAGE "DirectX - Win32 Release No Optimizations" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Common/DirectX", THBAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "DirectX - Win32 Release"

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

!ELSEIF  "$(CFG)" == "DirectX - Win32 Debug"

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
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x405 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x405 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "DirectX - Win32 Debug Static"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "DirectX___Win32_Debug_Static"
# PROP BASE Intermediate_Dir "DirectX___Win32_Debug_Static"
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

!ELSEIF  "$(CFG)" == "DirectX - Win32 Release No Optimizations"

# PROP BASE Use_MFC 1
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "DirectX___Win32_Release_No_Optimizations"
# PROP BASE Intermediate_Dir "DirectX___Win32_Release_No_Optimizations"
# PROP BASE Target_Dir ""
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release_No_Optimizations"
# PROP Intermediate_Dir "Release_No_Optimizations"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /Od /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /c
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

# Name "DirectX - Win32 Release"
# Name "DirectX - Win32 Debug"
# Name "DirectX - Win32 Debug Static"
# Name "DirectX - Win32 Release No Optimizations"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\DirectXErrors.rc

!IF  "$(CFG)" == "DirectX - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "DirectX - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "DirectX - Win32 Debug Static"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "DirectX - Win32 Release No Optimizations"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\DirectDraw.h
# End Source File
# Begin Source File

SOURCE=.\DirectInput.h
# End Source File
# Begin Source File

SOURCE=.\DirectSound.h
# End Source File
# Begin Source File

SOURCE=.\DirectX.h
# End Source File
# Begin Source File

SOURCE=.\DirectXErrors.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# End Group
# Begin Group "DirectDraw"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\DirectDraw\DDPrimarySurface.cpp
# End Source File
# Begin Source File

SOURCE=.\DirectDraw\DDPrimarySurface.h
# End Source File
# Begin Source File

SOURCE=.\DirectDraw\DDrawClipper.cpp
# End Source File
# Begin Source File

SOURCE=.\DirectDraw\DDrawClipper.h
# End Source File
# Begin Source File

SOURCE=.\DirectDraw\DDrawSurface.cpp

!IF  "$(CFG)" == "DirectX - Win32 Release"

!ELSEIF  "$(CFG)" == "DirectX - Win32 Debug"

!ELSEIF  "$(CFG)" == "DirectX - Win32 Debug Static"

!ELSEIF  "$(CFG)" == "DirectX - Win32 Release No Optimizations"

# ADD BASE CPP /Od
# ADD CPP /Od

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\DirectDraw\DDrawSurface.h
# End Source File
# Begin Source File

SOURCE=.\DirectDraw\DirectDraw.cpp
# End Source File
# Begin Source File

SOURCE=.\DirectDraw\DirectDraw.h
# End Source File
# Begin Source File

SOURCE=.\DirectDraw\DirectDrawException.cpp
# End Source File
# Begin Source File

SOURCE=.\DirectDraw\DirectDrawException.h
# End Source File
# Begin Source File

SOURCE=.\DirectDraw\ImageReader.cpp
# End Source File
# Begin Source File

SOURCE=.\DirectDraw\ImageReader.h
# End Source File
# Begin Source File

SOURCE=.\DirectDraw\ImageSurface.cpp
# End Source File
# Begin Source File

SOURCE=.\DirectDraw\ImageSurface.h
# End Source File
# Begin Source File

SOURCE=.\DirectDraw\ScratchSurface.cpp
# End Source File
# Begin Source File

SOURCE=.\DirectDraw\ScratchSurface.h
# End Source File
# Begin Source File

SOURCE=.\DirectDraw\TGAReader.cpp
# End Source File
# Begin Source File

SOURCE=.\DirectDraw\TGAReader.h
# End Source File
# Begin Source File

SOURCE=.\DirectDraw\TIFFReader.cpp
# End Source File
# Begin Source File

SOURCE=.\DirectDraw\TIFFReader.h
# End Source File
# End Group
# Begin Group "DirectSound"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\DirectSound\DirectSound.cpp
# End Source File
# Begin Source File

SOURCE=.\DirectSound\DirectSound.h
# End Source File
# Begin Source File

SOURCE=.\DirectSound\DirectSoundException.cpp
# End Source File
# Begin Source File

SOURCE=.\DirectSound\DirectSoundException.h
# End Source File
# Begin Source File

SOURCE=.\DirectSound\DSoundBuffer.cpp
# End Source File
# Begin Source File

SOURCE=.\DirectSound\DSoundBuffer.h
# End Source File
# Begin Source File

SOURCE=.\DirectSound\SoundBuffer.cpp
# End Source File
# Begin Source File

SOURCE=.\DirectSound\SoundBuffer.h
# End Source File
# Begin Source File

SOURCE=.\DirectSound\SoundReader.cpp
# End Source File
# Begin Source File

SOURCE=.\DirectSound\SoundReader.h
# End Source File
# Begin Source File

SOURCE=.\DirectSound\WAVReader.cpp
# End Source File
# Begin Source File

SOURCE=.\DirectSound\WAVReader.h
# End Source File
# End Group
# Begin Group "DirectInput"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\DirectInput\DIMouseDevice.cpp
# End Source File
# Begin Source File

SOURCE=.\DirectInput\DIMouseDevice.h
# End Source File
# Begin Source File

SOURCE=.\DirectInput\DirectInput.cpp
# End Source File
# Begin Source File

SOURCE=.\DirectInput\DirectInput.h
# End Source File
# Begin Source File

SOURCE=.\DirectInput\DirectInputDevice.cpp
# End Source File
# Begin Source File

SOURCE=.\DirectInput\DirectInputDevice.h
# End Source File
# Begin Source File

SOURCE=.\DirectInput\DirectInputException.cpp
# End Source File
# Begin Source File

SOURCE=.\DirectInput\DirectInputException.h
# End Source File
# End Group
# End Target
# End Project
