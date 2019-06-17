#if !defined(AFX_FINDPATHGRAPHEDITDLG_H__34B13458_80F0_11D4_B0FA_004F49068BD6__INCLUDED_)
#define AFX_FINDPATHGRAPHEDITDLG_H__34B13458_80F0_11D4_B0FA_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FindPathGraphEditDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFindPathGraphEditDlg dialog

class CEFindPathGraph;

class CFindPathGraphEditDlg : public CDialog
{
// Construction
public:
	CFindPathGraphEditDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CFindPathGraphEditDlg)
	enum { IDD = IDD_FINDPATHGRAPHEDIT };
	CListCtrl	m_wndList;
	CString	m_strName;
	//}}AFX_DATA

  void Create(CEFindPathGraph *pGraph){ m_pGraph = pGraph; }

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFindPathGraphEditDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFindPathGraphEditDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnClickList(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
  CImageList m_ImageList;
  CEFindPathGraph *m_pGraph;
  void UpdateList();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FINDPATHGRAPHEDITDLG_H__34B13458_80F0_11D4_B0FA_004F49068BD6__INCLUDED_)
