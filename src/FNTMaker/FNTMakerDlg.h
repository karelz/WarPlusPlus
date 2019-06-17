// FNTMakerDlg.h : header file
//

#if !defined(AFX_FNTMAKERDLG_H__C6DEDCE7_87AC_11D2_AB85_85B649344A60__INCLUDED_)
#define AFX_FNTMAKERDLG_H__C6DEDCE7_87AC_11D2_AB85_85B649344A60__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CFNTMakerDlg dialog

class CFNTMakerDlg : public CDialog
{
// Construction
public:
	CFNTMakerDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CFNTMakerDlg)
	enum { IDD = IDD_FNTMAKER_DIALOG };
	CButton	m_wndAlwaysOnTop;
	CString	m_strASCIICodes;
	CString	m_strBitmap;
	CString	m_strResult;
	DWORD	m_dwCharSpace;
	DWORD	m_dwLineSpace;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFNTMakerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CFNTMakerDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBrowseASCIICodes();
	afx_msg void OnBrowseBitmap();
	afx_msg void OnBrowseResult();
	afx_msg void OnAlwaysOnTop();
	afx_msg void OnConvert();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	BOOL m_bOnTop;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FNTMAKERDLG_H__C6DEDCE7_87AC_11D2_AB85_85B649344A60__INCLUDED_)
