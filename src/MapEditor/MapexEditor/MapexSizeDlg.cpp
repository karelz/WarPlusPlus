// MapexSizeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\MapEditor.h"
#include "MapexSizeDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMapexSizeDlg dialog


CMapexSizeDlg::CMapexSizeDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMapexSizeDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMapexSizeDlg)
	m_dwHeight = 0;
	m_dwWidth = 0;
	//}}AFX_DATA_INIT
}


void CMapexSizeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMapexSizeDlg)
	DDX_Text(pDX, IDC_HEIGHT, m_dwHeight);
	DDV_MinMaxDWord(pDX, m_dwHeight, 1, 500);
	DDX_Text(pDX, IDC_WIDTH, m_dwWidth);
	DDV_MinMaxDWord(pDX, m_dwWidth, 1, 500);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMapexSizeDlg, CDialog)
	//{{AFX_MSG_MAP(CMapexSizeDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMapexSizeDlg message handlers
