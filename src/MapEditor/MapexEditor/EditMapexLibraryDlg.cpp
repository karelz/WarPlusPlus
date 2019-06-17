// EditMapexLibraryDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\MapEditor.h"
#include "EditMapexLibraryDlg.h"

#include "MapexLibraryEditDlg.h"
#include "..\MapEditorDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEditMapexLibraryDlg dialog


CEditMapexLibraryDlg::CEditMapexLibraryDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CEditMapexLibraryDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEditMapexLibraryDlg)
	//}}AFX_DATA_INIT
}


void CEditMapexLibraryDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditMapexLibraryDlg)
	DDX_Control(pDX, IDC_EDIT, m_Edit);
	DDX_Control(pDX, IDC_LIST, m_List);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEditMapexLibraryDlg, CDialog)
	//{{AFX_MSG_MAP(CEditMapexLibraryDlg)
	ON_LBN_SELCHANGE(IDC_LIST, OnSelChangeList)
	ON_BN_CLICKED(IDC_EDIT, OnEdit)
	ON_LBN_DBLCLK(IDC_LIST, OnDblClkList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditMapexLibraryDlg message handlers

void CEditMapexLibraryDlg::Create(CEMap *pMap)
{
  ASSERT_VALID(pMap);

  m_pMap = pMap;
}

BOOL CEditMapexLibraryDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

  m_Edit.EnableWindow(FALSE);

  UpdateListItems();
  
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEditMapexLibraryDlg::OnSelChangeList() 
{
  if(m_List.GetSelCount() == 0){
    m_Edit.EnableWindow(FALSE);
  }
  else{
    m_Edit.EnableWindow(TRUE);
  }
}

void CEditMapexLibraryDlg::OnEdit() 
{
  if(m_List.GetSelCount() == 0) return;
  int nItem = m_List.GetCurSel();

  CEMapexLibrary *pMapexLibrary;
  pMapexLibrary = (CEMapexLibrary *)m_List.GetItemData(nItem);

  CMapexLibraryEditDlg dlg(this);
  dlg.Create(pMapexLibrary);

  if(dlg.DoModal() == IDOK){
    UpdateListItems();
    AppGetActiveDocument()->SetModifiedFlag();
  }
}

void CEditMapexLibraryDlg::OnDblClkList() 
{
  OnEdit();	
}

void CEditMapexLibraryDlg::UpdateListItems()
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
