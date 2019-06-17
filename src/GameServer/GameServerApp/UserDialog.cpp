// UserDialog.cpp : implementation file
//

#include "stdafx.h"
#include "GameServerApp.h"
#include "UserDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CUserDialog dialog


CUserDialog::CUserDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CUserDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CUserDialog)
	m_strComment = _T("");
	m_strName = _T("");
	m_strPassword = _T("");
	m_strPassword2 = _T("");
	//}}AFX_DATA_INIT

  m_bEnableName = TRUE;
}


void CUserDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CUserDialog)
	DDX_Control(pDX, IDC_NAME, m_wndName);
	DDX_Text(pDX, IDC_COMMENT, m_strComment);
	DDX_Text(pDX, IDC_NAME, m_strName);
	DDX_Text(pDX, IDC_PASSWORD, m_strPassword);
	DDX_Text(pDX, IDC_PASSWORD2, m_strPassword2);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CUserDialog, CDialog)
	//{{AFX_MSG_MAP(CUserDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUserDialog message handlers

void CUserDialog::OnOK() 
{
  UpdateData(TRUE);

  if(m_strPassword.Compare(m_strPassword2) != 0){
    AfxMessageBox(IDS_USER_PASSWORDSNOTMATCH);
    return;
  }
  
	CDialog::OnOK();
}

BOOL CUserDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
  if(!m_bEnableName){
    m_wndName.EnableWindow(FALSE);
  }
  
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
