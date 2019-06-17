// InvisibilityControl.cpp : implementation file
//

#include "stdafx.h"
#include "..\MapEditor.h"
#include "InvisibilityControl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CInvisibilityControl

CInvisibilityControl::CInvisibilityControl()
{
}

CInvisibilityControl::~CInvisibilityControl()
{
}


BEGIN_MESSAGE_MAP(CInvisibilityControl, CWnd)
	//{{AFX_MSG_MAP(CInvisibilityControl)
	ON_WM_CTLCOLOR()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CInvisibilityControl message handlers

HBRUSH CInvisibilityControl::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{	
	return (HBRUSH)m_BkgBrush;
}

void CInvisibilityControl::Create(CWnd *pParent, UINT nID, DWORD *pInvisibility)
{
  ASSERT(pInvisibility != NULL);

  CStatic *pPreview;
  DWORD dwStyle;
  CRect rcPreview;
  pPreview = (CStatic *)pParent->GetDlgItem(nID);
  dwStyle = pPreview->GetStyle();
  pPreview->GetWindowRect(&rcPreview);
  pParent->ScreenToClient(&rcPreview);
  pPreview->DestroyWindow();

  m_Font.CreatePointFont(80, "Courier New");
  m_BkgBrush.CreateSolidBrush(RGB(255, 255, 255));

  CWnd::Create(NULL, "", WS_CHILD | WS_VISIBLE, rcPreview, pParent, nID);
  CRect rcInner(0, 0, rcPreview.Width(), rcPreview.Height());
  m_wndInner.m_pInvisibility = pInvisibility;
  m_wndInner.Create("", dwStyle | SS_NOTIFY, rcInner, this);
  m_wndInner.SetFont(&m_Font);
  m_wndInner.ResetText();
}

void CInvisibilityControl::SetInvisibility(DWORD *pInvisibility)
{
  m_wndInner.m_pInvisibility = pInvisibility;
  m_wndInner.ResetText();
}

void CInvisibilityControl::OnDestroy() 
{
	CWnd::OnDestroy();
	
	m_Font.DeleteObject ();
  m_BkgBrush.DeleteObject ();
}
