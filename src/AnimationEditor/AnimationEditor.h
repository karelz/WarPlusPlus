// AnimationEditor.h : main header file for the ANIMATIONEDITOR application
//

#if !defined(AFX_ANIMATIONEDITOR_H__871D5314_247A_48F1_99FB_CD2CEFBBD154__INCLUDED_)
#define AFX_ANIMATIONEDITOR_H__871D5314_247A_48F1_99FB_CD2CEFBBD154__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CAnimationEditorApp:
// See AnimationEditor.cpp for the implementation of this class
//

class CAnimationEditorApp : public CWinApp
{
public:
	CAnimationEditorApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAnimationEditorApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CAnimationEditorApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
  // Some dummy event manager
  CEventManager m_cEventManager;
  // Primary surface
  CDDPrimarySurface m_cPrimarySurface;
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ANIMATIONEDITOR_H__871D5314_247A_48F1_99FB_CD2CEFBBD154__INCLUDED_)
