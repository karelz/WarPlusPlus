// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "AnimationEditor.h"

#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CMainFrame * g_pMainFrame = NULL;

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_COMMAND(ID_VIEW_ANIMATIONTOOLBAR, OnViewAnimationToolbar)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ANIMATIONTOOLBAR, OnUpdateViewAnimationToolbar)
	ON_WM_CLOSE()
	ON_UPDATE_COMMAND_UI(ID_VIEW_COLORINGTOOLBAR, OnUpdateViewColoringToolbar)
	ON_COMMAND(ID_VIEW_COLORINGTOOLBAR, OnViewColoringToolbar)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
  g_pMainFrame = this;

	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}
  m_wndToolBar.SetWindowText ( "General" );

	if (!m_wndAnimationToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_FIXED, CRect (0, 0, 0, 0), IDR_ANIMATION) ||
		!m_wndAnimationToolBar.LoadToolBar(IDR_ANIMATION))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}
  m_wndAnimationToolBar.SetWindowText ( "Animation" );

  // Create the current frame edit box
  {
    m_wndAnimationToolBar.SetButtonInfo ( m_wndAnimationToolBar.CommandToIndex ( ID_ANIMATION_FRAME ),
      ID_ANIMATION_FRAME, 0, 40 );
    m_cCurrentFrameFont.CreatePointFont ( 85, "Tahoma", NULL );
    CRect rcButton;
    m_wndAnimationToolBar.GetItemRect ( m_wndAnimationToolBar.CommandToIndex ( ID_ANIMATION_FRAME ), &rcButton );
    rcButton.top += ( rcButton.Height () - 14 ) / 2;
    rcButton.bottom = rcButton.top + 14;
    rcButton.left += 2; rcButton.right -= 2;
    m_wndCurrentFrame.Create ( WS_VISIBLE | WS_CHILD | ES_RIGHT , rcButton, &m_wndAnimationToolBar, ID_ANIMATION_FRAME );
    m_wndCurrentFrame.ShowWindow ( SW_SHOW );
    m_wndAnimationToolBar.GetDlgItem ( ID_ANIMATION_FRAME )->SetFont ( &m_cCurrentFrameFont );

    m_wndAnimationToolBar.SetButtonInfo ( m_wndAnimationToolBar.CommandToIndex ( ID_ANIMATION_DELAY ),
      ID_ANIMATION_DELAY, TBBS_SEPARATOR, 40 );
    m_wndAnimationToolBar.GetItemRect ( m_wndAnimationToolBar.CommandToIndex ( ID_ANIMATION_DELAY ), &rcButton );
    m_wndAnimationDelayUnder.Create ( "", WS_VISIBLE | WS_CHILD, rcButton, &m_wndAnimationToolBar );
    rcButton.top += ( rcButton.Height () - 14 ) / 2;
    rcButton.bottom = rcButton.top + 14;
    rcButton.left += 2; rcButton.right -= 2;
    m_wndAnimationDelay.Create ( WS_VISIBLE | WS_CHILD | ES_RIGHT , rcButton, &m_wndAnimationToolBar, ID_ANIMATION_DELAY );
    m_wndAnimationDelay.ShowWindow ( SW_SHOW );
    m_wndAnimationDelay.SetFont ( &m_cCurrentFrameFont );
  }

	if (!m_wndColoringToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_FIXED, CRect (0, 0, 0, 0), IDR_COLORING) ||
		!m_wndColoringToolBar.LoadToolBar(IDR_COLORING))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}
  m_wndColoringToolBar.SetWindowText ( "Coloring" );
  // Create the coloring controls
  {
    CRect rcButton;
    TBBUTTONINFO bi;
    bi.cbSize = sizeof ( bi );
    bi.dwMask = TBIF_SIZE | TBIF_STATE;
    bi.cx = 50;
    bi.fsState = 0;

    m_wndColoringToolBar.SetButtonInfo ( m_wndColoringToolBar.CommandToIndex ( ID_COLORING_HUECAPTION ),
      ID_COLORING_HUECAPTION, TBBS_SEPARATOR, 25 );
    m_wndColoringToolBar.GetItemRect ( m_wndColoringToolBar.CommandToIndex ( ID_COLORING_HUECAPTION ), &rcButton );
    rcButton.left += 3;
    m_wndHueCaption.Create ( "Hue:", WS_VISIBLE | WS_CHILD | SS_CENTERIMAGE , rcButton, &m_wndColoringToolBar, ID_COLORING_HUECAPTION );
    m_wndHueCaption.ShowWindow ( SW_SHOW );
    m_wndHueCaption.SetFont ( &m_cCurrentFrameFont );

    m_wndColoringToolBar.SetButtonInfo ( m_wndColoringToolBar.CommandToIndex ( ID_COLORING_HUEVALUE ),
      ID_COLORING_HUEVALUE, TBBS_SEPARATOR, 40 );
    m_wndColoringToolBar.GetItemRect ( m_wndColoringToolBar.CommandToIndex ( ID_COLORING_HUEVALUE ), &rcButton );
    m_wndHueValueUnder.Create ( "", WS_VISIBLE | WS_CHILD, rcButton, &m_wndColoringToolBar );
    rcButton.top += ( rcButton.Height () - 14 ) / 2;
    rcButton.bottom = rcButton.top + 14;
    rcButton.left += 2; rcButton.right -= 2;
    m_wndHueValue.Create ( WS_VISIBLE | WS_CHILD | ES_RIGHT , rcButton, &m_wndColoringToolBar, ID_COLORING_HUEVALUE );
    m_wndHueValue.ShowWindow ( SW_SHOW );
    m_wndHueValue.SetFont ( &m_cCurrentFrameFont );


    m_wndColoringToolBar.SetButtonInfo ( m_wndColoringToolBar.CommandToIndex ( ID_COLORING_SATCAPTION ),
      ID_COLORING_SATCAPTION, TBBS_SEPARATOR, 25 );
    m_wndColoringToolBar.GetItemRect ( m_wndColoringToolBar.CommandToIndex ( ID_COLORING_SATCAPTION ), &rcButton );
    rcButton.left += 3;
    m_wndSatCaption.Create ( "Sat:", WS_VISIBLE | WS_CHILD | SS_CENTERIMAGE , rcButton, &m_wndColoringToolBar, ID_COLORING_SATCAPTION );
    m_wndSatCaption.ShowWindow ( SW_SHOW );
    m_wndSatCaption.SetFont ( &m_cCurrentFrameFont );

    m_wndColoringToolBar.SetButtonInfo ( m_wndColoringToolBar.CommandToIndex ( ID_COLORING_SATVALUE ),
      ID_COLORING_SATVALUE, TBBS_SEPARATOR, 40 );
    m_wndColoringToolBar.GetItemRect ( m_wndColoringToolBar.CommandToIndex ( ID_COLORING_SATVALUE ), &rcButton );
    m_wndSatValueUnder.Create ( "", WS_VISIBLE | WS_CHILD, rcButton, &m_wndColoringToolBar );
    rcButton.top += ( rcButton.Height () - 14 ) / 2;
    rcButton.bottom = rcButton.top + 14;
    rcButton.left += 2; rcButton.right -= 2;
    m_wndSatValue.Create ( WS_VISIBLE | WS_CHILD | ES_RIGHT , rcButton, &m_wndColoringToolBar, ID_COLORING_SATVALUE );
    m_wndSatValue.ShowWindow ( SW_SHOW );
    m_wndSatValue.SetFont ( &m_cCurrentFrameFont );


    m_wndColoringToolBar.SetButtonInfo ( m_wndColoringToolBar.CommandToIndex ( ID_COLORING_VALCAPTION ),
      ID_COLORING_VALCAPTION, TBBS_SEPARATOR, 25 );
    m_wndColoringToolBar.GetItemRect ( m_wndColoringToolBar.CommandToIndex ( ID_COLORING_VALCAPTION ), &rcButton );
    rcButton.left += 3;
    m_wndValCaption.Create ( "Val:", WS_VISIBLE | WS_CHILD | SS_CENTERIMAGE , rcButton, &m_wndColoringToolBar, ID_COLORING_VALCAPTION );
    m_wndValCaption.ShowWindow ( SW_SHOW );
    m_wndValCaption.SetFont ( &m_cCurrentFrameFont );

    m_wndColoringToolBar.SetButtonInfo ( m_wndColoringToolBar.CommandToIndex ( ID_COLORING_VALVALUE ),
      ID_COLORING_VALVALUE, TBBS_SEPARATOR, 40 );
    m_wndColoringToolBar.GetItemRect ( m_wndColoringToolBar.CommandToIndex ( ID_COLORING_VALVALUE ), &rcButton );
    m_wndValValueUnder.Create ( "", WS_VISIBLE | WS_CHILD, rcButton, &m_wndColoringToolBar );
    rcButton.top += ( rcButton.Height () - 14 ) / 2;
    rcButton.bottom = rcButton.top + 14;
    rcButton.left += 2; rcButton.right -= 2;
    m_wndValValue.Create ( WS_VISIBLE | WS_CHILD | ES_RIGHT , rcButton, &m_wndColoringToolBar, ID_COLORING_VALVALUE );
    m_wndValValue.ShowWindow ( SW_SHOW );
    m_wndValValue.SetFont ( &m_cCurrentFrameFont );

    m_wndColoringToolBar.SetButtonInfo ( m_wndColoringToolBar.CommandToIndex ( ID_COLORING_SOURCECOLOR ),
      ID_COLORING_SOURCECOLOR, TBBS_SEPARATOR, 25 );
    m_wndColoringToolBar.GetItemRect ( m_wndColoringToolBar.CommandToIndex ( ID_COLORING_SOURCECOLOR ), &rcButton );
    rcButton.top += 2;
    rcButton.bottom -= 2;
    rcButton.left += 1;
    rcButton.right -= 2;
    m_wndSourceColor.Create ( ID_COLORING_SOURCECOLOR, WS_VISIBLE | WS_CHILD, rcButton, &m_wndColoringToolBar, this );
  }

	if (!m_wndFrameToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_FIXED, CRect (0, 0, 0, 0), IDR_FRAME) ||
		!m_wndFrameToolBar.LoadToolBar(IDR_FRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}
  m_wndFrameToolBar.SetWindowText ( "Frame" );

  {
    CRect rcButton;
    m_wndFrameToolBar.SetButtonInfo ( m_wndFrameToolBar.CommandToIndex ( ID_FRAME_DELAY ),
      ID_FRAME_DELAY, TBBS_SEPARATOR, 40 );
    m_wndFrameToolBar.GetItemRect ( m_wndFrameToolBar.CommandToIndex ( ID_FRAME_DELAY ), &rcButton );
    m_wndFrameDelayUnder.Create ( "", WS_VISIBLE | WS_CHILD, rcButton, &m_wndFrameToolBar );
    rcButton.top += ( rcButton.Height () - 14 ) / 2;
    rcButton.bottom = rcButton.top + 14;
    rcButton.left += 2; rcButton.right -= 2;
    m_wndFrameDelay.Create ( WS_VISIBLE | WS_CHILD | ES_RIGHT , rcButton, &m_wndFrameToolBar, ID_FRAME_DELAY );
    m_wndFrameDelay.ShowWindow ( SW_SHOW );
    m_wndFrameDelay.SetFont ( &m_cCurrentFrameFont );
  }

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	m_wndFrameToolBar.EnableDocking ( CBRS_ALIGN_TOP | CBRS_ALIGN_BOTTOM );
  m_wndAnimationToolBar.EnableDocking(CBRS_ALIGN_TOP | CBRS_ALIGN_BOTTOM);
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
  m_wndColoringToolBar.EnableDocking ( CBRS_ALIGN_TOP | CBRS_ALIGN_BOTTOM);
	EnableDocking(CBRS_ALIGN_ANY);
  DockControlBar ( &m_wndAnimationToolBar );
	DockControlBar(&m_wndToolBar);
  DockControlBar ( &m_wndFrameToolBar );
  DockControlBar ( &m_wndColoringToolBar );

	CFrameWnd::OnCreate(lpCreateStruct);

  LoadBarState("Layout\\MainFrame");
  {
    CWinApp *pApp = AfxGetApp ();
    CRect rcMainFrame;
    GetWindowRect ( &rcMainFrame );
    rcMainFrame.left = pApp->GetProfileInt ( "Layout\\MainFrame", "WindowPositionLeft", rcMainFrame.left );
    rcMainFrame.top = pApp->GetProfileInt ( "Layout\\MainFrame", "WindowPositionTop", rcMainFrame.top );
    rcMainFrame.right = pApp->GetProfileInt ( "Layout\\MainFrame", "WindowPositionRight", rcMainFrame.right );
    rcMainFrame.bottom = pApp->GetProfileInt ( "Layout\\MainFrame", "WindowPositionBottom", rcMainFrame.bottom );
    SetWindowPos ( NULL, rcMainFrame.left, rcMainFrame.top, rcMainFrame.Width (), rcMainFrame.Height (), SWP_NOZORDER );
  }

  return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;

  cs.style &= ~FWS_PREFIXTITLE;

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

void CMainFrame::OnViewAnimationToolbar() 
{
  if ( m_wndAnimationToolBar.IsWindowVisible () )
    ShowControlBar ( &m_wndAnimationToolBar, FALSE, FALSE );
  else
    ShowControlBar ( &m_wndAnimationToolBar, TRUE, FALSE );
}

void CMainFrame::OnUpdateViewAnimationToolbar(CCmdUI* pCmdUI) 
{
  pCmdUI->SetCheck ( (m_wndAnimationToolBar.IsWindowVisible ()) ? 1 : 0 );	
}

DWORD CMainFrame::GetCurrentFrameFromControl ()
{
  CString strText;
  m_wndCurrentFrame.GetWindowText ( strText );
  return (DWORD)atol ( strText );
}

void CMainFrame::SetCurrentFrameForControl ( DWORD dwCurrentFrame )
{
  CString strText;
  strText.Format ( "%d", dwCurrentFrame );
  m_wndCurrentFrame.SetWindowText ( strText );
}

void CMainFrame::OnClose() 
{
  {
    CRect rcMainFrame;
    GetWindowRect ( &rcMainFrame );
    CWinApp * pApp = AfxGetApp ();
    pApp->WriteProfileInt ( "Layout\\MainFrame", "WindowPositionLeft", rcMainFrame.left );
    pApp->WriteProfileInt ( "Layout\\MainFrame", "WindowPositionTop", rcMainFrame.top );
    pApp->WriteProfileInt ( "Layout\\MainFrame", "WindowPositionRight", rcMainFrame.right );
    pApp->WriteProfileInt ( "Layout\\MainFrame", "WindowPositionBottom", rcMainFrame.bottom );
  }

  SaveBarState("Layout\\MainFrame");
	
	CFrameWnd::OnClose();
}

void CMainFrame::OnUpdateViewColoringToolbar(CCmdUI* pCmdUI) 
{
  pCmdUI->SetCheck ( (m_wndColoringToolBar.IsWindowVisible ()) ? 1 : 0 );	
}

void CMainFrame::OnViewColoringToolbar() 
{
  if ( m_wndColoringToolBar.IsWindowVisible () )
    ShowControlBar ( &m_wndColoringToolBar, FALSE, FALSE );
  else
    ShowControlBar ( &m_wndColoringToolBar, TRUE, FALSE );
}
