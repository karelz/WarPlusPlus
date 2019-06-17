// UnitLibraryEditorDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\MapEditor.h"
#include "UnitLibraryEditorDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CUnitLibraryEditorDlg dialog


CUnitLibraryEditorDlg::CUnitLibraryEditorDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CUnitLibraryEditorDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CUnitLibraryEditorDlg)
	m_strFileName = _T("");
	m_strName = _T("");
	//}}AFX_DATA_INIT
}


void CUnitLibraryEditorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CUnitLibraryEditorDlg)
	DDX_Text(pDX, IDC_FILENAME, m_strFileName);
	DDX_Text(pDX, IDC_NAME, m_strName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CUnitLibraryEditorDlg, CDialog)
	//{{AFX_MSG_MAP(CUnitLibraryEditorDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUnitLibraryEditorDlg message handlers

BOOL CUnitLibraryEditorDlg::Create(CEUnitLibrary *pLibrary)
{ 
  ASSERT_VALID(pLibrary);

  m_pUnitLibrary = pLibrary;
  m_strFileName = pLibrary->GetFullPath();
  m_strName = pLibrary->GetName();
  return TRUE;
}


void CUnitLibraryEditorDlg::OnOK() 
{
  UpdateData(TRUE);

  m_pUnitLibrary->SetName(m_strName);  
  
	CDialog::OnOK();
}
