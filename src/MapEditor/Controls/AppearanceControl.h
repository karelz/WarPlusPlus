#if !defined(AFX_APPEARANCECONTROL_H__32B70983_7128_11D4_B0DB_004F49068BD6__INCLUDED_)
#define AFX_APPEARANCECONTROL_H__32B70983_7128_11D4_B0DB_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AppearanceControl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAppearanceControl window

class CEUnitType;

class CAppearanceControl : public CComboBox
{
// Construction
public:
	CAppearanceControl();

// Attributes
public:

// Operations
public:

  // Initializes the appearance control
  // pUnitType - unit type object to use to fill the combo with
  // bNoneItem - include the [none] item in the combo box (appearance ID = 0)
  void Create(CWnd *pParent, UINT nID, CEUnitType *pUnitType, DWORD dwSelectedAppearanceID = 0, BOOL bNoneItem = TRUE);

  // Returns the selected appearance ID
  DWORD GetSelectedAppearanceID();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAppearanceControl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CAppearanceControl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CAppearanceControl)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_APPEARANCECONTROL_H__32B70983_7128_11D4_B0DB_004F49068BD6__INCLUDED_)
