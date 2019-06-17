// ScriptEditor.cpp: implementation of the CScriptEditor class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\GameClient.h"
#include "ScriptEditor.h"

#include "OpenRemoteScriptDlg.h"
#include "SaveRemoteScriptDlg.h"
#include "OpenLocalScriptDlg.h"

#include "..\Common\CommonExceptions.h"

#include "Common\ServerClient\VirtualConnectionIDs.h"
#include "Common\ServerClient\ScriptEditor.h"
#include "..\GameClientNetwork.h"

#include "..\UnitHierarchy\UnitHierarchy.h"

#include "..\GameClientHelperFunctions.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define SCRIPTEDITOR_TOOLBAR_HEIGHT 40
#define SCRIPTEDITOR_ERROROUTPUT_HEIGHT 120

#define SCRIPTEDITOR_PROFILINGUPDATE_PERIOD 5000

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CScriptEditor, CWindow);

BEGIN_OBSERVER_MAP(CScriptEditor, CWindow)

  BEGIN_NOTIFIER(IDC_FILE)
    ON_BUTTONCOMMAND(OnFile)
  END_NOTIFIER()

  BEGIN_NOTIFIER(IDC_NEWSCRIPT)
    ON_BUTTONCOMMAND(OnNewScript)
  END_NOTIFIER()

  BEGIN_NOTIFIER(IDC_OPENREMOTESCRIPT)
    ON_BUTTONCOMMAND(OnOpenRemoteScript)
  END_NOTIFIER()
  BEGIN_NOTIFIER(IDC_SAVEREMOTESCRIPT)
    ON_BUTTONCOMMAND(OnSaveRemoteScript)
  END_NOTIFIER()
  BEGIN_NOTIFIER(IDC_SAVEASREMOTESCRIPT)
    ON_BUTTONCOMMAND(OnSaveAsRemoteScript)
  END_NOTIFIER()

  BEGIN_NOTIFIER(IDC_OPENLOCALSCRIPT)
    ON_BUTTONCOMMAND(OnOpenLocalScript)
  END_NOTIFIER()
  BEGIN_NOTIFIER(IDC_SAVELOCALSCRIPT)
    ON_BUTTONCOMMAND(OnSaveLocalScript)
  END_NOTIFIER()

  BEGIN_NOTIFIER(IDC_EXIT)
    ON_BUTTONCOMMAND(OnExit)
  END_NOTIFIER()



  BEGIN_NOTIFIER(IDC_COMPILEMENU)
    ON_BUTTONCOMMAND(OnCompileMenu)
  END_NOTIFIER()

  BEGIN_NOTIFIER(IDC_ERROROUTPUT_TOGGLE)
    ON_BUTTONCOMMAND(OnErrorOutputToggle)
  END_NOTIFIER()
  BEGIN_NOTIFIER(IDC_COMPILE)
    ON_BUTTONCOMMAND(OnCompile)
  END_NOTIFIER()
  BEGIN_NOTIFIER(IDC_COMPILEANDUPDATE)
    ON_BUTTONCOMMAND(OnCompileAndUpdate)
  END_NOTIFIER()
  BEGIN_NOTIFIER(IDC_MULTICOMPILE)
    ON_BUTTONCOMMAND(OnMultiCompile)
  END_NOTIFIER()
  BEGIN_NOTIFIER ( IDC_PROFILING )
    ON_BUTTONCOMMAND ( OnProfiling )
  END_NOTIFIER ()

  BEGIN_NOTIFIER(IDC_LOGWINDOW_TOGGLE)
    ON_BUTTONCOMMAND(OnLogWindowToggle)
  END_NOTIFIER()

  BEGIN_NOTIFIER(IDC_MAPVIEW)
    ON_BUTTONCOMMAND(OnMapView)
  END_NOTIFIER()
  BEGIN_NOTIFIER(IDC_HELPIT)
    ON_BUTTONCOMMAND(OnHelp)
  END_NOTIFIER()
  BEGIN_NOTIFIER(IDC_UNITHIERARCHY)
    ON_BUTTONCOMMAND(OnUnitHierarchy)
  END_NOTIFIER()


  BEGIN_NOTIFIER(IDC_EDIT)
    ON_BUTTONCOMMAND(OnEdit)
  END_NOTIFIER()

  BEGIN_NOTIFIER(IDC_CUT)
    ON_BUTTONCOMMAND(OnCut)
  END_NOTIFIER()
  BEGIN_NOTIFIER(IDC_COPY)
    ON_BUTTONCOMMAND(OnCopy)
  END_NOTIFIER()
  BEGIN_NOTIFIER(IDC_PASTE)
    ON_BUTTONCOMMAND(OnPaste)
  END_NOTIFIER()

  BEGIN_NOTIFIER(IDC_DELETE)
    ON_BUTTONCOMMAND(OnDelete)
  END_NOTIFIER()


  BEGIN_NOTIFIER(ID_VirtualConnection)
    EVENT(E_PACKETAVAILABLE)
      OnPacketAvailable(); return FALSE;
    EVENT(E_NETWORKERROR)
      OnNetworkError(); return FALSE;
  END_NOTIFIER()

  BEGIN_NOTIFIER(ID_Myself)
    EVENT(E_STARTWAITINGFORSERVER)
      OnStartWaitingForServer(); return FALSE;
  END_NOTIFIER()

  BEGIN_NOTIFIER(ID_LOGWINDOW)
    EVENT(CLogWindow::E_HIDDEN)
      OnLogWindowHidden();
  END_NOTIFIER()

  BEGIN_NOTIFIER ( ID_ProfilingWindow )
    EVENT ( CProfilingFrame::E_StopUpdating )
      OnStopProfilingUpdate (); return FALSE;
  END_NOTIFIER ()

  BEGIN_NOTIFIER(ID_ErrorOutput)
    EVENT(CErrorOutput::E_SelectErrorLine)
      OnSelectErrorLine();
  END_NOTIFIER()

  BEGIN_KEYBOARD()
    ON_KEYACCELERATOR()
  END_KEYBOARD()

  BEGIN_TIMER ()
    ON_TIMETICK()
  END_TIMER ()

END_OBSERVER_MAP(CScriptEditor, CWindow)

CScriptEditor::CScriptEditor()
{
  m_dwNetworkCommand = NC_None;
}

CScriptEditor::~CScriptEditor()
{
}

#ifdef _DEBUG

void CScriptEditor::AssertValid() const
{
  CWindow::AssertValid();
}

void CScriptEditor::Dump(CDumpContext &dc) const
{
  CWindow::Dump(dc);
}

#endif


BOOL CScriptEditor::Create(CDataArchive GraphicsArchive, CString strBackupDirectory, CString strShadowDirectory,
                           CWindow *pParent, DWORD dwEventThreadID, CUnitHierarchy *pUnitHierarchy)
{
  CString strCaption;
  CRect rcControl;

  m_bTransparent = TRUE;

  m_strBackupDirectory = strBackupDirectory;
  m_strLocalShadowDirectory = strShadowDirectory + "\\Local";
  // create the backup directory
  if(!CreateDirectory(m_strLocalShadowDirectory, NULL)){
    if(GetLastError() != ERROR_ALREADY_EXISTS)
      throw new CStringException("Can't create directory.\n");
  }
  m_strGlobalShadowDirectory = strShadowDirectory + "\\Global";
  // create the backup directory
  if(!CreateDirectory(m_strGlobalShadowDirectory, NULL)){
    if(GetLastError() != ERROR_ALREADY_EXISTS)
      throw new CStringException("Can't create directory.\n");
  }

  // create the window
  if(!CWindow::Create(g_pDDPrimarySurface->GetScreenRect(), pParent)) return FALSE;

  m_IconButtonLayout.Create(GraphicsArchive.CreateFile("ScriptEditor\\ToolBarButton.button"));

  m_NewScriptIcon.Create(GraphicsArchive.CreateFile("ScriptEditor\\Icons\\New.tga"));
  m_OpenRemoteScriptIcon.Create(GraphicsArchive.CreateFile("ScriptEditor\\Icons\\OpenRemoteScript.tga"));
  m_OpenLocalScriptIcon.Create(GraphicsArchive.CreateFile("ScriptEditor\\Icons\\OpenLocalScript.tga"));
  m_SaveRemoteScriptIcon.Create(GraphicsArchive.CreateFile("ScriptEditor\\Icons\\SaveRemoteScript.tga"));
  m_SaveLocalScriptIcon.Create(GraphicsArchive.CreateFile("ScriptEditor\\Icons\\SaveLocalScript.tga"));
  m_ShowErrorOutputIcon.Create(GraphicsArchive.CreateFile("ScriptEditor\\Icons\\ShowErrorOutput.tga"));
  m_HideErrorOutputIcon.Create(GraphicsArchive.CreateFile("ScriptEditor\\Icons\\HideErrorOutput.tga"));
  m_ShowLogWindowIcon.Create(GraphicsArchive.CreateFile("ScriptEditor\\Icons\\ShowLogWindow.tga"));
  m_HideLogWindowIcon.Create(GraphicsArchive.CreateFile("ScriptEditor\\Icons\\HideLogWindow.tga"));
  m_CompileIcon.Create(GraphicsArchive.CreateFile("ScriptEditor\\Icons\\Compile.tga"));
  m_CompileAndUpdateIcon.Create(GraphicsArchive.CreateFile("ScriptEditor\\Icons\\CompileAndUpdate.tga"));
  m_MultiCompileIcon.Create(GraphicsArchive.CreateFile("ScriptEditor\\Icons\\MultiCompile.tga"));
  m_MapViewIcon.Create(GraphicsArchive.CreateFile("ScriptEditor\\Icons\\MapView.tga"));
  m_HelpIcon.Create(GraphicsArchive.CreateFile("ScriptEditor\\Icons\\Help.tga"));
  m_UnitHierarchyIcon.Create(GraphicsArchive.CreateFile("ScriptEditor\\Icons\\UnitHierarchy.tga"));

  m_NewScriptButton.Create(CPoint(0, 0), &m_NewScriptIcon, &m_IconButtonLayout, this);
  m_NewScriptButton.Connect(this, IDC_NEWSCRIPT);
  m_NewScriptButton.SetToolTip(IDS_SCRIPTEDITOR_NEWSCRIPT_TT);
  m_NewScriptButton.SetKeyAcc('N', CKeyboard::CtrlDown);

  m_OpenRemoteScriptButton.Create(CPoint(42, 0), &m_OpenRemoteScriptIcon, &m_IconButtonLayout, this);
  m_OpenRemoteScriptButton.Connect(this, IDC_OPENREMOTESCRIPT);
  m_OpenRemoteScriptButton.SetToolTip(IDS_SCRIPTEDITOR_OPENREMOTESCRIPT_TT);
  m_OpenRemoteScriptButton.SetKeyAcc('O', CKeyboard::CtrlDown);
  m_SaveRemoteScriptButton.Create(CPoint(80, 0), &m_SaveRemoteScriptIcon, &m_IconButtonLayout, this);
  m_SaveRemoteScriptButton.Connect(this, IDC_SAVEREMOTESCRIPT);
  m_SaveRemoteScriptButton.SetToolTip(IDS_SCRIPTEDITOR_SAVEREMOTESCRIPT_TT);
  m_SaveRemoteScriptButton.SetKeyAcc('S', CKeyboard::CtrlDown);

  m_OpenLocalScriptButton.Create(CPoint(122, 0), &m_OpenLocalScriptIcon, &m_IconButtonLayout, this);
  m_OpenLocalScriptButton.Connect(this, IDC_OPENLOCALSCRIPT);
  m_OpenLocalScriptButton.SetToolTip(IDS_SCRIPTEDITOR_OPENLOCALSCRIPT_TT);
/*  m_SaveLocalScriptButton.Create(CPoint(160, 0), &m_SaveLocalScriptIcon, &m_IconButtonLayout, this);
  m_SaveLocalScriptButton.Connect(this, IDC_SAVELOCALSCRIPT);
  m_SaveLocalScriptButton.SetToolTip(IDS_SCRIPTEDITOR_SAVELOCALSCRIPT_TT);*/

  m_ToggleErrorOutput.Create(CPoint(202, 0), &m_HideErrorOutputIcon, &m_IconButtonLayout, this);
  m_ToggleErrorOutput.Connect(this, IDC_ERROROUTPUT_TOGGLE);
  m_ToggleErrorOutput.SetToolTip(IDS_SCRIPTEDITOR_ERROROUTPUT_HIDE_TT);
  m_Compile.Create(CPoint(240, 0), &m_CompileIcon, &m_IconButtonLayout, this);
  m_Compile.Connect(this, IDC_COMPILE);
  m_Compile.SetToolTip(IDS_SCRIPTEDITOR_COMPILE_TT);
  m_Compile.SetKeyAcc(VK_F7, 0);
  m_CompileAndUpdate.Create(CPoint(278, 0), &m_CompileAndUpdateIcon, &m_IconButtonLayout, this);
  m_CompileAndUpdate.Connect(this, IDC_COMPILEANDUPDATE);
  m_CompileAndUpdate.SetToolTip(IDS_SCRIPTEDITOR_COMPILEANDUPDATE_TT);
  m_CompileAndUpdate.SetKeyAcc(VK_F5, 0);
  m_MultiCompile.Create(CPoint(316, 0), &m_MultiCompileIcon, &m_IconButtonLayout, this);
  m_MultiCompile.Connect(this, IDC_MULTICOMPILE);
  m_MultiCompile.SetToolTip(IDS_SCRIPTEDITOR_MULTICOMPILE_TT);

  m_ToggleLogWindow.Create(CPoint(358, 0), &m_ShowLogWindowIcon, &m_IconButtonLayout, this);
  m_ToggleLogWindow.Connect(this, IDC_LOGWINDOW_TOGGLE);
  m_ToggleLogWindow.SetToolTip(IDS_SCRIPTEDITOR_LOGWINDOW_HIDE_TT);

  m_UnitHierarchy.Create(CPoint(686, 0), &m_UnitHierarchyIcon, &m_IconButtonLayout, this);
  m_UnitHierarchy.Connect(this, IDC_UNITHIERARCHY);
  m_UnitHierarchy.SetToolTip(IDS_SCRIPTEDITOR_UNITHIERARCHY_TT);
  m_Help.Create(CPoint(724, 0), &m_HelpIcon, &m_IconButtonLayout, this);
  m_Help.Connect(this, IDC_HELPIT);
  m_Help.SetToolTip(IDS_SCRIPTEDITOR_HELP_TT);
  m_MapView.Create(CPoint(762, 0), &m_MapViewIcon, &m_IconButtonLayout, this);
  m_MapView.Connect(this, IDC_MAPVIEW);
  m_MapView.SetToolTip(IDS_SCRIPTEDITOR_MAPVIEW_TT);

  strCaption.LoadString(IDS_SCRIPTEDITOR_FILE);
  rcControl.SetRect(400, 0, 480, 20);
  m_FileButton.Create(&rcControl, NULL, strCaption, this);
  m_FileButton.Connect(this, IDC_FILE);

  strCaption.LoadString(IDS_SCRIPTEDITOR_EDIT);
  rcControl.SetRect(480, 0, 560, 20);
  m_EditButton.Create(&rcControl, NULL, strCaption, this);
  m_EditButton.Connect(this, IDC_EDIT);

  strCaption.LoadString(IDS_SCRIPTEDITOR_COMPILEMENU);
  rcControl.SetRect(560, 0, 640, 20);
  m_CompileMenuButton.Create(&rcControl, NULL, strCaption, this);
  m_CompileMenuButton.Connect(this, IDC_COMPILEMENU);

  // create the script edit layout
  m_ScriptEditLayout.Create(GraphicsArchive.CreateFile("ScriptEditor\\Standard.scriptedit"));
  m_CaptionWindowLayout.Create(GraphicsArchive.CreateFile("Windows\\CaptionNonTransparent.window"));
  m_UnitHierarchyTreeControlLayout.Create(GraphicsArchive.CreateFile("Controls\\Standard.treectrl"));

  // Create the help window
  {
    CRect rcRect ( 290, 50, 800, 500 );
    CDataArchive Archive = GraphicsArchive.CreateArchive("ScriptEditor\\HelpBrowser");
    m_HelpFrame.SetTopMost();
    m_HelpFrame.Create( rcRect, this, &m_CaptionWindowLayout, &Archive );
    m_HelpFrame.HideWindow();

    ReadWindowPositionFromRegistry ( "Layout\\ScriptEditor\\Help", &m_HelpFrame, &rcRect, false );
  }

  // Create the unit hierarchy window
  {
    CRect rcRect ( 200, 50, 600, 400 );
    m_UnitHierarchyFrame.SetTopMost();
    m_UnitHierarchyFrame.Create( rcRect, this, &m_CaptionWindowLayout,
      &m_UnitHierarchyTreeControlLayout, pUnitHierarchy);
    m_UnitHierarchyFrame.HideWindow();

    ReadWindowPositionFromRegistry ( "Layout\\ScriptEditor\\UnitHierarchy", &m_UnitHierarchyFrame, &rcRect, false );
  }

  // Connect us to the timer
  g_pTimer->Connect ( this, 0 );

  // Create the profiling window
  {
    CRect rcRect ( 200, 50, 750, 400 );
    m_wndProfilingFrame.SetTopMost ();
    m_wndProfilingFrame.Create ( rcRect, this, &m_CaptionWindowLayout, this );
    m_wndProfilingFrame.HideWindow ();
    m_wndProfilingFrame.Connect ( this, ID_ProfilingWindow );

    ReadWindowPositionFromRegistry ( "Layout\\ScriptEditor\\ProfilingFrame", &m_wndProfilingFrame, &rcRect, false );
    if ( m_wndProfilingFrame.IsVisible () )
      g_pTimer->SetElapseTime ( this, 1 );
  }

  // create the error output layout
  m_ErrorOutputLayout.Create(GraphicsArchive.CreateFile("ScriptEditor\\ErrorOutput.scriptedit"));
  m_ErrorOutputFrameLayout.Create(GraphicsArchive.CreateFile("Windows\\ThinNonTransparent.window"));

  // init menus
  m_FileMenu.AddItem(IDS_SCRIPTEDITOR_FILE_NEWSCRIPT, IDC_NEWSCRIPT);
  m_FileMenu.AddSeparator();
  m_FileMenu.AddItem(IDS_SCRIPTEDITOR_FILE_OPENREMOTESCRIPT, IDC_OPENREMOTESCRIPT);
  m_FileMenu.AddItem(IDS_SCRIPTEDITOR_FILE_SAVEREMOTESCRIPT, IDC_SAVEREMOTESCRIPT);
  m_FileMenu.AddItem(IDS_SCRIPTEDITOR_FILE_SAVEASREMOTESCRIPT, IDC_SAVEASREMOTESCRIPT);
  m_FileMenu.AddSeparator();
  m_FileMenu.AddItem(IDS_SCRIPTEDITOR_FILE_OPENLOCALSCRIPT, IDC_OPENLOCALSCRIPT);
  m_FileMenu.AddItem(IDS_SCRIPTEDITOR_FILE_SAVELOCALSCRIPT, IDC_SAVELOCALSCRIPT);
  m_FileMenu.AddSeparator();
  m_FileMenu.AddItem(IDS_SCRIPTEDITOR_FILE_EXIT, IDC_EXIT);

  m_EditMenu.AddItem(IDS_CUT, IDC_CUT);
  m_EditMenu.AddItem(IDS_COPY, IDC_COPY);
  m_EditMenu.AddItem(IDS_PASTE, IDC_PASTE);
  m_EditMenu.AddSeparator();
  m_EditMenu.AddItem(IDS_DELETE, IDC_DELETE);

  m_CompileMenu.AddItem(IDS_SCRIPTEDITOR_COMPILE_COMPILE, IDC_COMPILE);
  m_CompileMenu.AddItem(IDS_SCRIPTEDITOR_COMPILE_COMPILEANDUPDATE, IDC_COMPILEANDUPDATE);
  m_CompileMenu.AddSeparator();
  m_CompileMenu.AddItem(IDS_SCRIPTEDITOR_COMPILE_MULTICOMPILE, IDC_MULTICOMPILE);
  m_CompileMenu.AddSeparator ();
  m_CompileMenu.AddItem(IDS_SCRIPTEDITOR_COMPILE_PROFILING, IDC_PROFILING );

  // init view layouts
  CScriptView::InitLayout(&m_CaptionWindowLayout, &m_ScriptEditLayout);

  // set the frame rect
  CRect rcMDIFrame, rcWndRect;
  rcWndRect = GetWindowPosition();
  rcMDIFrame.left = rcWndRect.left;
  rcMDIFrame.right = rcWndRect.right;
  rcMDIFrame.bottom = rcWndRect.bottom;
  rcMDIFrame.top = rcWndRect.top + SCRIPTEDITOR_TOOLBAR_HEIGHT;

  // create the frame
  m_MDIFrame.Create(rcMDIFrame, this, this);

  // create the virtual connection to the server's ZScriptEditor object
  {
    DWORD dwUserData;
    
    // fill in the user data
    dwUserData = VirtualConnection_ScriptEditor;

    // create the virtual connection
    m_VirtualConnection = g_pNetworkConnection->CreateVirtualConnectionEx(PACKET_GUARANTEED_DELIVERY | PACKET_GUARANTEED_ORDER,
      this, ID_VirtualConnection, g_dwNetworkEventThreadID, &dwUserData, sizeof(dwUserData));
  }

  CRect rcError;
  rcError = GetWindowPosition();
  rcError.top = rcError.bottom - SCRIPTEDITOR_ERROROUTPUT_HEIGHT;
  m_ErrorOutput.Create(&m_ErrorOutputLayout, &m_ErrorOutputFrameLayout, rcError, this);
  m_ErrorOutput.Connect(this, ID_ErrorOutput);
  ReadWindowPositionFromRegistry ( "Layout\\ScriptEditor\\ErrorOutput", &m_ErrorOutput, &rcError, true );

  ShowErrorOutput ( m_ErrorOutput.IsVisible () );

  // create the log window
  m_LogWindow.Create(&m_ErrorOutputLayout, &m_CaptionWindowLayout, this);
  m_LogWindow.Connect(this, ID_LOGWINDOW);
  {
    CRect rc = m_LogWindow.GetWindowPosition ();
    ReadWindowPositionFromRegistry ( "Layout\\ScriptEditor\\LogWindow", &m_LogWindow, &rc, false );
  }
  ShowLogWindow ( m_LogWindow.IsVisible () );

  // set some accelerators
  g_pKeyAccelerators->Connect(this, VK_F4, 0);  // next error
  g_pKeyAccelerators->Connect(this, VK_F4, CKeyboard::ShiftDown); // prev error

  // store the thread ID
  m_dwWindowEventThreadID = dwEventThreadID;

  // connect us to us
  Connect(this, ID_Myself);

  // reopen all backuped files and delete them
  {
    HANDLE hFind;
    WIN32_FIND_DATA FindData;

    CString strFileMask = m_strBackupDirectory + "\\*.3s";
    if((hFind = ::FindFirstFile(strFileMask, &FindData)) != INVALID_HANDLE_VALUE){
      // loop until there is some more files
      do{
        // open the file and crete a new view for it
        char *pFile;
        HANDLE hFile;
        DWORD dwRead;

        CString strFilePath = m_strBackupDirectory + "\\" + FindData.cFileName;
        hFile = ::CreateFile(strFilePath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if(hFile == INVALID_HANDLE_VALUE){
          DWORD dwError = GetLastError();
          continue;
        }

        DWORD dwSize = ::GetFileSize(hFile, NULL);
        pFile = new char[dwSize + 1];
        if(::ReadFile(hFile, pFile, dwSize, &dwRead, NULL) == 0){
          delete pFile;
          continue;
        }
        if(dwRead < dwSize){
          delete pFile;
          continue;
        }
        ::CloseHandle(hFile);

        // and now delete the file -> we have opened it so the data wan't be lost
        ::DeleteFile(strFilePath);

        pFile[dwSize] = 0;
          
        // create the new view
        CScriptView *pView = NULL;
        pView = m_MDIFrame.CreateView();

        pView->GetScriptEditCtrl()->SetAll(CString(pFile));
        pView->SetFileName(CString(FindData.cFileName));
        pView->SetModified(TRUE);
        pView->SetSaved(TRUE);
        pView->SetLocal(TRUE);

        delete pFile;
      }
      while(::FindNextFile(hFind, &FindData) != 0);

      ::FindClose(hFind);
    }
  }

  return TRUE;
}

void CScriptEditor::Delete()
{
  g_pKeyAccelerators->Disconnect(this);
  g_pKeyAccelerators->Disconnect(this);

  g_pTimer->Disconnect ( this );

  WriteWindowPositionToRegistry ( "Layout\\ScriptEditor\\LogWindow", &m_LogWindow );
  m_LogWindow.Delete();
  WriteWindowPositionToRegistry ( "Layout\\ScriptEditor\\ErrorOutput", &m_ErrorOutput );
  m_ErrorOutput.Delete();
  m_MDIFrame.Delete();

  // Delete the virtual connection
  if(g_pNetworkConnection != NULL)
    g_pNetworkConnection->DeleteVirtualConnection(m_VirtualConnection);

  CScriptView::CloseLayout();
  m_ScriptEditLayout.Delete();
  m_CaptionWindowLayout.Delete();

  m_UnitHierarchyTreeControlLayout.Delete();

  WriteWindowPositionToRegistry ( "Layout\\ScriptEditor\\ProfilingFrame", &m_wndProfilingFrame );
  m_wndProfilingFrame.Delete ();
  WriteWindowPositionToRegistry ( "Layout\\ScriptEditor\\Help", &m_HelpFrame );
  m_HelpFrame.Delete();
  WriteWindowPositionToRegistry ( "Layout\\ScriptEditor\\UnitHierarchy", &m_UnitHierarchyFrame );
  m_UnitHierarchyFrame.Delete();

  m_ErrorOutputLayout.Delete();
  m_ErrorOutputFrameLayout.Delete();

  m_FileMenu.Delete();
  m_EditMenu.Delete();
  m_CompileMenu.Delete();


  m_FileButton.Delete();
  m_NewScriptIcon.Delete();
  m_OpenRemoteScriptIcon.Delete(); m_OpenLocalScriptIcon.Delete();
  m_SaveRemoteScriptIcon.Delete(); m_SaveLocalScriptIcon.Delete();
  m_MapViewIcon.Delete(); m_HelpIcon.Delete(); m_UnitHierarchyIcon.Delete();

  m_CompileMenuButton.Delete();
  m_ShowErrorOutputIcon.Delete(); m_HideErrorOutputIcon.Delete();
  m_ShowLogWindowIcon.Delete(); m_HideLogWindowIcon.Delete();
  m_CompileIcon.Delete(); m_CompileAndUpdateIcon.Delete();
  m_MultiCompileIcon.Delete();

  m_NewScriptButton.Delete();
  m_OpenRemoteScriptButton.Delete(); m_SaveRemoteScriptButton.Delete();
  //m_OpenLocalScriptButton.Delete(); m_SaveLocalScriptButton.Delete();
  m_Compile.Delete(); m_CompileAndUpdate.Delete();
  m_MultiCompile.Delete();
  m_ToggleErrorOutput.Delete();
  m_ToggleLogWindow.Delete();
  m_MapView.Delete();
  m_Help.Delete();
  m_UnitHierarchy.Delete();

  m_EditButton.Delete();

  m_IconButtonLayout.Delete();

  CWindow::Delete();
}

void CScriptEditor::Draw(CDDrawSurface *pSurface, CRect *pRect)
{
  // just clear it
//  pSurface->Fill(0, pRect);
}


void CScriptEditor::OnFile()
{
  CRect rcBut = m_FileButton.GetWindowScreenPosition();
  m_FileMenu.Create(CPoint(rcBut.left, rcBut.bottom), this);
}

void CScriptEditor::OnCompileMenu()
{
  CRect rcBut = m_CompileMenuButton.GetWindowScreenPosition();
  m_CompileMenu.Create(CPoint(rcBut.left, rcBut.bottom), this);
}

void CScriptEditor::OnNewScript()
{
  CScriptView *pView = m_MDIFrame.CreateView();
  static DWORD dwNonameFileNumber = 1;

  pView->SetModified();
  CString strCaption;
  strCaption.Format(IDS_NONAMEFILE, dwNonameFileNumber++);
  pView->SetFileName(strCaption);
}

void CScriptEditor::OnOpenRemoteScript()
{
  // recieve the script list from the server

  // check the network
  // @@@@

  VERIFY(m_lockNetworkCommand.Lock());
  m_dwNetworkCommand = NC_WaitingForServer;
  
  // set the progress window
  StartProgress(IDS_SCRIPTEDITOR_DOWNLOADING);

  VERIFY(m_VirtualConnection.BeginSendCompoundBlock());

  // first send a command to server
  DWORD dwCommand = ScriptEditor_Command_ListFiles;
  VERIFY(m_VirtualConnection.SendBlock(&dwCommand, sizeof(dwCommand)));

  VERIFY(m_VirtualConnection.EndSendCompoundBlock());

  InlayEvent(E_STARTWAITINGFORSERVER, 0, this);

  // start our event loop - wait for the operation to end
  if(CEventManager::FindEventManager(GetCurrentThreadId())->DoEventLoop() != 0){
    // stop the progress window
    StopProgress();
    // some error occured
    CMessageBox::OK(IDS_SCRIPTEDITOR_SERVERSIDEERROR, IDS_SCRIPTEDITOR_CAPTION, this);
    // return;
  }

  StopProgress();

  // OK donkey - all was good -> show the dialog
  m_lockFileLists.Lock();

  COpenRemoteScriptDlg dlg;

  dlg.m_strLocalList = m_strLocalFileList;
  dlg.m_strGlobalList = m_strGlobalFileList;
  if(dlg.DoModal(CPoint(50, 50), this) == CDialogWindow::IDC_OK){
    m_lockFileLists.Unlock();
    m_bOpenFile_Local = dlg.m_bLocalScript;
    LoadScript(dlg.m_strFileName, dlg.m_bLocalScript);
    return;
  }

  m_lockFileLists.Unlock();
}

void CScriptEditor::OnSaveRemoteScript()
{
  CScriptView *pView = m_MDIFrame.GetActiveView();

  if(pView == NULL) return;

  SaveRemoteScript(pView);
}

BOOL CScriptEditor::SaveRemoteScript(CScriptView *pView)
{
  ASSERT(pView != NULL);

  if((!pView->GetSaved()) || (!pView->GetLocal())){
    // determine if we have the save name
    // if not call the SaveAs
    return SaveAsRemoteScript(pView);
  }

  // just save it here
  return SaveScript(pView, pView->GetFileName());
}

void CScriptEditor::OnSaveAsRemoteScript()
{
  // get the script view
  CScriptView *pView = m_MDIFrame.GetActiveView();
  if(pView == NULL) return;

  SaveAsRemoteScript(pView);
}

BOOL CScriptEditor::SaveAsRemoteScript(CScriptView *pView)
{
  ASSERT(pView != NULL);

  // recieve the script list from the server

  // check the network
  // @@@@

  VERIFY(m_lockNetworkCommand.Lock());
  m_dwNetworkCommand = NC_WaitingForServer;
  
  // set the progress window
  StartProgress(IDS_SCRIPTEDITOR_DOWNLOADING);

  VERIFY(m_VirtualConnection.BeginSendCompoundBlock());

  // first send a command to server
  DWORD dwCommand = ScriptEditor_Command_ListFiles;
  VERIFY(m_VirtualConnection.SendBlock(&dwCommand, sizeof(dwCommand)));

  VERIFY(m_VirtualConnection.EndSendCompoundBlock());

  InlayEvent(E_STARTWAITINGFORSERVER, 0, this);

  // start our event loop - wait for the operation to end
  if(CEventManager::FindEventManager(GetCurrentThreadId())->DoEventLoop() != 0){
    // stop the progress window
    StopProgress();
    // some error occured
    CMessageBox::OK(IDS_SCRIPTEDITOR_SERVERSIDEERROR, IDS_SCRIPTEDITOR_CAPTION, this);
    // return;
    return FALSE;
  }

  StopProgress();

  m_lockFileLists.Lock();

  CSaveRemoteScriptDlg dlg;

  if(pView->GetSaved())
    dlg.m_strFileName = pView->GetFileName();

  dlg.m_strLocalList = m_strLocalFileList;
  if(dlg.DoModal(CPoint(50, 50), this) == CDialogWindow::IDC_OK){
    m_lockFileLists.Unlock();
    SaveScript(pView, dlg.m_strFileName);
    return TRUE;
  }
  else{
    m_lockFileLists.Unlock();
    return FALSE;
  }
}

void CScriptEditor::OnOpenLocalScript()
{
  COpenLocalScriptDlg dlg;

  if(dlg.DoModal(this) != CDialogWindow::IDC_OK) return;

  if(!LoadScriptFromDisk(dlg.m_strName)){
    CMessageBox::OK(IDS_SCRIPTEDITOR_OPENLOCALSCRIPT_ERROR, IDS_SCRIPTEDITOR_OPENLOCALSCRIPT_CAPTION, this);
    return;
  }
}

void CScriptEditor::OnSaveLocalScript()
{
  // till now nothing
}


void CScriptEditor::OnEdit()
{
  CRect rcBut = m_EditButton.GetWindowScreenPosition();
  m_EditMenu.Create(CPoint(rcBut.left, rcBut.bottom), this);
}

void CScriptEditor::OnCut()
{
  if(m_MDIFrame.GetActiveView() != NULL){
    m_MDIFrame.GetActiveView()->Cut();
  }
}

void CScriptEditor::OnCopy()
{
  if(m_MDIFrame.GetActiveView() != NULL){
    m_MDIFrame.GetActiveView()->Copy();
  }
}

void CScriptEditor::OnPaste()
{
  if(m_MDIFrame.GetActiveView() != NULL){
    m_MDIFrame.GetActiveView()->Paste();
  }
}

void CScriptEditor::OnDelete()
{
  if(m_MDIFrame.GetActiveView() != NULL){
    m_MDIFrame.GetActiveView()->DeleteSelection();
  }
}



void CScriptEditor::OnSomeOpen()
{
  m_FileMenu.Create(CPoint(45, 5), this);
}



void CScriptEditor::ShowErrorOutput(BOOL bShow)
{
  CRect rcMDI = GetWindowPosition();

  rcMDI.top += SCRIPTEDITOR_TOOLBAR_HEIGHT;
  if(bShow)
    rcMDI.bottom -= SCRIPTEDITOR_ERROROUTPUT_HEIGHT;

  m_MDIFrame.SetWindowPosition(&rcMDI);

  if(bShow){
    m_ErrorOutput.ShowWindow();
    m_ToggleErrorOutput.SetIcon(&m_HideErrorOutputIcon);
    m_ToggleErrorOutput.SetToolTip(IDS_SCRIPTEDITOR_ERROROUTPUT_HIDE_TT);
  }
  else{
    m_ErrorOutput.HideWindow();
    m_ToggleErrorOutput.SetIcon(&m_ShowErrorOutputIcon);
    m_ToggleErrorOutput.SetToolTip(IDS_SCRIPTEDITOR_ERROROUTPUT_SHOW_TT);
  }
}

void CScriptEditor::ClearErrorOutput()
{
  m_ErrorOutput.Clear();
}

void CScriptEditor::OnErrorOutputToggle()
{
  if(m_ErrorOutput.IsVisible()){
    ShowErrorOutput(FALSE);
  }
  else{
    ShowErrorOutput(TRUE);
  }
}


void CScriptEditor::OnCompile()
{
  // get current view
  CScriptView *pView = m_MDIFrame.GetActiveView();
  
  if(pView == NULL) return;

  // save the script
  if(!SaveRemoteScript(pView)) return;

  // show the error output
  ShowErrorOutput(TRUE);
  // and clean it
  ClearErrorOutput();

  // compile it
  Compile(pView, FALSE);
}

void CScriptEditor::OnCompileAndUpdate()
{
  // get current view
  CScriptView *pView = m_MDIFrame.GetActiveView();
  
  if(pView == NULL) return;

  // save the script
  if(!SaveRemoteScript(pView)) return;

  // show the error output
  ShowErrorOutput(TRUE);
  // and clean it
  ClearErrorOutput();

  // compile it
  Compile(pView, TRUE);
}

// Reaction on the arriving packet
// ATTENTION - this method runs in other thread then the rest of this object
void CScriptEditor::OnPacketAvailable()
{
  // first recieve the command (or answer)
  DWORD dwAnswer;
  DWORD dwSize = sizeof(dwAnswer);

  VERIFY(m_VirtualConnection.BeginReceiveCompoundBlock());
  
  VERIFY(m_VirtualConnection.ReceiveBlock(&dwAnswer, dwSize));

  switch(dwAnswer){

    // list of files is available
  case ScriptEditor_Answer_ListFiles:
    {
      m_lockNetworkCommand.Lock();

      ASSERT(m_dwNetworkCommand != NC_None);

      // recieve the header structure
      SScriptEditor_ListFiles header;
      dwSize = sizeof(header);
      VERIFY(m_VirtualConnection.ReceiveBlock(&header, dwSize));
      
      // allocate buffers
      char *pLocals, *pGlobals;
      pLocals = new char[header.m_dwLocalLength];
      pGlobals = new char[header.m_dwGlobalLength];
      
      // get the buffers from the net
      VERIFY(m_VirtualConnection.ReceiveBlock(pLocals, header.m_dwLocalLength));
      VERIFY(m_VirtualConnection.ReceiveBlock(pGlobals, header.m_dwGlobalLength));
      
      // copy lists to member variables
      VERIFY(m_lockFileLists.Lock());
      m_strLocalFileList = pLocals;
      m_strGlobalFileList = pGlobals;
      VERIFY(m_lockFileLists.Unlock());
      
      // free buffers
      delete pLocals;
      delete pGlobals;
      
      m_dwNetworkCommand = NC_None;

      // just close the message loop
      CEventManager::FindEventManager(m_dwWindowEventThreadID)->InlayQuitEvent(0);

      m_lockNetworkCommand.Unlock();
    }
    break;
    
    // saving of the file was successful
  case ScriptEditor_Answer_SaveFile_Success:
    {
      m_lockNetworkCommand.Lock();
      ASSERT(m_dwNetworkCommand != NC_None);

      m_dwNetworkCommand = NC_None;
      // just close the message loop
      CEventManager::FindEventManager(m_dwWindowEventThreadID)->InlayQuitEvent(0);

      m_lockNetworkCommand.Unlock();
    }
    break;
    
    // load script done
  case ScriptEditor_Answer_LoadFile:
    {
      m_lockNetworkCommand.Lock();
      ASSERT(m_dwNetworkCommand != NC_None);

      // recieve the header
      SScriptEditor_LoadFile_Answer header;
      dwSize = sizeof(header);
      VERIFY(m_VirtualConnection.ReceiveBlock(&header, dwSize));
      
      // create the buffer
      char *pFileName = new char[header.m_dwFileNameLength];
      char *pBuffer = new char[header.m_dwFileLength];
      
      // load it from network
      dwSize = header.m_dwFileNameLength;
      VERIFY(m_VirtualConnection.ReceiveBlock(pFileName, dwSize));
      dwSize = header.m_dwFileLength;
      VERIFY(m_VirtualConnection.ReceiveBlock(pBuffer, dwSize));
      
      // store the source in the string
      m_strOpenFile_FileContents = pBuffer;
      m_strOpenFile_FileName = pFileName;
      
      delete pBuffer;
      delete pFileName;
      
      m_dwNetworkCommand = NC_None;

      // just close the message loop
      CEventManager::FindEventManager(m_dwWindowEventThreadID)->InlayQuitEvent(0);

      m_lockNetworkCommand.Unlock();
    }
    break;

    // The profiling info arrived
  case ScriptEditor_Answer_GetProfilingInfo:
    {
      SScriptEditor_GetProfilingInfo_Answer sHeader;
      DWORD dwSize = sizeof ( sHeader );
      VERIFY(m_VirtualConnection.ReceiveBlock ( &sHeader, dwSize ) );
      ASSERT ( dwSize == sizeof ( sHeader ) );

      __int64 *pCounters = new __int64 [ 2 ];
      pCounters [ 0 ] = sHeader.m_nLocalCounter;
      pCounters [ 1 ] = sHeader.m_nGlobalCounter;
      InlayEvent ( CProfilingFrame::E_UpdateCounters, (DWORD)pCounters, &m_wndProfilingFrame );

      // Get the string
      char *pProfilingInfo = new char [ sHeader.m_dwLength ];
      dwSize = sHeader.m_dwLength;
      VERIFY(m_VirtualConnection.ReceiveBlock ( pProfilingInfo, dwSize ) );
      ASSERT ( dwSize == sHeader.m_dwLength );

      // Send the event to update the info in the window
      InlayEvent ( CProfilingFrame::E_UpdateInfo, (DWORD)pProfilingInfo, &m_wndProfilingFrame );
    }
    break;
    
    // oops the server side error
  case ScriptEditor_Command_Error:
    // error recieved
    {
      m_lockNetworkCommand.Lock();
      ASSERT(m_dwNetworkCommand != NC_None);

      m_dwNetworkCommand = NC_None;
      // report the error
      CEventManager::FindEventManager(m_dwWindowEventThreadID)->InlayQuitEvent(1);

      m_lockNetworkCommand.Unlock();
    }
    break;

  default:
    TRACE("Unknown packet recieved.\n");
    ASSERT(FALSE);
  };
  
  VERIFY(m_VirtualConnection.EndReceiveCompoundBlock());
}

// some error occured
void CScriptEditor::OnNetworkError()
{
  // if there is some waiting network command -> cancel it
  m_lockNetworkCommand.Lock();

  if(m_dwNetworkCommand != NC_None){
    m_dwNetworkCommand = NC_None;
    // report the error
    CEventManager *pManager = CEventManager::FindEventManager(m_dwWindowEventThreadID);
    if(pManager != NULL)
      pManager->InlayQuitEvent(1);
  }

  m_lockNetworkCommand.Unlock();
}

// the probably first event after starting the waiting loop for network answer
void CScriptEditor::OnStartWaitingForServer()
{
  // verify if the network status is OK
  if(!g_pNetworkConnection->IsNetworkReady()){
    // OOOOPS .. network is down -> cancel the action
    m_dwNetworkCommand = NC_None;

    // report the error
    CEventManager::FindEventManager(m_dwWindowEventThreadID)->InlayQuitEvent(1);
  }
  // just unlock the network command
  VERIFY(m_lockNetworkCommand.Unlock());
}




// shows the progress and disables the script editor
void CScriptEditor::StartProgress(CString strMessage)
{
  // disable the window
  EnableWindow(FALSE);

  // show the progress window
  m_ProgressWindow.Create(strMessage, this->GetParentWindow());
}

// hides the progress and enables the script editor back again
void CScriptEditor::StopProgress()
{
  // hide the progress window
  m_ProgressWindow.Delete();

  // enable the window
  EnableWindow(TRUE);
}


// Saves the script from givven view to the server
BOOL CScriptEditor::SaveScript(CScriptView *pView, CString strFileName)
{
  ASSERT_VALID(pView);

  // recieve the script list from the server

  VERIFY(m_lockNetworkCommand.Lock());
  m_dwNetworkCommand = NC_WaitingForServer;
  
  // set the progress window
  StartProgress(IDS_SCRIPTEDITOR_SAVING);

  // prepare the packet
  DWORD dwCommand = ScriptEditor_Command_SaveFile;

  // send the command
  VERIFY(m_VirtualConnection.BeginSendCompoundBlock());
  VERIFY(m_VirtualConnection.SendBlock(&dwCommand, sizeof(dwCommand)));

  // create the header
  SScriptEditor_SaveFile header;
  CString strFile = pView->GetScriptEditCtrl()->GetAll();
  pView->SetFileName(strFileName);

  // Make the shadow copy of the file
  try{
    CString strShadowFileName = m_strLocalShadowDirectory + "\\" + strFileName;
    CArchiveFile file = CDataArchive::GetRootArchive()->CreateFile(strShadowFileName, CArchiveFile::modeWrite | CArchiveFile::modeCreate | CArchiveFile::modeUncompressed);
    file.Write((LPCSTR)strFile, strFile.GetLength());
  }
  catch(CException *e){
    e->Delete();
    TRACE("Error writing file to shadow directory.\n");
  }

  header.m_dwFileNameLength = strFileName.GetLength() + 1;
  header.m_dwFileLength = strFile.GetLength() + 1;

  // send the header
  VERIFY(m_VirtualConnection.SendBlock(&header, sizeof(header)));

  // send the file name
  VERIFY(m_VirtualConnection.SendBlock((LPCSTR)strFileName, strFileName.GetLength() + 1));
  // send the file
  VERIFY(m_VirtualConnection.SendBlock((LPCSTR)strFile, strFile.GetLength() + 1));

  VERIFY(m_VirtualConnection.EndSendCompoundBlock());

  InlayEvent(E_STARTWAITINGFORSERVER, 0, this);

  // start our event loop - wait for the operation to end
  if(CEventManager::FindEventManager(GetCurrentThreadId())->DoEventLoop() != 0){
    // stop the progress window
    StopProgress();
    // some error occured
    CMessageBox::OK(IDS_SCRIPTEDITOR_SERVERSIDEERROR, IDS_SCRIPTEDITOR_CAPTION, this);
    // return;
    return FALSE;
  }

  StopProgress();

  // mark the view as not modified
  pView->SetModified(FALSE);
  pView->SetLocal(TRUE);
  pView->SetSaved(TRUE);

  // and that's all
  return TRUE;
}

BOOL CScriptEditor::LoadScript(CString strFileName, BOOL bLocal)
{
  // set the progress window
  StartProgress(IDS_SCRIPTEDITOR_LOADING);

  VERIFY(m_lockNetworkCommand.Lock());
  m_dwNetworkCommand = NC_WaitingForServer;

  // prepare the packet
  DWORD dwCommand = ScriptEditor_Command_LoadFile;

  // send the command
  VERIFY(m_VirtualConnection.BeginSendCompoundBlock());
  VERIFY(m_VirtualConnection.SendBlock(&dwCommand, sizeof(dwCommand)));

  // create the header
  SScriptEditor_LoadFile_Command header;

  header.m_dwFileNameLength = strFileName.GetLength() + 1;
  if(bLocal)
    header.m_eFileTypes = ScriptEditor_LoadFile_UserFile;
  else
    header.m_eFileTypes = ScriptEditor_LoadFile_SystemFile;

  // send the header
  VERIFY(m_VirtualConnection.SendBlock(&header, sizeof(header)));

  // send the file name
  VERIFY(m_VirtualConnection.SendBlock((LPCSTR)strFileName, strFileName.GetLength() + 1));

  VERIFY(m_VirtualConnection.EndSendCompoundBlock());

  InlayEvent(E_STARTWAITINGFORSERVER, 0, this);

  // start our event loop - wait for the operation to end
  if(CEventManager::FindEventManager(GetCurrentThreadId())->DoEventLoop() != 0){
    // stop the progress window
    StopProgress();
    // some error occured
    CMessageBox::OK(IDS_SCRIPTEDITOR_SERVERSIDEERROR, IDS_SCRIPTEDITOR_CAPTION, this);
    // return;
    return FALSE;
  }

  StopProgress();

  // create new view
  CScriptView *pView;
  pView = m_MDIFrame.FindView(m_strOpenFile_FileName, m_bOpenFile_Local);
  if(pView == NULL)
    pView = m_MDIFrame.CreateView();
  pView->SetFileName(m_strOpenFile_FileName);
  m_strOpenFile_FileName.Empty();
  pView->SetSaved(TRUE);
  pView->SetLocal(m_bOpenFile_Local);

  pView->GetScriptEditCtrl()->SetAll(m_strOpenFile_FileContents);
  m_strOpenFile_FileContents.Empty();

  // activate loaded view
  pView->Activate();

  // and that's all
  return TRUE;
}

BOOL CScriptEditor::LoadScriptFromDisk(CString strFileName)
{
  CString strFile;
  try{
    CArchiveFile file = CDataArchive::GetRootArchive()->CreateFile(strFileName, CArchiveFile::modeRead | CFile::shareDenyWrite);

    file.Read(strFile.GetBuffer(file.GetLength()), file.GetLength());
    strFile.ReleaseBuffer(file.GetLength());
    file.Close();

  }
  catch(CException *e){
    e->Delete();
    return FALSE;
  }

  // create new view
  CScriptView *pView;
  pView = m_MDIFrame.FindView(strFileName, TRUE);
  if(pView == NULL)
    pView = m_MDIFrame.CreateView();
  pView->SetFileName(strFileName);
  pView->SetSaved(FALSE);
  pView->SetLocal(TRUE);

  pView->GetScriptEditCtrl()->SetAll(strFile);

  // activate loaded view
  pView->Activate();

  // and that's all
  return TRUE;
}

BOOL CScriptEditor::ClosingUnsavedView(CScriptView *pView)
{
  ASSERT_VALID(pView);
  ASSERT(pView->GetModified());

  // first ask the user if he wants to save it
  CString strQuestion, strCaption;
  strQuestion.Format(IDS_SCRIPTEDITOR_SAVEMODIFIEDVIEW, pView->GetFileName());
  strCaption.LoadString(IDS_SCRIPTEDITOR_CAPTION);
  switch(CMessageBox::YesNoCancel(strQuestion, strCaption, this)){
  case CMessageBox::IDC_No:
    return TRUE;
  case CMessageBox::IDC_Yes:
    break;
  default:
    return FALSE;
  }

  // OK - he's sure to save it
  // so do it
  if(!SaveRemoteScript(pView)) return FALSE;

  // return OK
  return TRUE;
}

BOOL CScriptEditor::CanClose()
{
  ASSERT_VALID(this);

//  if(!g_pNetworkConnection->IsNetworkReady()){
  if(TRUE){
    // Oh fuck - the network is down
    // so we must do some special closing

    // it means backup all scripts to local disk
    m_MDIFrame.BackupViews(m_strBackupDirectory);

    // and don's ask the user to save any files - it's impossible
    return TRUE;
  }

  // Call it on MDI frame
  if(!m_MDIFrame.CloseAllViews()) return FALSE;

  return TRUE;
}

void CScriptEditor::Compile(CScriptView *pView, BOOL bUpdate)
{
  // prepare the packet
  DWORD dwCommand = ScriptEditor_Command_Compile;

  // send the command
  VERIFY(m_VirtualConnection.BeginSendCompoundBlock());
  VERIFY(m_VirtualConnection.SendBlock(&dwCommand, sizeof(dwCommand)));

  // create the header
  SScriptEditor_Compile_Command header;

  CString strFileName = pView->GetFileName();
  header.m_dwFileNameLength = strFileName.GetLength() + 1;
  header.m_dwUpdate = bUpdate;

  // send the header
  VERIFY(m_VirtualConnection.SendBlock(&header, sizeof(header)));

  // send the file name
  VERIFY(m_VirtualConnection.SendBlock((LPCSTR)strFileName, strFileName.GetLength() + 1));

  VERIFY(m_VirtualConnection.EndSendCompoundBlock());
}

void CScriptEditor::ShowLogWindow(BOOL bShow)
{
  if(bShow){
    m_LogWindow.Show();
	m_LogWindow.Activate ();
    m_ToggleLogWindow.SetIcon(&m_HideLogWindowIcon);
    m_ToggleLogWindow.SetToolTip(IDS_SCRIPTEDITOR_LOGWINDOW_HIDE_TT);
  }
  else{
    m_LogWindow.Hide();
    m_ToggleLogWindow.SetIcon(&m_ShowLogWindowIcon);
    m_ToggleLogWindow.SetToolTip(IDS_SCRIPTEDITOR_LOGWINDOW_SHOW_TT);
  }
}

void CScriptEditor::OnLogWindowToggle()
{
  if(m_LogWindow.IsVisible()){
    ShowLogWindow(FALSE);
  }
  else{
    ShowLogWindow(TRUE);
  }
}

void CScriptEditor::OnLogWindowHidden()
{
  ShowLogWindow(FALSE);
}

void CScriptEditor::OnExit()
{
  // just post the close message to the main window
  AfxGetMainWnd()->PostMessage(WM_CLOSE);
}

void CScriptEditor::OnMultiCompile()
{
  // first save all modified windows
  if(!m_MDIFrame.SaveAllViews()) return;

  ClearErrorOutput();

  // send the command to server
  DWORD dwCommand = ScriptEditor_Command_MultiCompile;
  VERIFY(m_VirtualConnection.SendBlock(&dwCommand, sizeof(dwCommand)));

  // and that's all
}

void CScriptEditor::OnSelectErrorLine()
{
  // just open the right view with the right line
  OpenFileLine(m_ErrorOutput.GetErrorFileName(), m_ErrorOutput.GetErrorFileLine());
}

void CScriptEditor::OpenFileLine(CString strFileName, DWORD dwLine)
{
  CScriptView *pView;

  pView = m_MDIFrame.FindView(strFileName, TRUE);

  if(pView == NULL){
    // just load the script
    if(!LoadScript(strFileName, TRUE)) return;

    // now get the loaded view (the active one)
    pView = m_MDIFrame.GetActiveView();
  }

  // set the line
  pView->SetCaretLine(dwLine);
}

BOOL CScriptEditor::OnKeyAccelerator(UINT nChar, DWORD dwFlags)
{
  switch(nChar){
  case VK_F4:
    if(dwFlags & CKeyboard::ShiftDown){
      m_ErrorOutput.PrevError();
    }
    else{
      m_ErrorOutput.NextError();
    }
    break;
  }

  return FALSE;
}

void CScriptEditor::OnMapView()
{
  InlayEvent(E_MAPVIEW, 0);
}

void CScriptEditor::OnHelp()
{
  if(m_HelpFrame.IsVisible()){
    m_HelpFrame.HideWindow();
  }
  else{
    m_HelpFrame.ShowWindow();
	m_HelpFrame.ShowWindow ();
  }
}

void CScriptEditor::OnUnitHierarchy()
{
  if(m_UnitHierarchyFrame.IsVisible()){
    m_UnitHierarchyFrame.HideWindow();
  }
  else{
    m_UnitHierarchyFrame.ShowWindow();
	m_UnitHierarchyFrame.Activate ();
  }
}

void CScriptEditor::OnProfiling ()
{
  m_wndProfilingFrame.ShowWindow ();
  m_wndProfilingFrame.Activate ();

  g_pTimer->SetElapseTime ( this, 1 );
}

void CScriptEditor::OnProfilingUpdate ()
{
  // Send the profiling request to the server

  // prepare the packet
  DWORD dwCommand = ScriptEditor_Command_GetProfilingInfo;

  // send the command
  VERIFY(m_VirtualConnection.BeginSendCompoundBlock());
  VERIFY(m_VirtualConnection.SendBlock(&dwCommand, sizeof(dwCommand)));

  VERIFY(m_VirtualConnection.EndSendCompoundBlock () );

  g_pTimer->SetElapseTime ( this, SCRIPTEDITOR_PROFILINGUPDATE_PERIOD );
}

void CScriptEditor::OnStopProfilingUpdate ()
{
  // Stop the profiling requests
  g_pTimer->SetElapseTime ( this, 0 );
}

void CScriptEditor::OnTimeTick ( DWORD dwTime )
{
  OnProfilingUpdate ();
}
