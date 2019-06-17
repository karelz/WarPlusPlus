// AppearanceControl.cpp : implementation file
//

#include "stdafx.h"
#include "AppearanceControl.h"

#include "..\DataObjects\EUnitType.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAppearanceControl

CAppearanceControl::CAppearanceControl()
{
}

CAppearanceControl::~CAppearanceControl()
{
}


BEGIN_MESSAGE_MAP(CAppearanceControl, CComboBox)
	//{{AFX_MSG_MAP(CAppearanceControl)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAppearanceControl message handlers

void CAppearanceControl::Create(CWnd *pParent, UINT nID, CEUnitType *pUnitType, DWORD dwSelectedAppearanceID, BOOL bNoneItem)
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
  // Fill the combo box with appearances
  pUnitType->FillAppearancesCombo(this);
  // Add the none appearance if wanted
  if(bNoneItem){
    int nItem;
    nItem = InsertString(0, "[žádný]");
    SetItemData(nItem, 0);
  }

  // Go through items and find the one to be selected
  {
    int nItem, nCount = GetCount();
    CEUnitAppearanceType *pAppType;

    for(nItem = 0; nItem < nCount; nItem++){
      pAppType = (CEUnitAppearanceType *)GetItemData(nItem);
      if(pAppType == NULL){
        if(dwSelectedAppearanceID == NULL)
          SetCurSel(nItem);
      }
      else{
        if(pAppType->GetID() == dwSelectedAppearanceID)
          SetCurSel(nItem);
      }
    }
  }
}

DWORD CAppearanceControl::GetSelectedAppearanceID()
{
  // Get the selected item
  int nItem = GetCurSel();
  if(nItem == CB_ERR) return 0;
  CEUnitAppearanceType *pAppType = (CEUnitAppearanceType *)GetItemData(nItem);
  if(pAppType == NULL){
      return NULL;
  }
  else{
      return pAppType->GetID();
  }
}