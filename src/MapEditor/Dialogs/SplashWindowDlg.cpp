// SplashWindowDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\MapEditor.h"
#include "SplashWindowDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSplashWindowDlg dialog


CSplashWindowDlg::CSplashWindowDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSplashWindowDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSplashWindowDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}


void CSplashWindowDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSplashWindowDlg)
	DDX_Control(pDX, IDC_SPLASHBITMAP, m_wndSplashBitmap);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSplashWindowDlg, CDialog)
	//{{AFX_MSG_MAP(CSplashWindowDlg)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSplashWindowDlg message handlers

BOOL CSplashWindowDlg::PreCreateWindow(CREATESTRUCT& cs) 
{
	return CDialog::PreCreateWindow(cs);
}

extern CString *g_strStartupDirectory;

BOOL CSplashWindowDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
  CString strTitle;
  strTitle.LoadString ( AFX_IDS_APP_TITLE );
  SetWindowText ( strTitle );
	
  CString strBmp;
  strBmp.LoadString(IDS_SPLASHBITMAP);
  strBmp = *g_strStartupDirectory + "\\" + strBmp;

  m_hBitmap = (HBITMAP)LoadImage(AfxGetInstanceHandle(), strBmp, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
  m_wndSplashBitmap.SetBitmap(m_hBitmap);
  
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSplashWindowDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	
  if(m_hBitmap != NULL){
    DeleteObject(m_hBitmap);
    m_hBitmap = NULL;
  }
}
