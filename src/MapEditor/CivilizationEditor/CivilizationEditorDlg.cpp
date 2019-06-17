// CivilizationEditorDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\MapEditor.h"
#include "CivilizationEditorDlg.h"

#include "CivilizationDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCivilizationEditorDlg dialog


CCivilizationEditorDlg::CCivilizationEditorDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCivilizationEditorDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCivilizationEditorDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

  m_pMap = NULL;
}


void CCivilizationEditorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCivilizationEditorDlg)
	DDX_Control(pDX, IDC_EDIT, m_wndEdit);
	DDX_Control(pDX, IDC_DELETE, m_wndDelete);
	DDX_Control(pDX, IDC_LIST, m_wndList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCivilizationEditorDlg, CDialog)
	//{{AFX_MSG_MAP(CCivilizationEditorDlg)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	ON_BN_CLICKED(IDC_EDIT, OnEdit)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST, OnDblclkList)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST, OnItemChangedList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCivilizationEditorDlg message handlers

void CCivilizationEditorDlg::Create(CEMap *pMap)
{
  m_pMap = pMap;
}

BOOL CCivilizationEditorDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
  ASSERT(m_pMap != NULL);

  m_wndList.InsertColumn(0, "", LVCFMT_LEFT, 120);
  m_wndList.SetExtendedStyle(LVS_EX_FULLROWSELECT);
  m_pMap->FillCivilizationsListCtrl(&m_wndList);
  UpdateSelectedCivilization();
  
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CCivilizationEditorDlg::OnAdd() 
{
  CECivilization *pCiv;
  int aResources[RESOURCE_COUNT];
  memset(aResources, 0, RESOURCE_COUNT * sizeof(int));
  
  pCiv = new CECivilization();

  CCivilizationDlg dlg;
  dlg.Create(m_pMap, pCiv);
  dlg.m_dwColor = RGB32(255, 0, 0);
  dlg.m_strName = "Nová civilizace";
  dlg.m_pResources = aResources;
  dlg.m_pScriptSet = NULL;
  if(dlg.DoModal() != IDOK){
    delete pCiv;
    return;
  }

  pCiv->Create(m_pMap->GetNewCivilizationID(), dlg.m_strName, dlg.m_dwColor);
  pCiv->SetScriptSet(dlg.m_pScriptSet);
  pCiv->SetResources(aResources);
  if(!IsCivUnique(pCiv)){
    AfxMessageBox("Civilizace koliduje s jinou (musi se jmenovat jinak, a mit jinou barvu)");
    OnEdit();
  }
  m_pMap->AddCivilization(pCiv);
  m_pMap->FillCivilizationsListCtrl(&m_wndList);
  UpdateSelectedCivilization();
}

void CCivilizationEditorDlg::OnDelete() 
{
	if(m_wndList.GetItemCount() == 0) return;
  int nSel = m_wndList.GetNextItem(-1, LVNI_SELECTED);
  CECivilization *pCiv = (CECivilization *)m_wndList.GetItemData(nSel);

  CString str;
  str.Format("Opravdu chcete smazat civilizaci '%s'?", pCiv->GetName());
  if(AfxMessageBox(str, MB_YESNO) == IDNO)
    return;

  m_pMap->DeleteCivilization(pCiv);
  m_pMap->FillCivilizationsListCtrl(&m_wndList);
  UpdateSelectedCivilization();
}

void CCivilizationEditorDlg::OnEdit() 
{
	if(m_wndList.GetItemCount() == 0) return;
  int nSel = m_wndList.GetNextItem(-1, LVNI_SELECTED);
  if(nSel == -1) return;
  CECivilization *pCiv = (CECivilization *)m_wndList.GetItemData(nSel);

  CCivilizationDlg dlg;
  
  dlg.Create(m_pMap, pCiv);

  dlg.m_dwColor = pCiv->GetColor();
  dlg.m_pResources = pCiv->GetResources();
  dlg.m_strName = pCiv->GetName();
  dlg.m_pScriptSet = pCiv->GetScriptSet();
RetryIt:;
  if(dlg.DoModal() == IDOK){
    if(dlg.m_strName.GetLength() == 0){
      AfxMessageBox("Musíte zadat jméno civilizace.");
      goto RetryIt;
    }
    pCiv->SetColor(dlg.m_dwColor);
    pCiv->SetName(dlg.m_strName);
    pCiv->SetScriptSet(dlg.m_pScriptSet);
    if(!IsCivUnique(pCiv)){
      AfxMessageBox("Civilizace koliduje s jinou (musi se jmenovat jinak, a mit jinou barvu)");
      goto RetryIt;
    }

    m_pMap->FillCivilizationsListCtrl(&m_wndList);
    UpdateSelectedCivilization();
  }
  if(!IsCivUnique(pCiv)){
    AfxMessageBox("Civilizace koliduje s jinou (musi se jmenovat jinak, a mit jinou barvu)");
    goto RetryIt;
  }
}

void CCivilizationEditorDlg::OnDblclkList(NMHDR* pNMHDR, LRESULT* pResult) 
{
  OnEdit();
  
	*pResult = 0;
}

void CCivilizationEditorDlg::OnItemChangedList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

  if(pNMListView->uChanged & LVIF_STATE)
    UpdateSelectedCivilization();
  
	*pResult = 0;
}

void CCivilizationEditorDlg::UpdateSelectedCivilization()
{
  if(m_wndList.GetItemCount() == 0){
    m_wndDelete.EnableWindow(FALSE);
    m_wndEdit.EnableWindow(FALSE);
    return;
  }
  int nSel = m_wndList.GetNextItem(-1, LVNI_SELECTED);
  if(nSel == -1){
    m_wndDelete.EnableWindow(FALSE);
    m_wndEdit.EnableWindow(FALSE);
    return;
  }
  CECivilization *pCiv = (CECivilization *)m_wndList.GetItemData(nSel);
  if(pCiv->GetID() == 0){
    m_wndDelete.EnableWindow(FALSE);
  }
  else{
    m_wndDelete.EnableWindow(TRUE);
  }
  m_wndEdit.EnableWindow(TRUE);
}

BOOL CCivilizationEditorDlg::IsCivUnique(CECivilization *pCiv)
{
  ASSERT_VALID(m_pMap);

  return m_pMap->IsCivUnique(pCiv);
}