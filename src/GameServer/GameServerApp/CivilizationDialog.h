#if !defined(AFX_CIVILIZATIONDIALOG_H__D78F05D3_6C30_11D4_B529_00105ACA8325__INCLUDED_)
#define AFX_CIVILIZATIONDIALOG_H__D78F05D3_6C30_11D4_B529_00105ACA8325__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CivilizationDialog.h : header file
//

#include "ColorControl.h"

/////////////////////////////////////////////////////////////////////////////
// CCivilizationDialog dialog

class CSCivilization;

class CCivilizationDialog : public CDialog
{
// Construction
public:
	CCivilizationDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCivilizationDialog)
	enum { IDD = IDD_CIVILIZATION };
	CListCtrl	m_wndProfileList;
	CString	m_strName;
	CString	m_strGlobalProfile;
	CString	m_strLocalProfile;
	BOOL	m_bFreeze;
	CString	m_strGlobalCount;
	CString	m_strLocalCount;
	//}}AFX_DATA
  COLORREF m_Color;

  CSCivilization * m_pSCivilization;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCivilizationDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCivilizationDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
  afx_msg LRESULT OnProfileUpdated ( WPARAM, LPARAM );
	afx_msg void OnReset();
	afx_msg void OnColumnItemClickProfileList(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

  CColorControl m_wndColor;

  int m_nSortColumn;
  void InsertProfileLine ( CString &strLine, int &nItem );
  static int CALLBACK SortFunction (LPARAM p1, LPARAM p2, LPARAM pT);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CIVILIZATIONDIALOG_H__D78F05D3_6C30_11D4_B529_00105ACA8325__INCLUDED_)
