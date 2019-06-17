// UnitLibrariesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\MapEditor.h"
#include "UnitLibrariesDlg.h"

#include "UnitDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CUnitLibrariesDlg dialog


CUnitLibrariesDlg::CUnitLibrariesDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CUnitLibrariesDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CUnitLibrariesDlg)
	//}}AFX_DATA_INIT
}


void CUnitLibrariesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CUnitLibrariesDlg)
	DDX_Control(pDX, IDC_LIBRARYCOMBO, m_wndLibraryCombo);
	DDX_Control(pDX, IDC_UNITLISTCTRL, m_wndUnitListCtrl);
	DDX_Control(pDX, IDC_EDITUNIT, m_wndEdit);
	DDX_Control(pDX, IDC_DELETEUNIT, m_wndDelete);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CUnitLibrariesDlg, CDialog)
	//{{AFX_MSG_MAP(CUnitLibrariesDlg)
	ON_CBN_SELENDOK(IDC_LIBRARYCOMBO, OnSelEndOkLibraryCombo)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_UNITLISTCTRL, OnItemChangedUnitListCtrl)
	ON_NOTIFY(NM_DBLCLK, IDC_UNITLISTCTRL, OnDblClkUnitListCtrl)
	ON_BN_CLICKED(IDC_ADDUNIT, OnAddUnit)
	ON_BN_CLICKED(IDC_DELETEUNIT, OnDeleteUnit)
	ON_BN_CLICKED(IDC_EDITUNIT, OnEditUnit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUnitLibrariesDlg message handlers

void CUnitLibrariesDlg::UpdateSelectedLibrary()
{
  int nSel = m_wndLibraryCombo.GetCurSel();
  if(nSel == -1){
    m_pCurrentUnitLibrary = NULL;
    return;
  }
  m_pCurrentUnitLibrary = (CEUnitLibrary *)m_wndLibraryCombo.GetItemData(nSel);
}

BOOL CUnitLibrariesDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
  CStatic *pPreview;
  DWORD dwStyle;
  CRect rcPreview;
  pPreview = (CStatic *)GetDlgItem(IDC_UNITPREVIEW);
  dwStyle = pPreview->GetStyle();
  pPreview->GetWindowRect(&rcPreview);
  ScreenToClient(&rcPreview);
  pPreview->DestroyWindow();

  m_wndUnitPreview.Create(rcPreview, dwStyle, this, IDC_UNITPREVIEW);

  CRect rcList; m_wndUnitListCtrl.GetClientRect(&rcList);
  m_wndUnitListCtrl.SetExtendedStyle(LVS_EX_FULLROWSELECT);
  m_wndUnitListCtrl.InsertColumn(0, "", LVCFMT_LEFT, rcList.Width() - 16);
  m_pMap->FillUnitLibrariesComboBox(&m_wndLibraryCombo);
  m_wndLibraryCombo.SetCurSel(0);
  UpdateSelectedLibrary();
  UpdateListItems();
  UpdateSelectedUnit();
  
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CUnitLibrariesDlg::UpdateListItems()
{
  if(m_pCurrentUnitLibrary == NULL){
    m_wndUnitListCtrl.DeleteAllItems();
    return;
  }
  
  m_pCurrentUnitLibrary->FillUnitTypesListCtrl(&m_wndUnitListCtrl);
}

void CUnitLibrariesDlg::UpdateSelectedUnit()
{
  CWaitCursor wait;

  if(m_wndUnitListCtrl.GetSelectedCount() == 0){
    m_pCurrentUnit = NULL;
    m_wndDelete.EnableWindow(FALSE);
    m_wndEdit.EnableWindow(FALSE);
    m_wndUnitPreview.SetUnitType(NULL);
    return;
  }
  int nSel = m_wndUnitListCtrl.GetNextItem(-1, LVNI_SELECTED);
  m_pCurrentUnit = (CEUnitType *)m_wndUnitListCtrl.GetItemData(nSel);
  m_wndDelete.EnableWindow(TRUE);
  m_wndEdit.EnableWindow(TRUE);
  m_wndUnitPreview.SetUnitType(m_pCurrentUnit);
}

void CUnitLibrariesDlg::Create(CEMap *pMap)
{
  ASSERT_VALID(pMap);

  m_pMap = pMap;
}

void CUnitLibrariesDlg::OnSelEndOkLibraryCombo() 
{  
  UpdateSelectedLibrary();
  UpdateListItems();
  UpdateSelectedUnit();
}

void CUnitLibrariesDlg::OnItemChangedUnitListCtrl(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

  if(pNMListView->uChanged & LVIF_STATE)
    UpdateSelectedUnit();
  
	*pResult = 0;
}

void CUnitLibrariesDlg::OnDblClkUnitListCtrl(NMHDR* pNMHDR, LRESULT* pResult) 
{
  OnEditUnit();	
		
	*pResult = 0;
}

void CUnitLibrariesDlg::OnAddUnit() 
{
  CEUnitType *pUnitType;

  if(m_pCurrentUnitLibrary == NULL) return;

  pUnitType = m_pCurrentUnitLibrary->NewUnitType();

  CString str;
  str.Format("Jednotka - %s", pUnitType->GetName());
  CUnitDlg dlg(str, this);

  dlg.Create(pUnitType, m_pMap);
  if(dlg.DoModal() != IDOK){
    dlg.Delete();
    m_pCurrentUnitLibrary->DeleteUnitType(pUnitType);
    return;
  }
  dlg.Delete();

  UpdateListItems();
  UpdateSelectedUnit();
}

void CUnitLibrariesDlg::OnDeleteUnit() 
{
	if(m_pCurrentUnit == NULL) return;

  CString str;
  str.Format("Opravdu chcete smazat jednotku '%s'?", m_pCurrentUnit->GetName());
  if(AfxMessageBox(str, MB_ICONEXCLAMATION | MB_YESNO) != IDYES){
    return;
  }

  m_pCurrentUnitLibrary->DeleteUnitType(m_pCurrentUnit);

  UpdateListItems();
  UpdateSelectedUnit();
}

void CUnitLibrariesDlg::OnEditUnit() 
{
  CWaitCursor wait;

  if(m_pCurrentUnit == NULL) return;

  CString str;
  str.Format("Jednotka - %s", m_pCurrentUnit->GetName());
  CUnitDlg dlg(str, this);
  dlg.Create(m_pCurrentUnit, m_pMap);
  dlg.DoModal();
  dlg.Delete();

  UpdateListItems();
  UpdateSelectedUnit();	
}

void CUnitLibrariesDlg::OnOK() 
{
	CDialog::OnOK();
}
