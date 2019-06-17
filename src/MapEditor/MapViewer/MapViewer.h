// MapViewer.h : main header file for the MAPVIEWER application
//

#if !defined(AFX_MAPVIEWER_H__018E9267_7289_11D4_AB67_004F4906CFC9__INCLUDED_)
#define AFX_MAPVIEWER_H__018E9267_7289_11D4_AB67_004F4906CFC9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

#include "MapEditor\FindPathGraphCreation\MapDescription.h"
#include <afxmt.h>

/////////////////////////////////////////////////////////////////////////////
// CMapViewerApp:
// See MapViewer.cpp for the implementation of this class
//

class CMapViewerApp : public CWinApp
{
public:
	CMapViewerApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMapViewerApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CMapViewerApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	// zámek sdílené pamìti
	CMutex m_mutexSharedMemoryLock;

	// mapovaný soubor mapy
	HANDLE m_hMap;
	// mapovaný soubor popisu mapy
	HANDLE m_hMapDescription;

	// popis mapy
	struct SMapDescription *m_pMapDescription;
	// mapa
	signed char *m_pMap;
};

extern CMapViewerApp theApp;

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAPVIEWER_H__018E9267_7289_11D4_AB67_004F4906CFC9__INCLUDED_)
