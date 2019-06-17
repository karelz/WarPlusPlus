// DeleteUnitLibraryDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\MapEditor.h"
#include "DeleteUnitLibraryDlg.h"
#include "..\MapEditorDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDeleteUnitLibraryDlg dialog


CDeleteUnitLibraryDlg::CDeleteUnitLibraryDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDeleteUnitLibraryDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDeleteUnitLibraryDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDeleteUnitLibraryDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDeleteUnitLibraryDlg)
	DDX_Control(pDX, IDC_LIST, m_wndList);
	DDX_Control(pDX, IDC_DELETE, m_wndDelete);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDeleteUnitLibraryDlg, CDialog)
	//{{AFX_MSG_MAP(CDeleteUnitLibraryDlg)
	ON_LBN_SELCHANGE(IDC_LIST, OnSelChangeList)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	ON_LBN_DBLCLK(IDC_LIST, OnDblclkList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDeleteUnitLibraryDlg message handlers

BOOL CDeleteUnitLibraryDlg::Create(CEMap *pMap)
{
  ASSERT(pMap != NULL);

  m_pMap = pMap;
  return TRUE;
}

void CDeleteUnitLibraryDlg::UpdateListItems()
{
  int nSel = m_wndList.GetCurSel();
  m_wndList.ResetContent();

  POSITION pos;
  int i;
  CEUnitLibrary *pUnitLibrary;
  pos = m_pMap->GetFirstUnitLibraryPosition();
  i = 0;
  while(pos != NULL){
    pUnitLibrary = m_pMap->GetNextUnitLibrary(pos);

    m_wndList.InsertString(i, pUnitLibrary->GetName());
    m_wndList.SetItemData(i, (DWORD)pUnitLibrary);
    i++;
  }

  m_wndList.SetCurSel(nSel);
}

BOOL CDeleteUnitLibraryDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
  m_wndDelete.EnableWindow(FALSE);
  
  UpdateListItems();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDeleteUnitLibraryDlg::OnSelChangeList() 
{
  if(m_wndList.GetSelCount() == 0){
    m_wndDelete.EnableWindow(FALSE);
  }
  else{
    m_wndDelete.EnableWindow(TRUE);
  }
}

void CDeleteUnitLibraryDlg::OnDelete() 
{
	if(m_wndList.GetSelCount() == 0) return;

  int nItem;
  nItem = m_wndList.GetCurSel();

  CEUnitLibrary *pUnitLibrary;
  pUnitLibrary = (CEUnitLibrary *)m_wndList.GetItemData(nItem);

  CString str;
  str.Format("Opravdu odstranit knihovnu jednotek '%s' ?", pUnitLibrary->GetName());

  if(AfxMessageBox(str, MB_ICONEXCLAMATION | MB_YESNO) == IDYES){
    m_pMap->DeleteUnitLibrary(pUnitLibrary);

    AppGetActiveDocument()->SetModifiedFlag();

    UpdateListItems();
  }
}

void CDeleteUnitLibraryDlg::OnDblclkList() 
{
  OnDelete();
}
