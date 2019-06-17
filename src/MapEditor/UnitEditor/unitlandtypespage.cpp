// unitlandtypespage.cpp : implementation file
//

#include "stdafx.h"
#include "..\MapEditor.h"
#include "unitlandtypespage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CUnitLandTypesPage property page

IMPLEMENT_DYNCREATE(CUnitLandTypesPage, CPropertyPage)

CUnitLandTypesPage::CUnitLandTypesPage() : CPropertyPage(CUnitLandTypesPage::IDD)
{
	//{{AFX_DATA_INIT(CUnitLandTypesPage)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

  m_dwCurrentMode = 0x100;
}

CUnitLandTypesPage::~CUnitLandTypesPage()
{
}

void CUnitLandTypesPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CUnitLandTypesPage)
	DDX_Control(pDX, IDC_MODES, m_wndModes);
	DDX_Control(pDX, IDC_LIST, m_wndList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CUnitLandTypesPage, CPropertyPage)
	//{{AFX_MSG_MAP(CUnitLandTypesPage)
	ON_CBN_SELENDOK(IDC_MODES, OnSelEndOkModes)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST, OnItemchangedList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUnitLandTypesPage message handlers

void CUnitLandTypesPage::Create(CEUnitType *pUnitType, CEMap *pMap)
{
  ASSERT_VALID(pUnitType);
  ASSERT_VALID(pMap);

  m_pUnitType = pUnitType;
  m_pMap = pMap;
  m_pSelectedGraph = NULL;
}

BOOL CUnitLandTypesPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
  m_pUnitType->FillModesCombo(&m_wndModes);
  m_wndModes.SetCurSel(0);

  m_ImageList.Create(IDB_ULTCHECKIMAGELIST, 16, 1, RGB(192, 192, 192));
//  m_wndList.SetImageList(&m_ImageList, LVSIL_SMALL);

  CRect rcList;
  m_wndList.GetClientRect(&rcList);
  m_wndList.InsertColumn(0, "", LVCFMT_LEFT, rcList.Width() - 16);
  m_pMap->FillFindPathGraphListCtrl(&m_wndList);

  UpdateSelectedMode();

  // Select our graph
  int i;
  for ( i = 0; i < m_wndList.GetItemCount (); i++)
  {
      CEFindPathGraph *pGraph = (CEFindPathGraph *)m_wndList.GetItemData(i);
      if ( pGraph->GetID () == m_pUnitType->GetPathGraphID ( m_dwCurrentMode ) )
      {
        m_wndList.SetItemState( i, LVIS_SELECTED , LVIS_SELECTED );
      }
      else{
          m_wndList.SetItemState ( i, 0, LVIS_SELECTED );
      }
  }

  UpdateSelectedGraph();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CUnitLandTypesPage::UpdateSelectedGraph()
{
    int nSel;
    if ( m_wndList.GetSelectedCount() == 0 ){
        m_pSelectedGraph = NULL;
        m_pUnitType->SetPathGraphID ( m_dwCurrentMode, 0 );
        return;
    }
    nSel = m_wndList.GetNextItem ( -1, LVNI_SELECTED );
    m_pSelectedGraph = (CEFindPathGraph *)m_wndList.GetItemData ( nSel );
    ASSERT(m_pSelectedGraph != NULL);
    m_pUnitType->SetPathGraphID ( m_dwCurrentMode, m_pSelectedGraph->GetID () );
}

void CUnitLandTypesPage::UpdateSelectedMode()
{
  int nSel;
  nSel = m_wndModes.GetCurSel();
  if(nSel == -1){
    m_dwCurrentMode = 0x100;
    return;
  }

  m_dwCurrentMode = m_wndModes.GetItemData(nSel);
}

void CUnitLandTypesPage::OnSelEndOkModes() 
{
  UpdateSelectedMode();
  // Select our graph
  int i;
  for ( i = 0; i < m_wndList.GetItemCount (); i++)
  {
      CEFindPathGraph *pGraph = (CEFindPathGraph *)m_wndList.GetItemData(i);
      if ( pGraph->GetID () == m_pUnitType->GetPathGraphID ( m_dwCurrentMode ) )
      {
        m_wndList.SetItemState( i, LVIS_SELECTED , LVIS_SELECTED );
      }
  }
}

void CUnitLandTypesPage::OnItemchangedList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

    if(pNMListView->uChanged & LVIF_STATE){
        UpdateSelectedGraph ();
    }
	
	*pResult = 0;
}
