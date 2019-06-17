#if !defined(AFX_MOUSEWINDOW_H__576EC744_5AF9_11D2_8EB5_947204C10000__INCLUDED_)
#define AFX_MOUSEWINDOW_H__576EC744_5AF9_11D2_8EB5_947204C10000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// MouseWindow.h : header file
//

#include "..\Keyboard\KeyboardWindow.h"

/////////////////////////////////////////////////////////////////////////////
// CMouseWindow window

class CMouse;

class CMouseWindow : public CKeyboardWindow
{
  DECLARE_DYNAMIC(CMouseWindow);

// Construction
public:
	CMouseWindow();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMouseWindow)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMouseWindow();

#ifdef _DEBUG
  virtual void Dump(CDumpContext & dc) const;
  virtual void AssertValid() const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CMouseWindow)
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg LONG OnDIMouseAcquire(UINT wParam, LONG lParam);
	afx_msg LONG OnExitSizeMove(UINT wParam, LONG lParam);
	afx_msg LONG OnExitMenuLoop(UINT wParam, LONG lParam);
	afx_msg LONG OnEnterSizeMove(UINT wParam, LONG lParam);
	afx_msg LONG OnEnterMenuLoop(UINT wParam, LONG lParam);
  afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
  afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
  afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
  afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
  afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
  afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
  afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnCaptureChanged(CWnd *pWnd);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
  // sends acquire message
	void SendAcquire(BOOL bActive);
  // do acquire or unacquire
	void SyncAcquire();
  // if TRUE the window is active
	BOOL m_bActive;
  // pointer to a Mouse object that has created us
	CMouse * m_pMouse;

  friend class CMouse;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MOUSEWINDOW_H__576EC744_5AF9_11D2_8EB5_947204C10000__INCLUDED_)
