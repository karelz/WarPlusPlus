// emakeskilltypedlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\mapeditor.h"
#include "emakeskilltypedlg.h"
#include "MapEditor\DataObjects\EMap.h"
#include "EMakeSkillTypeRecordDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEMakeSkillTypeDlg dialog

CEMakeSkillTypeDlg::CEMakeSkillTypeDlg(CWnd* pParent /*=NULL*/)
	: CUnitSkillTypeDlg(CEMakeSkillTypeDlg::IDD, pParent)
{
    m_pData=NULL;
	//{{AFX_DATA_INIT(CEMakeSkillTypeDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CEMakeSkillTypeDlg::~CEMakeSkillTypeDlg() 
{
    if(m_pData!=NULL) {
        delete [] ((char *)m_pData);
        m_pData=NULL;
    }
}

void CEMakeSkillTypeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEMakeSkillTypeDlg)
	DDX_Control(pDX, IDC_REMOVEUNITTYPE, m_wndRemoveUnitType);
	DDX_Control(pDX, IDC_EDITUNITTYPE, m_wndEditUnitType);
	DDX_Control(pDX, IDC_ADDUNITTYPE, m_wndAddUnitType);
	DDX_Control(pDX, IDC_UNITTYPELIST, m_wndUnitTypeList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEMakeSkillTypeDlg, CDialog)
	//{{AFX_MSG_MAP(CEMakeSkillTypeDlg)
	ON_BN_CLICKED(IDC_ADDUNITTYPE, OnAddUnitType)
	ON_BN_CLICKED(IDC_EDITUNITTYPE, OnEditUnitType)
	ON_BN_CLICKED(IDC_REMOVEUNITTYPE, OnRemoveUnitType)
	ON_LBN_SELCHANGE(IDC_UNITTYPELIST, OnSelchangeUnitTypeList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

#define FIRST_RECORD  ((SMMakeSkillTypeRecord*)(((char*)m_pData)+sizeof(SMMakeSkillType)))

void CEMakeSkillTypeDlg::InitNewSkill(CEUnitSkillType *pSkillType)
{
    pSkillType->AllocateData(sizeof(SMMakeSkillType));

    // init the data    
    SMMakeSkillType *pData = (SMMakeSkillType *)pSkillType->GetData();
    pData->m_nRecords=0;
}

void CEMakeSkillTypeDlg::InitSkillTypeDlg(CEUnitSkillType *pSkillType)
{
    CUnitSkillTypeDlg::InitSkillTypeDlg(pSkillType);

    // copy the data to the dialog
    // can be also done in init dialog

    if(m_pData!=NULL) {
        delete [] ((char *)m_pData);
        m_pData=NULL;
    }

    SMMakeSkillType *pData = (SMMakeSkillType *)pSkillType->GetData();

    DWORD dwSize=MAKESKILLTYPE_SIZE(pData);
    m_pData=(SMMakeSkillType*)new char[dwSize];
    memcpy(m_pData, pData, dwSize);
}

void CEMakeSkillTypeDlg::CloseSkillType(BOOL bApply)
{
    if(bApply)
    {
        m_pSkillType->ReallocateData(MAKESKILLTYPE_SIZE(m_pData));
        SMMakeSkillType *pData = (SMMakeSkillType *)m_pSkillType->GetData();
        
        memcpy(pData, m_pData, MAKESKILLTYPE_SIZE(m_pData));
    }
}

void CEMakeSkillTypeDlg::DeleteSkill(CEUnitSkillType *pSkillType)
{
    // delete the skill data

    if(m_pData!=NULL) {
        delete [] (char*)m_pData;
        m_pData=NULL;
    }
}

/////////////////////////////////////////////////////////////////////////////
// CEMakeSkillTypeDlg message handlers

void CEMakeSkillTypeDlg::OnAddUnitType() 
{
    ASSERT(m_pData!=NULL);
    DWORD dwSize=MAKESKILLTYPE_SIZE(m_pData);
    
    // Prealokujeme data
    SMMakeSkillType *pTempData=(SMMakeSkillType*)new char[dwSize+sizeof(SMMakeSkillTypeRecord)];
    memcpy(pTempData, m_pData, dwSize);

    // Zjistime novy zaznam na konci
    SMMakeSkillTypeRecord *pRecord=(SMMakeSkillTypeRecord*)(((char*)pTempData)+dwSize);

    // Zvetsime pocet zaznamu
    pTempData->m_nRecords++;

    // Vynulujeme novy zaznam
    pRecord->m_dwAppearance=0;
    pRecord->m_dwUnitType=0;
    pRecord->m_nTimeToMake=0;
    
    for(int i=0; i<RESOURCE_COUNT; i++) {
        pRecord->m_ResourcesNeeded[i]=0;
    }

    CEMakeSkillTypeRecordDlg dlg(pRecord, m_pSkillType, this);

    int nRet=dlg.DoModal();

    switch(nRet) {
    case -1: 
        AfxMessageBox("Nepodaøilo se vytvoøit dialog.");
        break;
    case IDABORT:
    case IDCANCEL:
        break;
    case IDOK:
        if(pRecord->m_dwUnitType>0) {
            // Zadani se povedlo, muzeme skoncit
            // jenom zkopirujeme data
            delete [] m_pData;
            m_pData=pTempData;

            RefreshList();            
            return;
        }
        break;
    }

    // Kdyz jsme se dostali sem, znamena to, ze nove vytvorena
    // data se musi zase zrusit

    delete [] (char*)pTempData;
}

void CEMakeSkillTypeDlg::OnEditUnitType() 
{
    int selItems[1];
    VERIFY(m_wndUnitTypeList.GetSelItems(1, selItems)!=LB_ERR);
    
    ASSERT(m_pData!=NULL);
    ASSERT(m_pData->m_nRecords>selItems[0]);

    SMMakeSkillTypeRecord *pRecord=FIRST_RECORD;
    pRecord+=selItems[0];

    CEMakeSkillTypeRecordDlg dlg(pRecord, m_pSkillType, this);

    int nRet=dlg.DoModal();

    switch(nRet) {
    case -1: 
        AfxMessageBox("Nepodaøilo se vytvoøit dialog.");
        break;
    }

    RefreshList();
}

void CEMakeSkillTypeDlg::OnRemoveUnitType() 
{    

    ASSERT(m_pData!=NULL);

    int nCount=m_wndUnitTypeList.GetSelCount();
    ASSERT(nCount<=m_pData->m_nRecords);
    if(nCount==0) return;

    if(AfxMessageBox("Opravdu chcete vybrané položky odstranit?", MB_ICONEXCLAMATION | MB_YESNO) != IDYES){
        return;
    }

    CArray<int, int> aryListBoxSel;

    aryListBoxSel.SetSize(nCount);
    m_wndUnitTypeList.GetSelItems(nCount, aryListBoxSel.GetData());
    
    DWORD dwSize=sizeof(SMMakeSkillType)+(m_pData->m_nRecords-nCount)*sizeof(SMMakeSkillTypeRecord);
    
    // Prealokujeme data
    SMMakeSkillType *pTempData=(SMMakeSkillType*)new char[dwSize];

    // Nastavime spravny pocet zaznamu
    memcpy(pTempData, m_pData, sizeof(SMMakeSkillType));
    pTempData->m_nRecords=m_pData->m_nRecords-nCount;

    int i, j;
    SMMakeSkillTypeRecord *pRecord=FIRST_RECORD;
    SMMakeSkillTypeRecord *pTempRecord=(SMMakeSkillTypeRecord*)(pTempData+1);
    for(i=0; i<m_pData->m_nRecords; i++) {
        BOOL bFound=FALSE;
        for(j=0; j<nCount; j++) {
            if(aryListBoxSel[j]==i) bFound=TRUE;
        }
        if(!bFound) {
            memcpy(pTempRecord, pRecord, sizeof(SMMakeSkillTypeRecord));
            pTempRecord++;
        }
        pRecord++;
    }

    delete [] (char*)m_pData;
    m_pData=pTempData;

    RefreshList();
}

void CEMakeSkillTypeDlg::OnOK() 
{
    int i, j;
    ASSERT(m_pData!=NULL);

    SMMakeSkillTypeRecord *pRecord, *pRecord2;
    pRecord=FIRST_RECORD;
    for(i=0; i<m_pData->m_nRecords; i++) {
        pRecord2=pRecord;
        pRecord2++;
        for(j=i+1; j<m_pData->m_nRecords; j++) {
            if(pRecord->m_dwUnitType==pRecord2->m_dwUnitType) {
                MessageBox("Každý typ jednotky smí být použit jen jedenkrát.", "Špatná data");
                return;
            }
            pRecord2++;
        }
        pRecord++;
    }
       
	CDialog::OnOK();
}

BOOL CEMakeSkillTypeDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

    RefreshList();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEMakeSkillTypeDlg::OnSelchangeUnitTypeList() 
{
    UpdateButtons();
}

void CEMakeSkillTypeDlg::UpdateButtons() {
    int nSelCount=m_wndUnitTypeList.GetSelCount();

    if(nSelCount==0) {
        m_wndEditUnitType.EnableWindow(FALSE);
        m_wndRemoveUnitType.EnableWindow(FALSE);
    } else if(nSelCount==1) {
        m_wndEditUnitType.EnableWindow(TRUE);
        m_wndRemoveUnitType.EnableWindow(TRUE);
    } else if(nSelCount>1) {
        m_wndEditUnitType.EnableWindow(FALSE);
        m_wndRemoveUnitType.EnableWindow(TRUE);
    } else ASSERT(FALSE);

}

void CEMakeSkillTypeDlg::RefreshList()
{
    ASSERT(m_pData!=NULL);

    int i;
    int nCount=m_wndUnitTypeList.GetCount();
    for(i=0; i<nCount; i++) 
        m_wndUnitTypeList.DeleteString(0);

    SMMakeSkillTypeRecord *pRecord;
    pRecord=FIRST_RECORD;
    for(i=0; i<m_pData->m_nRecords; i++) {
        m_wndUnitTypeList.AddString(FormatItem(pRecord));
        pRecord++;
    }

    UpdateButtons();
}

CString CEMakeSkillTypeDlg::FormatItem(SMMakeSkillTypeRecord *pRecord)
{
    ASSERT(pRecord!=NULL);
    CString str;
    CEUnitType *pUnitType=AppGetActiveMap()->GetUnitType(pRecord->m_dwUnitType);
    if(pUnitType!=NULL) {
        str.Format(" (%d)", pRecord->m_nTimeToMake);
        str=pUnitType->GetName()+str;        
    } else {
        str="<unset unit type>";
    }
    return str;
}
