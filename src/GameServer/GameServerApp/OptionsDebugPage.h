#if !defined(AFX_OPTIONSDEBUGPAGE_H__FE9BFEA4_6DDB_11D4_B0D1_004F49068BD6__INCLUDED_)
#define AFX_OPTIONSDEBUGPAGE_H__FE9BFEA4_6DDB_11D4_B0D1_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptionsDebugPage.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COptionsDebugPage dialog

class COptionsDebugPage : public CPropertyPage
{
	DECLARE_DYNCREATE(COptionsDebugPage)

// Construction
public:
  COptionsDebugPage(){};
	COptionsDebugPage(int eGameState);
	~COptionsDebugPage();

// Dialog Data
	//{{AFX_DATA(COptionsDebugPage)
	enum { IDD = IDD_OPTIONS_DEBUG };
	CButton	m_wndUDPAlive;
	BOOL	m_bDeleteWorkingDirectoryOnCreate;
	BOOL	m_bDeleteWorkingDirectoryOnDelete;
	BOOL	m_bTraceClientRequests;
	BOOL	m_bTraceInterpret;
	BOOL	m_bTraceInterpretDetails;
	BOOL    m_nTraceCompiledCode;
	BOOL	m_bTraceNotifications;
	BOOL	m_bTraceServerClientCommunication;
	BOOL	m_bTraceServerLoops;
	BOOL	m_bTraceSkillCalls;
	BOOL	m_bTraceSkills;
	BOOL	m_bTraceErrorLogs;
	BOOL	m_bTraceNetwork;
	BOOL	m_bUDPAlive;
	BOOL	m_bTraceCompiledCode;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(COptionsDebugPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(COptionsDebugPage)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

  int m_eGameState;

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPTIONSDEBUGPAGE_H__FE9BFEA4_6DDB_11D4_B0D1_004F49068BD6__INCLUDED_)
