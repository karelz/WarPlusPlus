// ScriptSetProgressDlg.cpp : implementation file
//

#include "stdafx.h"
#include "GameServerApp.h"
#include "ScriptSetProgressDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CScriptSetProgressDlg dialog


CScriptSetProgressDlg::CScriptSetProgressDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CScriptSetProgressDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CScriptSetProgressDlg)
	m_strStatus = _T("");
	//}}AFX_DATA_INIT
  m_bSuccess = false;
}


void CScriptSetProgressDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CScriptSetProgressDlg)
	DDX_Control(pDX, IDC_ERROROUTPUT, m_wndErrorOutput);
	DDX_Control(pDX, IDCANCEL, m_wndOK);
	DDX_Text(pDX, IDC_STATUS, m_strStatus);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CScriptSetProgressDlg, CDialog)
	//{{AFX_MSG_MAP(CScriptSetProgressDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CScriptSetProgressDlg message handlers

void CScriptSetProgressDlg::OnCancel() 
{
  if(m_bProgress) return;
  
  m_pErrOutput->SetEdit(NULL);

	CDialog::OnCancel();
}

BOOL CScriptSetProgressDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
  m_bProgress = TRUE;
  
  m_wndOK.EnableWindow(FALSE);
  m_pErrOutput->SetEdit(&m_wndErrorOutput);

  ShowWindow(SW_SHOW);
  UpdateWindow();

  m_bSuccess = m_pScriptSetMaker->Compile( m_pErrOutput );

  if(m_bSuccess){
    m_strStatus.LoadString(IDS_SCRIPTSETPROGRESS_SUCCESS);
  }
  else{
    m_strStatus.LoadString(IDS_SCRIPTSETPROGRESS_ERROR);
  }
  m_bProgress = FALSE;

  m_wndOK.EnableWindow(TRUE);

  UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
