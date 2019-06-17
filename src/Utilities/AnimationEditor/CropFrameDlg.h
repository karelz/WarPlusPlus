#if !defined(AFX_CROPFRAMEDLG_H__1848B15B_0C55_4746_9354_43CEE2D132B2__INCLUDED_)
#define AFX_CROPFRAMEDLG_H__1848B15B_0C55_4746_9354_43CEE2D132B2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CropFrameDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCropFrameDlg dialog

class CCropFrameDlg : public CDialog
{
// Construction
public:
	CCropFrameDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCropFrameDlg)
	enum { IDD = IDD_CROPFRAME };
	DWORD	m_dwTolerance;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCropFrameDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCropFrameDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CROPFRAMEDLG_H__1848B15B_0C55_4746_9354_43CEE2D132B2__INCLUDED_)
