#if !defined(AFX_SELECTUNITDLG_H__A26A6478_E875_11D3_A8CB_00105ACA8325__INCLUDED_)
#define AFX_SELECTUNITDLG_H__A26A6478_E875_11D3_A8CB_00105ACA8325__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SelectUnitdlg.h : header file
//

#include "..\DataObjects\EUnitType.h"
#include "..\UnitEditor\UnitTypePreview.h"
#include "..\DataObjects\EMap.h"

/////////////////////////////////////////////////////////////////////////////
// CSelectUnitDlg dialog

class CSelectUnitDlg : public CDialog
{
// Construction
public:
  CECivilization *GetSelectedCivilization(){ return m_pCurrentCivilization; }
  CEUnitType *GetSelectedUnitType(){ return m_pCurrentUnitType; }
  void Create(CEMap *pMap);
	CSelectUnitDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSelectUnitDlg)
	enum { IDD = IDD_SELECTUNIT };
	CButton	m_OK;
	CListCtrl	m_UnitList;
	CComboBox	m_CivCombo;
	CComboBox	m_LibraryCombo;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSelectUnitDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSelectUnitDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelendokLibraryCombo();
	afx_msg void OnSelendokCivCombo();
	afx_msg void OnDblclkUnitList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemchangedUnitList(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void UpdateSelectedCiv();
	void UpdateSelectedUnit();
	void UpdateSelectedLibrary();
	void UpdateListItems();
	static int m_nLastSelectedLibrary;
  static int m_nLastSelectedCivilization;
	CEMap * m_pMap;
	CEUnitLibrary * m_pCurrentUnitLibrary;
	CEUnitType * m_pCurrentUnitType;
	CUnitTypePreview m_wndUnitTypePreview;
  CECivilization * m_pCurrentCivilization;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SELECTUNITDLG_H__A26A6478_E875_11D3_A8CB_00105ACA8325__INCLUDED_)
