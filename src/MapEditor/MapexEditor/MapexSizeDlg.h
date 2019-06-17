#if !defined(AFX_MAPEXSIZEDLG_H__68D06E44_6824_11D3_A05B_BC8EB5073538__INCLUDED_)
#define AFX_MAPEXSIZEDLG_H__68D06E44_6824_11D3_A05B_BC8EB5073538__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MapexSizeDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMapexSizeDlg dialog

class CMapexSizeDlg : public CDialog
{
// Construction
public:
	CMapexSizeDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CMapexSizeDlg)
	enum { IDD = IDD_MAPEXSIZE };
	DWORD	m_dwHeight;
	DWORD	m_dwWidth;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMapexSizeDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMapexSizeDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAPEXSIZEDLG_H__68D06E44_6824_11D3_A05B_BC8EB5073538__INCLUDED_)
