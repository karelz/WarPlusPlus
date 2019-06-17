// TextButton.cpp: implementation of the CTextButton class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TextButton.h"
#include "..\Layouts.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CTextButton, CButtonObject)

BEGIN_OBSERVER_MAP(CTextButton, CButtonObject)
  BEGIN_ANIMS()
    ON_ANIMSREPAINT()
  END_ANIMS()
END_OBSERVER_MAP(CTextButton, CButtonObject)

CTextButton::CTextButton()
{
}

CTextButton::~CTextButton()
{
}


#ifdef _DEBUG

void CTextButton::AssertValid() const
{
  CButtonObject::AssertValid();
}

void CTextButton::Dump(CDumpContext &dc) const
{
  CButtonObject::Dump(dc);
}

#endif



BOOL CTextButton::Create(CPoint ptPosition, CTextButtonLayout *pLayout, CString strText, CWindow *pParent, BOOL bRepeate)
{
  if(pLayout == NULL){
    m_pLayout = CLayouts::m_pDefaults->GetTextButtonLayout();
  }
  else{
    m_pLayout = pLayout;
  }

  m_strText = strText;

  CSize sizeText = m_pLayout->GetFont()->GetTextSize(strText);
  if(sizeText.cx < (int)(m_pLayout->m_dwMinimalTextWidth)) sizeText.cx = m_pLayout->m_dwMinimalTextWidth;
  m_sizeWhole.cx = m_pLayout->m_ptPassiveTextOrig.x + sizeText.cx + m_pLayout->m_dwPassiveRightMargin;
  m_sizeWhole.cy = m_pLayout->m_pPassiveLeft->GetSize().cy;
  m_sizeWhole.cx -= m_pLayout->m_sizeVirtual.cx;
  if(m_sizeWhole.cx < 1) m_sizeWhole.cx = 1;

  CRect rc;
  rc.left = ptPosition.x; rc.top = ptPosition.y;
  rc.right = rc.left + m_sizeWhole.cx + m_pLayout->m_sizeVirtual.cx; rc.bottom = rc.top + m_sizeWhole.cy;
  if(!Create(&rc, pLayout, strText, pParent, bRepeate)) return FALSE;

  return TRUE;
}

BOOL CTextButton::Create(CRect *pRect, CTextButtonLayout *pLayout, CString strText, CWindow *pParent, BOOL bRepeate)
{
  if(pLayout == NULL){
    m_pLayout = CLayouts::m_pDefaults->GetTextButtonLayout();
  }
  else{
    m_pLayout = pLayout;
  }

  m_strText = strText;

  CRect rcBound(pRect);
  rcBound.bottom = rcBound.top + m_pLayout->m_pPassiveLeft->GetSize().cy;

  // set the transparency
  if(m_pLayout->AreAnimsTransparent()) m_bTransparent = TRUE;
  else m_bTransparent = FALSE;

  // we want the keyboard input
  m_bTabStop = TRUE;
  // but no doubleclicks
  m_bDoubleClk = FALSE;

  m_sizeWhole = rcBound.Size();
  m_sizeWhole.cx -= m_pLayout->m_sizeVirtual.cx;
  if(m_sizeWhole.cx < 1) m_sizeWhole.cx = 1;

  SetWindowCursor(m_pLayout->m_pCursor);

  ChooseLayout();
  ComputePositions();

  return CButtonObject::Create(&rcBound, pParent, bRepeate);
}

void CTextButton::Delete()
{
  m_Left.Stop();
  m_Middle.Stop();
  m_Right.Stop();

  CButtonObject::Delete();
}

void CTextButton::Draw(CDDrawSurface *pSurface, CRect *pRect)
{
  DWORD dwLeft;
  CRect rc;
  CPoint pt;

  pSurface->Paste(m_Left.GetRect()->TopLeft(), m_Left.Frame());
  pSurface->Paste(m_Right.GetRect()->TopLeft(), m_Right.Frame());

  rc = *(m_Middle.Frame()->GetAllRect());
  pt = m_Middle.GetRect()->TopLeft();
  dwLeft = m_sizeMiddle.cx; if(dwLeft < 0) dwLeft = 0;
  while(dwLeft > 0){
    if(dwLeft < (DWORD)rc.Width()){
      rc.right = rc.left + dwLeft;
    }
    pSurface->Paste(pt, m_Middle.Frame(), &rc);
    pt.x += rc.Width();
    dwLeft -= rc.Width();
  }

  m_pLayout->GetFont()->PaintText(m_ptTextPos.x, m_ptTextPos.y, m_strText,
    pSurface, m_dwTextColor);
}

void CTextButton::ComputePositions()
{
  CSize sizeText = m_pLayout->GetFont()->GetTextSize(m_strText);
  CPoint ptRightPos, ptMiddlePos;

  DWORD dwTextWidth = (m_sizeWhole.cx - m_pLayout->m_dwPassiveRightMargin
    - m_pLayout->m_ptPassiveTextOrig.x);
  if(dwTextWidth < (DWORD)sizeText.cx) m_ptTextPos.x = 0;
  else m_ptTextPos.x = (dwTextWidth - sizeText.cx) / 2;

  if(m_bPressed){
    m_ptTextPos.x += m_pLayout->m_ptPressedTextOrig.x;
    m_ptTextPos.y = m_pLayout->m_ptPressedTextOrig.y;
  }
  else{
    if(m_bMouseActive){
      m_ptTextPos.x += m_pLayout->m_ptActiveTextOrig.x;
      m_ptTextPos.y = m_pLayout->m_ptActiveTextOrig.y;
    }
    else{
      m_ptTextPos.x += m_pLayout->m_ptPassiveTextOrig.x;
      m_ptTextPos.y = m_pLayout->m_ptPassiveTextOrig.y;
    }
  }

  ptRightPos.x = m_sizeWhole.cx - m_Right.GetSize().cx;
  ptRightPos.y = 0;
  ptMiddlePos.x = m_Left.GetSize().cx;
  ptMiddlePos.y = 0;
  m_sizeMiddle.cx = ptRightPos.x - ptMiddlePos.x;
  if(m_sizeMiddle.cx < 0) m_sizeMiddle.cx = 0;
  m_sizeMiddle.cy = m_Middle.GetSize().cy;

  m_Left.SetRect(CPoint(0, 0));
  m_Middle.SetRect(CRect(ptMiddlePos, m_sizeMiddle));
  m_Right.SetRect(ptRightPos);
}

void CTextButton::ChooseLayout()
{
  m_Left.Stop();
  m_Middle.Stop();
  m_Right.Stop();

  if(m_bPressed){
    m_Left.Create(m_pLayout->m_pPressedLeft, this);
    m_Middle.Create(m_pLayout->m_pPressedMiddle, this);
    m_Right.Create(m_pLayout->m_pPressedRight, this);

    m_dwTextColor = m_pLayout->m_dwPressedTextColor;
  }
  else{
    if(m_bMouseActive){
      if(HasFocus()){
        m_Left.Create(m_pLayout->m_pActiveKeyLeft, this);
        m_Middle.Create(m_pLayout->m_pActiveKeyMiddle, this);
        m_Right.Create(m_pLayout->m_pActiveKeyRight, this);
      }
      else{
        m_Left.Create(m_pLayout->m_pActiveLeft, this);
        m_Middle.Create(m_pLayout->m_pActiveMiddle, this);
        m_Right.Create(m_pLayout->m_pActiveRight, this);
      }
      m_dwTextColor = m_pLayout->m_dwActiveTextColor;
    }
    else{
      if(HasFocus()){
        m_Left.Create(m_pLayout->m_pPassiveKeyLeft, this);
        m_Middle.Create(m_pLayout->m_pPassiveKeyMiddle, this);
        m_Right.Create(m_pLayout->m_pPassiveKeyRight, this);
      }
      else{
        m_Left.Create(m_pLayout->m_pPassiveLeft, this);
        m_Middle.Create(m_pLayout->m_pPassiveMiddle, this);
        m_Right.Create(m_pLayout->m_pPassiveRight, this);
      }
      m_dwTextColor = m_pLayout->m_dwPassiveTextColor;
    }
  }

  m_Left.Play();
  m_Middle.Play();
  m_Right.Play();
}

void CTextButton::OnAnimsRepaint(CAnimationInstance *pAnimation)
{
  UpdateRect(pAnimation->GetRect());
}

void CTextButton::OnMouseActivate()
{
  if(m_pLayout->m_pHoverSound)
    m_pLayout->m_pHoverSound->Play();
  CButtonObject::OnMouseActivate();
}

void CTextButton::OnMouseDeactivate()
{
  if(m_pLayout->m_pLeaveSound)
    m_pLayout->m_pLeaveSound->Play();
  CButtonObject::OnMouseDeactivate();
}

void CTextButton::OnLayoutPress()
{
  if(m_pLayout->m_pPressSound)
    m_pLayout->m_pPressSound->Play();
  CButtonObject::OnLayoutPress();
}

void CTextButton::OnLayoutRelease()
{
  if(m_pLayout->m_pReleaseSound)
    m_pLayout->m_pReleaseSound->Play();
  CButtonObject::OnLayoutRelease();
}

void CTextButton::OnLayoutChange()
{
  ChooseLayout(); ComputePositions(); UpdateRect();
}

CString CTextButton::GetText()
{
  return m_strText;
}

void CTextButton::SetText(CString strText)
{
  m_strText = strText;

  ComputePositions();
  UpdateRect();
}

void CTextButton::OnSize(CSize size)
{
  m_sizeWhole = size;
  m_sizeWhole.cx -= m_pLayout->m_sizeVirtual.cx;
  if(m_sizeWhole.cx < 1) m_sizeWhole.cx = 1;
  ComputePositions();
}

CSize CTextButton::GetVirtualSize()
{
  ASSERT_VALID(m_pLayout);
  CSize size;
  size.cx = GetWindowPosition().Width() + 2 * m_pLayout->m_sizeVirtual.cx;
  size.cy = GetWindowPosition().Height() + m_pLayout->m_sizeVirtual.cy;
  return size;
}
