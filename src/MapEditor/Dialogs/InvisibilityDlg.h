#if !defined(AFX_INVISIBILITYDLG_H__A8F87A66_57D5_11D4_B0B4_004F49068BD6__INCLUDED_)
#define AFX_INVISIBILITYDLG_H__A8F87A66_57D5_11D4_B0B4_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// InvisibilityDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CInvisibilityDlg dialog

class CInvisibilityDlg : public CDialog
{
// Construction
public:
	CInvisibilityDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CInvisibilityDlg)
	enum { IDD = IDD_INVISIBILITY };
	CListCtrl	m_wndList;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInvisibilityDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CInvisibilityDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnEndLabelEditInvisibility(NMHDR* pNMHDR, LRESULT* pResult);
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INVISIBILITYDLG_H__A8F87A66_57D5_11D4_B0B4_004F49068BD6__INCLUDED_)
