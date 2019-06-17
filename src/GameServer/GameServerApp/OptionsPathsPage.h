#if !defined(AFX_OPTIONSPATHSPAGE_H__BF3C8825_6D3A_11D4_B0D0_004F49068BD6__INCLUDED_)
#define AFX_OPTIONSPATHSPAGE_H__BF3C8825_6D3A_11D4_B0D0_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptionsPathsPage.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COptionsPathsPage dialog

class COptionsPathsPage : public CPropertyPage
{
	DECLARE_DYNCREATE(COptionsPathsPage)

// Construction
public:
  COptionsPathsPage(){};
	COptionsPathsPage(int eGameState);
	~COptionsPathsPage();

// Dialog Data
	//{{AFX_DATA(COptionsPathsPage)
	enum { IDD = IDD_OPTIONS_PATHS };
	CEdit	m_wndGameTemp;
	CEdit	m_wndData;
	CButton	m_wndBrowseGameTemp;
	CButton	m_wndBrowseData;
	CButton	m_wndBrowseArchiveTemp;
	CEdit	m_wndArchiveTemp;
	CString	m_strData;
	CString	m_strGameTemp;
	CString	m_strArchiveTemp;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(COptionsPathsPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(COptionsPathsPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnBrowseArchiveTemp();
	afx_msg void OnBrowseData();
	afx_msg void OnBrowseGameTemp();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

  int m_eGameState;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPTIONSPATHSPAGE_H__BF3C8825_6D3A_11D4_B0D0_004F49068BD6__INCLUDED_)
