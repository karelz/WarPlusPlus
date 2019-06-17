#if !defined(AFX_UNITTYPEPREVIEW_H__F7C83743_9044_11D3_A878_00105ACA8325__INCLUDED_)
#define AFX_UNITTYPEPREVIEW_H__F7C83743_9044_11D3_A878_00105ACA8325__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// UnitTypePreview.h : header file
//

#include "..\DataObjects\EUnitType.h"

/////////////////////////////////////////////////////////////////////////////
// CUnitTypePreview window

class CUnitTypePreview : public CStatic
{
// Construction
public:
	CUnitTypePreview();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUnitTypePreview)
	//}}AFX_VIRTUAL

// Implementation
public:
	void SetUnitType(CEUnitType *pUnitType);
	void Create(CRect &rcBound, DWORD dwStyle, CWnd *pParent, UINT nID);
	virtual ~CUnitTypePreview();

	// Generated message map functions
protected:
	//{{AFX_MSG(CUnitTypePreview)
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

private:
	CDDrawClipper m_Clipper;
	CEUnitType * m_pUnitType;
	CScratchSurface m_Buffer;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_UNITTYPEPREVIEW_H__F7C83743_9044_11D3_A878_00105ACA8325__INCLUDED_)
