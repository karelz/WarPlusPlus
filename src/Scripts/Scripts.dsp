# Microsoft Developer Studio Project File - Name="Scripts" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Generic Project" 0x010a

CFG=Scripts - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Scripts.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Scripts.mak" CFG="Scripts - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Scripts - Win32 Release" (based on "Win32 (x86) Generic Project")
!MESSAGE "Scripts - Win32 Debug" (based on "Win32 (x86) Generic Project")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Scripts", JFEAAAAA"
# PROP Scc_LocalPath "."
MTL=midl.exe

!IF  "$(CFG)" == "Scripts - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""

!ELSEIF  "$(CFG)" == "Scripts - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=Updatuju mapu
PostBuild_Cmds=cd D:\Skola\Projekt\Data\Maps\AutoTestMap	update_skript_sets.bat
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "Scripts - Win32 Release"
# Name "Scripts - Win32 Debug"
# Begin Group "01"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\01\Jednotka.3s
# End Source File
# Begin Source File

SOURCE=.\01\PanelNastroju.3s
# End Source File
# Begin Source File

SOURCE=.\01\Pruzkumnik.3s
# End Source File
# Begin Source File

SOURCE=.\01\StartupCommander.3s
# End Source File
# Begin Source File

SOURCE=.\01\Velitel.3s
# End Source File
# Begin Source File

SOURCE=.\01\VelitelPruzkumniku.3s
# End Source File
# End Group
# Begin Group "SimpleScriptSet"

# PROP Default_Filter ""
# Begin Group "source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\SimpleScriptSet\source\Jednotka.3s
# End Source File
# Begin Source File

SOURCE=.\SimpleScriptSet\source\PanelNastroju.3s
# End Source File
# Begin Source File

SOURCE=.\SimpleScriptSet\source\Pruzkumnik.3s
# End Source File
# Begin Source File

SOURCE=.\SimpleScriptSet\source\StartupCommander.3s
# End Source File
# Begin Source File

SOURCE=.\SimpleScriptSet\source\Velitel.3s
# End Source File
# Begin Source File

SOURCE=.\SimpleScriptSet\source\VelitelPruzkumniku.3s
# End Source File
# End Group
# Begin Source File

SOURCE=.\SimpleScriptSet\Simple.ScriptSet
# End Source File
# End Group
# Begin Group "AutoTest"

# PROP Default_Filter ""
# Begin Group "Toolbars"

# PROP Default_Filter "*.tga, *.txt"
# Begin Source File

SOURCE=.\AutoTest\Toolbars\ArrowDown.tga
# End Source File
# Begin Source File

SOURCE=.\AutoTest\Toolbars\ArrowEast.tga
# End Source File
# Begin Source File

SOURCE=.\AutoTest\Toolbars\Base.tga
# End Source File
# Begin Source File

SOURCE=.\AutoTest\Toolbars\Bednarek.tga
# End Source File
# Begin Source File

SOURCE=.\AutoTest\Toolbars\Bolt.tga
# End Source File
# Begin Source File

SOURCE=.\AutoTest\Toolbars\boot.tga
# End Source File
# Begin Source File

SOURCE=.\AutoTest\Toolbars\builder.tga
# End Source File
# Begin Source File

SOURCE=.\AutoTest\Toolbars\Cross.tga
# End Source File
# Begin Source File

SOURCE=.\AutoTest\Toolbars\Crystal.tga
# End Source File
# Begin Source File

SOURCE=.\AutoTest\Toolbars\DefaultIcon.tga
# End Source File
# Begin Source File

SOURCE=.\AutoTest\Toolbars\DoubleArrow.tga
# End Source File
# Begin Source File

SOURCE=.\AutoTest\Toolbars\Empty.tga
# End Source File
# Begin Source File

SOURCE=.\AutoTest\Toolbars\explorer.tga
# End Source File
# Begin Source File

SOURCE=.\AutoTest\Toolbars\Eye.tga
# End Source File
# Begin Source File

SOURCE=.\AutoTest\Toolbars\Factory.tga
# End Source File
# Begin Source File

SOURCE=.\AutoTest\Toolbars\hold.tga
# End Source File
# Begin Source File

SOURCE=.\AutoTest\Toolbars\Jumper.tga
# End Source File
# Begin Source File

SOURCE=.\AutoTest\Toolbars\Question.tga
# End Source File
# Begin Source File

SOURCE=.\AutoTest\Toolbars\random.tga
# End Source File
# Begin Source File

SOURCE=.\AutoTest\Toolbars\Relay.tga
# End Source File
# Begin Source File

SOURCE=.\AutoTest\Toolbars\RoundArrow.tga
# End Source File
# Begin Source File

SOURCE=.\AutoTest\Toolbars\skull.tga
# End Source File
# Begin Source File

SOURCE=.\AutoTest\Toolbars\Toolbars.txt
# End Source File
# Begin Source File

SOURCE=.\AutoTest\Toolbars\Tools.tga
# End Source File
# Begin Source File

SOURCE=.\AutoTest\Toolbars\TwoArrows.tga
# End Source File
# Begin Source File

SOURCE=.\AutoTest\Toolbars\Yaghob.tga
# End Source File
# Begin Source File

SOURCE=.\AutoTest\Toolbars\Zavoral.tga
# End Source File
# End Group
# Begin Source File

SOURCE=.\AutoTest\AbstractCommander.3s
# End Source File
# Begin Source File

SOURCE=.\AutoTest\AbstractUnit.3s
# End Source File
# Begin Source File

SOURCE=.\AutoTest\AttackCommander.3s
# End Source File
# Begin Source File

SOURCE=.\AutoTest\BaseUnit.3s
# End Source File
# Begin Source File

SOURCE=.\AutoTest\BuildCommander.3s
# End Source File
# Begin Source File

SOURCE=.\AutoTest\BuildingUnit.3s
# End Source File
# Begin Source File

SOURCE=.\AutoTest\ExplorationCommander.3s
# End Source File
# Begin Source File

SOURCE=.\AutoTest\ExploringUnit.3s
# End Source File
# Begin Source File

SOURCE=.\AutoTest\FightingUnit.3s
# End Source File
# Begin Source File

SOURCE=.\AutoTest\MiningCommander.3s
# End Source File
# Begin Source File

SOURCE=.\AutoTest\MiningUnit.3s
# End Source File
# Begin Source File

SOURCE=.\AutoTest\PanelNastroju.3s
# End Source File
# Begin Source File

SOURCE=.\AutoTest\StartupCommander.3s
# End Source File
# Begin Source File

SOURCE=.\AutoTest\UnitQueue.3s
# End Source File
# Begin Source File

SOURCE=.\AutoTest\UserCommander.3s
# End Source File
# End Group
# Begin Group "Karby-User"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Karby\User\Custom.3s
# End Source File
# Begin Source File

SOURCE=.\Karby\User\Explorer.3s
# End Source File
# Begin Source File

SOURCE=.\Karby\User\SimpleUnit.3s
# End Source File
# Begin Source File

SOURCE=.\Karby\User\StartupCommander.3s
# End Source File
# Begin Source File

SOURCE=.\Karby\User\Toolbars.3s
# End Source File
# Begin Source File

SOURCE=.\Karby\User\Toolbars.txt
# End Source File
# End Group
# End Target
# End Project
