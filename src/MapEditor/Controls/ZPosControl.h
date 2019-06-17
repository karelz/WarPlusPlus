#if !defined(AFX_ZPOSCONTROL_H__5A5DD6A3_7A95_11D4_B0EE_004F49068BD6__INCLUDED_)
#define AFX_ZPOSCONTROL_H__5A5DD6A3_7A95_11D4_B0EE_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ZPosControl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CZPosControl window

class CZPosControl : public CComboBox
{
// Construction
public:
	CZPosControl();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CZPosControl)
	//}}AFX_VIRTUAL

  void Create(CWnd *pParent, UINT nID);
  DWORD GetZPos();
  void SetZPos(DWORD dwPos);

// Implementation
public:
	virtual ~CZPosControl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CZPosControl)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ZPOSCONTROL_H__5A5DD6A3_7A95_11D4_B0EE_004F49068BD6__INCLUDED_)
