// InvisibilityDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\MapEditor.h"
#include "InvisibilityDlg.h"

#include "..\DataObjects\EMap.h"
#include "Common\Map\Map.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CInvisibilityDlg dialog


CInvisibilityDlg::CInvisibilityDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CInvisibilityDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CInvisibilityDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CInvisibilityDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CInvisibilityDlg)
	DDX_Control(pDX, IDC_INVISIBILITY, m_wndList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CInvisibilityDlg, CDialog)
	//{{AFX_MSG_MAP(CInvisibilityDlg)
	ON_NOTIFY(LVN_ENDLABELEDIT, IDC_INVISIBILITY, OnEndLabelEditInvisibility)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInvisibilityDlg message handlers

BOOL CInvisibilityDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
  CEMap *pMap = AppGetActiveMap();
  if(pMap == NULL) return TRUE;

  m_wndList.InsertColumn(0, "Jméno", LVCFMT_LEFT, 150, 0);
  m_wndList.InsertColumn(1, "Index", LVCFMT_LEFT, 40, 1);

  int aColumns[2] = {1, 0};
  m_wndList.SetColumnOrderArray(2, aColumns);

  int i;
  CEInvisibility *pInvisibility;
  CString str;
  for(i = 0; i < INVISIBILITY_COUNT; i++){
    pInvisibility = pMap->GetInvisibility(i);
    str.Format("%d", i + 1);
    m_wndList.InsertItem(i, pInvisibility->GetName(), 0);
    m_wndList.SetItemText(i, 1, str);
    m_wndList.SetItemData(i, (DWORD)pInvisibility);
  }
  
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CInvisibilityDlg::OnEndLabelEditInvisibility(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
  
	*pResult = 1;
}

void CInvisibilityDlg::OnOK() 
{
  int i;
  CEInvisibility *pInvisibility;
  for(i = 0; i < INVISIBILITY_COUNT; i++){
    pInvisibility = (CEInvisibility *)m_wndList.GetItemData(i);
    pInvisibility->SetName(m_wndList.GetItemText(i, 0));
  }
  
	CDialog::OnOK();
}
