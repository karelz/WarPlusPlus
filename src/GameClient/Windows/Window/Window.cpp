// Window.cpp: implementation of the CWindow class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Window.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#include "..\Mouse\Mouse.h"
#include "..\Mouse\Cursor.h"
#include "..\Keyboard\Keyboard.h"
#include "WindowsWindow.h"

CWindow *g_pDesktopWindow = NULL;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CWindow, CNotifier);

BEGIN_OBSERVER_MAP(CWindow, CNotifier)
  BEGIN_KEYBOARD()
    ON_KEYDOWN()
  END_KEYBOARD()
  BEGIN_NOTIFIER(Update_Notifier)
    EVENT(E__UPDATE)
      OnUpdateNotification();
  END_NOTIFIER()
END_OBSERVER_MAP(CWindow, CNotifier)

CWindow::CWindow()
{
  m_pFirstChild = NULL;
  m_pLastChild  = NULL;
  m_pNextSibling = NULL;
  m_pPrevSibling = NULL;
  m_pParent = NULL;
  m_bDeleted = TRUE;
  m_bHasCapture = FALSE;
  m_pPrevCaptureWindow = NULL;
  m_bActive = FALSE;
  m_bTopMost = FALSE;
  m_bTransparent = FALSE;
  m_bFocus = FALSE;
  m_bTabStop = FALSE;
  m_pNextTabWindow = NULL;
  m_pPrevTabWindow = NULL;
  m_bDoubleClk = FALSE;
  m_bEnabled = TRUE;
  m_bInternalyEnabled = TRUE;
  m_pCursor = NULL;
  m_rcPrevMouseClipRect.SetRect(0, 0, 0, 0);

  m_bHidden = FALSE;
  m_bInternalyHidden = FALSE;

  m_pGraphicalBuffer = NULL;
  m_bBuffering = FALSE;
  m_rectInvalid.SetRectEmpty();  
}

CWindow::~CWindow()
{

}

//////////////////////////////////////////////////////////////////////
// Debug methods

#ifdef _DEBUG

void CWindow::AssertValid() const
{
  CNotifier::AssertValid();
}

void CWindow::Dump(CDumpContext &dc) const
{
  CNotifier::Dump(dc);

  dc << "Window rect " << m_rectWindow << " Client rect " << m_rectClient << "\n";
  dc << "Transparency " << m_bTransparent << "\n";
}

#endif

CWindow * CWindow::WindowFromPoint(CPoint & pt)
{
  ASSERT(!m_bDeleted);
  if(m_bDeleted) return NULL;

  if(m_bHidden) return NULL;
  if(!m_rectWindow.PtInRect(pt)) return NULL;

  CWindow *pRet = NULL;
  CWindow *pChild = m_pFirstChild;

  while(pChild != NULL){
    if((pRet = pChild->WindowFromPoint(pt)) != NULL){
      break;
    }
    pChild = pChild->m_pNextSibling;
  }

  if(pRet == NULL) pRet = this;
  return pRet;
}

CDDrawSurface *CWindow::m_pWindowsBufferSurface = NULL;
CWindow *CWindow::m_pFocusWindow = NULL;
CUpdateRectList *CWindow::m_pUpdateList = NULL;
CMutex CWindow::m_semaphoreUpdateList;
CMutex CWindow::m_lockWindowsPosition;
BOOL CWindow::m_bUpdatingEnabled = TRUE;

BOOL CWindow::Create(CRect * pWindowRect, CWindow * pParent)
{
  CWindow *pTabWnd;

  if(!CNotifier::Create()) return FALSE;
  
  FreezeWindows();
    
  try
  {

  m_rectWindow = *pWindowRect;
  if(pParent) m_rectWindow.OffsetRect(pParent->m_rectWindow.TopLeft());
  m_rectClient.SetRect(0, 0, m_rectWindow.Width(), m_rectWindow.Height());

  m_pParent = pParent;
  if(m_pParent != NULL){
    ASSERT_VALID(m_pParent);

    
    if(m_bTopMost){
      if(m_pParent->m_pFirstChild == NULL)
        m_pParent->m_pLastChild = this;
      else
        m_pParent->m_pFirstChild->m_pPrevSibling = this;
      m_pNextSibling = m_pParent->m_pFirstChild;
      m_pPrevSibling = NULL;
      m_pParent->m_pFirstChild = this;
    }
    else{
      CWindow *pWnd = m_pParent->m_pFirstChild;
      while(pWnd != NULL){
        if(!(pWnd->m_bTopMost)) break;
        pWnd = pWnd->m_pNextSibling;
      }
      // Insert us before the pWnd;
      if(pWnd != NULL){
        if(pWnd->m_pPrevSibling != NULL){
          pWnd->m_pPrevSibling->m_pNextSibling = this;
          m_pPrevSibling = pWnd->m_pPrevSibling;
          pWnd->m_pPrevSibling = this;
          m_pNextSibling = pWnd;
        }
        else{
          pWnd->m_pPrevSibling = this;
          m_pNextSibling = pWnd;
          m_pParent->m_pFirstChild = this;
          m_pPrevSibling = NULL;
        }
      }
      else{
        if(m_pParent->m_pLastChild == NULL){
          m_pParent->m_pFirstChild = this;
          m_pParent->m_pLastChild = this;
          m_pNextSibling = NULL;
          m_pPrevSibling = NULL;
        }
        else{
          m_pParent->m_pLastChild->m_pNextSibling = this;
          m_pPrevSibling = m_pParent->m_pLastChild;
          m_pParent->m_pLastChild = this;
          m_pNextSibling = NULL;
        }
      }
    }

    // insert this to the list of Tab windows
    m_pNextTabWindow = NULL;
    m_pPrevTabWindow = NULL;
    if(m_bTabStop){
      // find some tab window
      pTabWnd = m_pNextSibling;
      while((pTabWnd != NULL) && (!pTabWnd->m_bTabStop)){
        pTabWnd = pTabWnd->m_pNextSibling;
      }
      if(pTabWnd == NULL){
        pTabWnd = m_pPrevSibling;
        while((pTabWnd != NULL) && (!pTabWnd->m_bTabStop)){
          pTabWnd = pTabWnd->m_pPrevSibling;
        }
      }

      // find the last Tab window
      if(pTabWnd != NULL){
        while(pTabWnd->m_pNextTabWindow != NULL) pTabWnd = pTabWnd->m_pNextTabWindow;
        pTabWnd->m_pNextTabWindow = this;
        m_pPrevTabWindow = pTabWnd;
        m_pNextTabWindow = NULL;
      }
    }

    // copy the internal enable flag
    m_bInternalyEnabled = m_pParent->m_bInternalyEnabled;
  }
  else{
    // this should be the only one window with parent == NULL
    // so we will make it a desktop window
    ASSERT(g_pDesktopWindow == NULL);
    g_pDesktopWindow = this;

    // as a desktop window we have to allocate
    // the WindowsBufferSurface
    m_pWindowsBufferSurface = new CScratchSurface();
    ((CScratchSurface *)m_pWindowsBufferSurface)->SetWidth(g_pDDPrimarySurface->GetWidth());
    ((CScratchSurface *)m_pWindowsBufferSurface)->SetHeight(g_pDDPrimarySurface->GetHeight());
    if(!m_pWindowsBufferSurface->Create()) return FALSE;

    VERIFY ( m_semaphoreUpdateList.Lock() );
    ASSERT(m_pUpdateList == NULL);
    // create the update list
    m_pUpdateList = new CUpdateRectList();
    if(!m_pUpdateList->Create(1, 1)){
      VERIFY ( m_semaphoreUpdateList.Unlock() );
      UnfreezeWindows();
      return FALSE;
    }
    VERIFY ( m_semaphoreUpdateList.Unlock() );

    // connect us to us
    this->Connect(this, Update_Notifier);
  }

  // now connect to the keyboard
  // do it only when we have the m_bTabStop set
  // we have to do it otherway the TAB key would not work well
  if(g_pKeyboard)
    g_pKeyboard->Connect(this);

  // connect us to mouse -> I know not all windows need mouse
  // but it will be much more clear, because the mouse
  // doesn't care if the window is connected or not
  // she just sends us an event -> if not connected
  // the event is sunk, but its an unclear way
  // better to sunk it ourself
  if(g_pMouse)
    g_pMouse->Connect(this);

  m_bDeleted = FALSE;

  // update the window
  UpdateRect();

  }
  catch ( ... )
  {
    UnfreezeWindows ();
    throw;
  }

  UnfreezeWindows();

  return TRUE;
}

void CWindow::Delete()
{
  if(m_bDeleted) return;

/*  if ( IsActive () )
  {
    CWindow *pWindow = m_pNextSibling;
    while ( pWindow != NULL )
    {
      if ( pWindow->IsAccesable () )
      {
        pWindow->Activate ();
        break;
      }
      pWindow = pWindow->m_pNextSibling;
    }

    if ( pWindow == NULL )
    {
      pWindow = m_pPrevSibling;
      while ( pWindow != NULL )
      {
        if ( pWindow->IsAccesable () )
        {
          pWindow->Activate ();
          break;
        }
        pWindow = pWindow->m_pPrevSibling;
      }
    }
  }*/

  StopBuffering();
  
  FreezeWindows();

  try
  {

  CRect rcWindow = m_rectWindow;

  // If we have the capture -> release it
  ReleaseCapture();

  // set the cursor to none
  SetWindowCursor(NULL);

  if(g_pMouse)
    g_pMouse->Disconnect(this);

  if(g_pKeyboard)
    g_pKeyboard->Disconnect(this);

  // remove this from parents list of childs
  // note that if there is no error
  // there should be only one window in the system 
  // with parent == NULL
  if(m_pParent != NULL){
    if(m_pParent->m_pLastChild == this)
      m_pParent->m_pLastChild = m_pPrevSibling;
    if(m_pParent->m_pFirstChild == this)
      m_pParent->m_pFirstChild = m_pNextSibling;
  }
  else{
    // disconnect us from us (its a clear way)
    this->Disconnect(this);

    VERIFY( m_semaphoreUpdateList.Lock() );
    m_pUpdateList->Delete();
    delete m_pUpdateList;
    m_pUpdateList = NULL;
    VERIFY ( m_semaphoreUpdateList.Unlock() );
    
    g_pDesktopWindow = NULL;
    m_pWindowsBufferSurface->Delete();
    delete m_pWindowsBufferSurface;
  }
  
  // remove this from the list of siblings
  if(m_pNextSibling != NULL)
    m_pNextSibling->m_pPrevSibling = m_pPrevSibling;
  if(m_pPrevSibling != NULL)
    m_pPrevSibling->m_pNextSibling = m_pNextSibling;

  //remove this from the list of Tab windows
  if(m_pNextTabWindow != NULL)
    m_pNextTabWindow->m_pPrevTabWindow = m_pPrevTabWindow;
  if(m_pPrevTabWindow != NULL)
    m_pPrevTabWindow->m_pNextTabWindow = m_pNextTabWindow;

  // delete all children
  CWindow *pChild = m_pFirstChild, *pNextChild;
  while(pChild != NULL){
    pNextChild = pChild->m_pNextSibling;
    pChild->Delete();
    pChild = pNextChild;
  }

  if(m_pFocusWindow == this){
    if(m_pParent != NULL){
      m_pParent->TrySetFocus();
    }
    else{
      m_pFocusWindow = NULL;
    }
  }

  // all of these should be NULL now (children made it)
  // but we make it sure
  m_pFirstChild = NULL;
  m_pLastChild = NULL;
  m_pNextSibling = NULL;
  m_pPrevSibling = NULL;
  m_pNextTabWindow = NULL;
  m_pPrevTabWindow = NULL;
  m_pParent = NULL;
  m_bDeleted = TRUE;

  m_bActive = FALSE;

  CNotifier::Delete();

  // update the window rect so after the delete it will be cleared from the screen
  UpdateRect();

  }
  catch ( ... )
  {
    UnfreezeWindows ();
    throw;
  }

  UnfreezeWindows();
}

void CWindow::SetBuffering(DWORD dwXSize, DWORD dwYSize)
{
  m_bBuffering = TRUE;
  SetBufferingInternal(dwXSize, dwYSize);
}

void CWindow::SetBufferingInternal(DWORD dwXSize, DWORD dwYSize)
{
  ASSERT(m_bTransparent == FALSE);

  FreezeWindows();

  try
  {

  // no transparency is available
  m_bTransparent = FALSE;

  // allocate the buffer
  m_pGraphicalBuffer = new CScratchSurface();

  // create it
  if(dwXSize == 0x0FFFF){
    m_pGraphicalBuffer->SetWidth(m_rectWindow.Width());
    m_pGraphicalBuffer->SetHeight(m_rectWindow.Height());
  }
  else{
    m_pGraphicalBuffer->SetWidth(dwXSize);
    m_pGraphicalBuffer->SetHeight(dwYSize);
  }
  m_pGraphicalBuffer->Create();

  // mark us as invalid (the buffer must be filled)
  InvalidateRect(m_rectWindow);

  }
  catch ( ... )
  {
    UnfreezeWindows ();
    throw;
  }

  UnfreezeWindows();
}

void CWindow::StopBuffering()
{
  m_bBuffering = FALSE;
  StopBufferingInternal();
}

void CWindow::StopBufferingInternal()
{
  FreezeWindows();

  try
  {
  // close buffering if some was active
  if(m_pGraphicalBuffer != NULL){
    m_pGraphicalBuffer->Delete();
    delete m_pGraphicalBuffer;
    m_pGraphicalBuffer = NULL;
  }
  }
  catch ( ... )
  {
    UnfreezeWindows ();
    throw;
  }

  UnfreezeWindows();
}

void CWindow::Draw(CDDrawSurface * pDDSurface, CRect * pBoundRect)
{

}

BOOL CWindow::WhatToDraw(CRect * pRect, CWindow * * pWindowsList, BOOL bUpdateThis)
{
  CRect rc(pRect);
  BOOL bFilledWithChildren = FALSE;
  rc.IntersectRect(&rc, &m_rectWindow);

// this rect is outside us -> skip us
  if(rc.IsRectEmpty()) return FALSE;
// also if we are invisible -> skip us (as if we isn't there)
  if ( m_bInternalyHidden || m_bHidden ) return FALSE;

  CWindow *pChild = m_pFirstChild;

  if((m_pGraphicalBuffer != NULL) && (!bUpdateThis)){
    // if buffered -> add us to the list and no children
    m_pNextWindow = *pWindowsList;
    *pWindowsList = this;

    if(rc == *pRect) return TRUE;
    else return FALSE;
  }

  while(pChild != NULL){
    if(pChild->WhatToDraw(pRect, pWindowsList, FALSE)){
      bFilledWithChildren = TRUE;
      break;
    }
    pChild = pChild->m_pNextSibling;
  }
  //if the rect was not covered by children we have to draw a part
  // of this window too
  if(!bFilledWithChildren){
    // add us to the head of the list
    m_pNextWindow = *pWindowsList;
    *pWindowsList = this;
  }

  // if the intersected rect is equal to givven rect
  // it means that this window covers whole the pRect
  // so we will return TRUE 
  if((rc == *pRect) && (!m_bTransparent)) return TRUE;
  else return FALSE;
}

// redraws givven client rectangle
void CWindow::UpdateRect(CRect * pRect)
{
  if ( g_pDesktopWindow == NULL ) return; // no desktop, no draw
  if ( m_bInternalyHidden || m_bHidden ) return; // not visible -> no draw
  if ( pRect == NULL ) pRect = &m_rectClient;
  CRect rc(pRect);

  // offset the rect, so it will mean the coords on the screen
  rc.OffsetRect(m_rectWindow.left, m_rectWindow.top);

  VERIFY ( m_semaphoreUpdateList.Lock() );

  try{
  if(m_pUpdateList->IsEmpty() && m_bUpdatingEnabled){
    g_pDesktopWindow->InlayEvent(E__UPDATE, 0, g_pDesktopWindow, CEventManager::Priority_Low - 1);
  }

  // do the updateing of the rect
  m_pUpdateList->AddRect(&rc);

  // invalidate this window
  InvalidateRect(rc);
  }
  catch(CException *)
  {
    VERIFY ( m_semaphoreUpdateList.Unlock() );
    throw;
  }

  VERIFY ( m_semaphoreUpdateList.Unlock() );
}

// enables updating of the windows
void CWindow::EnableWindowsUpdating(BOOL bEnabled)
{
  // lock the update list
  VERIFY(m_semaphoreUpdateList.Lock());

  try{
  // set the updating flag
  m_bUpdatingEnabled = bEnabled;

  // if we're going to update enabled mode -> inlay event if there's something to draw
  if(bEnabled && !m_pUpdateList->IsEmpty()){
    g_pDesktopWindow->InlayEvent(E__UPDATE, 0, g_pDesktopWindow, CEventManager::Priority_Low - 1);
  }
  }
  catch(CException *)
  {
    VERIFY(m_semaphoreUpdateList.Unlock());
    throw;
  }

  // release the lock
  VERIFY(m_semaphoreUpdateList.Unlock());
}

// draws all children windows for this window to given surface
void CWindow::DrawChildren(CDDrawSurface *pSurface)
{
  DoUpdateRect(pSurface, pSurface->GetAllRect());
}

// updates this and all children in the givven rect
// it should be called only on desktop window
// because other windows would not have the background
void CWindow::DoUpdateRect(CDDrawSurface *pSurface, CRect *pRect)
{
  CWindow *pFirstToDraw = NULL;
  CRect rcOldClip;
  CPoint ptOldTrans;

  
  // this should return TRUE but I will not test it now
  // if it returns FALSE some part of the pRect wan't be redrawn
  // in the right way
  WhatToDraw(pRect, &pFirstToDraw, TRUE);

  // set the clipping rectangle for the surface
  pSurface->SetClipRect(pRect, &rcOldClip);

  ptOldTrans = pSurface->GetTransformation();

  // go through the list and draw the windows
  while(pFirstToDraw != NULL){
    CRect rcclip(pRect);

    // set new clipping rectangle
    // we have to go through all the parent windows
    // for given window and clip the rect with them
    // if not so -> child windows could bew draw outside the parent ones
    // which is against the philosophy of this system
    CWindow *pCurrent = pFirstToDraw;
    while(pCurrent != NULL){
      rcclip.IntersectRect(&rcclip, &pCurrent->m_rectWindow);
      pCurrent = pCurrent->m_pParent;
    }
    // transform the rect to the client coords
    rcclip.OffsetRect(-pFirstToDraw->m_rectWindow.left,
      -pFirstToDraw->m_rectWindow.top);

    // set the transformation for the surface
    pSurface->SetTransformation(CPoint(pFirstToDraw->m_rectWindow.left + ptOldTrans.x, pFirstToDraw->m_rectWindow.top + ptOldTrans.y));

    // set the clip rect (with no transformation)
    pSurface->SetClipRect(&rcclip, NULL);

    // draw the window
    if(pFirstToDraw == this)
      pFirstToDraw->DoDraw(pSurface, &rcclip, TRUE);
    else
      pFirstToDraw->DoDraw(pSurface, &rcclip, FALSE);

    // move to the next window
    pFirstToDraw = pFirstToDraw->m_pNextWindow;
  }

  pSurface->SetTransformation(ptOldTrans);
  pSurface->SetClipRect(&rcOldClip, NULL);
}

void CWindow::SetWindowPosition(CPoint * pPoint)
{
  CRect rcNew;
  rcNew.SetRect(pPoint->x, pPoint->y,
     pPoint->x + m_rectWindow.Width(), m_rectWindow.Height() + pPoint->y);

  SetWindowPosition(&rcNew);
}

void CWindow::SetWindowPosition(CRect * pRect)
{
  FreezeWindows();

  try
  {

  // rectangles to redraw after moving the window
  // the top and bottom will include the portions which
  // is included in both of the intersected rects
  CRect rcRect(pRect);
  if(m_pParent){
    rcRect.OffsetRect(m_pParent->m_rectWindow.TopLeft());
  }

  CRect rcTop, rcBottom, rcLeft, rcRight, rcWhole;

  rcLeft.right = rcRect.left;
  rcLeft.left = m_rectWindow.left;
  rcLeft.top = __max(rcRect.top, m_rectWindow.top);
  rcLeft.bottom = __min(rcRect.bottom, m_rectWindow.bottom);

  rcRight.left = rcRect.right;
  rcRight.right = m_rectWindow.right;
  rcRight.top = rcLeft.top;
  rcRight.bottom = rcLeft.bottom;

  rcTop.top = m_rectWindow.top;
  rcTop.bottom = rcRect.top;
  rcTop.left = m_rectWindow.left;
  rcTop.right = m_rectWindow.right;

  rcBottom.top = rcRect.bottom;
  rcBottom.bottom = m_rectWindow.bottom;
  rcBottom.left = m_rectWindow.left;
  rcBottom.right = m_rectWindow.right;

  SetThePosition(&rcRect);

  // redraw the old parts of the window
  // we have to do this on the desktop window because
  // we have only the screen coords and its outside this window
  if((rcLeft.left < rcLeft.right) && (!rcLeft.IsRectEmpty()))
    g_pDesktopWindow->UpdateRect(&rcLeft);
  if((rcRight.left < rcRight.right) && (!rcRight.IsRectEmpty()))
    g_pDesktopWindow->UpdateRect(&rcRight);
  if((rcTop.top < rcTop.bottom) && (!rcTop.IsRectEmpty()))
    g_pDesktopWindow->UpdateRect(&rcTop);
  if((rcBottom.top < rcBottom.bottom) && (!rcBottom.IsRectEmpty()))
    g_pDesktopWindow->UpdateRect(&rcBottom);

  // redraw all the new window
  InvalidateRect(m_rectWindow);
  g_pDesktopWindow->UpdateRect(&m_rectWindow);

  }
  catch ( ... )
  {
    UnfreezeWindows ();
    throw;
  }

  UnfreezeWindows();
}

CRect CWindow::GetWindowPosition()
{
  CRect rcWindow = m_rectWindow;
  if(m_pParent){
    rcWindow.OffsetRect(-m_pParent->m_rectWindow.left, -m_pParent->m_rectWindow.top);
  }
  return rcWindow;
}

CWindow * CWindow::SetCapture()
{
  if(!g_pMouse) return NULL;

  m_pPrevCaptureWindow = g_pMouse->SetCapture(this);
  m_bHasCapture = TRUE;
  return m_pPrevCaptureWindow;
}

void CWindow::ReleaseCapture()
{
  if(!g_pMouse) return;

  if(m_bHasCapture)
    g_pMouse->SetCapture(m_pPrevCaptureWindow);
  m_bHasCapture = FALSE;
}

CRect CWindow::SetMouseClip(CRect *pRect)
{
  if(!g_pMouse) return CRect();

  CRect rcNew;
  if(pRect == NULL)
    rcNew = m_rectWindow;
  else
    rcNew = *pRect;

  m_rcPrevMouseClipRect = g_pMouse->GetClipRect();

  g_pMouse->SetClipRect(rcNew);

  return m_rcPrevMouseClipRect;
}

void CWindow::ReleaseMouseClip()
{
  if(!g_pMouse) return;

  // can't be empty .. it's error
  ASSERT(!m_rcPrevMouseClipRect.IsRectEmpty());

  g_pMouse->SetClipRect(m_rcPrevMouseClipRect);

  m_rcPrevMouseClipRect.SetRect(0, 0, 0, 0);
}

CWindow * CWindow::DoActivation(BOOL &bWantActivationEvent)
{
  CWindow *pPrevSibling, *pWindowToRedraw, *pActiveWindow;

  bWantActivationEvent = TRUE;

  // move our window to the top of the siblings
  // one cycle will move our window over one window
  while(m_pPrevSibling){
    // if we are not top most and the window above us does
    // no more move over
    if((m_pPrevSibling->m_bTopMost) && (!m_bTopMost)) break;

    pPrevSibling = m_pPrevSibling;

    // to the next write the new prev sibling
    // or if there is no next sibling
    // change the last child of our parent window
    if(m_pNextSibling) m_pNextSibling->m_pPrevSibling = pPrevSibling;
    else if(m_pParent) m_pParent->m_pLastChild = pPrevSibling;

    // to the prev write the new next sibling
    pPrevSibling->m_pNextSibling = m_pNextSibling;

    // we will have as a prev sibling the prev of the prev
    m_pPrevSibling = pPrevSibling->m_pPrevSibling;
    // and so we will write us as the next for the prev of the prev
    // or if there is no prev of the prev
    // we will write us as the first child of the parent window
    if(m_pPrevSibling) m_pPrevSibling->m_pNextSibling = this;
    else if(m_pParent) m_pParent->m_pFirstChild = this;

    // to the prev we will write us as the prev
    pPrevSibling->m_pPrevSibling = this;
    // and we will have the prev as our new next
    m_pNextSibling = pPrevSibling;
  }

  // if our window was active before - no action needed
  if(!m_bActive){
    // send to the previous active window the deactivate notification

    // try to find it in next siblings
    pActiveWindow = m_pNextSibling;
    while(pActiveWindow){
      if(pActiveWindow->m_bActive){
        pActiveWindow->DoDeactivation();
        break;
      }
      pActiveWindow = pActiveWindow->m_pNextSibling;
    }

    // if not found -> continue searching in the prev siblings
    // this means it was a topmost window
    if(!pActiveWindow){
      pActiveWindow = m_pPrevSibling;
      while(pActiveWindow){
        if(pActiveWindow->m_bActive){
          pActiveWindow->DoDeactivation();
          break;
        }
        pActiveWindow = pActiveWindow->m_pPrevSibling;
      }
    }

    // activate our parent window
    if(m_pParent){
      // if the activation of parent window returns some window
      // to redraw, we will return the same
      // it has to be one of ours ancestors - so we are included
      BOOL bSome; // for parent window, we can forget this param
      // beacause no event will be sent to the parent window
      pWindowToRedraw = m_pParent->DoActivation(bSome);
      if(pWindowToRedraw == NULL) pWindowToRedraw = this;
    }
    else{
      pWindowToRedraw = this;
    }

    // send us the activate notification
    m_bActive = TRUE;
    bWantActivationEvent = OnActivate();

    return pWindowToRedraw;
  }

  return NULL;
}

void CWindow::DoDeactivation()
{
  CWindow *pChild;

  // deactivate all our children
  pChild = m_pFirstChild;
  while(pChild){
    if(pChild->m_bActive){
      pChild->DoDeactivation();
    }
    pChild = pChild->m_pNextSibling;
  }

  // deactivate this window
  m_bActive = FALSE;
  OnDeactivate();
}

BOOL CWindow::Activate()
{
  CWindow *pWindowToRedraw;
  BOOL bWantEvent;

  // activate us
  pWindowToRedraw = DoActivation(bWantEvent);

  TrySetFocus();

  // if there is some window to redraw so do it
  if(pWindowToRedraw) pWindowToRedraw->UpdateRect();

  return bWantEvent;
}

void CWindow::OnDeactivate()
{
}

BOOL CWindow::OnActivate()
{
  return TRUE;
}

void CWindow::BringToTop(BOOL bMakeTopMost)
{
  m_bTopMost = bMakeTopMost;
  Activate();
}

void CWindow::SetThePosition(CRect *pRect)
{
  CPoint ptMove;
  CWindow *pChild;
  BOOL bSized = FALSE, bMoved = FALSE;

  if((pRect->Width() != m_rectWindow.Width()) ||
    (pRect->Height() != m_rectWindow.Height()))
    bSized = TRUE;

  if((pRect->left != m_rectWindow.left) ||
    (pRect->top != m_rectWindow.top))
    bMoved = TRUE;
  
  // determine the move of the window
  ptMove.x = pRect->left - m_rectWindow.left;
  ptMove.y = pRect->top  - m_rectWindow.top;

  // set the new position
  m_rectWindow = pRect;
  m_rectClient.SetRect(0, 0, m_rectWindow.Width(), m_rectWindow.Height());

  // move all children with the window
  pChild = m_pFirstChild;
  while(pChild != NULL){
    CRect rc(pChild->m_rectWindow);
    rc.OffsetRect(ptMove);
    pChild->SetThePosition(&rc);
    pChild = pChild->m_pNextSibling;
  }

  if(bSized) OnSize(m_rectWindow.Size());
  if(bMoved) OnMove(m_rectWindow.TopLeft());
}

CRect * CWindow::GetClientRect()
{
  return &m_rectClient;
}

BOOL CWindow::HasFocus()
{
  return m_bFocus;
}

CWindow * CWindow::GetNextTabWindow()
{
  CWindow *pFirstWnd;
  
  // if there is some next Tab window -> return it
  if(m_pNextTabWindow != NULL) return m_pNextTabWindow;

  // else try to find the first Tab window in the list
  pFirstWnd = this;
  while(pFirstWnd->m_pPrevTabWindow != NULL)
    pFirstWnd = pFirstWnd->m_pPrevTabWindow;

  if(pFirstWnd == this) return NULL;

  return pFirstWnd;
}

CWindow * CWindow::GetPrevTabWindow()
{
  CWindow *pLastWnd;

  if(m_pPrevTabWindow != NULL) return m_pPrevTabWindow;

  pLastWnd = this;
  while(pLastWnd->m_pNextTabWindow != NULL)
    pLastWnd = pLastWnd->m_pNextTabWindow;

  if(pLastWnd == this) return NULL;

  return pLastWnd;
}

BOOL CWindow::TrySetFocus()
{
  // if we or our children have the focus do nothing
  if(FindTheFocus()) return TRUE;
  
  // If we have the m_bTabStop -> set it to us
  if(m_bTabStop && m_bInternalyEnabled && !m_bInternalyHidden && !m_bHidden){
    SetFocus();
    return TRUE;
  }

  // if the focus don't have any of our children
  // we have to take it away from the window
  // wich has it
  if(m_pFocusWindow != NULL){
    m_pFocusWindow->m_bFocus = FALSE;
    m_pFocusWindow->OnLoseFocus();
    m_pFocusWindow = NULL;
  }

  // try to find the focus window in our children
  CWindow *pChild;
  // find out the first Tab child window
  pChild = m_pFirstChild;
  if(pChild == NULL) return FALSE;
  while(pChild->m_pPrevTabWindow != NULL)
    pChild = pChild->m_pPrevTabWindow;

  // try to set the focus to our children
  while(pChild != NULL){
    if(pChild->TrySetFocus()) return TRUE;
    pChild = pChild->m_pNextTabWindow;
  }

  return FALSE;
}

BOOL CWindow::FindTheFocus()
{
  if(m_pFocusWindow == this) return TRUE;

  CWindow *pChild;
  pChild = m_pFirstChild;
  if(pChild == NULL) return NULL;
  while(pChild->m_pPrevTabWindow != NULL)
    pChild = pChild->m_pPrevTabWindow;

  while(pChild != NULL){
    if(pChild->FindTheFocus()) return TRUE;
    pChild = pChild->m_pNextTabWindow;
  }

  return FALSE;
}

void CWindow::SetFocus()
{
  // if already have the focus -> do nothing
  if(this == m_pFocusWindow) return;

  // take a focus from the window which had it before
  if(m_pFocusWindow){
    m_pFocusWindow->m_bFocus = FALSE;
    m_pFocusWindow->OnLoseFocus();
    m_pFocusWindow = NULL;
  }

  // set the focus to us
  if(m_bTabStop){
    m_pFocusWindow = this;
    m_bFocus = TRUE;
    OnSetFocus();
  }
}

void CWindow::OnSetFocus()
{
  // do nothing
}

void CWindow::OnLoseFocus()
{
  // do nothing
}

void CWindow::EnableWindow(BOOL bEnable)
{
  ASSERT_VALID(this);
  // if it's the same -> do nothing
  if(m_bEnabled == bEnable) return;

  // enable this window first
  m_bEnabled = bEnable;

  // now go through all childrens and internaly enable them
  InternalEnableWindow(bEnable);

  // call the notifications
  if(m_bEnabled)
    OnEnableWindow();
  else
    OnDisableWindow();
}

void CWindow::OnEnableWindow()
{
}

void CWindow::OnDisableWindow()
{
}

void CWindow::OnInternalEnableWindow()
{
}

void CWindow::OnInternalDisableWindow()
{
}

void CWindow::InternalEnableWindow(BOOL bEnable)
{
  if(bEnable){
    if(!m_bInternalyEnabled){
      m_bInternalyEnabled = TRUE;
      OnInternalEnableWindow();
    }

    // go through all childrens and set them too
    // but if some has the m_bEnabled set to FALSE -> don't set the internal to TRUE
    // it was disabled manualy so left it as it was
    CWindow *pChild = m_pFirstChild;
    while(pChild != NULL){
      // set it only if the child is manualy enabled
      if(pChild->m_bEnabled)
        pChild->InternalEnableWindow(TRUE);

      // go to the next child
      pChild = pChild->m_pNextSibling;
    }
  }
  else{
    if(m_bInternalyEnabled){
      m_bInternalyEnabled = FALSE;
      OnInternalDisableWindow();
    }

    // the FALSE value has to be set to all children, those who have already set the
    // m_bEnabled to FALSE can be skipped - they already have it FALSE
    CWindow *pChild = m_pFirstChild;
    while(pChild != NULL){
      if(pChild->m_bEnabled)
        pChild->InternalEnableWindow(FALSE);

      // go to the next one
      pChild = pChild->m_pNextSibling;
    }
  }
}

// Hides or unhides internaly the window (and all its children)
void CWindow::InternalyHideWindow ( BOOL bHide )
{
  if ( bHide )
  {
    // We're already normaly hidden -> no need to do
    if ( m_bHidden )
    {
      // Just hide us also internaly
      m_bInternalyHidden = TRUE;
      return;
    }
    if ( m_bInternalyHidden ) return;
    // Hide us internaly
    m_bInternalyHidden = TRUE;
    // Hide all our children
    CWindow * pChild = m_pFirstChild;
    while ( pChild != NULL )
    {
      pChild->InternalyHideWindow ( TRUE );
      pChild = pChild->m_pNextSibling;
    }
  }
  else
  {
    // Already in such a state -> do nothing
    if ( !m_bInternalyHidden ) return;
    // If we're hidden normaly, don't touch this one
    if ( m_bHidden )
    {
      // Just remove the internal flag
      m_bInternalyHidden = FALSE;
      return;
    }
    // Show us internaly
    m_bInternalyHidden = FALSE;
    // Show all our children
    CWindow * pChild = m_pFirstChild;
    while ( pChild != NULL )
    {
      pChild->InternalyHideWindow ( FALSE );
      pChild = pChild->m_pNextSibling;
    }
  }
}

BOOL CWindow::OnKeyDown(DWORD dwKey, DWORD dwFlags)
{
  switch(dwKey){
  case VK_TAB:  // on tab key we will move the focus to another window
    if(dwFlags & CKeyboard::CtrlDown){
      return FALSE;
    }
    if(dwFlags & CKeyboard::AltDown){
      return FALSE;
    }
    if(dwFlags & CKeyboard::ShiftDown){ // Shift-TAB move to prev
      SetFocusToPrev();
    }
    else{ // TAB move to next
      SetFocusToNext();
    }
    return TRUE;
  }
  return FALSE;
}

CCursor * CWindow::SetWindowCursor(CCursor *pCursor)
{
#ifdef _DEBUG
  if(pCursor != NULL) ASSERT_VALID(pCursor);
#endif

  CCursor *pOldOne = m_pCursor;

  if(m_pCursor == pCursor) return m_pCursor;

  if(g_pDesktopWindow){
    // if the cursor is on our window, we have to change it too
    if(g_pDesktopWindow->WindowFromPoint(g_pMouse->GetPosition()) == this){
      g_pMouse->SetCursor(pCursor);
    }
  }
  m_pCursor = pCursor;

  return pOldOne;
}

BOOL CWindow::IsMouseAbove(CPoint point)
{
  point.Offset(m_rectWindow.TopLeft());
  if(WindowFromPoint(point) == this) return TRUE;
  else return FALSE;
}

void CWindow::OnSize(CSize size)
{
  // do nothing
}

void CWindow::OnMove(CPoint point)
{
  // do nothing
}

void CWindow::OnUpdateNotification()
{
  // go through the list of updates and update each of them

  CUpdateRectList::SRectNode *pNode;

  VERIFY ( m_semaphoreUpdateList.Lock() );

  try{
  // If the updating is not enabled -> forget it
  if(!m_bUpdatingEnabled){
    VERIFY ( m_semaphoreUpdateList.Unlock() );
    return;
  }

  // if our application is not active we can't draw anything
  if(g_pDirectDraw->ExclusiveMode() && (!g_pMainWindow->IsActive())){
    m_pUpdateList->RemoveAll();
    VERIFY ( m_semaphoreUpdateList.Unlock() );
    return;
  }

  pNode = m_pUpdateList->GetFirstRectNode();
  while(pNode != NULL){
    // update it
    DoUpdateRect(m_pWindowsBufferSurface, &(pNode->m_rcRect));


    g_pDDPrimarySurface->Paste(pNode->m_rcRect.left, pNode->m_rcRect.top,
      m_pWindowsBufferSurface, &(pNode->m_rcRect));

    pNode = pNode->m_pNext;
  }

  m_pUpdateList->RemoveAll();
  }
  catch(CException *){
    VERIFY ( m_semaphoreUpdateList.Unlock() );
    throw;
  }

  VERIFY ( m_semaphoreUpdateList.Unlock() );
}

BOOL CWindow::OnKeyUp(UINT nChar, DWORD dwFlags)
{
  return FALSE;
}

void CWindow::SetFocusToNext()
{
  CWindow *pWnd = GetNextTabWindow();
  if(pWnd) pWnd->SetFocus();
}

void CWindow::SetFocusToPrev()
{
  CWindow *pWnd = GetPrevTabWindow();
  if(pWnd) pWnd->SetFocus();
}

void CWindow::UpdateNow()
{
  OnUpdateNotification();
}


void CWindow::InvalidateRect(CRect &rcInvalid)
{
  m_rectInvalid.UnionRect(&m_rectInvalid, &rcInvalid);

  if((m_pGraphicalBuffer == NULL) && (m_pParent != NULL)){
    m_pParent->InvalidateRect(rcInvalid);
  }
}

void CWindow::DoDraw(CDDrawSurface *pSurface, CRect *pRect, BOOL bUpdatingThis)
{
  // if no buffer just draw the window
  if((m_pGraphicalBuffer == NULL) || (bUpdatingThis)){
    Draw(pSurface, pRect);
    return;
  }

  // elseway we will check the invalide flag
  if(!m_rectInvalid.IsRectEmpty()){
    CWindow *pNextWindow = m_pNextWindow;
    CRect rcclip(m_rectInvalid);

    m_pGraphicalBuffer->SetTransformation(CPoint(-m_rectWindow.left, -m_rectWindow.top));

    // if invalid we must first draw the whole window into the buffer
    DoUpdateRect(m_pGraphicalBuffer, &rcclip);

    m_pGraphicalBuffer->SetTransformation(CPoint(0, 0));

    // mark us as valid
    m_rectInvalid.SetRectEmpty();

    m_pNextWindow = pNextWindow;
  }

  // now copy wanted rectangle into the dest surface
  pSurface->Paste(pRect->TopLeft(), m_pGraphicalBuffer, pRect);
}

void CWindow::HideWindow()
{
  // if we are already hidden do nothing
  if ( m_bHidden ) return;

  // If the window is deleted - just set the flag
  if ( m_bDeleted )
  {
    m_bHidden = TRUE;
	  return;
  }

/*  if ( IsActive () )
  {
    CWindow *pWindow = m_pNextSibling;
    while ( pWindow != NULL )
    {
      if ( pWindow->IsAccesable () )
      {
        pWindow->Activate ();
        break;
      }
      pWindow = pWindow->m_pNextSibling;
    }

    if ( pWindow == NULL )
    {
      pWindow = m_pPrevSibling;
      while ( pWindow != NULL )
      {
        if ( pWindow->IsAccesable () )
        {
          pWindow->Activate ();
          break;
        }
        pWindow = pWindow->m_pPrevSibling;
      }
    }
  }*/

  FreezeWindows();

  try
  {

  // If we're internaly hidden already
  // Just set the flag
  if ( m_bInternalyHidden )
  {
    m_bHidden = TRUE;
  }
  else
  {

    // must call the update with m_bHidde FALSE -> it will be done
    UpdateRect ();

    // First internaly hide all our children
    CWindow * pChild = m_pFirstChild;
    while ( pChild != NULL )
    {
      pChild->InternalyHideWindow ( TRUE );
      pChild = pChild->m_pNextSibling;
    }
    // Hide us only normaly
    m_bHidden = TRUE;

    // Also disable all children and us
    InternalEnableWindow(FALSE);
  }

  }
  catch ( ... )
  {
    UnfreezeWindows ();
    throw;
  }

  UnfreezeWindows();
}

void CWindow::ShowWindow()
{
  // if we are visible -> do nothing
  if ( !m_bHidden ) return;

  if ( m_bDeleted )
  {
    m_bHidden = FALSE;
    return;
  }

  FreezeWindows();

  try
  {

  // If we're internaly hidden -> it means
  // that one of our parents is normaly hidden
  // So just remove the Hidden flag from us
  if ( m_bInternalyHidden )
  {
    m_bHidden = FALSE;
  }
  else
  {

    // Show all our children
    CWindow * pChild = m_pFirstChild;
    while ( pChild != NULL )
    {
      pChild->InternalyHideWindow ( FALSE );
      pChild = pChild->m_pNextSibling;
    }
    // Show us normaly
    m_bHidden = FALSE;

    // Enable us
    InternalEnableWindow(TRUE);

    UpdateRect();
  }

  }
  catch ( ... )
  {
    UnfreezeWindows ();
    throw;
  }

  UnfreezeWindows();
}

BOOL CWindow::IsVisible()
{
  ASSERT_VALID(this);

  if(m_bHidden){
    return FALSE;
  }
  else{
    return TRUE;
  }
}

// draws all neede updates in the windows subsystem
// to the specified surface (it's assumed that it's whole screen surface)
// But this function do it carefully, it means it buffers the drawing if soem
// window is not buffer by itself
// pStartFromWindow parameter specifies the window who's children we'll test for buffering
// if NULL -> the desktop window is used
void CWindow::UpdateNowBuffered(CDDrawSurface *pDestination, CWindow *pStartFromWindow)
{
  CUpdateRectList::SRectNode *pNode;
  BOOL bBufferIt;
  CRect rcIntersect;

  // If not specified start window -> use the desktop
  if(pStartFromWindow == NULL)
    pStartFromWindow = g_pDesktopWindow;

  // Lock the update list
  VERIFY ( m_semaphoreUpdateList.Lock() );

  try{
  // go through it
  pNode = m_pUpdateList->GetFirstRectNode();
  while(pNode != NULL){
    // first go through children of the start window and determine if we'll use buffering or not

    bBufferIt = FALSE;
    {
      CWindow *pChild;
      pChild = pStartFromWindow->m_pFirstChild;
      while(pChild != NULL){
        // we'll test only children which intersects with given rectangle
        rcIntersect.IntersectRect(&(pNode->m_rcRect), &(pChild->m_rectWindow));
        if(!rcIntersect.IsRectEmpty()){
          // if the window is not buffer -> set the buffer to TRUE
          if(pChild->m_pGraphicalBuffer == NULL) bBufferIt = TRUE;
        }

        pChild = pChild->m_pNextSibling;
      }
    }

    if(bBufferIt){
      // If we're about to buffer it first draw all into the windows buffer
      g_pDesktopWindow->DoUpdateRect(m_pWindowsBufferSurface, &(pNode->m_rcRect));

      // Than paste it to the destination surface
      pDestination->Paste(pNode->m_rcRect.TopLeft(), m_pWindowsBufferSurface, &(pNode->m_rcRect));
    }
    else{
      // If the buffering is not neccessary draw it directly to the destination
      g_pDesktopWindow->DoUpdateRect(pDestination, &(pNode->m_rcRect));
    }

    // Get the next update node
    pNode = pNode->m_pNext;
  }

  // clear the list
  m_pUpdateList->RemoveAll();

  }
  catch(CException *)
  {
    VERIFY ( m_semaphoreUpdateList.Unlock() );
    throw;
  }
  // unlock it
  VERIFY ( m_semaphoreUpdateList.Unlock() );
}

// Clears the list of updates
// But doesn't draw anything
void CWindow::ValidateAll()
{
  VERIFY ( m_semaphoreUpdateList.Lock() );
  m_pUpdateList->Delete();
  VERIFY ( m_semaphoreUpdateList.Unlock() );
}