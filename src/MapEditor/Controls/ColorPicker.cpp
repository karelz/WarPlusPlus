// ColorPicker.cpp : implementation file
//

#include "stdafx.h"
#include "..\MapEditor.h"
#include "ColorPicker.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CColorPicker

CColorPicker::CColorPicker()
{
  m_dwColor = 0;
}

CColorPicker::~CColorPicker()
{
}


BEGIN_MESSAGE_MAP(CColorPicker, CStatic)
	//{{AFX_MSG_MAP(CColorPicker)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CColorPicker message handlers

void CColorPicker::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
  CBrush brush;
  brush.CreateSolidBrush(RGB(R32(m_dwColor), G32(m_dwColor), B32(m_dwColor)));
  CRect rcClient;
  GetClientRect(&rcClient);
  dc.FillRect(&rcClient, &brush);
  brush.DeleteObject();
}

void CColorPicker::SetColor(DWORD dwColor)
{
  m_dwColor = dwColor;
  if(IsWindow(GetSafeHwnd()))
    InvalidateRect(NULL);
}

DWORD CColorPicker::GetColor()
{
  return m_dwColor;
}

void CColorPicker::OnLButtonDown(UINT nFlags, CPoint point) 
{
  COLORREF color = RGB(R32(m_dwColor), G32(m_dwColor), B32(m_dwColor));
  CColorDialog dlg(color, CC_ANYCOLOR | CC_FULLOPEN, this);

  if(dlg.DoModal() == IDOK){
    color = dlg.GetColor();
    m_dwColor = RGB32(GetRValue(color), GetGValue(color), GetBValue(color));
    InvalidateRect(NULL);
  }
  
	CStatic::OnLButtonDown(nFlags, point);
}
