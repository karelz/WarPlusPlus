// UnitSkillsPage.cpp : implementation file
//

#include "stdafx.h"
#include "..\MapEditor.h"
#include "UnitSkillsPage.h"
#include "UnitLibrariesDlg.h"

#include "NewSkillTypeDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CUnitSkillsPage property page

IMPLEMENT_DYNCREATE(CUnitSkillsPage, CPropertyPage)

CUnitSkillsPage::CUnitSkillsPage() : CPropertyPage(CUnitSkillsPage::IDD)
{
	//{{AFX_DATA_INIT(CUnitSkillsPage)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CUnitSkillsPage::~CUnitSkillsPage()
{
}

void CUnitSkillsPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CUnitSkillsPage)
	DDX_Control(pDX, IDC_UP, m_wndUp);
	DDX_Control(pDX, IDC_DOWN, m_wndDown);
	DDX_Control(pDX, IDC_ENABLED, m_wndEnabled);
	DDX_Control(pDX, IDC_EDIT, m_wndEdit);
	DDX_Control(pDX, IDC_DELETE, m_wndDelete);
	DDX_Control(pDX, IDC_MODELIST, m_wndModeList);
	DDX_Control(pDX, IDC_LIST, m_wndList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CUnitSkillsPage, CPropertyPage)
	//{{AFX_MSG_MAP(CUnitSkillsPage)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST, OnItemChangedList)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST, OnDblClkList)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	ON_BN_CLICKED(IDC_EDIT, OnEdit)
	ON_NOTIFY(NM_CLICK, IDC_MODELIST, OnClickModeList)
	ON_BN_CLICKED(IDC_DOWN, OnDown)
	ON_BN_CLICKED(IDC_UP, OnUp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUnitSkillsPage message handlers

void CUnitSkillsPage::Create(CEUnitType *pUnitType)
{
  ASSERT_VALID(pUnitType);

  m_pUnitType = pUnitType;
}

BOOL CUnitSkillsPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
  CRect rcList;
  m_wndList.GetClientRect(&rcList);
  m_wndList.InsertColumn(0, "Jméno", LVCFMT_LEFT, rcList.Width() - 168, 0);
  int i;
  CString str;
  for(i = 1; i <= 8; i++){
    str.Format("%d", i);
    m_wndList.InsertColumn(i, str, LVCFMT_LEFT, 19, i);
  }
  m_wndList.SetExtendedStyle(LVS_EX_FULLROWSELECT);

  m_CheckModeImageList.Create(IDB_USMCHECKIMAGELIST, 16, 1, RGB(192, 192, 192));
  m_wndModeList.SetImageList(&m_CheckModeImageList, LVSIL_SMALL);
  m_pUnitType->FillModesListCtrl(&m_wndModeList);
  
  UpdateSkillsList();
  UpdateSelectedSkill();
  UpdateSelectedProps();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CUnitSkillsPage::UpdateSkillsList()
{
  int nSel;
  nSel = m_wndList.GetNextItem(-1, LVNI_SELECTED);
  CEUnitSkillType *pSel;
  if(nSel == -1) pSel = NULL;
  else pSel = (CEUnitSkillType *)m_wndList.GetItemData(nSel);

  m_wndList.DeleteAllItems();

  POSITION pos;
  int i, j, nItem;
  CEUnitSkillType *pSkillType;

  pos = m_pUnitType->GetFirstSkillTypePosition();
  i = 0;
  while(pos != NULL){
    pSkillType = m_pUnitType->GetNextSkillType(pos);

    nItem = m_wndList.InsertItem(i++, pSkillType->GetName());

    if(pSkillType == pSel) nSel = nItem;

    m_wndList.SetItemData(nItem, (DWORD)pSkillType);
    for(j = 0; j < 8; j++){
      if(pSkillType->GetAllowedMode(j)){
        m_wndList.SetItemText(nItem, j + 1, "#");
      }
      else{
        m_wndList.SetItemText(nItem, j + 1, "");
      }
    }
  }

  m_wndList.SetItemState(nSel, LVIS_SELECTED, LVIS_SELECTED);
}

void CUnitSkillsPage::OnAdd() 
{
  CNewSkillTypeDlg dlg;
  
  WriteSelectedProps();

  if(dlg.DoModal() != IDOK)
    return;

  CUnitSkillTypeDlg *pTypeDlg = dlg.GetSelectedSkillType();
  CEUnitSkillType *pSkillType;
  pSkillType = m_pUnitType->AddSkillType(pTypeDlg->GetName());

  pTypeDlg->InitNewSkill(pSkillType);

  UpdateSkillsList();
  UpdateSelectedSkill();
  UpdateSelectedProps();
}

void CUnitSkillsPage::UpdateSelectedSkill()
{
  if(m_wndList.GetSelectedCount() == 0){
    m_pCurrentSkillType = NULL;
    m_wndEdit.EnableWindow(FALSE);
    m_wndDelete.EnableWindow(FALSE);
    m_wndUp.EnableWindow(FALSE);
    m_wndDown.EnableWindow(FALSE);
    return;
  }
  int nSel = m_wndList.GetNextItem(-1, LVNI_SELECTED);
  m_pCurrentSkillType = (CEUnitSkillType *)m_wndList.GetItemData(nSel);
  m_wndEdit.EnableWindow(TRUE);
  m_wndDelete.EnableWindow(TRUE);
  if(m_pUnitType->GetPrevSkillType(m_pCurrentSkillType) != NULL)
    m_wndUp.EnableWindow(TRUE);
  else
    m_wndUp.EnableWindow(FALSE);
  if(m_pUnitType->GetNextSkillType(m_pCurrentSkillType) != NULL)
    m_wndDown.EnableWindow(TRUE);
  else
    m_wndDown.EnableWindow(FALSE);
}

void CUnitSkillsPage::OnItemChangedList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

  if(pNMListView->uChanged & LVIF_STATE){
    WriteSelectedProps();
    UpdateSelectedSkill();
    UpdateSelectedProps();
  }
  
	*pResult = 0;
}

void CUnitSkillsPage::OnDblClkList(NMHDR* pNMHDR, LRESULT* pResult) 
{
  OnEdit();
  
	*pResult = 0;
}

void CUnitSkillsPage::OnDelete() 
{
  if(m_pCurrentSkillType == NULL) return;

  CString str;
  str.Format("Opravdu chcete odstranit dovednost '%s'?", m_pCurrentSkillType->GetName());
  if(AfxMessageBox(str, MB_YESNO) != IDYES){
    return;
  }

  CUnitSkillTypeDlg *pTypeDlg = g_pUnitSkillTypeCollection->GetByName(m_pCurrentSkillType->GetName());
  if(pTypeDlg != NULL)
    pTypeDlg->DeleteSkill(m_pCurrentSkillType);
  m_pUnitType->DeleteSkillType(m_pCurrentSkillType);
  UpdateSkillsList();
  UpdateSelectedSkill();
  UpdateSelectedProps();
}

void CUnitSkillsPage::OnEdit() 
{
  WriteSelectedProps();

  if(m_pCurrentSkillType == NULL) return;

  CUnitSkillTypeDlg *pTypeDlg = g_pUnitSkillTypeCollection->GetByName(m_pCurrentSkillType->GetName());
  if(pTypeDlg == NULL){
    AfxMessageBox("Neznámý typ dovednosti. (vnitøní chyba)");
    return;
  }

  pTypeDlg->InitSkillTypeDlg(m_pCurrentSkillType);

  if(pTypeDlg->DoModal() == IDOK){
    pTypeDlg->CloseSkillType(TRUE);
  }
  else{
    pTypeDlg->CloseSkillType(FALSE);
  }

  UpdateSkillsList();
  UpdateSelectedSkill();
  UpdateSelectedProps();
}

void CUnitSkillsPage::UpdateSelectedProps()
{
  int nCount, i;
  if(m_pCurrentSkillType == NULL){
    nCount = m_wndModeList.GetItemCount();
    for(i = 0; i < nCount; i++){
      m_wndModeList.SetItem(i, 0, LVIF_IMAGE, NULL, 0, 0, 0, 0);
    }
    m_wndModeList.EnableWindow(FALSE);

    m_wndEnabled.SetCheck(0);
    m_wndEnabled.EnableWindow(FALSE);
    return;
  }

  nCount = m_wndModeList.GetItemCount();
  for(i = 0; i < nCount; i++){
    if(m_pCurrentSkillType->GetAllowedMode(i)){
      m_wndModeList.SetItem(i, 0, LVIF_IMAGE, NULL, 1, 0, 0, 0);
    }
    else{
      m_wndModeList.SetItem(i, 0, LVIF_IMAGE, NULL, 0, 0, 0, 0);
    }
  }
  m_wndModeList.EnableWindow(TRUE);

  if(m_pCurrentSkillType->GetEnabled()){
    m_wndEnabled.SetCheck(1);
  }
  else{
    m_wndEnabled.SetCheck(0);
  }
  m_wndEnabled.EnableWindow(TRUE);
}

void CUnitSkillsPage::OnClickModeList(NMHDR* pNMHDR, LRESULT* pResult) 
{
  if(m_pCurrentSkillType == NULL){
  	*pResult = 0;
    return;
  }
  if(m_wndModeList.GetSelectedCount() == 0){
  	*pResult = 0;
    return;
  }

  int nSel = m_wndModeList.GetNextItem(-1, LVNI_SELECTED);
  DWORD dwMode = m_wndModeList.GetItemData(nSel);
  BOOL bChecked;
  if(m_pCurrentSkillType->GetAllowedMode(dwMode)){
    m_pCurrentSkillType->SetAllowedMode(dwMode, FALSE);
    bChecked = FALSE;
  }
  else{
    m_pCurrentSkillType->SetAllowedMode(dwMode, TRUE);
    bChecked = TRUE;
  }
  UpdateSelectedProps();

  int i, nCount = m_wndList.GetItemCount();
  for(i = 0; i < nCount; i++){
    if(((CEUnitSkillType *)m_wndList.GetItemData(i)) == m_pCurrentSkillType){
      if(bChecked){
        m_wndList.SetItemText(i, dwMode + 1, "#");
      }
      else{
        m_wndList.SetItemText(i, dwMode + 1, "");
      }
    }
  }
  
	*pResult = 0;
}

void CUnitSkillsPage::WriteSelectedProps()
{
  if(m_pCurrentSkillType == NULL) return;

  UpdateData(TRUE);
  if(m_wndEnabled.GetCheck() == 0)
    m_pCurrentSkillType->SetEnabled(FALSE);
  if(m_wndEnabled.GetCheck() == 1)
    m_pCurrentSkillType->SetEnabled(TRUE);
}

void CUnitSkillsPage::OnOK() 
{
  WriteSelectedProps();
  
	CPropertyPage::OnOK();
}

void CUnitSkillsPage::OnDown() 
{
  if(m_pCurrentSkillType == NULL) return;
  m_pUnitType->MoveSkillTypeDown(m_pCurrentSkillType);	
  UpdateSkillsList();
}

void CUnitSkillsPage::OnUp() 
{
	if(m_pCurrentSkillType == NULL) return;
  m_pUnitType->MoveSkillTypeUp(m_pCurrentSkillType);
  UpdateSkillsList();
}
