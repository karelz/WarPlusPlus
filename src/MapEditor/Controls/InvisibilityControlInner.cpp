// InvisibilityControlInner.cpp : implementation file
//

#include "stdafx.h"
#include "..\MapEditor.h"
#include "InvisibilityControlInner.h"

#include "..\Dialogs\EditInvisibility.h"
#include "Common\Map\Map.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CInvisibilityControlInner

CInvisibilityControlInner::CInvisibilityControlInner()
{
}

CInvisibilityControlInner::~CInvisibilityControlInner()
{
}


BEGIN_MESSAGE_MAP(CInvisibilityControlInner, CStatic)
	//{{AFX_MSG_MAP(CInvisibilityControlInner)
	ON_WM_LBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInvisibilityControlInner message handlers

void CInvisibilityControlInner::ResetText()
{
  CString str;

  int i;
  DWORD dwMask = 1;
  for(i = 0; i < INVISIBILITY_COUNT; i++){
    if((*m_pInvisibility) & dwMask)
      str += "*";
    else
      str += ".";
    dwMask <<= 1;
  }
  SetWindowText(str);

  InvalidateRect(NULL);
}


void CInvisibilityControlInner::OnLButtonDown(UINT nFlags, CPoint point) 
{
  CEditInvisibility dlg;
  DWORD dwInvisibility = *m_pInvisibility;

  dlg.m_pInvisibility = &dwInvisibility;
  dlg.m_strTitle = m_strTitle;
  if(dlg.DoModal() == IDOK){
    *m_pInvisibility = dwInvisibility;
  }

  ResetText();

	CStatic::OnLButtonDown(nFlags, point);
}
