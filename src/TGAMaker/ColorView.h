#if !defined(AFX_COLORVIEW_H__B40F8AC5_1E88_11D3_9FBD_B8C954153938__INCLUDED_)
#define AFX_COLORVIEW_H__B40F8AC5_1E88_11D3_9FBD_B8C954153938__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ColorView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CColorView window

#include "PreviewWindow.h"

class CColorView : public CStatic
{
// Construction
public:
	CColorView();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CColorView)
	//}}AFX_VIRTUAL

// Implementation
public:
	void SetPreview(CPreviewWindow *pPreview);
	void SetColor(DWORD dwColor);
	DWORD GetColor();
	virtual ~CColorView();

	// Generated message map functions
protected:
	//{{AFX_MSG(CColorView)
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
private:
	CPreviewWindow * m_pPreview;
	DWORD m_dwColor;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COLORVIEW_H__B40F8AC5_1E88_11D3_9FBD_B8C954153938__INCLUDED_)
