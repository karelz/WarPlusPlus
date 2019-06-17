#if !defined(AFX_WINDOWSWINDOW_H__6DFDFC52_848A_11D2_AB79_8D599BDA9961__INCLUDED_)
#define AFX_WINDOWSWINDOW_H__6DFDFC52_848A_11D2_AB79_8D599BDA9961__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// WindowsWindow.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CWindowsWindow frame

#include "..\Mouse\MouseWindow.h"

class CWindowsWindow : public CMouseWindow
{
	DECLARE_DYNCREATE(CWindowsWindow)

protected:
  // construtor
  CWindowsWindow();           // protected constructor used by dynamic creation

public:
#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWindowsWindow)
	//}}AFX_VIRTUAL

// Implementation
public:
	// returns TRUE if the application (and its main window) is active
  BOOL IsActive();
  // destructor
  virtual ~CWindowsWindow();

  void LockActivation();
  void UnlockActivation();

  // Waits until the window is active
  void WaitForActive(){ VERIFY(m_eventActive.Lock()); }
  // Returns handle to object to wait for the activation
  HANDLE GetWaitableForActive(){ return m_eventActive.m_hObject; }

protected:
  // Generated message map functions
	//{{AFX_MSG(CWindowsWindow)
	afx_msg void OnPaint();
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg LONG OnEnterMenuLoop(UINT wParam, LONG lParam);
	afx_msg LONG OnExitSizeMove(UINT wParam, LONG lParam);
	afx_msg LONG OnExitMenuLoop(UINT wParam, LONG lParam);
	afx_msg LONG OnEnterSizeMove(UINT wParam, LONG lParam);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg BOOL OnNcActivate(BOOL bActive);
	afx_msg void OnNcPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
  // TRUE if the application is active
	BOOL m_bActive;
  // Lock for access to the m_bActive member
  CMutex m_lockActive;

  // Manual event which is set when the window is active
  CEvent m_eventActive;

  void SetActive(BOOL bActive);
};

// global pointer to the main app window
// with this you can determine if the application is active
extern CWindowsWindow *g_pMainWindow;

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WINDOWSWINDOW_H__6DFDFC52_848A_11D2_AB79_8D599BDA9961__INCLUDED_)
