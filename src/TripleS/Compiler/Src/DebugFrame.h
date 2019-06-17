/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Prekladac
 *   Autor: Helena Kupková
 *  
 *   Popis: Okno pro vypis debugovacich vypisu prekladace,
 *          definovano je-li definovano makro _DEBUG a je-li
 *          nastaven priznak pro debugovani pred zacatkem
 *          kompilace (CCompiler::SetDebug()).
 * 
 ***********************************************************/

#if !defined(AFX_DEBUGFRAME_H__D795F328_33DA_11D3_AF0F_004F49068BD6__INCLUDED_)
#define AFX_DEBUGFRAME_H__D795F328_33DA_11D3_AF0F_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CRichEditView;

/////////////////////////////////////////////////////////////////////////////
// CDebugFrame frame

#ifdef _DEBUG

// class CDebugFrame
// =================
// Trida pro vypisovani debugovacich hlasek bisona & pozastavovani prekladu.
class CDebugFrame : public CFrameWnd
{
	DECLARE_DYNCREATE(CDebugFrame)
//protected:
public:
	CDebugFrame();           
	virtual ~CDebugFrame();

// Attributes
public:
    // debugovaci view bisona - RichEdit view
    CRichEditView *m_pwndDebugView;

// Operations
public:
    bool Pause();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDebugFrame)
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDebugFrame)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DEBUGFRAME_H__D795F328_33DA_11D3_AF0F_004F49068BD6__INCLUDED_)
