// FNTMaker.h : main header file for the FNTMAKER application
//

#if !defined(AFX_FNTMAKER_H__C6DEDCE5_87AC_11D2_AB85_85B649344A60__INCLUDED_)
#define AFX_FNTMAKER_H__C6DEDCE5_87AC_11D2_AB85_85B649344A60__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CFNTMakerApp:
// See FNTMaker.cpp for the implementation of this class
//

class CFNTMakerApp : public CWinApp
{
public:
	CFNTMakerApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFNTMakerApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CFNTMakerApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FNTMAKER_H__C6DEDCE5_87AC_11D2_AB85_85B649344A60__INCLUDED_)
