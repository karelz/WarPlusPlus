// CaptionWindow.cpp: implementation of the CCaptionWindow class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CaptionWindow.h"
#include "..\Controls\Layouts.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CCaptionWindow, CFrameWindow)

CCaptionWindow::CCaptionWindow()
{

}

CCaptionWindow::~CCaptionWindow()
{

}

#ifdef _DEBUG

void CCaptionWindow::AssertValid() const
{
  CFrameWindow::AssertValid();
}

void CCaptionWindow::Dump(CDumpContext &dc) const
{
  CFrameWindow::Dump(dc);
}

#endif


BOOL CCaptionWindow::Create(CRect &rcBound, CString strCaption, CCaptionWindowLayout *pLayout, CWindow *pParent, BOOL bResizeable)
{
  if(pLayout == NULL){
    m_pCaptionLayout = CLayouts::m_pDefaults->GetCaptionWindowLayout();
  }
  else{
    ASSERT_VALID(pLayout);
    m_pCaptionLayout = pLayout;
  }
  if(!CFrameWindow::Create(rcBound, m_pCaptionLayout, pParent, bResizeable)) return FALSE;

  m_strCaption = strCaption;

  return TRUE;
}

void CCaptionWindow::Delete()
{
  CFrameWindow::Delete();
}

void CCaptionWindow::Draw(CDDrawSurface *pSurface, CRect *pRect)
{
  CFrameWindow::Draw(pSurface, pRect);

  CPoint pt;
  DWORD dwColor;

  pt.y = m_pCaptionLayout->m_ptCaptionPos.y;
  pt.x = m_pCaptionLayout->m_ptCaptionPos.x +
    (GetWindowPosition().Width() - m_pCaptionLayout->m_nRightMargin
    - m_pCaptionLayout->m_ptCaptionPos.x - m_pCaptionLayout->GetFont()->GetTextSize(m_strCaption).cx) / 2;

  if(m_bActiveFrame) dwColor = m_pCaptionLayout->m_dwACaptionColor;
  else dwColor = m_pCaptionLayout->m_dwPCaptionColor;

  m_pCaptionLayout->GetFont()->PaintText(pt.x, pt.y, m_strCaption, pSurface, dwColor);
}

CString CCaptionWindow::GetCaption()
{
  return m_strCaption;
}

void CCaptionWindow::SetCaption(CString strCaption)
{
  m_strCaption = strCaption;
  UpdateRect();
}

void CCaptionWindow::OnDeactivate()
{
  UpdateRect();
  CFrameWindow::OnDeactivate();
}
