// OptionsGamePage.cpp : implementation file
//

#include "stdafx.h"
#include "GameServerApp.h"
#include "OptionsGamePage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionsGamePage property page

IMPLEMENT_DYNCREATE(COptionsGamePage, CPropertyPage)

COptionsGamePage::COptionsGamePage(int eGameState) : CPropertyPage(COptionsGamePage::IDD)
{
	//{{AFX_DATA_INIT(COptionsGamePage)
	m_dwTimesliceLength = 0;
	m_bShowProfiling = FALSE;
	m_bShowTrayInfos = FALSE;
	//}}AFX_DATA_INIT
  m_eGameState = eGameState;
}

COptionsGamePage::~COptionsGamePage()
{
}

void COptionsGamePage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptionsGamePage)
	DDX_Control(pDX, IDC_SHOWTRAYINFOS, m_wndShowTrayInfos);
	DDX_Text(pDX, IDC_TIMESLICE_LENGTH, m_dwTimesliceLength);
	DDV_MinMaxDWord(pDX, m_dwTimesliceLength, 1, 10000);
	DDX_Check(pDX, IDC_SHOWPROFILING, m_bShowProfiling);
	DDX_Check(pDX, IDC_SHOWTRAYINFOS, m_bShowTrayInfos);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COptionsGamePage, CPropertyPage)
	//{{AFX_MSG_MAP(COptionsGamePage)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionsGamePage message handlers

extern BOOL g_bWinVer50;

BOOL COptionsGamePage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
  if ( !g_bWinVer50 )
  {
    m_wndShowTrayInfos.EnableWindow ( FALSE );
  }
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
