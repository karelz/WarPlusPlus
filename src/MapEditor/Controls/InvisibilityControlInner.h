#if !defined(AFX_INVISIBILITYCONTROLINNER_H__A8F87A69_57D5_11D4_B0B4_004F49068BD6__INCLUDED_)
#define AFX_INVISIBILITYCONTROLINNER_H__A8F87A69_57D5_11D4_B0B4_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// InvisibilityControlInner.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CInvisibilityControlInner window

class CInvisibilityControlInner : public CStatic
{
// Construction
public:
	CInvisibilityControlInner();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInvisibilityControlInner)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CInvisibilityControlInner();

  void ResetText();
  DWORD *m_pInvisibility;

  CString m_strTitle;

	// Generated message map functions
protected:
	//{{AFX_MSG(CInvisibilityControlInner)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INVISIBILITYCONTROLINNER_H__A8F87A69_57D5_11D4_B0B4_004F49068BD6__INCLUDED_)
