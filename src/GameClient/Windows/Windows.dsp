# Microsoft Developer Studio Project File - Name="Windows" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=Windows - Win32 Debug Static
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Windows.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Windows.mak" CFG="Windows - Win32 Debug Static"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Windows - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Windows - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "Windows - Win32 Debug Static" (based on "Win32 (x86) Static Library")
!MESSAGE "Windows - Win32 Release No Optimizations" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/GameClient/Windows", MSBAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Windows - Win32 Release"

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

!ELSEIF  "$(CFG)" == "Windows - Win32 Debug"

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

!ELSEIF  "$(CFG)" == "Windows - Win32 Debug Static"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Windows___Win32_Debug_Static"
# PROP BASE Intermediate_Dir "Windows___Win32_Debug_Static"
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

!ELSEIF  "$(CFG)" == "Windows - Win32 Release No Optimizations"

# PROP BASE Use_MFC 1
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Windows___Win32_Release_No_Optimizations"
# PROP BASE Intermediate_Dir "Windows___Win32_Release_No_Optimizations"
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

# Name "Windows - Win32 Release"
# Name "Windows - Win32 Debug"
# Name "Windows - Win32 Debug Static"
# Name "Windows - Win32 Release No Optimizations"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\Mouse\WaitingCursor.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\Animation.h
# End Source File
# Begin Source File

SOURCE=.\Controls.h
# End Source File
# Begin Source File

SOURCE=.\Fonts.h
# End Source File
# Begin Source File

SOURCE=.\Keyboard.h
# End Source File
# Begin Source File

SOURCE=.\Mouse.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\Timer.h
# End Source File
# Begin Source File

SOURCE=.\Mouse\WaitingCursor.h
# End Source File
# Begin Source File

SOURCE=.\Window.h
# End Source File
# Begin Source File

SOURCE=.\Windows.h
# End Source File
# End Group
# Begin Group "Animation"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Animation\Animation.cpp
# End Source File
# Begin Source File

SOURCE=.\Animation\Animation.h
# End Source File
# Begin Source File

SOURCE=.\Animation\AnimationCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\Animation\AnimationCtrl.h
# End Source File
# End Group
# Begin Group "Fonts"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Fonts\FontObject.cpp
# End Source File
# Begin Source File

SOURCE=.\Fonts\FontObject.h
# End Source File
# Begin Source File

SOURCE=.\Fonts\GraphicCacheFont.cpp
# End Source File
# Begin Source File

SOURCE=.\Fonts\GraphicCacheFont.h
# End Source File
# Begin Source File

SOURCE=.\Fonts\GraphicFont.cpp
# End Source File
# Begin Source File

SOURCE=.\Fonts\GraphicFont.h
# End Source File
# Begin Source File

SOURCE=.\Fonts\WindowsFont.cpp
# End Source File
# Begin Source File

SOURCE=.\Fonts\WindowsFont.h
# End Source File
# End Group
# Begin Group "Keyboard"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Keyboard\KeyAccelerators.cpp
# End Source File
# Begin Source File

SOURCE=.\Keyboard\KeyAccelerators.h
# End Source File
# Begin Source File

SOURCE=.\Keyboard\Keyboard.cpp
# End Source File
# Begin Source File

SOURCE=.\Keyboard\Keyboard.h
# End Source File
# Begin Source File

SOURCE=.\Keyboard\KeyboardNotifier.cpp
# End Source File
# Begin Source File

SOURCE=.\Keyboard\KeyboardNotifier.h
# End Source File
# Begin Source File

SOURCE=.\Keyboard\KeyboardWindow.cpp
# End Source File
# Begin Source File

SOURCE=.\Keyboard\KeyboardWindow.h
# End Source File
# End Group
# Begin Group "Mouse"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Mouse\Cursor.cpp
# End Source File
# Begin Source File

SOURCE=.\Mouse\Cursor.h
# End Source File
# Begin Source File

SOURCE=.\Mouse\Mouse.cpp
# End Source File
# Begin Source File

SOURCE=.\Mouse\Mouse.h
# End Source File
# Begin Source File

SOURCE=.\Mouse\MouseNotifier.cpp
# End Source File
# Begin Source File

SOURCE=.\Mouse\MouseNotifier.h
# End Source File
# Begin Source File

SOURCE=.\Mouse\MousePrimarySurface.cpp
# End Source File
# Begin Source File

SOURCE=.\Mouse\MousePrimarySurface.h
# End Source File
# Begin Source File

SOURCE=.\Mouse\MouseWindow.cpp
# End Source File
# Begin Source File

SOURCE=.\Mouse\MouseWindow.h
# End Source File
# End Group
# Begin Group "Timer"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Timer\Timer.cpp
# End Source File
# Begin Source File

SOURCE=.\Timer\Timer.h
# End Source File
# End Group
# Begin Group "Window"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Window\CaptionWindow.cpp
# End Source File
# Begin Source File

SOURCE=.\Window\CaptionWindow.h
# End Source File
# Begin Source File

SOURCE=.\Window\CaptionWindowLayout.cpp
# End Source File
# Begin Source File

SOURCE=.\Window\CaptionWindowLayout.h
# End Source File
# Begin Source File

SOURCE=.\Window\DialogWindow.cpp
# End Source File
# Begin Source File

SOURCE=.\Window\DialogWindow.h
# End Source File
# Begin Source File

SOURCE=.\Window\DialogWindowLayout.cpp
# End Source File
# Begin Source File

SOURCE=.\Window\DialogWindowLayout.h
# End Source File
# Begin Source File

SOURCE=.\Window\FrameWindow.cpp
# End Source File
# Begin Source File

SOURCE=.\Window\FrameWindow.h
# End Source File
# Begin Source File

SOURCE=.\Window\FrameWindowLayout.cpp
# End Source File
# Begin Source File

SOURCE=.\Window\FrameWindowLayout.h
# End Source File
# Begin Source File

SOURCE=.\Window\MessageBox.cpp
# End Source File
# Begin Source File

SOURCE=.\Window\MessageBox.h
# End Source File
# Begin Source File

SOURCE=.\Window\UpdateRectList.cpp
# End Source File
# Begin Source File

SOURCE=.\Window\UpdateRectList.h
# End Source File
# Begin Source File

SOURCE=.\Window\Window.cpp
# End Source File
# Begin Source File

SOURCE=.\Window\Window.h
# End Source File
# Begin Source File

SOURCE=.\Window\WindowsWindow.cpp
# End Source File
# Begin Source File

SOURCE=.\Window\windowswindow.h
# End Source File
# End Group
# Begin Group "Controls"

# PROP Default_Filter ""
# Begin Group "Buttons"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Controls\Buttons\ButtonObject.cpp
# End Source File
# Begin Source File

SOURCE=.\Controls\Buttons\ButtonObject.h
# End Source File
# Begin Source File

SOURCE=.\Controls\Buttons\IconButton.cpp
# End Source File
# Begin Source File

SOURCE=.\Controls\Buttons\IconButton.h
# End Source File
# Begin Source File

SOURCE=.\Controls\Buttons\IconButtonLayout.cpp
# End Source File
# Begin Source File

SOURCE=.\Controls\Buttons\IconButtonLayout.h
# End Source File
# Begin Source File

SOURCE=.\Controls\Buttons\ImageButton.cpp
# End Source File
# Begin Source File

SOURCE=.\Controls\Buttons\ImageButton.h
# End Source File
# Begin Source File

SOURCE=.\Controls\Buttons\ImageButtonLayout.cpp
# End Source File
# Begin Source File

SOURCE=.\Controls\Buttons\ImageButtonLayout.h
# End Source File
# Begin Source File

SOURCE=.\Controls\Buttons\TextButton.cpp
# End Source File
# Begin Source File

SOURCE=.\Controls\Buttons\TextButton.h
# End Source File
# Begin Source File

SOURCE=.\Controls\Buttons\TextButtonLayout.cpp
# End Source File
# Begin Source File

SOURCE=.\Controls\Buttons\TextButtonLayout.h
# End Source File
# End Group
# Begin Group "EditBox"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Controls\EditBox\EditBox.cpp
# End Source File
# Begin Source File

SOURCE=.\Controls\EditBox\EditBox.h
# End Source File
# Begin Source File

SOURCE=.\Controls\EditBox\EditBoxLayout.cpp
# End Source File
# Begin Source File

SOURCE=.\Controls\EditBox\EditBoxLayout.h
# End Source File
# End Group
# Begin Group "Static"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Controls\Static\StaticText.cpp
# End Source File
# Begin Source File

SOURCE=.\Controls\Static\StaticText.h
# End Source File
# Begin Source File

SOURCE=.\Controls\Static\StaticTextLayout.cpp
# End Source File
# Begin Source File

SOURCE=.\Controls\Static\StaticTextLayout.h
# End Source File
# End Group
# Begin Group "ListControl"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Controls\ListControl\ListControl.cpp
# End Source File
# Begin Source File

SOURCE=.\Controls\ListControl\ListControl.h
# End Source File
# Begin Source File

SOURCE=.\Controls\ListControl\ListControlLayout.cpp
# End Source File
# Begin Source File

SOURCE=.\Controls\ListControl\ListControlLayout.h
# End Source File
# Begin Source File

SOURCE=.\Controls\ListControl\ListControlScroll.cpp
# End Source File
# Begin Source File

SOURCE=.\Controls\ListControl\ListControlScroll.h
# End Source File
# Begin Source File

SOURCE=.\Controls\ListControl\TreeControl.cpp
# End Source File
# Begin Source File

SOURCE=.\Controls\ListControl\TreeControl.h
# End Source File
# Begin Source File

SOURCE=.\Controls\ListControl\TreeControlLayout.cpp
# End Source File
# Begin Source File

SOURCE=.\Controls\ListControl\TreeControlLayout.h
# End Source File
# End Group
# Begin Group "ScrollControl"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Controls\ScrollControl\ScrollControl.cpp
# End Source File
# Begin Source File

SOURCE=.\Controls\ScrollControl\ScrollControl.h
# End Source File
# Begin Source File

SOURCE=.\Controls\ScrollControl\ScrollControlLayout.cpp
# End Source File
# Begin Source File

SOURCE=.\Controls\ScrollControl\ScrollControlLayout.h
# End Source File
# End Group
# Begin Group "ScriptEdit"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Controls\ScriptEdit\ScEdLexInput.cpp
# End Source File
# Begin Source File

SOURCE=.\Controls\ScriptEdit\ScEdLexInput.h
# End Source File
# Begin Source File

SOURCE=.\Controls\ScriptEdit\ScriptEdit.cpp
# End Source File
# Begin Source File

SOURCE=.\Controls\ScriptEdit\ScriptEdit.h
# End Source File
# Begin Source File

SOURCE=.\Controls\ScriptEdit\ScriptEditLayout.cpp
# End Source File
# Begin Source File

SOURCE=.\Controls\ScriptEdit\ScriptEditLayout.h
# End Source File
# Begin Source File

SOURCE=.\Controls\ScriptEdit\ScriptEditScroll.cpp
# End Source File
# Begin Source File

SOURCE=.\Controls\ScriptEdit\ScriptEditScroll.h
# End Source File
# End Group
# Begin Group "Layouts"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Controls\Layouts\Layouts.cpp
# End Source File
# Begin Source File

SOURCE=.\Controls\Layouts\Layouts.h
# End Source File
# End Group
# Begin Group "Menu"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Controls\Menu\PopupMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\Controls\Menu\PopupMenu.h
# End Source File
# End Group
# Begin Group "ToolTip"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Controls\ToolTip\ToolTip.cpp
# End Source File
# Begin Source File

SOURCE=.\Controls\ToolTip\ToolTip.h
# End Source File
# Begin Source File

SOURCE=.\Controls\ToolTip\ToolTipLayout.cpp
# End Source File
# Begin Source File

SOURCE=.\Controls\ToolTip\ToolTipLayout.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\Controls\Buttons.h
# End Source File
# Begin Source File

SOURCE=.\Controls\EditBox.h
# End Source File
# Begin Source File

SOURCE=.\Controls\Layouts.h
# End Source File
# Begin Source File

SOURCE=.\Controls\ListControl.h
# End Source File
# Begin Source File

SOURCE=.\Controls\Menu.h
# End Source File
# Begin Source File

SOURCE=.\Controls\ScriptEdit.h
# End Source File
# Begin Source File

SOURCE=.\Controls\ScrollControl.h
# End Source File
# Begin Source File

SOURCE=.\Controls\Static.h
# End Source File
# Begin Source File

SOURCE=.\Controls\ToolTip.h
# End Source File
# End Group
# End Target
# End Project
