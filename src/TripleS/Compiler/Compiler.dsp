# Microsoft Developer Studio Project File - Name="Compiler" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=Compiler - Win32 Debug Static
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Compiler.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Compiler.mak" CFG="Compiler - Win32 Debug Static"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Compiler - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Compiler - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "Compiler - Win32 Debug Static" (based on "Win32 (x86) Static Library")
!MESSAGE "Compiler - Win32 Release No Optimizations" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/TripleS/Compiler", DTAAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Compiler - Win32 Release"

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
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\CICommon\Src" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Compiler - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\CICommon\Src" /D "_WINDOWS" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Compiler - Win32 Debug Static"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Compiler___Win32_Debug_Static"
# PROP BASE Intermediate_Dir "Compiler___Win32_Debug_Static"
# PROP BASE Target_Dir ""
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug_Static"
# PROP Intermediate_Dir "Debug_Static"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\CICommon\Src" /D "_WINDOWS" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\CICommon\Src" /D "_WINDOWS" /D "WIN32" /D "_DEBUG" /D "_MBCS" /Yu"stdafx.h" /FD /GZ /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Compiler - Win32 Release No Optimizations"

# PROP BASE Use_MFC 1
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Compiler___Win32_Release_No_Optimizations"
# PROP BASE Intermediate_Dir "Compiler___Win32_Release_No_Optimizations"
# PROP BASE Target_Dir ""
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release_No_Optimizations"
# PROP Intermediate_Dir "Release_No_Optimizations"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "..\CICommon\Src" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /Zi /Od /I "..\CICommon\Src" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /c
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

# Name "Compiler - Win32 Release"
# Name "Compiler - Win32 Debug"
# Name "Compiler - Win32 Debug Static"
# Name "Compiler - Win32 Release No Optimizations"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Src\Compiler.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\CompilerKernel.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\DebugFrame.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\LexNsp.cpp

!IF  "$(CFG)" == "Compiler - Win32 Release"

# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "Compiler - Win32 Debug"

# ADD CPP /W1
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "Compiler - Win32 Debug Static"

# ADD BASE CPP /W1
# SUBTRACT BASE CPP /YX /Yc /Yu
# ADD CPP /W1
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "Compiler - Win32 Release No Optimizations"

# SUBTRACT BASE CPP /YX /Yc /Yu
# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Src\Lexyy1st.cpp

!IF  "$(CFG)" == "Compiler - Win32 Release"

# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "Compiler - Win32 Debug"

# ADD CPP /W1
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "Compiler - Win32 Debug Static"

# ADD BASE CPP /W1
# SUBTRACT BASE CPP /YX /Yc /Yu
# ADD CPP /W1
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "Compiler - Win32 Release No Optimizations"

# SUBTRACT BASE CPP /YX /Yc /Yu
# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Src\Lexyy2nd.cpp

!IF  "$(CFG)" == "Compiler - Win32 Release"

# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "Compiler - Win32 Debug"

# ADD CPP /W1
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "Compiler - Win32 Debug Static"

# ADD BASE CPP /W1
# SUBTRACT BASE CPP /YX /Yc /Yu
# ADD CPP /W1
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "Compiler - Win32 Release No Optimizations"

# SUBTRACT BASE CPP /YX /Yc /Yu
# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Src\Nsp.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\Src\PCode.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\PCodeStringTable.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\PCurrentGlobalFunction.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\PCurrentStruct.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\PCurrentUnit.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\PExpr.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\PLex1stPass.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\PLex2ndPass.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\PLexNsp.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\PNamespace.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\PPreview.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\stdafx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\Src\Syntax.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\Src\Type.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\Compiler.h
# End Source File
# Begin Source File

SOURCE=.\Src\CompilerKernel.h
# End Source File
# Begin Source File

SOURCE=.\Src\DebugFrame.h
# End Source File
# Begin Source File

SOURCE=.\Src\Nsp.h
# End Source File
# Begin Source File

SOURCE=.\Src\PCode.h
# End Source File
# Begin Source File

SOURCE=.\Src\PCodeStringTable.h
# End Source File
# Begin Source File

SOURCE=.\Src\PCurrentGlobalFunction.h
# End Source File
# Begin Source File

SOURCE=.\Src\PCurrentStruct.h
# End Source File
# Begin Source File

SOURCE=.\Src\PCurrentUnit.h
# End Source File
# Begin Source File

SOURCE=.\Src\PExpr.h
# End Source File
# Begin Source File

SOURCE=.\Src\PLex1stPass.h
# End Source File
# Begin Source File

SOURCE=.\Src\PLex2ndPass.h
# End Source File
# Begin Source File

SOURCE=.\Src\PLexNsp.h
# End Source File
# Begin Source File

SOURCE=.\Src\PNamespace.h
# End Source File
# Begin Source File

SOURCE=.\Src\PPreview.h
# End Source File
# Begin Source File

SOURCE=.\Src\stdafx.h
# End Source File
# Begin Source File

SOURCE=.\Src\Syntax.h
# End Source File
# Begin Source File

SOURCE=.\Src\Type.h
# End Source File
# End Group
# Begin Group "Grammars"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Flex\LexNsp.y
# End Source File
# Begin Source File

SOURCE=.\Flex\Lexyy1st.y
# End Source File
# Begin Source File

SOURCE=.\Flex\Lexyy2nd.y
# End Source File
# Begin Source File

SOURCE=.\Bison\Nsp.y
# End Source File
# Begin Source File

SOURCE=.\Bison\Syntax.y
# End Source File
# End Group
# Begin Group "Batch"

# PROP Default_Filter "*.bat"
# Begin Source File

SOURCE=.\Batch\Make1st.bat
# End Source File
# Begin Source File

SOURCE=.\Batch\Make2nd.bat
# End Source File
# Begin Source File

SOURCE=.\Batch\MakeLexNsp.bat
# End Source File
# Begin Source File

SOURCE=.\Batch\MakeNsp.bat
# End Source File
# Begin Source File

SOURCE=.\Batch\MakeSyntax.bat
# End Source File
# End Group
# End Target
# End Project
