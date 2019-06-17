// OptionsDebugPage.cpp : implementation file
//

#include "stdafx.h"
#include "GameServerApp.h"
#include "OptionsDebugPage.h"

#include "GameServerAppDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionsDebugPage property page

IMPLEMENT_DYNCREATE(COptionsDebugPage, CPropertyPage)

COptionsDebugPage::COptionsDebugPage(int eGameState) : CPropertyPage(COptionsDebugPage::IDD)
{
	//{{AFX_DATA_INIT(COptionsDebugPage)
	m_bDeleteWorkingDirectoryOnCreate = FALSE;
	m_bDeleteWorkingDirectoryOnDelete = FALSE;
	m_bTraceClientRequests = FALSE;
	m_bTraceInterpret = FALSE;
	m_bTraceInterpretDetails = FALSE;
	m_bTraceNotifications = FALSE;
	m_bTraceServerClientCommunication = FALSE;
	m_bTraceServerLoops = FALSE;
	m_bTraceSkillCalls = FALSE;
	m_bTraceSkills = FALSE;
	m_bTraceErrorLogs = FALSE;
	m_bTraceNetwork = FALSE;
	m_bUDPAlive = FALSE;
	m_bTraceCompiledCode = FALSE;
	//}}AFX_DATA_INIT

  m_eGameState = eGameState;
}

COptionsDebugPage::~COptionsDebugPage()
{
}

void COptionsDebugPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptionsDebugPage)
	DDX_Control(pDX, IDC_UDPALIVE, m_wndUDPAlive);
	DDX_Check(pDX, IDC_DELETE_WORKING_DIRECTORY_ON_CREATE, m_bDeleteWorkingDirectoryOnCreate);
	DDX_Check(pDX, IDC_DELETE_WORKING_DIRECTORY_ON_DELETE, m_bDeleteWorkingDirectoryOnDelete);
	DDX_Check(pDX, IDC_TRACE_CLIENT_REQUESTS, m_bTraceClientRequests);
	DDX_Check(pDX, IDC_TRACE_INTERPRET, m_bTraceInterpret);
	DDX_Check(pDX, IDC_TRACE_INTERPRET_DETAILS, m_bTraceInterpretDetails);
	DDX_Check(pDX, IDC_TRACE_NOTIFICATIONS, m_bTraceNotifications);
	DDX_Check(pDX, IDC_TRACE_SERVER_CLIENT_COMMUNICATION, m_bTraceServerClientCommunication);
	DDX_Check(pDX, IDC_TRACE_SERVER_LOOPS, m_bTraceServerLoops);
	DDX_Check(pDX, IDC_TRACE_SKILL_CALLS, m_bTraceSkillCalls);
	DDX_Check(pDX, IDC_TRACE_SKILLS, m_bTraceSkills);
	DDX_Check(pDX, IDC_TRACE_ERROR_LOGS, m_bTraceErrorLogs);
	DDX_Check(pDX, IDC_TRACE_NETWORK, m_bTraceNetwork);
	DDX_Check(pDX, IDC_UDPALIVE, m_bUDPAlive);
	DDX_Check(pDX, IDC_TRACE_COMPILED_CODE, m_bTraceCompiledCode);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COptionsDebugPage, CPropertyPage)
	//{{AFX_MSG_MAP(COptionsDebugPage)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionsDebugPage message handlers

BOOL COptionsDebugPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
    if(m_eGameState == CGameServerAppDoc::Game_Unpaused){
        m_wndUDPAlive.EnableWindow(FALSE);
    }
    
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
