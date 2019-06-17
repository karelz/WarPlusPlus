// EventWindow.cpp : implementation file
//

#include "stdafx.h"
#include "EventWindow.h"
#include "EventManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEventWindow

IMPLEMENT_DYNAMIC(CEventWindow, CFrameWnd)

CEventWindow::CEventWindow()
{
}

CEventWindow::~CEventWindow()
{
}


BEGIN_MESSAGE_MAP(CEventWindow, CFrameWnd)
	//{{AFX_MSG_MAP(CEventWindow)
	ON_WM_ACTIVATE()
	ON_WM_CLOSE()
  ON_MESSAGE(WM_CLOSEWINDOW, OnCloseWindow)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////
// debug methods

#ifdef _DEBUG

void CEventWindow::AssertValid() const
{
  CFrameWnd::AssertValid();
}

void CEventWindow::Dump(CDumpContext &dc) const
{
  CFrameWnd::Dump(dc);
}

#endif

/////////////////////////////////////////////////////////////////////////////
// CEventWindow message handlers


void CEventWindow::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized) 
{
	CFrameWnd::OnActivate(nState, pWndOther, bMinimized);

  if((nState == WA_ACTIVE) || (nState == WA_CLICKACTIVE)){
  }
  else{
  }  
}

void CEventWindow::OnClose() 
{
  if(g_pEventManager == NULL){
    CFrameWnd::OnClose();
    return;
  }
  g_pEventManager->InlayQuitEvent(CEventManager::Quit_OnClose);
  CFrameWnd::OnClose();
}

// this message do just the same as WM_CLOSE
// because we use the WM_CLOSE for other purposes
LONG CEventWindow::OnCloseWindow(UINT wParam, LONG lParam)
{
  TRACE("OnCloseWindow called.\n");
  CFrameWnd::OnClose();

  return 0;
}

