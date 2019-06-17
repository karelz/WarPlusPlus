// ScriptEditor.h: interface for the CScriptEditor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SCRIPTEDITOR_H__176028F1_AB37_11D3_99C2_99ADDD254479__INCLUDED_)
#define AFX_SCRIPTEDITOR_H__176028F1_AB37_11D3_99C2_99ADDD254479__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ScriptMDIFrame.h"
#include "ErrorOutput.h"
#include "LogWindow.h"
#include "HelpFrame.h"
#include "ProfilingFrame.h"

#include "..\Controls\ProgressWindow.h"

#include "..\UnitHierarchy\UnitHierarchyFrame.h"

class CUnitHierarchy;

class CScriptEditor : public CWindow  
{
  DECLARE_DYNAMIC(CScriptEditor);
  DECLARE_OBSERVER_MAP(CScriptEditor);

public:
  // Called when the user wants to exit the application
  // returns TRUE if everything is OK and real exit can proceed
  // elseway returns FALSE - means no exit should appear
  BOOL CanClose();

  // called when closing unsaved view
  // returns TRUE if the view can be closed
  // elseway FALSE
  BOOL ClosingUnsavedView(CScriptView *pView);

  // called by system for drawing
	virtual void Draw(CDDrawSurface *pSurface, CRect *pRect);

  // deletes the editor
	virtual void Delete();
  // creates the editor -> you have to specify the archive from which it will load the graphcis
	BOOL Create(CDataArchive GraphicsArchive, CString strBackupDirectory, CString strShadowDirectory,
    CWindow *pParent, DWORD dwEventThreadID, CUnitHierarchy *pUnitHierarchy);

  // constructor & destructor
	CScriptEditor();
	virtual ~CScriptEditor();

  // Events sent by this object
  enum{
    E_MAPVIEW = 0x01000,
  };

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

  enum{
    IDC_FILE = 0x0200,
    IDC_NEWSCRIPT = 0x0201,
    IDC_OPENREMOTESCRIPT = 0x0202,
    IDC_SAVEREMOTESCRIPT = 0x0203,
    IDC_OPENLOCALSCRIPT = 0x0204,
    IDC_SAVELOCALSCRIPT = 0x0205,
    IDC_SAVEASREMOTESCRIPT = 0x0206,
    IDC_EXIT = 0x0207,

    IDC_COMPILEMENU = 0x0210,
    IDC_ERROROUTPUT_TOGGLE = 0x0211,
    IDC_COMPILE = 0x0212,
    IDC_COMPILEANDUPDATE = 0x0213,
    IDC_MULTICOMPILE = 0x0214,
    IDC_LOGWINDOW_TOGGLE = 0x0215,

    IDC_PROFILING = 0x0216,

    IDC_EDIT = 0x0300,
    IDC_CUT = 0x0301,
    IDC_COPY = 0x0302,
    IDC_PASTE = 0x0303,
    IDC_DELETE = 0x0304,

    IDC_MAPVIEW = 0x0400,
    IDC_HELPIT = 0x0401,
    IDC_UNITHIERARCHY = 0x0402,

    ID_LOGWINDOW = 0x10,
    ID_ProfilingWindow = 0x11,

    ID_VirtualConnection = 1,
    ID_ErrorOutput = 2,
    ID_Myself = 0x0FFFF,
  };

  CPopupMenu m_FileMenu;
  CPopupMenu m_EditMenu;
  CPopupMenu m_CompileMenu;

  CIconButton m_NewScriptButton;
  CIconButton m_OpenRemoteScriptButton;
  CIconButton m_SaveRemoteScriptButton;
  CIconButton m_OpenLocalScriptButton;
  CIconButton m_SaveLocalScriptButton;
  CIconButton m_ToggleErrorOutput;
  CIconButton m_Compile;
  CIconButton m_CompileAndUpdate;
  CIconButton m_MultiCompile;
  CIconButton m_ToggleLogWindow;
  CIconButton m_MapView;
  CIconButton m_Help;
  CIconButton m_UnitHierarchy;

  CTextButton m_FileButton;
  CTextButton m_EditButton;
  CTextButton m_CompileMenuButton;

  CAnimation m_NewScriptIcon;
  CAnimation m_OpenRemoteScriptIcon;
  CAnimation m_OpenLocalScriptIcon;
  CAnimation m_SaveRemoteScriptIcon;
  CAnimation m_SaveLocalScriptIcon;
  CAnimation m_ShowErrorOutputIcon, m_HideErrorOutputIcon;
  CAnimation m_CompileIcon;
  CAnimation m_CompileAndUpdateIcon;
  CAnimation m_MultiCompileIcon;
  CAnimation m_ShowLogWindowIcon, m_HideLogWindowIcon;
  CAnimation m_MapViewIcon;
  CAnimation m_HelpIcon;
  CAnimation m_UnitHierarchyIcon;

  CIconButtonLayout m_IconButtonLayout;
protected:
	
  void OnFile();
  void OnNewScript();
	void OnOpenRemoteScript();
  void OnSaveRemoteScript();
  void OnSaveAsRemoteScript();
  void OnOpenLocalScript();
  void OnSaveLocalScript();
  void OnExit();

  void OnEdit();
  void OnCut();
  void OnCopy();
  void OnPaste();
  void OnDelete();

  void OnCompileMenu();
  void OnErrorOutputToggle();
  void OnCompile();
  void OnCompileAndUpdate();
  void OnMultiCompile();
  void OnLogWindowToggle();

  void OnUnitHierarchy();
  void OnHelp();
  void OnMapView();

  void OnProfiling ();
  void OnProfilingUpdate ();
  void OnStopProfilingUpdate ();

  void OnSomeOpen();

  // Reactions on network events
  void OnPacketAvailable();
  void OnNetworkError();

  // Reactions on our events
  void OnStartWaitingForServer();

  // reaction on log window events
  void OnLogWindowHidden();

  // reaction on error output window events
  void OnSelectErrorLine();

  BOOL OnKeyAccelerator(UINT nChar, DWORD dwFlags);

  void OnTimeTick ( DWORD dwTime );

  enum{
    E_STARTWAITINGFORSERVER = 1,
  };

private:

  // the MDI frame - parent of all vies
  CScriptMDIFrame m_MDIFrame;

  // the error output window
  CErrorOutput m_ErrorOutput;

  // the log window
  CLogWindow m_LogWindow;

  // the help frame window
  CHelpFrame m_HelpFrame;

  // the unit hierarchy window
  CUnitHierarchyFrame m_UnitHierarchyFrame;

  // the profiling frame
  CProfilingFrame m_wndProfilingFrame;

  // resizes the MDIFrame in a way the error output will be visible or not
  void ShowErrorOutput(BOOL bShow = TRUE);
  // cleans the error output
  void ClearErrorOutput();

  // shows the log window
  void ShowLogWindow(BOOL bShow = TRUE);

  // The virtual connection to the server's ZScriptEditor object
  CVirtualConnection m_VirtualConnection;

  // layout of the script edit
  CScriptEditLayout m_ScriptEditLayout;
  CCaptionWindowLayout m_CaptionWindowLayout;
  CTreeControlLayout m_UnitHierarchyTreeControlLayout;

  // layout of the error output window (in fact script edit)
  CScriptEditLayout m_ErrorOutputLayout;
  CFrameWindowLayout m_ErrorOutputFrameLayout;

  // Progress window for everything
  CProgressWindow m_ProgressWindow;

  // shows the progress window with givven string & disables the whole script editor
  void StartProgress(UINT nID){ CString strMessage; strMessage.LoadString(nID); StartProgress(strMessage); }
  void StartProgress(CString strMessage);
  // hides the progress window & enables the script editor back
  void StopProgress();

  // save the script to the server
  BOOL SaveRemoteScript(CScriptView *pView);
  // save as the script to the server
  BOOL SaveAsRemoteScript(CScriptView *pView);

  // Saves the script from given view
  BOOL SaveScript(CScriptView *pView, CString strFileName);
  // Loads the script from the server to new view
  BOOL LoadScript(CString strFileName, BOOL bLocal);

  // loads the script from local disk
  BOOL LoadScriptFromDisk(CString strFileName);

  // compiles the current script
  void Compile(CScriptView *pView, BOOL bUpdate = FALSE);

  // open the given file (remote user script) and sets the caret to the given line
  void OpenFileLine(CString strFileName, DWORD dwLine);

  // mutex for network status
  CSemaphore m_lockNetworkCommand;
  // the network status
  DWORD m_dwNetworkCommand;

  enum{
    NC_None = 0,
    NC_WaitingForServer = 1
  };

  void SetNetworkCommand(DWORD dwCommand){
    VERIFY(m_lockNetworkCommand.Lock());
    ASSERT(m_dwNetworkCommand == NC_None); m_dwNetworkCommand = dwCommand;
    VERIFY(m_lockNetworkCommand.Unlock());
  }

  // the thread where are window events processed
  DWORD m_dwWindowEventThreadID;

  // lock for the file lists
  CMutex m_lockFileLists;
  // the local list of files
  CString m_strLocalFileList;
  CString m_strGlobalFileList;

  // the file contents for loading
  CString m_strOpenFile_FileContents;
  // the file name for loading
  CString m_strOpenFile_FileName;
  // the flag - Local - Global
  BOOL m_bOpenFile_Local;

  // path to the shadow directory
  CString m_strLocalShadowDirectory;
  CString m_strGlobalShadowDirectory;

  // path to the backup directory (for unsaved scripts)
  CString m_strBackupDirectory;

  friend class CScriptMDIFrame;
};

#endif // !defined(AFX_SCRIPTEDITOR_H__176028F1_AB37_11D3_99C2_99ADDD254479__INCLUDED_)
