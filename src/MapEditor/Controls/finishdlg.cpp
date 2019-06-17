// FinishDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\MapEditor.h"
#include "FinishDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFinishDlg dialog


CFinishDlg::CFinishDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFinishDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFinishDlg)
	m_strProcessing = _T("");
	//}}AFX_DATA_INIT
}


void CFinishDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFinishDlg)
	DDX_Control(pDX, IDC_PROGRESS, m_wndProgress);
	DDX_Text(pDX, IDC_PROCESSING, m_strProcessing);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFinishDlg, CDialog)
	//{{AFX_MSG_MAP(CFinishDlg)
  ON_MESSAGE(WM_SETFINISHTEXT, OnSetText)
  ON_MESSAGE(WM_SETFINISHPOS, OnSetPos)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFinishDlg message handlers

void CFinishDlg::SetText(CString strText)
{
  m_strProcessing = strText;
  UpdateData(FALSE);
  UpdateWindow();
}

void CFinishDlg::SetProgress(UINT nPercent)
{
  m_wndProgress.SetPos(nPercent);
}

BOOL CFinishDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

//  SetWindowPos(&wndTopMost, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
	
  m_wndProgress.SetRange(0, 100);
  
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

LRESULT CFinishDlg::OnSetText(WPARAM wParam ,LPARAM lParam)
{
  LPCSTR pText = (LPCSTR)lParam;

  SetText(pText);

  return 0;
}

LRESULT CFinishDlg::OnSetPos(WPARAM wParam , LPARAM lParam)
{
  SetProgress(lParam);
  return 0;
}