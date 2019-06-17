// EditInvisibility.cpp : implementation file
//

#include "stdafx.h"
#include "..\MapEditor.h"
#include "EditInvisibility.h"

#include "..\DataObjects\EMap.h"
#include "Common\Map\Map.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEditInvisibility dialog


CEditInvisibility::CEditInvisibility(CWnd* pParent /*=NULL*/)
	: CDialog(CEditInvisibility::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEditInvisibility)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CEditInvisibility::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditInvisibility)
	DDX_Control(pDX, IDC_INVISIBILITY, m_wndList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEditInvisibility, CDialog)
	//{{AFX_MSG_MAP(CEditInvisibility)
	ON_NOTIFY(NM_CLICK, IDC_INVISIBILITY, OnClickInvisibility)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditInvisibility message handlers

void CEditInvisibility::OnOK() 
{
	
	CDialog::OnOK();
}

BOOL CEditInvisibility::OnInitDialog() 
{
	CDialog::OnInitDialog();

  int i;
  CEMap *pMap = AppGetActiveMap();
  if(pMap == NULL) return TRUE;

  SetWindowText(m_strTitle);

  m_ImageList.Create(IDB_VISCHECKIMAGELIST, 16, 1, RGB(192, 192, 192));
  m_wndList.SetImageList(&m_ImageList, LVSIL_SMALL);

  m_wndList.InsertColumn(0, "Jméno", LVCFMT_LEFT, 150, 0);

  CEInvisibility *pInvisibility;
  for(i = 0; i < INVISIBILITY_COUNT; i++){
    pInvisibility = pMap->GetInvisibility(i);
    m_wndList.InsertItem(i, pInvisibility->GetName(), 0);
    m_wndList.SetItemData(i, (DWORD)pInvisibility);
  }

  UpdateList();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEditInvisibility::UpdateList()
{
  int i;
  DWORD dwMask = 1;
  for(i = 0; i < INVISIBILITY_COUNT; i++){
    if((*m_pInvisibility) & dwMask){
      m_wndList.SetItem(i, 0, LVIF_IMAGE, NULL, 1, 0, 0, 0);
    }
    else{
      m_wndList.SetItem(i, 0, LVIF_IMAGE, NULL, 0, 0, 0, 0);
    }
    dwMask <<= 1;
  }
}

void CEditInvisibility::OnClickInvisibility(NMHDR* pNMHDR, LRESULT* pResult) 
{
  if(m_wndList.GetSelectedCount() == 0)
    return;
  int nSel = m_wndList.GetNextItem(-1, LVNI_SELECTED);

  CEInvisibility *pInvisibility;
  pInvisibility = (CEInvisibility *)m_wndList.GetItemData(nSel);
  DWORD dwMask = 1;
  dwMask <<= pInvisibility->GetIndex();
  if((*m_pInvisibility) & dwMask){
    *m_pInvisibility &= (~dwMask);
  }
  else{
    *m_pInvisibility |= dwMask;
  }

  UpdateList();
  
	*pResult = 0;
}
