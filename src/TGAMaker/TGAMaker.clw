; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CTGAMakerDlg
LastTemplate=CFrameWnd
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "TGAMaker.h"

ClassCount=3
Class1=CTGAMakerApp
Class2=CTGAMakerDlg

ResourceCount=2
Resource1=IDR_MAINFRAME
Class3=CPreviewWindow
Resource2=IDD_TGAMAKER_DIALOG

[CLS:CTGAMakerApp]
Type=0
HeaderFile=TGAMaker.h
ImplementationFile=TGAMaker.cpp
Filter=N

[CLS:CTGAMakerDlg]
Type=0
HeaderFile=TGAMakerDlg.h
ImplementationFile=TGAMakerDlg.cpp
Filter=D
LastObject=CTGAMakerDlg
BaseClass=CDialog
VirtualFilter=dWC



[DLG:IDD_TGAMAKER_DIALOG]
Type=1
Class=?
ControlCount=17
Control1=IDC_BITMAP,edit,1350631552
Control2=IDC_ALPHA,edit,1350631552
Control3=IDC_RESULT,edit,1350631552
Control4=IDC_AUTOMATIC_TRANSPARENT_NAME,button,1342242819
Control5=IDC_AUTOMATIC_RESULT_NAME,button,1342242819
Control6=IDC_ALWAYSONTOP,button,1342242819
Control7=IDC_CONVERT,button,1342242817
Control8=IDOK,button,1342242816
Control9=IDC_BROWSEBITMAP,button,1342242816
Control10=IDC_BROWSEALPHA,button,1342242816
Control11=IDC_BROWSERESULT,button,1342242816
Control12=IDC_BKGCOLOR,static,1342181636
Control13=IDC_STATIC,static,1342308864
Control14=IDC_STATIC,static,1342308864
Control15=IDC_STATIC,static,1342308864
Control16=IDC_STATIC,static,1342308352
Control17=IDC_ALPHAINVERSE,button,1342242819

[CLS:CPreviewWindow]
Type=0
HeaderFile=PreviewWindow.h
ImplementationFile=PreviewWindow.cpp
BaseClass=CFrameWnd
Filter=T
VirtualFilter=fWC
LastObject=CPreviewWindow

