#if !defined(AFX_MINIMAPFRAME_H__9CFA5665_7984_11D3_A082_DF16643F0638__INCLUDED_)
#define AFX_MINIMAPFRAME_H__9CFA5665_7984_11D3_A082_DF16643F0638__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MiniMapFrame.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMiniMapFrame frame

class CMiniMapFrame : public CMiniFrameWnd
{
	DECLARE_DYNCREATE(CMiniMapFrame)
protected:

// Attributes
public:
	CMiniMapFrame();           // protected constructor used by dynamic creation
	virtual ~CMiniMapFrame();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMiniMapFrame)
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMiniMapFrame)
	afx_msg void OnDestroy();
	afx_msg void OnClose();
	afx_msg void OnPaint();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void SetViewPosition(CPoint point);
	CRect m_rcMap;
	CPoint m_ptDragStart;
	HCURSOR m_hMoveCursor;
	int m_nMouseDrag;
	CDDrawClipper m_Clipper;
  CScratchSurface m_VerticalLine;
  CScratchSurface m_HorizontalLine;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MINIMAPFRAME_H__9CFA5665_7984_11D3_A082_DF16643F0638__INCLUDED_)
