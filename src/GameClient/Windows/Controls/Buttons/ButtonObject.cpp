// ButtonObject.cpp: implementation of the CButtonObject class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ButtonObject.h"
#include "..\..\Mouse.h"
#include "..\..\Timer.h"
#include "..\..\Keyboard.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CButtonObject, CWindow)

BEGIN_OBSERVER_MAP(CButtonObject, CWindow)
  BEGIN_MOUSE()
    ON_MOUSEMOVE()
    ON_MOUSELEAVE()
    ON_LBUTTONDOWN()
    ON_LBUTTONUP()
  END_MOUSE()

  BEGIN_KEYBOARD()
    ON_KEYDOWN()
    ON_KEYUP()
    ON_KEYACCELERATOR()
  END_KEYBOARD()

  BEGIN_TIMER()
    ON_TIMETICK()
  END_TIMER()
END_OBSERVER_MAP(CButtonObject, CWindow)

DWORD CButtonObject::RepeateDelay = 100;
DWORD CButtonObject::RepeateBeginDelay = 250;

CButtonObject::CButtonObject()
{
  m_bMouseActive = FALSE;
  m_bMouseDrag = FALSE;
  m_bPressed = FALSE;
  m_bRepeate = FALSE;
  m_rcSensitiveRectangle.SetRect ( 0, 0, 0, 0 );
}

CButtonObject::~CButtonObject()
{
}

#ifdef _DEBUG

void CButtonObject::AssertValid() const
{
  CObject::AssertValid();
}

void CButtonObject::Dump(CDumpContext &dc) const
{
  CObject::Dump(dc);
}

#endif


void CButtonObject::OnMouseActivate()
{
  OnLayoutChange();
}

void CButtonObject::OnMouseDeactivate()
{
  OnLayoutChange();
}

void CButtonObject::OnPress()
{
  InlayEvent(E_PRESSED, 0);
}

void CButtonObject::OnRelease()
{
  InlayEvent(E_RELEASED, 0);
}

BOOL CButtonObject::Create(CRect *pRect, CWindow *pParent, BOOL bRepeate)
{
  if(!CWindow::Create(pRect, pParent)) return FALSE;

  m_bRepeate = bRepeate;
  
  return TRUE;
}

void CButtonObject::Delete()
{
  m_bMouseActive = FALSE;
  m_bMouseDrag = FALSE;
  m_bPressed = FALSE;

  if(g_pKeyAccelerators) g_pKeyAccelerators->Disconnect(this);

  // disconnect us from timer
  // inly the repeated buttons can be connected to timer
  // and even them maybe are not -> no one touched them
  // so the notifier will take care about them -> it just
  // won't be able to find the connection -> do nothing (hopefully)
  if(m_bRepeate){
    if(g_pTimer) g_pTimer->Disconnect(this);
  }
  CWindow::Delete();
}

void CButtonObject::OnMouseMove(CPoint point)
{
  if(m_bMouseDrag){
    // if the mouse is above push us
    if(IsMouseAbove(point)){
      if(!m_bPressed){
        m_bPressed = TRUE;
        OnLayoutPress();
      }
    }
    else{
      // if we were pressed -> release
      if(m_bPressed){
        m_bPressed = FALSE;
        OnLayoutRelease();
      }
    }
  }
  else{
    // the mouse entered this window
    if(!m_bMouseActive){
      m_bMouseActive = TRUE;
      OnMouseActivate();
    }
  }
}

void CButtonObject::OnMouseLeave()
{
  if(m_bMouseActive){
    m_bMouseActive = FALSE;
    OnMouseDeactivate();
  }
}

void CButtonObject::OnLButtonDown(CPoint point)
{
  // if its outside -> don't care
  if(!GetClientRect()->PtInRect(point)) return;

  if(!m_bMouseDrag){
    // if we are not active -> do it
    if(!m_bMouseActive){
      m_bMouseActive = TRUE;
    }

    // capture the mouse for draging
    m_bMouseDrag = TRUE;
    SetCapture();
    if(m_bRepeate){
      // start the repeate timer
      g_pTimer->Connect(this, RepeateBeginDelay);
    }

    // press us
    m_bPressed = TRUE;
    OnLayoutPress();

    OnPress();
  }
}

void CButtonObject::OnLButtonUp(CPoint point)
{
  // is this event for us ?
  if(!m_bMouseDrag) return;

  // close the draging
  m_bMouseDrag = FALSE;
  ReleaseCapture();
  // stop the repeate timer
  if(m_bRepeate) g_pTimer->Disconnect(this);

  if(IsMouseAbove(point)){
    // if we were pressed -> release us
    if(m_bPressed){
      m_bPressed = FALSE;
      OnLayoutRelease();

      OnRelease();
    }
  }
  else{
    // release us
    if(m_bPressed){
      m_bPressed = FALSE;
      OnLayoutRelease();
      
      // deactivate us
      m_bMouseActive = FALSE;
    }
    else{
      // deactivate us
      if(m_bMouseActive){
        m_bMouseActive = FALSE;
        OnMouseDeactivate();
      }
    }
  }
}

BOOL CButtonObject::OnActivate()
{
  CWindow::OnActivate();

  if(IsMouseAbove(g_pMouse->GetPosition())){
    m_bMouseActive = TRUE;
    OnMouseActivate();
  }

  return TRUE;
}

void CButtonObject::OnDeactivate()
{
  if(m_bMouseDrag){
    m_bMouseDrag = FALSE;
    ReleaseCapture();
    if(m_bRepeate) g_pTimer->Disconnect(this);
  }

/*  if(m_bPressed){
    m_bPressed = FALSE;
    OnLayoutRelease();
    m_bMouseActive = FALSE;
  }
  else{
    if(m_bMouseActive){
      m_bMouseActive = FALSE;
      OnMouseDeactivate();
    }
  }*/

  CWindow::OnDeactivate();
}

void CButtonObject::OnLayoutPress()
{
  OnLayoutChange();
}

void CButtonObject::OnLayoutRelease()
{
  OnLayoutChange();
}

void CButtonObject::OnTimeTick(DWORD dwTime)
{
  // repeate the press if the mouse is on the button
  if(m_bPressed){
    OnPress();
    // set the new repeate time out
    g_pTimer->SetElapseTime(this, RepeateDelay);
  }
}

void CButtonObject::OnLayoutChange()
{
  // here do nothing
}

void CButtonObject::OnSetFocus()
{
  OnLayoutChange();
}

void CButtonObject::OnLoseFocus()
{
  OnLayoutChange();
}

BOOL CButtonObject::OnKeyDown(UINT nChar, DWORD dwFlags)
{
  switch(nChar){
  case VK_SPACE:
    if(!m_bPressed){
      m_bPressed = TRUE;
      OnLayoutPress();
      OnPress();
    }
    else{
      if(m_bRepeate){
        OnPress();
      }
    }
    return TRUE;
  }
  return CWindow::OnKeyDown(nChar, dwFlags);
}

BOOL CButtonObject::OnKeyUp(UINT nChar, DWORD dwFlags)
{
  switch(nChar){
  case VK_SPACE:
    if(m_bPressed){
      m_bPressed = FALSE;
      OnLayoutRelease();
      OnRelease();
    }
    return TRUE;
  }
  return CWindow::OnKeyUp(nChar, dwFlags);
}

void CButtonObject::SetKeyAcc(UINT nChar, DWORD dwFlags)
{
  if(!g_pKeyAccelerators) return;

  g_pKeyAccelerators->Disconnect(this);
  g_pKeyAccelerators->Connect(this, nChar, dwFlags);
  m_nKeyAccChar = nChar;
  m_dwKeyAccFlags = dwFlags;
}

BOOL CButtonObject::OnKeyAccelerator(UINT nChar, DWORD dwFlags)
{
  if((m_nKeyAccChar == nChar) && ((m_dwKeyAccFlags & dwFlags) || (m_dwKeyAccFlags == 0))){
    InlayEvent(E_COMMAND, 0); // Just inlay the event
    // we shouldn't do any more actions because
    // this situation is quite unstandard
    return TRUE;
  }

  return FALSE;
}

void CButtonObject::OnInternalEnableWindow()
{
  if(IsEnabled()){
    if(IsMouseAbove(g_pMouse->GetPosition())){
      m_bMouseActive = TRUE;
      OnMouseActivate();
    }
  }

  CWindow::OnInternalEnableWindow();
}

void CButtonObject::OnInternalDisableWindow()
{
  if(m_bMouseDrag){
    m_bMouseDrag = FALSE;
    ReleaseCapture();
    if(m_bRepeate) g_pTimer->Disconnect(this);
    m_bPressed = FALSE;
  }
  if(m_bMouseActive){
    m_bMouseActive = FALSE;
    OnLayoutChange();
  }

  CWindow::OnInternalDisableWindow();
}

CSize CButtonObject::GetVirtualSize()
{
  return GetWindowPosition().Size();
}

void CButtonObject::SetSensitiveRectangle(CRect *pNewRect)
{
  // Just copy the rect
  m_rcSensitiveRectangle = *pNewRect;
}

CWindow *CButtonObject::WindowFromPoint(CPoint &pt)
{
  CWindow *pResult = CWindow::WindowFromPoint(pt);

  if(pResult == this){
    if(m_rcSensitiveRectangle.IsRectEmpty()) return pResult;
    CPoint ptAsk;
    ptAsk.x = pt.x - GetWindowScreenPosition().left;
    ptAsk.y = pt.y - GetWindowScreenPosition().top;
    
    if(m_rcSensitiveRectangle.PtInRect(ptAsk)) return pResult;

    return NULL;
  }

  return pResult;
}