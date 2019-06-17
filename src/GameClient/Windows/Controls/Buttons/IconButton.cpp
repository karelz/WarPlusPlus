// IconButton.cpp: implementation of the CIconButton class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IconButton.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CIconButton, CButtonObject);

BEGIN_OBSERVER_MAP(CIconButton, CButtonObject)
  BEGIN_ANIMS()
    ON_ANIMSREPAINT()
  END_ANIMS()
END_OBSERVER_MAP(CIconButton, CButtonObject)

CIconButton::CIconButton()
{
}

CIconButton::~CIconButton()
{
}

#ifdef _DEBUG

void CIconButton::AssertValid() const
{
  CButtonObject::AssertValid();
}

void CIconButton::Dump(CDumpContext &dc) const
{
  CButtonObject::Dump(dc);
}

#endif

BOOL CIconButton::Create(CPoint ptPosition, CAnimation *pIcon, CIconButtonLayout *pLayout, CWindow *pParent, BOOL bRepeate, CWindow *pDesktopWindow)
{
  ASSERT_VALID(pLayout);
  ASSERT(pIcon != NULL);

  m_pLayout = pLayout;
  ASSERT_VALID(m_pLayout->m_pPassive);

  // set the transparency
  if(m_pLayout->AreAnimsTransparent()) m_bTransparent = TRUE;
  else m_bTransparent = FALSE;

  // start the icon animation
  m_IconAnim.Create(pIcon, this);
  m_IconAnim.Play();

  m_bTabStop = FALSE;
  m_bDoubleClk = FALSE;

  // set the cursor
  SetWindowCursor(m_pLayout->m_pCursor);

  CSize size;
  size = m_pLayout->m_pPassive->GetSize();
  CRect rcBound(ptPosition, size);

  // create buffers
  m_OverlayBuffer.SetWidth(size.cx);
  m_OverlayBuffer.SetHeight(size.cy);
  m_OverlayBuffer.SetAlphaChannel(TRUE);
  m_OverlayBuffer.Create();
  m_OverlayBuffer.Fill(RGB32(0, 0, 255));
  memset(m_OverlayBuffer.GetAlphaChannel(), 80, size.cx * size.cy);
  m_OverlayBuffer.ReleaseAlphaChannel(TRUE);

  m_DrawBuffer.SetWidth(size.cx);
  m_DrawBuffer.SetHeight(size.cy);
  m_DrawBuffer.SetAlphaChannel(TRUE);
  m_DrawBuffer.Create();

  // get the proper layout anim
  ChooseLayout();

  if(!CButtonObject::Create(&rcBound, pParent, bRepeate)) return FALSE;

  /// init the tooltip
  m_wndToolTip.Create("", NULL, this, pDesktopWindow);

  return TRUE;
}

void CIconButton::Delete()
{
  m_wndToolTip.Delete();
  CButtonObject::Delete();

  m_IconAnim.Stop();
  m_Anim.Stop();

  m_pLayout = NULL;

  m_OverlayBuffer.Delete();
  m_DrawBuffer.Delete();
}

void CIconButton::SetIcon(CAnimation *pIcon)
{
  m_IconAnim.Stop();
  m_IconAnim.Create(pIcon, this);
  m_IconAnim.Play();
  ChooseLayout();
  UpdateRect();
}

void CIconButton::Draw(CDDrawSurface *pSurface, CRect *pRect)
{
  CRect rcIcon(CPoint(0, 0), m_IconAnim.GetSize());
  pSurface->Paste(0, 0, m_Anim.Frame());
  pSurface->Paste(m_ptIconOffset, &m_DrawBuffer, &rcIcon);
}

void CIconButton::ChooseLayout()
{
  m_Anim.Stop();

  CSize size2 = m_DrawBuffer.GetAllRect()->Size();
  m_DrawBuffer.ClearAlphaChannel();
  m_DrawBuffer.Fill(0);
  m_DrawBuffer.Paste(0, 0, m_IconAnim.Frame());

  if(m_bPressed){
    m_Anim.Create(m_pLayout->m_pPressed, this);

    m_DrawBuffer.Paste(0, 0, &m_OverlayBuffer);

    if(m_pLayout->m_bCenter){
      m_ptIconOffset.x = (m_Anim.GetSize().cx - m_IconAnim.Frame()->GetAllRect()->Width()) / 2;
      m_ptIconOffset.y = (m_Anim.GetSize().cy - m_IconAnim.Frame()->GetAllRect()->Height()) / 2;
    }
    else{
      m_ptIconOffset = m_pLayout->m_ptPressedOffset;
    }
  }
  else{
    if(m_bMouseActive){
      m_Anim.Create(m_pLayout->m_pActive, this);

      if(m_pLayout->m_bCenter){
        m_ptIconOffset.x = (m_Anim.GetSize().cx - m_IconAnim.Frame()->GetAllRect()->Width()) / 2 - 1;
        m_ptIconOffset.y = (m_Anim.GetSize().cy - m_IconAnim.Frame()->GetAllRect()->Height()) / 2 - 1;
      }
      else{
        m_ptIconOffset = m_pLayout->m_ptActiveOffset;
      }
    }
    else{
      m_Anim.Create(m_pLayout->m_pPassive, this);

      if(m_pLayout->m_bCenter){
        m_ptIconOffset.x = (m_Anim.GetSize().cx - m_IconAnim.Frame()->GetAllRect()->Width()) / 2;
        m_ptIconOffset.y = (m_Anim.GetSize().cy - m_IconAnim.Frame()->GetAllRect()->Height()) / 2;
      }
      else{
        m_ptIconOffset = m_pLayout->m_ptPassiveOffset;
      }
    }
  }

  if(m_IconAnim.Frame()->HasAlphaChannel()){
    BYTE *pAlpha = m_DrawBuffer.GetAlphaChannel();
    memset(pAlpha, 0, size2.cx * size2.cy);
    int i;
    CSize size = m_IconAnim.Frame()->GetAllRect()->Size();
    BYTE *pSource = m_IconAnim.Frame()->GetAlphaChannel(TRUE);
    int width = size.cx;
    if(width > size2.cx) width = size2.cx;
    
    for(i = 0; i < size.cy; i++){
      memcpy(&pAlpha[i * size2.cx], &pSource[i * size.cx], width);
    }
    m_IconAnim.Frame()->ReleaseAlphaChannel();
    m_DrawBuffer.ReleaseAlphaChannel();
  }
  else{
    memset(m_DrawBuffer.GetAlphaChannel(), 255, size2.cx * size2.cy);
    m_DrawBuffer.ReleaseAlphaChannel();
  }
  
  m_Anim.Play();
}

void CIconButton::OnAnimsRepaint(CAnimationInstance *pAnimation)
{
  UpdateRect();
}

void CIconButton::OnLayoutChange()
{
  ChooseLayout(); UpdateRect();
}

void CIconButton::OnMouseActivate()
{
  if(m_pLayout->m_pHoverSound)
    m_pLayout->m_pHoverSound->Play();

  if(!m_wndToolTip.IsEmpty())
    m_wndToolTip.Show();

  CButtonObject::OnMouseActivate();
}

void CIconButton::OnMouseDeactivate()
{
  if(m_pLayout->m_pLeaveSound)
    m_pLayout->m_pLeaveSound->Play();

  m_wndToolTip.Hide();

  CButtonObject::OnMouseDeactivate();
}

void CIconButton::OnLayoutPress()
{
  if(m_pLayout->m_pPressSound)
    m_pLayout->m_pPressSound->Play();

  m_wndToolTip.Hide();

  CButtonObject::OnLayoutPress();
}

void CIconButton::OnLayoutRelease()
{
  if(m_pLayout->m_pReleaseSound)
    m_pLayout->m_pReleaseSound->Play();
  CButtonObject::OnLayoutRelease();
}

void CIconButton::OnDisableWindow()
{
  m_wndToolTip.Hide();
}