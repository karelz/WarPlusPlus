#if !defined(AFX_EDITUNITLIBRARYDLG_H__E43F8248_8FCC_11D3_A0C0_D87876DDE731__INCLUDED_)
#define AFX_EDITUNITLIBRARYDLG_H__E43F8248_8FCC_11D3_A0C0_D87876DDE731__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditUnitLibraryDlg.h : header file
//

#include "..\DataObjects\EMap.h"

/////////////////////////////////////////////////////////////////////////////
// CEditUnitLibraryDlg dialog

class CEditUnitLibraryDlg : public CDialog
{
// Construction
public:
	BOOL Create(CEMap *pMap);
	CEditUnitLibraryDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEditUnitLibraryDlg)
	enum { IDD = IDD_EDITUNITLIBRARY };
	CListBox	m_wndList;
	CButton	m_wndEdit;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditUnitLibraryDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEditUnitLibraryDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelChangeList();
	afx_msg void OnEdit();
	afx_msg void OnDblclkList();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void UpdateListItems();
  CEMap *m_pMap;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITUNITLIBRARYDLG_H__E43F8248_8FCC_11D3_A0C0_D87876DDE731__INCLUDED_)
