// GameServerApp.h : main header file for the GAMESERVERAPP application
//

#if !defined(AFX_GAMESERVERAPP_H__8B3C855E_5C99_11D4_B527_00105ACA8325__INCLUDED_)
#define AFX_GAMESERVERAPP_H__8B3C855E_5C99_11D4_B527_00105ACA8325__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CGameServerAppApp:
// See GameServerApp.cpp for the implementation of this class
//

class CGameServerAppApp : public CWinApp
{
public:
	CGameServerAppApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGameServerAppApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CGameServerAppApp)
	afx_msg void OnAppAbout();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:

  friend class CGameServerAppDoc;
};

extern CString g_strStartupDirectory;

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GAMESERVERAPP_H__8B3C855E_5C99_11D4_B527_00105ACA8325__INCLUDED_)
