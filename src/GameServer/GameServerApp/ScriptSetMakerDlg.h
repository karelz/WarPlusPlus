#if !defined(AFX_SCRIPTSETMAKERDLG_H__0FE21CA3_780C_11D4_B0EA_004F49068BD6__INCLUDED_)
#define AFX_SCRIPTSETMAKERDLG_H__0FE21CA3_780C_11D4_B0EA_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ScriptSetMakerDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CScriptSetMakerDlg dialog

class CScriptSetMakerDlg : public CDialog
{
// Construction
public:
	CScriptSetMakerDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CScriptSetMakerDlg)
	enum { IDD = IDD_SCRIPTSETMAKER };
	CString	m_strDestination;
	CString	m_strSource;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CScriptSetMakerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CScriptSetMakerDlg)
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	afx_msg void OnBrowseDestination();
	afx_msg void OnBrowseSource();
	afx_msg void OnProcess();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SCRIPTSETMAKERDLG_H__0FE21CA3_780C_11D4_B0EA_004F49068BD6__INCLUDED_)
