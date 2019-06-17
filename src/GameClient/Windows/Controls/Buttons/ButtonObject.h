// ButtonObject.h: interface for the CButtonObject class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BUTTONOBJECT_H__5D3A6A97_C99E_11D2_AC00_BE4A29095160__INCLUDED_)
#define AFX_BUTTONOBJECT_H__5D3A6A97_C99E_11D2_AC00_BE4A29095160__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\..\Window\Window.h"

class CButtonObject : public CWindow
{
  DECLARE_DYNAMIC(CButtonObject);
  DECLARE_OBSERVER_MAP(CButtonObject);

public:
	virtual CSize GetVirtualSize();
	virtual void SetKeyAcc(UINT nChar, DWORD dwFlags);
	// deletes the object
  virtual void Delete();
  // creates the object
  // the parameter repeat tells if to repeat the pressing
	BOOL Create(CRect *pRect, CWindow *pParent, BOOL bRepeate = FALSE);
  // constructor
	CButtonObject();
  // destructor
	virtual ~CButtonObject();

  // Sets new sensitive rectangle (where the button should react on mouse messages)
  // It's given in client coords (for the button)
  // If the rect is empty, whole window will be sensitive
  void SetSensitiveRectangle(CRect *pNewRect);

  // Reaction on asking if the point is in the window
  virtual CWindow *WindowFromPoint(CPoint &pt);

#ifdef _DEBUG
  virtual void CButtonObject::AssertValid() const;
  virtual void CButtonObject::Dump(CDumpContext &dc) const;
#endif

  enum Events{
    E_PRESSED = 1, // sent if the button is pressed
    E_RELEASED = 2, // sent if the button is released
    E_COMMAND = 2 // the command from the button is equal with release
  };

protected:

	virtual void OnInternalDisableWindow();
	virtual void OnInternalEnableWindow();

	// reaction on the keyboard event
  BOOL OnKeyAccelerator(UINT nChar, DWORD dwFlags);
  // the flags for the hot key for our button
	DWORD m_dwKeyAccFlags;
  // the virtual key code for the hot key of our button
	UINT m_nKeyAccChar;

	// reaction on the keyboard event
  BOOL OnKeyUp(UINT nChar, DWORD dwFlags);
  // reaction on the keyboard event
	BOOL OnKeyDown(UINT nChar, DWORD dwFlags);
  // reaction for losing the focus
  // just calls the OnLayoutChange
	virtual void OnLoseFocus();
  // reaction for getting the focus
  // just calls the OnLayoutChange
	virtual void OnSetFocus();
  // virtual method - reaction for redrawing the layout
  // used by derived objects. Called if the layout changes
	virtual void OnLayoutChange();
  // reaction for timer - do the repeat presses
	void OnTimeTick(DWORD dwTime);
  // virtual - called when the button layout was released
	virtual void OnLayoutRelease();
  // virtual - called when the button layout was pressed
	virtual void OnLayoutPress();
  // TRUE if the button is pressed (his layout)
	BOOL m_bPressed;
  // reaction for decativating this window
	virtual void OnDeactivate();
  // reaction for activating this window
	virtual BOOL OnActivate();
  // TRUE if the button is activaed by mouse ( the cursor is on it)
	BOOL m_bMouseActive;
  // reaction for mouse LButtonUp event
	void OnLButtonUp(CPoint point);
  // reaction for mouse LButtonDown event
	void OnLButtonDown(CPoint point);
  // reaction for mouse Leave event
	void OnMouseLeave();
  // reaction for mouse Move event
	void OnMouseMove(CPoint point);
  // virtual - called when the button is released
  // the derived class has to call this method
  // this method send the event about releasing the button
	virtual void OnRelease();
  // virtual - called when the button is pressed
  // the derived class has to call this method
  // it sends the event about pressing the button
	virtual void OnPress();
  // virtual - called when the button loses his activation
  // by mouse (the cursor leaves its area)
	virtual void OnMouseDeactivate();
  // virtual - called when the button gaines the activation
  // by mouse (the cursor moves on it)
	virtual void OnMouseActivate();
private:
  // TRUE - if the mouse LButton is down and the button
  // has the mouse capture (something like draging)
	BOOL m_bMouseDrag;
  // TRUE - if the button is to repeate the presses
	BOOL m_bRepeate;

  // constant - delay between repeates of presses in milliseconds
  static DWORD RepeateDelay;
  // constant - delay before the first repeate of the press in milliseconds
  static DWORD RepeateBeginDelay;

  // Rectangle to accept mouse events from
  // This rectangle is seen from mouse point of view as our window
  // It's in client coords
  CRect m_rcSensitiveRectangle;
};

#define ON_BUTTONPRESSED(fncname) \
  EVENT(CButtonObject::E_PRESSED) \
    fncname(); return FALSE;

#define ON_BUTTONRELEASED(fncname) \
  EVENT(CButtonObject::E_RELEASED) \
    fncname(); return FALSE;

#define ON_BUTTONCOMMAND(fncname) \
  EVENT(CButtonObject::E_COMMAND) \
    fncname(); return FALSE;

#define THECOMMAND(id, fncname) \
  BEGIN_NOTIFIER(id) \
    ON_BUTTONCOMMAND(fncname) \
  END_NOTIFIER()

#endif // !defined(AFX_BUTTONOBJECT_H__5D3A6A97_C99E_11D2_AC00_BE4A29095160__INCLUDED_)
