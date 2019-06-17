// AnimationCtrl.cpp: implementation of the CAnimationCtrl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AnimationCtrl.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CAnimationCtrl, CWindow);

BEGIN_OBSERVER_MAP(CAnimationCtrl, CWindow)
  BEGIN_ANIMS()
    ON_ANIMSREPAINT()
  END_ANIMS()
END_OBSERVER_MAP(CAnimationCtrl, CWindow)

CAnimationCtrl::CAnimationCtrl()
{
}

CAnimationCtrl::~CAnimationCtrl()
{
}

#ifdef _DEBUG

void CAnimationCtrl::AssertValid() const
{
  CWindow::AssertValid();
}

void CAnimationCtrl::Dump(CDumpContext &dc) const
{
  CWindow::Dump(dc);
  dc << m_Animation << "\n";
}

#endif

BOOL CAnimationCtrl::Create(CPoint point, CAnimation *pAnimation, CWindow *pParent, CSize *pSize)
{
  CRect rc;
  CSize size;
  
  m_Animation.Create(pAnimation, this);

  rc.left = point.x; rc.top = point.y;
  rc.right = rc.left; rc.bottom = rc.top;

  m_bTransparent = TRUE;
  m_bTabStop = FALSE;

  if(m_Animation.GetAnimation()){
    size = m_Animation.GetSize();
    rc.right = rc.left + size.cx;
    rc.bottom = rc.top + size.cy;
    m_bTransparent = m_Animation.IsTransparent();

    m_Animation.SetRect(CPoint(0, 0));
  }
  
  if(!CWindow::Create(&rc, pParent)) return FALSE;

  m_Animation.Play();

  return TRUE;
}

void CAnimationCtrl::Delete()
{
  m_Animation.Stop();

  CWindow::Delete();
}

void CAnimationCtrl::Draw(CDDrawSurface *pSurface, CRect *pRectBound)
{
  CDDrawSurface *pFrame;
  if(!m_Animation.GetAnimation()) return;

  pFrame = m_Animation.Frame();
  // if the frame doesn't exists don't draw it
  if(pFrame == NULL){
    if(m_bTransparent) return;

    // if the windows is not transparent
    // some draw bugs would appear -> so we will clear
    // the window with the black color
    pSurface->Fill(RGB32(0, 0, 0), pRectBound);
    return;
  }

  // draw the frame
  pSurface->Paste(0, 0, pFrame);
}

void CAnimationCtrl::Play()
{
  m_Animation.Play();
}

void CAnimationCtrl::Stop()
{
  m_Animation.Stop();
}

CAnimation * CAnimationCtrl::GetAnimation()
{
  return m_Animation.GetAnimation();
}

DWORD CAnimationCtrl::GetFrame()
{
  return m_Animation.GetPosition();
}

void CAnimationCtrl::SetFrame(DWORD dwFrame)
{
  m_Animation.SetPosition(dwFrame);

  UpdateRect();
}

void CAnimationCtrl::OnAnimsRepaint(CAnimationInstance *pAnimation)
{
  UpdateRect(pAnimation->GetRect());
}
