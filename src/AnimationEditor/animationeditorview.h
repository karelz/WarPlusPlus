// AnimationEditorView.h : interface of the CAnimationEditorView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_ANIMATIONEDITORVIEW_H__686491BA_AFFF_4D2F_BB9C_F5F35B1E4682__INCLUDED_)
#define AFX_ANIMATIONEDITORVIEW_H__686491BA_AFFF_4D2F_BB9C_F5F35B1E4682__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CAnimationEditorView : public CScrollView
{
protected: // create from serialization only
	CAnimationEditorView();
	DECLARE_DYNCREATE(CAnimationEditorView)

// Attributes
public:
	CAnimationEditorDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAnimationEditorView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual void OnInitialUpdate(); // called first time after construct
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CAnimationEditorView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

  // Sets current frame index
  void SetCurrentFrame ( DWORD dwCurrentFrame );
  // Returns index of the current frame
  DWORD GetCurrentFrame () const { return m_dwCurrentFrame; }

  DWORD GetLastCropTolerance () const { return m_dwLastCropTolerance; }
  void SetLastCropTolerance ( DWORD dwCropTolerance ) { m_dwLastCropTolerance = dwCropTolerance; }

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CAnimationEditorView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnViewOptions();
	afx_msg void OnUpdateAnimationPlay(CCmdUI* pCmdUI);
	afx_msg void OnUpdateAnimationStop(CCmdUI* pCmdUI);
	afx_msg void OnUpdateAnimationFirst(CCmdUI* pCmdUI);
	afx_msg void OnUpdateAnimationLast(CCmdUI* pCmdUI);
	afx_msg void OnUpdateAnimationNext(CCmdUI* pCmdUI);
	afx_msg void OnUpdateAnimationPrev(CCmdUI* pCmdUI);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnAnimationFirst();
	afx_msg void OnAnimationPrev();
	afx_msg void OnAnimationNext();
	afx_msg void OnAnimationLast();
	afx_msg void OnAnimationPlay();
	afx_msg void OnAnimationStop();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnUpdateFrameMoveLeft(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFrameMoveRight(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFrameDelete(CCmdUI* pCmdUI);
	afx_msg void OnFrameMoveLeft();
	afx_msg void OnFrameMoveRight();
	afx_msg void OnUpdateFrameInsert(CCmdUI* pCmdUI);
	afx_msg void OnFrameDelete();
	afx_msg void OnFrameInsert();
	afx_msg void OnUpdateFrameDelay(CCmdUI* pCmdUI);
	afx_msg void OnUpdateAnimationEditOffset(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFrameEditOffset(CCmdUI* pCmdUI);
	afx_msg void OnAnimationEditOffset();
	afx_msg void OnFrameEditOffset();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnUpdateFrameCrop(CCmdUI* pCmdUI);
	afx_msg void OnFrameCrop();
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
  // View state
  enum EViewState
  {
    State_Edit,
    State_Preview,
  };

  EViewState m_eState;

  // Clipper for this window
  CDDrawClipper m_cClipper;

  // Current frame index
  DWORD m_dwCurrentFrame;

  // Set to true after the view is created
  bool m_bCreated;

  // Draw buffer
  CScratchSurface m_cDrawBuffer;

  // Color to draw the background with
  DWORD m_dwBackgroundColor;
  // Animation file path for bkg animation
  CString m_strBackgroundAnimationFilePath;
  // Background animation
  CAAnimation m_cBackgroundAnimation;
  // True if the bkg animation is loaded successfully
  bool m_bUseBackgroundAnimation;
  // True if the background should be drawen as overlay :-)
  bool m_bDisplayBackgroundAsOverlay;

  // The animation color
  DWORD m_dwAnimationColor;
  // True if we're about to draw the cross hair overlay
  bool m_bShowCrossHair;
  // The color to show cross hair with
  DWORD m_dwCrossHairColor;

  // True if we're about to draw bounding rectangle
  bool m_bShowBoundingRectangle;

  // Horizontal line of the cross hair
  CScratchSurface m_cCrossHairHorizontalLine;
  // Vertical line of the cross hair
  CScratchSurface m_cCrossHairVerticalLine;

  // The offset buttons
  CImageSurface m_cMoveUpButton;
  CImageSurface m_cMoveLeftButton;
  CImageSurface m_cMoveRightButton;
  CImageSurface m_cMoveDownButton;
  CPoint m_ptMoveUpButton, m_ptMoveLeftButton, m_ptMoveRightButton, m_ptMoveDownButton;
  // true if the move buttons should be visible
  bool m_bShowMoveButtons;
  // true if the animation offset is editting, false if the frame offset is editting
  // (To enable editting of offsets, you also must set the m_bShowMoveButtons to true)
  bool m_bEditAnimationOffset;
  // If zero - no button is hovered
  // 1, 2, 3, 4 - Up, Left, Down, Right
  int m_nHoverButton;
  // If zero - no button is pressed
  // 1, 2, 3, 4 - Up, Left, Down, Right
  int m_nPressedButton;
  // Timer for the move buttons
  int m_nMoveTimer;

  // Timer ID for frame timing
  int m_nFrameTimer;

  // Last crop tolerance
  DWORD m_dwLastCropTolerance;

// Helper functions
  // Centres the view
  void CenterView ();
  // Draws given rectangle to the given surface
  // The rectangle is given in coords relative to the center of the view (which is always 0, 0 )
  void DrawRect ( CRect * pRect, CDDrawSurface * pSurface );
  // Draws given rect in window coords to the window
  void DrawClientRect ( CRect * pRect );
  // Sets timer for next frame in the animation (used in preview mode)
  void SetNextFrameTimer ();

  // Recomputes move buttons positions
  void RecomputeMoveButtons ();

  // Returns button under the mouse position
  // Zero - no, 1, 2, 3, 4 - Up, Left, Down, Right
  int GetButtonUnderMouse ();

  // Updates state of the buttons from mouse position
  void UpdateHoverButton ();

  // Performs the move operation
  void DoMoving ();
};

#ifndef _DEBUG  // debug version in AnimationEditorView.cpp
inline CAnimationEditorDoc* CAnimationEditorView::GetDocument()
   { return (CAnimationEditorDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ANIMATIONEDITORVIEW_H__686491BA_AFFF_4D2F_BB9C_F5F35B1E4682__INCLUDED_)
