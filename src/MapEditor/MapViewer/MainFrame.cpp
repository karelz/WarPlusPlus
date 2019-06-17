// MainFrame.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "MapViewer.h"

#include "MainFrame.h"

#include "MapViewerDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_SEPARATOR,           // Position of the map rectangle
	ID_SEPARATOR,           // Position of mouse on the map
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
	
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}
	m_wndStatusBar.SetPaneInfo(1, ID_SEPARATOR, 0, 130);
	m_wndStatusBar.SetPaneInfo(2, ID_SEPARATOR, 0, 80);

	// TODO: Delete these three lines if you don't want the toolbar to
	//  be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CMDIFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CMDIFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers


void CMainFrame::SetMapPositionPane(int nX, int nY)
{
	CString str;
	str.Format("%d, %d", nX, nY);
	m_wndStatusBar.SetPaneText(2, str);
}

void CMainFrame::SetMapPositionPaneClear()
{
	m_wndStatusBar.SetPaneText(2, "");
}

void CMainFrame::SetMapPositionsPane(int nLeftTopX, int nLeftTopY, int nRightBottomX, 
	int nRightBottomY)
{
	CString str;
	str.Format("( %d, %d ) - ( %d, %d )", nLeftTopX, nLeftTopY, nRightBottomX, 
		nRightBottomY);
	m_wndStatusBar.SetPaneText(1, str);
}

void CMainFrame::SetMapPositionsPaneClear()
{
	m_wndStatusBar.SetPaneText(1, "");
}

void CMainFrame::OnTimer(UINT nIDEvent) 
{
	CMapViewerDoc *pThis = (CMapViewerDoc *)nIDEvent;

	if ( memcmp ( pThis->m_pMapDescription, &pThis->m_sMapDescription, 
		sizeof ( SMapDescription ) ) != 0 )
	{	// zmìnil se popis mapy
		VERIFY ( theApp.m_mutexSharedMemoryLock.Lock () );
		pThis->m_sMapDescription = *pThis->m_pMapDescription;
		VERIFY ( theApp.m_mutexSharedMemoryLock.Unlock () );

		pThis->UpdateAllViews ( NULL, 1 );
	}

//	CMDIFrameWnd::OnTimer(nIDEvent);
}
