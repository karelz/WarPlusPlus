// ScriptSetMakerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "GameServerApp.h"
#include "ScriptSetMakerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CScriptSetMakerDlg dialog

CScriptSetMakerDlg::CScriptSetMakerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CScriptSetMakerDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CScriptSetMakerDlg)
	m_strDestination = _T("");
	m_strSource = _T("");
	//}}AFX_DATA_INIT
}


void CScriptSetMakerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CScriptSetMakerDlg)
	DDX_Text(pDX, IDC_DESTINATION, m_strDestination);
	DDX_Text(pDX, IDC_SOURCE, m_strSource);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CScriptSetMakerDlg, CDialog)
	//{{AFX_MSG_MAP(CScriptSetMakerDlg)
	ON_BN_CLICKED(IDC_BROWSEDESTINATION, OnBrowseDestination)
	ON_BN_CLICKED(IDC_BROWSESOURCE, OnBrowseSource)
	ON_BN_CLICKED(IDC_PROCESS, OnProcess)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CScriptSetMakerDlg message handlers

BOOL CScriptSetMakerDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CWinApp *pApp = AfxGetApp();
	m_strSource = pApp->GetProfileString("Scriptsets", "Source Directory", "");
	m_strDestination = pApp->GetProfileString("Scriptsets", "Scriptset", "");
	
	UpdateData( FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CScriptSetMakerDlg::OnCancel() 
{
	UpdateData(TRUE);

	CDialog::OnCancel();
}

void CScriptSetMakerDlg::OnBrowseDestination() 
{
  UpdateData(TRUE);

  CString strFilter;
  strFilter.LoadString(IDS_SCRIPTSETMAKER_DESTFILTER);
  CFileDialog dlg(FALSE, "*.ScriptSet", m_strDestination, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
    strFilter, this);
  CString strTitle;
  strTitle.LoadString(IDS_SCRIPTSETMAKER_DESTTITLE);
  dlg.m_ofn.lpstrTitle = strTitle;

  if(dlg.DoModal() != IDOK){
    return;
  }

  m_strDestination = dlg.GetPathName();
  UpdateData(FALSE);
}

void CScriptSetMakerDlg::OnBrowseSource() 
{
  UpdateData(TRUE);

  char pDir[MAX_PATH];
  pDir[0] = 0;
  BROWSEINFO sBrowseInfo;
  CString strTitle;
  strTitle.LoadString(IDS_SCRIPTSETMAKER_SOURCETITLE);

  strcpy(pDir, m_strSource);

  // Fill the browse info structure
  sBrowseInfo.hwndOwner = (HWND)GetSafeHwnd();
  sBrowseInfo.pidlRoot = NULL;
  sBrowseInfo.pszDisplayName = pDir;
  sBrowseInfo.lpszTitle = strTitle;
  sBrowseInfo.ulFlags = BIF_RETURNONLYFSDIRS;
  sBrowseInfo.lpfn = NULL;
  sBrowseInfo.lParam = 0;

  LPITEMIDLIST lpItemID;

  // Browse for it
  if((lpItemID = SHBrowseForFolder(&sBrowseInfo)) != NULL){
    // Translate the ListID to path
    SHGetPathFromIDList(lpItemID, pDir);
    // Write the path to the edit box
    m_strSource = pDir;
    UpdateData(FALSE);
  }
}

void CScriptSetMakerDlg::OnProcess() 
{
  UpdateData(TRUE);

  CWinApp *pApp = AfxGetApp();
  pApp->WriteProfileString("Scriptsets", "Source Directory", m_strSource);
  pApp->WriteProfileString("Scriptsets", "Scriptset", m_strDestination);

  EndDialog(IDOK);
}
