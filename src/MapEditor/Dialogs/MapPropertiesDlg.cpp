// MapPropertiesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\MapEditor.h"
#include "MapPropertiesDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMapPropertiesDlg dialog


CMapPropertiesDlg::CMapPropertiesDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMapPropertiesDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMapPropertiesDlg)
	m_strFileName = _T("");
	m_strMapDescription = _T("");
	m_strMapName = _T("");
	m_dwHeight = 0;
	m_dwWidth = 0;
	//}}AFX_DATA_INIT
}


void CMapPropertiesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMapPropertiesDlg)
	DDX_Text(pDX, IDC_FILENAME, m_strFileName);
	DDX_Text(pDX, IDC_MAPDESCRIPTION, m_strMapDescription);
	DDX_Text(pDX, IDC_MAPNAME, m_strMapName);
	DDX_Text(pDX, IDC_HEIGHT, m_dwHeight);
	DDX_Text(pDX, IDC_WIDTH, m_dwWidth);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMapPropertiesDlg, CDialog)
	//{{AFX_MSG_MAP(CMapPropertiesDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMapPropertiesDlg message handlers
