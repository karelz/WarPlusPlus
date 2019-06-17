#if !defined(AFX_HELPBROWSERWINDOW_H__AC9A7047_7BFD_11D4_B0F0_004F49068BD6__INCLUDED_)
#define AFX_HELPBROWSERWINDOW_H__AC9A7047_7BFD_11D4_B0F0_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// HelpBrowserWindow.h : header file
//

#include "WebBrowser2.h"

/////////////////////////////////////////////////////////////////////////////
// CHelpBrowserWindow window

class CHelpBrowser;

class CHelpBrowserWindow : public CWnd
{
// Construction
public:
	CHelpBrowserWindow();

// Attributes
public:

// Operations
public:
  // The web browser control
  CWebBrowser2 *m_pWebBrowser;
  CHelpBrowser *m_pHelpBrowser;

  void SetURL(CString strURL);
  void SetHomeURL(CString strURL){ m_strHomeURL = strURL; }
  CString GetHomeURL(){ return m_strHomeURL; }

  CString GetTitle();
  BOOL IsAnchor(CPoint pt);

  void GoHome();
  void GoBack();
  void GoForward();
  void Refresh();

  enum{
    E_TitleChange = 100,
    E_DocumentComplete = 101,
    E_NavigateBefore = 102,
  };
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHelpBrowserWindow)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CHelpBrowserWindow();

  CRect GetPageSize(){ return m_rcPage; }
  CRect GetWindowSize(){ return m_rcWindow; }
	// Generated message map functions
protected:
	//{{AFX_MSG(CHelpBrowserWindow)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnDocumentComplete(LPDISPATCH pDisp, VARIANT FAR* URL);
	afx_msg void OnTitleChange(LPCTSTR Text);
	afx_msg void OnBeforeNavigate(LPDISPATCH pDisp, VARIANT FAR* URL, VARIANT FAR* Flags, VARIANT FAR* TargetFrameName, VARIANT FAR* PostData, VARIANT FAR* Headers, BOOL FAR* Cancel);
  afx_msg LRESULT OnHelpBrowserAction(WPARAM wParam, LPARAM lParam);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()

  CRect m_rcPage;
  CRect m_rcWindow;

  CString m_strHomeURL;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HELPBROWSERWINDOW_H__AC9A7047_7BFD_11D4_B0F0_004F49068BD6__INCLUDED_)
