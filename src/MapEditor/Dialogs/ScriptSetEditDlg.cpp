// ScriptSetEditDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\MapEditor.h"
#include "ScriptSetEditDlg.h"

#include "ScriptSetDlg.h"
#include "..\DataObjects\EMap.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CScriptSetEditDlg dialog


CScriptSetEditDlg::CScriptSetEditDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CScriptSetEditDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CScriptSetEditDlg)
	m_strPath = _T("");
	//}}AFX_DATA_INIT

  m_pMap = NULL;
}


void CScriptSetEditDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CScriptSetEditDlg)
	DDX_Control(pDX, IDC_EDIT, m_wndEdit);
	DDX_Control(pDX, IDC_ADD, m_wndAdd);
	DDX_Control(pDX, IDC_DELETE, m_wndDelete);
	DDX_Control(pDX, IDC_LIST, m_wndList);
	DDX_Text(pDX, IDC_PATH, m_strPath);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CScriptSetEditDlg, CDialog)
	//{{AFX_MSG_MAP(CScriptSetEditDlg)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST, OnItemchangedList)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_EDIT, OnEdit)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST, OnDblclkList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CScriptSetEditDlg::Create(CEMap *pMap)
{
  m_pMap = pMap;
}

/////////////////////////////////////////////////////////////////////////////
// CScriptSetEditDlg message handlers

BOOL CScriptSetEditDlg::OnInitDialog() 
{
  ASSERT(m_pMap != NULL);

	CDialog::OnInitDialog();

  m_wndList.InsertColumn(0, "Jméno", LVCFMT_LEFT, 150, 0);

  m_pSelected = NULL;
  UpdateList();
  UpdateSelected();
  
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CScriptSetEditDlg::UpdateList()
{
  m_pMap->FillScriptSetListCtrl(&m_wndList);
}

void CScriptSetEditDlg::UpdateSelected()
{
  int nSel;

  if(m_wndList.GetSelectedCount() == 0){
    m_wndDelete.EnableWindow(FALSE);
    m_wndEdit.EnableWindow(FALSE);
    m_strPath.Empty();
    UpdateData(FALSE);
    return;
  }
  nSel = m_wndList.GetNextItem(-1, LVNI_SELECTED);

  m_pSelected = (CEScriptSet *)m_wndList.GetItemData(nSel);
  m_wndDelete.EnableWindow(TRUE);
  m_wndEdit.EnableWindow(TRUE);
  if(m_pSelected != NULL){
    m_strPath = m_pSelected->GetPath();
  }
  else{
    m_strPath.Empty();
  }
  UpdateData(FALSE);
}

void CScriptSetEditDlg::OnItemchangedList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	
  if(pNMListView->uChanged & LVIF_STATE){
    UpdateSelected();
  }

	*pResult = 0;
}

void CScriptSetEditDlg::OnDelete() 
{
  if(m_pSelected == NULL) return;
	CString strAsk;
  strAsk.Format("Opravdu chcete smazat SkriptSet %s ?", m_pSelected->GetName());
  if(AfxMessageBox(strAsk, MB_YESNO) != IDYES) return;

  m_pMap->DeleteScriptSet(m_pSelected);

  UpdateList();
  UpdateSelected();
}

void CScriptSetEditDlg::OnAdd() 
{
  CScriptSetDlg dlg;

  dlg.m_strName.Empty();
  dlg.m_strPath.Empty();

  if(dlg.DoModal() != IDOK){
    return;
  }

  CDataArchive Archive;
  try{
    Archive.Create(dlg.m_strPath, CArchiveFile::modeRead | CFile::shareDenyWrite, CDataArchiveInfo::archiveFile);
  }
  catch(CException *e){
    e->ReportError();
    AfxMessageBox("SkriptSet nelze otevøít. Nebude vytvoøen.");
    e->Delete();
    return;
  }

  CEScriptSet *pScriptSet = new CEScriptSet();
  pScriptSet->Create(m_pMap->GetNewCivilizationID(), Archive);
  pScriptSet->SetName(dlg.m_strName);
  m_pMap->AddScriptSet(pScriptSet);

  UpdateList();
  UpdateSelected();
}

void CScriptSetEditDlg::OnEdit() 
{
  CScriptSetDlg dlg;

  if(m_pSelected == NULL) return;

  dlg.m_strName = m_pSelected->GetName();
  dlg.m_strPath = m_pSelected->GetPath();

  if(dlg.DoModal() != IDOK){
    return;
  }

  CDataArchive Archive;
  try{
    Archive.Create(dlg.m_strPath, CArchiveFile::modeRead | CFile::shareDenyWrite, CDataArchiveInfo::archiveFile);
  }
  catch(CException *e){
    e->ReportError();
    AfxMessageBox("SkriptSet nelze otevøít. Nebude vytvoøen.");
    e->Delete();
    return;
  }

  m_pSelected->SetPath(dlg.m_strPath);
  m_pSelected->SetName(dlg.m_strName);

  UpdateList();
  UpdateSelected();
}

void CScriptSetEditDlg::OnDblclkList(NMHDR* pNMHDR, LRESULT* pResult) 
{
  UpdateSelected();

  OnEdit();
  
	*pResult = 0;
}
