// TGAMaker.h : main header file for the TGAMAKER application
//

#if !defined(AFX_TGAMAKER_H__64AF7614_7B24_11D2_AB5A_99CC716C8761__INCLUDED_)
#define AFX_TGAMAKER_H__64AF7614_7B24_11D2_AB5A_99CC716C8761__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CTGAMakerApp:
// See TGAMaker.cpp for the implementation of this class
//

class CTGAMakerApp : public CWinApp
{
public:
	CTGAMakerApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTGAMakerApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CTGAMakerApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TGAMAKER_H__64AF7614_7B24_11D2_AB5A_99CC716C8761__INCLUDED_)
