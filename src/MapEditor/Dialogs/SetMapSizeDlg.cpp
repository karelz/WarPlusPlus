// SetMapSizeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\mapeditor.h"
#include "SetMapSizeDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSetMapSizeDlg dialog


CSetMapSizeDlg::CSetMapSizeDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSetMapSizeDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSetMapSizeDlg)
	m_dwWidth = 0;
	m_dwHeight = 0;
	//}}AFX_DATA_INIT
}


void CSetMapSizeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSetMapSizeDlg)
	DDX_Text(pDX, IDC_HEIGHT, m_dwHeight);
	DDV_MinMaxDWord(pDX, m_dwHeight, 1, 100000);
	DDX_Text(pDX, IDC_WIDTH, m_dwWidth);
	DDV_MinMaxLong(pDX, m_dwWidth, 1, 100000);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSetMapSizeDlg, CDialog)
	//{{AFX_MSG_MAP(CSetMapSizeDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSetMapSizeDlg message handlers
