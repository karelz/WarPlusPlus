#if !defined(AFX_DELETEMAPEXLIBRARYDLG_H__6868A9AB_6755_11D3_A059_B7FF8C0EE331__INCLUDED_)
#define AFX_DELETEMAPEXLIBRARYDLG_H__6868A9AB_6755_11D3_A059_B7FF8C0EE331__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DeleteMapexLibraryDlg.h : header file
//

#include "..\DataObjects\EMap.h"

/////////////////////////////////////////////////////////////////////////////
// CDeleteMapexLibraryDlg dialog

class CDeleteMapexLibraryDlg : public CDialog
{
// Construction
public:
	void UpdateListItems();
	void Create(CEMap *pMap);
	CDeleteMapexLibraryDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDeleteMapexLibraryDlg)
	enum { IDD = IDD_DELETELIBRARY };
	CListBox	m_List;
	CButton	m_Delete;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDeleteMapexLibraryDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDeleteMapexLibraryDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelChangeList();
	afx_msg void OnDblClkList();
	afx_msg void OnDelete();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	CEMap * m_pMap;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DELETEMAPEXLIBRARYDLG_H__6868A9AB_6755_11D3_A059_B7FF8C0EE331__INCLUDED_)
