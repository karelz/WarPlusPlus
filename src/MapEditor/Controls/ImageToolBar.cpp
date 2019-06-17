// ImageToolBar.cpp: implementation of the CImageToolBar class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ImageToolBar.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CImageToolBar, CToolBar)
  ON_WM_CREATE()
END_MESSAGE_MAP()

CImageToolBar::CImageToolBar()
{

}

CImageToolBar::~CImageToolBar()
{

}

void CImageToolBar::InsertImageButton(int nIndex, UINT nID)
{
  CToolBarCtrl &Ctrl = GetToolBarCtrl();

  TBBUTTON but;
  but.iBitmap = NULL;
  but.iString = NULL;
  but.fsState = TBSTATE_ENABLED;
  but.idCommand = nID;
  but.fsStyle = TBSTYLE_BUTTON;
  but.dwData = 1;
  Ctrl.InsertButton(nIndex, &but);
}

int CImageToolBar::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
  if(CToolBar::OnCreate(lpCreateStruct) == -1) return -1;

  CToolBarCtrl &Ctrl = GetToolBarCtrl();

  DWORD dwStyle = Ctrl.GetStyle();
  dwStyle &= ~(CCS_NODIVIDER);
  Ctrl.SetStyle(dwStyle);

  return 0;
}