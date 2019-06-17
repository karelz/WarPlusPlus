// FrameWindow.cpp: implementation of the CFrameWindow class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FrameWindow.h"
#include "..\Timer\Timer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CFrameWindow, CWindow)

BEGIN_OBSERVER_MAP(CFrameWindow, CWindow)
  BEGIN_MOUSE()
    ON_MOUSEMOVE();
    ON_LBUTTONDOWN()
    ON_LBUTTONUP()
  END_MOUSE()

  BEGIN_ANIMS()
    ON_ANIMSREPAINT()
  END_ANIMS()
END_OBSERVER_MAP(CFrameWindow, CWindow)

CFrameWindow::CFrameWindow()
{
  m_bMoving = FALSE;
  m_bResizeable = FALSE;
  m_nSizing = None;
  m_bActiveFrame = FALSE;
  m_bStopBufferingOnSize = FALSE;
}

CFrameWindow::~CFrameWindow()
{

}

#ifdef _DEBUG

void CFrameWindow::AssertValid() const
{
  CWindow::AssertValid();
}

void CFrameWindow::Dump(CDumpContext &dc) const
{
  CWindow::Dump(dc);
}

#endif

void CFrameWindow::ComputePositions()
{
  CSize size;

  size = m_TopLeft.GetSize();
  m_ptMiddle.x = size.cx;
  m_ptMiddle.y = size.cy;

  size = m_BottomRight.GetSize();
  m_ptBottomRight.x = m_rectWhole.right - size.cx;
  m_ptBottomRight.y = m_rectWhole.bottom - size.cy;

  m_sizeMiddle.cx = m_ptBottomRight.x - m_ptMiddle.x;
  m_sizeMiddle.cy = m_ptBottomRight.y - m_ptMiddle.y;

  m_TopLeft.SetRect(CPoint(0, 0));
  m_Top.SetRect(CRect(m_ptMiddle.x, 0, m_ptMiddle.x + m_sizeMiddle.cx, m_ptMiddle.y));
  m_TopRight.SetRect(CPoint(m_ptBottomRight.x, 0));
  m_Left.SetRect(CRect(0, m_ptMiddle.y, m_ptMiddle.x, m_ptMiddle.y + m_sizeMiddle.cy));
  m_Middle.SetRect(CRect(m_ptMiddle, m_sizeMiddle));
  m_Right.SetRect(CRect(m_ptBottomRight.x, m_ptMiddle.y, m_rectWhole.right, m_ptMiddle.y + m_sizeMiddle.cy));
  m_BottomLeft.SetRect(CPoint(0, m_ptBottomRight.y));
  m_Bottom.SetRect(CRect(m_ptMiddle.x, m_ptBottomRight.y, m_ptMiddle.x + m_sizeMiddle.cx, m_rectWhole.bottom));
  m_BottomRight.SetRect(m_ptBottomRight);
}

void CFrameWindow::ChooseLayout()
{
  m_TopLeft.Stop(); m_Top.Stop(); m_TopRight.Stop();
  m_Left.Stop(); m_Middle.Stop(); m_Right.Stop();
  m_BottomLeft.Stop(); m_Bottom.Stop(); m_BottomRight.Stop();

  if(m_bActiveFrame){
    m_TopLeft.Create(m_pLayout->GetATopLeft(), this);
    m_Top.Create(m_pLayout->GetATop(), this);
    m_TopRight.Create(m_pLayout->GetATopRight(), this);
    m_Left.Create(m_pLayout->GetALeft(), this);
    m_Middle.Create(m_pLayout->GetAMiddle(), this);
    m_Right.Create(m_pLayout->GetARight(), this);
    m_BottomLeft.Create(m_pLayout->GetABottomLeft(), this);
    m_Bottom.Create(m_pLayout->GetABottom(), this);
    m_BottomRight.Create(m_pLayout->GetABottomRight(), this);
  }
  else{
    m_TopLeft.Create(m_pLayout->GetPTopLeft(), this);
    m_Top.Create(m_pLayout->GetPTop(), this);
    m_TopRight.Create(m_pLayout->GetPTopRight(), this);
    m_Left.Create(m_pLayout->GetPLeft(), this);
    m_Middle.Create(m_pLayout->GetPMiddle(), this);
    m_Right.Create(m_pLayout->GetPRight(), this);
    m_BottomLeft.Create(m_pLayout->GetPBottomLeft(), this);
    m_Bottom.Create(m_pLayout->GetPBottom(), this);
    m_BottomRight.Create(m_pLayout->GetPBottomRight(), this);
  }

  m_TopLeft.Play(); m_Top.Play(); m_TopRight.Play();
  m_Left.Play(); m_Middle.Play(); m_Right.Play();
  m_BottomLeft.Play(); m_Bottom.Play(); m_BottomRight.Play();
}

void CFrameWindow::Draw(CDDrawSurface *pSurface, CRect *pBoundRect)
{
  CSize size;
  CRect rectl, rectr, rectt, rectb, rectm;
  int xpos, ypos, xlen, ylen;

  // paste the corners
  pSurface->Paste(0, 0, m_TopLeft.Frame());
  pSurface->Paste(0, m_ptBottomRight.y, m_BottomLeft.Frame());
  pSurface->Paste(m_ptBottomRight.x, 0, m_TopRight.Frame());
  pSurface->Paste(m_ptBottomRight, m_BottomRight.Frame());

  // draw the top and bottom resizables
  size = m_Top.GetSize();
  xlen = m_sizeMiddle.cx; xpos = m_ptMiddle.x;
  rectt.SetRect(0, 0, size.cx, m_ptMiddle.y);
  rectb.SetRect(0, 0, size.cx, m_rectWhole.bottom - m_ptBottomRight.y);
  while(xlen > 0){
    if(xlen < size.cx){
      rectt.right = xlen;
      rectb.right = xlen;
    }
    pSurface->Paste(xpos, 0, m_Top.Frame(), &rectt);
    pSurface->Paste(xpos, m_ptBottomRight.y, m_Bottom.Frame(), &rectb);
    xpos += size.cx;
    xlen -= rectt.Width();
  }
  
  // draw the left, middle, right resizables
  size = m_Left.GetSize();
  ylen = m_sizeMiddle.cy; ypos = m_ptMiddle.y;
  rectl.SetRect(0, 0, m_ptMiddle.x, size.cy);
  rectr.SetRect(0, 0, m_rectWhole.right - m_ptBottomRight.x, size.cy);
  if(m_Middle.Frame() != NULL)
    rectm.SetRect(0, 0, m_Middle.Frame()->GetWidth(), m_Middle.Frame()->GetHeight());
  while(ylen > 0){
    if(ylen < size.cy){
      rectl.bottom = ylen;
      rectr.bottom = ylen;
      rectm.bottom = ylen;
    }

    // draw the left and right
    pSurface->Paste(0, ypos, m_Left.Frame(), &rectl);
    pSurface->Paste(m_ptBottomRight.x, ypos, m_Right.Frame(), &rectr);

    if(m_Middle.Frame() != NULL){
      // draw the middle
      xlen = m_sizeMiddle.cx; xpos = m_ptMiddle.x;
      rectm.right = m_Middle.Frame()->GetWidth();
      while(xlen > 0){
        if(xlen < (int)m_Middle.Frame()->GetWidth()){
          rectm.right = xlen;
        }
        pSurface->Paste(xpos, ypos, m_Middle.Frame(), &rectm);
        xlen -= rectm.Width();
        xpos += rectm.Width();
      }
    }

    ylen -= rectl.Height();
    ypos += size.cy;
  }

  if(m_Middle.Frame() == NULL){
    DWORD dwColor;
    if(m_bActiveFrame)
      dwColor = m_pLayout->m_dwActiveMiddleColor;
    else
      dwColor = m_pLayout->m_dwPassiveMiddleColor;
    CRect rc(m_ptMiddle, m_sizeMiddle);
    pSurface->Fill(dwColor, &rc);
  }
}

void CFrameWindow::OnAnimsRepaint(CAnimationInstance *pAnimation)
{
  UpdateRect(pAnimation->GetRect());
}

BOOL CFrameWindow::Create(CRect &rcBound, CFrameWindowLayout *pLayout, CWindow *pParent, BOOL bResizeable)
{
  m_pLayout = pLayout;
  m_bResizeable = bResizeable;
  m_rectWhole.SetRect(0, 0, rcBound.Width(), rcBound.Height());

  m_bTransparent = m_pLayout->AreAnimsTransparent();

  ChooseLayout();
  ComputePositions();

  if(!CWindow::Create(&rcBound, pParent)) return FALSE;

  return TRUE;
}

void CFrameWindow::Delete()
{
  CWindow::Delete();
}

void CFrameWindow::OnMouseMove(CPoint point)
{
  CPoint ptNew;
  CRect rcNew;
  CSize szMin;
  CSize szMax;

  if(m_bMoving){
    ptNew.x = point.x - m_ptStartPos.x + GetWindowPosition().left;
    ptNew.y = point.y - m_ptStartPos.y + GetWindowPosition().top;
    
    SetWindowPosition(&ptNew);

    return;
  }

  if(m_nSizing != None){
    rcNew = *GetWindowPosition();
    szMin = GetMinimumSize();
    szMax = GetMaximumSize();
    switch(m_nSizing){
    case LeftEdge:
      rcNew.left += point.x - m_ptStartPos.x;
      if(rcNew.Width() < szMin.cx) rcNew.left = rcNew.right - szMin.cx;
      if(rcNew.Width() > szMax.cx) rcNew.left = rcNew.right - szMax.cx;
      break;
    case RightEdge:
      rcNew.right = point.x - m_ptStartPos.x + m_rectOldPos.right;
      if(rcNew.Width() < szMin.cx) rcNew.right = rcNew.left + szMin.cx;
      if(rcNew.Width() > szMax.cx) rcNew.right = rcNew.left + szMax.cx;
      break;
    case TopEdge:
      rcNew.top += point.y - m_ptStartPos.y;
      if(rcNew.Height() < szMin.cy) rcNew.top = rcNew.bottom - szMin.cy;
      if(rcNew.Height() > szMax.cy) rcNew.top = rcNew.bottom - szMax.cy;
      break;
    case BottomEdge:
      rcNew.bottom = point.y - m_ptStartPos.y + m_rectOldPos.bottom;
      if(rcNew.Height() < szMin.cy) rcNew.bottom = rcNew.top + szMin.cy;
      if(rcNew.Height() > szMax.cy) rcNew.bottom = rcNew.top + szMax.cy;
      break;
    case TopLeftCorner:
      rcNew.left += point.x - m_ptStartPos.x;
      if(rcNew.Width() < szMin.cx) rcNew.left = rcNew.right - szMin.cx;
      if(rcNew.Width() > szMax.cx) rcNew.left = rcNew.right - szMax.cx;
      rcNew.top += point.y - m_ptStartPos.y;
      if(rcNew.Height() < szMin.cy) rcNew.top = rcNew.bottom - szMin.cy;
      if(rcNew.Height() > szMax.cy) rcNew.top = rcNew.bottom - szMax.cy;
      break;
    case BottomLeftCorner:
      rcNew.left += point.x - m_ptStartPos.x;
      if(rcNew.Width() < szMin.cx) rcNew.left = rcNew.right - szMin.cx;
      if(rcNew.Width() > szMax.cx) rcNew.left = rcNew.right - szMax.cx;
      rcNew.bottom = point.y - m_ptStartPos.y + m_rectOldPos.bottom;
      if(rcNew.Height() < szMin.cy) rcNew.bottom = rcNew.top + szMin.cy;
      if(rcNew.Height() > szMax.cy) rcNew.bottom = rcNew.top + szMax.cy;
      break;
    case TopRightCorner:
      rcNew.right = point.x - m_ptStartPos.x + m_rectOldPos.right;
      if(rcNew.Width() < szMin.cx) rcNew.right = rcNew.left + szMin.cx;
      if(rcNew.Width() > szMax.cx) rcNew.right = rcNew.left + szMax.cx;
      rcNew.top += point.y - m_ptStartPos.y;
      if(rcNew.Height() < szMin.cy) rcNew.top = rcNew.bottom - szMin.cy;
      if(rcNew.Height() > szMax.cy) rcNew.top = rcNew.bottom - szMax.cy;
      break;
    case BottomRightCorner:
      rcNew.right = point.x - m_ptStartPos.x + m_rectOldPos.right;
      if(rcNew.Width() < szMin.cx) rcNew.right = rcNew.left + szMin.cx;
      if(rcNew.Width() > szMax.cx) rcNew.right = rcNew.left + szMax.cx;
      rcNew.bottom = point.y - m_ptStartPos.y + m_rectOldPos.bottom;
      if(rcNew.Height() < szMin.cy) rcNew.bottom = rcNew.top + szMin.cy;
      if(rcNew.Height() > szMax.cy) rcNew.bottom = rcNew.top + szMax.cy;
      break;
    }
    SetWindowPosition(&rcNew);

    return;
  }

  switch(InSizeArea(point)){
  case LeftEdge:
  case RightEdge:
    SetWindowCursor(m_pLayout->m_pLRCursor);
    return;
  case TopEdge:
  case BottomEdge:
    SetWindowCursor(m_pLayout->m_pTBCursor);
    return;
  case TopLeftCorner:
  case BottomRightCorner:
    SetWindowCursor(m_pLayout->m_pTLBRCursor);
    return;
  case TopRightCorner:
  case BottomLeftCorner:
    SetWindowCursor(m_pLayout->m_pTRBLCursor);
    return;

  case None:
  default:
    break;
  }

  if(InMoveArea(point)){
    SetWindowCursor(m_pLayout->m_pMoveCursor);
    return;
  }

  SetWindowCursor(m_pLayout->m_pNormalCursor);
}

void CFrameWindow::OnLButtonDown(CPoint point)
{
  if(m_nSizing != None) return;
  if(m_bMoving) return;

  int nSizing;
  if((nSizing = InSizeArea(point)) != None){
    m_ptStartPos = point;
    m_rectOldPos = *GetWindowPosition();
    m_nSizing = nSizing;
    SetCapture();
    GetParentWindow()->SetMouseClip();

    if(m_bStopBufferingOnSize && IsBuffered()){
      StopBuffering();
    }
    return;
  }

  if(InMoveArea(point)){
    m_ptStartPos = point;
    m_rectOldPos = *GetWindowPosition();
    SetCapture();
    GetParentWindow()->SetMouseClip();
    m_bMoving = TRUE;
    return;
  }
}

void CFrameWindow::OnLButtonUp(CPoint point)
{
  ReleaseCapture();
  if(m_bMoving){
    GetParentWindow()->ReleaseMouseClip();
  }
  if(m_nSizing != None){
    if(m_bStopBufferingOnSize && IsBuffered()){
      SetBuffering();
    }
    GetParentWindow()->ReleaseMouseClip();
  }
  m_bMoving = FALSE;
  m_nSizing = None;
}

BOOL CFrameWindow::InMoveArea(CPoint point)
{
  if(m_pLayout->m_nDragLeft == -1) return FALSE;
  if(m_pLayout->m_nDragTop == -1) return FALSE;

  if(point.x < m_pLayout->m_nDragLeft) return FALSE;
  if(m_pLayout->m_nDragRight < 0){
    if(point.x > m_pLayout->m_nDragLeft - m_pLayout->m_nDragRight) return FALSE;
  }
  else{
    if(point.x > m_rectWhole.right - m_pLayout->m_nDragRight) return FALSE;
  }
  if(point.y < m_pLayout->m_nDragTop) return FALSE;
  if(m_pLayout->m_nDragBottom < 0){
    if(point.y > m_pLayout->m_nDragTop - m_pLayout->m_nDragBottom) return FALSE;
  }
  else{
    if(point.y > m_rectWhole.bottom - m_pLayout->m_nDragBottom) return FALSE;
  }

  return TRUE;
}

BOOL CFrameWindow::OnActivate()
{
  if(m_pLayout->m_pActivateSound)
    m_pLayout->m_pActivateSound->Play();

  m_bActiveFrame = TRUE;
  ChooseLayout();

  return CWindow::OnActivate();
}

void CFrameWindow::OnDeactivate()
{
  if(m_bMoving){
    ReleaseCapture();
    GetParentWindow()->ReleaseMouseClip();
    m_bMoving = FALSE;
  }

  if(m_pLayout->m_pDeactivateSound)
    m_pLayout->m_pDeactivateSound->Play();

  m_bActiveFrame = FALSE;
  ChooseLayout();
  UpdateRect();

  CWindow::OnDeactivate();
}

void CFrameWindow::OnSize(CSize size)
{
  m_rectWhole.SetRect(0, 0, GetWindowPosition().Width(), GetWindowPosition().Height());
  ComputePositions();
  if((!m_nSizing) && m_bStopBufferingOnSize && IsBuffered()){
    FreezeWindows();
    StopBufferingInternal();
    SetBufferingInternal();
    UnfreezeWindows();
  }
}

int CFrameWindow::InSizeArea(CPoint point)
{
  if(!m_bResizeable) return None;

  if(point.x < m_pLayout->m_nEdgeLeft){
    if(point.y < m_pLayout->m_nCornerSize)
      return TopLeftCorner;
    if(point.y > (m_rectWhole.bottom - m_pLayout->m_nCornerSize))
      return BottomLeftCorner;
    return LeftEdge;
  }
  if(point.y < m_pLayout->m_nEdgeTop){
    if(point.x < m_pLayout->m_nCornerSize)
      return TopLeftCorner;
    if(point.x > (m_rectWhole.right - m_pLayout->m_nCornerSize))
      return TopRightCorner;
    return TopEdge;
  }
  if(point.x > (m_rectWhole.right - m_pLayout->m_nEdgeRight)){
    if(point.y < m_pLayout->m_nCornerSize)
      return TopRightCorner;
    if(point.y > (m_rectWhole.bottom - m_pLayout->m_nCornerSize))
      return BottomRightCorner;
    return RightEdge;
  }
  if(point.y > (m_rectWhole.bottom - m_pLayout->m_nEdgeBottom)){
    if(point.x < m_pLayout->m_nCornerSize)
      return BottomLeftCorner;
    if(point.x > (m_rectWhole.right - m_pLayout->m_nCornerSize))
      return BottomRightCorner;
    return BottomEdge;
  }

  return None;
}

CSize CFrameWindow::GetMinimumSize()
{
  return m_pLayout->m_sizeMin;
}

CSize CFrameWindow::GetMaximumSize()
{
  return m_pLayout->m_sizeMax;
}

void CFrameWindow::OnDisableWindow()
{
  if(m_bMoving){
    ReleaseCapture();
    m_bMoving = FALSE;
  }
  
  m_bActiveFrame = FALSE;
  ChooseLayout();
  UpdateRect();

  CWindow::OnDisableWindow();
}

void CFrameWindow::OnEnableWindow()
{
  if(m_bActive){
    m_bActiveFrame = TRUE;
    ChooseLayout();
    UpdateRect();
  }

  CWindow::OnEnableWindow();
}
