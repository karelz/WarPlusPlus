// SendMessageDlg.cpp : implementation file
//

#include "stdafx.h"
#include "GameServerApp.h"
#include "SendMessageDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSendMessageDlg dialog


CSendMessageDlg::CSendMessageDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSendMessageDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSendMessageDlg)
	m_strMessage = _T("");
	//}}AFX_DATA_INIT
}


void CSendMessageDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSendMessageDlg)
	DDX_Text(pDX, IDC_MESSAGE, m_strMessage);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSendMessageDlg, CDialog)
	//{{AFX_MSG_MAP(CSendMessageDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSendMessageDlg message handlers

BOOL CSendMessageDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
  SetWindowText(m_strCaption);
  
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
