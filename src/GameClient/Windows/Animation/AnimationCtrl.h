// AnimationCtrl.h: interface for the CAnimationCtrl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ANIMATIONCTRL_H__2E23CB76_9D8B_11D2_ABB2_CE8C1A7E5760__INCLUDED_)
#define AFX_ANIMATIONCTRL_H__2E23CB76_9D8B_11D2_ABB2_CE8C1A7E5760__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\WINDOW\Window.h"
#include "Animation.h"

class CAnimationCtrl : public CWindow  
{
  DECLARE_DYNAMIC(CAnimationCtrl);
  DECLARE_OBSERVER_MAP(CAnimationCtrl);

public:
	// sets the current frame (index)
  // if the index is not in the animation
  // no image will be drawn
  void SetFrame(DWORD dwFrame);
  // returns index of current frame
	DWORD GetFrame();
	// returns the pointer to the animation object
  CAnimation * GetAnimation();
  // stops playing the animation
	void Stop();
  // starts playing the animation
	void Play();
	// just draws the content
  // called from the window system
  virtual void Draw(CDDrawSurface *pSurface, CRect *pRectBound);
  // deletes the object
  virtual void Delete();
  // creates the animation control
  // from animation object
  // if the pSize is NULL the size will be the same as
  // the one of the largest animation frame
  BOOL Create(CPoint point, CAnimation *pAnimation, CWindow *pParent, CSize *pSize = NULL);
	// constructs the object
  CAnimationCtrl();
  // destroyes the object
	virtual ~CAnimationCtrl();

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

private:
  // animation object which is shown here
  CAnimationInstance m_Animation;
protected:
	void OnAnimsRepaint(CAnimationInstance *pAnimation);
  // reaction on the timer event
};

#endif // !defined(AFX_ANIMATIONCTRL_H__2E23CB76_9D8B_11D2_ABB2_CE8C1A7E5760__INCLUDED_)
