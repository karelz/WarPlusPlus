#if !defined(AFX_EXCLUSIVEKEYBOARDWINDOW_H__249A672A_616C_4E92_93B6_707D7EF4A94F__INCLUDED_)
#define AFX_EXCLUSIVEKEYBOARDWINDOW_H__249A672A_616C_4E92_93B6_707D7EF4A94F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ExclusiveKeyboardWindow.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CExclusiveKeyboardWindow window

class CExclusiveKeyboardWindow : public CWnd
{
// Construction
public:
	CExclusiveKeyboardWindow();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CExclusiveKeyboardWindow)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CExclusiveKeyboardWindow();
	void SetSendToWindow ( CWnd * pSendToWnd ){ m_pSendToWnd = pSendToWnd; }

	// Generated message map functions
protected:
	//{{AFX_MSG(CExclusiveKeyboardWindow)
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	// Send all keyboard messages to this window
	CWnd * m_pSendToWnd;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EXCLUSIVEKEYBOARDWINDOW_H__249A672A_616C_4E92_93B6_707D7EF4A94F__INCLUDED_)
