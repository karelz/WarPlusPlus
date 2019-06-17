#if !defined(AFX_EDITMAPEXLIBRARYDLG_H__6868A9AA_6755_11D3_A059_B7FF8C0EE331__INCLUDED_)
#define AFX_EDITMAPEXLIBRARYDLG_H__6868A9AA_6755_11D3_A059_B7FF8C0EE331__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditMapexLibraryDlg.h : header file
//

#include "..\DataObjects\EMap.h"

/////////////////////////////////////////////////////////////////////////////
// CEditMapexLibraryDlg dialog

class CEditMapexLibraryDlg : public CDialog
{
// Construction
public:
	void Create(CEMap *pMap);
	CEditMapexLibraryDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEditMapexLibraryDlg)
	enum { IDD = IDD_EDITLIBRARY };
	CButton	m_Edit;
	CListBox	m_List;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditMapexLibraryDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEditMapexLibraryDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelChangeList();
	afx_msg void OnEdit();
	afx_msg void OnDblClkList();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void UpdateListItems();
	CEMap * m_pMap;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITMAPEXLIBRARYDLG_H__6868A9AA_6755_11D3_A059_B7FF8C0EE331__INCLUDED_)
