// MapEditorView.h : interface of the CMapEditorView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAPEDITORVIEW_H__43E086CF_6434_11D3_A054_ADE3A89A0638__INCLUDED_)
#define AFX_MAPEDITORVIEW_H__43E086CF_6434_11D3_A054_ADE3A89A0638__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CMapEditorView : public CScrollView
{
protected: // create from serialization only
	CMapEditorView();
	DECLARE_DYNCREATE(CMapEditorView)

// Attributes
public:
	CMapEditorDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMapEditorView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnInitialUpdate();
	virtual BOOL OnScroll(UINT nScrollCode, UINT nPos, BOOL bDoScroll = TRUE);
	protected:
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
public:
	void GetULMapexViewRect(CEULMapexInstance *pMapexInstance, CRect *pRect);
	void GetSelectionViewRect(CMapexInstanceSelection *pSelection, CRect *pRect);
	void SetVisibleRect(CRect rcRect);
	void GetMapexViewRect(CEMapexInstance *pMapexInstance, CRect *pRect);
  void GetUnitViewRect(CEUnit *pUnit, CRect *pRect);
	void DrawRect(CRect *pRect);
	virtual ~CMapEditorView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CMapEditorView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	CPoint m_ptDragTrans;
	BOOL m_bInitialized;
  BOOL m_bMoved;
	void UpdateVisibleRect();
	HCURSOR m_hMoveCursor, m_hSelectionCursor;
	int m_nDragging;
	CPoint GetMapCellCoords(DWORD dwX, DWORD dwY);
	CPoint GetMapPosition(CPoint point);
	CDDrawClipper m_Clipper;
	CScratchSurface m_Buffer;
};

#ifndef _DEBUG  // debug version in MapEditorView.cpp
inline CMapEditorDoc* CMapEditorView::GetDocument()
   { return (CMapEditorDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAPEDITORVIEW_H__43E086CF_6434_11D3_A054_ADE3A89A0638__INCLUDED_)
