// UnitToolBar.cpp : implementation file
//

#include "stdafx.h"
#include "..\MapEditor.h"
#include "UnitToolBar.h"

#include "..\DataObjects\EMap.h"
#include "..\MapEditorDoc.h"
#include "..\Dialogs\SelectUnitDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define ID_UNITBUTTONS 20200

/////////////////////////////////////////////////////////////////////////////
// CUnitToolBar

CUnitToolBar::CUnitToolBar()
{
}

CUnitToolBar::~CUnitToolBar()
{
}


BEGIN_MESSAGE_MAP(CUnitToolBar, CToolBar)
	//{{AFX_MSG_MAP(CUnitToolBar)
	ON_WM_DESTROY()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
  ON_COMMAND_RANGE(ID_UNITBUTTONS, ID_UNITBUTTONS + 19, OnUnitButton)
  ON_NOTIFY_RANGE(CImageTBButton::N_DOUBLECLICKED, ID_UNITBUTTONS, ID_UNITBUTTONS + 19, OnUnitButtonDblClk)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUnitToolBar message handlers

DWORD CUnitToolBar::m_aUnitsSelected[20] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
DWORD CUnitToolBar::m_aCivilizationsSelected[20] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int CUnitToolBar::m_nUnitSelected = 0;

void CUnitToolBar::OnDestroy() 
{
	CToolBar::OnDestroy();
	
  DeleteButtons();
}

void CUnitToolBar::OnSize(UINT nType, int cx, int cy) 
{
	CToolBar::OnSize(nType, cx, cy);
	
  int i;
  CRect rcItem;
  for(i = 0; i < m_aButtons.GetSize(); i++){
    GetItemRect(i, &rcItem);
    m_aButtons[i]->SetWindowPos(NULL, rcItem.left, rcItem.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
  }
}

void CUnitToolBar::CreateButtons()
{
  int i;
  CRect rcItem;
  m_aButtons.SetSize(5);
  CUnitTBButton *pButton;
  for(i = 0; i < m_aButtons.GetSize(); i++){
    GetItemRect(i, &rcItem);
    pButton = new CUnitTBButton();
    pButton->Create(rcItem, this, ID_UNITBUTTONS + i);
    m_aButtons[i] = pButton;
  }
}

void CUnitToolBar::SaveState(CString strSection)
{
  CString strVariable;
  CWinApp *pApp = AfxGetApp();

  int i;
  for(i = 0; i < 20; i++){
    strVariable.Format("Unit%d", i);
    pApp->WriteProfileInt(strSection, strVariable, m_aUnitsSelected[i]);
    strVariable.Format("Civilization%d", i);
    pApp->WriteProfileInt(strSection, strVariable, m_aCivilizationsSelected[i]);
  }
  pApp->WriteProfileInt(strSection, "SelectedUnit", m_nUnitSelected);
}

void CUnitToolBar::LoadState(CString strSection)
{
  CString strVariable;
  CWinApp *pApp = AfxGetApp();

  int i;
  for(i = 0; i < 20; i++){
    strVariable.Format("Unit%d", i);
    m_aUnitsSelected[i] = pApp->GetProfileInt(strSection, strVariable, 0);
    strVariable.Format("Civilization%d", i);
    m_aCivilizationsSelected[i] = pApp->GetProfileInt(strSection, strVariable, 0);
  }
  m_nUnitSelected = pApp->GetProfileInt(strSection, "SelectedUnit", 0);
}

void CUnitToolBar::SetSelectedUnit(CEMap *pMap)
{
  int i;
  for(i = 0; i < m_aButtons.GetSize(); i++){
    if(pMap == NULL)
      m_aButtons[i]->SetUnitType(NULL, NULL);
    else
      m_aButtons[i]->SetUnitType(pMap->GetUnitType(m_aUnitsSelected[i]), pMap->GetCivilization(m_aCivilizationsSelected[i]));
  }
  if(m_aButtons.GetSize() > 0){
    m_aButtons[m_nUnitSelected]->SetSelected(TRUE);
//    AppGe
//    AppGetActiveDocument()->SetSelectedMapex(m_aButtons[m_nMapexSelected]->GetMapex());
  }
  Invalidate();
}

void CUnitToolBar::OnUnitButton(UINT nID)
{
  int nIndex = nID - ID_UNITBUTTONS;
  int i;
  for(i = 0; i < m_aButtons.GetSize(); i++){
    if(i == nIndex)
      m_aButtons[i]->SetSelected(TRUE);
    else
      m_aButtons[i]->SetSelected(FALSE);
  }

  CEUnitType *pUnitType = m_aButtons[nIndex]->GetUnitType();
  CECivilization *pCiv = m_aButtons[nIndex]->GetUnitCivilization();
  AppGetActiveDocument()->SetSelectedUnitType(pUnitType, pCiv);
}

void CUnitToolBar::OnUnitButtonDblClk(UINT nID, NMHDR *pNotifyStruct, LRESULT *pResult)
{
  int nIndex = nID - ID_UNITBUTTONS;

  CSelectUnitDlg dlg;
  dlg.Create(AppGetActiveMap());

  if(dlg.DoModal() == IDOK){
    m_aButtons[nIndex]->SetUnitType(dlg.GetSelectedUnitType(), dlg.GetSelectedCivilization());
    AppGetActiveDocument()->SetSelectedUnitType(dlg.GetSelectedUnitType(), dlg.GetSelectedCivilization());
    if((dlg.GetSelectedUnitType() != NULL) && (dlg.GetSelectedCivilization() != NULL)){
      m_aUnitsSelected[nIndex] = dlg.GetSelectedUnitType()->GetID();
      m_aCivilizationsSelected[nIndex] = dlg.GetSelectedCivilization()->GetID();
    }
    else{
      m_aUnitsSelected[nIndex] = 0;
      m_aCivilizationsSelected[nIndex] = 0;
    }
  }

  *pResult = 0;
}

void CUnitToolBar::ResetSelection()
{
  int i;

  for(i = 0; i < m_aButtons.GetSize(); i++){
    m_aButtons[i]->SetSelected(FALSE);
  }
}

void CUnitToolBar::DeleteButtons()
{
  int i;
  for(i = 0; i < m_aButtons.GetSize(); i++){
    if(m_aButtons[i]->GetUnitType() != NULL){
      m_aUnitsSelected[i] = m_aButtons[i]->GetUnitType()->GetID();
      if(m_aButtons[i]->GetUnitCivilization() != NULL)
        m_aCivilizationsSelected[i] = m_aButtons[i]->GetUnitCivilization()->GetID();
      else
        m_aCivilizationsSelected[i] = 0;
    }
    else{
      m_aUnitsSelected[i] = 0;
      m_aCivilizationsSelected[i] = 0;
    }

    if(m_aButtons[i]->GetSelected())
      m_nUnitSelected = i;

    m_aButtons[i]->DestroyWindow();
    delete m_aButtons[i];
  }
  m_aButtons.RemoveAll();
}