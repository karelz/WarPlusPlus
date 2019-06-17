#if !defined(AFX_UNITMODESPAGE_H__B25631C3_945C_11D3_A0D0_99C52D082338__INCLUDED_)
#define AFX_UNITMODESPAGE_H__B25631C3_945C_11D3_A0D0_99C52D082338__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// UnitModesPage.h : header file
//

#include "..\DataObjects\EUnitType.h"
#include "..\Controls\InvisibilityControl.h"

/////////////////////////////////////////////////////////////////////////////
// CUnitModesPage dialog

class CUnitModesPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CUnitModesPage)

// Construction
public:
	void Create(CEUnitType *pUnitType);
	CUnitModesPage();
	~CUnitModesPage();

// Dialog Data
	//{{AFX_DATA(CUnitModesPage)
	enum { IDD = IDD_UNITMODESPAGE };
	CComboBox	m_wndAppearance;
	CComboBox	m_wndMode;
	CString	m_strName;
	DWORD	m_dwMoveWidth;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CUnitModesPage)
	public:
	virtual void OnOK();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CUnitModesPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelEndOkMode();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void WriteNewData();
	void UpdateModePage();
	void UpdateSelectedMode();
  CEUnitType *m_pUnitType;
  DWORD m_dwCurrentMode;
  CInvisibilityControl m_wndInvisibility;
  CInvisibilityControl m_wndDetection;
  DWORD m_dwInvisibility;
  DWORD m_dwDetection;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_UNITMODESPAGE_H__B25631C3_945C_11D3_A0D0_99C52D082338__INCLUDED_)
