// EMineSkillTypeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\mapeditor.h"
#include "EMineSkillTypeDlg.h"
#include "..\..\Common\Map\Map.h"
#include "..\DataObjects\EUnitType.h"

extern DWORD g_dwMapFileVersion;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CEMineSkillTypeDlg dialog


CEMineSkillTypeDlg::CEMineSkillTypeDlg(CWnd* pParent /*=NULL*/)
	: CUnitSkillTypeDlg(CEMineSkillTypeDlg::IDD, pParent)
{

	//{{AFX_DATA_INIT(CEMineSkillTypeDlg)
	m_dwUnloadDistance = 0;
	m_dwMineDistance = 0;
	m_dwTimeslicesAfterMine = 0;
	m_dwTimeslicesAfterUnload = 0;
	m_dwTimeslicesBeforeMine = 0;
	m_dwTimeslicesBeforeUnload = 0;
	m_dwTimeslicesAfterUnloadFull = 0;
	m_dwTimeslicesBeforeMineFull = 0;
	//}}AFX_DATA_INIT
}


void CEMineSkillTypeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEMineSkillTypeDlg)
	DDX_Control(pDX, IDC_LOADED_MODE, m_wndMode);
	DDX_Text(pDX, IDC_UNLOAD_DISTANCE, m_dwUnloadDistance);
	DDV_MinMaxDWord(pDX, m_dwUnloadDistance, 0, 100000);
	DDX_Text(pDX, IDC_MINE_DISTANCE, m_dwMineDistance);
	DDV_MinMaxDWord(pDX, m_dwMineDistance, 0, 100000);
	DDX_Text(pDX, IDC_TIMESLICES_AFTER_MINE, m_dwTimeslicesAfterMine);
	DDV_MinMaxDWord(pDX, m_dwTimeslicesAfterMine, 0, 100000);
	DDX_Text(pDX, IDC_TIMESLICES_AFTER_UNLOAD, m_dwTimeslicesAfterUnload);
	DDV_MinMaxDWord(pDX, m_dwTimeslicesAfterUnload, 0, 100000);
	DDX_Text(pDX, IDC_TIMESLICES_BEFORE_MINE, m_dwTimeslicesBeforeMine);
	DDV_MinMaxDWord(pDX, m_dwTimeslicesBeforeMine, 0, 100000);
	DDX_Text(pDX, IDC_TIMESLICES_BEFORE_UNLOAD, m_dwTimeslicesBeforeUnload);
	DDV_MinMaxDWord(pDX, m_dwTimeslicesBeforeUnload, 0, 100000);
	DDX_Text(pDX, IDC_TIMESLICES_AFTER_UNLOAD_FULL, m_dwTimeslicesAfterUnloadFull);
	DDV_MinMaxDWord(pDX, m_dwTimeslicesAfterUnloadFull, 0, 100000);
	DDX_Text(pDX, IDC_TIMESLICES_BEFORE_MINE_FULL, m_dwTimeslicesBeforeMineFull);
	DDV_MinMaxDWord(pDX, m_dwTimeslicesBeforeMineFull, 0, 100000);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEMineSkillTypeDlg, CDialog)
	//{{AFX_MSG_MAP(CEMineSkillTypeDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CEMineSkillTypeDlg::InitNewSkill(CEUnitSkillType *pSkillType)
{
	pSkillType->AllocateData(sizeof(SMMineSkillType));

	// init the data    
	SMMineSkillType *pData = (SMMineSkillType *)pSkillType->GetData();

	// fill the data with default value
	for(int i=0; i<RESOURCE_COUNT; i++) {
		pData->m_StorageSize[i]=0;
		pData->m_UnloadPerTimeslice[i]=0;
		pData->m_LoadPerTimeslice[i]=0;
	}

	pData->m_dwAppearanceBeforeMining=0;
	pData->m_dwAppearanceWhileMining=0;
	pData->m_dwAppearanceAfterMining=0;

	pData->m_dwAppearanceBeforeUnload=0;
	pData->m_dwAppearanceWhileUnload=0;
	pData->m_dwAppearanceAfterUnload=0;

	pData->m_dwAppearanceBeforeMiningFull=0;
	pData->m_dwAppearanceWhileMiningFull=0;
	pData->m_dwAppearanceAfterUnloadFull=0;


	pData->m_dwMaxMineDistance=10;
	pData->m_dwMaxUnloadDistance=10;

	pData->m_dwTimeslicesBeforeMining=0;
	pData->m_dwTimeslicesAfterMining=0;

	pData->m_dwTimeslicesBeforeUnload=0;
	pData->m_dwTimeslicesAfterUnload=0;	

	pData->m_dwTimeslicesBeforeMiningFull=0;		
	pData->m_dwTimeslicesAfterUnloadFull=0;

	pData->m_nModeFull=0;
}

void CEMineSkillTypeDlg::InitSkillTypeDlg(CEUnitSkillType *pSkillType)
{
	CUnitSkillTypeDlg::InitSkillTypeDlg(pSkillType);

	// copy the data to the dialog
	// can be also done in init dialog

	if(pSkillType->GetData()!=NULL) {
		memcpy(&m_Data, (SMMineSkillType *)pSkillType->GetData(), sizeof(m_Data));

		m_dwMineDistance = m_Data.m_dwMaxMineDistance;
		m_dwUnloadDistance = m_Data.m_dwMaxUnloadDistance;

		m_dwTimeslicesBeforeMine = m_Data.m_dwTimeslicesBeforeMining;
		m_dwTimeslicesAfterMine = m_Data.m_dwTimeslicesAfterMining;

		m_dwTimeslicesBeforeUnload = m_Data.m_dwTimeslicesBeforeUnload;
		m_dwTimeslicesAfterUnload = m_Data.m_dwTimeslicesAfterUnload;

		m_dwTimeslicesBeforeMineFull = m_Data.m_dwTimeslicesBeforeMiningFull;
		m_dwTimeslicesAfterUnloadFull = m_Data.m_dwTimeslicesAfterUnloadFull;
	}
}

void CEMineSkillTypeDlg::CloseSkillType(BOOL bApply)
{
	if(bApply)
	{		 
		m_Data.m_dwMaxMineDistance = m_dwMineDistance;
		m_Data.m_dwMaxUnloadDistance = m_dwUnloadDistance;

		m_Data.m_dwTimeslicesBeforeMining = m_dwTimeslicesBeforeMine;
		m_Data.m_dwTimeslicesAfterMining = m_dwTimeslicesAfterMine;

		m_Data.m_dwTimeslicesBeforeUnload = m_dwTimeslicesBeforeUnload;
		m_Data.m_dwTimeslicesAfterUnload = m_dwTimeslicesAfterUnload;

		m_Data.m_dwTimeslicesBeforeMiningFull = m_dwTimeslicesBeforeMineFull;
		m_Data.m_dwTimeslicesAfterUnloadFull = m_dwTimeslicesAfterUnloadFull;
		
		SMMineSkillType *pData = (SMMineSkillType *)m_pSkillType->GetData();		
		memcpy(pData, &m_Data, sizeof(m_Data));
	}
}

void CEMineSkillTypeDlg::DeleteSkill(CEUnitSkillType *pSkillType)
{
    // TODO: delete the skill data
}

/////////////////////////////////////////////////////////////////////////////
// CEMineSkillTypeDlg message handlers

void CEMineSkillTypeDlg::OnOK() 
{
	// TODO: Add extra validation here

	m_Data.m_dwAppearanceBeforeMining = m_wndBeforeMineAppearance.GetSelectedAppearanceID();
	m_Data.m_dwAppearanceWhileMining = m_wndMineAppearance.GetSelectedAppearanceID();
	m_Data.m_dwAppearanceAfterMining = m_wndAfterMineAppearance.GetSelectedAppearanceID();

	m_Data.m_dwAppearanceBeforeUnload = m_wndBeforeUnloadAppearance.GetSelectedAppearanceID();
	m_Data.m_dwAppearanceWhileUnload = m_wndUnloadAppearance.GetSelectedAppearanceID();
	m_Data.m_dwAppearanceAfterUnload = m_wndAfterUnloadAppearance.GetSelectedAppearanceID();

	m_Data.m_dwAppearanceBeforeMiningFull = m_wndBeforeMineFullAppearance.GetSelectedAppearanceID();
	m_Data.m_dwAppearanceWhileMiningFull = m_wndMineFullAppearance.GetSelectedAppearanceID();
	m_Data.m_dwAppearanceAfterUnloadFull = m_wndAfterUnloadFullAppearance.GetSelectedAppearanceID();

	m_Data.m_dwMaxMineDistance = m_dwMineDistance;
	m_Data.m_dwMaxUnloadDistance = m_dwUnloadDistance;

	m_Data.m_dwTimeslicesBeforeMining = m_dwTimeslicesBeforeMine;
	m_Data.m_dwTimeslicesAfterMining = m_dwTimeslicesAfterMine;

	m_Data.m_dwTimeslicesBeforeUnload = m_dwTimeslicesBeforeUnload;
	m_Data.m_dwTimeslicesAfterUnload = m_dwTimeslicesAfterUnload;

	m_Data.m_dwTimeslicesBeforeMiningFull = m_dwTimeslicesBeforeMineFull;
	m_Data.m_dwTimeslicesAfterUnloadFull = m_dwTimeslicesAfterUnloadFull;

	int nSel = m_wndMode.GetCurSel();
	if(nSel == -1) {
		m_Data.m_nModeFull = 0;		
	} else {
		m_Data.m_nModeFull = m_wndMode.GetItemData(nSel);
	}

	CDialog::OnOK();
}

BOOL CEMineSkillTypeDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_wndBeforeMineAppearance.Create(this, IDC_APPEARANCE_BEFORE_MINE, m_pSkillType->GetUnitType(), m_Data.m_dwAppearanceBeforeMining);
	m_wndMineAppearance.Create(this, IDC_APPEARANCE_MINE, m_pSkillType->GetUnitType(), m_Data.m_dwAppearanceWhileMining);
	m_wndAfterMineAppearance.Create(this, IDC_APPEARANCE_AFTER_MINE, m_pSkillType->GetUnitType(), m_Data.m_dwAppearanceAfterMining);

	m_wndBeforeUnloadAppearance.Create(this, IDC_APPEARANCE_BEFORE_UNLOAD, m_pSkillType->GetUnitType(), m_Data.m_dwAppearanceBeforeUnload);
	m_wndUnloadAppearance.Create(this, IDC_APPEARANCE_UNLOAD, m_pSkillType->GetUnitType(), m_Data.m_dwAppearanceWhileUnload);
	m_wndAfterUnloadAppearance.Create(this, IDC_APPEARANCE_AFTER_UNLOAD, m_pSkillType->GetUnitType(), m_Data.m_dwAppearanceAfterUnload);
		
	m_wndStorageResources.Create(this, IDC_STORAGE_RESOURCES, m_Data.m_StorageSize);
	m_wndMineResources.Create(this, IDC_MINE_RESOURCES, m_Data.m_LoadPerTimeslice);
	m_wndUnloadResources.Create(this, IDC_UNLOAD_RESOURCES, m_Data.m_UnloadPerTimeslice);

	m_wndBeforeMineFullAppearance.Create(this, IDC_APPEARANCE_BEFORE_MINE_FULL, m_pSkillType->GetUnitType(), m_Data.m_dwAppearanceBeforeMiningFull);
	m_wndMineFullAppearance.Create(this, IDC_APPEARANCE_MINE_FULL, m_pSkillType->GetUnitType(), m_Data.m_dwAppearanceWhileMiningFull);
	m_wndAfterUnloadFullAppearance.Create(this, IDC_APPEARANCE_AFTER_UNLOAD_FULL, m_pSkillType->GetUnitType(), m_Data.m_dwAppearanceAfterUnloadFull);

	m_pSkillType->GetUnitType()->FillModesCombo(&m_wndMode);
	m_wndMode.SetCurSel(m_Data.m_nModeFull);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEMineSkillTypeDlg::OnLoadSkill(CEUnitSkillType *pSkillType)
{
	if(g_dwMapFileVersion>=MAP_FILE_VERSION(1, 6)) {
		// Mame uz vsechny vzhledy
	} else {
		pSkillType->ReallocateData(sizeof(m_Data));
	}
	if(g_dwMapFileVersion<=MAP_FILE_VERSION(1, 5)) {
		// Jenom pribyla informace o prepnuti modu
		// Konverze - pridani vzhledu

		SMMineSkillType *pData = (SMMineSkillType *)pSkillType->GetData();
		pData->m_dwAppearanceBeforeMiningFull=0;
		pData->m_dwTimeslicesBeforeMiningFull=0;
		
		pData->m_dwAppearanceWhileMiningFull=0;

		pData->m_dwAppearanceAfterUnloadFull=0;
		pData->m_dwTimeslicesAfterUnloadFull=0;
	} 
	if(g_dwMapFileVersion<=MAP_FILE_VERSION(1, 4)) {
		// Konverze - pridani modu

		SMMineSkillType *pData = (SMMineSkillType *)pSkillType->GetData();
		pData->m_nModeFull=0;
	}
}
