// TextButtonLayout.h: interface for the CTextButtonLayout class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TEXTBUTTONLAYOUT_H__A8386F73_DF68_11D2_AC26_96E0A5B35760__INCLUDED_)
#define AFX_TEXTBUTTONLAYOUT_H__A8386F73_DF68_11D2_AC26_96E0A5B35760__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\..\Animation\Animation.h"
#include "..\..\Mouse.h"
#include "..\..\Fonts\FontObject.h"

class CTextButton;

class CTextButtonLayout : public CObject  
{
public:
  // returns the font for the button
	CFontObject * GetFont();
  // returns TRUE if any animation is not filled
  // or if any animation has some transparent pixel
	BOOL AreAnimsTransparent();
  // deletes the object
	virtual void Delete();
  // creates the object from the given config file
	virtual BOOL Create(CConfigFile *pCfgFile);
  // creates the object from the given config file
	virtual BOOL Create(CArchiveFile CfgFile);
  // creates empty object
	virtual BOOL Create();

  // constructor
	CTextButtonLayout();
  // destructor
	virtual ~CTextButtonLayout();

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

protected:
  // color of the pressed text
	DWORD m_dwPressedTextColor;
  // color of the active text
	DWORD m_dwActiveTextColor;
  // color of the passive text
	DWORD m_dwPassiveTextColor;
  // right margin from the text to the right edge of the button
  // for the pressed button (use the passive one)
	DWORD m_dwPressedRightMargin;
  // right margin from the text to the right edge of the button
  // for the active button (use the passive one)
	DWORD m_dwActiveRightMargin;
  // right margin from the text to the right edge of the button
  // for the passive button
	DWORD m_dwPassiveRightMargin;
  // position of the pressed test in the button
	CPoint m_ptPressedTextOrig;
  // position of the active test in the button
	CPoint m_ptActiveTextOrig;
  // position of the passive test in the button
	CPoint m_ptPassiveTextOrig;

  // virtual size to be taken in menus...
  CSize m_sizeVirtual;

  // minimal width of the text in button (if smaller - this size is used insdead)
  DWORD m_dwMinimalTextWidth;

	// animations for the passive state
  CAnimation *m_pPassiveLeft;
	CAnimation *m_pPassiveMiddle;
	CAnimation *m_pPassiveRight;

	// animations for the passive state with the keyboard focus
  CAnimation *m_pPassiveKeyLeft;
	CAnimation *m_pPassiveKeyMiddle;
	CAnimation *m_pPassiveKeyRight;

	// animations for the active state
  CAnimation *m_pActiveLeft;
	CAnimation *m_pActiveMiddle;
	CAnimation *m_pActiveRight;

	// animations for the active state with the keyboard focus
  CAnimation *m_pActiveKeyLeft;
	CAnimation *m_pActiveKeyMiddle;
	CAnimation *m_pActiveKeyRight;

	// animations for the pressed state
  CAnimation *m_pPressedLeft;
	CAnimation *m_pPressedMiddle;
	CAnimation *m_pPressedRight;

  // sound played if the button is actvated by the mouse
  CSoundBuffer *m_pHoverSound;
  // sound played if the button is deactivated by the mouse
  CSoundBuffer *m_pLeaveSound;
  // sound played when the button is pressed
  CSoundBuffer *m_pPressSound;
  // sound played when the button is released
  CSoundBuffer *m_pReleaseSound;

  // font of the text in the button
  CFontObject *m_pFont;

  // cursor for the button window
  CCursor *m_pCursor;

  friend CTextButton;
};

#endif // !defined(AFX_TEXTBUTTONLAYOUT_H__A8386F73_DF68_11D2_AC26_96E0A5B35760__INCLUDED_)
