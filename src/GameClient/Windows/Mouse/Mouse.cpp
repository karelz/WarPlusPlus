// Mouse.cpp: implementation of the CMouse class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Mouse.h"
#include "..\Timer\Timer.h"
#include "..\Window\Window.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CMouse *g_pMouse = NULL; // Pointer to created mouse object
// there can't be more than one CMouse object so we can have
// a globa pointer to it

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CMouse, CNotifier);

BEGIN_OBSERVER_MAP(CMouse, CNotifier)
  BEGIN_TIMER()
    ON_TIMETICK()
  END_TIMER()
  BEGIN_ABORT()
    ON_ABORT()
  END_ABORT()
  BEGIN_NOTIFIER(MouseNotifierID)
    case 0:
    default:
      OnMouseEvent(dwEvent, dwParam);
      return TRUE;
  END_NOTIFIER()
END_OBSERVER_MAP(CMouse, CNotifier)

CMouse::CMouse()
{
  m_dwXCoord = 0; m_dwYCoord = 0;
  m_lBigX = 0; m_lBigY = 0;
  m_dwXSensitivity = 1; m_dwYSensitivity = 1;
  m_dwLButtonDownTime = 0; m_dwRButtonDownTime = 0;
  m_bLeftButtonDown = FALSE; m_bRightButtonDown = FALSE;

  m_pLastEventWindow = NULL;
  m_pMousePrimarySurface = NULL;
  m_pMouseWindow = NULL;
  m_pCaptureWindow = NULL;

  m_pActiveCursor = NULL;

  m_bExclusive = FALSE;

  m_pWaitingCursor = NULL;
}

CMouse::~CMouse()
{

}

DWORD CMouse::DoubleClickTime = 250;

//////////////////////////////////////////////////////////////////////
// Debug methods

#ifdef _DEBUG

void CMouse::AssertValid() const
{
  CNotifier::AssertValid();
}

void CMouse::Dump(CDumpContext &dc) const
{
  CNotifier::Dump(dc);
  dc << "Mouse position : " << m_dwXCoord << ", " << m_dwYCoord << "\n";
  dc << "Mouse sensitivity : " << m_dwXSensitivity << ", " << m_dwYSensitivity << "\n";
  dc << "Left button : " << m_bLeftButtonDown << ", Right button : " << m_bRightButtonDown << "\n";
}

#endif


//////////////////////////////////////////////////////////////////////
// Other methods

// static function which creates an instance of CMouse and
// assing it to the g_pMouse pointer and initialize it
BOOL CMouse::Init(CMouseWindow *pMouseWindow, BOOL bExclusive)
{
  ASSERT(g_pMouse == NULL);
  ASSERT_VALID(pMouseWindow);

  g_pMouse = new CMouse();

  // create the Mouse device
  if(!g_pMouse->Create(pMouseWindow, bExclusive)){
    TRACE("Failed to create mouse device.\n");
    ASSERT(FALSE);
    return FALSE;
  }

  // all was succesful
  // after you create the DirectDraw just call CMouse::CreateMouseSurface
  // and than you will be able to use mouse even in graphic
  return TRUE;
}

// static function which deletes the g_pMouse pointer
void CMouse::Close()
{
  // be carfull there has to be called CMouse::DeleteMouseSurface
  // before this call
  if(g_pMouse){
    g_pMouse->Delete();
    delete g_pMouse;
    g_pMouse = NULL;
  }
}

BOOL CMouse::Create(CMouseWindow *pMouseWindow, BOOL bExclusive)
{
  if(!CNotifier::Create()) return FALSE;

  m_pDefaultCursor = NULL;

  m_bExclusive = bExclusive;
  g_pTimer->Connect(this, 0);

#ifdef _DEBUG
  if(bExclusive){
    ASSERT(g_pDirectInput);
  }
#endif

  // connect us to us
  m_MouseNotifier.Connect(this, MouseNotifierID);

  // just create the device
  // there still can be no Primary surface or even no DirectDraw
  // to be visible on
  return CreateDIDevice(pMouseWindow);
}

void CMouse::Delete()
{
  m_MouseNotifier.Disconnect(this);

  CNotifier::Delete();

  if(g_pTimer)
    g_pTimer->Disconnect(this);

  // just delete the device
  DeleteDIDevice();
}

UINT CMouse::MouseThreadProc(LPVOID lpParam)
{
  CMouse *pMouse = (CMouse *)lpParam;
  DWORD dwWaitReturn;
  CSyncObject *pSyncObjects[2];
  pSyncObjects[0] = &pMouse->m_eventNotification;
  pSyncObjects[1] = &pMouse->m_eventClose;
  CMultiLock lock(pSyncObjects, 2);

  DWORD dwElements;
  DIDEVICEOBJECTDATA didod;
  DWORD dwSequence = 0x0FFFFFFFF;
  HRESULT hResult;
  BOOL bDone;

  SSequenceChanges sSC;

  while(TRUE){  // Infinite loop for waiting
    // Wait for one of those events
    dwWaitReturn = lock.Lock(INFINITE, FALSE);

    TRY{ // must catch all exceptions ( separeted thread )

    switch(dwWaitReturn){
    case WAIT_OBJECT_0 + 0:
      bDone = FALSE;

      pMouse->InitSequenceChanges(&sSC);

      while(!bDone){
        dwElements = 1;

        hResult = pMouse->m_MouseDevice.GetDeviceData(&didod, &dwElements);
        // If input lost we try to reacquire it
        if(hResult == DIERR_INPUTLOST){ 
          pMouse->m_MouseDevice.Acquire();
          continue;
        }
        // If some other error or there were no data in buffer
        // go on to wait for another event
        if((hResult != DI_OK) || (dwElements == 0)){
          // Do the changes we made
          pMouse->DoSequenceChanges(&sSC);
          // no need for reinit -> will be done after next wait

          dwSequence = didod.dwSequence;
          bDone = TRUE;
          continue;
        }

        // if the first sequence -> use the current seq
        if(dwSequence == 0xFFFFFFFF)
          dwSequence = didod.dwSequence;

        // if this is the first data don't care about changes
        // no changes can occure
        // if this is not the first and we reached a new sequence
        // than we'll look for some changes to take effect
        if(didod.dwSequence != dwSequence){
          // do the changes we made
          pMouse->DoSequenceChanges(&sSC);
          // reinit the structure
          // for example the coords are made to fit in screen
          pMouse->InitSequenceChanges(&sSC);

          dwSequence = didod.dwSequence;
        }

        switch(didod.dwOfs){
        case DIMOFS_BUTTON0:  // The left button state changed
          if(didod.dwData & 0x000000FF) sSC.m_bLeftButtonDown = TRUE;
          else sSC.m_bLeftButtonDown = FALSE;
          break;

        case DIMOFS_BUTTON1:  // The right button state changed
          if(didod.dwData & 0x000000FF) sSC.m_bRightButtonDown = TRUE;
          else sSC.m_bRightButtonDown = FALSE;
          break;

        case DIMOFS_BUTTON2:  // Some other buttons - don't care
        case DIMOFS_BUTTON3:
          break;

        case DIMOFS_X:  // The X coordinate changed
          sSC.m_lBigX += (LONG)(pMouse->m_dwXSensitivity * didod.dwData);
          break;
        case DIMOFS_Y:  // The Y coordinate changed
          sSC.m_lBigY += (LONG)(pMouse->m_dwYSensitivity * didod.dwData);
          break;
        case DIMOFS_Z:  // The Z coordinate changed - sorry no 3D
          break;
        default: // Some unknown data -> ignore it
          break;
        }
      }

      break;
    case WAIT_OBJECT_0 + 1: // an close event signaled -> exit thread
      return 0;
    default:
      TRACE("Waiting for mouse notification failed.\n");
      break;
    }

    }
    CATCH_ALL(e){
      TRACE("An exception occured\n");
      char txt[100]; txt[0] = 0;
      e->GetErrorMessage(txt, 100);
      TRACE("    %s\n", txt);
    }
    END_CATCH_ALL
  }

  return 1; // should never get here
}

CPoint CMouse::GetPosition()
{
  CPoint pt(m_dwXCoord, m_dwYCoord);
  return pt;
}

void CMouse::GetPosition(CPoint * pPt)
{
  ASSERT(pPt != NULL);
  
  pPt->x = m_dwXCoord; pPt->y = m_dwYCoord;
}

void CMouse::SetPosition(CPoint * pPt)
{
  ASSERT(pPt != NULL);
  
  CSingleLock lock(&m_semaphoreMouse);

  if(m_pMousePrimarySurface)
    m_pMousePrimarySurface->SetMousePos(pPt->x, pPt->y);  

  lock.Lock();
  m_dwXCoord = pPt->x; m_dwYCoord = pPt->y;
  lock.Unlock();
}

void CMouse::SetClipRect(CRect &rcClip)
{
  CRect rc = rcClip;
  if(m_pMousePrimarySurface){
    rc.IntersectRect(&rc, m_pMousePrimarySurface->GetScreenRect());
  }

  m_semaphoreMouse.Lock();

  m_rcClip = rc;

  m_semaphoreMouse.Unlock();
}

CRect CMouse::GetClipRect()
{
  return m_rcClip;
}

void CMouse::SetSensitivity(DWORD dwXSens, DWORD dwYSens)
{
  ASSERT(dwXSens > 0);
  ASSERT(dwYSens > 0);

  CSingleLock lock(&m_semaphoreMouse);
  lock.Lock();
  m_dwXSensitivity = dwXSens;
  m_dwYSensitivity = dwYSens;
  lock.Unlock();
}

void CMouse::GetSensitivity(LPDWORD pXSens, LPDWORD pYSens)
{
  ASSERT(pXSens != NULL);
  ASSERT(pYSens != NULL);

  *pXSens = m_dwXSensitivity;
  *pYSens = m_dwYSensitivity;
}

BOOL CMouse::LeftButtonDown()
{
  return m_bLeftButtonDown;
}

BOOL CMouse::RightButtonDown()
{
  return m_bRightButtonDown;
}

void CMouse::DoSequenceChanges(SSequenceChanges * pSC)
{
  // we have to permit only one thread in this function
  CSingleLock lock(&m_semaphoreMouse);
  lock.Lock();

  LONG lNewX, lNewY;
  // convert big numbers to small ones
  lNewX = pSC->m_lBigX / 4; lNewY = pSC->m_lBigY / 4;

  // If the mouse is to be outside the screen, return it there
  if(m_pMousePrimarySurface){
    if(lNewX < m_rcClip.left)
      lNewX = m_rcClip.left;
    if(lNewX > (m_rcClip.right - 1))
      lNewX = m_rcClip.right - 1;
    if(lNewY < m_rcClip.top)
      lNewY = m_rcClip.top;
    if(lNewY > (m_rcClip.bottom - 1))
      lNewY = m_rcClip.bottom - 1;
  }

  if((lNewX != (LONG)m_dwXCoord) || (lNewY != (LONG)m_dwYCoord)){
    // Position changed
    if(m_pMousePrimarySurface)
      m_pMousePrimarySurface->SetMousePos(lNewX, lNewY);

    m_dwXCoord = lNewX; m_dwYCoord = lNewY;
    m_lBigX = lNewX << 2; m_lBigY = lNewY << 2;

    m_MouseNotifier.InlayEvent(E_MOVE, MOUSE_MAKEPOINT(m_dwXCoord, m_dwYCoord), this);
  }

  if(m_bLeftButtonDown != pSC->m_bLeftButtonDown){
    // Left button state changed
    m_bLeftButtonDown = pSC->m_bLeftButtonDown;
    if(m_bLeftButtonDown){
      if(g_pTimer != NULL){
        if((g_pTimer->GetTime() - m_dwLButtonDownTime) <= DoubleClickTime){
          m_MouseNotifier.InlayEvent(E_LBUTTONDBLCLK, MOUSE_MAKEPOINT(m_dwXCoord, m_dwYCoord), this);
          m_dwLButtonDownTime = 0;
        }
        else{
          m_MouseNotifier.InlayEvent(E_LBUTTONDOWN, MOUSE_MAKEPOINT(m_dwXCoord, m_dwYCoord), this);
          m_dwLButtonDownTime = g_pTimer->GetTime();
        }
      }
      else{
        m_MouseNotifier.InlayEvent(E_LBUTTONDOWN, MOUSE_MAKEPOINT(m_dwXCoord, m_dwYCoord), this);
      }
    }
    else{
      m_MouseNotifier.InlayEvent(E_LBUTTONUP, MOUSE_MAKEPOINT(m_dwXCoord, m_dwYCoord), this);
    }
  }

  if(m_bRightButtonDown != pSC->m_bRightButtonDown){
    // Right button state changed
    m_bRightButtonDown = pSC->m_bRightButtonDown;
    if(m_bRightButtonDown){
      // determine if this was a double click or not
      if(g_pTimer != NULL){
        // a small trick - downtime is 0 for the first click
        // the system time is always bigger than doubleclick time
        // (it's imposibble to start the game so soon after the system
        // startup
        if((g_pTimer->GetTime() - m_dwRButtonDownTime) <= DoubleClickTime){
          m_MouseNotifier.InlayEvent(E_RBUTTONDBLCLK, MOUSE_MAKEPOINT(m_dwXCoord, m_dwYCoord), this);
          m_dwRButtonDownTime = 0;
        }
        else{
          m_MouseNotifier.InlayEvent(E_RBUTTONDOWN, MOUSE_MAKEPOINT(m_dwXCoord, m_dwYCoord), this);
          m_dwRButtonDownTime = g_pTimer->GetTime();
        }
      }
      else{
        m_MouseNotifier.InlayEvent(E_RBUTTONDOWN, MOUSE_MAKEPOINT(m_dwXCoord, m_dwYCoord), this);
      }
    }
    else{
      m_MouseNotifier.InlayEvent(E_RBUTTONUP, MOUSE_MAKEPOINT(m_dwXCoord, m_dwYCoord), this);
    }
  }

  // all is done - let another thread go in
  lock.Unlock();
}

void CMouse::InitSequenceChanges(SSequenceChanges * pSC)
{
  pSC->m_lBigX = m_lBigX;
  pSC->m_lBigY = m_lBigY;
  pSC->m_bLeftButtonDown = m_bLeftButtonDown;
  pSC->m_bRightButtonDown = m_bRightButtonDown;
}

void CMouse::Connect(CWindow * pWindow, DWORD dwNotID)
{
  CNotifier::Connect(pWindow, dwNotID);
}

BOOL CMouse::CreateDIDevice(CMouseWindow * pMouseWindow)
{
  DIPROPDWORD dipdw;
  
#ifdef _DEBUG
  if(m_bExclusive){
    ASSERT_VALID(g_pDirectInput);
  }
  ASSERT_VALID(pMouseWindow);
#endif

  m_pMouseWindow = pMouseWindow;

  if(m_bExclusive){
    // Create the mouse device
    if(!g_pDirectInput->CreateDevice(m_MouseDevice)) return FALSE;

    // Create the notification thread
    m_pNotificationThread = AfxBeginThread(CMouse::MouseThreadProc,
      (LPVOID)this, THREAD_PRIORITY_ABOVE_NORMAL);
    m_pNotificationThread->m_bAutoDelete = FALSE;

    // Set the buffer size
    dipdw.diph.dwSize = sizeof(dipdw);
    dipdw.diph.dwHeaderSize = sizeof(dipdw.diph);
    dipdw.diph.dwObj = 0;
    dipdw.diph.dwHow = DIPH_DEVICE;
    dipdw.dwData = 16; // Reasonable size of the mouse buffer
    m_MouseDevice.SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph);

    // Set the device notification
    m_MouseDevice.SetEventNotification((HANDLE)m_eventNotification);

    m_MouseDevice.Acquire();
  }

  // tell our window that the mouse object exists
  m_pMouseWindow->m_pMouse = this;

  return TRUE;
}

void CMouse::DeleteDIDevice()
{

  if(m_bExclusive){
    // set event that we are closing the shop
    // all child threads has to exit imidietly
    m_eventClose.SetEvent();
    // Wait for notification thread to exit
    // can't wait more than a few (2) seconds
    if(m_pNotificationThread != NULL){
      switch(::WaitForSingleObject(m_pNotificationThread->m_hThread, 2000)){
      case WAIT_OBJECT_0:
        // OK the thread exited so we can continue
        break;
      default:
        // something wrong happend
        // timeout or some other error occured
        // so we have to terminate the thread manualy ( not a clear way )
        ::TerminateThread(m_pNotificationThread->m_hThread, 0);
        break;
      }
      delete m_pNotificationThread;
      m_pNotificationThread = NULL;
    }
    
    if(m_MouseDevice.IsValid()){
      m_MouseDevice.Unacquire();
      m_MouseDevice.SetEventNotification(NULL);
    }
    
    m_MouseDevice.Delete();
  }

  if(m_pMouseWindow){
    m_pMouseWindow->m_pMouse = NULL;
  }
}

BOOL CMouse::CreateGraphicMouse()
{
  // The MousePrimarySurface has to be created before
  // the Mouse object
  if(g_pMousePrimarySurface == NULL){
    TRACE("Mouse object initialized with NULL DirectInput object.\n");
    ASSERT(FALSE);
    return FALSE; // In this case we have to exit at both,
                  //DEBUG and RELEASE
  }
  ASSERT_VALID(g_pMousePrimarySurface);

  // Just copy the Primary surface
  m_pMousePrimarySurface = g_pMousePrimarySurface;

  // connect to the abort notifier here
  // no sooner, because we have to be sure, that the DirectDraw
  // will get his AbortEvent later than us (we have some of its stuff)
  g_AbortNotifier.Connect(this);

  // Set the mouse position to the center of the screen
  m_dwXCoord = m_pMousePrimarySurface->GetWidth() / 2;
  m_dwYCoord = m_pMousePrimarySurface->GetHeight() / 2;
  // These variables will be used to store the position perfectly
  m_lBigX = m_dwXCoord << 2;
  m_lBigY = m_dwYCoord << 2;

  // acquire the device only if we are in the exclusive mode
  if(m_bExclusive)
    this->m_MouseDevice.Acquire();

  return TRUE;
}

void CMouse::DeleteGraphicMouse()
{
  // no primary surface to be visible on
  m_pMousePrimarySurface = NULL;

  g_AbortNotifier.Disconnect(this);
}

BOOL CMouse::CreateMouseSurface(DWORD dwBackBufferCount)
{
  // create our primary surface
  if(!CMousePrimarySurface::Init((m_bExclusive) ? NULL : m_pMouseWindow, dwBackBufferCount)) return FALSE;

  // this time, there has to be some DirectDraw and also
  // some primary surface to be visible on
  if(!CreateGraphicMouse()) return FALSE;

  // set the clipping rectangle
  m_rcClip = *m_pMousePrimarySurface->GetScreenRect();

  return TRUE;
}

void CMouse::DeleteMouseSurface()
{
  // just remove us from the screen
  // the DirectDraw is about to be deleted
  DeleteGraphicMouse();

  CMousePrimarySurface::Close();
}

void CMouse::ShowMouse()
{
  if(m_bExclusive) m_MouseDevice.Acquire();
  if(m_pMousePrimarySurface) m_pMousePrimarySurface->ShowMouse();
}

void CMouse::HideMouse()
{
  if(m_pMousePrimarySurface) m_pMousePrimarySurface->HideMouse();
}

CWindow * CMouse::SetCapture(CWindow * pNew)
{
  CWindow *pOldOne = m_pCaptureWindow;

  m_pCaptureWindow = pNew;

  if(!m_bExclusive){
    // if this was the first time to capture the mouse
    // -> set the capture to our window
    if((pOldOne == NULL) && (pNew != NULL))
      m_pMouseWindow->SetCapture();

    // if this was the last time to release the capture
    // -> release the capture from our window
    if((pNew == NULL) && (pOldOne != NULL))
      ReleaseCapture();
  }

  return pOldOne;
}

CWindow * CMouse::GetCapture()
{
  return m_pCaptureWindow;
}

void CMouse::OnTimeTick(DWORD dwTime)
{
  if(m_pActiveCursor)
    if(m_pActiveCursor->OnTimeTick(dwTime)){
      UpdateCursor();
    }
}

void CMouse::SetCursor(CCursor *pCursor)
{
#ifdef _DEBUG
  if(pCursor != NULL) ASSERT_VALID(pCursor);
#endif

  m_pActiveCursor = pCursor;
  if(m_pActiveCursor == NULL) m_pActiveCursor = m_pDefaultCursor;

  if(m_pActiveCursor)
    g_pTimer->SetElapseTime(this, m_pActiveCursor->GetElapseTime());
  else
    g_pTimer->SetElapseTime(this, 0);

  UpdateCursor();
}

void CMouse::UpdateCursor()
{
  if(!m_pActiveCursor){
    m_pMousePrimarySurface->SetMouseCursor(NULL, 0, 0);
  }
  else{
    if(!m_pMouseWindow->m_bActive) return;
    if(m_pMousePrimarySurface){
      m_pMousePrimarySurface->SetMouseCursor(m_pActiveCursor->GetCursorBitmap(),
        m_pActiveCursor->GetHotX(), m_pActiveCursor->GetHotY());
    }
  }
}

void CMouse::OnAbort(DWORD dwExitCode)
{
  // if there is mouse surface -> destroy it too
  if(m_pMousePrimarySurface){
    DeleteMouseSurface();
  }
  Delete();
}

CCursor * CMouse::SetDefaultCursor(CCursor *pCursor)
{
  CCursor *pOldOne = m_pDefaultCursor;

  m_pDefaultCursor = pCursor;

  return pOldOne;
}

void CMouse::OnMouseEvent(DWORD dwEvent, DWORD dwParam)
{
  // if there is some capture window -> all events to it
  if(m_pCaptureWindow != NULL){
    m_pLastEventWindow = m_pCaptureWindow;
  }
  else{
    // elseway we have to find the window for this message

    CPoint pt(MOUSE_X(dwParam), MOUSE_Y(dwParam));
    CWindow *pPrevEventWindow;
    CCursor *pCursor;

    pPrevEventWindow = m_pLastEventWindow;
    m_pLastEventWindow = NULL;
    if(g_pDesktopWindow == NULL) return;
    m_pLastEventWindow = g_pDesktopWindow->WindowFromPoint(pt);
    // if the window under the cursor changed we have to send
    // the leave event
    // and also we have to change the cursor for the new window
    if((dwEvent == E_MOVE) && (pPrevEventWindow != NULL) &&
      (pPrevEventWindow != m_pLastEventWindow)){
      
      // send the event only if it's enabled
      if(pPrevEventWindow->m_bInternalyEnabled)
        // inlay the leave event for the old window
        InlayEvent(E_LEAVE, 0, pPrevEventWindow);
    }

    if(dwEvent == E_MOVE){
      // if the window has its own cursor set it
      // if not try to set the default one
      // if there is no such one do nothing and leave the cursor
      // as it was

      if((m_pLastEventWindow != NULL) && (m_pLastEventWindow->m_bInternalyEnabled))
        pCursor = m_pLastEventWindow->m_pCursor;
      else
        pCursor = m_pDefaultCursor;

      if(pCursor == NULL) pCursor = m_pDefaultCursor;
      if((pCursor != m_pActiveCursor) && (pCursor != NULL)){
        SetCursor(pCursor);
      }
    }
  }

  if ( m_pLastEventWindow == NULL )
  {
	  // Ooops no window to send the event to...
	  // forget it
      return;
  }

  // if we have a doubleclick and the dest window doesn't
  // accept doubleclicks -> change it to normal click
  if(!m_pLastEventWindow->m_bDoubleClk){
    if(dwEvent == E_LBUTTONDBLCLK){
      dwEvent = E_LBUTTONDOWN;
    }
    if(dwEvent == E_RBUTTONDBLCLK){
      dwEvent = E_RBUTTONDOWN;  
    }
  }
  // for clicks (only the ones down and double clicks)
  // before sending the notification we have to activate
  // the affected window
  if((dwEvent == E_LBUTTONDOWN) || (dwEvent == E_RBUTTONDOWN)
    || (dwEvent == E_LBUTTONDBLCLK) || (dwEvent == E_RBUTTONDBLCLK)){
    BOOL bSendEvent = FALSE;
    if(m_pLastEventWindow->m_bInternalyEnabled){
      bSendEvent = m_pLastEventWindow->Activate();
      m_pLastEventWindow->TrySetFocus();
    }
    if(!bSendEvent) return;
  }

  // transform coordinates to the window
  dwParam = MOUSE_MAKEPOINT(MOUSE_X(dwParam) - m_pLastEventWindow->m_rectWindow.left,
    MOUSE_Y(dwParam) - m_pLastEventWindow->m_rectWindow.top);

  // only if it's enabled
  if(m_pLastEventWindow->m_bInternalyEnabled){
    // send the event
    SendEvent(dwEvent, dwParam, m_pLastEventWindow);
  }
}

void CMouse::Disconnect(CObserver *pObserver)
{
  if(pObserver == (CObserver *)m_pLastEventWindow){
    m_pLastEventWindow = NULL;
  }
  CNotifier::Disconnect(pObserver);
}

void CMouse::RestoreCursor()
{
  if(g_pDesktopWindow == NULL){
    SetCursor(m_pDefaultCursor);
    return;
  }
  CWindow::FreezeWindows();
  CWindow *pWindow = g_pDesktopWindow->WindowFromPoint(GetPosition());
  CWindow::UnfreezeWindows();
  if(pWindow->m_pCursor == NULL){
    SetCursor(m_pDefaultCursor);
  }
  else{
    SetCursor(pWindow->m_pCursor);
  }
}