#if !defined(AFX_FINISHDLG_H__8BE7C1D4_1BA0_11D4_84B1_004F4E0004AA__INCLUDED_)
#define AFX_FINISHDLG_H__8BE7C1D4_1BA0_11D4_84B1_004F4E0004AA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FinishDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFinishDlg dialog

#define WM_SETFINISHTEXT WM_USER + 1
#define WM_SETFINISHPOS WM_USER + 2

class CFinishDlg : public CDialog
{
// Construction
public:
	void SetProgress(UINT nPercent);
	void SetText(CString strText);
	CFinishDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CFinishDlg)
	enum { IDD = IDD_FINISH_PROGRESS };
	CProgressCtrl	m_wndProgress;
	CString	m_strProcessing;
	//}}AFX_DATA

  afx_msg LRESULT OnSetText(WPARAM wParam, LPARAM lParam);
  afx_msg LRESULT OnSetPos(WPARAM wParam, LPARAM lParam);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFinishDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFinishDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FINISHDLG_H__8BE7C1D4_1BA0_11D4_84B1_004F4E0004AA__INCLUDED_)
