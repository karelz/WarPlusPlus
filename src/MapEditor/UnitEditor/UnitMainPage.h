#if !defined(AFX_UNITMAINPAGE_H__CB5410F7_8E0D_11D3_A876_00105ACA8325__INCLUDED_)
#define AFX_UNITMAINPAGE_H__CB5410F7_8E0D_11D3_A876_00105ACA8325__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// UnitMainPage.h : header file
//

#include "..\DataObjects\EUnitType.h"

/////////////////////////////////////////////////////////////////////////////
// CUnitMainPage dialog

class CUnitMainPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CUnitMainPage)

// Construction
public:
  // creates the object (fills the data)
	void Create(CEUnitType *pUnitType);

  // constructor & destructor
	CUnitMainPage();
	~CUnitMainPage();

// Dialog Data
	//{{AFX_DATA(CUnitMainPage)
	enum { IDD = IDD_UNITMAINPAGE };
	CComboBox	m_wndZPos;
	CString	m_strName;
	DWORD	m_dwLifeMax;
	DWORD	m_dwViewRadius;
	BOOL	m_bSelectable;
	CString	m_strZPos;
	//}}AFX_DATA
  DWORD m_dwZPos;

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CUnitMainPage)
	public:
	virtual void OnOK();
	virtual BOOL OnKillActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CUnitMainPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelectable();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
  // pointer to the edited unit type
	CEUnitType * m_pUnitType;

  DWORD m_dwLifeBarFlags;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_UNITMAINPAGE_H__CB5410F7_8E0D_11D3_A876_00105ACA8325__INCLUDED_)
