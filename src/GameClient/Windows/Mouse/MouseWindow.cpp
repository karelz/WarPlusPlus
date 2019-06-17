// MouseWindow.cpp : implementation file
//

#include "stdafx.h"
#include "MouseWindow.h"
#include "Mouse.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define WM_DIMOUSEACQUIRE WM_USER + 0x100  // Our small message
#define DIMA_EXITMENULOOP 0  // The message was posted on Exit menu loop
#define DIMA_EXITSIZEMOVE 1  // The message was posted on Exit size move

/////////////////////////////////////////////////////////////////////////////
// CMouseWindow

IMPLEMENT_DYNAMIC(CMouseWindow, CKeyboardWindow);

CMouseWindow::CMouseWindow()
{
  m_pMouse = NULL;
  m_bActive = FALSE;
}

CMouseWindow::~CMouseWindow()
{
  if( m_pMouse != NULL )
    m_pMouse->m_pMouseWindow = NULL;
}


BEGIN_MESSAGE_MAP(CMouseWindow, CKeyboardWindow)
	//{{AFX_MSG_MAP(CMouseWindow)
	ON_WM_ACTIVATE()
  ON_MESSAGE(WM_DIMOUSEACQUIRE, OnDIMouseAcquire)
  ON_MESSAGE(WM_EXITSIZEMOVE, OnExitSizeMove)
  ON_MESSAGE(WM_EXITMENULOOP, OnExitMenuLoop)
  ON_MESSAGE(WM_ENTERSIZEMOVE, OnEnterSizeMove)
  ON_MESSAGE(WM_ENTERMENULOOP, OnEnterMenuLoop)
  ON_WM_SYSCOMMAND()
	ON_WM_SIZE()
  ON_WM_LBUTTONDOWN()
  ON_WM_LBUTTONUP()
  ON_WM_LBUTTONDBLCLK()
  ON_WM_RBUTTONDOWN()
  ON_WM_RBUTTONUP()
  ON_WM_RBUTTONDBLCLK()
  ON_WM_MOUSEMOVE()
	ON_WM_CAPTURECHANGED()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CMouseWindow message handlers

/////////////////////////////////////////////////////////////////////////////
// Debug methods

#ifdef _DEBUG

void CMouseWindow::AssertValid() const
{
  CKeyboardWindow::AssertValid();
}

void CMouseWindow::Dump(CDumpContext & dc) const
{
  CKeyboardWindow::Dump(dc);
  dc << "Mouse : " << m_pMouse << "\n";
  dc << "Active : " << m_bActive << "\n";
}

#endif


#include "Mouse.h"

// As the window is deactivated we unacquire the device
// and when it is reactivated we reacquire the device
void CMouseWindow::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized) 
{
  CKeyboardWindow::OnActivate(nState, pWndOther, bMinimized);
  if(m_pMouse == NULL) return;

  m_bActive = !(nState == WA_INACTIVE);

#ifdef _DEBUG
  if(m_bActive){
    TRACE("WM_ACTIVATE - activated\n");
  }
  else{
    TRACE("WM_ACTIVATE - deactivated\n");
  }
#endif


  if(IsIconic()) m_bActive = FALSE;
  SyncAcquire();
}

// We have to Unacquire the device if the user works with menus
// or if he resizes or moves our window
LONG CMouseWindow::OnEnterMenuLoop(UINT wParam, LONG lParam)
{
  m_bActive = FALSE;
  SyncAcquire();
  return CWnd::DefWindowProc(WM_ENTERMENULOOP, wParam, lParam);
}

LONG CMouseWindow::OnEnterSizeMove(UINT wParam, LONG lParam)
{
  m_bActive = FALSE;
  SyncAcquire();
  return CWnd::DefWindowProc(WM_ENTERSIZEMOVE, wParam, lParam);
}

// After the user ends his actions we can't reacquire the device
// imidietly because the menu is still visible
// so we send a small message to ourself and there we reacquire
// the device

// Also we can't reacquire it as it is
// if we are not the active window (can happen) it's illegal
// also if we are in iconic state (just taskbar button)
// we can't reacquire it -> it doesn't make sense in that case
LONG CMouseWindow::OnExitMenuLoop(UINT wParam, LONG lParam)
{
  m_bActive = !(::GetActiveWindow() != m_hWnd || IsIconic());
  PostMessage(WM_DIMOUSEACQUIRE, DIMA_EXITMENULOOP, 0L);
  return CWnd::DefWindowProc(WM_EXITMENULOOP, wParam, lParam);
}

LONG CMouseWindow::OnExitSizeMove(UINT wParam, LONG lParam)
{
  m_bActive = !(::GetActiveWindow() != m_hWnd || IsIconic());
  PostMessage(WM_DIMOUSEACQUIRE, DIMA_EXITSIZEMOVE, 0L);
  return CWnd::DefWindowProc(WM_EXITSIZEMOVE, wParam, lParam);
}

void CMouseWindow::OnSysCommand(UINT nID, LPARAM lParam) 
{
	CKeyboardWindow::OnSysCommand(nID, lParam);

  if(IsWindow(m_hWnd)){
    SyncAcquire();
  }
}

// If we get our internal message we can acquire the device
LONG CMouseWindow::OnDIMouseAcquire(UINT wParam, LONG lParam)
{
  SyncAcquire();
  return 0;
}

void CMouseWindow::SyncAcquire()
{
  if(!m_pMouse) return;
   
  if(m_pMouse->m_bExclusive){
    if(m_bActive){
      m_pMouse->m_MouseDevice.Acquire();
    }
    else{
      m_pMouse->m_MouseDevice.Unacquire();
    }
  }
}

void CMouseWindow::SendAcquire(BOOL bActive)
{
  m_bActive = bActive;
  PostMessage(WM_DIMOUSEACQUIRE, DIMA_EXITSIZEMOVE, 0L);
}

void CMouseWindow::OnSize(UINT nType, int cx, int cy) 
{
  m_bActive = !(::GetActiveWindow() != m_hWnd || IsIconic());
	CKeyboardWindow::OnSize(nType, cx, cy);
}


void CMouseWindow::OnMouseMove(UINT nFlags, CPoint point)
{
  if(m_pMouse){
    if(!m_pMouse->m_bExclusive){
      m_pMouse->m_MouseNotifier.InlayEvent(CMouse::E_MOVE, MOUSE_MAKEPOINT(point.x, point.y));
    }
  }
  CKeyboardWindow::OnMouseMove(nFlags, point);
}

void CMouseWindow::OnLButtonDown(UINT nFlags, CPoint point)
{
  if(m_pMouse){
    if(!m_pMouse->m_bExclusive){
      m_pMouse->m_MouseNotifier.InlayEvent(CMouse::E_LBUTTONDOWN, MOUSE_MAKEPOINT(point.x, point.y));
    }
  }
  CKeyboardWindow::OnLButtonDown(nFlags, point);
}

void CMouseWindow::OnLButtonUp(UINT nFlags, CPoint point)
{
  if(m_pMouse){
    if(!m_pMouse->m_bExclusive){
      m_pMouse->m_MouseNotifier.InlayEvent(CMouse::E_LBUTTONUP, MOUSE_MAKEPOINT(point.x, point.y));
    }
  }
  CKeyboardWindow::OnLButtonUp(nFlags, point);
}

void CMouseWindow::OnLButtonDblClk(UINT nFlags, CPoint point)
{
  if(m_pMouse){
    if(!m_pMouse->m_bExclusive){
      m_pMouse->m_MouseNotifier.InlayEvent(CMouse::E_LBUTTONDBLCLK, MOUSE_MAKEPOINT(point.x, point.y));
    }
  }
  CKeyboardWindow::OnLButtonDblClk(nFlags, point);
}

void CMouseWindow::OnRButtonDown(UINT nFlags, CPoint point)
{
  if(m_pMouse){
    if(!m_pMouse->m_bExclusive){
      m_pMouse->m_MouseNotifier.InlayEvent(CMouse::E_RBUTTONDOWN, MOUSE_MAKEPOINT(point.x, point.y));
    }
  }
  CKeyboardWindow::OnRButtonDown(nFlags, point);
}

void CMouseWindow::OnRButtonUp(UINT nFlags, CPoint point)
{
  if(m_pMouse){
    if(!m_pMouse->m_bExclusive){
      m_pMouse->m_MouseNotifier.InlayEvent(CMouse::E_RBUTTONUP, MOUSE_MAKEPOINT(point.x, point.y));
    }
  }
  CKeyboardWindow::OnRButtonUp(nFlags, point);
}

void CMouseWindow::OnRButtonDblClk(UINT nFlags, CPoint point)
{
  if(m_pMouse){
    if(!m_pMouse->m_bExclusive){
      m_pMouse->m_MouseNotifier.InlayEvent(CMouse::E_RBUTTONDBLCLK, MOUSE_MAKEPOINT(point.x, point.y));
    }
  }
  CKeyboardWindow::OnRButtonDblClk(nFlags, point);
}

void CMouseWindow::OnCaptureChanged(CWnd *pWnd) 
{
  if(pWnd != this){
    m_bActive = !(::GetActiveWindow() != m_hWnd || IsIconic());
    PostMessage(WM_DIMOUSEACQUIRE, DIMA_EXITSIZEMOVE, 0L);
  }
  
	CKeyboardWindow::OnCaptureChanged(pWnd);
}
