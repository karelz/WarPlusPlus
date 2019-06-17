// EBulletDefenseSkillTypeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\mapeditor.h"
#include "EBulletDefenseSkillTypeDlg.h"

#include "Common\Map\MBulletDefenseSkillType.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEBulletDefenseSkillTypeDlg dialog


CEBulletDefenseSkillTypeDlg::CEBulletDefenseSkillTypeDlg(CWnd* pParent /*=NULL*/)
	: CUnitSkillTypeDlg(CEBulletDefenseSkillTypeDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEBulletDefenseSkillTypeDlg)
	m_dwAbsolute = 0;
	m_fLinear = 0.0f;
	//}}AFX_DATA_INIT
}


void CEBulletDefenseSkillTypeDlg::DoDataExchange(CDataExchange* pDX)
{
	CUnitSkillTypeDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEBulletDefenseSkillTypeDlg)
	DDX_Text(pDX, IDC_ABSOLUTE, m_dwAbsolute);
	DDX_Text(pDX, IDC_LINEAR, m_fLinear);
	DDV_MinMaxFloat(pDX, m_fLinear, 0.f, 100.f);
	//}}AFX_DATA_MAP

	DDV_MinMaxDWord(pDX, m_dwAbsolute, SM_BULLETDEFENSE_SKILLTYPE_ABS_MIN, SM_BULLETDEFENSE_SKILLTYPE_ABS_MAX);
}


BEGIN_MESSAGE_MAP(CEBulletDefenseSkillTypeDlg, CUnitSkillTypeDlg)
	//{{AFX_MSG_MAP(CEBulletDefenseSkillTypeDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEBulletDefenseSkillTypeDlg message handlers

void CEBulletDefenseSkillTypeDlg::InitNewSkill(CEUnitSkillType *pSkillType)
{
    pSkillType->AllocateData(sizeof(SMBulletDefenseSkillType));
    
    // init the data
    SMBulletDefenseSkillType *pData = (SMBulletDefenseSkillType *) pSkillType->GetData();
    pData->m_dwLinearCoefficient = 1000;
    pData->m_dwAbsoluteCoefficient = 0;
}

void CEBulletDefenseSkillTypeDlg::InitSkillTypeDlg(CEUnitSkillType *pSkillType)
{
    CUnitSkillTypeDlg::InitSkillTypeDlg(pSkillType);
    
    // copy the data to the dialog
    // can be also done in init dialog
    SMBulletDefenseSkillType *pData = (SMBulletDefenseSkillType *) pSkillType->GetData();

    m_fLinear = static_cast<float>(1000 - pData->m_dwLinearCoefficient) / 10.f;
    m_dwAbsolute = pData->m_dwAbsoluteCoefficient;
}


void CEBulletDefenseSkillTypeDlg::CloseSkillType(BOOL bApply)
{
    SMBulletDefenseSkillType *pData = (SMBulletDefenseSkillType *) m_pSkillType->GetData();
    
    if (bApply)
    {
        pData->m_dwLinearCoefficient = 1000 - static_cast<DWORD>(m_fLinear * 10.f + 0.5f);
        pData->m_dwAbsoluteCoefficient = m_dwAbsolute;
    }
}


void CEBulletDefenseSkillTypeDlg::DeleteSkill(CEUnitSkillType *pSkillType)
{
    SMBulletDefenseSkillType *pData = (SMBulletDefenseSkillType *) pSkillType->GetData();
    
    // delete the skill data
}
