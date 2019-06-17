// ScreenWindow.h: interface for the CScreenWindow class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SCREENWINDOW_H__48895113_01AC_11D4_847F_004F4E0004AA__INCLUDED_)
#define AFX_SCREENWINDOW_H__48895113_01AC_11D4_847F_004F4E0004AA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// Window used for representing the whole screen parent
// like:
//   Script editor is all child for one of the screen window - the script editor screen window
// The window is completely transparent - invisible
class CScreenWindow : public CWindow  
{
  DECLARE_DYNAMIC(CScreenWindow);

public:
  // constructor & destructor
	CScreenWindow();
	virtual ~CScreenWindow();

// debug functions
#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

// Creation
  // creates the window -> it will be full screen
  BOOL Create(CWindow *pParent);
  // Deletes the window
  virtual void Delete();

// Drawing
  virtual void Draw(CDDrawSurface *pSurface, CRect *pBoundRect);
};

#endif // !defined(AFX_SCREENWINDOW_H__48895113_01AC_11D4_847F_004F4E0004AA__INCLUDED_)
