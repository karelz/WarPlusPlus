#if !defined(AFX_RESOURCEPREVIEW_H__25FAD958_557F_11D4_B0B1_004F49068BD6__INCLUDED_)
#define AFX_RESOURCEPREVIEW_H__25FAD958_557F_11D4_B0B1_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ResourcePreview.h : header file
//

#include "..\DataObjects\EResource.h"

/////////////////////////////////////////////////////////////////////////////
// CResourcePreview window

class CResourcePreview : public CStatic
{
// Construction
public:
	CResourcePreview();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CResourcePreview)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CResourcePreview();

  void Create(CRect &rcBound, DWORD dwStyle, CWnd *pParent, UINT nID);
  void SetResource(CEResource *pResource);

	// Generated message map functions
protected:
	//{{AFX_MSG(CResourcePreview)
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP();

private:
  CEResource *m_pResource;
	CScratchSurface m_Buffer;
	CDDrawClipper m_Clipper;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RESOURCEPREVIEW_H__25FAD958_557F_11D4_B0B1_004F49068BD6__INCLUDED_)
