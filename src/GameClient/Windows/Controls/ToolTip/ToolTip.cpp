// ToolTip.cpp: implementation of the CToolTip class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ToolTip.h"

#include "..\layouts.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CToolTip, CWindow);

CToolTip::CToolTip()
{
  m_pLayout = NULL;
  m_pOwner = NULL;
  m_bVisible = FALSE;
  m_pParentWindow = NULL;
}

CToolTip::~CToolTip()
{
  ASSERT(m_pOwner == NULL);
  ASSERT(m_pParentWindow == NULL);
}

#ifdef _DEBUG

void CToolTip::AssertValid() const
{
  ASSERT_VALID(m_pLayout);
  ASSERT_VALID(m_pOwner);
  ASSERT_VALID(m_pParentWindow);
  CWindow::AssertValid();
}

void CToolTip::Dump(CDumpContext &dc) const
{
  CWindow::Dump(dc);
}

#endif


BOOL CToolTip::Create(CString strText, CToolTipLayout *pLayout, CWindow *pParent, CWindow *pDesktopWindow)
{
  // copy layout - if none -> default
  if(pLayout == NULL) m_pLayout = CLayouts::m_pDefaults->GetToolTipLayout();
  else m_pLayout = pLayout;
  ASSERT_VALID(m_pLayout);

  // copy the text
  m_strText = strText;

  if(pDesktopWindow == NULL) pDesktopWindow = g_pDesktopWindow;
  ASSERT_VALID(pDesktopWindow);
  m_pParentWindow = pDesktopWindow;
    
  // set the owner window (visible as parent)
  ASSERT_VALID(pParent);
  m_pOwner = pParent;

  // visible to FALSE
  m_bVisible = FALSE;

  // create the buffer
  m_Buffer.SetWidth(BufferWidth);
  m_Buffer.SetHeight(m_pLayout->GetFont()->GetCharSize('A').cy + m_pLayout->m_ptTextOffset.y + m_pLayout->m_sizeRBMargin.cy);
  m_Buffer.SetAlphaChannel(TRUE);
  m_Buffer.Create();

  // fill it and set the alpha channel
  m_Buffer.Fill(m_pLayout->m_dwBackgroundColor);
  memset(m_Buffer.GetAlphaChannel(), m_pLayout->m_nOpaque, m_Buffer.GetWidth() * m_Buffer.GetHeight());
  m_Buffer.ReleaseAlphaChannel(TRUE);

  return TRUE;
}

void CToolTip::Delete()
{
  if(m_bVisible){
    CWindow::Delete();
    m_bVisible = FALSE;
  }
  // remove the buffer
  m_Buffer.Delete();

  // clear variables
  m_pLayout = NULL;
  m_pOwner = NULL;
  m_pParentWindow = NULL;
}

void CToolTip::SetText(CString strText)
{
  BOOL bShow = m_bVisible;

  // hide us
  if(bShow) Hide();

  // copy the text
  m_strText = strText;

  // show us if we were visible
  if(bShow) Show();
}

void CToolTip::Hide()
{
  ASSERT_VALID(this);

  if(!m_bVisible) return;

  // delete the window -> hide us
  CWindow::Delete();

  m_bVisible = FALSE;
}

void CToolTip::Show()
{
  ASSERT_VALID(this);

  if(m_bVisible) return;

  // compute the window position
  CRect rcWindow;
  rcWindow.left = m_pOwner->GetWindowScreenPosition().left;
  rcWindow.top = m_pOwner->GetWindowScreenPosition().bottom;

  // and its size
  CSize sizeText = m_pLayout->GetFont()->GetTextSize(m_strText);
  rcWindow.right = rcWindow.left + sizeText.cx + m_pLayout->m_ptTextOffset.x + m_pLayout->m_sizeRBMargin.cx;
  rcWindow.bottom = rcWindow.top + sizeText.cy + m_pLayout->m_ptTextOffset.y + m_pLayout->m_sizeRBMargin.cy;

  // move the tooltip somewhere else if needed
  CRect rcDesktop = g_pDesktopWindow->GetWindowPosition();
  CSize sz = rcWindow.Size();
  if(rcWindow.bottom > rcDesktop.bottom){
    rcWindow.bottom = m_pOwner->GetWindowScreenPosition().top;
    rcWindow.top = rcWindow.bottom - sz.cy;
  }
  if(rcWindow.right > rcDesktop.right){
    rcWindow.right = rcDesktop.right;
    rcWindow.left = rcWindow.right - sz.cx;
  }
  
  ASSERT_VALID(g_pDesktopWindow);

  // set some flags
  m_bTransparent = TRUE;
  m_bTopMost = TRUE;

  // create the window
  CWindow::Create(&rcWindow, m_pParentWindow);

  m_bVisible = TRUE;
}

void CToolTip::Draw(CDDrawSurface *pSurface, CRect *pRect)
{
  int nLeft, nPos;

  // first draw the background
  nLeft = GetWindowPosition().Width();
  nPos = 0;
  while(nLeft > 0){
    pSurface->Paste(nPos, 0, &m_Buffer);
    nLeft -= m_Buffer.GetWidth();
    nPos += m_Buffer.GetWidth();
  }

  // than the text
  m_pLayout->GetFont()->PaintText(m_pLayout->m_ptTextOffset.x, m_pLayout->m_ptTextOffset.y,
    m_strText, pSurface, m_pLayout->m_dwTextColor);
}

void CToolTip::OnDeactivate()
{
  Hide();
}
