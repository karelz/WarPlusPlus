// SelectUnitdlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\MapEditor.h"
#include "SelectUnitdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSelectUnitDlg dialog

int CSelectUnitDlg::m_nLastSelectedCivilization = 0;
int CSelectUnitDlg::m_nLastSelectedLibrary = 0;


CSelectUnitDlg::CSelectUnitDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSelectUnitDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSelectUnitDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
  m_pCurrentUnitType = NULL;
  m_pCurrentUnitLibrary = NULL;
  m_pCurrentCivilization = NULL;
}


void CSelectUnitDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSelectUnitDlg)
	DDX_Control(pDX, IDOK, m_OK);
	DDX_Control(pDX, IDC_UNITLIST, m_UnitList);
	DDX_Control(pDX, IDC_CIVCOMBO, m_CivCombo);
	DDX_Control(pDX, IDC_LIBRARYCOMBO, m_LibraryCombo);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSelectUnitDlg, CDialog)
	//{{AFX_MSG_MAP(CSelectUnitDlg)
	ON_CBN_SELENDOK(IDC_LIBRARYCOMBO, OnSelendokLibraryCombo)
	ON_CBN_SELENDOK(IDC_CIVCOMBO, OnSelendokCivCombo)
	ON_NOTIFY(NM_DBLCLK, IDC_UNITLIST, OnDblclkUnitList)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_UNITLIST, OnItemchangedUnitList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSelectUnitDlg message handlers

void CSelectUnitDlg::UpdateListItems()
{
  if(m_pCurrentUnitLibrary == NULL){
    m_UnitList.DeleteAllItems();
    return;
  }
  
  m_pCurrentUnitLibrary->FillUnitTypesListCtrl(&m_UnitList);
}

void CSelectUnitDlg::UpdateSelectedLibrary()
{
  int nSel = m_LibraryCombo.GetCurSel();
  m_nLastSelectedLibrary = nSel;
  if(nSel == -1){
    m_pCurrentUnitLibrary = NULL;
    return;
  }
  m_pCurrentUnitLibrary = (CEUnitLibrary *)m_LibraryCombo.GetItemData(nSel);
}

void CSelectUnitDlg::UpdateSelectedCiv()
{
  int nSel = m_CivCombo.GetCurSel();
  m_nLastSelectedCivilization = nSel;
  if(nSel == -1){
    m_pCurrentCivilization = NULL;
    m_OK.EnableWindow(FALSE);
    return;
  }
  m_pCurrentCivilization = (CECivilization *)m_CivCombo.GetItemData(nSel);
  if(m_pCurrentUnitType != NULL)
    m_OK.EnableWindow(TRUE);
  else
    m_OK.EnableWindow(FALSE);
}

void CSelectUnitDlg::UpdateSelectedUnit()
{
  if(m_UnitList.GetSelectedCount() == 0){
    m_pCurrentUnitType = NULL;
    m_OK.EnableWindow(FALSE);
    m_wndUnitTypePreview.SetUnitType(NULL);
    return;
  }
  int nSel = m_UnitList.GetNextItem(-1, LVNI_SELECTED);
  m_pCurrentUnitType = (CEUnitType *)m_UnitList.GetItemData(nSel);
  if(m_pCurrentCivilization != NULL)
    m_OK.EnableWindow(TRUE);
  else
    m_OK.EnableWindow(FALSE);

  m_wndUnitTypePreview.SetUnitType(m_pCurrentUnitType);
}

BOOL CSelectUnitDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
  CWnd *pPreview = GetDlgItem(IDC_UNITPREVIEW);
  DWORD dwStyle = pPreview->GetStyle();
  CRect rcPreview;
  pPreview->GetWindowRect(&rcPreview);
  ScreenToClient(&rcPreview);
  pPreview->DestroyWindow();
  m_wndUnitTypePreview.Create(rcPreview, dwStyle, this, IDC_UNITPREVIEW);
  
  CRect rcList; m_UnitList.GetClientRect(&rcList);
  m_UnitList.SetExtendedStyle(LVS_EX_FULLROWSELECT);
  m_UnitList.InsertColumn(0, "", LVCFMT_LEFT, rcList.Width() - 16);
  m_pMap->FillUnitLibrariesComboBox(&m_LibraryCombo);
  m_LibraryCombo.SetCurSel(m_nLastSelectedLibrary);
  m_pMap->FillCivilizationsComboBox(&m_CivCombo);
  m_CivCombo.SetCurSel(m_nLastSelectedCivilization);
  UpdateSelectedLibrary();
  UpdateListItems();
  UpdateSelectedUnit();
  UpdateSelectedCiv();
  
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSelectUnitDlg::OnSelendokLibraryCombo() 
{
  UpdateSelectedLibrary();
  UpdateListItems();
  UpdateSelectedUnit();
}

void CSelectUnitDlg::OnSelendokCivCombo() 
{
  UpdateSelectedCiv();
}

void CSelectUnitDlg::Create(CEMap *pMap)
{
  m_pMap = pMap;
}

void CSelectUnitDlg::OnDblclkUnitList(NMHDR* pNMHDR, LRESULT* pResult) 
{
  if(m_pCurrentUnitType == NULL) return;
  EndDialog(IDOK);	
	
	*pResult = 0;
}

void CSelectUnitDlg::OnItemchangedUnitList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	
  if(pNMListView->uChanged & LVIF_STATE)
    UpdateSelectedUnit();

	*pResult = 0;
}
