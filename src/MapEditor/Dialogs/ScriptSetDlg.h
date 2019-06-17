#if !defined(AFX_SCRIPTSETDLG_H__1C8EC1B7_79AB_11D4_B0ED_004F49068BD6__INCLUDED_)
#define AFX_SCRIPTSETDLG_H__1C8EC1B7_79AB_11D4_B0ED_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ScriptSetDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CScriptSetDlg dialog

class CScriptSetDlg : public CDialog
{
// Construction
public:
	CScriptSetDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CScriptSetDlg)
	enum { IDD = IDD_SCRIPTSET };
	CString	m_strName;
	CString	m_strPath;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CScriptSetDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CScriptSetDlg)
	afx_msg void OnBrowse();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SCRIPTSETDLG_H__1C8EC1B7_79AB_11D4_B0ED_004F49068BD6__INCLUDED_)
