// ColorView.cpp : implementation file
//

#include "stdafx.h"
#include "ColorView.h"
#include "DirectX\DirectDraw.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CColorView

CColorView::CColorView()
{
  m_pPreview = NULL;
  m_dwColor = 0;
}

CColorView::~CColorView()
{
}


BEGIN_MESSAGE_MAP(CColorView, CStatic)
	//{{AFX_MSG_MAP(CColorView)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CColorView message handlers

DWORD CColorView::GetColor()
{
  return m_dwColor;
}

void CColorView::SetColor(DWORD dwColor)
{
  m_dwColor = dwColor;
}

void CColorView::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

  CRect rc;
  GetClientRect(&rc);

  CBrush brush(RGB(R32(m_dwColor), G32(m_dwColor), B32(m_dwColor)));

  dc.FillRect(&rc, &brush);
}

void CColorView::OnLButtonDown(UINT nFlags, CPoint point) 
{
  CColorDialog cdlg(RGB(R32(m_dwColor), G32(m_dwColor), B32(m_dwColor)), CC_ANYCOLOR | CC_FULLOPEN, this);

  if(cdlg.DoModal() == IDOK){
    COLORREF clr = cdlg.GetColor();
    m_dwColor = RGB32(GetRValue(clr), GetGValue(clr), GetBValue(clr));
    if(m_pPreview)
      m_pPreview->SetBkgColor(m_dwColor);
    Invalidate();
  }
}

void CColorView::SetPreview(CPreviewWindow *pPreview)
{
  m_pPreview = pPreview;
}
