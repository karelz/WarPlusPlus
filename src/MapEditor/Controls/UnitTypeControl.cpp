// UnitTypeControl.cpp : implementation file
//

#include "stdafx.h"
#include "..\MapEditor.h"
#include "UnitTypeControl.h"

#include "..\DataObjects\EMap.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CUnitTypeControl

CUnitTypeControl::CUnitTypeControl()
{
}

CUnitTypeControl::~CUnitTypeControl()
{
}


BEGIN_MESSAGE_MAP(CUnitTypeControl, CComboBox)
	//{{AFX_MSG_MAP(CUnitTypeControl)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUnitTypeControl message handlers

void CUnitTypeControl::Create(CWnd *pParent, UINT nID, CEUnitType *pSelectedUnitType)
{
  CWnd *pPrev = pParent->GetDlgItem(nID);
  CRect rcRect;
  DWORD dwStyle = pPrev->GetStyle();
  pPrev->GetWindowRect(&rcRect);
  pParent->ScreenToClient(&rcRect);
  HFONT hFont = (HFONT)pPrev->SendMessage(WM_GETFONT);
  pPrev->DestroyWindow();

  rcRect.bottom = rcRect.top + 150;
  CComboBox::Create(dwStyle | CBS_DROPDOWNLIST, rcRect, pParent, nID);
  SendMessage(WM_SETFONT, (WPARAM)hFont, 0);

  AppGetActiveMap()->FillUnitTypesComboBox(this);

  SetSelectedUnitType(pSelectedUnitType);
}

void CUnitTypeControl::SetSelectedUnitType(CEUnitType *pUnitType)
{
  int i;
  int nCount = GetCount();
  CEUnitType *pType;

  for(i = 0; i < nCount; i++){
    pType = (CEUnitType *)GetItemData(i);
    if(pType == pUnitType){
      SetCurSel(i);
    }
  }
}

CEUnitType * CUnitTypeControl::GetSelectedUnitType()
{
  int nItem = GetCurSel();
  if(nItem == CB_ERR) return NULL;

  CEUnitType *pType = (CEUnitType *)GetItemData(nItem);

  return pType;
}

void CUnitTypeControl::SetSelectedUnitType(DWORD dwUnitType)
{
  CEUnitType *pType = AppGetActiveMap()->GetUnitType(dwUnitType);
  SetSelectedUnitType(pType);
}