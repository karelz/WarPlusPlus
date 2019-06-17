#if !defined(AFX_STARTDLG_H__A8F87A63_57D5_11D4_B0B4_004F49068BD6__INCLUDED_)
#define AFX_STARTDLG_H__A8F87A63_57D5_11D4_B0B4_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// StartDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CStartDlg dialog

class CStartDlg : public CDialog
{
// Construction
public:
	CStartDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CStartDlg)
	enum { IDD = IDD_STARTDLG };
	CString	m_strLastOpenedMap;
	//}}AFX_DATA

  DWORD m_dwWhatToDo;
  BOOL m_bDisableLastOpened;
  enum{
    NewMap = 1,
    LastOpened = 2,
    OpenMap = 3,
  };

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CStartDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

  HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CStartDlg)
	afx_msg void OnLastOpened();
	afx_msg void OnNewMap();
	afx_msg void OnOpen();
	virtual BOOL OnInitDialog();
	afx_msg void OnDoubleClickedLastOpened();
	afx_msg void OnDoubleClickedNewMap();
	afx_msg void OnDoubleClickedOpen();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STARTDLG_H__A8F87A63_57D5_11D4_B0B4_004F49068BD6__INCLUDED_)
