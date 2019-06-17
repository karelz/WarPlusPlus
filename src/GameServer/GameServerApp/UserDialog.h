#if !defined(AFX_USERDIALOG_H__4DA9AB75_5E49_11D4_8526_FFFFFFFFFFFF__INCLUDED_)
#define AFX_USERDIALOG_H__4DA9AB75_5E49_11D4_8526_FFFFFFFFFFFF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// UserDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CUserDialog dialog

class CUserDialog : public CDialog
{
// Construction
public:
	CUserDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CUserDialog)
	enum { IDD = IDD_USER };
	CEdit	m_wndName;
	CString	m_strComment;
	CString	m_strName;
	CString	m_strPassword;
	CString	m_strPassword2;
	//}}AFX_DATA

  BOOL m_bEnableName;


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUserDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CUserDialog)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_USERDIALOG_H__4DA9AB75_5E49_11D4_8526_FFFFFFFFFFFF__INCLUDED_)
