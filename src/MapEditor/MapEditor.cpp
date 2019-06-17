// MapEditor.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "MapEditor.h"

#include "MainFrm.h"
#include "MapEditorDoc.h"
#include "MapEditorView.h"

#include "..\GameClient\Common\CommonExceptions.h"
#include "Dialogs\SplashWindowDlg.h"
#include "Dialogs\StartDlg.h"

#include "..\GameClient\Windows\Timer.h"

#include "MapexEditor\MapexEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMapEditorApp

BEGIN_MESSAGE_MAP(CMapEditorApp, CWinApp)
	//{{AFX_MSG_MAP(CMapEditorApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMapEditorApp construction

CMapEditorApp::CMapEditorApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CMapEditorApp object

CMapEditorApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CMapEditorApp initialization

BOOL bMainFrameCreated = FALSE;

CString *g_strStartupDirectory;

BOOL CMapEditorApp::InitInstance()
{
  BOOL bLoadLast = FALSE;

  char txt[1024];
  GetCurrentDirectory(1023, txt);
  g_strStartupDirectory = new CString(txt);

  CSplashWindowDlg Splash;
  Splash.Create(CSplashWindowDlg::IDD, NULL);
  Splash.ShowWindow(SW_SHOW);
  Splash.UpdateWindow();
  m_pMainWnd = NULL;
  
  AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	SetRegistryKey(_T("Strategy Team"));

	LoadStdProfileSettings();  // Load standard INI file options (including MRU)

  try{
    // create the event manager for this thread
    if(!m_EventManager.Create()){
      throw new CStringException("Error initializing event manager.");
    }

    // init archive library
    CDataArchive::Initialize("Temp");

    // init internlal timer
    if(!CTimer::Init()){
      throw new CStringException("Error initializing timer.");
    }

    // Init DirectDraw
    if(!CDirectDraw::Init(NULL, CDirectDraw::DM_NoChange)){
      throw new CStringException("Can't initialize DirectDraw.");
    }
    if(g_pDirectDraw->GetBPP() < 24){
      throw new CStringException("This program will run only if the display mode is 24 or 32 bit.");
    }

    // create primary surface
    m_DDPrimarySurface.SetBackBufferCount(0);
    if(!m_DDPrimarySurface.Create()){
      throw new CStringException("Can't create primary surface.");
    }
  }
  catch(CException *e){
    AbortApplication();
    e->ReportError();
    e->Delete();

    Splash.DestroyWindow();

    return FALSE;
  }

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CMapEditorDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CMapEditorView));
	AddDocTemplate(pDocTemplate);

	// Enable DDE Execute open
	EnableShellOpen();
	RegisterShellFileTypes(TRUE);

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

  if(cmdInfo.m_nShellCommand == CCommandLineInfo::FileNew){
    cmdInfo.m_nShellCommand = CCommandLineInfo::FileNothing;
    CString strFile;
    BOOL bDisableLastOpened = FALSE;
    strFile = GetProfileString("Recent File List", "File1");
    if(!strFile.IsEmpty()){
      try{
        CArchiveFile file;
        CDataArchive::GetRootArchive()->CreateFile(strFile);
      }
      catch(CException *e){
        e->Delete();
        bDisableLastOpened = TRUE;
        strFile = "(Last opened file not found)";
      }
    }

    CStartDlg dlg;
    dlg.m_bDisableLastOpened = bDisableLastOpened;
    if(bDisableLastOpened)
      dlg.m_dwWhatToDo = CStartDlg::NewMap;
    else
      dlg.m_dwWhatToDo = CStartDlg::LastOpened;
    dlg.m_strLastOpenedMap = strFile;

RedoStartDlg:;
    if(dlg.DoModal() != IDOK){
      return FALSE;
    }

    switch(dlg.m_dwWhatToDo){
    case CStartDlg::NewMap:
      cmdInfo.m_nShellCommand = CCommandLineInfo::FileNew;
      goto ProcessCmdLine;
    case CStartDlg::LastOpened:
      try{
          if(OpenDocumentFile(strFile) == NULL){
            cmdInfo.m_nShellCommand = CCommandLineInfo::FileNew;
            goto ProcessCmdLine;
          }
      }
      catch(CException *e){
        e->ReportError();
        e->Delete();
        cmdInfo.m_nShellCommand = CCommandLineInfo::FileNew;
        goto ProcessCmdLine;
      }
      break;
    case CStartDlg::OpenMap:
      {
        CFileDialog dlg(TRUE, "*.map", NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
          "Soubor mapy (*.map)|*.map|Všechny soubory (*.*)|*.*||", NULL);
        dlg.m_ofn.lpstrTitle = "Otevøít mapu";
        if(dlg.DoModal() != IDOK){
          goto RedoStartDlg;
        }
        try{
            if(OpenDocumentFile(dlg.GetPathName()) == NULL){
              cmdInfo.m_nShellCommand = CCommandLineInfo::FileNew;
              goto ProcessCmdLine;
            }
        }
        catch(CException *e){
          e->ReportError();
          e->Delete();
          cmdInfo.m_nShellCommand = CCommandLineInfo::FileNew;
          goto ProcessCmdLine;
        }
      }
      break;
    }
  }
ProcessCmdLine:
  Splash.DestroyWindow();

  // Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
    return FALSE;

	// The one and only window has been initialized, so show and update it.
	if(((CMainFrame *)m_pMainWnd)->m_bMaximized)
    m_pMainWnd->ShowWindow(SW_MAXIMIZE);
  else
    m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();

  ((CMainFrame *)m_pMainWnd)->OnNewMap(AppGetActiveMap());

	// Enable drag/drop open
	m_pMainWnd->DragAcceptFiles();

  // load states
  CMapexEdit::LoadState("MapexEdit");

  if(bLoadLast){
  }

  bMainFrameCreated = TRUE;

	return TRUE;
}

int CMapEditorApp::ExitInstance() 
{
  // save states
  CMapexEdit::SaveState("MapexEdit");

  try{

    // delete primary surface
    m_DDPrimarySurface.Delete();

    // close direct draw
    CDirectDraw::Close();

    // destroy the timer
    CTimer::Close();

    // close the archive library
    CDataArchive::Done();

    // destroy the event manager
    m_EventManager.Delete();

    // abort application
    AbortApplication(0);

    delete g_strStartupDirectory;
  }
  catch(CException *e){
    AbortApplication(1);
    e->ReportError();
    e->Delete();
  }
  	
	return CWinApp::ExitInstance();
}

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	CStatic	m_wndSplashBitmap;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	HBITMAP m_hBitmap;
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	DDX_Control(pDX, IDC_SPLASHBITMAP, m_wndSplashBitmap);
	//}}AFX_DATA_MAP
}

extern CString *g_strStartupDirectory;

BOOL CAboutDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
  CString strBmp;
  strBmp.LoadString(IDS_SPLASHBITMAP);
  strBmp = *g_strStartupDirectory + "\\" + strBmp;

  m_hBitmap = (HBITMAP)LoadImage(AfxGetInstanceHandle(), strBmp, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
  m_wndSplashBitmap.SetBitmap(m_hBitmap);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAboutDlg::OnDestroy() 
{
	CDialog::OnDestroy();

  if(m_hBitmap != NULL){
    DeleteObject(m_hBitmap);
    m_hBitmap = NULL;
  }
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void CMapEditorApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CMapEditorApp message handlers

BOOL CMapEditorApp::OnIdle(LONG lCount) 
{
  DWORD dwQuitParam;
	
  CWinApp::OnIdle(lCount);

  if(lCount > 10)
    m_EventManager.DoEventMsg(&dwQuitParam);

  return TRUE; // allways want more idle processing
}




CMapEditorDoc *AppGetActiveDocument()
{
  CMainFrame *pFrame = (CMainFrame *)(((CMapEditorApp *)AfxGetApp())->m_pMainWnd);
  return (CMapEditorDoc *)pFrame->GetActiveDocument();
}


CEMap *g_pActiveMap = NULL;

CEMap *AppGetActiveMap()
{
  return g_pActiveMap;
}

void AppSetActiveMap(CEMap *pMap)
{
  g_pActiveMap = pMap;
}
