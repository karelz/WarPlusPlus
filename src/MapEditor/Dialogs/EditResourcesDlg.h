#if !defined(AFX_EDITRESOURCESDLG_H__F5C8BF31_5709_11D4_B0B3_004F49068BD6__INCLUDED_)
#define AFX_EDITRESOURCESDLG_H__F5C8BF31_5709_11D4_B0B3_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditResourcesDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEditResourcesDlg dialog

class CEditResourcesDlg : public CDialog
{
// Construction
public:
	CEditResourcesDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEditResourcesDlg)
	enum { IDD = IDD_EDITRESOURCES };
	CListCtrl	m_wndList;
	//}}AFX_DATA
  int *m_pResources;
  CString m_strTitle;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditResourcesDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEditResourcesDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnEndLabelEditResources(NMHDR* pNMHDR, LRESULT* pResult);
	virtual void OnOK();
	afx_msg void OnItemChangedResources(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITRESOURCESDLG_H__F5C8BF31_5709_11D4_B0B3_004F49068BD6__INCLUDED_)
