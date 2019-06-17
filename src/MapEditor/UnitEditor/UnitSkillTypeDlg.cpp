// UnitSkillTypeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\MapEditor.h"
#include "UnitSkillTypeDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CUnitSkillTypeDlg dialog


CUnitSkillTypeDlg::CUnitSkillTypeDlg(int nID, CWnd* pParent /*=NULL*/)
	: CDialog(nID, pParent)
{
	//{{AFX_DATA_INIT(CUnitSkillTypeDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

  m_pSkillType = NULL;
}


void CUnitSkillTypeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CUnitSkillTypeDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CUnitSkillTypeDlg, CDialog)
	//{{AFX_MSG_MAP(CUnitSkillTypeDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUnitSkillTypeDlg message handlers

void CUnitSkillTypeDlg::Create()
{

}

void CUnitSkillTypeDlg::Delete()
{

}

void CUnitSkillTypeDlg::InitSkillTypeDlg(CEUnitSkillType *pSkillType)
{
  ASSERT_VALID(pSkillType);

  m_pSkillType = pSkillType;
}

CString CUnitSkillTypeDlg::GetName()
{
  CString res;
  res.Empty();
  return res;
}

void CUnitSkillTypeDlg::CloseSkillType(BOOL bApply)
{

}

void CUnitSkillTypeDlg::InitNewSkill(CEUnitSkillType *pSkillType)
{

}

void CUnitSkillTypeDlg::OnLoadSkill(CEUnitSkillType *pSkillType)
{

}

void CUnitSkillTypeDlg::DeleteSkill(CEUnitSkillType *pSkillType)
{

}

BOOL CUnitSkillTypeDlg::CheckValid(CEUnitSkillType *pSkillType)
{
  return TRUE;
}
