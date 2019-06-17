// TextButton.h: interface for the CTextButton class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TEXTBUTTON_H__901A9614_DAF4_11D2_AC1E_F47F5EE3BD61__INCLUDED_)
#define AFX_TEXTBUTTON_H__901A9614_DAF4_11D2_AC1E_F47F5EE3BD61__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ButtonObject.h"
#include "TextButtonLayout.h"

class CTextButton : public CButtonObject  
{
  DECLARE_DYNAMIC(CTextButton)
  DECLARE_OBSERVER_MAP(CTextButton)

public:
	virtual CSize GetVirtualSize();
	void SetText(CString strText);
	CString GetText();
  // deletes the object
	virtual void Delete();
  // draws the window (button)
	virtual void Draw(CDDrawSurface *pSurface, CRect *pRect);
  // creates the button from the point
  // it computes the size from givven text and layout of the window
	BOOL Create(CPoint ptPosition, CTextButtonLayout *pLayout, CString strText, CWindow *pParent, BOOL bRepeate = FALSE);
  // creates the button from the reactangle
	BOOL Create(CRect *pRect, CTextButtonLayout *pLayout, CString strText, CWindow *pParent, BOOL bRepeate = FALSE);
	// constructor
  CTextButton();
  // destructor
	virtual ~CTextButton();

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

private:
  // chooses the layout of the button (for current state)
  // fills the m_Left, m_Right, m_Middle animation instances
	void ChooseLayout();
  // computes the position of the text and animations
  // for current state of the button
	void ComputePositions();
  // position of the text in the button
	CPoint m_ptTextPos;
  // size of the resizeable middle section
	CSize m_sizeMiddle;
  // size of the whole button
	CSize m_sizeWhole;
  // color of the text
	DWORD m_dwTextColor;
  // the button text
	CString m_strText;
  // pointer to the layout object
	CTextButtonLayout * m_pLayout;
  // instance of the right animation
	CAnimationInstance m_Right;
  // instance of the middle animation
	CAnimationInstance m_Middle;
  // instance of the left animation
	CAnimationInstance m_Left;
protected:
  // reaction for the change of the layout
  // chooses the animations, recomputes positions and redraws it
	virtual void OnLayoutChange();
  // reaction for releasing the button layout
  // plays the sound
	virtual void OnLayoutRelease();
  // reaction for pressing the button layout
  // plays the sound
	virtual void OnLayoutPress();
  // reaction for activating the button
  // playes the sound
	virtual void OnMouseDeactivate();
  // reaction for deactivating the button
  // playes the sound
	virtual void OnMouseActivate();
  // reaction for animation moving to the next frame
  // redraws the area of this animation
	void OnAnimsRepaint(CAnimationInstance *pAnimation);
  // reaction on size change (of the window)
  void OnSize(CSize size);
};

#endif // !defined(AFX_TEXTBUTTON_H__901A9614_DAF4_11D2_AC1E_F47F5EE3BD61__INCLUDED_)
