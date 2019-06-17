#if !defined(AFX_UNITANIMATIONPREVIEW_H__8CA351A6_92D4_11D3_A0C7_92DF3C7AE131__INCLUDED_)
#define AFX_UNITANIMATIONPREVIEW_H__8CA351A6_92D4_11D3_A0C7_92DF3C7AE131__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// UnitAnimationPreview.h : header file
//

#include "..\DataObjects\EUnitAnimation.h"
#include "..\DataObjects\EUnitAppearanceType.h"

/////////////////////////////////////////////////////////////////////////////
// CUnitAnimationPreview window

class CUnitAnimationPreview : public CStatic
{
// Construction
public:
	CUnitAnimationPreview();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUnitAnimationPreview)
	//}}AFX_VIRTUAL

// Implementation
public:
	void SetAnimation(CEUnitAnimation *pAnimation);
	void Create(CRect &rcBound, DWORD dwStyle, CWnd *pParent, UINT nID, CEUnitAppearanceType *pAppearanceType, DWORD dwDirection);
	virtual ~CUnitAnimationPreview();

	// Generated message map functions
protected:
	//{{AFX_MSG(CUnitAnimationPreview)
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

private:
  CEUnitAppearanceType * m_pAppearanceType;
	DWORD m_dwFrame;
	UINT m_nTimer;
 	CDDrawClipper m_Clipper;
	CEUnitAnimation *m_pUnitAnimation;
	CScratchSurface m_Buffer;
  DWORD m_dwDirection;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_UNITANIMATIONPREVIEW_H__8CA351A6_92D4_11D3_A0C7_92DF3C7AE131__INCLUDED_)
