#if !defined(AFX_COLORPICKER_H__688CEF76_8A12_11D3_A0A6_FEA9F52CDF31__INCLUDED_)
#define AFX_COLORPICKER_H__688CEF76_8A12_11D3_A0A6_FEA9F52CDF31__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ColorPicker.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CColorPicker window

class CColorPicker : public CStatic
{
// Construction
public:
	CColorPicker();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CColorPicker)
	//}}AFX_VIRTUAL

// Implementation
public:
  void Create ( UINT nID, CWnd * pParent );
  void Create ( UINT nID, DWORD dwStyle, CRect &rcControl, CWnd * pParent, CWnd * pObserver );

	DWORD GetColor();
	void SetColor(DWORD dwColor);
	virtual ~CColorPicker();

	// Generated message map functions
protected:
	//{{AFX_MSG(CColorPicker)
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

private:
  DWORD m_dwColor;
  CWnd * m_pObserver;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COLORPICKER_H__688CEF76_8A12_11D3_A0A6_FEA9F52CDF31__INCLUDED_)
