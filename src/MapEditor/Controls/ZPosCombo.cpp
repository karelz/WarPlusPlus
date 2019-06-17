// ZPosCombo.cpp : implementation file
//

#include "stdafx.h"
#include "..\MapEditor.h"
#include "ZPosCombo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CZPosCombo

CZPosCombo::CZPosCombo()
{
}

CZPosCombo::~CZPosCombo()
{
}


BEGIN_MESSAGE_MAP(CZPosCombo, CComboBox)
	//{{AFX_MSG_MAP(CZPosCombo)
	ON_WM_CHAR()
	ON_WM_KEYDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CZPosCombo message handlers

void CZPosCombo::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
  switch(nChar){
  case 13:
    SendMessage(WM_COMMAND, GetDlgCtrlID(), (LPARAM)GetSafeHwnd());
    break;
  }
  
	CComboBox::OnChar(nChar, nRepCnt, nFlags);
}

void CZPosCombo::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
  switch(nChar){
  case VK_RETURN:
    SendMessage(WM_COMMAND, GetDlgCtrlID(), (LPARAM)GetSafeHwnd());
    break;
  }
  
	CComboBox::OnKeyDown(nChar, nRepCnt, nFlags);
}
