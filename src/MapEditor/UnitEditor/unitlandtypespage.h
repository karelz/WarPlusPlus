#if !defined(AFX_UNITLANDTYPESPAGE_H__D8CBFB54_945F_11D3_A0D0_99C52D082338__INCLUDED_)
#define AFX_UNITLANDTYPESPAGE_H__D8CBFB54_945F_11D3_A0D0_99C52D082338__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// unitlandtypespage.h : header file
//

#include "..\DataObjects\EUnitType.h"
#include "..\DataObjects\EMap.h"
#include "..\DataObjects\EFindPathGraph.h"

/////////////////////////////////////////////////////////////////////////////
// CUnitLandTypesPage dialog

class CUnitLandTypesPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CUnitLandTypesPage)

// Construction
public:
	void UpdateSelectedMode();
	void Create(CEUnitType *pUnitType, CEMap *pMap);
	CUnitLandTypesPage();
	~CUnitLandTypesPage();

// Dialog Data
	//{{AFX_DATA(CUnitLandTypesPage)
	enum { IDD = IDD_UNITLANDTYPESPAGE };
	CComboBox	m_wndModes;
	CListCtrl	m_wndList;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CUnitLandTypesPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CUnitLandTypesPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelEndOkModes();
	afx_msg void OnItemchangedList(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void UpdateSelectedGraph();
	CImageList m_ImageList;
  CEUnitType *m_pUnitType;
  CEMap *m_pMap;
  DWORD m_dwCurrentMode;
  CEFindPathGraph *m_pSelectedGraph;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_UNITLANDTYPESPAGE_H__D8CBFB54_945F_11D3_A0D0_99C52D082338__INCLUDED_)
