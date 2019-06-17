// ScreenWindow.cpp: implementation of the CScreenWindow class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ScreenWindow.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CScreenWindow, CWindow);

// Constructor
CScreenWindow::CScreenWindow()
{

}

// Destructor
CScreenWindow::~CScreenWindow()
{

}

// Debug functions
#ifdef _DEBUG

void CScreenWindow::AssertValid() const
{
  CWindow::AssertValid();
}

void CScreenWindow::Dump(CDumpContext &dc) const
{
  CWindow::Dump(dc);
}

#endif


// Creation

// Creates the window
BOOL CScreenWindow::Create(CWindow *pParent)
{
  ASSERT_VALID(g_pDDPrimarySurface);

  CRect rcWindow(g_pDDPrimarySurface->GetScreenRect());

  m_bTransparent = TRUE;

  return CWindow::Create(&rcWindow, pParent);
}

// Deletes the window
void CScreenWindow::Delete()
{
  CWindow::Delete();
}


// Drawing

// Draws the window -> it means does nothing
void CScreenWindow::Draw(CDDrawSurface *pSurface, CRect *pBoundRect)
{
  CWindow::Draw(pSurface, pBoundRect);
}