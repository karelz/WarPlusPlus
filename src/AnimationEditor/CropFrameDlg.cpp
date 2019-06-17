// CropFrameDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AnimationEditor.h"
#include "CropFrameDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCropFrameDlg dialog


CCropFrameDlg::CCropFrameDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCropFrameDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCropFrameDlg)
	m_dwTolerance = 0;
	//}}AFX_DATA_INIT
}


void CCropFrameDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCropFrameDlg)
	DDX_Text(pDX, IDC_TOLERANCE, m_dwTolerance);
	DDV_MinMaxDWord(pDX, m_dwTolerance, 1, 255);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCropFrameDlg, CDialog)
	//{{AFX_MSG_MAP(CCropFrameDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCropFrameDlg message handlers
