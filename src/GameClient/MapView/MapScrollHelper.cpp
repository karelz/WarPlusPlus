// MapScrollHelper.cpp: implementation of the CMapScrollHelper class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MapScrollHelper.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CMapScrollHelper, CWindow);

BEGIN_OBSERVER_MAP(CMapScrollHelper, CWindow)
  BEGIN_MOUSE()
    ON_MOUSEMOVE()
    ON_MOUSELEAVE()
  END_MOUSE()

  BEGIN_TIMER()
    ON_TIMETICK()
  END_TIMER()
END_OBSERVER_MAP(CMapScrollHelper, CWindow)

CMapScrollHelper::CMapScrollHelper()
{
  m_bTimerConnected = FALSE;
}

CMapScrollHelper::~CMapScrollHelper()
{
  ASSERT(m_bTimerConnected == FALSE);
}

#ifdef _DEBUG

void CMapScrollHelper::AssertValid() const
{
  CWindow::AssertValid();
}

void CMapScrollHelper::Dump(CDumpContext &dc) const
{
  CWindow::Dump(dc);
}

#endif

void CMapScrollHelper::Create(CRect &rcWindow, CWindow *pParent, DWORD dwTimeDelay)
{
  m_dwTimeDelay = dwTimeDelay;

  m_bTransparent = TRUE;
  m_bTopMost = TRUE;

  VERIFY(CWindow::Create(&rcWindow, pParent));
}

void CMapScrollHelper::Delete()
{
  if(m_bTimerConnected){
    g_pTimer->Disconnect(this);
    m_bTimerConnected = FALSE;
  }

  CWindow::Delete();
}

void CMapScrollHelper::OnMouseMove(CPoint pt)
{
  if(!m_bTimerConnected){
    InlayEvent(E_SCROLLEVENT, 0);
    g_pTimer->Connect(this, m_dwTimeDelay);
    m_bTimerConnected = TRUE;
  }
}

void CMapScrollHelper::OnMouseLeave()
{
  if(m_bTimerConnected){
    g_pTimer->Disconnect(this);
    m_bTimerConnected = FALSE;
  }
}

void CMapScrollHelper::OnTimeTick(DWORD dwTime)
{
  if(m_bTimerConnected){
    if(g_pMainWindow != NULL){
      if(!g_pMainWindow->IsActive()) return;
    }
    InlayEvent(E_SCROLLEVENT, 0);
  }
}