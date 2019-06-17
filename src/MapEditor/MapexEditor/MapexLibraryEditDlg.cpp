// MapexLibraryEditDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\MapEditor.h"
#include "MapexLibraryEditDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMapexLibraryEditDlg dialog


CMapexLibraryEditDlg::CMapexLibraryEditDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMapexLibraryEditDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMapexLibraryEditDlg)
	m_strFileName = _T("");
	m_strName = _T("");
	//}}AFX_DATA_INIT
}


void CMapexLibraryEditDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMapexLibraryEditDlg)
	DDX_Text(pDX, IDC_FILENAME, m_strFileName);
	DDX_Text(pDX, IDC_NAME, m_strName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMapexLibraryEditDlg, CDialog)
	//{{AFX_MSG_MAP(CMapexLibraryEditDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMapexLibraryEditDlg message handlers

void CMapexLibraryEditDlg::Create(CEMapexLibrary *pMapexLibrary)
{
  ASSERT_VALID(pMapexLibrary);
  m_pMapexLibrary = pMapexLibrary;

  m_strFileName = pMapexLibrary->GetFullPath();
  m_strName = pMapexLibrary->GetName();
}

void CMapexLibraryEditDlg::OnOK() 
{
  UpdateData(TRUE);

  m_pMapexLibrary->SetName(m_strName);  
  
	CDialog::OnOK();
}
