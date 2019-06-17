// EBulletAttackSkillTypeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\mapeditor.h"
#include "EBulletAttackSkillTypeDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEBulletAttackSkillTypeDlg dialog


CEBulletAttackSkillTypeDlg::CEBulletAttackSkillTypeDlg(CWnd* pParent /*=NULL*/)
	: CUnitSkillTypeDlg(CEBulletAttackSkillTypeDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEBulletAttackSkillTypeDlg)
	m_nIntensity = 0;
	m_dwLength = 0;
	m_dwLoadTime = 0;
	m_dwOffset = 0;
	m_dwPeriod = 0;
	m_dwAltitudeMax = 0;
	m_dwAltitudeMin = 0;
	m_dwRadius = 0;
	//}}AFX_DATA_INIT

    m_dwBulletAttackAppearanceID = 0;
}


void CEBulletAttackSkillTypeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEBulletAttackSkillTypeDlg)
	DDX_Text(pDX, IDC_INTENSITY, m_nIntensity);
	DDX_Text(pDX, IDC_LENGTH, m_dwLength);
	DDX_Text(pDX, IDC_LOADTIME, m_dwLoadTime);
	DDX_Text(pDX, IDC_OFFSET, m_dwOffset);
	DDX_Text(pDX, IDC_PERIOD, m_dwPeriod);
	DDX_Text(pDX, IDC_ALTITUDEMAX, m_dwAltitudeMax);
	DDX_Text(pDX, IDC_ALTITUDEMIN, m_dwAltitudeMin);
	DDX_Text(pDX, IDC_RADIUS, m_dwRadius);
	//}}AFX_DATA_MAP

	DDV_MinMaxInt(pDX, m_nIntensity, SM_BULLETATTACK_SKILLTYPE_INTENSITY_MIN, SM_BULLETATTACK_SKILLTYPE_INTENSITY_MAX);
	DDV_MinMaxDWord(pDX, m_dwLength, SM_BULLETATTACK_SKILLTYPE_ANIMATIONLENGTH_MIN, UINT_MAX);
	DDV_MinMaxDWord(pDX, m_dwLoadTime, 0, SM_BULLETATTACK_SKILLTYPE_LOADTIME_MAX);
	DDV_MinMaxDWord(pDX, m_dwPeriod, 0, SM_BULLETATTACK_SKILLTYPE_FIREPERIOD_MAX);
	DDV_MinMaxDWord(pDX, m_dwAltitudeMax, 0, SM_BULLETATTACK_SKILLTYPE_ALTITUDE_MAX);
	DDV_MinMaxDWord(pDX, m_dwAltitudeMin, SM_BULLETATTACK_SKILLTYPE_ALTITUDE_MIN, UINT_MAX);
	DDV_MinMaxDWord(pDX, m_dwRadius, SM_BULLETATTACK_SKILLTYPE_RADIUS_MIN, SM_BULLETATTACK_SKILLTYPE_RADIUS_MAX);
    
    if ((pDX->m_bSaveAndValidate) && (m_dwAltitudeMin > m_dwAltitudeMax))
    {
        ::AfxMessageBox(_T("Špatné hodnoty u výšky cíle, minimální výška nesmí být vìtší než maximální výška cíle"), MB_OK | MB_ICONEXCLAMATION, 0);
        pDX->Fail();
    }
}


BEGIN_MESSAGE_MAP(CEBulletAttackSkillTypeDlg, CDialog)
	//{{AFX_MSG_MAP(CEBulletAttackSkillTypeDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEBulletAttackSkillTypeDlg message handlers

void CEBulletAttackSkillTypeDlg::InitNewSkill(CEUnitSkillType *pSkillType)
{
    pSkillType->AllocateData(sizeof(SMBulletAttackSkillType));
    
    // init the data
    SMBulletAttackSkillType *pData = (SMBulletAttackSkillType *) pSkillType->GetData();
    pData->m_nAttackIntensity = 10;
    pData->m_dwAttackMinAltitude = 1;
    pData->m_dwAttackMaxAltitude = 1000000;
    pData->m_dwAttackRadius = 10;
    pData->m_dwLoadTime = 3;
    pData->m_dwFirePeriod = 10;
    pData->m_dwAnimationOffset = 0;
    pData->m_dwAnimationLength = 10;
    pData->m_dwAppearanceID = 0;
}

void CEBulletAttackSkillTypeDlg::InitSkillTypeDlg(CEUnitSkillType *pSkillType)
{
    CUnitSkillTypeDlg::InitSkillTypeDlg(pSkillType);
    
    // copy the data to the dialog
    // can be also done in init dialog
    SMBulletAttackSkillType *pData = (SMBulletAttackSkillType *) pSkillType->GetData();

    m_nIntensity = pData->m_nAttackIntensity;
    m_dwAltitudeMin = pData->m_dwAttackMinAltitude;
    m_dwAltitudeMax = pData->m_dwAttackMaxAltitude;
    m_dwRadius = pData->m_dwAttackRadius;
    m_dwLoadTime = pData->m_dwLoadTime;
    m_dwPeriod = pData->m_dwFirePeriod;
    m_dwOffset = pData->m_dwAnimationOffset;
    m_dwLength = pData->m_dwAnimationLength;
    m_dwBulletAttackAppearanceID = pData->m_dwAppearanceID;
}

void CEBulletAttackSkillTypeDlg::CloseSkillType(BOOL bApply)
{
    SMBulletAttackSkillType *pData = (SMBulletAttackSkillType *) m_pSkillType->GetData();
    
    if (bApply)
    {
        pData->m_nAttackIntensity = m_nIntensity;
        pData->m_dwAttackMinAltitude = m_dwAltitudeMin;
        pData->m_dwAttackMaxAltitude = m_dwAltitudeMax;
        pData->m_dwAttackRadius = m_dwRadius;
        pData->m_dwLoadTime = m_dwLoadTime;
        pData->m_dwFirePeriod = m_dwPeriod;
        pData->m_dwAnimationOffset = m_dwOffset;
        pData->m_dwAnimationLength = m_dwLength;
        pData->m_dwAppearanceID = m_dwBulletAttackAppearanceID;
    }
}

void CEBulletAttackSkillTypeDlg::DeleteSkill(CEUnitSkillType *pSkillType)
{
    SMBulletAttackSkillType *pData = (SMBulletAttackSkillType *) pSkillType->GetData();
    
    // delete the skill data
}


BOOL CEBulletAttackSkillTypeDlg::OnInitDialog() 
{
    CUnitSkillTypeDlg::OnInitDialog();
    
    // Create the move appearance control
    m_wndBulletAttackAppearance.Create(this, IDC_APPEARANCE, m_pSkillType->GetUnitType(), m_dwBulletAttackAppearanceID);
    m_wndMinAltitude.Create(this, IDC_ALTITUDEMIN);
    m_wndMaxAltitude.Create(this, IDC_ALTITUDEMAX);

    return TRUE;  // return TRUE unless you set the focus to a control
}


void CEBulletAttackSkillTypeDlg::OnOK() 
{
    m_dwBulletAttackAppearanceID = m_wndBulletAttackAppearance.GetSelectedAppearanceID();
    
    CUnitSkillTypeDlg::OnOK();
}
