# Microsoft Developer Studio Project File - Name="MapEditor" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=MapEditor - Win32 Debug Static
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "MapEditor.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "MapEditor.mak" CFG="MapEditor - Win32 Debug Static"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "MapEditor - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "MapEditor - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "MapEditor - Win32 Release No Optimizations" (based on "Win32 (x86) Application")
!MESSAGE "MapEditor - Win32 Debug Static" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/MapEditor", OABAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "MapEditor - Win32 Release"

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
# ADD BASE RSC /l 0x405 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x405 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 ddraw.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"MSVCRT" /nodefaultlib:"mfc42.lib" /nodefaultlib:"LIBC"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "MapEditor - Win32 Debug"

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
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x405 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x405 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ddraw.lib FindPathGraphCreation\Debug\FindPathGraphCreation.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"LIBCD"
# SUBTRACT LINK32 /profile

!ELSEIF  "$(CFG)" == "MapEditor - Win32 Release No Optimizations"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "MapEditor___Win32_Release_No_Optimizations"
# PROP BASE Intermediate_Dir "MapEditor___Win32_Release_No_Optimizations"
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
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x405 /d "NDEBUG"
# ADD RSC /l 0x405 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ddraw.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"MSVCRT" /nodefaultlib:"mfc42.lib"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 ddraw.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"MSVCRT" /nodefaultlib:"libc" /nodefaultlib:"mfc42.lib"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "MapEditor - Win32 Debug Static"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "MapEditor___Win32_Debug_Static"
# PROP BASE Intermediate_Dir "MapEditor___Win32_Debug_Static"
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
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x405 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x405 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ddraw.lib FindPathGraphCreation\Debug\FindPathGraphCreation.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"LIBCD"
# SUBTRACT BASE LINK32 /profile
# ADD LINK32 ddraw.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"LIBCD"
# SUBTRACT LINK32 /profile

!ENDIF 

# Begin Target

# Name "MapEditor - Win32 Release"
# Name "MapEditor - Win32 Debug"
# Name "MapEditor - Win32 Release No Optimizations"
# Name "MapEditor - Win32 Debug Static"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\MapEditor.cpp
# End Source File
# Begin Source File

SOURCE=.\MapEditor.rc
# End Source File
# Begin Source File

SOURCE=.\MapEditorDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\MapEditorView.cpp
# End Source File
# Begin Source File

SOURCE=.\dialogs\SetMapSizeDlg.cpp
# ADD CPP /I ".."
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\Controls\UnitTypeControl.cpp
# ADD CPP /I ".."
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\Constants.h
# End Source File
# Begin Source File

SOURCE=.\MainFrm.h
# End Source File
# Begin Source File

SOURCE=.\MapEditor.h
# End Source File
# Begin Source File

SOURCE=.\MapEditorDoc.h
# End Source File
# Begin Source File

SOURCE=.\MapEditorView.h
# End Source File
# Begin Source File

SOURCE=.\MapFormats.h
# End Source File
# Begin Source File

SOURCE=.\MapOldFormats.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\dialogs\SetMapSizeDlg.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\Controls\UnitTypeControl.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\bitmap1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00001.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00002.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00003.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00004.bmp
# End Source File
# Begin Source File

SOURCE=.\res\cur00001.cur
# End Source File
# Begin Source File

SOURCE=.\res\cursor1.cur
# End Source File
# Begin Source File

SOURCE=.\res\ico00001.ico
# End Source File
# Begin Source File

SOURCE=.\res\ico00002.ico
# End Source File
# Begin Source File

SOURCE=.\res\icon1.ico
# End Source File
# Begin Source File

SOURCE=.\res\krystal.bmp
# End Source File
# Begin Source File

SOURCE=.\res\krystal2.bmp
# End Source File
# Begin Source File

SOURCE=.\res\krystal3.bmp
# End Source File
# Begin Source File

SOURCE=.\res\MapEditor.ico
# End Source File
# Begin Source File

SOURCE=.\res\MapEditor.rc2
# End Source File
# Begin Source File

SOURCE=.\res\MapEditorDoc.ico
# End Source File
# Begin Source File

SOURCE=.\res\Toolbar.bmp
# End Source File
# Begin Source File

SOURCE=.\res\toolbar1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\ToolbarColor.bmp
# End Source File
# Begin Source File

SOURCE=.\res\ToolbarDisable.bmp
# End Source File
# Begin Source File

SOURCE=.\res\ToolbarGray.bmp
# End Source File
# End Group
# Begin Group "DataObjects"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\DataObjects\ECivilization.cpp
# End Source File
# Begin Source File

SOURCE=.\DataObjects\ECivilization.h
# End Source File
# Begin Source File

SOURCE=.\DataObjects\EFindPathGraph.cpp
# End Source File
# Begin Source File

SOURCE=.\DataObjects\EFindPathGraph.h
# End Source File
# Begin Source File

SOURCE=.\DataObjects\EInvisibility.cpp
# End Source File
# Begin Source File

SOURCE=.\DataObjects\EInvisibility.h
# End Source File
# Begin Source File

SOURCE=.\DataObjects\EMap.cpp
# End Source File
# Begin Source File

SOURCE=.\DataObjects\EMap.h
# End Source File
# Begin Source File

SOURCE=.\DataObjects\EMapex.cpp
# End Source File
# Begin Source File

SOURCE=.\DataObjects\EMapex.h
# End Source File
# Begin Source File

SOURCE=.\DataObjects\EMapexInstance.cpp
# End Source File
# Begin Source File

SOURCE=.\DataObjects\EMapexInstance.h
# End Source File
# Begin Source File

SOURCE=.\DataObjects\EMapexLibrary.cpp
# End Source File
# Begin Source File

SOURCE=.\DataObjects\EMapexLibrary.h
# End Source File
# Begin Source File

SOURCE=.\DataObjects\EMapSquare.cpp
# End Source File
# Begin Source File

SOURCE=.\DataObjects\EMapSquare.h
# End Source File
# Begin Source File

SOURCE=.\DataObjects\EResource.cpp
# End Source File
# Begin Source File

SOURCE=.\DataObjects\EResource.h
# End Source File
# Begin Source File

SOURCE=.\DataObjects\EScriptSet.cpp
# End Source File
# Begin Source File

SOURCE=.\DataObjects\EScriptSet.h
# End Source File
# Begin Source File

SOURCE=.\DataObjects\EULMapexInstance.cpp
# End Source File
# Begin Source File

SOURCE=.\DataObjects\EULMapexInstance.h
# End Source File
# Begin Source File

SOURCE=.\DataObjects\EUnit.cpp
# End Source File
# Begin Source File

SOURCE=.\DataObjects\EUnit.h
# End Source File
# Begin Source File

SOURCE=.\DataObjects\EUnitAnimation.cpp
# End Source File
# Begin Source File

SOURCE=.\DataObjects\EUnitAnimation.h
# End Source File
# Begin Source File

SOURCE=.\DataObjects\EUnitAppearance.cpp
# End Source File
# Begin Source File

SOURCE=.\DataObjects\EUnitAppearance.h
# End Source File
# Begin Source File

SOURCE=.\DataObjects\EUnitAppearanceType.cpp
# End Source File
# Begin Source File

SOURCE=.\DataObjects\EUnitAppearanceType.h
# End Source File
# Begin Source File

SOURCE=.\DataObjects\EUnitLibrary.cpp
# End Source File
# Begin Source File

SOURCE=.\DataObjects\EUnitLibrary.h
# End Source File
# Begin Source File

SOURCE=.\DataObjects\EUnitSkillType.cpp
# End Source File
# Begin Source File

SOURCE=.\DataObjects\EUnitSkillType.h
# End Source File
# Begin Source File

SOURCE=.\DataObjects\EUnitSurface.cpp
# End Source File
# Begin Source File

SOURCE=.\DataObjects\EUnitSurface.h
# End Source File
# Begin Source File

SOURCE=.\DataObjects\EUnitType.cpp
# End Source File
# Begin Source File

SOURCE=.\DataObjects\EUnitType.h
# End Source File
# Begin Source File

SOURCE=.\DataObjects\LandType.cpp
# End Source File
# Begin Source File

SOURCE=.\DataObjects\LandType.h
# End Source File
# End Group
# Begin Group "MapexEditor"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\MapexEditor\DeleteMapexLibraryDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MapexEditor\DeleteMapexLibraryDlg.h
# End Source File
# Begin Source File

SOURCE=.\MapexEditor\EditMapexLibraryDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MapexEditor\EditMapexLibraryDlg.h
# End Source File
# Begin Source File

SOURCE=.\MapexEditor\LandTypeSelectDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MapexEditor\LandTypeSelectDlg.h
# End Source File
# Begin Source File

SOURCE=.\MapexEditor\LandTypeTBButton.cpp
# End Source File
# Begin Source File

SOURCE=.\MapexEditor\LandTypeTBButton.h
# End Source File
# Begin Source File

SOURCE=.\MapexEditor\MapexEdit.cpp
# End Source File
# Begin Source File

SOURCE=.\MapexEditor\MapexEdit.h
# End Source File
# Begin Source File

SOURCE=.\MapexEditor\MapexEditorDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MapexEditor\MapexEditorDlg.h
# End Source File
# Begin Source File

SOURCE=.\MapexEditor\MapexEditView.cpp
# End Source File
# Begin Source File

SOURCE=.\MapexEditor\MapexEditView.h
# End Source File
# Begin Source File

SOURCE=.\MapexEditor\MapexLibraryEditDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MapexEditor\MapexLibraryEditDlg.h
# End Source File
# Begin Source File

SOURCE=.\MapexEditor\MapexSizeDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MapexEditor\MapexSizeDlg.h
# End Source File
# End Group
# Begin Group "UnitEditor"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\UnitEditor\DeleteUnitLibraryDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\UnitEditor\DeleteUnitLibraryDlg.h
# End Source File
# Begin Source File

SOURCE=.\UnitEditor\EditUnitLibraryDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\UnitEditor\EditUnitLibraryDlg.h
# End Source File
# Begin Source File

SOURCE=.\UnitEditor\NewSkillTypeDlg.cpp
# ADD CPP /I ".."
# End Source File
# Begin Source File

SOURCE=.\UnitEditor\NewSkillTypeDlg.h
# End Source File
# Begin Source File

SOURCE=.\UnitEditor\UnitAnimationPreview.cpp
# End Source File
# Begin Source File

SOURCE=.\UnitEditor\UnitAnimationPreview.h
# End Source File
# Begin Source File

SOURCE=.\UnitEditor\UnitAppearanceDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\UnitEditor\UnitAppearanceDlg.h
# End Source File
# Begin Source File

SOURCE=.\UnitEditor\UnitAppearancePage.cpp
# End Source File
# Begin Source File

SOURCE=.\UnitEditor\UnitAppearancePage.h
# End Source File
# Begin Source File

SOURCE=.\UnitEditor\UnitDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\UnitEditor\UnitDlg.h
# End Source File
# Begin Source File

SOURCE=.\UnitEditor\unitlandtypespage.cpp
# ADD CPP /I ".."
# End Source File
# Begin Source File

SOURCE=.\UnitEditor\unitlandtypespage.h
# End Source File
# Begin Source File

SOURCE=.\UnitEditor\UnitLibrariesDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\UnitEditor\UnitLibrariesDlg.h
# End Source File
# Begin Source File

SOURCE=.\UnitEditor\UnitLibraryEditorDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\UnitEditor\UnitLibraryEditorDlg.h
# End Source File
# Begin Source File

SOURCE=.\UnitEditor\UnitMainPage.cpp
# End Source File
# Begin Source File

SOURCE=.\UnitEditor\UnitMainPage.h
# End Source File
# Begin Source File

SOURCE=.\UnitEditor\UnitModesPage.cpp
# End Source File
# Begin Source File

SOURCE=.\UnitEditor\UnitModesPage.h
# End Source File
# Begin Source File

SOURCE=.\UnitEditor\UnitSkillsPage.cpp
# ADD CPP /I ".."
# End Source File
# Begin Source File

SOURCE=.\UnitEditor\UnitSkillsPage.h
# End Source File
# Begin Source File

SOURCE=.\UnitEditor\UnitSkillTypeCollection.cpp
# End Source File
# Begin Source File

SOURCE=.\UnitEditor\UnitSkillTypeCollection.h
# End Source File
# Begin Source File

SOURCE=.\UnitEditor\UnitSkillTypeDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\UnitEditor\UnitSkillTypeDlg.h
# End Source File
# Begin Source File

SOURCE=.\UnitEditor\UnitTypePreview.cpp
# End Source File
# Begin Source File

SOURCE=.\UnitEditor\UnitTypePreview.h
# End Source File
# End Group
# Begin Group "Controls"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Controls\AppearanceControl.cpp
# ADD CPP /I ".."
# End Source File
# Begin Source File

SOURCE=.\Controls\AppearanceControl.h
# End Source File
# Begin Source File

SOURCE=.\Controls\ColorPicker.cpp
# End Source File
# Begin Source File

SOURCE=.\Controls\ColorPicker.h
# End Source File
# Begin Source File

SOURCE=.\Controls\ImageTBButton.cpp
# End Source File
# Begin Source File

SOURCE=.\Controls\ImageTBButton.h
# End Source File
# Begin Source File

SOURCE=.\Controls\ImageToolBar.cpp
# End Source File
# Begin Source File

SOURCE=.\Controls\ImageToolBar.h
# End Source File
# Begin Source File

SOURCE=.\Controls\InvisibilityControl.cpp
# End Source File
# Begin Source File

SOURCE=.\Controls\InvisibilityControl.h
# End Source File
# Begin Source File

SOURCE=.\Controls\InvisibilityControlInner.cpp
# End Source File
# Begin Source File

SOURCE=.\Controls\InvisibilityControlInner.h
# End Source File
# Begin Source File

SOURCE=.\Controls\MapexPreview.cpp
# End Source File
# Begin Source File

SOURCE=.\Controls\MapexPreview.h
# End Source File
# Begin Source File

SOURCE=.\Controls\MapexTBButton.cpp
# End Source File
# Begin Source File

SOURCE=.\Controls\MapexTBButton.h
# End Source File
# Begin Source File

SOURCE=.\Controls\MapexToolBar.cpp
# End Source File
# Begin Source File

SOURCE=.\Controls\MapexToolBar.h
# End Source File
# Begin Source File

SOURCE=.\Controls\ResourceControl.cpp
# ADD CPP /I ".."
# End Source File
# Begin Source File

SOURCE=.\Controls\ResourceControl.h
# End Source File
# Begin Source File

SOURCE=.\Controls\ResourceControlInner.cpp
# ADD CPP /I ".."
# End Source File
# Begin Source File

SOURCE=.\Controls\ResourceControlInner.h
# End Source File
# Begin Source File

SOURCE=.\Controls\ResourcePreview.cpp
# ADD CPP /I ".."
# End Source File
# Begin Source File

SOURCE=.\Controls\ResourcePreview.h
# End Source File
# Begin Source File

SOURCE=.\Controls\UnitTBButton.cpp
# End Source File
# Begin Source File

SOURCE=.\Controls\UnitTBButton.h
# End Source File
# Begin Source File

SOURCE=.\Controls\UnitToolBar.cpp
# End Source File
# Begin Source File

SOURCE=.\Controls\UnitToolBar.h
# End Source File
# Begin Source File

SOURCE=.\Controls\ZPosCombo.cpp
# End Source File
# Begin Source File

SOURCE=.\Controls\ZPosCombo.h
# End Source File
# Begin Source File

SOURCE=.\Controls\ZPosControl.cpp
# ADD CPP /I ".."
# End Source File
# Begin Source File

SOURCE=.\Controls\ZPosControl.h
# End Source File
# End Group
# Begin Group "Dialogs"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Dialogs\EditInvisibility.cpp
# End Source File
# Begin Source File

SOURCE=.\Dialogs\EditInvisibility.h
# End Source File
# Begin Source File

SOURCE=.\Dialogs\EditResourcesDlg.cpp
# ADD CPP /I ".."
# End Source File
# Begin Source File

SOURCE=.\Dialogs\EditResourcesDlg.h
# End Source File
# Begin Source File

SOURCE=.\Dialogs\FindPathGraphEditDlg.cpp
# ADD CPP /I ".."
# End Source File
# Begin Source File

SOURCE=.\Dialogs\FindPathGraphEditDlg.h
# End Source File
# Begin Source File

SOURCE=.\Dialogs\FindPathGraphsDlg.cpp
# ADD CPP /I ".."
# End Source File
# Begin Source File

SOURCE=.\Dialogs\FindPathGraphsDlg.h
# End Source File
# Begin Source File

SOURCE=.\Controls\finishdlg.cpp
# ADD CPP /I ".."
# End Source File
# Begin Source File

SOURCE=.\Controls\finishdlg.h
# End Source File
# Begin Source File

SOURCE=.\Dialogs\InvisibilityDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Dialogs\InvisibilityDlg.h
# End Source File
# Begin Source File

SOURCE=.\Dialogs\MapPropertiesDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Dialogs\MapPropertiesDlg.h
# End Source File
# Begin Source File

SOURCE=.\Dialogs\NewMapDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Dialogs\NewMapDlg.h
# End Source File
# Begin Source File

SOURCE=.\Dialogs\ResourcesDlg.cpp
# ADD CPP /I ".."
# End Source File
# Begin Source File

SOURCE=.\Dialogs\ResourcesDlg.h
# End Source File
# Begin Source File

SOURCE=.\Dialogs\ScriptSetDlg.cpp
# ADD CPP /I ".."
# End Source File
# Begin Source File

SOURCE=.\Dialogs\ScriptSetDlg.h
# End Source File
# Begin Source File

SOURCE=.\Dialogs\ScriptSetEditDlg.cpp
# ADD CPP /I ".."
# End Source File
# Begin Source File

SOURCE=.\Dialogs\ScriptSetEditDlg.h
# End Source File
# Begin Source File

SOURCE=.\Dialogs\SelectMapexDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Dialogs\SelectMapexDlg.h
# End Source File
# Begin Source File

SOURCE=.\Dialogs\SelectUnitdlg.cpp
# ADD CPP /I ".."
# End Source File
# Begin Source File

SOURCE=.\Dialogs\SelectUnitdlg.h
# End Source File
# Begin Source File

SOURCE=.\Dialogs\SplashWindowDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Dialogs\SplashWindowDlg.h
# End Source File
# Begin Source File

SOURCE=.\Dialogs\StartDlg.cpp
# ADD CPP /I ".."
# End Source File
# Begin Source File

SOURCE=.\Dialogs\StartDlg.h
# End Source File
# End Group
# Begin Group "CivilizationEditor"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CivilizationEditor\CivilizationDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\CivilizationEditor\CivilizationDlg.h
# End Source File
# Begin Source File

SOURCE=.\CivilizationEditor\CivilizationEditorDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\CivilizationEditor\CivilizationEditorDlg.h
# End Source File
# End Group
# Begin Group "AbstractDataClasses"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\AbstractDataClasses\256BitArray.h
# End Source File
# Begin Source File

SOURCE=.\AbstractDataClasses\BitArray.h
# End Source File
# Begin Source File

SOURCE=.\AbstractDataClasses\StaticBitArray.h
# End Source File
# End Group
# Begin Group "SkillTypes"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\skilltypes\EBulletAttackSkillTypeDlg.cpp
# ADD CPP /I ".."
# End Source File
# Begin Source File

SOURCE=.\skilltypes\EBulletAttackSkillTypeDlg.h
# End Source File
# Begin Source File

SOURCE=.\skilltypes\EBulletDefenseSkillTypeDlg.cpp
# ADD CPP /I ".."
# End Source File
# Begin Source File

SOURCE=.\skilltypes\EBulletDefenseSkillTypeDlg.h
# End Source File
# Begin Source File

SOURCE=.\skilltypes\emakeskilltypedlg.cpp
# End Source File
# Begin Source File

SOURCE=.\skilltypes\emakeskilltypedlg.h
# End Source File
# Begin Source File

SOURCE=.\skilltypes\EMakeSkillTypeRecordDlg.cpp
# ADD CPP /I ".."
# End Source File
# Begin Source File

SOURCE=.\skilltypes\EMakeSkillTypeRecordDlg.h
# End Source File
# Begin Source File

SOURCE=.\skilltypes\EMineSkillTypeDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\skilltypes\EMineSkillTypeDlg.h
# End Source File
# Begin Source File

SOURCE=.\SkillTypes\EMoveSkillTypeDlg.cpp
# ADD CPP /I ".."
# End Source File
# Begin Source File

SOURCE=.\SkillTypes\EMoveSkillTypeDlg.h
# End Source File
# Begin Source File

SOURCE=.\skilltypes\EResourceSkillTypeDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\skilltypes\EResourceSkillTypeDlg.h
# End Source File
# Begin Source File

SOURCE=.\skilltypes\EStorageSkillTypeDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\skilltypes\EStorageSkillTypeDlg.h
# End Source File
# Begin Source File

SOURCE=.\SkillTypes\ExampleSkillTypeDlg.cpp
# ADD CPP /I ".."
# End Source File
# Begin Source File

SOURCE=.\SkillTypes\ExampleSkillTypeDlg.h
# End Source File
# End Group
# Begin Group "Minimap"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\MiniMap.cpp
# End Source File
# Begin Source File

SOURCE=.\MiniMap.h
# End Source File
# Begin Source File

SOURCE=.\MiniMapFrame.cpp
# End Source File
# Begin Source File

SOURCE=.\MiniMapFrame.h
# End Source File
# End Group
# Begin Group "Selection"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Clipboard.cpp
# End Source File
# Begin Source File

SOURCE=.\Clipboard.h
# End Source File
# Begin Source File

SOURCE=.\MapexInstanceSelection.cpp
# End Source File
# Begin Source File

SOURCE=.\MapexInstanceSelection.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\MapEditor.reg
# End Source File
# Begin Source File

SOURCE=.\splashfin.bmp
# End Source File
# End Target
# End Project
