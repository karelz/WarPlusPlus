// UnitAppearanceDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\MapEditor.h"
#include "UnitAppearanceDlg.h"

#include "..\DataObjects\EUnitAppearance.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CUnitAppearanceDlg dialog


CUnitAppearanceDlg::CUnitAppearanceDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CUnitAppearanceDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CUnitAppearanceDlg)
	m_strName = _T("");
	//}}AFX_DATA_INIT
}


void CUnitAppearanceDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CUnitAppearanceDlg)
	DDX_Control(pDX, IDC_NAME, m_wndName);
	DDX_Text(pDX, IDC_NAME, m_strName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CUnitAppearanceDlg, CDialog)
	//{{AFX_MSG_MAP(CUnitAppearanceDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUnitAppearanceDlg message handlers

void CUnitAppearanceDlg::Create(CEUnitAppearanceType *pAppearanceType)
{
  ASSERT_VALID(pAppearanceType);

  m_pAppearanceType = pAppearanceType;
}

BOOL CUnitAppearanceDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

  m_strName = m_pAppearanceType->GetName();
	
  if(!m_bEditName){
    m_wndName.EnableWindow(FALSE);
  }

  CStatic *pPreview;
  DWORD dwStyle;
  CRect rcPreview;

  pPreview = (CStatic *)GetDlgItem(IDC_NORTH); dwStyle = pPreview->GetStyle();
  pPreview->GetWindowRect(&rcPreview); ScreenToClient(&rcPreview);
  pPreview->DestroyWindow();
  m_wndNorth.Create(rcPreview, dwStyle, this, IDC_NORTH, m_pAppearanceType, CEUnitAppearanceType::Dir_North);
  m_wndNorth.SetAnimation(m_pAppearanceType->GetInstance()->GetDirection(CEUnitAppearanceType::Dir_North));
	
  pPreview = (CStatic *)GetDlgItem(IDC_NORTHEAST); dwStyle = pPreview->GetStyle();
  pPreview->GetWindowRect(&rcPreview); ScreenToClient(&rcPreview);
  pPreview->DestroyWindow();
  m_wndNorthEast.Create(rcPreview, dwStyle, this, IDC_NORTHEAST, m_pAppearanceType, CEUnitAppearanceType::Dir_NorthEast);
  m_wndNorthEast.SetAnimation(m_pAppearanceType->GetInstance()->GetDirection(CEUnitAppearanceType::Dir_NorthEast));

  pPreview = (CStatic *)GetDlgItem(IDC_EAST); dwStyle = pPreview->GetStyle();
  pPreview->GetWindowRect(&rcPreview); ScreenToClient(&rcPreview);
  pPreview->DestroyWindow();
  m_wndEast.Create(rcPreview, dwStyle, this, IDC_EAST, m_pAppearanceType, CEUnitAppearanceType::Dir_East);
  m_wndEast.SetAnimation(m_pAppearanceType->GetInstance()->GetDirection(CEUnitAppearanceType::Dir_East));

  pPreview = (CStatic *)GetDlgItem(IDC_SOUTHEAST); dwStyle = pPreview->GetStyle();
  pPreview->GetWindowRect(&rcPreview); ScreenToClient(&rcPreview);
  pPreview->DestroyWindow();
  m_wndSouthEast.Create(rcPreview, dwStyle, this, IDC_SOUTHEAST, m_pAppearanceType, CEUnitAppearanceType::Dir_SouthEast);
  m_wndSouthEast.SetAnimation(m_pAppearanceType->GetInstance()->GetDirection(CEUnitAppearanceType::Dir_SouthEast));

  pPreview = (CStatic *)GetDlgItem(IDC_SOUTH); dwStyle = pPreview->GetStyle();
  pPreview->GetWindowRect(&rcPreview); ScreenToClient(&rcPreview);
  pPreview->DestroyWindow();
  m_wndSouth.Create(rcPreview, dwStyle, this, IDC_SOUTH, m_pAppearanceType, CEUnitAppearanceType::Dir_South);
  m_wndSouth.SetAnimation(m_pAppearanceType->GetInstance()->GetDirection(CEUnitAppearanceType::Dir_South));

  pPreview = (CStatic *)GetDlgItem(IDC_SOUTHWEST); dwStyle = pPreview->GetStyle();
  pPreview->GetWindowRect(&rcPreview); ScreenToClient(&rcPreview);
  pPreview->DestroyWindow();
  m_wndSouthWest.Create(rcPreview, dwStyle, this, IDC_SOUTHWEST, m_pAppearanceType, CEUnitAppearanceType::Dir_SouthWest);
  m_wndSouthWest.SetAnimation(m_pAppearanceType->GetInstance()->GetDirection(CEUnitAppearanceType::Dir_SouthWest));

  pPreview = (CStatic *)GetDlgItem(IDC_WEST); dwStyle = pPreview->GetStyle();
  pPreview->GetWindowRect(&rcPreview); ScreenToClient(&rcPreview);
  pPreview->DestroyWindow();
  m_wndWest.Create(rcPreview, dwStyle, this, IDC_WEST, m_pAppearanceType, CEUnitAppearanceType::Dir_West);
  m_wndWest.SetAnimation(m_pAppearanceType->GetInstance()->GetDirection(CEUnitAppearanceType::Dir_West));

  pPreview = (CStatic *)GetDlgItem(IDC_NORTHWEST); dwStyle = pPreview->GetStyle();
  pPreview->GetWindowRect(&rcPreview); ScreenToClient(&rcPreview);
  pPreview->DestroyWindow();
  m_wndNorthWest.Create(rcPreview, dwStyle, this, IDC_NORTHWEST, m_pAppearanceType, CEUnitAppearanceType::Dir_NorthWest);
  m_wndNorthWest.SetAnimation(m_pAppearanceType->GetInstance()->GetDirection(CEUnitAppearanceType::Dir_NorthWest));

  UpdateData(FALSE);

  return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CUnitAppearanceDlg::OnOK() 
{
  UpdateData(TRUE);

  m_pAppearanceType->SetName(m_strName);
  m_pAppearanceType->GetInstance()->SaveToType(m_pAppearanceType);
  
	CDialog::OnOK();
}

