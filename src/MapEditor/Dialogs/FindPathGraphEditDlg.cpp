// FindPathGraphEditDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\MapEditor.h"
#include "FindPathGraphEditDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "..\DataObjects\EMap.h"

/////////////////////////////////////////////////////////////////////////////
// CFindPathGraphEditDlg dialog


CFindPathGraphEditDlg::CFindPathGraphEditDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFindPathGraphEditDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFindPathGraphEditDlg)
	m_strName = _T("");
	//}}AFX_DATA_INIT
}


void CFindPathGraphEditDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFindPathGraphEditDlg)
	DDX_Control(pDX, IDC_LIST, m_wndList);
	DDX_Text(pDX, IDC_NAME, m_strName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFindPathGraphEditDlg, CDialog)
	//{{AFX_MSG_MAP(CFindPathGraphEditDlg)
	ON_NOTIFY(NM_CLICK, IDC_LIST, OnClickList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFindPathGraphEditDlg message handlers

BOOL CFindPathGraphEditDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

  m_ImageList.Create(IDB_ULTCHECKIMAGELIST, 16, 1, RGB(192, 192, 192));
  m_wndList.SetImageList(&m_ImageList, LVSIL_SMALL);
	
  m_strName = m_pGraph->GetName();
  m_wndList.InsertColumn(0, "Jméno", LVCFMT_LEFT, 150);
  AppGetActiveMap()->FillLandTypesListCtrl(&m_wndList);
  
  // we have to remove the transparent type of land
  int i;
  int nCount = m_wndList.GetItemCount();
  CLandType *pLandType;
  for(i = 0; i < nCount; i++){
    pLandType = (CLandType *)m_wndList.GetItemData(i);
    if(pLandType->GetID() == 0){ // transparent one
      m_wndList.DeleteItem(i);
      break;
    }
  }

  UpdateList();

  UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CFindPathGraphEditDlg::OnOK() 
{
  UpdateData(TRUE);

  m_pGraph->SetName(m_strName);
  CDialog::OnOK();
}

void CFindPathGraphEditDlg::OnClickList(NMHDR* pNMHDR, LRESULT* pResult) 
{
  if(m_wndList.GetSelectedCount() == 0)
    return;
  int nSel = m_wndList.GetNextItem(-1, LVNI_SELECTED);

  CLandType *pLandType;
  pLandType = (CLandType *)m_wndList.GetItemData(nSel);

  if(m_pGraph->GetLandType(pLandType->GetID())){
    m_pGraph->SetLandType(pLandType->GetID(), FALSE);
  }
  else{
    m_pGraph->SetLandType(pLandType->GetID(), TRUE);
  }
  UpdateList();
  
	*pResult = 0;
}

void CFindPathGraphEditDlg::UpdateList()
{
  int i;
  int nCount = m_wndList.GetItemCount();
  CLandType *pLandType;

  for(i = 0; i < nCount; i++){
    pLandType = (CLandType *)m_wndList.GetItemData(i);
    if(m_pGraph->GetLandType(pLandType->GetID())){
      m_wndList.SetItem(i, 0, LVIF_IMAGE, NULL, 1, 0, 0, 0);
    }
    else{
      m_wndList.SetItem(i, 0, LVIF_IMAGE, NULL, 0, 0, 0, 0);
    }
  }
}