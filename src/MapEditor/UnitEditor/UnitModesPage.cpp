// UnitModesPage.cpp : implementation file
//

#include "stdafx.h"
#include "..\MapEditor.h"
#include "UnitModesPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CUnitModesPage property page

IMPLEMENT_DYNCREATE(CUnitModesPage, CPropertyPage)

CUnitModesPage::CUnitModesPage() : CPropertyPage(CUnitModesPage::IDD)
{
	//{{AFX_DATA_INIT(CUnitModesPage)
	m_strName = _T("");
	m_dwMoveWidth = 0;
	//}}AFX_DATA_INIT

  m_dwCurrentMode = 0x100;
}

CUnitModesPage::~CUnitModesPage()
{
}

void CUnitModesPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CUnitModesPage)
	DDX_Control(pDX, IDC_APPEARANCE, m_wndAppearance);
	DDX_Control(pDX, IDC_MODE, m_wndMode);
	DDX_Text(pDX, IDC_NAME, m_strName);
	DDX_Text(pDX, IDC_MOVEWIDTH, m_dwMoveWidth);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CUnitModesPage, CPropertyPage)
	//{{AFX_MSG_MAP(CUnitModesPage)
	ON_CBN_SELENDOK(IDC_MODE, OnSelEndOkMode)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUnitModesPage message handlers

void CUnitModesPage::Create(CEUnitType *pUnitType)
{
  m_pUnitType = pUnitType;
}

BOOL CUnitModesPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

  m_wndInvisibility.Create(this, IDC_INVISIBILITY, &m_dwInvisibility);
  m_wndInvisibility.SetTitle("Neviditelnost");
  m_wndDetection.Create(this, IDC_DETECTION, &m_dwDetection);
  m_wndDetection.SetTitle("Detekce");

  m_pUnitType->FillModesCombo(&m_wndMode);
  m_wndMode.SetCurSel(0);

  m_pUnitType->FillAppearancesCombo(&m_wndAppearance);
  int nItem = m_wndAppearance.InsertString(0, "(žádný)");
  m_wndAppearance.SetItemData(nItem, 0);

  UpdateSelectedMode();
  UpdateModePage();
	
  UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CUnitModesPage::OnOK() 
{
  UpdateData(TRUE);
  WriteNewData();

	CPropertyPage::OnOK();
}

void CUnitModesPage::UpdateSelectedMode()
{
  int nSel = m_wndMode.GetCurSel();
  if(nSel == -1){
    m_dwCurrentMode = 0x100;
    return;
  }
  m_dwCurrentMode = m_wndMode.GetItemData(nSel);
}

void CUnitModesPage::UpdateModePage()
{
  if(m_dwCurrentMode >= 8) return;

  int i, nCount;
  DWORD dwAppearanceID = m_pUnitType->GetDefaultAppearance(m_dwCurrentMode);
  CEUnitAppearanceType *pAppType;

  // have to find the appearance which is selected in the unit type
  nCount = m_wndAppearance.GetCount();
  m_wndAppearance.SetCurSel(-1);
  for(i = 0; i < nCount; i++){
    pAppType = (CEUnitAppearanceType *)m_wndAppearance.GetItemData(i);
    if(pAppType == NULL){
      if(dwAppearanceID == 0){
        m_wndAppearance.SetCurSel(i);
        break;
      }
    }
    else{
      if(pAppType->GetID() == dwAppearanceID){
        m_wndAppearance.SetCurSel(i);
        break;
      }
    }
  }

  m_strName = m_pUnitType->GetModeName(m_dwCurrentMode);
  m_dwMoveWidth = m_pUnitType->GetMoveWidth(m_dwCurrentMode);

  m_dwInvisibility = m_pUnitType->GetInvisibilityFlags(m_dwCurrentMode);
  m_wndInvisibility.SetInvisibility(&m_dwInvisibility);
  m_dwDetection = m_pUnitType->GetDetectionFlags(m_dwCurrentMode);
  m_wndDetection.SetInvisibility(&m_dwDetection);

  UpdateData(FALSE);
}

void CUnitModesPage::WriteNewData()
{
  if(m_dwCurrentMode >= 8) return;

  UpdateData(TRUE);

  m_pUnitType->SetModeName(m_dwCurrentMode, m_strName);
  m_pUnitType->SetMoveWidth(m_dwCurrentMode, m_dwMoveWidth);
  int nSel;
  CEUnitAppearanceType *pAppType;
  nSel = m_wndAppearance.GetCurSel();
  if(nSel == -1){
    m_pUnitType->SetDefaultAppearance(m_dwCurrentMode, 0);
  }
  else{
    pAppType = (CEUnitAppearanceType *)m_wndAppearance.GetItemData(nSel);
    if(pAppType == NULL){
      m_pUnitType->SetDefaultAppearance(m_dwCurrentMode, 0);
    }
    else{
      m_pUnitType->SetDefaultAppearance(m_dwCurrentMode, pAppType->GetID());
    }
  }
  m_pUnitType->SetInvisibilityFlags(m_dwCurrentMode, m_dwInvisibility);
  m_pUnitType->SetDetectionFlags(m_dwCurrentMode, m_dwDetection);

  int i = m_wndMode.GetCurSel(), nCount;
  DWORD dwMode = m_wndMode.GetItemData(i);
  m_pUnitType->FillModesCombo(&m_wndMode);
  nCount = m_wndMode.GetCount();
  for(i = 0; i < nCount; i++){
    if(m_wndMode.GetItemData(i) == dwMode){
      m_wndMode.SetCurSel(i);
      break;
    }
  }
}

void CUnitModesPage::OnSelEndOkMode() 
{
  WriteNewData();

  UpdateSelectedMode();
  UpdateModePage();
}
