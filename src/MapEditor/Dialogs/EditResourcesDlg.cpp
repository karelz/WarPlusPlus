// EditResourcesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\MapEditor.h"
#include "EditResourcesDlg.h"

#include "..\DataObjects\EResource.h"
#include "..\DataObjects\EMap.h"
#include "Common\Map\Map.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEditResourcesDlg dialog


CEditResourcesDlg::CEditResourcesDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CEditResourcesDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEditResourcesDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CEditResourcesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditResourcesDlg)
	DDX_Control(pDX, IDC_RESOURCES, m_wndList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEditResourcesDlg, CDialog)
	//{{AFX_MSG_MAP(CEditResourcesDlg)
	ON_NOTIFY(LVN_ENDLABELEDIT, IDC_RESOURCES, OnEndLabelEditResources)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_RESOURCES, OnItemChangedResources)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditResourcesDlg message handlers

BOOL CEditResourcesDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
  ASSERT(m_pResources != NULL);
	
  CEMap *pMap = AppGetActiveMap();
  if(pMap == NULL) EndDialog(IDCANCEL);

  SetWindowText(m_strTitle);
  m_wndList.InsertColumn(0, "Jméno", LVCFMT_LEFT, 150, 0);
  m_wndList.InsertColumn(0, "Množství", LVCFMT_LEFT, 80, 0);

  int i;
  CEResource *pResource;
  CString str;
  for(i = 0; i < RESOURCE_COUNT; i++){
    pResource = pMap->GetResource(i);
    str.Format("%d", m_pResources[i]);
    m_wndList.InsertItem(i, str, 0);
    m_wndList.SetItemText(i, 1, pResource->GetName());
    m_wndList.SetItemData(i, (DWORD)pResource);
  }

  int aColumns[2] = {1, 0};
  m_wndList.SetColumnOrderArray(2, aColumns);
  
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEditResourcesDlg::OnEndLabelEditResources(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;

  if(pDispInfo->item.pszText != NULL){
    CString strText = pDispInfo->item.pszText;

    int nValue = atol(strText);

    strText.Format("%d", nValue);
    m_wndList.SetItemText(pDispInfo->item.iItem, 0, strText);
  }
  
	*pResult = 0;
}

void CEditResourcesDlg::OnOK() 
{
  int i;
  for(i = 0; i < RESOURCE_COUNT; i++){
    m_pResources[i] = atol(m_wndList.GetItemText(i, 0));
  }
  
	CDialog::OnOK();
}

void CEditResourcesDlg::OnItemChangedResources(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	*pResult = 0;
}
