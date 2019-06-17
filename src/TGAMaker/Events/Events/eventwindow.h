#if !defined(AFX_EVENTWINDOW_H__D1F3EFD0_7693_11D2_AB4F_DED7AE979861__INCLUDED_)
#define AFX_EVENTWINDOW_H__D1F3EFD0_7693_11D2_AB4F_DED7AE979861__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// EventWindow.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEventWindow frame

// message to sen dif you want to close the window
#define WM_CLOSEWINDOW WM_USER + 0x101

class CEventWindow : public CFrameWnd
{
	DECLARE_DYNAMIC(CEventWindow)

// Attributes
public:
#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

// Operations
public:
  CEventWindow();
	virtual ~CEventWindow();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEventWindow)
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEventWindow)
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg void OnClose();
	afx_msg LONG OnCloseWindow(UINT wParam, LONG lParam);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EVENTWINDOW_H__D1F3EFD0_7693_11D2_AB4F_DED7AE979861__INCLUDED_)
