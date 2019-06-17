// DialogWindow.cpp: implementation of the CDialogWindow class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DialogWindow.h"
#include "..\Controls\Layouts.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CDialogWindow, CCaptionWindow);

BEGIN_OBSERVER_MAP(CDialogWindow, CCaptionWindow)
  BEGIN_NOTIFIER(CDialogWindow::IDC_CANCEL)
    ON_BUTTONRELEASED(OnCancel)
  END_NOTIFIER()
END_OBSERVER_MAP(CDialogWindow, CCaptionWindow)

CDialogWindow::CDialogWindow()
{
  m_pOwner = NULL;
  m_pLayout = NULL;
  m_pEventManager = NULL;
}

CDialogWindow::~CDialogWindow()
{
}

#ifdef _DEBUG

void CDialogWindow::AssertValid() const
{
  CCaptionWindow::AssertValid();
}

void CDialogWindow::Dump(CDumpContext &dc) const
{
  CCaptionWindow::Dump(dc);
}

#endif


BOOL CDialogWindow::Create(CRect &rcBound, CString strCaption, CDialogWindowLayout *pLayout, CWindow *pParent)
{
  if(pLayout == NULL){
    m_pLayout = CLayouts::m_pDefaults->GetDialogWindowLayout();
  }
  else{
    ASSERT_VALID(pLayout);
    m_pLayout = pLayout;
  }

  // set our window as topmost
  m_bTopMost = TRUE;

  // remeber the owner (given as parent)
  m_pOwner = pParent;

  // don't have the tab stop - no keyboard input for us
  m_bTabStop = FALSE;

  // create the dialog window
  if(!CCaptionWindow::Create(rcBound, strCaption, m_pLayout, g_pDesktopWindow, FALSE)) return FALSE;

  // first create the close button
  {
    // create the point and button itself there
    CPoint pt(rcBound.Width() - m_pLayout->m_dwCloseRightMargin,
      m_pLayout->m_dwCloseTopMargin);
    m_CloseButton.Create(pt, &(m_pLayout->m_CloseButtonLayout), this, FALSE);
//    CRect rcSensitive;
//    rcSensitive.l

    // set the accelerator to Esc
    m_CloseButton.SetKeyAcc(VK_ESCAPE, 0);

    // connect it
    m_CloseButton.Connect(this, IDC_CANCEL);
  }

  // call the InitDialog function
  InitDialog();

  // activate us
  Activate();

  // set the focus to the first control in dialog
  // it's same as set focus to the first child window
  // if it doesn't have the TabStop flag set, this functin will find first next child
  // which does have
  TrySetFocus();

  return TRUE;
}

void CDialogWindow::Delete()
{
  m_CloseButton.Delete();
  m_pLayout = NULL;
  CCaptionWindow::Delete();
}

void CDialogWindow::InitDialog()
{
  // here do nothing
}

DWORD CDialogWindow::DoLoop()
{
  DWORD dwRet;

  // first we have to determine the event manager for this thread
  CEventManager *pEventManager = CEventManager::FindEventManager(GetCurrentThreadId());

  // then disable the owner window
  if(m_pOwner){
    m_pOwner->EnableWindow(FALSE);
  }
  // internaly enable us
  // we could be disabled by the previous action
  InternalEnableWindow(TRUE);

  // say that we're starting the loop for this manager
  m_pEventManager = pEventManager;

  // start our own message loop
  // remeber the returned value - it's the value of the dialog end cause
  dwRet = pEventManager->DoEventLoop();

  // we've done looping for this manager
  m_pEventManager = NULL;

  // enable the owner
  if(m_pOwner){
    m_pOwner->EnableWindow(TRUE);
  }

  // OK we've done
  return dwRet;
}

void CDialogWindow::EndDialog(DWORD dwCode)
{
  // we have to determine, if the dialog was in modal state
  // if so -> inlay the quit event
  if(m_pEventManager){
    m_pEventManager->InlayQuitEvent(dwCode);
  }
}

DWORD CDialogWindow::DoModal(CRect &rcBound, CString strCaption, CDialogWindowLayout *pLayout, CWindow *pParent)
{
  DWORD dwRet;

  // create the dialog
  Create(rcBound, strCaption, pLayout, pParent);

  // do the loop
  dwRet = DoLoop();

  // delete the dialog
  Delete();

  return dwRet;
}

DWORD CDialogWindow::DoModal(CWindow *pParent)
{
  // use defaults
  return DoModal(GetDefaultPosition(), GetDefaultCaption(), NULL, pParent);
}

DWORD CDialogWindow::DoModal()
{
  // were we created ?
  ASSERT_VALID(m_pLayout);

  // just do the loop
  return DoLoop();
}

void CDialogWindow::OnCancel()
{
  // if cancel just end the dialog with apropriate code
  EndDialog(IDC_CANCEL);
}

CRect CDialogWindow::GetDefaultPosition()
{
  return CRect(50, 50, 300, 200);
}

CString CDialogWindow::GetDefaultCaption()
{
  return CString();
}

CRect CDialogWindow::GetCenterPosition(CSize size)
{
  CRect rc;
  rc.left = (g_pDDPrimarySurface->GetScreenRect()->Width() - size.cx) / 2;
  rc.right = rc.left + size.cx;
  rc.top = (g_pDDPrimarySurface->GetScreenRect()->Height() - size.cy) / 2;
  rc.bottom = rc.top + size.cy;

  return rc;
}
