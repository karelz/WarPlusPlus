#if !defined(AFX_SPLASHWINDOWDLG_H__43E086D9_6434_11D3_A054_ADE3A89A0638__INCLUDED_)
#define AFX_SPLASHWINDOWDLG_H__43E086D9_6434_11D3_A054_ADE3A89A0638__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SplashWindowDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSplashWindowDlg dialog

class CSplashWindowDlg : public CDialog
{
// Construction
public:
	CSplashWindowDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSplashWindowDlg)
	enum { IDD = IDD_SPLASHWINDOW };
	CStatic	m_wndSplashBitmap;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSplashWindowDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSplashWindowDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	HBITMAP m_hBitmap;
  HICON m_hIcon;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SPLASHWINDOWDLG_H__43E086D9_6434_11D3_A054_ADE3A89A0638__INCLUDED_)
