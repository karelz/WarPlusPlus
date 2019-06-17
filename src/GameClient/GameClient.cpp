// GameClient.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "GameClient.h"

#include "MainFrm.h"

#include "ScriptEditor\HelpBrowser\HelpBrowserWindow.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CString g_strStartDirectory;

#define HELP_WIDTH 500
#define HELP_HEIGHT 2000
CHelpBrowserWindow *g_pHelpBrowserWindow = NULL;

#ifdef _DEBUG
BOOL g_bTraceNetwork = FALSE;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGameClientApp

BEGIN_MESSAGE_MAP(CGameClientApp, CWinApp)
	//{{AFX_MSG_MAP(CGameClientApp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGameClientApp construction

CGameClientApp::CGameClientApp()
{
  m_pApplication = NULL;
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CGameClientApp object

CGameClientApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CGameClientApp initialization

class CGameClientCommandLineInfo : public CCommandLineInfo
{
public:
  CGameClientCommandLineInfo(){ m_bExclusiveMode = TRUE; }

  virtual void ParseParam(LPCTSTR lpszParam, BOOL bFlag, BOOL bLast);

  BOOL m_bExclusiveMode;
};

void CGameClientCommandLineInfo::ParseParam(LPCSTR lpszParam, BOOL bFlag, BOOL bLast)
{
  if(bFlag){
    CString str;
    str = "Exclusive";
    if(str.CompareNoCase(lpszParam) == 0){
      m_bExclusiveMode = TRUE;
    }

    str = "NonExclusive";
    if(str.CompareNoCase(lpszParam) == 0){
      m_bExclusiveMode = FALSE;
    }

    str = "Debug";
    if(str.CompareNoCase(lpszParam) == 0){
      m_bExclusiveMode = FALSE;
    }

#ifdef _DEBUG
    str = "TraceNetwork";
    if(str.CompareNoCase(lpszParam) == 0){
      g_bTraceNetwork = TRUE;
    }
#endif
  }
}

BOOL g_bExclusive = FALSE;

BOOL CGameClientApp::InitInstance()
{
  TRACE("GameClient loaded.\n");

	AfxEnableControlContainer();

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

  SetRegistryKey(_T("Strategy Team"));

  CGameClientCommandLineInfo CmdInfo;
  ParseCommandLine(CmdInfo);

  g_bExclusive = CmdInfo.m_bExclusiveMode;

  if(g_bExclusive) ShowCursor(FALSE);

	// To create the main window, this code creates a new frame window
	// object and then sets it as the application's main window object.

	CMainFrame* pFrame = new CMainFrame;
	m_pMainWnd = pFrame;
  pFrame->SetExclusive(g_bExclusive);

	// create and load the frame with its resources
  HICON hIcon = LoadIcon(IDR_MAINFRAME);
  pFrame->Create(AfxRegisterWndClass(CS_DBLCLKS, LoadStandardCursor(IDC_ARROW), NULL, hIcon), "GameClient", WS_OVERLAPPED);

  pFrame->ShowWindow(SW_SHOW);
  TRACE("Frame shown.\n");
  pFrame->UpdateWindow();
  TRACE("Frame updated.\n");

  {
    char txt[1024];
    GetCurrentDirectory(1024, txt);
    CString str = txt;
    str.Replace('\\','/');
    str = "file:///" + str;
    str += "/../Help/obsah.html";
    CRect rcHelp(0, 0, HELP_WIDTH, HELP_HEIGHT);
    g_pHelpBrowserWindow = new CHelpBrowserWindow();
    g_pHelpBrowserWindow->Create(NULL, "HelpBrowserWindow", WS_OVERLAPPED, rcHelp, pFrame, 0);
    g_pHelpBrowserWindow->SetHomeURL(str);
  }

  try{

  m_pApplication = new CGameClientApplication();

  if(!m_pApplication->Init(g_bExclusive, pFrame)) return FALSE;

  m_pApplication->Start();

  }
  catch(CException *e){
    e->ReportError();
    e->Delete();
    return FALSE;
  }

	return TRUE;
}

int CGameClientApp::ExitInstance() 
{
  if(m_pApplication != NULL){
    m_pApplication->Stop();

    m_pApplication->Close();

    delete m_pApplication;
    m_pApplication = NULL;
  }

  if(g_bExclusive) ShowCursor(TRUE);

  if(g_pHelpBrowserWindow != NULL){
    delete g_pHelpBrowserWindow;
    g_pHelpBrowserWindow = NULL;
  }

	return CWinApp::ExitInstance();
}

/////////////////////////////////////////////////////////////////////////////
// CGameClientApp message handlers
