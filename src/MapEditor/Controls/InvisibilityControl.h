#if !defined(AFX_INVISIBILITYCONTROL_H__A8F87A68_57D5_11D4_B0B4_004F49068BD6__INCLUDED_)
#define AFX_INVISIBILITYCONTROL_H__A8F87A68_57D5_11D4_B0B4_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// InvisibilityControl.h : header file
//

#include "InvisibilityControlInner.h"

/////////////////////////////////////////////////////////////////////////////
// CInvisibilityControl window

class CInvisibilityControl : public CWnd
{
// Construction
public:
	CInvisibilityControl();

// Attributes
public:

// Operations
public:

  void Create(CWnd *pParent, UINT nID, DWORD *pInvisibility);
  void SetInvisibility(DWORD *pInvisibility);
  void SetTitle(CString strTitle){ m_wndInner.m_strTitle = strTitle; }

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInvisibilityControl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CInvisibilityControl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CInvisibilityControl)
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
  CInvisibilityControlInner m_wndInner;
  CFont m_Font;
  CBrush m_BkgBrush;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INVISIBILITYCONTROL_H__A8F87A68_57D5_11D4_B0B4_004F49068BD6__INCLUDED_)
