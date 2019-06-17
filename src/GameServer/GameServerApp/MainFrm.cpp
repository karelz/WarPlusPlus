// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "GameServerApp.h"

#include "GameServerAppDoc.h"
#include "..\GameServer\SFileManager.h"

#include "MainFrm.h"
#include "..\GameServer\SMap.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define WM_SHELLICONMESSAGE WM_USER + 2
#define WM_MAPOBSERVEREVENT WM_USER + 1

typedef struct _MY_NOTIFYICONDATAA {
        DWORD cbSize;
        HWND hWnd;
        UINT uID;
        UINT uFlags;
        UINT uCallbackMessage;
        HICON hIcon;
        CHAR   szTip[128];
        DWORD dwState;
        DWORD dwStateMask;
        CHAR   szInfo[256];
        union {
            UINT  uTimeout;
            UINT  uVersion;
        } DUMMYUNIONNAME;
        CHAR   szInfoTitle[64];
        DWORD dwInfoFlags;
} MY_NOTIFYICONDATAA, *PMY_NOTIFYICONDATAA;
typedef struct _MY_NOTIFYICONDATAW {
        DWORD cbSize;
        HWND hWnd;
        UINT uID;
        UINT uFlags;
        UINT uCallbackMessage;
        HICON hIcon;
        WCHAR  szTip[128];
        DWORD dwState;
        DWORD dwStateMask;
        WCHAR  szInfo[256];
        union {
            UINT  uTimeout;
            UINT  uVersion;
        } DUMMYUNIONNAME;
        WCHAR  szInfoTitle[64];
        DWORD dwInfoFlags;
} MY_NOTIFYICONDATAW, *PMY_NOTIFYICONDATAW;
#ifdef UNICODE
typedef MY_NOTIFYICONDATAW MY_NOTIFYICONDATA;
typedef PMY_NOTIFYICONDATAW PMY_NOTIFYICONDATA;
#else
typedef MY_NOTIFYICONDATAA MY_NOTIFYICONDATA;
typedef PMY_NOTIFYICONDATAA PMY_NOTIFYICONDATA;
#endif // UNICODE


/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
  ON_MESSAGE(WM_MAPOBSERVEREVENT, OnMapObserverEvent)
  ON_MESSAGE(WM_SHELLICONMESSAGE, OnShellIconMessage )
	ON_WM_SIZE()
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
  ID_SEPARATOR,           // Timeslice
  ID_SEPARATOR,           // Instruction counters
  ID_SEPARATOR,           // Game status
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
	m_bTrayIcon = false;
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}
  {
    m_wndToolBar.SetSizes(CSize(31, 29), CSize(24, 23));
    CBitmap bmpToolBar;
    bmpToolBar.LoadBitmap(IDB_MAINFRAME_TB_COLOR);
    m_imglsToolBarColor.Create(24, 23, ILC_COLOR24 | ILC_MASK, 0, 1);
    m_imglsToolBarColor.Add(&bmpToolBar, RGB(192, 192, 192));
    bmpToolBar.DeleteObject();
    bmpToolBar.LoadBitmap(IDB_MAINFRAME_TB_GRAY);
    m_imglsToolBarGray.Create(24, 23, ILC_COLOR24 | ILC_MASK, 0, 1);
    m_imglsToolBarGray.Add(&bmpToolBar, RGB(192, 192, 192));
    bmpToolBar.DeleteObject();
    bmpToolBar.LoadBitmap(IDB_MAINFRAME_TB_DISABLED);
    m_imglsToolBarDisabled.Create(24, 23, ILC_COLOR24 | ILC_MASK, 0, 1);
    m_imglsToolBarDisabled.Add(&bmpToolBar, RGB(192, 192, 192));
    bmpToolBar.DeleteObject();
    m_wndToolBar.GetToolBarCtrl().SetHotImageList(&m_imglsToolBarColor);
    m_wndToolBar.GetToolBarCtrl().SetImageList(&m_imglsToolBarGray);
    m_wndToolBar.GetToolBarCtrl().SetDisabledImageList(&m_imglsToolBarDisabled);
  }

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}
  m_wndStatusBar.SetPaneInfo(1, ID_SEPARATOR, SBPS_NORMAL, 100);
  m_wndStatusBar.SendMessage(SB_SETICON, 1, (DWORD)(HICON)::LoadImage(AfxGetInstanceHandle(),
      MAKEINTRESOURCE(IDI_TIMESLICE_ICON), IMAGE_ICON, 0, 0, 0));
  m_wndStatusBar.SetPaneInfo(3, ID_SEPARATOR, SBPS_NORMAL, 120);
  m_wndStatusBar.SetPaneInfo(2, ID_SEPARATOR, SBPS_NORMAL, 150);
  m_wndStatusBar.SendMessage ( SB_SETICON, 2, (DWORD)(HICON)::LoadImage ( AfxGetInstanceHandle(),
    MAKEINTRESOURCE(IDI_INSTRUCTIONCOUNTER_ICON), IMAGE_ICON, 0, 0, 0 ) );

	// TODO: Delete these three lines if you don't want the toolbar to
	//  be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

	return 0;
}

BOOL g_bWinVer50 = FALSE;

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;

  cs.style &= (~FWS_ADDTOTITLE);

  // Determine the version of OS
  OSVERSIONINFO ovi;
  ovi.dwOSVersionInfoSize = sizeof ( ovi );
  if ( !GetVersionEx ( &ovi ) )
  {
	  AfxMessageBox ( "Invalid OS Version.\nRequires Windows 95/NT 4.0 at least." );
	  return FALSE;
  }
  else
  {
	  if ( ovi.dwMajorVersion < 4 )
	  {
		  AfxMessageBox ( "Invalid OS Version.\nRequires Windows 95/NT 4.0 at least." );
		  return FALSE;
	  }

	  if ( ovi.dwMajorVersion >= 5 )
	  {
		  g_bWinVer50 = TRUE;
	  }
	  else
	  {
		  g_bWinVer50 = FALSE;
	  }
  }

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers

void CMainFrame::SetGameStatusPane(UINT nStringID, HICON hIcon)
{
  CString strText;
  strText.LoadString(nStringID);

  m_wndStatusBar.SetPaneText(3, strText);
  m_wndStatusBar.SendMessage(SB_SETICON, 3, (DWORD)hIcon);
}

void CMainFrame::SetTimeslicePane(DWORD dwTimeslice)
{
  CString strText;
  strText.Format("%d", dwTimeslice);

  if(dwTimeslice == 0)
    m_wndStatusBar.SetPaneText(1, "");
  else
    m_wndStatusBar.SetPaneText(1, strText);
}

CString PrintInt64ByLocale ( __int64 nNumber );

void CMainFrame::SetInstructionCountersPane ( __int64 dwLocal, __int64 dwGlobal )
{
  CString strText;
  strText.Format ( "%s/%s", PrintInt64ByLocale ( dwLocal ), PrintInt64ByLocale ( dwGlobal ) );
  
  m_wndStatusBar.SetPaneText ( 2, strText );
}

void CMainFrame::OnFileOpen() 
{
  // Display the open dialog
  CString strOpenFilter;
  strOpenFilter.LoadString(IDS_FILE_OPENFILTER);
  CString strTitle;
  strTitle.LoadString(IDS_FILE_OPENTITLE);
  CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY, strOpenFilter, this);
  dlg.m_ofn.lpstrTitle = strTitle;

  if(dlg.DoModal() != IDOK){
    return;
  }

  // Test returned filename
  {
    CString strFileName = dlg.GetPathName();

    // Get the extension
    CString strExt = strFileName.Mid(strFileName.ReverseFind('.') + 1);

    // Compare the extension with our extensions
    CString strTheMapExt;
    strTheMapExt.LoadString(IDS_FILE_THEMAPEXT);
    CString strSaveExt;
    strSaveExt.LoadString(IDS_FILE_SAVEEXT);
    if(strExt.CompareNoCase(strTheMapExt) == 0){
      // We've got TheMap file
      // Try to open it
      CFile file;
      CFileException e;
      if(!file.Open(strFileName, CFile::modeRead | CFile::shareDenyWrite, &e)){
        // Oops we can't open the file
        AfxMessageBox(IDS_FILE_CANTOPENFILE);
        return;
      }
    }
    else if(strExt.CompareNoCase(strSaveExt) == 0){
      // We've got the savegame
      // Try to open it
      CFile file;
      CFileException e;
      if(!file.Open(strFileName, CFile::modeRead | CFile::shareDenyWrite, &e)){
        // Can't open it -> try it as directory name
        CString strDirName;
        strDirName = strFileName.Left(strFileName.ReverseFind('.'));

        // Try to find the directory
        CFileFind find;
        find.FindFile(strDirName + "\\*.*");
        if(!find.FindNextFile()){
          // Oops we can't open the file
          AfxMessageBox(IDS_FILE_CANTOPENFILE);
          return;
        }

        // We can't use empty names
        if(strDirName.IsEmpty()){
          // Oops we can't open the file
          AfxMessageBox(IDS_FILE_CANTOPENFILE);
          return;
        }
      }
    }
    else{
      // Unknown extension
      AfxMessageBox(IDS_FILE_WRONGEXTENSION);
      return;
    }
  }

  // Open the document
  AfxGetApp()->OpenDocumentFile(dlg.GetPathName());
}


void CMainFrame::OnFileSave() 
{
	// First ask the user to specify the save game archive
  CString strExtension, strFilter, strTitle;
  strExtension.LoadString(IDS_FILE_SAVEEXTENSION);
  strFilter.LoadString(IDS_FILE_SAVEFILTER);
  strTitle.LoadString(IDS_FILE_SAVETITLE);
  CFileDialog dlg(FALSE, strExtension, NULL, OFN_HIDEREADONLY,
    strFilter, this);
  dlg.m_ofn.lpstrTitle = strTitle;

  if(dlg.DoModal() == IDOK){
    // Test if it exists

    try{
        {
            CDataArchive SaveArchive;
            SaveArchive.Create(dlg.GetPathName(), CArchiveFile::modeWrite, CDataArchiveInfo::archiveFile);
            // Oops it does exists -> ask the user to overwrite
            CString strAsk, strTitle;
            strAsk.Format(IDS_FILE_SAVE_OVERWRITEPROMPT, dlg.GetPathName());
            if(AfxMessageBox(strAsk, MB_YESNO | MB_ICONQUESTION) != IDYES){
                // Nothing to do, just exit
                return;
            }
            SaveArchive.Close();
        }

      // Delete the old savegame
      // !!!@@@### Lately rewrite it to delete the archive file
      if ( !::DeleteFile(dlg.GetPathName()) )
      {
        CString strError;
        strError.Format ( IDS_CANTOVERWRITESAVEGAME, dlg.GetPathName () );
        AfxMessageBox ( strError );
        return;
      }
    }
    catch(CException *e){
      e->Delete();
      // It doesn't exists -> OK
    }

    CDataArchive SaveArchive;
    // Now save the game
    try{
      SaveArchive.Create(dlg.GetPathName(), CArchiveFile::modeWrite | CArchiveFile::modeCreate, CDataArchiveInfo::archiveFile);
      ((CGameServerAppDoc *)GetActiveDocument())->SaveGame(SaveArchive);
    }
    catch(CException *e){
      // Some error while saving
      e->ReportError();
      e->Delete();
      return;
    }
  }
}

LRESULT CMainFrame::OnMapObserverEvent(WPARAM wParam, LPARAM lParam)
{
  return ((CGameServerAppDoc *)GetActiveDocument())->OnMapObserverEvent(wParam, lParam);
}

void CMainFrame::OnSize(UINT nType, int cx, int cy) 
{
	CFrameWnd::OnSize(nType, cx, cy);
	
  if ( nType == SIZE_MINIMIZED )
  {
	  ShowWindow ( SW_HIDE );
	  CreateTrayIcon ();
  }
  else 
  {
	  DeleteTrayIcon ();
  }
}

// Tray functions
void CMainFrame::CreateTrayIcon ()
{
	// Create the tray icon
	NOTIFYICONDATA nid;
	nid.cbSize = sizeof ( nid );
	nid.hWnd = GetSafeHwnd ();
	nid.uID = 1;
	nid.uFlags = NIF_MESSAGE | NIF_TIP | NIF_ICON;
	nid.hIcon = AfxGetApp ()->LoadIcon ( IDI_TRAY );
	nid.uCallbackMessage = WM_SHELLICONMESSAGE;
	CString strText;
	GetWindowText ( strText );
	strncpy ( nid.szTip, strText, 63 );
	nid.szTip [ 63 ] = 0;

	Shell_NotifyIcon ( NIM_ADD, &nid );

  m_bTrayIcon = true;
}

void CMainFrame::DeleteTrayIcon ()
{
    NOTIFYICONDATA nid;
    nid.cbSize = sizeof ( nid );
    nid.hWnd = GetSafeHwnd ();
    nid.uID = 1;
    nid.uFlags = 0;
    Shell_NotifyIcon ( NIM_DELETE, &nid );

    m_bTrayIcon = false;
}

void CMainFrame::OnClose() 
{
	DeleteTrayIcon ();
	
	CFrameWnd::OnClose();
}

LRESULT CMainFrame::OnShellIconMessage ( WPARAM wParam, LPARAM lParam )
{
	if ( wParam == 1 )
	{
		switch ( lParam )
		{
		case WM_LBUTTONDOWN:
			// Restore the window
			ShowWindow ( SW_SHOW );
			ShowWindow ( SW_RESTORE );
			break;
		}
	}
	return 0;
}

void CMainFrame::DisplayTaskBarNotification ( CString strInfo )
{
	if ( g_bWinVer50 && m_bTrayIcon)
	{
		MY_NOTIFYICONDATA nid;

		nid.cbSize = sizeof ( nid );
		nid.hWnd = GetSafeHwnd ();
		nid.uID = 1;
		nid.uFlags = 0x00000010;
		CString strTitle;
		strTitle.LoadString ( AFX_IDS_APP_TITLE );
		strncpy ( nid.szInfoTitle, strTitle, 63 );
		strncpy ( nid.szTip, strTitle, 63 );
		strncpy ( nid.szInfo, strInfo, 63 );
		nid.uTimeout = 5000;
		nid.dwInfoFlags = 0x00000001;

		Shell_NotifyIcon ( NIM_MODIFY, (NOTIFYICONDATA*)&nid );
  }
}
