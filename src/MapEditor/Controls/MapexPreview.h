#if !defined(AFX_MAPEXPREVIEW_H__2CD57B45_6CDC_11D3_A067_ED5A17535738__INCLUDED_)
#define AFX_MAPEXPREVIEW_H__2CD57B45_6CDC_11D3_A067_ED5A17535738__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MapexPreview.h : header file
//

#include "..\DataObjects\EMapex.h"

/////////////////////////////////////////////////////////////////////////////
// CMapexPreview window

class CMapexPreview : public CStatic
{
// Construction
public:
	CMapexPreview();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMapexPreview)
	//}}AFX_VIRTUAL

// Implementation
public:
	void SetMapex(CEMapex *pMapex);
	void Create(CRect &rcBound, DWORD dwStyle, CWnd *pParent, UINT nID);
	virtual ~CMapexPreview();

	// Generated message map functions
protected:
	//{{AFX_MSG(CMapexPreview)
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
private:
	CDDrawClipper m_Clipper;
	CEMapex * m_pMapex;
	CScratchSurface m_Buffer;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAPEXPREVIEW_H__2CD57B45_6CDC_11D3_A067_ED5A17535738__INCLUDED_)
