#if !defined(AFX_RESOURCECONTROLINNER_H__F5C8BF37_5709_11D4_B0B3_004F49068BD6__INCLUDED_)
#define AFX_RESOURCECONTROLINNER_H__F5C8BF37_5709_11D4_B0B3_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ResourceControlInner.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CResourceControlInner window

class CResourceControlInner : public CStatic
{
// Construction
public:
	CResourceControlInner();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CResourceControlInner)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CResourceControlInner();

  void ResetText();
  int *m_pResources;

  CString m_strTitle;

	// Generated message map functions
protected:
	//{{AFX_MSG(CResourceControlInner)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RESOURCECONTROLINNER_H__F5C8BF37_5709_11D4_B0B3_004F49068BD6__INCLUDED_)
