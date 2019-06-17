// Window.h: interface for the CWindow class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WINDOW_H__BA168970_79A0_11D2_AB55_B62502B5AF61__INCLUDED_)
#define AFX_WINDOW_H__BA168970_79A0_11D2_AB55_B62502B5AF61__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "UpdateRectList.h"

// window class
// !! always has a connection to Mouse and Keyboard

class CMouse;
class CKeyboard;
class CCursor;

// we have to derive our object from CNotifier, because
// our childrens wouldn't be able to send events
class CWindow : public CNotifier
{
  DECLARE_DYNAMIC(CWindow)
  DECLARE_OBSERVER_MAP(CWindow)

public:
	void UpdateNow();
  // sets the focus to the previous window (in the tab stop order)
	void SetFocusToPrev();
  // sets the focus to the next window (in the tab stop order)
	void SetFocusToNext();

  // Hides the window
  void HideWindow();
  /// Shows the window
  void ShowWindow();
  // returns TRUE if the window is visible
  BOOL IsVisible();

  // returns TRUE if the mouse is above the window
	BOOL IsMouseAbove(CPoint point);
  // sets cursor which will be used if the mouse
  // is above this window (can be NULL)
	CCursor * SetWindowCursor(CCursor *pCursor);

  // enables or disables the window
  void EnableWindow(BOOL bEnable = TRUE);
  // returns TRUE if the window is enabled
  BOOL IsEnabled(){ return m_bEnabled; }

  // return TRUE if the window can be accessed by mouse or so
  // If FALSE it can be enabled but hidden or something
  BOOL IsAccesable(){ return m_bInternalyEnabled && !m_bInternalyHidden && !m_bHidden; }

  // Returns true if the window is already deleted, or if it has not yet been created
  bool IsDeleted () { return m_bDeleted ? true : false; }
  // Returns true if the window is active
  bool IsActive () { return m_bActive ? true : false; }

  // returns the parent window
  CWindow *GetParentWindow(){ return m_pParent; }

  // sets the focus to this window
	void SetFocus();
  // Returns wheter the window has the focus or not
  BOOL HasFocus();
  // returns window with focus (can be NULL)
  CWindow *GetFocusWindow(){ return m_pFocusWindow; }

  // Return client rectangle
	CRect * GetClientRect();
  // Brings the window to the top of all windows
	void BringToTop(BOOL bMakeTopMost);
  // Activates the window
	BOOL Activate();

  // Releases mouse capture
	void ReleaseCapture();
  // Sets the mouse capture to this window
	CWindow * SetCapture();

  // Set Mouse clip rectangle
  // if pRect is NULL -> whole window rect is used
  CRect SetMouseClip(CRect *pRect = NULL);
  // releases the Mouse clip rect
  void ReleaseMouseClip();

  // return window rectangle (position)
	virtual CRect GetWindowPosition();
  // returns teh window position on the screen
  virtual CRect GetWindowScreenPosition(){ return m_rectWindow; }
  // sets the window position (rectangle)
	virtual void SetWindowPosition(CRect *pRect);
  // sets the window position (point)
	virtual void SetWindowPosition(CPoint *pPoint);

  // Updates the rectangle in the window
	void UpdateRect(CRect *pRect = NULL);

  // Draws a piece of window
	virtual void Draw(CDDrawSurface *pDDSurface, CRect *pBoundRect);


  // Deletes the window
	virtual void Delete();
  // Creates the window
	virtual BOOL Create(CRect *pWindowRect, CWindow *pParent);



  // Returns the window from givven screen point
  // works only on subtree of windows
	virtual CWindow * WindowFromPoint(CPoint &pt);


  // Returns first child window of this window
  CWindow *GetFirstChild(){ return m_pFirstChild; }
  // Returns next child window
  CWindow *GetNextChild(CWindow *pWindow){ return pWindow->m_pNextSibling; }

  // Freezes position of all windows
  static void FreezeWindows(){ VERIFY(m_lockWindowsPosition.Lock()); }
  // Unfreezes the position of all windows
  static void UnfreezeWindows(){ VERIFY(m_lockWindowsPosition.Unlock()); }

  // sets buffering for this window -> if set the window can't be transparent
  // You can set the  size of the buffer - the window can't be larger than that size
  void SetBuffering(DWORD dwXSize = 0x0FFFF, DWORD dwYSize = 0x0FFFF);
  // Stops buffering for this window
  void StopBuffering();

  // Sets the window to be the TopMost window
  void SetTopMost(BOOL bTopMost = TRUE){ m_bTopMost = bTopMost; }

  // draws all neede updates in the windows subsystem
  // to the specified surface (it's assumed that it's whole screen surface)
  // But this function do it carefully, it means it buffers the drawing if soem
  // window is not buffer by itself
  // pStartFromWindow parameter specifies the window who's children we'll test for buffering
  // if NULL -> the desktop window is used
  static void UpdateNowBuffered(CDDrawSurface *pDestination, CWindow *pStartFromWindow = NULL);

  // Clears the list of updates
  static void ValidateAll();

  // enables/disables updating of windows (if disabled no windows will be drawn)
  static void EnableWindowsUpdating(BOOL bEnabled = TRUE);
  // draws all children of this window into given surface (it's assumed to be the fullscreen surface)
  void DrawChildren(CDDrawSurface * pSurface);

	// constructor
  CWindow();
  // destructor
	virtual ~CWindow();

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

private:
	void OnUpdateNotification();
	// inserts new rect no update to the update list
  enum{ Update_Notifier = 0x0FFFF0000 };
  enum{ E__UPDATE = 0x01 };

  // the list of rects to update
  static CUpdateRectList *m_pUpdateList;
  static CMutex m_semaphoreUpdateList;
  static BOOL m_bUpdatingEnabled;

  // Lock that if it's down no window can be moved
  static CMutex m_lockWindowsPosition;

	// cursor for this window
  CCursor * m_pCursor;
  // returns TRUE if any of our children has the focus
	BOOL FindTheFocus();
  // The only one pointer to the window with focus
  // this is like a global variable
  static CWindow * m_pFocusWindow;
	// Previous window in the Tab order
  CWindow * m_pPrevTabWindow;
	// next window in the Tab order
  CWindow * m_pNextTabWindow;
  // return previous window in the tab order
	CWindow * GetPrevTabWindow();
  // returns next window, which has the m_bTabStop set and
  // is next in the tab order
  // all of this is meant on children
  // This function will be called by our children
  CWindow * GetNextTabWindow();
	// TRUE - window has the focus
  BOOL m_bFocus;
  // Sets the position of the window and all it's children
	void SetThePosition(CRect *pRect);
  // Deactivates the window
	void DoDeactivation();
  // Activates the window
	CWindow * DoActivation(BOOL &bWantActivationEvent);

  // TRUE - window has mouse capture
	BOOL m_bHasCapture;
  // Previous window which had the mouse capture
	CWindow * m_pPrevCaptureWindow;

  // the previous clipping rectangle
  CRect m_rcPrevMouseClipRect;

  // The only one window buffer surface
  // used by the desktop window
	static CDDrawSurface * m_pWindowsBufferSurface;
  // updates the rectangle
	void DoUpdateRect(CDDrawSurface *pSurface, CRect *pRect);
  // Next window in some lists
	CWindow * m_pNextWindow;
  // TRUE if the window was deleted before
	BOOL m_bDeleted;
  // Parent window
	CWindow * m_pParent;
  // Next sibling window
  CWindow *m_pNextSibling;
  // Previous sibling window
  CWindow *m_pPrevSibling;
  // First child of this window
  CWindow *m_pFirstChild;
  // Last child of this window
  CWindow *m_pLastChild;

  // TRUE - the window is enabled
  // FALSE - the window is internaly disabled
  // for example all child windows of disabled window are internaly disabled
  // if the m_bEnabled is FALSE -> this is also FALSE
  // but if the m_bEnabled is TRUE this can be FALSE or TRUE
  // this variable is used to determine if the input is about to go to this window or not
  BOOL m_bInternalyEnabled;

  // TRUE - the window is internaly hidden, it means that the window is normaly hidden
  // or that on eof his parents is normaly hidden
  // FALSE - the windows is not hidden -> it is visible
  BOOL m_bInternalyHidden;

  // enables or disables internaly this window and all its childrens
  void InternalEnableWindow(BOOL bEnable);

  // Hides or unhides internaly the window (and all its children)
  void InternalyHideWindow ( BOOL bHide );

  // window position (screen coords)
  CRect m_rectWindow;
  // client rectangle (window coords)
  CRect m_rectClient;

  // invalid rectangle for this window
  CRect m_rectInvalid;
  // sets invalid rect for this window
  void InvalidateRect(CRect &rcInvalid);

  // called to draw the window (here we will intercept if the window is buffered)
  void DoDraw(CDDrawSurface *pSurface, CRect *pRect, BOOL bUpdatingThis);

  // buffer for buffered windows
  CScratchSurface *m_pGraphicalBuffer;
  // TRUE if buffering is on
  BOOL m_bBuffering;


  friend CMouse;
  friend CKeyboard;
  friend class CKeyAccelerators;
protected:
	virtual void OnInternalDisableWindow();
	virtual void OnInternalEnableWindow();

  void SetBufferingInternal(DWORD dwXSize = 0x0FFFF, DWORD dwYSize = 0x0FFFF);
  void StopBufferingInternal();
  BOOL IsBuffered(){ return m_bBuffering; }

  // notifications
  // called when the window is disabled
  virtual void OnDisableWindow();
  // called when the window is enabled
  virtual void OnEnableWindow();

  // called when the position of the window has changed
  virtual void OnMove(CPoint point);
  // called when the size of the window has changed
	virtual void OnSize(CSize size);

  // TRUE - window wants doubleclicks
  // FALSE - all clicks are normal (no doubleclicks)
	BOOL m_bDoubleClk;

  // TRUE - the window is hidden - not visible, aplies to all children
  BOOL m_bHidden;

  // reaction on the KeyUp event -> manages the TAB keys
	BOOL OnKeyUp(UINT nChar, DWORD dwFlags);
  // handler for the KeyDown message
  BOOL OnKeyDown(DWORD dwKey, DWORD dwFlags);

  // called when the window loses focus
	virtual void OnLoseFocus();
  // called when the window gaines the focus
	virtual void OnSetFocus();
  // TRUE - the window can have the focus
  BOOL m_bTabStop;
	// TRUE - the window is the topmost window
  BOOL m_bTopMost;
  // TRUE - the window is active
	BOOL m_bActive;

  // TRUE - the window is enabled - it means the keyboard and mouse events are sent to it
  // FALSE - it's disabled - no mouse or keyboard input
  BOOL m_bEnabled;

  // Called when the window is activated
  // return TRUE if you want to recieve the event from mouse which
  // activated this window (if there was such event)
  // return FALSE if don't want to recieve the mouse event
  // but you still recieve other events, that could activate the window
	virtual BOOL OnActivate();
  // Called when the window is deactivated
	virtual void OnDeactivate();
  
  // Determines what windows has to be draw in givven rect
	virtual BOOL WhatToDraw(CRect *pRect, CWindow **pWindowsList, BOOL bUpdatingThis);

	// Tries to set focus to this window
  BOOL TrySetFocus();

  // TRUE - the window is transparent
  // it means everything under this window has to be redrawn
  // when this window is redrawn
	BOOL m_bTransparent;

  friend class CDialogWindow;
};

// The desktop window
extern CWindow *g_pDesktopWindow;

#endif // !defined(AFX_WINDOW_H__BA168970_79A0_11D2_AB55_B62502B5AF61__INCLUDED_)
