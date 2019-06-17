// ResourceControl.cpp : implementation file
//

#include "stdafx.h"
#include "..\MapEditor.h"
#include "ResourceControl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CResourceControl

CResourceControl::CResourceControl()
{
}

CResourceControl::~CResourceControl()
{
}


BEGIN_MESSAGE_MAP(CResourceControl, CWnd)
	//{{AFX_MSG_MAP(CResourceControl)
	ON_WM_CTLCOLOR()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CResourceControl message handlers

void CResourceControl::Create(CWnd *pParent, UINT nID, int *pResources)
{
  ASSERT(pResources != NULL);

  CStatic *pPreview;
  DWORD dwStyle;
  CRect rcPreview;
  pPreview = (CStatic *)pParent->GetDlgItem(nID);
  dwStyle = pPreview->GetStyle();
  pPreview->GetWindowRect(&rcPreview);
  pParent->ScreenToClient(&rcPreview);
  pPreview->DestroyWindow();

  m_Font.CreatePointFont(80, "MS Sans Serif", NULL);
  m_BkgBrush.CreateSolidBrush(RGB(255, 255, 255));

  CWnd::Create(NULL, "", WS_CHILD | WS_VISIBLE, rcPreview, pParent, nID);
  CRect rcInner(0, 0, rcPreview.Width(), rcPreview.Height());
  m_wndInner.m_pResources = pResources;
  m_wndInner.Create("", dwStyle | SS_NOTIFY, rcInner, this);
  m_wndInner.SetFont(&m_Font);
  m_wndInner.ResetText();
}

HBRUSH CResourceControl::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
  pDC->SetBkColor(RGB(255, 255, 255));
  
	return (HBRUSH)m_BkgBrush;
}

void CResourceControl::OnDestroy() 
{
	CWnd::OnDestroy();
	
  m_Font.DeleteObject ();
  m_BkgBrush.DeleteObject ();
}
