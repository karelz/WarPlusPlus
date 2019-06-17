// UnitAppearancePage.cpp : implementation file
//

#include "stdafx.h"
#include "..\MapEditor.h"
#include "UnitAppearancePage.h"

#include "UnitAppearanceDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CUnitAppearancePage property page

IMPLEMENT_DYNCREATE(CUnitAppearancePage, CPropertyPage)

CUnitAppearancePage::CUnitAppearancePage() : CPropertyPage(CUnitAppearancePage::IDD)
{
	//{{AFX_DATA_INIT(CUnitAppearancePage)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CUnitAppearancePage::~CUnitAppearancePage()
{
}

void CUnitAppearancePage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CUnitAppearancePage)
	DDX_Control(pDX, IDC_EDIT, m_wndEdit);
	DDX_Control(pDX, IDC_DELETE, m_wndDelete);
	DDX_Control(pDX, IDC_LIST, m_wndList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CUnitAppearancePage, CPropertyPage)
	//{{AFX_MSG_MAP(CUnitAppearancePage)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST, OnItemChangedList)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST, OnDblClkList)
	ON_BN_CLICKED(IDC_EDIT, OnEdit)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUnitAppearancePage message handlers

void CUnitAppearancePage::Create(CEUnitType *pUnitType)
{
  ASSERT_VALID(pUnitType);

  m_pUnitType = pUnitType;
  m_pCurrentAppearance = NULL;
}

BOOL CUnitAppearancePage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
  ASSERT_VALID(m_pUnitType);

  CRect rcList; m_wndList.GetClientRect(&rcList);
  m_wndList.InsertColumn(0, "", LVCFMT_LEFT, rcList.Width() - 16);

  UpdateListItems();
  UpdateSelectedAppearance();
  
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CUnitAppearancePage::UpdateListItems()
{
  ASSERT_VALID(m_pUnitType);
  m_pUnitType->FillAppearancesListCtrl(&m_wndList, TRUE);
}

void CUnitAppearancePage::OnItemChangedList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

  if(pNMListView->uChanged & LVIF_STATE)
    UpdateSelectedAppearance();	

	*pResult = 0;
}

void CUnitAppearancePage::UpdateSelectedAppearance()
{
  if(m_wndList.GetSelectedCount() == 0){
    m_pCurrentAppearance = NULL;
    m_wndDelete.EnableWindow(FALSE);
    m_wndEdit.EnableWindow(FALSE);
    return;
  }
  int nSel = m_wndList.GetNextItem(-1, LVNI_SELECTED);
  m_pCurrentAppearance = (CEUnitAppearanceType *)m_wndList.GetItemData(nSel);
  if(m_pCurrentAppearance == m_pUnitType->GetSelectionMarkAppearance()){
    m_wndDelete.EnableWindow(FALSE);
    m_wndEdit.EnableWindow(TRUE);
  }
  else{
    m_wndDelete.EnableWindow(TRUE);
    m_wndEdit.EnableWindow(TRUE);
  }
}

void CUnitAppearancePage::OnDblClkList(NMHDR* pNMHDR, LRESULT* pResult) 
{
  OnEdit();		

	*pResult = 0;
}

void CUnitAppearancePage::OnEdit() 
{
  if(m_pCurrentAppearance == NULL) return;

  CUnitAppearanceDlg dlg;

  dlg.m_bEditName = TRUE;
  if(m_pCurrentAppearance == m_pUnitType->GetSelectionMarkAppearance()){
    dlg.m_bEditName = FALSE;
  }
  dlg.Create(m_pCurrentAppearance);
  dlg.DoModal();
  
  UpdateListItems();
  UpdateSelectedAppearance();
}

void CUnitAppearancePage::OnDelete() 
{
  if(m_pCurrentAppearance == NULL) return;
  if(m_pCurrentAppearance == m_pUnitType->GetSelectionMarkAppearance()) return;
  // ask it
  CString str;
  str.Format("Opravdu chcete odstranit vzhled '%s'?", m_pCurrentAppearance->GetName());
  if(AfxMessageBox(str, MB_ICONEXCLAMATION | MB_YESNO) != IDYES){
    return;
  }
  m_pUnitType->DeleteAppearance(m_pCurrentAppearance);

  UpdateListItems();
  UpdateSelectedAppearance();
}

void CUnitAppearancePage::OnAdd() 
{
  CEUnitAppearanceType *pAppType;

  // create new one
  pAppType = m_pUnitType->NewAppearance();
  // create its instance
  pAppType->CreateInstance();

  CUnitAppearanceDlg dlg;
  dlg.Create(pAppType);
  if(dlg.DoModal() != IDOK){
    // if canceled -> delete it
    m_pUnitType->DeleteAppearance(pAppType);
    return;
  }
  // elseway update our lists
  UpdateListItems();
  UpdateSelectedAppearance();
}
