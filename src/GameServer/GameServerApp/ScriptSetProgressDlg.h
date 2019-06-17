#if !defined(AFX_SCRIPTSETPROGRESSDLG_H__0FE21CA6_780C_11D4_B0EA_004F49068BD6__INCLUDED_)
#define AFX_SCRIPTSETPROGRESSDLG_H__0FE21CA6_780C_11D4_B0EA_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ScriptSetProgressDlg.h : header file
//

#include "EditErrorOutput.h"

/////////////////////////////////////////////////////////////////////////////
// CScriptSetProgressDlg dialog

class CScriptSetProgressDlg : public CDialog
{
// Construction
public:
	CScriptSetProgressDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CScriptSetProgressDlg)
	enum { IDD = IDD_SCRIPTSETPROGRESS };
	CEdit	m_wndErrorOutput;
	CButton	m_wndOK;
	CString	m_strStatus;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CScriptSetProgressDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

public:
  CScriptSetMaker *m_pScriptSetMaker;
  CEditErrorOutput *m_pErrOutput;
  BOOL m_bProgress;

  bool m_bSuccess;

// Implementation
protected:


	// Generated message map functions
	//{{AFX_MSG(CScriptSetProgressDlg)
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SCRIPTSETPROGRESSDLG_H__0FE21CA6_780C_11D4_B0EA_004F49068BD6__INCLUDED_)
