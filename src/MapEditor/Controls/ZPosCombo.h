#if !defined(AFX_ZPOSCOMBO_H__8D1969D5_8888_11D3_A0A0_AA491F88ED31__INCLUDED_)
#define AFX_ZPOSCOMBO_H__8D1969D5_8888_11D3_A0A0_AA491F88ED31__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ZPosCombo.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CZPosCombo window

class CZPosCombo : public CComboBox
{
// Construction
public:
	CZPosCombo();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CZPosCombo)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CZPosCombo();

	// Generated message map functions
protected:
	//{{AFX_MSG(CZPosCombo)
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ZPOSCOMBO_H__8D1969D5_8888_11D3_A0A0_AA491F88ED31__INCLUDED_)
