// Cursor.h: interface for the CCursor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CURSOR_H__BE6D3E0B_9CA0_11D2_ABB1_BFAA62284960__INCLUDED_)
#define AFX_CURSOR_H__BE6D3E0B_9CA0_11D2_ABB1_BFAA62284960__INCLUDED_

#include "..\Animation\Animation.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMouse;

class CCursor : public CObject  
{
public:
  // deletes the object
	void Delete();
  // creates the object from image/anim file
	BOOL Create(CArchiveFile File);
  // constructor
	CCursor();
  // destructor
	virtual ~CCursor();

private:
  // X coord of the hot spot
	DWORD m_dwHotY;
  // Y coord of the hot spot
	DWORD m_dwHotX;
  // current frame of the animation
	DWORD m_dwFrame;
  // animation - cursor layout
	CAnimation m_Animation;
protected:
  // returns delay between two frames
	DWORD GetElapseTime();
  // returns hot spot Y coord
	DWORD GetHotY();
  // returns hot spot X coord
	DWORD GetHotX();
  // returns current frame of the cursor
	CDDrawSurface * GetCursorBitmap();
	// return TRUE - want to change the cursor bitmap
  // return FALSE - no change
  BOOL OnTimeTick(DWORD dwTime);

  friend CMouse;
};

#endif // !defined(AFX_CURSOR_H__BE6D3E0B_9CA0_11D2_ABB1_BFAA62284960__INCLUDED_)
