// EMoveSkillTypeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\MapEditor.h"
#include "EMoveSkillTypeDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEMoveSkillTypeDlg dialog


CEMoveSkillTypeDlg::CEMoveSkillTypeDlg(CWnd* pParent /*=NULL*/)
	: CUnitSkillTypeDlg(CEMoveSkillTypeDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEMoveSkillTypeDlg)
	m_dwSpeed = 0;
	//}}AFX_DATA_INIT
}


void CEMoveSkillTypeDlg::DoDataExchange(CDataExchange* pDX)
{
	CUnitSkillTypeDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEMoveSkillTypeDlg)
	DDX_Text(pDX, IDC_SPEED, m_dwSpeed);
	DDV_MinMaxDWord(pDX, m_dwSpeed, 0, 1000);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEMoveSkillTypeDlg, CUnitSkillTypeDlg)
	//{{AFX_MSG_MAP(CEMoveSkillTypeDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEMoveSkillTypeDlg message handlers

void CEMoveSkillTypeDlg::InitNewSkill(CEUnitSkillType *pSkillType)
{
  pSkillType->AllocateData(sizeof(SMMoveSkillType));

  // init the data
  SMMoveSkillType *pData = (SMMoveSkillType *)pSkillType->GetData();
  pData->m_dwSpeed = 10;
  pData->m_dwMoveAppearanceID = 0;
}

void CEMoveSkillTypeDlg::InitSkillTypeDlg(CEUnitSkillType *pSkillType)
{
  CUnitSkillTypeDlg::InitSkillTypeDlg(pSkillType);

  // copy the data to the dialog
  // can be also done in init dialog
  SMMoveSkillType *pData = (SMMoveSkillType *)pSkillType->GetData();
  m_dwSpeed = pData->m_dwSpeed;
  m_dwMoveAppearanceID = pData->m_dwMoveAppearanceID;
}

void CEMoveSkillTypeDlg::CloseSkillType(BOOL bApply)
{
  SMMoveSkillType *pData = (SMMoveSkillType *)m_pSkillType->GetData();

  if(bApply){
    pData->m_dwSpeed = m_dwSpeed;
    pData->m_dwMoveAppearanceID = m_dwMoveAppearanceID;
  }
}

void CEMoveSkillTypeDlg::DeleteSkill(CEUnitSkillType *pSkillType)
{
  SMMoveSkillType *pData = (SMMoveSkillType *)pSkillType->GetData();
  
  // delete the skill data
}

BOOL CEMoveSkillTypeDlg::OnInitDialog() 
{
	CUnitSkillTypeDlg::OnInitDialog();
	
  // Create the move appearance control
  m_wndMoveAppearance.Create(this, IDC_MOVEAPPEARANCE, m_pSkillType->GetUnitType(), m_dwMoveAppearanceID);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEMoveSkillTypeDlg::OnOK() 
{
  m_dwMoveAppearanceID = m_wndMoveAppearance.GetSelectedAppearanceID();
  
	CUnitSkillTypeDlg::OnOK();
}
