// MapViewStatusBar.cpp: implementation of the CMapViewStatusBar class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MapViewStatusBar.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define STATUSBAR_LEFT 4
#define STATUSBAR_RIGHT 2
#define STATUSBAR_BOTTOM 0

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMapViewStatusBar::CMapViewStatusBar()
{

}

CMapViewStatusBar::~CMapViewStatusBar()
{

}

#ifdef _DEBUG

void CMapViewStatusBar::AssertValid() const
{
  CWindow::AssertValid();
}

void CMapViewStatusBar::Dump(CDumpContext &dc) const
{
  CWindow::Dump(dc);
}

#endif


void CMapViewStatusBar::Create(CDataArchive Archive, CWindow *pParent)
{
  m_Left.Create(Archive.CreateFile("StatusBar\\StatusBarL.tga"));
  m_Middle.Create(Archive.CreateFile("StatusBar\\StatusBarM.tga"));
  m_Right.Create(Archive.CreateFile("StatusBar\\StatusBarR.tga"));

  m_bTransparent = TRUE;
  CRect rc(0, 0, 10, 10);
  CWindow::Create(&rc, pParent);
  CWindow::HideWindow();
}

void CMapViewStatusBar::Delete()
{
  CWindow::Delete();
  m_Right.Delete(); m_Middle.Delete(); m_Left.Delete();
}

void CMapViewStatusBar::Show(CString strText, DWORD dwColor)
{
  m_strText = strText;
  m_dwColor = dwColor;

  CSize sz = g_pSystemFont->GetTextSize(strText);
  sz.cx += STATUSBAR_LEFT + STATUSBAR_RIGHT;

  CSize szParent = GetParentWindow()->GetWindowPosition().Size();
  DWORD dwMinX = m_Left.GetAllRect()->Width() + m_Right.GetAllRect()->Width();
  CRect rcWindow;
  rcWindow.left = 0;
  rcWindow.top = szParent.cy - m_Left.GetAllRect()->Height();
  rcWindow.bottom = szParent.cy;
  rcWindow.right = sz.cx;
  if(sz.cx < (int)dwMinX) rcWindow.right = dwMinX;

  CWindow::SetWindowPosition(&rcWindow);

  CWindow::ShowWindow();
}

void CMapViewStatusBar::Hide()
{
  CWindow::HideWindow();
}

void CMapViewStatusBar::Draw(CDDrawSurface *pDDSurface, CRect *pRect)
{
  DWORD dwXPos = 0, dwYPos;
  CRect rcWindow = GetWindowPosition();
  dwYPos = rcWindow.Height() - m_Left.GetAllRect()->Height();
  pDDSurface->Paste(dwXPos, dwYPos, &m_Left);
  dwXPos += m_Left.GetAllRect()->Width();

  DWORD dwLen = rcWindow.Width() - m_Left.GetAllRect()->Width() - m_Right.GetAllRect()->Width();
  CRect rc;

  while(dwLen > 0){
    if((int)dwLen >= m_Middle.GetAllRect()->Width()){
      pDDSurface->Paste(dwXPos, dwYPos, &m_Middle);
      dwLen -= m_Middle.GetAllRect()->Width();
      dwXPos += m_Middle.GetAllRect()->Width();
    }
    else{
      rc.left = 0; rc.top = 0; rc.bottom = m_Middle.GetAllRect()->Height();
      rc.right = dwLen;
      pDDSurface->Paste(dwXPos, dwYPos, &m_Middle, &rc);
      dwXPos += dwLen;
      dwLen = 0;
    }
  }

  pDDSurface->Paste(dwXPos, dwYPos, &m_Right);

  g_pSystemFont->PaintText(STATUSBAR_LEFT, rcWindow.Height() - STATUSBAR_BOTTOM - g_pSystemFont->GetCharSize('A').cy,
    m_strText, pDDSurface, m_dwColor);
}