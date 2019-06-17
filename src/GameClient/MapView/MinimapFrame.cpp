// MinimapFrame.cpp: implementation of the CMinimapFrame class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MinimapFrame.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// Be carefull this must be made to fit (or be less) than the max size of the minimap itself
// plus the borders
#define MINIMAPFRAME_MAXSIZE_X 210
#define MINIMAPFRAME_MAXSIZE_Y 210

#define MINIMAPFRAME_BORDER_LEFT 7
#define MINIMAPFRAME_BORDER_RIGHT 7
#define MINIMAPFRAME_BORDER_TOP 12
#define MINIMAPFRAME_BORDER_BOTTOM 6

IMPLEMENT_DYNAMIC(CMinimapFrame, CFrameWindow)

BEGIN_OBSERVER_MAP(CMinimapFrame, CFrameWindow)
  THECOMMAND(IDC_CLOSE, OnClose)
  THECOMMAND(IDC_PLUS, OnPlus)
  THECOMMAND(IDC_MINUS, OnMinus)
END_OBSERVER_MAP(CMinimapFrame, CFrameWindow)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMinimapFrame::CMinimapFrame()
{
}

CMinimapFrame::~CMinimapFrame()
{
}

#ifdef _DEBUG

void CMinimapFrame::AssertValid() const
{
  CFrameWindow::AssertValid();
}

void CMinimapFrame::Dump(CDumpContext &dc) const
{
  CFrameWindow::Dump(dc);
}

#endif


void CMinimapFrame::Create(CRect &rcRect, CWindow *pParent, CCMap *pMap, CDataArchive *pGraphicsArchive,
                           CViewport *pViewport, CCMiniMapClip *pMiniMapClip)
{
  ASSERT(pMap != NULL);
  ASSERT(pGraphicsArchive != NULL);

  // Create the layout
  m_Layout.Create(pGraphicsArchive->CreateFile("MapView\\MiniMap\\MiniMap.window"));
  m_CloseLayout.Create(pGraphicsArchive->CreateFile("MapView\\MiniMap\\MinClose.button"));
  m_PlusLayout.Create(pGraphicsArchive->CreateFile("MapView\\MiniMap\\Plus.button"));
  m_MinusLayout.Create(pGraphicsArchive->CreateFile("MapView\\MiniMap\\Minus.button"));

  // Create the frame window
  VERIFY(CFrameWindow::Create(rcRect, &m_Layout, pParent, TRUE));
  m_bTransparent = FALSE;
  SetBuffering(MINIMAPFRAME_MAXSIZE_X, MINIMAPFRAME_MAXSIZE_Y);

  // Create the minimap
  {
    CRect rcMinimap(MINIMAPFRAME_BORDER_LEFT, MINIMAPFRAME_BORDER_TOP,
      rcRect.Width() - MINIMAPFRAME_BORDER_RIGHT, rcRect.Height() - MINIMAPFRAME_BORDER_BOTTOM);

    m_wndMinimap.Create(rcMinimap, this, pMap, pViewport, pMiniMapClip);
  }

  {
    m_wndClose.SetTopMost();
    VERIFY(m_wndClose.Create(CPoint(rcRect.Width() - 25, 0), &m_CloseLayout, this));
    m_wndClose.Connect(this, IDC_CLOSE);
    CRect rc(7, 0, 25, 19);
    m_wndClose.SetSensitiveRectangle(&rc);
  }
  {
    m_wndPlus.SetTopMost();
    VERIFY(m_wndPlus.Create(CPoint(0, 0), &m_PlusLayout, this));
    m_wndPlus.Connect(this, IDC_PLUS);
    CRect rc(5, 5, 30, 15);
    m_wndPlus.SetSensitiveRectangle(&rc);
  }
  {
    m_wndMinus.SetTopMost();
    VERIFY(m_wndMinus.Create(CPoint(30, 0), &m_MinusLayout, this));
    m_wndMinus.Connect(this, IDC_MINUS);
    CRect rc(0, 5, 21, 15);
    m_wndMinus.SetSensitiveRectangle(&rc);
  }
}

void CMinimapFrame::Delete()
{
  // Delete the minimap
  m_wndMinimap.Delete();

  m_wndMinus.Delete();
  m_wndPlus.Delete();
  m_wndClose.Delete();
  // Delete the frame window
  CFrameWindow::Delete();

  // Delete the layout
  m_MinusLayout.Delete();  
  m_PlusLayout.Delete();
  m_CloseLayout.Delete();
  m_Layout.Delete();
}



CSize CMinimapFrame::GetMaximumSize()
{
  return CSize(MINIMAPFRAME_MAXSIZE_X, MINIMAPFRAME_MAXSIZE_Y);
}

CSize CMinimapFrame::GetMinimumSize()
{
  return CFrameWindow::GetMinimumSize();
}

void CMinimapFrame::OnSize(CSize size)
{
  CFrameWindow::OnSize(size);

  // Get the window rect
  CRect rcWnd = GetWindowPosition();

  // Compute the new position of the minimap
  CRect rcMinimap(MINIMAPFRAME_BORDER_LEFT, MINIMAPFRAME_BORDER_TOP,
    rcWnd.Width() - MINIMAPFRAME_BORDER_RIGHT, rcWnd.Height() - MINIMAPFRAME_BORDER_BOTTOM);

  // And set the new position (mainly the size) of the minimap view
  m_wndMinimap.SetWindowPosition(&rcMinimap);

  // Set new position of the close button
  CPoint ptClose(rcWnd.Width() - 25, 0);
  m_wndClose.SetWindowPosition(&ptClose);
}

void CMinimapFrame::OnClose()
{
  InlayEvent(E_CLOSEWINDOW, 0);
}

void CMinimapFrame::OnPlus()
{
  switch(m_wndMinimap.GetZoom()){
  case 4:
    m_wndMinimap.SetZoom(2); break;
  case 8:
    m_wndMinimap.SetZoom(4); break;
  case 16:
    m_wndMinimap.SetZoom(8); break;
  }
}

void CMinimapFrame::OnMinus()
{
  switch(m_wndMinimap.GetZoom()){
  case 2:
    m_wndMinimap.SetZoom(4); break;
  case 4:
    m_wndMinimap.SetZoom(8); break;
  case 8:
    m_wndMinimap.SetZoom(16); break;
  }
}