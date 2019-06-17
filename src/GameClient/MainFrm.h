// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__F8D9619B_1A95_11D3_9FB6_CF2879F40338__INCLUDED_)
#define AFX_MAINFRM_H__F8D9619B_1A95_11D3_9FB6_CF2879F40338__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ExclusiveKeyboardWindow.h"

#define WM_MAINFRAME_SETFOCUSTOOURWINDOW  WM_USER + 100

class CMainFrame : public CWindowsWindow
{
	
public:
	CMainFrame();
protected: 
	DECLARE_DYNAMIC(CMainFrame)

// Attributes
public:

// Operations
public:

  void SetFocusToOurWindow () { if (::IsWindow(m_wndExclusiveKeyboard.GetSafeHwnd())) m_wndExclusiveKeyboard.SetFocus (); }
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	void SetExclusive(BOOL bExclusive);
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg LRESULT OnSetFocusToOurWindow ( WPARAM, LPARAM );
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	BOOL m_bExclusive;
	CExclusiveKeyboardWindow m_wndExclusiveKeyboard;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__F8D9619B_1A95_11D3_9FB6_CF2879F40338__INCLUDED_)
