#if !defined(AFX_UNITLIBRARIESDLG_H__E43F8245_8FCC_11D3_A0C0_D87876DDE731__INCLUDED_)
#define AFX_UNITLIBRARIESDLG_H__E43F8245_8FCC_11D3_A0C0_D87876DDE731__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// UnitLibrariesDlg.h : header file
//

#include "..\DataObjects\EUnitLibrary.h"
#include "UnitTypePreview.h"
#include "..\DataObjects\EMap.h"
#include "UnitSkillTypeCollection.h"

/////////////////////////////////////////////////////////////////////////////
// CUnitLibrariesDlg dialog

class CUnitLibrariesDlg : public CDialog
{
// Construction
public:
	void Create(CEMap *pMap);
	CUnitLibrariesDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CUnitLibrariesDlg)
	enum { IDD = IDD_UNITLIBRARIES };
	CComboBox	m_wndLibraryCombo;
	CListCtrl	m_wndUnitListCtrl;
	CButton	m_wndEdit;
	CButton	m_wndDelete;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUnitLibrariesDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CUnitLibrariesDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelEndOkLibraryCombo();
	afx_msg void OnItemChangedUnitListCtrl(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblClkUnitListCtrl(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnAddUnit();
	afx_msg void OnDeleteUnit();
	afx_msg void OnEditUnit();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void UpdateSelectedUnit();
	void UpdateListItems();
	void UpdateSelectedLibrary();
  CEMap *m_pMap;
  CEUnitLibrary *m_pCurrentUnitLibrary;
  CEUnitType *m_pCurrentUnit;
  CUnitTypePreview m_wndUnitPreview;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_UNITLIBRARIESDLG_H__E43F8245_8FCC_11D3_A0C0_D87876DDE731__INCLUDED_)
