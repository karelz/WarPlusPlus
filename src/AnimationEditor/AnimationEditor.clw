; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CViewOptionsDlg
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "AnimationEditor.h"
LastPage=0

ClassCount=7
Class1=CAnimationEditorApp
Class2=CAnimationEditorDoc
Class3=CAnimationEditorView
Class4=CMainFrame

ResourceCount=9
Resource1=IDD_ABOUTBOX
Resource2=IDR_MAINFRAME
Class5=CAboutDlg
Resource3=IDR_FRAME (English (U.S.))
Resource4=IDR_ANIMATION (English (U.S.))
Resource5=IDD_VIEWOPTIONS
Class6=CViewOptionsDlg
Resource6=IDD_ABOUTBOX (English (U.S.))
Resource7=IDD_CROPFRAME
Resource8=IDR_COLORING (English (U.S.))
Class7=CCropFrameDlg
Resource9=IDR_MAINFRAME (English (U.S.))

[CLS:CAnimationEditorApp]
Type=0
HeaderFile=AnimationEditor.h
ImplementationFile=AnimationEditor.cpp
Filter=N
BaseClass=CWinApp
VirtualFilter=AC
LastObject=CAnimationEditorApp

[CLS:CAnimationEditorDoc]
Type=0
HeaderFile=AnimationEditorDoc.h
ImplementationFile=AnimationEditorDoc.cpp
Filter=N
BaseClass=CDocument
VirtualFilter=DC
LastObject=CAnimationEditorDoc

[CLS:CMainFrame]
Type=0
HeaderFile=MainFrm.h
ImplementationFile=MainFrm.cpp
Filter=T
BaseClass=CFrameWnd
VirtualFilter=fWC
LastObject=CMainFrame




[CLS:CAboutDlg]
Type=0
HeaderFile=AnimationEditor.cpp
ImplementationFile=AnimationEditor.cpp
Filter=D
LastObject=CAboutDlg

[DLG:IDD_ABOUTBOX]
Type=1
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308352
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889
Class=CAboutDlg

[MNU:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command3=ID_FILE_SAVE
Command4=ID_FILE_SAVE_AS
Command5=ID_FILE_MRU_FILE1
Command6=ID_APP_EXIT
Command10=ID_EDIT_PASTE
Command11=ID_VIEW_TOOLBAR
Command12=ID_VIEW_STATUS_BAR
Command13=ID_APP_ABOUT
CommandCount=13
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command7=ID_EDIT_UNDO
Command8=ID_EDIT_CUT
Command9=ID_EDIT_COPY

[ACL:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command5=ID_EDIT_CUT
Command6=ID_EDIT_COPY
Command7=ID_EDIT_PASTE
Command8=ID_EDIT_UNDO
Command9=ID_EDIT_CUT
Command10=ID_EDIT_COPY
Command11=ID_EDIT_PASTE
Command12=ID_NEXT_PANE
CommandCount=13
Command4=ID_EDIT_UNDO
Command13=ID_PREV_PANE


[TB:IDR_MAINFRAME (English (U.S.))]
Type=1
Class=?
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_EDIT_CUT
Command5=ID_EDIT_COPY
Command6=ID_EDIT_PASTE
CommandCount=6

[MNU:IDR_MAINFRAME (English (U.S.))]
Type=1
Class=CMainFrame
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_FILE_SAVE_AS
Command5=ID_FILE_MRU_FILE1
Command6=ID_APP_EXIT
Command7=ID_EDIT_UNDO
Command8=ID_EDIT_CUT
Command9=ID_EDIT_COPY
Command10=ID_EDIT_PASTE
Command11=ID_VIEW_TOOLBAR
Command12=ID_VIEW_STATUS_BAR
Command13=ID_VIEW_ANIMATIONTOOLBAR
Command14=ID_VIEW_COLORINGTOOLBAR
Command15=ID_VIEW_OPTIONS
Command16=ID_APP_ABOUT
CommandCount=16

[ACL:IDR_MAINFRAME (English (U.S.))]
Type=1
Class=?
Command1=ID_EDIT_COPY
Command2=ID_FILE_NEW
Command3=ID_FILE_OPEN
Command4=ID_FILE_SAVE
Command5=ID_EDIT_PASTE
Command6=ID_EDIT_UNDO
Command7=ID_EDIT_CUT
Command8=ID_NEXT_PANE
Command9=ID_PREV_PANE
Command10=ID_EDIT_COPY
Command11=ID_EDIT_PASTE
Command12=ID_ANIMATION_PREV
Command13=ID_ANIMATION_FIRST
Command14=IDOK
Command15=ID_ANIMATION_NEXT
Command16=ID_ANIMATION_LAST
Command17=ID_EDIT_CUT
Command18=ID_EDIT_UNDO
CommandCount=18

[DLG:IDD_ABOUTBOX (English (U.S.))]
Type=1
Class=CAboutDlg
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889

[TB:IDR_ANIMATION (English (U.S.))]
Type=1
Class=?
Command1=ID_ANIMATION_PLAY
Command2=ID_ANIMATION_STOP
Command3=ID_ANIMATION_FIRST
Command4=ID_ANIMATION_PREV
Command5=ID_ANIMATION_FRAME
Command6=ID_ANIMATION_NEXT
Command7=ID_ANIMATION_LAST
Command8=ID_ANIMATION_DELAY
Command9=ID_ANIMATION_LOOP
Command10=ID_ANIMATION_EDITOFFSET
CommandCount=10

[CLS:CAnimationEditorView]
Type=0
HeaderFile=animationeditorview.h
ImplementationFile=animationeditorview.cpp
BaseClass=CScrollView
LastObject=ID_FRAME_CROP
Filter=C
VirtualFilter=VWC

[DLG:IDD_VIEWOPTIONS]
Type=1
Class=CViewOptionsDlg
ControlCount=15
Control1=IDC_BACKGROUNDCOLOR,static,1342181380
Control2=IDC_STATIC,static,1342177286
Control3=IDC_BACKGROUNDANIMATION,edit,1342242944
Control4=IDC_BROWSEBKGANIMATION,button,1342242816
Control5=IDC_ANIMATIONCOLOR,static,1342181380
Control6=IDC_CROSSHAIRCOLOR,static,1342181380
Control7=IDC_SHOWCROSSHAIR,button,1342242819
Control8=IDOK,button,1342242817
Control9=IDCANCEL,button,1342242816
Control10=IDC_STATIC,static,1342308352
Control11=IDC_STATIC,static,1342308352
Control12=IDC_STATIC,static,1342308352
Control13=IDC_STATIC,static,1342308352
Control14=IDC_DISPLAYBACKGROUNDASOVERLAY,button,1342242819
Control15=IDC_SHOWBOUNDINGRECTANGLE,button,1342242819

[CLS:CViewOptionsDlg]
Type=0
HeaderFile=ViewOptionsDlg.h
ImplementationFile=ViewOptionsDlg.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=CViewOptionsDlg

[TB:IDR_COLORING (English (U.S.))]
Type=1
Class=?
Command1=ID_COLORING_HUECAPTION
Command2=ID_COLORING_HUELESS
Command3=ID_COLORING_HUEVALUE
Command4=ID_COLORING_HUEMORE
Command5=ID_COLORING_SATCAPTION
Command6=ID_COLORING_SATLESS
Command7=ID_COLORING_SATVALUE
Command8=ID_COLORING_SATMORE
Command9=ID_COLORING_VALCAPTION
Command10=ID_COLORING_VALLESS
Command11=ID_COLORING_VALVALUE
Command12=ID_COLORING_VALMORE
Command13=ID_COLORING_SOURCECOLOR
CommandCount=13

[TB:IDR_FRAME (English (U.S.))]
Type=1
Class=?
Command1=ID_FRAME_MOVELEFT
Command2=ID_FRAME_MOVERIGHT
Command3=ID_FRAME_INSERT
Command4=ID_FRAME_DELETE
Command5=ID_FRAME_DELAY
Command6=ID_FRAME_EDITOFFSET
Command7=ID_FRAME_CROP
CommandCount=7

[DLG:IDD_CROPFRAME]
Type=1
Class=CCropFrameDlg
ControlCount=5
Control1=IDC_TOLERANCE,edit,1350631552
Control2=IDOK,button,1342242817
Control3=IDCANCEL,button,1342242816
Control4=IDC_STATIC,static,1342308352
Control5=IDC_STATIC,static,1342308352

[CLS:CCropFrameDlg]
Type=0
HeaderFile=CropFrameDlg.h
ImplementationFile=CropFrameDlg.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC

