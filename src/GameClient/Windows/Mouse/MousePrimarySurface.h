#if !defined(AFX_MOUSEPRIMARYSURFACE_H__330111A9_53C0_11D2_8EB5_947204C10000__INCLUDED_)
#define AFX_MOUSEPRIMARYSURFACE_H__330111A9_53C0_11D2_8EB5_947204C10000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// MousePrimarySurface.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMousePrimarySurface command target

class CMousePrimarySurface : public CDDPrimarySurface
{
	DECLARE_DYNAMIC(CMousePrimarySurface)

public:
  // constructor
  CMousePrimarySurface();           // protected constructor used by dynamic creation

// Attributes
public:

// Operations
public:

// Implementation
public:
	virtual void Restore();
	// does the flip operation
  // takes care about the cursor - no flicking
  virtual BOOL Flip();
  // does the paste operation
  // takes care about the cursor  - no flicking
	virtual void DoPaste(LONG lX, LONG lY, CDDrawSurface * pSrcSurface, CRect * pRectSource);
	// some after create operations
  // creates some buffer surfaces
  virtual BOOL PostCreate(LPDIRECTDRAWSURFACE lpDDSurface, BOOL bCallRelease = TRUE);
  // deletes the object
	virtual void Delete();
  // hides the mouse cursor
	void HideMouse();
  // static Close - deletes the  global object
	static void Close();
  // static Init - creates the global object
	static BOOL Init(CWnd *pClipWindow, DWORD dwBackBufferCount = 0);
  // sets the cursor position in pixels
  // doesn't affect the mouse object
	void SetMousePos(DWORD dwX, DWORD dwY);
  // sets current cursor layout
	void SetMouseCursor(CDDrawSurface *pMouseBitmap, DWORD dwHotX, DWORD dwHotY);
  // shows the mouse cursor
	void ShowMouse();
  // destructor
  virtual ~CMousePrimarySurface();

#ifdef _DEBUG
  virtual void Dump(CDumpContext &dc) const;
  virtual void AssertValid() const;
#endif

private:
  // pointer to the window in which is our primary surface
  // if in fullscreen should be NULL
	CWnd * m_pClipWindow;
  // semaphore for accesing the primary surface
  // does critical sections
	CMutex m_lockExclusive;
  // hot spot Y coord
	DWORD m_dwHotY;
  // hot spot X coord
	DWORD m_dwHotX;
  // moves the cursor without any flickering
	void MoveMouse(DWORD dwNewX, DWORD dwNewY);
  // removes the cursor from the screen
	void ClearMouse();
  // stores graphics under the cursor to the buffer
	void ScanMouse();
  // draws the cursor on the screen
	void DrawMouse();
  // buffer for storing graphics under the cursor
	CScratchSurface m_MouseBackground;
  // buffer for smooth moving
	CScratchSurface m_MoveSurface;
  // TRUE if the cursor is visible
	BOOL m_bVisible;
  // layout of the cursor
	CDDrawSurface * m_pMouseBitmap;
  // cursors X position in pixels
	DWORD m_dwMouseY;
  // cursors Y position in pixels
	DWORD m_dwMouseX;
};

extern CMousePrimarySurface *g_pMousePrimarySurface;

/////////////////////////////////////////////////////////////////////////////

#endif // !defined(AFX_MOUSEPRIMARYSURFACE_H__330111A9_53C0_11D2_8EB5_947204C10000__INCLUDED_)
