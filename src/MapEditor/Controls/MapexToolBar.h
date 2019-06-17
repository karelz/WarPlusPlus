#if !defined(AFX_MAPEXTOOLBAR_H__2315213B_6E73_11D3_A06B_C85E707FE131__INCLUDED_)
#define AFX_MAPEXTOOLBAR_H__2315213B_6E73_11D3_A06B_C85E707FE131__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MapexToolBar.h : header file
//

#include "MapexTBButton.h"
#include "..\DataObjects\EMap.h"

/////////////////////////////////////////////////////////////////////////////
// CMapexToolBar window

class CMapexToolBar : public CToolBar
{
// Construction
public:
	CMapexToolBar();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMapexToolBar)
	//}}AFX_VIRTUAL

// Implementation
public:
	void ResetSelection();
	void SetSelectedMapexes(CEMap *pMap);
	void LoadState(CString strSection);
	void SaveState(CString strSection);
	void CreateButtons();
  void DeleteButtons();
	virtual ~CMapexToolBar();

	// Generated message map functions
protected:
	//{{AFX_MSG(CMapexToolBar)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
  afx_msg void OnMapexButton(UINT nID);
  afx_msg void OnMapexButtonDblClk(UINT nID, NMHDR *pNotifyStruct, LRESULT *pResult);
	DECLARE_MESSAGE_MAP()

private:
  CTypedPtrArray<CPtrArray, CMapexTBButton *> m_aButtons;
  static DWORD m_aMapexesSelected[20];
  static int m_nMapexSelected;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAPEXTOOLBAR_H__2315213B_6E73_11D3_A06B_C85E707FE131__INCLUDED_)
