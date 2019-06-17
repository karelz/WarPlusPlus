#if !defined(AFX_RESOURCECONTROL_H__F5C8BF34_5709_11D4_B0B3_004F49068BD6__INCLUDED_)
#define AFX_RESOURCECONTROL_H__F5C8BF34_5709_11D4_B0B3_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ResourceControl.h : header file
//

#include "ResourceControlInner.h"

/////////////////////////////////////////////////////////////////////////////
// CResourceControl window

class CResourceControl : public CWnd
{
// Construction
public:
	CResourceControl();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CResourceControl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CResourceControl();

  void Create(CWnd *pParent, UINT nID, int *pResources);
  void SetTitle(CString strTitle){ m_wndInner.m_strTitle = strTitle; }

	// Generated message map functions
protected:
	//{{AFX_MSG(CResourceControl)
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnDestroy();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

private:
  CResourceControlInner m_wndInner;
  CFont m_Font;
  CBrush m_BkgBrush;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RESOURCECONTROL_H__F5C8BF34_5709_11D4_B0B3_004F49068BD6__INCLUDED_)
