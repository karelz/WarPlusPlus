// LandTypeSelectDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\MapEditor.h"
#include "LandTypeSelectDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLandTypeSelectDlg dialog


CLandTypeSelectDlg::CLandTypeSelectDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLandTypeSelectDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLandTypeSelectDlg)
	//}}AFX_DATA_INIT
  m_pSelected = NULL;
}


void CLandTypeSelectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLandTypeSelectDlg)
	DDX_Control(pDX, IDC_LANDTYPELIST, m_List);
	DDX_Control(pDX, IDOK, m_OK);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLandTypeSelectDlg, CDialog)
	//{{AFX_MSG_MAP(CLandTypeSelectDlg)
	ON_NOTIFY(NM_DBLCLK, IDC_LANDTYPELIST, OnDblClkLandTypeList)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LANDTYPELIST, OnItemChangedLandTypeList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLandTypeSelectDlg message handlers

void CLandTypeSelectDlg::Create(CEMap *pMap, CLandType *pSelected)
{
  ASSERT_VALID(pMap);

  m_pMap = pMap;
  m_pSelected = pSelected;
}

BOOL CLandTypeSelectDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
  CRect rcList; m_List.GetClientRect(&rcList);
  m_List.SetExtendedStyle(LVS_EX_FULLROWSELECT);
  m_List.InsertColumn(0, "", LVCFMT_LEFT, rcList.Width() - 16);

  m_pMap->FillLandTypesListCtrl(&m_List);
  int nCount, i;
  nCount = m_List.GetItemCount();
  CLandType *pLandType;
  for(i = 0; i < nCount; i++){
    pLandType = (CLandType *)m_List.GetItemData(i);
    if(pLandType == m_pSelected)
      m_List.SetItemState(i, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
  }

  UpdateSelected();
  
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CLandTypeSelectDlg::UpdateSelected()
{
  POSITION selpos = m_List.GetFirstSelectedItemPosition();
  if(selpos == NULL){
    m_OK.EnableWindow(FALSE);
    m_pSelected = NULL;
    return;
  }
  int nSel = m_List.GetNextSelectedItem(selpos);
  m_pSelected = (CLandType *)m_List.GetItemData(nSel);
  m_OK.EnableWindow(TRUE);
}

CLandType * CLandTypeSelectDlg::GetSelected()
{
  return m_pSelected;
}

void CLandTypeSelectDlg::OnDblClkLandTypeList(NMHDR* pNMHDR, LRESULT* pResult) 
{
  UpdateSelected();
  if(m_pSelected == NULL) return;
  EndDialog(IDOK);
	
	*pResult = 0;
}

void CLandTypeSelectDlg::OnItemChangedLandTypeList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

  if(pNMListView->uChanged & LVIF_STATE)
    UpdateSelected();

	*pResult = 0;
}
