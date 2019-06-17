#if !defined(AFX_UNITAPPEARANCEPAGE_H__C724C026_915E_11D3_A0C4_BA0897ADE231__INCLUDED_)
#define AFX_UNITAPPEARANCEPAGE_H__C724C026_915E_11D3_A0C4_BA0897ADE231__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// UnitAppearancePage.h : header file
//

#include "..\DataObjects\EUnitType.h"

/////////////////////////////////////////////////////////////////////////////
// CUnitAppearancePage dialog

class CUnitAppearancePage : public CPropertyPage
{
	DECLARE_DYNCREATE(CUnitAppearancePage)

// Construction
public:
  // creates the object (fills the data)
	void Create(CEUnitType *pUnitType);

  // constructor destructor
	CUnitAppearancePage();
	~CUnitAppearancePage();

// Dialog Data
	//{{AFX_DATA(CUnitAppearancePage)
	enum { IDD = IDD_UNITAPPEARANCEPAGE };
	CButton	m_wndEdit;
	CButton	m_wndDelete;
	CListCtrl	m_wndList;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CUnitAppearancePage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CUnitAppearancePage)
	virtual BOOL OnInitDialog();
	afx_msg void OnItemChangedList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblClkList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEdit();
	afx_msg void OnDelete();
	afx_msg void OnAdd();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	CEUnitAppearanceType * m_pCurrentAppearance;
	void UpdateSelectedAppearance();
	void UpdateListItems();
  // pointer to the edited unit type
	CEUnitType * m_pUnitType;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_UNITAPPEARANCEPAGE_H__C724C026_915E_11D3_A0C4_BA0897ADE231__INCLUDED_)
