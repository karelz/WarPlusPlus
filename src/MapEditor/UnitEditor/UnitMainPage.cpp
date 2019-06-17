// UnitMainPage.cpp : implementation file
//

#include "stdafx.h"
#include "..\MapEditor.h"
#include "UnitMainPage.h"

#include "Common\Map\Map.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CUnitMainPage property page

IMPLEMENT_DYNCREATE(CUnitMainPage, CPropertyPage)

CUnitMainPage::CUnitMainPage() : CPropertyPage(CUnitMainPage::IDD)
{
	//{{AFX_DATA_INIT(CUnitMainPage)
	m_strName = _T("");
	m_dwLifeMax = 10;
	m_dwViewRadius = 30;
	m_bSelectable = FALSE;
	m_strZPos = _T("");
	//}}AFX_DATA_INIT
  m_dwLifeBarFlags = UnitTypeFlags_LifeBar_ShowAlways;
}

CUnitMainPage::~CUnitMainPage()
{
}

void CUnitMainPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);

  m_strZPos.Format("%d", m_dwZPos);
	//{{AFX_DATA_MAP(CUnitMainPage)
	DDX_Control(pDX, IDC_ZPOS, m_wndZPos);
	DDX_Text(pDX, IDC_NAME, m_strName);
	DDX_Text(pDX, IDC_LIFEMAX, m_dwLifeMax);
	DDX_Text(pDX, IDC_VIEWRADIUS, m_dwViewRadius);
	DDX_Check(pDX, IDC_SELECTABLE, m_bSelectable);
	DDX_CBString(pDX, IDC_ZPOS, m_strZPos);
	//}}AFX_DATA_MAP
  if(atol(m_strZPos) > 0){
    m_dwZPos = atol(m_strZPos);
  }

  if ( pDX->m_bSaveAndValidate )
  {
    CButton * pButton;
    pButton = (CButton *)GetDlgItem ( IDC_LIFEBAR_HIDE );
    if ( pButton->GetCheck () == 1 )
    {
      m_dwLifeBarFlags = (m_dwLifeBarFlags & (~UnitTypeFlags_LifeBar_Mask)) | UnitTypeFlags_LifeBar_ShowNever;
    }
    pButton = (CButton *)GetDlgItem ( IDC_LIFEBAR_SELECTED );
    if ( pButton->GetCheck () == 1 )
    {
      m_dwLifeBarFlags = (m_dwLifeBarFlags & (~UnitTypeFlags_LifeBar_Mask)) | UnitTypeFlags_LifeBar_ShowWhenSelected;
    }
    pButton = (CButton *)GetDlgItem ( IDC_LIFEBAR_SHOW );
    if ( pButton->GetCheck () == 1 )
    {
      m_dwLifeBarFlags = (m_dwLifeBarFlags & (~UnitTypeFlags_LifeBar_Mask)) | UnitTypeFlags_LifeBar_ShowAlways;
    }
  }
  else
  {
    CButton * pButton;
    switch ( m_dwLifeBarFlags & UnitTypeFlags_LifeBar_Mask )
    {
    case UnitTypeFlags_LifeBar_ShowNever:
      pButton = (CButton *)GetDlgItem ( IDC_LIFEBAR_HIDE );
      pButton->SetCheck ( 1 );
      pButton = (CButton *)GetDlgItem ( IDC_LIFEBAR_SELECTED );
      pButton->SetCheck ( 0 );
      pButton = (CButton *)GetDlgItem ( IDC_LIFEBAR_SHOW );
      pButton->SetCheck ( 0 );
      break;
    case UnitTypeFlags_LifeBar_ShowWhenSelected:
      pButton = (CButton *)GetDlgItem ( IDC_LIFEBAR_HIDE );
      pButton->SetCheck ( 0 );
      pButton = (CButton *)GetDlgItem ( IDC_LIFEBAR_SELECTED );
      pButton->SetCheck ( 1 );
      pButton = (CButton *)GetDlgItem ( IDC_LIFEBAR_SHOW );
      pButton->SetCheck ( 0 );
      break;
    case UnitTypeFlags_LifeBar_ShowAlways:
      pButton = (CButton *)GetDlgItem ( IDC_LIFEBAR_HIDE );
      pButton->SetCheck ( 0 );
      pButton = (CButton *)GetDlgItem ( IDC_LIFEBAR_SELECTED );
      pButton->SetCheck ( 0 );
      pButton = (CButton *)GetDlgItem ( IDC_LIFEBAR_SHOW );
      pButton->SetCheck ( 1 );
      break;
    }
  }
}


BEGIN_MESSAGE_MAP(CUnitMainPage, CPropertyPage)
	//{{AFX_MSG_MAP(CUnitMainPage)
	ON_BN_CLICKED(IDC_SELECTABLE, OnSelectable)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUnitMainPage message handlers

BOOL CUnitMainPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

  m_strName = m_pUnitType->GetName();
  m_dwLifeMax = m_pUnitType->GetLifeMax();
  m_dwViewRadius = m_pUnitType->GetVeiwRadius();
  m_bSelectable = m_pUnitType->GetFlags() & UnitTypeFlags_Selectable;
  m_dwLifeBarFlags = m_pUnitType->GetFlags () & UnitTypeFlags_LifeBar_Mask;

  m_dwZPos = m_pUnitType->GetZPos();
  m_wndZPos.AddString("200000 Vysoká letadla");
  m_wndZPos.AddString("150000 Nízká letadla");
  m_wndZPos.AddString("45000 Vznášedla");
  m_wndZPos.AddString("35000 Pozemní jednotky");
  m_wndZPos.AddString("25000 Podzemí");
  m_wndZPos.AddString("15000 Voda");
  m_wndZPos.AddString("5000 Pod vodou");

  UpdateData ( FALSE );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CUnitMainPage::Create(CEUnitType *pUnitType)
{
  ASSERT_VALID(pUnitType);

  m_pUnitType = pUnitType;
}

void CUnitMainPage::OnOK() 
{
	CPropertyPage::OnOK();
}

BOOL CUnitMainPage::OnKillActive() 
{
  UpdateData(TRUE);

  if(m_strName.GetLength() == 0){
    AfxMessageBox("Musíte zadat jméno jednotky.");
    return FALSE;
  }

  m_pUnitType->SetName(m_strName);
  m_pUnitType->SetLifeMax(m_dwLifeMax);
  m_pUnitType->SetViewRadius(m_dwViewRadius);
  if(m_bSelectable){
    m_pUnitType->SetFlags(m_pUnitType->GetFlags() | UnitTypeFlags_Selectable);
  }
  else{
    m_pUnitType->SetFlags(m_pUnitType->GetFlags() & (~UnitTypeFlags_Selectable));
  }
  m_pUnitType->SetFlags ( (m_pUnitType->GetFlags () & (~UnitTypeFlags_LifeBar_Mask)) | m_dwLifeBarFlags );
  m_pUnitType->SetZPos(m_dwZPos);
  
	return CPropertyPage::OnKillActive();
}

void CUnitMainPage::OnSelectable() 
{
  UpdateData ( TRUE );

	CButton * pButton = (CButton *)GetDlgItem ( IDC_LIFEBAR_SELECTED );
  ASSERT_VALID ( pButton );

  if ( m_bSelectable )
  {
    pButton->EnableWindow ( TRUE );
  }
  else
  {
    if ( pButton->GetCheck () == 1 )
    {
      CButton * pShowButton = (CButton *)GetDlgItem ( IDC_LIFEBAR_SHOW );
      ASSERT_VALID ( pShowButton );
      pShowButton->SetCheck ( 1 );
    }
    pButton->SetCheck ( 0 );
    pButton->EnableWindow ( FALSE );
  }
}
