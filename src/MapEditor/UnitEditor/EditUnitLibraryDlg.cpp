// EditUnitLibraryDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\MapEditor.h"
#include "EditUnitLibraryDlg.h"

#include "..\MapEditorDoc.h"
#include "UnitLibraryEditorDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEditUnitLibraryDlg dialog


CEditUnitLibraryDlg::CEditUnitLibraryDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CEditUnitLibraryDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEditUnitLibraryDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CEditUnitLibraryDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditUnitLibraryDlg)
	DDX_Control(pDX, IDC_LIST, m_wndList);
	DDX_Control(pDX, IDC_EDIT, m_wndEdit);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEditUnitLibraryDlg, CDialog)
	//{{AFX_MSG_MAP(CEditUnitLibraryDlg)
	ON_LBN_SELCHANGE(IDC_LIST, OnSelChangeList)
	ON_BN_CLICKED(IDC_EDIT, OnEdit)
	ON_LBN_DBLCLK(IDC_LIST, OnDblclkList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditUnitLibraryDlg message handlers

BOOL CEditUnitLibraryDlg::Create(CEMap *pMap)
{
  ASSERT(pMap != NULL);

  m_pMap = pMap;
  return TRUE;
}

void CEditUnitLibraryDlg::UpdateListItems()
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

BOOL CEditUnitLibraryDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
  m_wndEdit.EnableWindow(FALSE);

  UpdateListItems();
  
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEditUnitLibraryDlg::OnSelChangeList() 
{
  if(m_wndList.GetSelCount() == 0){
    m_wndEdit.EnableWindow(FALSE);
  }
  else{
    m_wndEdit.EnableWindow(TRUE);
  }
}

void CEditUnitLibraryDlg::OnEdit() 
{
  if(m_wndList.GetSelCount() == 0) return;
  int nItem = m_wndList.GetCurSel();

  CEUnitLibrary *pUnitLibrary;
  pUnitLibrary = (CEUnitLibrary *)m_wndList.GetItemData(nItem);

  CUnitLibraryEditorDlg dlg(this);
  dlg.Create(pUnitLibrary);

  if(dlg.DoModal() == IDOK){
    UpdateListItems();
    AppGetActiveDocument()->SetModifiedFlag();
  }
}

void CEditUnitLibraryDlg::OnDblclkList() 
{
  OnEdit();	
}
