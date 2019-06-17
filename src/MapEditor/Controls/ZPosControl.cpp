// ZPosControl.cpp : implementation file
//

#include "stdafx.h"
#include "..\MapEditor.h"
#include "ZPosControl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CZPosControl

CZPosControl::CZPosControl()
{
}

CZPosControl::~CZPosControl()
{
}


BEGIN_MESSAGE_MAP(CZPosControl, CComboBox)
	//{{AFX_MSG_MAP(CZPosControl)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CZPosControl message handlers

void CZPosControl::Create(CWnd *pParent, UINT nID)
{
  CWnd *pPrev = pParent->GetDlgItem(nID);
  CRect rcRect;
  DWORD dwStyle = pPrev->GetStyle();
  pPrev->GetWindowRect(&rcRect);
  pParent->ScreenToClient(&rcRect);
  HFONT hFont = (HFONT)pPrev->SendMessage(WM_GETFONT);
  CString strText;
  pPrev->GetWindowText(strText);
  pPrev->DestroyWindow();

  rcRect.bottom = rcRect.top + 150;
  CComboBox::Create((dwStyle | CBS_DROPDOWN) & (~CBS_SORT), rcRect, pParent, nID);
  SetWindowText(strText + "   ");
  SendMessage(WM_SETFONT, (WPARAM)hFont, 0);

  AddString("200000 Vysoká letadla");
  AddString("150000 Nízká letadla");
  AddString("45000 Vznášedla");
  AddString("35000 Pozemní jednotky");
  AddString("25000 Podzemí");
  AddString("15000 Voda");
  AddString("5000 Pod vodou");
}

DWORD CZPosControl::GetZPos()
{
  CString strText;
  GetWindowText(strText);

  int nPos =  atol(strText);
  if(nPos < 0) return 0;
  return (DWORD)nPos;
}

void CZPosControl::SetZPos(DWORD dwPos)
{
  CString strText;
  strText.Format("%d", dwPos);
  SetWindowText(strText);
}