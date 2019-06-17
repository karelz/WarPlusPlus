#if !defined(AFX_SETMAPSIZEDLG_H__933013EF_2A73_44F0_8EB7_0FF208D089CA__INCLUDED_)
#define AFX_SETMAPSIZEDLG_H__933013EF_2A73_44F0_8EB7_0FF208D089CA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SetMapSizeDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSetMapSizeDlg dialog

class CSetMapSizeDlg : public CDialog
{
// Construction
public:
	CSetMapSizeDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSetMapSizeDlg)
	enum { IDD = IDD_SETMAPSIZE };
	DWORD	m_dwWidth;
	DWORD	m_dwHeight;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSetMapSizeDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSetMapSizeDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SETMAPSIZEDLG_H__933013EF_2A73_44F0_8EB7_0FF208D089CA__INCLUDED_)
