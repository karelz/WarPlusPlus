// MapViewerView.h : interface of the CMapViewerView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAPVIEWERVIEW_H__018E9271_7289_11D4_AB67_004F4906CFC9__INCLUDED_)
#define AFX_MAPVIEWERVIEW_H__018E9271_7289_11D4_AB67_004F4906CFC9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CMapViewerView : public CScrollView
{
protected: // create from serialization only
	CMapViewerView();
	DECLARE_DYNCREATE(CMapViewerView)

// Attributes
public:
	CMapViewerDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMapViewerView)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual void OnInitialUpdate(); // called first time after construct
	virtual void OnDraw(CDC* pDC);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMapViewerView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	void DrawRect ( CRect &rectUpdate );
	void UpdateScroll ( CPoint pt );
	UINT m_nTimerID;
	BOOL m_bDrawView;
	CFont m_cFont;

// Generated message map functions
protected:
	//{{AFX_MSG(CMapViewerView)
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnPaint();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnToggleView();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in MapViewerView.cpp
inline CMapViewerDoc* CMapViewerView::GetDocument()
   { return (CMapViewerDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAPVIEWERVIEW_H__018E9271_7289_11D4_AB67_004F4906CFC9__INCLUDED_)
