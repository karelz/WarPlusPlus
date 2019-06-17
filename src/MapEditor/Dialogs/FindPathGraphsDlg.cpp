// FindPathGraphsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\MapEditor.h"
#include "FindPathGraphsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "..\DataObjects\EMap.h"
#include "FindPathGraphEditDlg.h"

/////////////////////////////////////////////////////////////////////////////
// CFindPathGraphsDlg dialog


CFindPathGraphsDlg::CFindPathGraphsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFindPathGraphsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFindPathGraphsDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CFindPathGraphsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFindPathGraphsDlg)
	DDX_Control(pDX, IDC_LIST, m_wndList);
	DDX_Control(pDX, IDC_EDIT, m_wndEdit);
	DDX_Control(pDX, IDC_DELETE, m_wndDelete);
	DDX_Control(pDX, IDC_ADD, m_wndAdd);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFindPathGraphsDlg, CDialog)
	//{{AFX_MSG_MAP(CFindPathGraphsDlg)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST, OnItemChangedList)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	ON_BN_CLICKED(IDC_EDIT, OnEdit)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST, OnDblClkList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFindPathGraphsDlg message handlers

BOOL CFindPathGraphsDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

  m_wndList.InsertColumn(0, "Jméno", LVCFMT_LEFT, 150);

  UpdateList();
  UpdateSelectedItem();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CFindPathGraphsDlg::UpdateSelectedItem()
{
  int nSel;

  if(m_wndList.GetSelectedCount() == 0){
    m_wndDelete.EnableWindow(FALSE);
    m_wndEdit.EnableWindow(FALSE);
    return;
  }

  nSel = m_wndList.GetNextItem(-1, LVNI_SELECTED);
  CEFindPathGraph *pGraph = (CEFindPathGraph *)m_wndList.GetItemData(nSel);

  m_pSelected = pGraph;
  if(m_pSelected == NULL){
    m_wndDelete.EnableWindow(FALSE);
    m_wndEdit.EnableWindow(FALSE);
    return;
  }
  else{
    m_wndDelete.EnableWindow(TRUE);
    m_wndEdit.EnableWindow(TRUE);
  }
}

void CFindPathGraphsDlg::UpdateList()
{
  m_wndList.DeleteAllItems();
  m_pMap->FillFindPathGraphListCtrl(&m_wndList);
}

void CFindPathGraphsDlg::OnItemChangedList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	
  if(pNMListView->uChanged & LVIF_STATE){
    UpdateSelectedItem();
  }

  *pResult = 0;
}


void CFindPathGraphsDlg::OnAdd() 
{
  CFindPathGraphEditDlg dlg;

  CEFindPathGraph *pNew = new CEFindPathGraph();
  pNew->Create(m_pMap->GetNewUnitTypeID (), "Nový graf");

  dlg.Create(pNew);

  if(dlg.DoModal() != IDOK){
    pNew->Delete();
    delete pNew;
    return;
  }

  m_pMap->AddFindPathGraph(pNew);

  UpdateList();
  UpdateSelectedItem();
}

void CFindPathGraphsDlg::OnDelete() 
{
	if(m_pSelected == NULL) return;

  CString str;
  str.Format("Urèitì chcete smazat graf pro hledání cesty %s ?", m_pSelected->GetName());
  if(AfxMessageBox(str, MB_YESNO) != IDYES){
    return;
  }

  m_pMap->DeleteFindPathGraph(m_pSelected);
  UpdateList();
  UpdateSelectedItem();
}

void CFindPathGraphsDlg::OnEdit() 
{
  if(m_pSelected == NULL) return;

	CFindPathGraphEditDlg dlg;

  dlg.Create(m_pSelected);

  if(dlg.DoModal() != IDOK){
    return;
  }

  UpdateList();
  UpdateSelectedItem();
}

void CFindPathGraphsDlg::OnDblClkList(NMHDR* pNMHDR, LRESULT* pResult) 
{
  UpdateSelectedItem();

  OnEdit();
  
	*pResult = 0;
}
