#if !defined(AFX_OPTIONSGAMEPAGE_H__FE9BFEA7_6DDB_11D4_B0D1_004F49068BD6__INCLUDED_)
#define AFX_OPTIONSGAMEPAGE_H__FE9BFEA7_6DDB_11D4_B0D1_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptionsGamePage.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COptionsGamePage dialog

class COptionsGamePage : public CPropertyPage
{
	DECLARE_DYNCREATE(COptionsGamePage)

// Construction
public:
  COptionsGamePage(){};
	COptionsGamePage(int eGameStatus);
	~COptionsGamePage();

// Dialog Data
	//{{AFX_DATA(COptionsGamePage)
	enum { IDD = IDD_OPTIONS_GAME };
	CButton	m_wndShowTrayInfos;
	DWORD	m_dwTimesliceLength;
	BOOL	m_bShowProfiling;
	BOOL	m_bShowTrayInfos;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(COptionsGamePage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(COptionsGamePage)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

  int m_eGameState;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPTIONSGAMEPAGE_H__FE9BFEA7_6DDB_11D4_B0D1_004F49068BD6__INCLUDED_)
