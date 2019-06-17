#if !defined(AFX_OPTIONSNETWORKPAGE_H__FE9BFEA3_6DDB_11D4_B0D1_004F49068BD6__INCLUDED_)
#define AFX_OPTIONSNETWORKPAGE_H__FE9BFEA3_6DDB_11D4_B0D1_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptionsNetworkPage.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COptionsNetworkPage dialog

class COptionsNetworkPage : public CPropertyPage
{
	DECLARE_DYNCREATE(COptionsNetworkPage)

// Construction
public:
  COptionsNetworkPage(){};
	COptionsNetworkPage(int eGameState);
	~COptionsNetworkPage();

// Dialog Data
	//{{AFX_DATA(COptionsNetworkPage)
	enum { IDD = IDD_OPTIONS_NETWORK };
	CEdit	m_wndPort;
	DWORD	m_dwPort;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(COptionsNetworkPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(COptionsNetworkPage)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

  int m_eGameState;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPTIONSNETWORKPAGE_H__FE9BFEA3_6DDB_11D4_B0D1_004F49068BD6__INCLUDED_)
