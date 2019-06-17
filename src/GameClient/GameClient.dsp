# Microsoft Developer Studio Project File - Name="GameClient" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=GameClient - Win32 Debug Static
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "GameClient.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "GameClient.mak" CFG="GameClient - Win32 Debug Static"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "GameClient - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "GameClient - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "GameClient - Win32 Debug Static" (based on "Win32 (x86) Application")
!MESSAGE "GameClient - Win32 Release No Optimizations" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/GameClient", WABAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "GameClient - Win32 Release"

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
# ADD LINK32 ddraw.lib dsound.lib dinput.lib ws2_32.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"MSVCRT" /nodefaultlib:"libc"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "GameClient - Win32 Debug"

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
# ADD BASE RSC /l 0x405 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x405 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ddraw.lib dsound.lib dinput.lib ws2_32.lib /nologo /subsystem:windows /profile /map /debug /machine:I386 /nodefaultlib:"LIBCD"

!ELSEIF  "$(CFG)" == "GameClient - Win32 Debug Static"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "GameClient___Win32_Debug_Static"
# PROP BASE Intermediate_Dir "GameClient___Win32_Debug_Static"
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
# ADD BASE RSC /l 0x405 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x405 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ddraw.lib dsound.lib dinput.lib ws2_32.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"LIBCD" /pdbtype:sept /libpath:"D:\Vitek\Projekt\Network\Network"
# ADD LINK32 ddraw.lib dsound.lib dinput.lib ws2_32.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"LIBCD" /pdbtype:sept /libpath:"D:\Vitek\Projekt\Network\Network"

!ELSEIF  "$(CFG)" == "GameClient - Win32 Release No Optimizations"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "GameClient___Win32_Release_No_Optimizations"
# PROP BASE Intermediate_Dir "GameClient___Win32_Release_No_Optimizations"
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
# ADD BASE RSC /l 0x405 /d "NDEBUG"
# ADD RSC /l 0x405 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ddraw.lib dsound.lib dinput.lib ws2_32.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"MSVCRT"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 ddraw.lib dsound.lib dinput.lib ws2_32.lib /nologo /subsystem:windows /profile /map /debug /machine:I386 /nodefaultlib:"MSVCRT" /nodefaultlib:"LIBC"

!ENDIF 

# Begin Target

# Name "GameClient - Win32 Release"
# Name "GameClient - Win32 Debug"
# Name "GameClient - Win32 Debug Static"
# Name "GameClient - Win32 Release No Optimizations"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\ExclusiveKeyboardWindow.cpp
# End Source File
# Begin Source File

SOURCE=.\GameClient.cpp
# End Source File
# Begin Source File

SOURCE=.\GameClient.rc
# End Source File
# Begin Source File

SOURCE=.\GameClientApplication.cpp
# End Source File
# Begin Source File

SOURCE=.\GameClientHelperFunctions.cpp
# End Source File
# Begin Source File

SOURCE=.\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\ScriptEditor\OpenLocalScriptDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\ExclusiveKeyboardWindow.h
# End Source File
# Begin Source File

SOURCE=.\GameClient.h
# End Source File
# Begin Source File

SOURCE=.\GameClientApplication.h
# End Source File
# Begin Source File

SOURCE=.\GameClientGlobal.h
# End Source File
# Begin Source File

SOURCE=.\GameClientHelperFunctions.h
# End Source File
# Begin Source File

SOURCE=.\GameClientNetwork.h
# End Source File
# Begin Source File

SOURCE=.\LoadException.h
# End Source File
# Begin Source File

SOURCE=.\MainFrm.h
# End Source File
# Begin Source File

SOURCE=.\ScriptEditor\OpenLocalScriptDlg.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\GameClient.ico
# End Source File
# Begin Source File

SOURCE=.\res\GameClient.rc2
# End Source File
# End Group
# Begin Group "DataObjects"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\DataObjects\CCivilization.cpp
# End Source File
# Begin Source File

SOURCE=.\DataObjects\CCivilization.h
# End Source File
# Begin Source File

SOURCE=.\DataObjects\CGeneralUnitAppearance.cpp
# End Source File
# Begin Source File

SOURCE=.\DataObjects\CGeneralUnitAppearance.h
# End Source File
# Begin Source File

SOURCE=.\DataObjects\CGeneralUnitType.cpp
# End Source File
# Begin Source File

SOURCE=.\DataObjects\CGeneralUnitType.h
# End Source File
# Begin Source File

SOURCE=.\DataObjects\CInisibility.cpp
# End Source File
# Begin Source File

SOURCE=.\DataObjects\CInvisibility.h
# End Source File
# Begin Source File

SOURCE=.\DataObjects\CMap.cpp
# End Source File
# Begin Source File

SOURCE=.\DataObjects\CMap.h
# End Source File
# Begin Source File

SOURCE=.\DataObjects\CMapex.cpp
# End Source File
# Begin Source File

SOURCE=.\DataObjects\CMapex.h
# End Source File
# Begin Source File

SOURCE=.\DataObjects\CMapexInstance.cpp
# End Source File
# Begin Source File

SOURCE=.\DataObjects\CMapexInstance.h
# End Source File
# Begin Source File

SOURCE=.\DataObjects\CMapexLibrary.cpp
# End Source File
# Begin Source File

SOURCE=.\DataObjects\CMapexLibrary.h
# End Source File
# Begin Source File

SOURCE=.\DataObjects\CMapSquare.cpp
# End Source File
# Begin Source File

SOURCE=.\DataObjects\CMapSquare.h
# End Source File
# Begin Source File

SOURCE=.\DataObjects\CResource.cpp
# End Source File
# Begin Source File

SOURCE=.\DataObjects\CResource.h
# End Source File
# Begin Source File

SOURCE=.\DataObjects\CULMapexInstance.cpp
# End Source File
# Begin Source File

SOURCE=.\DataObjects\CULMapexInstance.h
# End Source File
# Begin Source File

SOURCE=.\DataObjects\CUnit.cpp
# End Source File
# Begin Source File

SOURCE=.\DataObjects\CUnit.h
# End Source File
# Begin Source File

SOURCE=.\DataObjects\CUnitAnimation.cpp
# End Source File
# Begin Source File

SOURCE=.\DataObjects\CUnitAnimation.h
# End Source File
# Begin Source File

SOURCE=.\DataObjects\CUnitAppearance.cpp
# End Source File
# Begin Source File

SOURCE=.\DataObjects\CUnitAppearance.h
# End Source File
# Begin Source File

SOURCE=.\DataObjects\CUnitSurface.cpp
# End Source File
# Begin Source File

SOURCE=.\DataObjects\CUnitSurface.h
# End Source File
# Begin Source File

SOURCE=.\DataObjects\CUnitSurfaceManager.cpp
# End Source File
# Begin Source File

SOURCE=.\DataObjects\CUnitSurfaceManager.h
# End Source File
# Begin Source File

SOURCE=.\DataObjects\CUnitType.cpp
# End Source File
# Begin Source File

SOURCE=.\DataObjects\CUnitType.h
# End Source File
# Begin Source File

SOURCE=.\DataObjects\CUnitTypeLibrary.cpp
# End Source File
# Begin Source File

SOURCE=.\DataObjects\CUnitTypeLibrary.h
# End Source File
# Begin Source File

SOURCE=.\DataObjects\MapexCache.cpp
# End Source File
# Begin Source File

SOURCE=.\DataObjects\MapexCache.h
# End Source File
# Begin Source File

SOURCE=.\DataObjects\UnitAnimationCache.cpp
# End Source File
# Begin Source File

SOURCE=.\DataObjects\UnitAnimationCache.h
# End Source File
# Begin Source File

SOURCE=.\DataObjects\UnitCache.cpp
# End Source File
# Begin Source File

SOURCE=.\DataObjects\UnitCache.h
# End Source File
# Begin Source File

SOURCE=.\DataObjects\UnitTypeCache.cpp
# End Source File
# Begin Source File

SOURCE=.\DataObjects\UnitTypeCache.h
# End Source File
# End Group
# Begin Group "GraphicalObjects"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\GraphicalObjects\BackgroundCache.cpp
# End Source File
# Begin Source File

SOURCE=.\GraphicalObjects\BackgroundCache.h
# End Source File
# Begin Source File

SOURCE=.\GraphicalObjects\BackgroundCachePiece.cpp
# End Source File
# Begin Source File

SOURCE=.\GraphicalObjects\BackgroundCachePiece.h
# End Source File
# Begin Source File

SOURCE=.\GraphicalObjects\SpecialBlt.cpp
# End Source File
# Begin Source File

SOURCE=.\GraphicalObjects\SpecialBlt.h
# End Source File
# Begin Source File

SOURCE=.\GraphicalObjects\Viewport.cpp
# End Source File
# Begin Source File

SOURCE=.\GraphicalObjects\Viewport.h
# End Source File
# End Group
# Begin Group "ScriptEditor"

# PROP Default_Filter ""
# Begin Group "HelpBrowser"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ScriptEditor\HelpBrowser\HelpBrowser.cpp

!IF  "$(CFG)" == "GameClient - Win32 Release"

# ADD CPP /Od
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "GameClient - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "GameClient - Win32 Debug Static"

# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "GameClient - Win32 Release No Optimizations"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScriptEditor\HelpBrowser\HelpBrowser.h
# End Source File
# Begin Source File

SOURCE=.\ScriptEditor\HelpBrowser\HelpBrowserScroll.cpp

!IF  "$(CFG)" == "GameClient - Win32 Release"

# ADD CPP /O2

!ELSEIF  "$(CFG)" == "GameClient - Win32 Debug"

!ELSEIF  "$(CFG)" == "GameClient - Win32 Debug Static"

!ELSEIF  "$(CFG)" == "GameClient - Win32 Release No Optimizations"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScriptEditor\HelpBrowser\HelpBrowserScroll.h
# End Source File
# Begin Source File

SOURCE=.\ScriptEditor\HelpBrowser\HelpBrowserWindow.cpp

!IF  "$(CFG)" == "GameClient - Win32 Release"

# ADD CPP /Od /I "..\.."
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "GameClient - Win32 Debug"

# ADD CPP /I "..\.."
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "GameClient - Win32 Debug Static"

# ADD CPP /I "..\.."
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "GameClient - Win32 Release No Optimizations"

# ADD CPP /I "..\.."
# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScriptEditor\HelpBrowser\HelpBrowserWindow.h
# End Source File
# Begin Source File

SOURCE=.\ScriptEditor\HelpBrowser\webbrowser2.cpp

!IF  "$(CFG)" == "GameClient - Win32 Release"

# ADD CPP /O2

!ELSEIF  "$(CFG)" == "GameClient - Win32 Debug"

!ELSEIF  "$(CFG)" == "GameClient - Win32 Debug Static"

!ELSEIF  "$(CFG)" == "GameClient - Win32 Release No Optimizations"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScriptEditor\HelpBrowser\webbrowser2.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\ScriptEditor\ErrorOutput.cpp
# End Source File
# Begin Source File

SOURCE=.\ScriptEditor\ErrorOutput.h
# End Source File
# Begin Source File

SOURCE=.\ScriptEditor\ErrorOutputEdit.cpp
# End Source File
# Begin Source File

SOURCE=.\ScriptEditor\ErrorOutputEdit.h
# End Source File
# Begin Source File

SOURCE=.\ScriptEditor\HelpFrame.cpp
# End Source File
# Begin Source File

SOURCE=.\ScriptEditor\HelpFrame.h
# End Source File
# Begin Source File

SOURCE=.\ScriptEditor\LogWindow.cpp
# End Source File
# Begin Source File

SOURCE=.\ScriptEditor\LogWindow.h
# End Source File
# Begin Source File

SOURCE=.\ScriptEditor\OpenRemoteScriptDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ScriptEditor\OpenRemoteScriptDlg.h
# End Source File
# Begin Source File

SOURCE=.\ScriptEditor\ProfilingFrame.cpp
# End Source File
# Begin Source File

SOURCE=.\ScriptEditor\ProfilingFrame.h
# End Source File
# Begin Source File

SOURCE=.\ScriptEditor\SaveRemoteScriptDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ScriptEditor\SaveRemoteScriptDlg.h
# End Source File
# Begin Source File

SOURCE=.\ScriptEditor\ScriptEditor.cpp
# End Source File
# Begin Source File

SOURCE=.\ScriptEditor\ScriptEditor.h
# End Source File
# Begin Source File

SOURCE=.\ScriptEditor\ScriptMDIFrame.cpp
# End Source File
# Begin Source File

SOURCE=.\ScriptEditor\ScriptMDIFrame.h
# End Source File
# Begin Source File

SOURCE=.\ScriptEditor\ScriptView.cpp
# End Source File
# Begin Source File

SOURCE=.\ScriptEditor\ScriptView.h
# End Source File
# Begin Source File

SOURCE=.\ScriptEditor\ScriptViewEdit.cpp
# End Source File
# Begin Source File

SOURCE=.\ScriptEditor\ScriptViewEdit.h
# End Source File
# End Group
# Begin Group "Controls"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Controls\ConnectDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Controls\ConnectDlg.h
# End Source File
# Begin Source File

SOURCE=.\DesktopWindow.cpp
# End Source File
# Begin Source File

SOURCE=.\DesktopWindow.h
# End Source File
# Begin Source File

SOURCE=.\DownloadMap.cpp
# End Source File
# Begin Source File

SOURCE=.\DownloadMap.h
# End Source File
# Begin Source File

SOURCE=.\Controls\ProgressWindow.cpp
# End Source File
# Begin Source File

SOURCE=.\Controls\ProgressWindow.h
# End Source File
# Begin Source File

SOURCE=.\Controls\ScreenWindow.cpp
# End Source File
# Begin Source File

SOURCE=.\Controls\ScreenWindow.h
# End Source File
# End Group
# Begin Group "Server Communication"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ServerCommunication\CRequestVirtualConnection.cpp
# End Source File
# Begin Source File

SOURCE=.\ServerCommunication\CRequestVirtualConnection.h
# End Source File
# Begin Source File

SOURCE=.\ServerCommunication\CServerUnitInfoReceiver.cpp
# End Source File
# Begin Source File

SOURCE=.\ServerCommunication\CServerUnitInfoReceiver.h
# End Source File
# Begin Source File

SOURCE=.\ServerCommunication\CTimestampedVirtualConnection.cpp
# End Source File
# Begin Source File

SOURCE=.\ServerCommunication\CTimestampedVirtualConnection.h
# End Source File
# Begin Source File

SOURCE=.\ServerCommunication\CWatchedRectangle.cpp
# End Source File
# Begin Source File

SOURCE=.\ServerCommunication\CWatchedRectangle.h
# End Source File
# End Group
# Begin Group "MapView"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\MapView\InteractiveViewport.cpp
# End Source File
# Begin Source File

SOURCE=.\MapView\InteractiveViewport.h
# End Source File
# Begin Source File

SOURCE=.\MapView\MapScrollHelper.cpp
# End Source File
# Begin Source File

SOURCE=.\MapView\MapScrollHelper.h
# End Source File
# Begin Source File

SOURCE=.\MapView\MapView.cpp
# End Source File
# Begin Source File

SOURCE=.\MapView\MapView.h
# End Source File
# Begin Source File

SOURCE=.\MapView\MapViewMessageBar.cpp
# End Source File
# Begin Source File

SOURCE=.\MapView\MapViewMessageBar.h
# End Source File
# Begin Source File

SOURCE=.\MapView\MapViewStatusBar.cpp
# End Source File
# Begin Source File

SOURCE=.\MapView\MapViewStatusBar.h
# End Source File
# Begin Source File

SOURCE=.\MapView\MapViewToolbar.cpp
# End Source File
# Begin Source File

SOURCE=.\MapView\MapViewToolbar.h
# End Source File
# Begin Source File

SOURCE=.\MapView\MinimapWindow.cpp
# End Source File
# Begin Source File

SOURCE=.\MapView\MinimapWindow.h
# End Source File
# Begin Source File

SOURCE=.\MapView\ResourcesBar.cpp
# End Source File
# Begin Source File

SOURCE=.\MapView\ResourcesBar.h
# End Source File
# Begin Source File

SOURCE=.\MapView\SelectionWindow.cpp
# End Source File
# Begin Source File

SOURCE=.\MapView\SelectionWindow.h
# End Source File
# Begin Source File

SOURCE=.\MapView\UnitInterpolations.cpp
# End Source File
# Begin Source File

SOURCE=.\MapView\UnitInterpolations.h
# End Source File
# Begin Source File

SOURCE=.\MapView\UnitSelection.cpp
# End Source File
# Begin Source File

SOURCE=.\MapView\UnitSelection.h
# End Source File
# Begin Source File

SOURCE=.\MapView\WatchUnitFrame.cpp
# End Source File
# Begin Source File

SOURCE=.\MapView\WatchUnitFrame.h
# End Source File
# End Group
# Begin Group "Toolbars"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ToolbarsSrc\ToolbarButtonInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\ToolbarsSrc\ToolbarButtonInfo.h
# End Source File
# Begin Source File

SOURCE=.\ToolbarsSrc\ToolbarInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\ToolbarsSrc\ToolbarInfo.h
# End Source File
# Begin Source File

SOURCE=.\ToolbarsSrc\ToolbarsConfig.cpp
# End Source File
# Begin Source File

SOURCE=.\ToolbarsSrc\ToolbarsConfig.h
# End Source File
# Begin Source File

SOURCE=.\ToolbarsSrc\ToolbarsErrorOutput.cpp
# End Source File
# Begin Source File

SOURCE=.\ToolbarsSrc\ToolbarsErrorOutput.h
# End Source File
# Begin Source File

SOURCE=.\ToolbarsSrc\UnitToolbar.cpp
# End Source File
# Begin Source File

SOURCE=.\ToolbarsSrc\UnitToolbar.h
# End Source File
# End Group
# Begin Group "MiniMap"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ServerCommunication\CMiniMapClip.cpp
# End Source File
# Begin Source File

SOURCE=.\ServerCommunication\CMiniMapClip.h
# End Source File
# Begin Source File

SOURCE=.\MapView\MinimapFrame.cpp
# End Source File
# Begin Source File

SOURCE=.\MapView\MinimapFrame.h
# End Source File
# End Group
# Begin Group "UnitHierarchy"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\UnitHierarchy\UnitHierarchy.cpp
# End Source File
# Begin Source File

SOURCE=.\UnitHierarchy\UnitHierarchy.h
# End Source File
# Begin Source File

SOURCE=.\UnitHierarchy\UnitHierarchyChangeNameDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\UnitHierarchy\UnitHierarchyChangeNameDlg.h
# End Source File
# Begin Source File

SOURCE=.\UnitHierarchy\UnitHierarchyChangeScriptDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\UnitHierarchy\UnitHierarchyChangeScriptDlg.h
# End Source File
# Begin Source File

SOURCE=.\UnitHierarchy\UnitHierarchyFrame.cpp
# End Source File
# Begin Source File

SOURCE=.\UnitHierarchy\UnitHierarchyFrame.h
# End Source File
# Begin Source File

SOURCE=.\UnitHierarchy\UnitHierarchyUnit.cpp
# End Source File
# Begin Source File

SOURCE=.\UnitHierarchy\UnitHierarchyUnit.h
# End Source File
# End Group
# End Target
# End Project
# Section GameClient : {D30C1661-CDAF-11D0-8A3E-00C04FC9E26E}
# 	2:5:Class:CWebBrowser2
# 	2:10:HeaderFile:scripteditor\helpbrowser\webbrowser2.h
# 	2:8:ImplFile:scripteditor\helpbrowser\webbrowser2.cpp
# End Section
# Section GameClient : {8856F961-340A-11D0-A96B-00C04FD705A2}
# 	2:21:DefaultSinkHeaderFile:scripteditor\helpbrowser\webbrowser2.h
# 	2:16:DefaultSinkClass:CWebBrowser2
# End Section
