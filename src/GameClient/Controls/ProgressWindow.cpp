// ProgressWindow.cpp: implementation of the CProgressWindow class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\gameclient.h"
#include "ProgressWindow.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CProgressWindow, CFrameWindow);

CProgressWindow::CProgressWindow()
{

}

CProgressWindow::~CProgressWindow()
{

}

#ifdef _DEBUG
  
void CProgressWindow::AssertValid() const
{
  CFrameWindow::AssertValid();
}

void CProgressWindow::Dump(CDumpContext &dc) const
{
  CFrameWindow::Dump(dc);
}

#endif


void CProgressWindow::Create(CString strText, CWindow *pParent)
{
  CSize sz(250, 50);
  CRect rc;

  rc.left = (g_pDDPrimarySurface->GetScreenRect()->Width() - sz.cx) / 2;
  rc.right = rc.left + sz.cx;
  rc.top = (g_pDDPrimarySurface->GetScreenRect()->Height() - sz.cy) / 2;
  rc.bottom = rc.top + sz.cy;

  m_bTopMost = TRUE;

  CFrameWindow::Create(rc, CLayouts::m_pDefaults->GetPopupMenuLayout(), pParent, FALSE);

  rc.SetRect(17, 17, 233, 32);
  m_Text.Create(rc, NULL, strText, this);
}
