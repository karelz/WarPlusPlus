// TGAMakerDlg.h : header file
//

#if !defined(AFX_TGAMAKERDLG_H__64AF7616_7B24_11D2_AB5A_99CC716C8761__INCLUDED_)
#define AFX_TGAMAKERDLG_H__64AF7616_7B24_11D2_AB5A_99CC716C8761__INCLUDED_

#include "ColorView.h"	// Added by ClassView
#include "PreviewWindow.h"	// Added by ClassView
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/////////////////////////////////////////////////////////////////////////////
// CTGAMakerDlg dialog

class CTGAMakerDlg : public CDialog
{
// Construction
public:
	virtual void ComputeAutomaticAlphaName();
	void ComputeAutomaticResultName();
	CTGAMakerDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CTGAMakerDlg)
	enum { IDD = IDD_TGAMAKER_DIALOG };
	CButton	m_wndBrowseResult;
	CButton	m_wndBrowseAlpha;
	CEdit	m_wndAlpha;
	CEdit	m_wndResult;
	CButton	m_wndAlwaysOnTop;
	CString	m_strAlpha;
	CString	m_strBitmap;
	CString	m_strResult;
	BOOL	m_bAutomaticResultName;
	BOOL	m_bAutomaticTransparentName;
	BOOL	m_bAlphaInverse;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTGAMakerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CTGAMakerDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBrowseAlpha();
	afx_msg void OnBrowseBitmap();
	afx_msg void OnBrowseResult();
	afx_msg void OnConvert();
	afx_msg void OnAlwaysOnTop();
	afx_msg void OnDestroy();
	afx_msg void OnAutomaticResultName();
	afx_msg void OnAutomaticTransparentName();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	CPreviewWindow *m_pPreview;
	CColorView m_BkgColor;
	BOOL m_bOnTop;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TGAMAKERDLG_H__64AF7616_7B24_11D2_AB5A_99CC716C8761__INCLUDED_)
