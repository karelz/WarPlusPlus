// ColorControl.cpp : implementation file
//

#include "stdafx.h"
#include "GameServerApp.h"
#include "ColorControl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CColorControl

CColorControl::CColorControl()
{
  m_Color = RGB(0, 0, 0);
}

CColorControl::~CColorControl()
{
}


BEGIN_MESSAGE_MAP(CColorControl, CStatic)
	//{{AFX_MSG_MAP(CColorControl)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CColorControl message handlers

void CColorControl::Create(CRect &rcRect, DWORD dwStyle, CWnd *pParent, COLORREF Color, UINT nID)
{
  m_Color = Color;

  CStatic::Create("", dwStyle, rcRect, pParent, nID);
}

void CColorControl::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
  CRect rcClient;
  GetClientRect(&rcClient);

  dc.FillSolidRect(&rcClient, m_Color);
}
