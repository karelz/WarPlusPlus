// ScrollControl.h: interface for the CScrollControl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SCROLLCONTROL_H__22FDD4EA_5081_11D3_A02E_DD87D6C1E031__INCLUDED_)
#define AFX_SCROLLCONTROL_H__22FDD4EA_5081_11D3_A02E_DD87D6C1E031__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\..\Window.h"
#include "ScrollControlLayout.h"

class CScrollControl : public CWindow  
{
  DECLARE_DYNAMIC(CScrollControl);
  DECLARE_OBSERVER_MAP(CScrollControl);

public:
	int GetRangeMax();
	int GetRangeMin();
	int GetPageSize();
	int GetPosition();
	void SetPosition(int nPosition);
	void SetPageSize(int nPageSize);
	void SetRange(int nRangeMin, int nRangeMax);
  // draws the window -> called from the windows system
	virtual void Draw(CDDrawSurface *pSurface, CRect *pRectBound);
  // deletes the object
	virtual void Delete();
  // creates the object
	BOOL Create(CRect &rcBound, CScrollControlLayout *pLayout, CWindow *pParent, BOOL bHorizontal = FALSE);

  // constructor
	CScrollControl();
  // destructor
	virtual ~CScrollControl();

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

  enum Events{
    E_STEPUP = 1,   // one step up
    E_STEPDOWN = 2, // one step down
    E_PAGEUP = 3,   // one page up
    E_PAGEDOWN = 4, // one page down
    E_POSITION = 5  // drag position change
  };

private:
	int m_nMiddleStartPos;
  // TRUE - if the mouse is draging some button in the scroll bar
	BOOL m_bMouseDrag;

  // returns TRUE - if the point is in the page down area
	BOOL InPageDown(CPoint &point);
  // returns TRUE - if the point is in the page up area
	BOOL InPageUp(CPoint &point);
  // returns TRUE - if the point is in the middle button
	BOOL InMiddleButton(CPoint &point);
  // returns TRUE - if the point is in the bottom button
	BOOL InBottomButton(CPoint &point);
  // returns TRUE - if the point is in the top button
	BOOL InTopButton(CPoint &point);
  // states of the buttons
  int m_nBottomButtonState; // 0 - passive
  int m_nMiddleButtonState; // 1 - active (mouse over)
  int m_nTopButtonState;    // 2 - pressed
  int m_nPageDownState;
  int m_nPageUpState;

  // sets the new state of button - returns TRUE if the
  // state has changed
	BOOL SetBottomButtonState(int nState);
	BOOL SetMiddleButtonState(int nState);
	BOOL SetTopButtonState(int nState);
	BOOL SetPageDownState(int nState);
	BOOL SetPageUpState(int nState);

  // animations for the drawing
  CAnimationInstance m_Top;
  CAnimationInstance m_Bottom;
  CAnimationInstance m_UpMiddle;
  CAnimationInstance m_DownMiddle;
  CAnimationInstance m_ButtonTop;
  CAnimationInstance m_ButtonMiddle;
  CAnimationInstance m_ButtonBottom;
  // the layout object
	CScrollControlLayout * m_pLayout;
protected:
	virtual void OnSize(CSize size);
	BOOL m_bHorizontal;
	void OnTimeTick(DWORD dwTime);
	virtual void OnPageDown();
	virtual void OnPageUp();
	virtual void OnStepDown();
	virtual void OnStepUp();
  // reactions on notifications and events
	virtual void OnDeactivate();
	void OnMouseLeave();
	void OnMouseMove(CPoint point);
	void OnLButtonUp(CPoint point);
	void OnLButtonDown(CPoint point);
	void OnAnimsRepaint(CAnimationInstance *pAnimation);

  // the button position
  CRect m_rcButton;
  // recompute the position of the button
	virtual void ComputePositions();
	// current position of the scroll bar button
  int m_nPosition;
  // size of the page (the button)
	int m_nPageSize;
  // minimal value
	int m_nRangeMin;
  // maximal value
	int m_nRangeMax;

  // time values for repeateing
  static DWORD RepeateBeginDelay;
  static DWORD RepeateDelay;
};

#define ON_STEPUP(fncname) \
  EVENT(CScrollControl::E_STEPUP) \
    fncname(); return FALSE;

#define ON_STEPDOWN(fncname) \
  EVENT(CScrollControl::E_STEPDOWN) \
    fncname(); return FALSE;

#define ON_PAGEUP(fncname) \
  EVENT(CScrollControl::E_PAGEUP) \
    fncname(); return FALSE;

#define ON_PAGEDOWN(fncname) \
  EVENT(CScrollControl::E_PAGEDOWN) \
    fncname(); return FALSE;

#define ON_POSITION(fncname) \
  EVENT(CScrollControl::E_POSITION) \
    fncname((int)dwParam); return FALSE;

#endif // !defined(AFX_SCROLLCONTROL_H__22FDD4EA_5081_11D3_A02E_DD87D6C1E031__INCLUDED_)
