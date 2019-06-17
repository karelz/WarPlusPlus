// EStorageSkillTypeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\mapeditor.h"
#include "EStorageSkillTypeDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEStorageSkillTypeDlg dialog


CEStorageSkillTypeDlg::CEStorageSkillTypeDlg(CWnd* pParent /*=NULL*/)
	: CUnitSkillTypeDlg(CEStorageSkillTypeDlg::IDD, pParent)
{

	//{{AFX_DATA_INIT(CEStorageSkillTypeDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CEStorageSkillTypeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEStorageSkillTypeDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEStorageSkillTypeDlg, CDialog)
	//{{AFX_MSG_MAP(CEStorageSkillTypeDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CEStorageSkillTypeDlg::InitNewSkill(CEUnitSkillType *pSkillType)
{
	pSkillType->AllocateData(sizeof(SMStorageSkillType));

	// init the data    
	SMStorageSkillType *pData = (SMStorageSkillType *)pSkillType->GetData();
    
	// fill the data with zeroes
	pData->m_dwAppearanceWhileUnloading=0;
	for(int i=0; i<RESOURCE_COUNT; i++) pData->m_AcceptedPerTimeslice[i]=0;
}

void CEStorageSkillTypeDlg::InitSkillTypeDlg(CEUnitSkillType *pSkillType)
{
    CUnitSkillTypeDlg::InitSkillTypeDlg(pSkillType);

    // copy the data to the dialog
    // can be also done in init dialog

	 if(pSkillType->GetData()!=NULL) {
		memcpy(&m_Data, (SMStorageSkillType *)pSkillType->GetData(), sizeof(m_Data));
	 }
}

void CEStorageSkillTypeDlg::CloseSkillType(BOOL bApply)
{
    if(bApply)
    {
        SMStorageSkillType *pData = (SMStorageSkillType *)m_pSkillType->GetData();
        memcpy(pData, &m_Data, sizeof(m_Data));
	 }
}

void CEStorageSkillTypeDlg::DeleteSkill(CEUnitSkillType *pSkillType)
{
    // delete the skill data
}

/////////////////////////////////////////////////////////////////////////////
// CEStorageSkillTypeDlg message handlers

BOOL CEStorageSkillTypeDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_wndUnloadAppearance.Create(this, IDC_APPEARANCE, m_pSkillType->GetUnitType(), m_Data.m_dwAppearanceWhileUnloading);
	m_wndResources.Create(this, IDC_RESOURCES, m_Data.m_AcceptedPerTimeslice);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEStorageSkillTypeDlg::OnOK() 
{
	// TODO: Add extra validation here
	m_Data.m_dwAppearanceWhileUnloading = m_wndUnloadAppearance.GetSelectedAppearanceID();

	CDialog::OnOK();
}
