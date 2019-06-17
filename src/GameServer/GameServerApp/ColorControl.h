#if !defined(AFX_COLORCONTROL_H__D78F05D4_6C30_11D4_B529_00105ACA8325__INCLUDED_)
#define AFX_COLORCONTROL_H__D78F05D4_6C30_11D4_B529_00105ACA8325__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ColorControl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CColorControl window

class CColorControl : public CStatic
{
// Construction
public:
	CColorControl();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CColorControl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CColorControl();

  void Create(CRect &rcRect, DWORD dwStyle, CWnd *pParent, COLORREF Color, UINT nID);

	// Generated message map functions
protected:
	//{{AFX_MSG(CColorControl)
	afx_msg void OnPaint();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

private:
  // The color to display
  COLORREF m_Color;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COLORCONTROL_H__D78F05D4_6C30_11D4_B529_00105ACA8325__INCLUDED_)
