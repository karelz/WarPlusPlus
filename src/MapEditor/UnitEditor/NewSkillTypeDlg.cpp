// NewSkillTypeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\MapEditor.h"
#include "NewSkillTypeDlg.h"

#include "UnitLibrariesDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNewSkillTypeDlg dialog


CNewSkillTypeDlg::CNewSkillTypeDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CNewSkillTypeDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNewSkillTypeDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CNewSkillTypeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNewSkillTypeDlg)
	DDX_Control(pDX, IDOK, m_wndOK);
	DDX_Control(pDX, IDC_LIST, m_wndList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNewSkillTypeDlg, CDialog)
	//{{AFX_MSG_MAP(CNewSkillTypeDlg)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST, OnItemChangedList)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST, OnDblClkList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNewSkillTypeDlg message handlers

void CNewSkillTypeDlg::UpdateSelectedItem()
{
  if(m_wndList.GetSelectedCount() == 0){
    m_wndOK.EnableWindow(FALSE);
    m_pCurrentDlg = NULL;
    return;
  }
  int nSel;
  nSel = m_wndList.GetNextItem(-1, LVNI_SELECTED);
  m_pCurrentDlg = (CUnitSkillTypeDlg *)m_wndList.GetItemData(nSel);
  m_wndOK.EnableWindow(TRUE);
}

void CNewSkillTypeDlg::OnItemChangedList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

  if(pNMListView->uChanged & LVIF_STATE)
    UpdateSelectedItem();  

	*pResult = 0;
}

void CNewSkillTypeDlg::OnDblClkList(NMHDR* pNMHDR, LRESULT* pResult) 
{
  if(m_pCurrentDlg == NULL) return;

  EndDialog(IDOK);
  
	*pResult = 0;
}

BOOL CNewSkillTypeDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
  CRect rcList;
  m_wndList.GetClientRect(&rcList);
  m_wndList.InsertColumn(0, "", LVCFMT_LEFT, rcList.Width() - 16);

  g_pUnitSkillTypeCollection->FillListCtrl(&m_wndList);

  UpdateSelectedItem();
  
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
