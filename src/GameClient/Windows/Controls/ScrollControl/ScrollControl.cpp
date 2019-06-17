// ScrollControl.cpp: implementation of the CScrollControl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ScrollControl.h"

#include "..\..\Timer.h"
#include "..\Layouts.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CScrollControl, CWindow)

BEGIN_OBSERVER_MAP(CScrollControl, CWindow)
  BEGIN_MOUSE()
    ON_LBUTTONDOWN()
    ON_LBUTTONUP()
    ON_MOUSEMOVE()
    ON_MOUSELEAVE()
  END_MOUSE()

  BEGIN_ANIMS()
    ON_ANIMSREPAINT()
  END_ANIMS()

  BEGIN_TIMER()
    ON_TIMETICK()
  END_TIMER();
END_OBSERVER_MAP(CScrollControl, CWindow)

DWORD CScrollControl::RepeateBeginDelay = 250;
DWORD CScrollControl::RepeateDelay = 100;

CScrollControl::CScrollControl()
{
  m_pLayout = NULL;
}

CScrollControl::~CScrollControl()
{

}

#ifdef _DEBUG

void CScrollControl::AssertValid() const
{
  CWindow::AssertValid();

  ASSERT_VALID(m_pLayout);
}

void CScrollControl::Dump(CDumpContext &dc) const
{
  CWindow::Dump(dc);
}

#endif

BOOL CScrollControl::Create(CRect &rcBound, CScrollControlLayout *pLayout, CWindow *pParent, BOOL bHorizontal)
{
  if(pLayout == NULL){
    if(bHorizontal)
      m_pLayout = CLayouts::m_pDefaults->GetHorizontalScrollControlLayout();
    else
      m_pLayout = CLayouts::m_pDefaults->GetVerticalScrollControlLayout();
  }
  else{
    m_pLayout = pLayout;
  }
  ASSERT_VALID(pLayout);
  CRect rcWindow(rcBound);

  m_bHorizontal = bHorizontal;

  if(m_bHorizontal){
    rcWindow.bottom = rcWindow.top + m_pLayout->m_pPassiveTop->GetSize().cy;
  }
  else{
    rcWindow.right = rcWindow.left + m_pLayout->m_pPassiveTop->GetSize().cx;
  }

  if(m_pLayout->m_pPassiveTop->IsTransparent())
    m_bTransparent = TRUE;
  else
    m_bTransparent = FALSE;

  m_nRangeMin = 0;
  m_nRangeMax = 1;
  m_nPageSize = 1;
  m_nPosition = 0;

  m_nTopButtonState = 0;
  m_nMiddleButtonState = 0;
  m_nBottomButtonState = 0;
  m_nPageDownState = 0;
  m_nPageUpState = 0;

  m_bMouseDrag = FALSE;

  m_Top.Create(m_pLayout->m_pPassiveTop, this); m_Top.Play();
  m_UpMiddle.Create(m_pLayout->m_pPassiveMiddle, this); m_UpMiddle.Play();
  m_DownMiddle.Create(m_pLayout->m_pPassiveMiddle, this); m_DownMiddle.Play();
  m_Bottom.Create(m_pLayout->m_pPassiveBottom, this); m_Bottom.Play();

  m_ButtonTop.Create(m_pLayout->m_pPassiveButtonTop, this); m_ButtonTop.Play();
  m_ButtonMiddle.Create(m_pLayout->m_pPassiveButtonMiddle, this); m_ButtonMiddle.Play();
  m_ButtonBottom.Create(m_pLayout->m_pPassiveButtonBottom, this); m_ButtonBottom.Play();

  if(!CWindow::Create(&rcWindow, pParent))
    return FALSE;

  ComputePositions();

  return TRUE;
}

void CScrollControl::Delete()
{
  CWindow::Delete();

  m_pLayout = NULL;
}

void CScrollControl::ComputePositions()
{
  int nScrollSize, nRange, nButtonSize;
  int nPositionSpace, nPosition;
  double dbUnitSize, dbPositionUnitSize;
  CRect rcMiddlePart;

  if(m_bHorizontal){
    rcMiddlePart.left = m_pLayout->m_nTopButtonSize;
    rcMiddlePart.right = GetClientRect()->Width() - m_pLayout->m_nBottomButtonSize;
    rcMiddlePart.top = 0;
    rcMiddlePart.bottom = GetClientRect()->Height();
    
    nScrollSize = rcMiddlePart.Width();
    nRange = m_nRangeMax - m_nRangeMin;
    dbUnitSize = (double)nScrollSize / (double)nRange;
    
    nButtonSize = (int)(m_nPageSize * dbUnitSize);
    if(nButtonSize < m_pLayout->m_nMinButtonSize)
      nButtonSize = m_pLayout->m_nMinButtonSize;

    nPositionSpace = rcMiddlePart.Width() - nButtonSize;
    dbPositionUnitSize = (double)nPositionSpace / (double)(nRange - m_nPageSize);
    
    nPosition = (int)(m_nPosition * dbPositionUnitSize);
    
    m_rcButton = rcMiddlePart;
    m_rcButton.left += nPosition;
    m_rcButton.right = m_rcButton.left + nButtonSize;
  }
  else{
    rcMiddlePart.left = 0;
    rcMiddlePart.right = GetClientRect()->Width();
    rcMiddlePart.top = m_pLayout->m_nTopButtonSize;
    rcMiddlePart.bottom = GetClientRect()->Height() - m_pLayout->m_nBottomButtonSize;
    
    nScrollSize = rcMiddlePart.Height();
    nRange = m_nRangeMax - m_nRangeMin;
    dbUnitSize = (double)nScrollSize / (double)nRange;
    
    nButtonSize = (int)(m_nPageSize * dbUnitSize);
    if(nButtonSize < m_pLayout->m_nMinButtonSize)
      nButtonSize = m_pLayout->m_nMinButtonSize;
    
    nPositionSpace = rcMiddlePart.Height() - nButtonSize;
    dbPositionUnitSize = (double)nPositionSpace / (double)(nRange - m_nPageSize);
    
    nPosition = (int)(m_nPosition * dbPositionUnitSize);
    
    m_rcButton = rcMiddlePart;
    m_rcButton.top += nPosition;
    m_rcButton.bottom = m_rcButton.top + nButtonSize;
  }
}

BOOL CScrollControl::SetTopButtonState(int nState)
{
  if(nState == m_nTopButtonState) return FALSE;
  m_Top.Stop();
  m_nTopButtonState = nState;

  switch(nState){
  case 0:
    m_Top.Create(m_pLayout->m_pPassiveTop, this);
    break;
  case 1:
    m_Top.Create(m_pLayout->m_pActiveTop, this);
    break;
  case 2:
    m_Top.Create(m_pLayout->m_pPressedTop, this);
    break;
  }
  m_Top.Play();
  UpdateRect();

  return TRUE;
}

BOOL CScrollControl::SetMiddleButtonState(int nState)
{
  if(nState == m_nMiddleButtonState) return FALSE;
  m_ButtonTop.Stop();
  m_ButtonMiddle.Stop();
  m_ButtonBottom.Stop();
  m_nMiddleButtonState = nState;

  switch(nState){
  case 0:
    m_ButtonTop.Create(m_pLayout->m_pPassiveButtonTop, this);
    m_ButtonMiddle.Create(m_pLayout->m_pPassiveButtonMiddle, this);
    m_ButtonBottom.Create(m_pLayout->m_pPassiveButtonBottom, this);
    break;
  case 1:
    m_ButtonTop.Create(m_pLayout->m_pActiveButtonTop, this);
    m_ButtonMiddle.Create(m_pLayout->m_pActiveButtonMiddle, this);
    m_ButtonBottom.Create(m_pLayout->m_pActiveButtonBottom, this);
    break;
  case 2:
    m_ButtonTop.Create(m_pLayout->m_pPressedButtonTop, this);
    m_ButtonMiddle.Create(m_pLayout->m_pPressedButtonMiddle, this);
    m_ButtonBottom.Create(m_pLayout->m_pPressedButtonBottom, this);
    break;
  }

  m_ButtonTop.Play();
  m_ButtonMiddle.Play();
  m_ButtonBottom.Play();
  UpdateRect();

  return TRUE;
}

BOOL CScrollControl::SetBottomButtonState(int nState)
{
  if(nState == m_nBottomButtonState) return FALSE;
  m_Bottom.Stop();
  m_nBottomButtonState = nState;

  switch(nState){
  case 0:
    m_Bottom.Create(m_pLayout->m_pPassiveBottom, this);
    break;
  case 1:
    m_Bottom.Create(m_pLayout->m_pActiveBottom, this);
    break;
  case 2:
    m_Bottom.Create(m_pLayout->m_pPressedBottom, this);
    break;
  }
  m_Bottom.Play();
  UpdateRect();

  return TRUE;
}

BOOL CScrollControl::SetPageDownState(int nState)
{
  if(nState == m_nPageDownState) return FALSE;
  m_DownMiddle.Stop();
  m_nPageDownState = nState;

  switch(nState){
  case 0:
    m_DownMiddle.Create(m_pLayout->m_pPassiveMiddle, this);
    break;
  case 1:
    m_DownMiddle.Create(m_pLayout->m_pActiveMiddle, this);
    break;
  case 2:
    m_DownMiddle.Create(m_pLayout->m_pPressedMiddle, this);
    break;
  }

  m_DownMiddle.Play();
  UpdateRect();
  return TRUE;
}

BOOL CScrollControl::SetPageUpState(int nState)
{
  if(nState == m_nPageUpState) return FALSE;
  m_UpMiddle.Stop();
  m_nPageUpState = nState;

  switch(nState){
  case 0:
    m_UpMiddle.Create(m_pLayout->m_pPassiveMiddle, this);
    break;
  case 1:
    m_UpMiddle.Create(m_pLayout->m_pActiveMiddle, this);
    break;
  case 2:
    m_UpMiddle.Create(m_pLayout->m_pPressedMiddle, this);
    break;
  }

  m_UpMiddle.Play();
  UpdateRect();
  return TRUE;
}

void CScrollControl::Draw(CDDrawSurface *pSurface, CRect *pRectBound)
{
  int nLeft, nPos;
  CDDrawSurface *pTop, *pMiddle, *pBottom;
  CDDrawSurface *pUpMiddle, *pDownMiddle;
  pTop = m_Top.Frame(); pUpMiddle = m_UpMiddle.Frame();
  pDownMiddle = m_DownMiddle.Frame(); pBottom = m_Bottom.Frame();

  if(m_bHorizontal){
    // draw the top button (now left button)
    pSurface->Paste(0, 0, pTop);
    
    // draw the middle up part
    nLeft = m_rcButton.left - pTop->GetWidth();
    nPos = pTop->GetWidth();
    while(nLeft > 0){
      if(nLeft >= (int)pUpMiddle->GetWidth()){
        pSurface->Paste(nPos, 0, pUpMiddle);
        nLeft -= pUpMiddle->GetWidth();
        nPos += pUpMiddle->GetWidth();
      }
      else{
        CRect rcHlp(0, 0, nLeft, pUpMiddle->GetHeight());
        pSurface->Paste(nPos, 0, pUpMiddle, &rcHlp);
        nLeft = 0;
      }
    }
    
    // draw the middle down part
    nLeft = GetClientRect()->right - m_rcButton.right - pBottom->GetWidth();
    nPos = m_rcButton.right;
    while(nLeft > 0){
      if(nLeft >= (int)pDownMiddle->GetWidth()){
        pSurface->Paste(nPos, 0, pDownMiddle);
        nLeft -= pDownMiddle->GetWidth();
        nPos += pDownMiddle->GetWidth();
      }
      else{
        CRect rcHlp(0, 0, nLeft, pDownMiddle->GetHeight());
        pSurface->Paste(nPos, 0, pDownMiddle, &rcHlp);
        nLeft = 0;
      }
    }
    
    // draw the bottom button
    pSurface->Paste(GetClientRect()->right - pBottom->GetWidth(), 0, pBottom);
    
    // now draw the middle button
    pTop = m_ButtonTop.Frame(); pMiddle = m_ButtonMiddle.Frame(); pBottom = m_ButtonBottom.Frame();
    // draw the top part
    pSurface->Paste(m_rcButton.left, 0, pTop);
    
    // draw the middle part
    nLeft = m_rcButton.Width() - pTop->GetWidth() - pBottom->GetWidth();
    nPos = pTop->GetWidth() + m_rcButton.left;
    while(nLeft > 0){
      if(nLeft >= (int)pMiddle->GetWidth()){
        pSurface->Paste(nPos, 0, pMiddle);
        nLeft -= pMiddle->GetWidth();
        nPos += pMiddle->GetWidth();
      }
      else{
        CRect rcHlp(0, 0, nLeft, pMiddle->GetHeight());
        pSurface->Paste(nPos, 0, pMiddle, &rcHlp);
        nLeft = 0;
      }
    }
    
    // draw the bottom part
    pSurface->Paste(m_rcButton.right - pBottom->GetWidth(), 0, pBottom);
  }
  else{
    // draw the top button
    pSurface->Paste(0, 0, pTop);
    
    // draw the middle up part
    nLeft = m_rcButton.top - pTop->GetHeight();
    nPos = pTop->GetHeight();
    while(nLeft > 0){
      if(nLeft >= (int)pUpMiddle->GetHeight()){
        pSurface->Paste(0, nPos, pUpMiddle);
        nLeft -= pUpMiddle->GetHeight();
        nPos += pUpMiddle->GetHeight();
      }
      else{
        CRect rcHlp(0, 0, pUpMiddle->GetWidth(), nLeft);
        pSurface->Paste(0, nPos, pUpMiddle, &rcHlp);
        nLeft = 0;
      }
    }
    
    // draw the middle down part
    nLeft = GetClientRect()->bottom - m_rcButton.bottom - pBottom->GetHeight();
    nPos = m_rcButton.bottom;
    while(nLeft > 0){
      if(nLeft >= (int)pDownMiddle->GetHeight()){
        pSurface->Paste(0, nPos, pDownMiddle);
        nLeft -= pDownMiddle->GetHeight();
        nPos += pDownMiddle->GetHeight();
      }
      else{
        CRect rcHlp(0, 0, pDownMiddle->GetWidth(), nLeft);
        pSurface->Paste(0, nPos, pDownMiddle, &rcHlp);
        nLeft = 0;
      }
    }
    
    // draw the bottom button
    pSurface->Paste(0, GetClientRect()->bottom - pBottom->GetHeight(), pBottom);
    
    // now draw the middle button
    pTop = m_ButtonTop.Frame(); pMiddle = m_ButtonMiddle.Frame(); pBottom = m_ButtonBottom.Frame();
    // draw the top part
    pSurface->Paste(0, m_rcButton.top, pTop);
    
    // draw the middle part
    nLeft = m_rcButton.Height() - pTop->GetHeight() - pBottom->GetHeight();
    nPos = pTop->GetHeight() + m_rcButton.top;
    while(nLeft > 0){
      if(nLeft >= (int)pMiddle->GetHeight()){
        pSurface->Paste(0, nPos, pMiddle);
        nLeft -= pMiddle->GetHeight();
        nPos += pMiddle->GetHeight();
      }
      else{
        CRect rcHlp(0, 0, pMiddle->GetWidth(), nLeft);
        pSurface->Paste(0, nPos, pMiddle, &rcHlp);
        nLeft = 0;
      }
    }
    
    // draw the bottom part
    pSurface->Paste(0, m_rcButton.bottom - pBottom->GetHeight(), pBottom);
  }
}

void CScrollControl::OnAnimsRepaint(CAnimationInstance *pAnimation)
{
  UpdateRect();
}

void CScrollControl::OnLButtonDown(CPoint point)
{
  if(InTopButton(point)){
    this->SetCapture();
    m_bMouseDrag = TRUE;
    SetTopButtonState(2);
    g_pTimer->Connect(this, RepeateBeginDelay);
    OnStepUp();
  } else
  if(InBottomButton(point)){
    this->SetCapture();
    m_bMouseDrag = TRUE;
    SetBottomButtonState(2);
    g_pTimer->Connect(this, RepeateBeginDelay);
    OnStepDown();
  } else
  if(InPageUp(point)){
    this->SetCapture();
    m_bMouseDrag = TRUE;
    SetPageUpState(2);
    g_pTimer->Connect(this, RepeateBeginDelay);
    OnPageUp();
  } else
  if(InPageDown(point)){
    this->SetCapture();
    m_bMouseDrag = TRUE;
    SetPageDownState(2);
    g_pTimer->Connect(this, RepeateBeginDelay);
    OnPageDown();
  } else
  if(InMiddleButton(point)){
    this->SetCapture();
    m_bMouseDrag = TRUE;
    SetMiddleButtonState(2);
    if(m_bHorizontal){
      m_nMiddleStartPos = point.x - m_rcButton.left;
    }
    else{
      m_nMiddleStartPos = point.y - m_rcButton.top;
    }
  }
}

void CScrollControl::OnLButtonUp(CPoint point)
{
  if(m_bMouseDrag){
    ReleaseCapture();
    m_bMouseDrag = FALSE;
    g_pTimer->Disconnect(this);
    if(InTopButton(point))
      SetTopButtonState(1);
    else
      SetTopButtonState(0);

    if(InBottomButton(point))
      SetBottomButtonState(1);
    else
      SetBottomButtonState(0);

    if(InMiddleButton(point))
      SetMiddleButtonState(1);
    else
      SetMiddleButtonState(0);

    if(InPageUp(point))
      SetPageUpState(1);
    else
      SetPageUpState(0);

    if(InPageDown(point))
      SetPageDownState(1);
    else
      SetPageDownState(0);
  }
}

void CScrollControl::OnMouseMove(CPoint point)
{
  if(m_bMouseDrag){
    if(m_nTopButtonState != 0){
      if(InTopButton(point)){
        if(SetTopButtonState(2)) // press the button
          OnStepUp();
      }
      else{
        SetTopButtonState(1); // just activate it
      }
    }
    if(m_nBottomButtonState != 0){
      if(InBottomButton(point)){
        if(SetBottomButtonState(2)) // press it
          OnStepDown();
      }
      else{
        SetBottomButtonState(1); // just activate it
      }
    }
    if(m_nPageUpState != 0){
      if(InPageUp(point)){
        if(SetPageUpState(2)) // press it
          OnPageUp();
      }
      else{
        SetPageUpState(1);
      }
    }
    if(m_nPageDownState != 0){
      if(InPageDown(point)){
        if(SetPageDownState(2)) // press it
          OnPageDown();
      }
      else{
        SetPageDownState(1);
      }
    }

    if(m_nMiddleButtonState != 0){
      // the mouse has moved -> compute new position
      // and if it differs from the old one -> send notification
      int nNewPosition;
      if(m_bHorizontal){
        int nPosSpace = GetClientRect()->Width() - m_rcButton.Width() -
          m_pLayout->m_nTopButtonSize - m_pLayout->m_nBottomButtonSize;
        int nRange = m_nRangeMax - m_nRangeMin;
        double dbPosUnit = (double)nPosSpace / (double)(nRange - m_nPageSize);
        
        int nNewPos = point.x - m_nMiddleStartPos;
        if(nNewPos < (m_pLayout->m_nTopButtonSize)) nNewPos = m_pLayout->m_nTopButtonSize;
        if(nNewPos > (GetClientRect()->right - m_pLayout->m_nBottomButtonSize - m_rcButton.Width()))
          nNewPos = GetClientRect()->right - m_pLayout->m_nBottomButtonSize - m_rcButton.Width();
        
        nNewPosition = (int)((double)(nNewPos - m_pLayout->m_nTopButtonSize) / dbPosUnit);
      }
      else{
        int nPosSpace = GetClientRect()->Height() - m_rcButton.Height() -
          m_pLayout->m_nTopButtonSize - m_pLayout->m_nBottomButtonSize;
        int nRange = m_nRangeMax - m_nRangeMin;
        double dbPosUnit = (double)nPosSpace / (double)(nRange - m_nPageSize);
        
        int nNewPos = point.y - m_nMiddleStartPos;
        if(nNewPos < (m_pLayout->m_nTopButtonSize)) nNewPos = m_pLayout->m_nTopButtonSize;
        if(nNewPos > (GetClientRect()->bottom - m_pLayout->m_nBottomButtonSize - m_rcButton.Height()))
          nNewPos = GetClientRect()->bottom - m_pLayout->m_nBottomButtonSize - m_rcButton.Height();
        
        nNewPosition = (int)((double)(nNewPos - m_pLayout->m_nTopButtonSize) / dbPosUnit);
      }

      if(m_nPosition != nNewPosition){
        InlayEvent(E_POSITION, nNewPosition);
      }
    }
  }
  else{
    if(InTopButton(point)){
      SetTopButtonState(1);
      SetMiddleButtonState(0);
      SetBottomButtonState(0);
      SetPageUpState(0);
      SetPageDownState(0);
    } else
    if(InMiddleButton(point)){
      SetTopButtonState(0);
      SetMiddleButtonState(1);
      SetBottomButtonState(0);
      SetPageUpState(0);
      SetPageDownState(0);
    } else
    if(InBottomButton(point)){
      SetTopButtonState(0);
      SetMiddleButtonState(0);
      SetBottomButtonState(1);
      SetPageUpState(0);
      SetPageDownState(0);
    } else
    if(InPageUp(point)){
      SetTopButtonState(0);
      SetMiddleButtonState(0);
      SetBottomButtonState(0);
      SetPageUpState(1);
      SetPageDownState(0);
    } else
      if(InPageDown(point)){
      SetTopButtonState(0);
      SetMiddleButtonState(0);
      SetBottomButtonState(0);
      SetPageUpState(0);
      SetPageDownState(1);
    }
  }
}

void CScrollControl::OnMouseLeave()
{
  if(m_bMouseDrag){
    if(m_nTopButtonState != 0)
      SetTopButtonState(1);
    if(m_nBottomButtonState != 0)
      SetBottomButtonState(1);
    if(m_nPageUpState != 0)
      SetPageUpState(1);
    if(m_nPageDownState != 0)
      SetPageDownState(1);
  }
  else{
    SetTopButtonState(0);
    SetBottomButtonState(0);
    SetMiddleButtonState(0);
    SetPageUpState(0);
    SetPageDownState(0);
  }
}

BOOL CScrollControl::InTopButton(CPoint &point)
{
  if(!GetClientRect()->PtInRect(point)) return FALSE;
  if(m_bHorizontal){
    if(point.x > m_pLayout->m_nTopButtonSize) return FALSE;
  }
  else{
    if(point.y > m_pLayout->m_nTopButtonSize) return FALSE;
  }
  return TRUE;
}

BOOL CScrollControl::InBottomButton(CPoint &point)
{
  if(!GetClientRect()->PtInRect(point)) return FALSE;
  if(m_bHorizontal){
    if(point.x < (GetClientRect()->right - m_pLayout->m_nBottomButtonSize)) return FALSE;
  }
  else{
    if(point.y < (GetClientRect()->bottom - m_pLayout->m_nBottomButtonSize)) return FALSE;
  }
  return TRUE;
}

BOOL CScrollControl::InMiddleButton(CPoint &point)
{
  return m_rcButton.PtInRect(point);
}

BOOL CScrollControl::InPageUp(CPoint &point)
{
  if(!GetClientRect()->PtInRect(point)) return FALSE;
  if(m_bHorizontal){
    if(point.x < m_pLayout->m_nTopButtonSize) return FALSE;
    if(point.x >= m_rcButton.left) return FALSE;
  }
  else{
    if(point.y < m_pLayout->m_nTopButtonSize) return FALSE;
    if(point.y >= m_rcButton.top) return FALSE;
  }
  return TRUE;
}

BOOL CScrollControl::InPageDown(CPoint &point)
{
  if(!GetClientRect()->PtInRect(point)) return FALSE;
  if(m_bHorizontal){
    if(point.x < m_rcButton.right) return FALSE;
    if(point.x >= (GetClientRect()->right - m_pLayout->m_nBottomButtonSize)) return FALSE;
  }
  else{
    if(point.y < m_rcButton.bottom) return FALSE;
    if(point.y >= (GetClientRect()->bottom - m_pLayout->m_nBottomButtonSize)) return FALSE;
  }
  return TRUE;
}

void CScrollControl::OnDeactivate()
{
/*  if(m_bMouseDrag){
    ReleaseCapture();
    SetTopButtonState(0);
    SetMiddleButtonState(0);
    SetBottomButtonState(0);
    SetPageUpState(0);
    SetPageDownState(0);
    g_pTimer->Disconnect(this);
    m_bMouseDrag = FALSE;
  }*/
}

void CScrollControl::OnStepUp()
{
  InlayEvent(E_STEPUP, 0);
}

void CScrollControl::OnStepDown()
{
  InlayEvent(E_STEPDOWN, 0);
}

void CScrollControl::OnPageUp()
{
  InlayEvent(E_PAGEUP, 0);
}

void CScrollControl::OnPageDown()
{
  InlayEvent(E_PAGEDOWN, 0);
}

void CScrollControl::OnTimeTick(DWORD dwTime)
{
  if(m_bMouseDrag){
    if(m_nTopButtonState == 2){
      OnStepUp();
    }
    if(m_nBottomButtonState == 2){
      OnStepDown();
    }
    if(m_nPageUpState == 2){
      OnPageUp();
    }
    if(m_nPageDownState == 2){
      OnPageDown();
    }
    g_pTimer->SetElapseTime(this, RepeateDelay);
  }
}

void CScrollControl::SetRange(int nRangeMin, int nRangeMax)
{
  ASSERT_VALID(this);

  m_nRangeMin = nRangeMin;
  m_nRangeMax = nRangeMax;
  // it has to be at least 1 unit size range
  if(m_nRangeMax <= m_nRangeMin) m_nRangeMax = m_nRangeMin + 1;
  m_nPosition = m_nRangeMin;
  m_nPageSize = m_nRangeMax - m_nRangeMin;

  ComputePositions();
  UpdateRect();
}

void CScrollControl::SetPageSize(int nPageSize)
{
  ASSERT_VALID(this);

  m_nPageSize = nPageSize;
  // make the page size fit in the range
  if(m_nPageSize > (m_nRangeMax - m_nRangeMin))
    m_nPageSize = m_nRangeMax - m_nRangeMin;
  // correct the position if too big for the page size
  if(m_nPosition + m_nPageSize > m_nRangeMax){
    m_nPosition = m_nRangeMax - m_nPageSize;
  }

  ComputePositions();
  UpdateRect();
}

void CScrollControl::SetPosition(int nPosition)
{
  ASSERT_VALID(this);

  m_nPosition = nPosition;
  // make the position fit in the range and page size
  if(m_nPosition < m_nRangeMin){
    m_nPosition = m_nRangeMin;
  }
  if(m_nPosition + m_nPageSize > m_nRangeMax){
    m_nPosition = m_nRangeMax - m_nPageSize;
  }

  ComputePositions();
  UpdateRect();
}

int CScrollControl::GetPosition()
{
  return m_nPosition;
}

int CScrollControl::GetPageSize()
{
  return m_nPageSize;
}

int CScrollControl::GetRangeMin()
{
  return m_nRangeMin;
}

int CScrollControl::GetRangeMax()
{
  return m_nRangeMax;
}

void CScrollControl::OnSize(CSize size)
{
  ComputePositions();

  CWindow::OnSize(size);
}
