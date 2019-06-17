// EMakeSkillTypeRecordDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\mapeditor.h"
#include "EMakeSkillTypeRecordDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEMakeSkillTypeRecordDlg dialog


CEMakeSkillTypeRecordDlg::CEMakeSkillTypeRecordDlg(SMMakeSkillTypeRecord *pRecord, CEUnitSkillType * pSkillType, CWnd* pParent /*=NULL*/)
	: CDialog(CEMakeSkillTypeRecordDlg::IDD, pParent)
{
    ASSERT(pSkillType!=NULL);
    ASSERT(pRecord!=NULL);

    m_pSkillType=pSkillType;
    m_pRecord=pRecord;

	//{{AFX_DATA_INIT(CEMakeSkillTypeRecordDlg)
	m_nTimeToMake = 0;
	//}}AFX_DATA_INIT

    m_nTimeToMake=m_pRecord->m_nTimeToMake;
}


void CEMakeSkillTypeRecordDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEMakeSkillTypeRecordDlg)
	DDX_Text(pDX, IDC_TIMETOMAKE, m_nTimeToMake);
	//}}AFX_DATA_MAP

	DDV_MinMaxInt(pDX, m_nTimeToMake, 0, SMMakeSkillType::maxTimeToMake);
}


BEGIN_MESSAGE_MAP(CEMakeSkillTypeRecordDlg, CDialog)
	//{{AFX_MSG_MAP(CEMakeSkillTypeRecordDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEMakeSkillTypeRecordDlg message handlers

BOOL CEMakeSkillTypeRecordDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

    m_dwMakeAppearanceID=m_pRecord->m_dwAppearance;
    m_nTimeToMake=m_pRecord->m_nTimeToMake;
    memcpy(m_ResourcesNeeded, m_pRecord->m_ResourcesNeeded, sizeof(m_ResourcesNeeded));

    m_wndMakeAppearance.Create(this, IDC_APPEARANCE, m_pSkillType->GetUnitType(), m_dwMakeAppearanceID);
    m_wndResources.Create(this, IDC_RESOURCES, m_ResourcesNeeded);
    m_wndUnitType.Create(this, IDC_UNITTYPE, NULL);
    if(m_pRecord->m_dwUnitType!=0) {
        m_wndUnitType.SetSelectedUnitType(m_pRecord->m_dwUnitType);
    }

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEMakeSkillTypeRecordDlg::OnOK() 
{
    ASSERT(m_pRecord!=NULL);
    
    CEUnitType *pUnitType=m_wndUnitType.GetSelectedUnitType();
    if(pUnitType==NULL) {
        MessageBox( "Musíte vybrat typ jednotky.\n", "Špatnì zadaná data.");
        return;
    }

    UpdateData(TRUE);
    m_pRecord->m_dwAppearance=m_wndMakeAppearance.GetSelectedAppearanceID();
    m_pRecord->m_nTimeToMake=m_nTimeToMake;
    memcpy(m_pRecord->m_ResourcesNeeded, &m_ResourcesNeeded, sizeof(m_ResourcesNeeded));
    m_pRecord->m_dwUnitType=pUnitType->GetID();

    CDialog::OnOK();
}
