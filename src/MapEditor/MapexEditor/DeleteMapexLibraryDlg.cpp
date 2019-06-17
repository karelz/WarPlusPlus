// DeleteMapexLibraryDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\MapEditor.h"
#include "DeleteMapexLibraryDlg.h"

#include "..\MapEditorDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDeleteMapexLibraryDlg dialog


CDeleteMapexLibraryDlg::CDeleteMapexLibraryDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDeleteMapexLibraryDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDeleteMapexLibraryDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDeleteMapexLibraryDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDeleteMapexLibraryDlg)
	DDX_Control(pDX, IDC_LIST, m_List);
	DDX_Control(pDX, IDC_DELETE, m_Delete);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDeleteMapexLibraryDlg, CDialog)
	//{{AFX_MSG_MAP(CDeleteMapexLibraryDlg)
	ON_LBN_SELCHANGE(IDC_LIST, OnSelChangeList)
	ON_LBN_DBLCLK(IDC_LIST, OnDblClkList)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDeleteMapexLibraryDlg message handlers

void CDeleteMapexLibraryDlg::Create(CEMap *pMap)
{
  ASSERT_VALID(pMap);

  m_pMap = pMap;
}

BOOL CDeleteMapexLibraryDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

  m_Delete.EnableWindow(FALSE);
  
  UpdateListItems();
  
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDeleteMapexLibraryDlg::UpdateListItems()
{
  int nSel = m_List.GetCurSel();
  m_List.ResetContent();

  POSITION pos;
  int i;
  CEMapexLibrary *pMapexLibrary;
  pos = m_pMap->GetFirstMapexLibraryPosition();
  i = 0;
  while(pos != NULL){
    pMapexLibrary = m_pMap->GetNextMapexLibrary(pos);

    m_List.InsertString(i, pMapexLibrary->GetName());
    m_List.SetItemData(i, (DWORD)pMapexLibrary);
    i++;
  }

  m_List.SetCurSel(nSel);
}

void CDeleteMapexLibraryDlg::OnSelChangeList() 
{
  if(m_List.GetSelCount() == 0){
    m_Delete.EnableWindow(FALSE);
  }
  else{
    m_Delete.EnableWindow(TRUE);
  }
}

void CDeleteMapexLibraryDlg::OnDblClkList() 
{
  OnDelete();
}

void CDeleteMapexLibraryDlg::OnDelete() 
{
	if(m_List.GetSelCount() == 0) return;

  int nItem;
  nItem = m_List.GetCurSel();

  CEMapexLibrary *pMapexLibrary;
  pMapexLibrary = (CEMapexLibrary *)m_List.GetItemData(nItem);

  CString str;
  str.Format("Opravdu odstranit knihovnu mapexù '%s' ?", pMapexLibrary->GetName());

  if(AfxMessageBox(str, MB_ICONEXCLAMATION | MB_YESNO) == IDYES){
    m_pMap->DeleteMapexLibrary(pMapexLibrary);

    AppGetActiveDocument()->SetModifiedFlag();

    UpdateListItems();
  }
}
