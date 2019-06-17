// WindowsWindow.cpp : implementation file
//

#include "stdafx.h"
#include "WindowsWindow.h"

#include "..\Mouse\Mouse.h"
#include "Window.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWindowsWindow

CWindowsWindow *g_pMainWindow = NULL;

IMPLEMENT_DYNCREATE(CWindowsWindow, CMouseWindow)

CWindowsWindow::CWindowsWindow() :
  m_eventActive(FALSE, TRUE)
{
  m_bActive = FALSE;

  g_pMainWindow = this;
}

CWindowsWindow::~CWindowsWindow()
{
  g_pMainWindow = NULL;
}


#ifdef _DEBUG

void CWindowsWindow::AssertValid() const
{
  CMouseWindow::AssertValid();
}

void CWindowsWindow::Dump(CDumpContext &dc) const
{
  CMouseWindow::Dump(dc);
}

#endif

BEGIN_MESSAGE_MAP(CWindowsWindow, CMouseWindow)
	//{{AFX_MSG_MAP(CWindowsWindow)
	ON_WM_PAINT()
	ON_WM_ACTIVATE()
  ON_MESSAGE(WM_ENTERMENULOOP, OnEnterMenuLoop)
  ON_MESSAGE(WM_EXITSIZEMOVE, OnExitSizeMove)
  ON_MESSAGE(WM_EXITMENULOOP, OnExitMenuLoop)
  ON_MESSAGE(WM_ENTERSIZEMOVE, OnEnterSizeMove)
	ON_WM_SIZE()
	ON_WM_SYSCOMMAND()
	ON_WM_NCACTIVATE()
	ON_WM_NCPAINT()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWindowsWindow message handlers


void CWindowsWindow::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

  if((g_pDirectDraw == NULL) || (!g_pDirectDraw->ExclusiveMode())){
    CRect rc;
    GetClientRect(&rc);
    dc.FillSolidRect(&rc, RGB(0, 0, 0));
    return;
  }

  // we can redraw the application only when it's maximized
  // and active
  if(g_pDirectDraw->ExclusiveMode() && (::GetActiveWindow() != m_hWnd || IsIconic())) return;
  
  // now we have to redraw our application
  // so we will call redraw for the whole desktop window

  // we also have to hide the mouse and than show it again
  // this is because the mouse can be changed in some wierd state
  // some impossible images in its buffer and so on

  if(g_pMouse) g_pMouse->HideMouse();
  if(g_pDesktopWindow)
    g_pDesktopWindow->UpdateRect(NULL);
  if(g_pMouse) g_pMouse->ShowMouse();
	
	// Do not call CMouseWindow::OnPaint() for painting messages
}

void CWindowsWindow::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized) 
{
  CMouseWindow::OnActivate(nState, pWndOther, bMinimized);
  BOOL bActive;
  bActive = !(nState == WA_INACTIVE);
  if(IsIconic()) bActive = FALSE;

  SetActive(bActive);
}

LONG CWindowsWindow::OnEnterMenuLoop(UINT wParam, LONG lParam)
{
  SetActive(FALSE);

  return CMouseWindow::OnEnterMenuLoop(wParam, lParam);
}

LONG CWindowsWindow::OnEnterSizeMove(UINT wParam, LONG lParam)
{
  SetActive(FALSE);
  return CMouseWindow::OnEnterSizeMove(wParam, lParam);
}

LONG CWindowsWindow::OnExitMenuLoop(UINT wParam, LONG lParam)
{
  SetActive(!(::GetActiveWindow() != m_hWnd || IsIconic()));
  return CMouseWindow::OnExitMenuLoop(wParam, lParam);
}

LONG CWindowsWindow::OnExitSizeMove(UINT wParam, LONG lParam)
{
  SetActive(!(::GetActiveWindow() != m_hWnd || IsIconic()));
  return CMouseWindow::OnExitSizeMove(wParam, lParam);
}

BOOL CWindowsWindow::IsActive()
{
  BOOL bActive;
  ASSERT(this != NULL);
  VERIFY(m_lockActive.Lock());
  bActive = m_bActive;
  VERIFY(m_lockActive.Unlock());
  return bActive;
}

void CWindowsWindow::OnSize(UINT nType, int cx, int cy) 
{
  SetActive(!(::GetActiveWindow() != m_hWnd || IsIconic()));
	CMouseWindow::OnSize(nType, cx, cy);
}

extern BOOL g_bExclusive;
void CWindowsWindow::OnSysCommand(UINT nID, LPARAM lParam) 
{
  switch(nID & 0x0FFF0){
  case SC_KEYMENU:
  case SC_MOUSEMENU:
  case SC_SIZE:
    return;
  case SC_SCREENSAVE:
  case SC_MOVE:
    if(g_bExclusive) return;
  default:
    break;
  }
  
	CMouseWindow::OnSysCommand(nID, lParam);
}

void CWindowsWindow::LockActivation()
{
  VERIFY(m_lockActive.Lock());
}

void CWindowsWindow::UnlockActivation()
{ 
  VERIFY(m_lockActive.Unlock()); 
}

void CWindowsWindow::SetActive(BOOL bActive)
{
  VERIFY(m_lockActive.Lock());
  m_bActive = bActive;

  if(!m_bActive){
    if(g_pDirectDraw != NULL) g_pDirectDraw->Enable(FALSE);
  }
  else{
    if(g_pDirectDraw != NULL) g_pDirectDraw->Enable(TRUE);
  }

#ifdef _DEBUG
  if(m_bActive) TRACE("Window - Activated\n");
  else TRACE("Window - Deactivated\n");
#endif

  if(m_bActive){
    m_eventActive.SetEvent();
  }
  else{
    m_eventActive.ResetEvent();
  }

  VERIFY(m_lockActive.Unlock());
}

BOOL CWindowsWindow::OnNcActivate(BOOL bActive) 
{
  if((g_pDirectDraw != NULL) && (g_pDirectDraw->ExclusiveMode()))
    return TRUE;  
  return CMouseWindow::OnNcActivate(bActive);
}

void CWindowsWindow::OnNcPaint() 
{
  if((g_pDirectDraw != NULL) && (g_pDirectDraw->ExclusiveMode()))
    return;
  CMouseWindow::OnNcPaint();
}

BOOL CWindowsWindow::OnEraseBkgnd(CDC* pDC) 
{
//  CRect rcRect;

//  GetClientRect(&rcRect);
//  pDC->FillSolidRect(&rcRect, RGB(0, 0, 0));
  if((g_pDirectDraw != NULL) && (g_pDirectDraw->ExclusiveMode())) 
    return TRUE;
  return CMouseWindow::OnEraseBkgnd(pDC);
}
