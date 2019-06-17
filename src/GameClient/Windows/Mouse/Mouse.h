// Mouse.h: interface for the CMouse class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MOUSE_H__6AC28CE2_5B72_11D2_8EB5_947204C10000__INCLUDED_)
#define AFX_MOUSE_H__6AC28CE2_5B72_11D2_8EB5_947204C10000__INCLUDED_

#include "MouseNotifier.h"	// Added by ClassView
#include "MousePrimarySurface.h"
#include "MouseWindow.h"	// Added by ClassView
#include "Cursor.h"
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#define MOUSE_MAKEPOINT(XCoord, YCoord) ((XCoord & 0x0FFFF) + ((YCoord & 0x0FFFF) << 16))
#define MOUSE_X(dwParam) ((short int)(dwParam & 0x0FFFF))
#define MOUSE_Y(dwParam) ((short int)((dwParam & 0x0FFFF0000) >> 16))

class CWindow;

class CMouse : public CNotifier
{
  DECLARE_DYNAMIC(CMouse);
  DECLARE_OBSERVER_MAP(CMouse);

public:
	virtual void Disconnect(CObserver *pObserver);

  // Returns the waiting cursor
  CCursor * GetWaitingCursor(){ return m_pWaitingCursor; }
  // Sets new waiting cursor
  void SetWaitingCursor(CCursor *pWaitingCursor){ m_pWaitingCursor = pWaitingCursor; }

  void RestoreCursor();

  // sets the default cursor
  // this cursor will be used when the window
  // under the mouse doesn't set any cursor
	CCursor * SetDefaultCursor(CCursor *pCursor);
  // sets current cursor
	void SetCursor(CCursor *pCursor);
  // returns window which has the capture
  // NULL if none
	CWindow * GetCapture();
  // sets the window which is to have the capture
  // returns the previous one
	CWindow * SetCapture(CWindow *pNew);
  // hides the mouse cursor
	void HideMouse();
  // shows the mouse cursor
	void ShowMouse();
  // deletes mouse surface (detach from the primary surface)
  // you have to call this when exitting the application
  // and before you deletes the DirectDraw object
	void DeleteMouseSurface();
  // creates the mouse surface (attach to the primary surface)
  // you have to call this after creating the DirectDraw object
  // to have the mous functioning perfectly
	BOOL CreateMouseSurface(DWORD dwBackBufferCount = 0);
  // connects the window to the mouse
  // takes care about cursor position above windows
  // and their z order
	virtual void Connect(CWindow *pWindow, DWORD dwNotID = DefaultNotID);
  // connects any observer to the mouse
  // doesnt take care of anything
  // all events will be recieved
  // returns TRUE if the right button is pressed
	BOOL RightButtonDown();
  // returns TRUE if the left button is pressed
	BOOL LeftButtonDown();
  // returns the sensitivity of the mouse device
	void GetSensitivity(LPDWORD pXSens, LPDWORD pYSens);
  // sets the sensitivity of the mouse device
	void SetSensitivity(DWORD dwXSens, DWORD dwYSens);
  // sets the position of the cursor
	void SetPosition(CPoint *pPt);
  // returns the position of the cursor
	void GetPosition(CPoint *pPt);
  // returns the position of the cursor
	CPoint GetPosition();

  // sets the clipping rectangle (mouse can't go out of it)
  // if the mouse is'n in it, it will align on its next move
  void SetClipRect(CRect &rcClip);
  // returns the clipping rectangle
  CRect GetClipRect();

  // deletes the object
	void Delete();
  // static Close - deletes the global mouse object
	static void Close();
  // static Init - creates the global mouse object
  // the parametr should be the main app window
	static BOOL Init(CMouseWindow *pMouseWindow, BOOL bExclusive);
  // creates the object for the givven window
	BOOL Create(CMouseWindow *pMouseWindow, BOOL bExclusive);
  // constructor
	CMouse();
  // destructor
	virtual ~CMouse();

  enum { DefaultNotID = 0x0FFFF0002 };
  enum Events{
    E_MOVE = 1,
    E_LBUTTONDOWN = 2,
    E_LBUTTONUP = 3,
    E_LBUTTONDBLCLK = 4,
    E_RBUTTONDOWN = 5,
    E_RBUTTONUP = 6,
    E_RBUTTONDBLCLK = 7,
    E_LEAVE = 8
  };

  // biggest time between two clicks (in milliseconds) to
  // be recieved as a double click
  static DWORD DoubleClickTime;

#ifdef _DEBUG
  virtual void Dump(CDumpContext &dc) const;
  virtual void AssertValid() const;
#endif

private:
  struct tagSSequenceChanges{
    LONG m_lBigX; // perfect mouse coord X (in device units)
    LONG m_lBigY; // perfect mouse coord Y (in device units)
    BOOL m_bLeftButtonDown; // TRUE - left button is down
    BOOL m_bRightButtonDown; // TRUE - right button is down
  };
  typedef struct tagSSequenceChanges SSequenceChanges;

  enum { MouseNotifierID = 0x0EFFFFFFF };
private:
private:
	CMouseNotifier m_MouseNotifier;
	void OnMouseEvent(DWORD dwEvent, DWORD dwParam);

  // clipping rectangle
  CRect m_rcClip;

  // TRUE if the mouse is in exclusive mode
	BOOL m_bExclusive;
	// updates the cursor
  // resets the cursor layout to the parimary surface
  void UpdateCursor();
  // active cursor - visible on the screen
	CCursor * m_pActiveCursor;
  // default cursor, which is used when the window
  // doesn't sets its own
  CCursor * m_pDefaultCursor;
  // Waiting cursor
  CCursor * m_pWaitingCursor;
  // window which has the capture
	CWindow * m_pCaptureWindow;
  // detaches from the primary surface
	void DeleteGraphicMouse();
  // attach the primary surface
	BOOL CreateGraphicMouse();
  // deletes the DirectInput mouse device
  // stops the thread
	void DeleteDIDevice();
  // creates the DirectInput mouse device
  // and starts the thread for the catching events
	BOOL CreateDIDevice(CMouseWindow *pMouseWindow);
  // last window which recieved some message
	CWindow * m_pLastEventWindow;
  // time passed after last right click
	DWORD m_dwRButtonDownTime;
  // time passed after last left click
  DWORD m_dwLButtonDownTime;
  // demaphore for acces to the mouse object
  // does critical sections
	CSemaphore m_semaphoreMouse;
  // some helper
	BOOL m_bFirstAcquire;
  // inits the structure
	void InitSequenceChanges(SSequenceChanges *pSC);
  // writes changes in the structure to the object
  // moves the cursor and sends events
	void DoSequenceChanges(SSequenceChanges *pSC);
	// TRUE - right button is down
  BOOL m_bRightButtonDown;
  // TRUE - left button is down
	BOOL m_bLeftButtonDown;
  // sensitivity in Y coord of the device
	DWORD m_dwYSensitivity;
  // sensitivity in X coord of the device
	DWORD m_dwXSensitivity;
  // perfect Y coord of the cursor (in device units)
	LONG m_lBigY;
  // perfect X coord of the cursor (in device units)
	LONG m_lBigX;
  // Y coord of the cursor in pixels
	DWORD m_dwYCoord;
  // X coord of the cursor in pixels
	DWORD m_dwXCoord;
  // main app window which takes care about aquiring the mouse
	CMouseWindow *m_pMouseWindow;
  // thread which recievs the events from DirectInput
	CWinThread *m_pNotificationThread;
  // thread procedure
	static UINT MouseThreadProc(LPVOID lpParam);
  // event for closing the thread
  // if set the thread will exit immidietly
	CEvent m_eventClose;
  // pointer to the primary surface
	CMousePrimarySurface * m_pMousePrimarySurface;
  // DirectInput device for the mouse
	CDIMouseDevice m_MouseDevice;
  // event for DirectInput
  // set if some chage occured on the mouse
	CEvent m_eventNotification;

  friend CMouseWindow;
protected:
  // reaction on abnormal program termination
  // delets the object
	void OnAbort(DWORD dwExitCode);
  // reaction to timer tick
  // animates the cursor
	void OnTimeTick(DWORD dwTime);
  // removes additional informations from the connection
  // prepares whole event for notification
  // determines the window under cursor and so on
};
// global pointer to the mouse object
extern CMouse *g_pMouse;
#define BEGIN_MOUSE() BEGIN_NOTIFIER(CMouse::DefaultNotID)
#define END_MOUSE() END_NOTIFIER()
#define ON_MOUSEMOVE() \
  case CMouse::E_MOVE: \
    OnMouseMove(CPoint(MOUSE_X(dwParam), MOUSE_Y(dwParam))); \
    return TRUE;
#define ON_MOUSELEAVE() \
  case CMouse::E_LEAVE: \
    OnMouseLeave(); \
    return TRUE;
#define ON_LBUTTONDOWN() \
  case CMouse::E_LBUTTONDOWN: \
    OnLButtonDown(CPoint(MOUSE_X(dwParam), MOUSE_Y(dwParam))); \
    return TRUE;
#define ON_LBUTTONUP() \
  case CMouse::E_LBUTTONUP: \
    OnLButtonUp(CPoint(MOUSE_X(dwParam), MOUSE_Y(dwParam))); \
    return TRUE;
#define ON_LBUTTONDBLCLK() \
  case CMouse::E_LBUTTONDBLCLK: \
    OnLButtonDblClk(CPoint(MOUSE_X(dwParam), MOUSE_Y(dwParam))); \
    return TRUE;
#define ON_RBUTTONDOWN() \
  case CMouse::E_RBUTTONDOWN: \
    OnRButtonDown(CPoint(MOUSE_X(dwParam), MOUSE_Y(dwParam))); \
    return TRUE;
#define ON_RBUTTONUP() \
  case CMouse::E_RBUTTONUP: \
    OnRButtonUp(CPoint(MOUSE_X(dwParam), MOUSE_Y(dwParam))); \
    return TRUE;
#define ON_RBUTTONDBLCLK() \
  case CMouse::E_RBUTTONDBLCLK: \
    OnRButtonDblClk(CPoint(MOUSE_X(dwParam), MOUSE_Y(dwParam))); \
    return TRUE;

#endif // !defined(AFX_MOUSE_H__6AC28CE2_5B72_11D2_8EB5_947204C10000__INCLUDED_)
