// ExampleSkillTypeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\MapEditor.h"
#include "ExampleSkillTypeDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CExampleSkillTypeDlg dialog


CExampleSkillTypeDlg::CExampleSkillTypeDlg(CWnd* pParent /*=NULL*/)
	: CUnitSkillTypeDlg(CExampleSkillTypeDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CExampleSkillTypeDlg)
	m_strDescription = _T("");
	//}}AFX_DATA_INIT
}


void CExampleSkillTypeDlg::DoDataExchange(CDataExchange* pDX)
{
	CUnitSkillTypeDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CExampleSkillTypeDlg)
	DDX_Text(pDX, IDC_EDIT1, m_strDescription);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CExampleSkillTypeDlg, CDialog)
	//{{AFX_MSG_MAP(CExampleSkillTypeDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CExampleSkillTypeDlg message handlers

void CExampleSkillTypeDlg::InitNewSkill(CEUnitSkillType *pSkillType)
{
  pSkillType->AllocateData(sizeof(SSkillData));

  // init the data
  SSkillData *pData = (SSkillData *)pSkillType->GetData();
  memset(pData, 0, sizeof(SSkillData));
}

void CExampleSkillTypeDlg::InitSkillTypeDlg(CEUnitSkillType *pSkillType)
{
  CUnitSkillTypeDlg::InitSkillTypeDlg(pSkillType);

  // copy the data to the dialog
  // can be also done in init dialog
  SSkillData *pData = (SSkillData *)pSkillType->GetData();
  m_strDescription = pData->m_pDescription;
}

BOOL CExampleSkillTypeDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CExampleSkillTypeDlg::CloseSkillType(BOOL bApply)
{
  SSkillData *pData = (SSkillData *)m_pSkillType->GetData();

  if(bApply){
    memcpy(pData, m_strDescription, 30);
  }
}

void CExampleSkillTypeDlg::DeleteSkill(CEUnitSkillType *pSkillType)
{
  SSkillData *pData = (SSkillData *)pSkillType->GetData();
  
  // delete the skill data
}
