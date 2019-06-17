// GameClient.h : main header file for the GAMECLIENT application
//

#if !defined(AFX_GAMECLIENT_H__F8D96197_1A95_11D3_9FB6_CF2879F40338__INCLUDED_)
#define AFX_GAMECLIENT_H__F8D96197_1A95_11D3_9FB6_CF2879F40338__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

#include "GameClientApplication.h"

/////////////////////////////////////////////////////////////////////////////
// CGameClientApp:
// See GameClient.cpp for the implementation of this class
//

class CGameClientApp : public CWinApp
{
public:
	CGameClientApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGameClientApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation

public:
  CGameClientApplication *m_pApplication;

	//{{AFX_MSG(CGameClientApp)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
};


extern CString g_strStartDirectory;
/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GAMECLIENT_H__F8D96197_1A95_11D3_9FB6_CF2879F40338__INCLUDED_)
