// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CWindowsWindow)

BEGIN_MESSAGE_MAP(CMainFrame, CWindowsWindow)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_ERASEBKGND()
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_MESSAGE ( WM_MAINFRAME_SETFOCUSTOOURWINDOW, OnSetFocusToOurWindow )
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
  m_bExclusive = FALSE;
}

CMainFrame::~CMainFrame()
{
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CWindowsWindow::PreCreateWindow(cs) )
		return FALSE;

  if(m_bExclusive){
	  cs.style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
	  cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
    cs.dwExStyle |= WS_EX_WINDOWEDGE;
  }
  else{
    cs.style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
    cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
    cs.dwExStyle |= WS_EX_WINDOWEDGE;
  }
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CWindowsWindow::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CWindowsWindow::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers

BOOL CMainFrame::OnEraseBkgnd(CDC* pDC) 
{
	return CWindowsWindow::OnEraseBkgnd(pDC);
}

void CMainFrame::SetExclusive(BOOL bExclusive)
{
  m_bExclusive = bExclusive;
}

CMainFrame * g_pMainFrame = NULL;

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWindowsWindow::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	CRect rcWnd ( 0, 0, 1, 1 );
	m_wndExclusiveKeyboard.Create ( NULL, "Exclusive Keyboard Window", WS_CHILD, rcWnd, this, 1, NULL);
	m_wndExclusiveKeyboard.ShowWindow ( SW_HIDE );
	m_wndExclusiveKeyboard.SetSendToWindow ( this );
	
	g_pMainFrame = this;

	return 0;
}

void CMainFrame::OnClose() 
{
	m_wndExclusiveKeyboard.DestroyWindow ();

	g_pMainFrame = NULL;
	
	CWindowsWindow::OnClose();
}

LRESULT CMainFrame::OnSetFocusToOurWindow ( WPARAM wParam, LPARAM lParam )
{
  if ( this->IsActive () )
	  SetFocusToOurWindow ();
	return 0;
}
