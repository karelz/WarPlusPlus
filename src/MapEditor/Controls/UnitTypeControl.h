#if !defined(AFX_UNITTYPECONTROL_H__E44B2BF4_7FF6_11D4_B0F8_004F49068BD6__INCLUDED_)
#define AFX_UNITTYPECONTROL_H__E44B2BF4_7FF6_11D4_B0F8_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// UnitTypeControl.h : header file
//

#include "..\DataObjects\EUnitType.h"

/////////////////////////////////////////////////////////////////////////////
// CUnitTypeControl window

class CUnitTypeControl : public CComboBox
{
// Construction
public:
	CUnitTypeControl();

// Attributes
public:

// Operations
public:

  void Create(CWnd *pParent, UINT nID, CEUnitType *pSelectedUnitType);

  CEUnitType *GetSelectedUnitType();
  void SetSelectedUnitType(CEUnitType *pUnitType);

  void SetSelectedUnitType(DWORD dwUnitTypeID);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUnitTypeControl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CUnitTypeControl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CUnitTypeControl)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_UNITTYPECONTROL_H__E44B2BF4_7FF6_11D4_B0F8_004F49068BD6__INCLUDED_)
