// EResourceSkillTypeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\mapeditor.h"
#include "EResourceSkillTypeDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEResourceSkillTypeDlg dialog


CEResourceSkillTypeDlg::CEResourceSkillTypeDlg(CWnd* pParent /*=NULL*/)
	: CUnitSkillTypeDlg(CEResourceSkillTypeDlg::IDD, pParent)
{

	//{{AFX_DATA_INIT(CEResourceSkillTypeDlg)
	m_dwTimeslicesAfterMine = 0;
	//}}AFX_DATA_INIT
}


void CEResourceSkillTypeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEResourceSkillTypeDlg)
	DDX_Text(pDX, IDC_TIMESLICES_AFTER_MINE, m_dwTimeslicesAfterMine);
	DDV_MinMaxDWord(pDX, m_dwTimeslicesAfterMine, 0, 100000);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEResourceSkillTypeDlg, CDialog)
	//{{AFX_MSG_MAP(CEResourceSkillTypeDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CEResourceSkillTypeDlg::InitNewSkill(CEUnitSkillType *pSkillType)
{
	int i, j;

	pSkillType->AllocateData(sizeof(SMResourceSkillType));

	// init the data    
	SMResourceSkillType *pData = (SMResourceSkillType *)pSkillType->GetData();
    
	// fill the data with zeroes
	for(i=0; i<3; i++) {
	  pData->m_dwAppearance[i]=pData->m_dwAppearanceMine[i]=0;
	}
	
	pData->m_dwAppearanceAfterMining=0;
	pData->m_dwTimeslicesAfterMining=0;	

	for(i=0; i<RESOURCE_COUNT; i++) {
		pData->m_AvailablePerTimeslice[i]=0;
		pData->m_StorageSize[i]=0;
		for(j=0; j<3; j++) {
			pData->m_ResourcesLimit[j][i]=0;
		}
	}
}

void CEResourceSkillTypeDlg::InitSkillTypeDlg(CEUnitSkillType *pSkillType)
{
	CUnitSkillTypeDlg::InitSkillTypeDlg(pSkillType);

	// copy the data to the dialog
	// can be also done in init dialog

	if(pSkillType->GetData()!=NULL) {
		memcpy(&m_Data, (SMResourceSkillType *)pSkillType->GetData(), sizeof(m_Data));
		
		m_dwTimeslicesAfterMine=m_Data.m_dwTimeslicesAfterMining;
	}
}

void CEResourceSkillTypeDlg::CloseSkillType(BOOL bApply)
{
    if(bApply)
    {
        SMResourceSkillType *pData = (SMResourceSkillType *)m_pSkillType->GetData();

		m_Data.m_dwTimeslicesAfterMining=m_dwTimeslicesAfterMine;

        memcpy(pData, &m_Data, sizeof(m_Data));
	 }
}

void CEResourceSkillTypeDlg::DeleteSkill(CEUnitSkillType *pSkillType)
{
    // TODO: delete the skill data
}

/////////////////////////////////////////////////////////////////////////////
// CEResourceSkillTypeDlg message handlers

BOOL CEResourceSkillTypeDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_wndAppearance1.Create(this, IDC_APPEARANCE1, m_pSkillType->GetUnitType(), m_Data.m_dwAppearance[0]);
	m_wndAppearance2.Create(this, IDC_APPEARANCE2, m_pSkillType->GetUnitType(), m_Data.m_dwAppearance[1]);
	m_wndAppearance3.Create(this, IDC_APPEARANCE3, m_pSkillType->GetUnitType(), m_Data.m_dwAppearance[2]);

	m_wndAppearanceMine1.Create(this, IDC_APPEARANCEMINE1, m_pSkillType->GetUnitType(), m_Data.m_dwAppearanceMine[0]);
	m_wndAppearanceMine2.Create(this, IDC_APPEARANCEMINE2, m_pSkillType->GetUnitType(), m_Data.m_dwAppearanceMine[1]);
	m_wndAppearanceMine3.Create(this, IDC_APPEARANCEMINE3, m_pSkillType->GetUnitType(), m_Data.m_dwAppearanceMine[2]);

	m_wndAppearanceAfterMining.Create(this, IDC_APPEARANCE_AFTER_MINE, m_pSkillType->GetUnitType(), m_Data.m_dwAppearanceAfterMining);

	m_wndInitResources.Create(this, IDC_INITRESOURCES, m_Data.m_StorageSize);
	m_wndTimeSliceResources.Create(this, IDC_TSRESOURCES, m_Data.m_AvailablePerTimeslice);

	m_wndResourcesLimit1.Create(this, IDC_RESOURCESLIMIT1, m_Data.m_ResourcesLimit[0]);
	m_wndResourcesLimit2.Create(this, IDC_RESOURCESLIMIT2, m_Data.m_ResourcesLimit[1]);
	m_wndResourcesLimit3.Create(this, IDC_RESOURCESLIMIT3, m_Data.m_ResourcesLimit[2]);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEResourceSkillTypeDlg::OnOK() 
{
	// TODO: Add extra validation here
	
	m_Data.m_dwAppearance[0]=m_wndAppearance1.GetSelectedAppearanceID();
	m_Data.m_dwAppearance[1]=m_wndAppearance2.GetSelectedAppearanceID();
	m_Data.m_dwAppearance[2]=m_wndAppearance3.GetSelectedAppearanceID();

	m_Data.m_dwAppearanceMine[0]=m_wndAppearanceMine1.GetSelectedAppearanceID();
	m_Data.m_dwAppearanceMine[1]=m_wndAppearanceMine2.GetSelectedAppearanceID();
	m_Data.m_dwAppearanceMine[2]=m_wndAppearanceMine3.GetSelectedAppearanceID();

	m_Data.m_dwAppearanceAfterMining=m_wndAppearanceAfterMining.GetSelectedAppearanceID();	
	
	m_Data.m_dwTimeslicesAfterMining=m_dwTimeslicesAfterMine;

	CDialog::OnOK();
}
