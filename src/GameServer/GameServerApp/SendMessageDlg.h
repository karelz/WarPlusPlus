#if !defined(AFX_SENDMESSAGEDLG_H__5A5DD6A6_7A95_11D4_B0EE_004F49068BD6__INCLUDED_)
#define AFX_SENDMESSAGEDLG_H__5A5DD6A6_7A95_11D4_B0EE_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SendMessageDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSendMessageDlg dialog

class CSendMessageDlg : public CDialog
{
// Construction
public:
	CSendMessageDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSendMessageDlg)
	enum { IDD = IDD_SENDMESSAGE };
	CString	m_strMessage;
	//}}AFX_DATA

  CString m_strCaption;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSendMessageDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSendMessageDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SENDMESSAGEDLG_H__5A5DD6A6_7A95_11D4_B0EE_004F49068BD6__INCLUDED_)
