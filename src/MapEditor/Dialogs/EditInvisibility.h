#if !defined(AFX_EDITINVISIBILITY_H__A8F87A6A_57D5_11D4_B0B4_004F49068BD6__INCLUDED_)
#define AFX_EDITINVISIBILITY_H__A8F87A6A_57D5_11D4_B0B4_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditInvisibility.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEditInvisibility dialog

class CEditInvisibility : public CDialog
{
// Construction
public:
	CEditInvisibility(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEditInvisibility)
	enum { IDD = IDD_EDITINVISIBILITY };
	CListCtrl	m_wndList;
	//}}AFX_DATA

  DWORD *m_pInvisibility;
  CString m_strTitle;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditInvisibility)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEditInvisibility)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnClickInvisibility(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
  CImageList m_ImageList;
  void UpdateList();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITINVISIBILITY_H__A8F87A6A_57D5_11D4_B0B4_004F49068BD6__INCLUDED_)
