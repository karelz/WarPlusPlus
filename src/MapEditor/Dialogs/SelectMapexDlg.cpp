// SelectMapexDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\MapEditor.h"
#include "SelectMapexDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSelectMapexDlg dialog


int CSelectMapexDlg::m_nLastSelectedLibrary = 0;

CSelectMapexDlg::CSelectMapexDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSelectMapexDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSelectMapexDlg)
	m_strMapexSize = _T("");
	//}}AFX_DATA_INIT
  m_pCurrentMapex = NULL;
  m_pCurrentMapexLibrary = NULL;
}


void CSelectMapexDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSelectMapexDlg)
	DDX_Control(pDX, IDC_MAPEXLIST, m_MapexList);
	DDX_Control(pDX, IDOK, m_OK);
	DDX_Control(pDX, IDC_LIBRARYCOMBO, m_LibraryCombo);
	DDX_Text(pDX, IDC_MAPEXSIZE, m_strMapexSize);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSelectMapexDlg, CDialog)
	//{{AFX_MSG_MAP(CSelectMapexDlg)
	ON_CBN_SELENDOK(IDC_LIBRARYCOMBO, OnSelendOKLibraryCombo)
	ON_NOTIFY(NM_DBLCLK, IDC_MAPEXLIST, OnDblClkMapexList)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_MAPEXLIST, OnItemChangedMapexList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSelectMapexDlg message handlers

void CSelectMapexDlg::UpdateListItems()
{
  if(m_pCurrentMapexLibrary == NULL){
    m_MapexList.DeleteAllItems();
    return;
  }
  
  m_pCurrentMapexLibrary->FillMapexesListCtrl(&m_MapexList);
}

void CSelectMapexDlg::UpdateSelectedLibrary()
{
  int nSel = m_LibraryCombo.GetCurSel();
  m_nLastSelectedLibrary = nSel;
  if(nSel == -1){
    m_pCurrentMapexLibrary = NULL;
    return;
  }
  m_pCurrentMapexLibrary = (CEMapexLibrary *)m_LibraryCombo.GetItemData(nSel);
}

void CSelectMapexDlg::UpdateSelectedMapex()
{
  if(m_MapexList.GetSelectedCount() == 0){
    m_pCurrentMapex = NULL;
    m_OK.EnableWindow(FALSE);
    m_strMapexSize.Empty();
    UpdateData(FALSE);
    m_wndMapexPreview.SetMapex(NULL);
    return;
  }
  int nSel = m_MapexList.GetNextItem(-1, LVNI_SELECTED);
  m_pCurrentMapex = (CEMapex *)m_MapexList.GetItemData(nSel);
  m_OK.EnableWindow(TRUE);
  m_strMapexSize.Format("%d x %d", m_pCurrentMapex->GetSize().cx, m_pCurrentMapex->GetSize().cy);
  UpdateData(FALSE);
  m_wndMapexPreview.SetMapex(m_pCurrentMapex);
}

BOOL CSelectMapexDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
  CWnd *pPreview = GetDlgItem(IDC_MAPEXPREVIEW);
  DWORD dwStyle = pPreview->GetStyle();
  CRect rcPreview;
  pPreview->GetWindowRect(&rcPreview);
  ScreenToClient(&rcPreview);
  pPreview->DestroyWindow();
  m_wndMapexPreview.Create(rcPreview, dwStyle, this, IDC_MAPEXPREVIEW);
  
  CRect rcList; m_MapexList.GetClientRect(&rcList);
  m_MapexList.SetExtendedStyle(LVS_EX_FULLROWSELECT);
  m_MapexList.InsertColumn(0, "", LVCFMT_LEFT, rcList.Width() - 16);
  m_pMap->FillMapexLibrariesComboBox(&m_LibraryCombo);
  m_LibraryCombo.SetCurSel(m_nLastSelectedLibrary);
  UpdateSelectedLibrary();
  UpdateListItems();
  UpdateSelectedMapex();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSelectMapexDlg::OnSelendOKLibraryCombo() 
{
  UpdateSelectedLibrary();
  UpdateListItems();
  UpdateSelectedMapex();
}

void CSelectMapexDlg::Create(CEMap *pMap)
{
  m_pMap = pMap;
}

CEMapex * CSelectMapexDlg::GetSelected()
{
  return m_pCurrentMapex;
}

void CSelectMapexDlg::OnDblClkMapexList(NMHDR* pNMHDR, LRESULT* pResult) 
{
  if(m_pCurrentMapex == NULL) return;
  EndDialog(IDOK);	
	
	*pResult = 0;
}

void CSelectMapexDlg::OnItemChangedMapexList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	
  if(pNMListView->uChanged & LVIF_STATE)
    UpdateSelectedMapex();

	*pResult = 0;
}
