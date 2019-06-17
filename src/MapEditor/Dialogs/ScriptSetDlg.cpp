// ScriptSetDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\MapEditor.h"
#include "ScriptSetDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CScriptSetDlg dialog


CScriptSetDlg::CScriptSetDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CScriptSetDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CScriptSetDlg)
	m_strName = _T("");
	m_strPath = _T("");
	//}}AFX_DATA_INIT
}


void CScriptSetDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CScriptSetDlg)
	DDX_Text(pDX, IDC_NAME, m_strName);
	DDX_Text(pDX, IDC_PATH, m_strPath);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CScriptSetDlg, CDialog)
	//{{AFX_MSG_MAP(CScriptSetDlg)
	ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CScriptSetDlg message handlers

void CScriptSetDlg::OnBrowse() 
{
  UpdateData(TRUE);

  CFileDialog dlg(FALSE, "ScriptSet", m_strPath, OFN_HIDEREADONLY, "SkriptSety (*.ScriptSet)|*.ScriptSet|Všechny soubory (*.*)|*.*||", this);
  dlg.m_ofn.lpstrTitle = "SkriptSet";

  if(dlg.DoModal() == IDOK){
    m_strPath = dlg.GetPathName();
    UpdateData(FALSE);
  }
}
