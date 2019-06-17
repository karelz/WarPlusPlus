// ImageButton.cpp: implementation of the CImageButton class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ImageButton.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CImageButton, CButtonObject)

BEGIN_OBSERVER_MAP(CImageButton, CButtonObject)
  BEGIN_ANIMS()
    ON_ANIMSREPAINT()
  END_ANIMS()
END_OBSERVER_MAP(CImageButton, CButtonObject)

CImageButton::CImageButton()
{
}

CImageButton::~CImageButton()
{
}

#ifdef _DEBUG

void CImageButton::AssertValid() const
{
  CButtonObject::AssertValid();
}

void CImageButton::Dump(CDumpContext &dc) const
{
  CButtonObject::Dump(dc);
}

#endif


BOOL CImageButton::Create(CPoint ptPosition, CImageButtonLayout *pLayout, CWindow *pParent, BOOL bRepeate, CWindow *pDesktopWindow)
{
  ASSERT_VALID(pLayout);

  m_pLayout = pLayout;

  // set the transparency
  if(m_pLayout->AreAnimsTransparent()) m_bTransparent = TRUE;
  else m_bTransparent = FALSE;

  // no keyboard for this type of button
  m_bTabStop = FALSE;
  m_bDoubleClk = FALSE;

  // compute the size of our window (the size of the passive animation)
  CSize size;
  size.cx = m_pLayout->m_pPassive->GetSize().cx;
  size.cy = m_pLayout->m_pPassive->GetSize().cy;
  CRect rcBound(ptPosition, size);

  // set the cursor
  SetWindowCursor(m_pLayout->m_pCursor);

  // set the layout
  ChooseLayout();

  // create the button object (and the window)
  if(!CButtonObject::Create(&rcBound, pParent, bRepeate)) return FALSE;

  /// init the tooltip
  m_wndToolTip.Create("", NULL, this, pDesktopWindow);

  return TRUE;
}

void CImageButton::Delete()
{
  m_wndToolTip.Delete();

  m_Anim.Stop();

  CButtonObject::Delete();
}


void CImageButton::ChooseLayout()
{
  m_Anim.Stop();

  if(m_bPressed){
    m_Anim.Create(m_pLayout->m_pPressed, this);
  }
  else{
    if(m_bMouseActive){
      m_Anim.Create(m_pLayout->m_pActive, this);
    }
    else{
      m_Anim.Create(m_pLayout->m_pPassive, this);
    }
  }

  m_Anim.Play();
}

void CImageButton::OnAnimsRepaint(CAnimationInstance *pAnimation)
{
  UpdateRect();
}

void CImageButton::Draw(CDDrawSurface *pSurface, CRect *pRect)
{
  pSurface->Paste(0, 0, m_Anim.Frame());
}

void CImageButton::OnLayoutChange()
{
  ChooseLayout(); UpdateRect();
}

void CImageButton::OnMouseActivate()
{
  if(m_pLayout->m_pHoverSound)
    m_pLayout->m_pHoverSound->Play();
  if(!m_wndToolTip.IsEmpty())
    m_wndToolTip.Show();
  CButtonObject::OnMouseActivate();
}

void CImageButton::OnMouseDeactivate()
{
  if(m_pLayout->m_pLeaveSound)
    m_pLayout->m_pLeaveSound->Play();
  m_wndToolTip.Hide();
  CButtonObject::OnMouseDeactivate();
}

void CImageButton::OnLayoutPress()
{
  if(m_pLayout->m_pPressSound)
    m_pLayout->m_pPressSound->Play();
  m_wndToolTip.Hide();
  CButtonObject::OnLayoutPress();
}

void CImageButton::OnLayoutRelease()
{
  if(m_pLayout->m_pReleaseSound)
    m_pLayout->m_pReleaseSound->Play();
  CButtonObject::OnLayoutRelease();
}

void CImageButton::OnDisableWindow()
{
  m_wndToolTip.Hide();
}