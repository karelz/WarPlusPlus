#if !defined(AFX_UNITTOOLBAR_H__A26A6476_E875_11D3_A8CB_00105ACA8325__INCLUDED_)
#define AFX_UNITTOOLBAR_H__A26A6476_E875_11D3_A8CB_00105ACA8325__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// UnitToolBar.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CUnitToolBar window

#include "UnitTBButton.h"

class CUnitToolBar : public CToolBar
{
// Construction
public:
	CUnitToolBar();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUnitToolBar)
	//}}AFX_VIRTUAL

// Implementation
public:
	void ResetSelection();
	void SetSelectedUnit(CEMap *pMap);
	void LoadState(CString strSection);
	void SaveState(CString strSection);
	void CreateButtons();
  void DeleteButtons();
	virtual ~CUnitToolBar();

	// Generated message map functions
protected:
	void OnUnitButtonDblClk(UINT nID, NMHDR *pNotifyStruct, LRESULT *pResult);
	void OnUnitButton(UINT nID);
	//{{AFX_MSG(CUnitToolBar)
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

private:
  CTypedPtrArray<CPtrArray, CUnitTBButton *> m_aButtons;
  static DWORD m_aUnitsSelected[20];
  static DWORD m_aCivilizationsSelected[20];
  static int m_nUnitSelected;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_UNITTOOLBAR_H__A26A6476_E875_11D3_A8CB_00105ACA8325__INCLUDED_)
