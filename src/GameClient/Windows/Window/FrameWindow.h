// FrameWindow.h: interface for the CFrameWindow class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FRAMEWINDOW_H__14523514_E207_11D2_AC29_8B1CBE5F5660__INCLUDED_)
#define AFX_FRAMEWINDOW_H__14523514_E207_11D2_AC29_8B1CBE5F5660__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "FrameWindowLayout.h"
#include "Window.h"
#include "..\Animation\Animation.h"

class CFrameWindow : public CWindow  
{
  DECLARE_DYNAMIC(CFrameWindow);
  DECLARE_OBSERVER_MAP(CFrameWindow);

public:
	virtual void Delete();
	BOOL Create(CRect &rcBound, CFrameWindowLayout *pLayout, CWindow *pWindow, BOOL bResizeable);
	virtual void Draw(CDDrawSurface *pSurface, CRect *pBoundRect);
	CFrameWindow();
	virtual ~CFrameWindow();

  // This enables resizing of buffered windows
  // elseway the buffer must be set to the maximum size of the window
  void StopBufferingOnSize(BOOL bStopIt = TRUE){ m_bStopBufferingOnSize = bStopIt; }

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif
protected:
	virtual void OnEnableWindow();
	virtual void OnDisableWindow();
	BOOL m_bActiveFrame;
	virtual CSize GetMinimumSize();
  virtual CSize GetMaximumSize();
	virtual int InSizeArea(CPoint point);

  enum SizingMode{
    None = 0,
    LeftEdge = 1,
    RightEdge = 2,
    TopEdge = 3,
    BottomEdge = 4,
    TopLeftCorner = 5,
    BottomLeftCorner = 6,
    TopRightCorner = 7,
    BottomRightCorner = 8
  };

	virtual void OnSize(CSize size);
	virtual void OnDeactivate();
	virtual BOOL OnActivate();
	virtual BOOL InMoveArea(CPoint point);
	void OnLButtonUp(CPoint point);
	void OnLButtonDown(CPoint point);
	void OnMouseMove(CPoint point);
	void OnAnimsRepaint(CAnimationInstance *pAnimation);
	virtual void ChooseLayout();
	CRect m_rectWhole;
	CAnimationInstance m_TopLeft;
	CAnimationInstance m_Top;
	CAnimationInstance m_TopRight;
	CAnimationInstance m_Left;
	CAnimationInstance m_Middle;
	CAnimationInstance m_Right;
	CAnimationInstance m_BottomLeft;
	CAnimationInstance m_Bottom;
	CAnimationInstance m_BottomRight;

	CSize m_sizeMiddle;
	CPoint m_ptBottomRight;
	CPoint m_ptMiddle;
	virtual void ComputePositions();
	CFrameWindowLayout * m_pLayout;
private:
	CRect m_rectOldPos;
	int m_nSizing;
	BOOL m_bResizeable;
	CPoint m_ptStartPos;
	BOOL m_bMoving;
  BOOL m_bStopBufferingOnSize;
};

#endif // !defined(AFX_FRAMEWINDOW_H__14523514_E207_11D2_AC29_8B1CBE5F5660__INCLUDED_)
