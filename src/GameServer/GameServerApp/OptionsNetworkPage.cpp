// OptionsNetworkPage.cpp : implementation file
//

#include "stdafx.h"
#include "GameServerApp.h"
#include "OptionsNetworkPage.h"

#include "GameServerAppDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionsNetworkPage property page

IMPLEMENT_DYNCREATE(COptionsNetworkPage, CPropertyPage)

COptionsNetworkPage::COptionsNetworkPage(int eGameState) : CPropertyPage(COptionsNetworkPage::IDD)
{
	//{{AFX_DATA_INIT(COptionsNetworkPage)
	m_dwPort = 0;
	//}}AFX_DATA_INIT
  m_eGameState = eGameState;
}

COptionsNetworkPage::~COptionsNetworkPage()
{
}

void COptionsNetworkPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptionsNetworkPage)
	DDX_Control(pDX, IDC_PORT, m_wndPort);
	DDX_Text(pDX, IDC_PORT, m_dwPort);
	DDV_MinMaxDWord(pDX, m_dwPort, 0, 65535);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COptionsNetworkPage, CPropertyPage)
	//{{AFX_MSG_MAP(COptionsNetworkPage)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionsNetworkPage message handlers

BOOL COptionsNetworkPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
  // If the game is already initialized -> can't change some network settings
  if(m_eGameState != CGameServerAppDoc::Game_None){
    m_wndPort.EnableWindow(FALSE);
  }
  
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
