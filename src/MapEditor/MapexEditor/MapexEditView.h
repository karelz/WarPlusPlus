#if !defined(AFX_MAPEXEDITVIEW_H__2CD57B46_6CDC_11D3_A067_ED5A17535738__INCLUDED_)
#define AFX_MAPEXEDITVIEW_H__2CD57B46_6CDC_11D3_A067_ED5A17535738__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MapexEditView.h : header file
//

#include "..\DataObjects\LandType.h"
#include "..\DataObjects\EMapex.h"

/////////////////////////////////////////////////////////////////////////////
// CMapexEditView window

class CMapexEditView : public CWnd
{
// Construction
public:
	CMapexEditView();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMapexEditView)
	//}}AFX_VIRTUAL

// Implementation
public:
	void OnMapexGraphicsChanged();
	void SetCurrentLandType(CLandType *pLandType);
	void OnMapexSizeChanged();
	void Create(CEMapex *pMapex, CRect &rcBound, DWORD dwStyles, CWnd *pParent, UINT nID);
	virtual ~CMapexEditView();

  typedef enum tagEViewType{
    VIEW_BOTH = 0,
    VIEW_GRAPHICS = 1,
    VIEW_GRID = 2
  } EViewType;
	EViewType GetViewType();
	void SetViewType(EViewType eType);

	// Generated message map functions
protected:
	//{{AFX_MSG(CMapexEditView)
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
private:
	void ReallocateBuffers();
  EViewType m_eViewType;
  BOOL m_bMouseDrag;
	DWORD m_dwLastYPos;
	DWORD m_dwLastXPos;
	CLandType * m_pCurrentLandType;
	CScratchSurface m_TransBuffer;
	CDDrawClipper m_Clipper;
	CEMapex * m_pMapex;
	CScratchSurface m_Buffer;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAPEXEDITVIEW_H__2CD57B46_6CDC_11D3_A067_ED5A17535738__INCLUDED_)
