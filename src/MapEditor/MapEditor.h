// MapEditor.h : main header file for the MAPEDITOR application
//

#if !defined(AFX_MAPEDITOR_H__43E086C7_6434_11D3_A054_ADE3A89A0638__INCLUDED_)
#define AFX_MAPEDITOR_H__43E086C7_6434_11D3_A054_ADE3A89A0638__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CMapEditorApp:
// See MapEditor.cpp for the implementation of this class
//

class CMapEditorApp : public CWinApp
{
public:
	CMapEditorApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMapEditorApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	virtual BOOL OnIdle(LONG lCount);
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CMapEditorApp)
	afx_msg void OnAppAbout();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	CEventManager m_EventManager;
	CDDPrimarySurface m_DDPrimarySurface;
};

class CMapEditorDoc;
CMapEditorDoc *AppGetActiveDocument();

class CEMap;
CEMap *AppGetActiveMap();
void AppSetActiveMap(CEMap *pMap);

extern BOOL bMainFrameCreated;

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAPEDITOR_H__43E086C7_6434_11D3_A054_ADE3A89A0638__INCLUDED_)
