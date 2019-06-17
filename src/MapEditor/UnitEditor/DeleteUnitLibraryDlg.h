#if !defined(AFX_DELETEUNITLIBRARYDLG_H__E43F8246_8FCC_11D3_A0C0_D87876DDE731__INCLUDED_)
#define AFX_DELETEUNITLIBRARYDLG_H__E43F8246_8FCC_11D3_A0C0_D87876DDE731__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DeleteUnitLibraryDlg.h : header file
//

#include "..\DataObjects\EMap.h"

/////////////////////////////////////////////////////////////////////////////
// CDeleteUnitLibraryDlg dialog

class CDeleteUnitLibraryDlg : public CDialog
{
// Construction
public:
	BOOL Create(CEMap *pMap);
	CDeleteUnitLibraryDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDeleteUnitLibraryDlg)
	enum { IDD = IDD_DELETEUNITLIBRARY };
	CListBox	m_wndList;
	CButton	m_wndDelete;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDeleteUnitLibraryDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDeleteUnitLibraryDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelChangeList();
	afx_msg void OnDelete();
	afx_msg void OnDblclkList();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void UpdateListItems();
  CEMap *m_pMap;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DELETEUNITLIBRARYDLG_H__E43F8246_8FCC_11D3_A0C0_D87876DDE731__INCLUDED_)
