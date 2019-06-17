// EditBox.h: interface for the CEditBox class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EDITBOX_H__F729F994_4B42_11D3_A024_FDE940773438__INCLUDED_)
#define AFX_EDITBOX_H__F729F994_4B42_11D3_A024_FDE940773438__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\..\Window.h"
#include "EditBoxLayout.h"

class CEditBox : public CFrameWindow  
{
  DECLARE_DYNAMIC(CEditBox);
  DECLARE_OBSERVER_MAP(CEditBox);

public:
#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

public:
  // sets the password mode (TRUE - set, FALSE reset)
	void SetPassword(BOOL bPassword = TRUE);
  // sets maximum length of the edited text
  // if -1 -> infinite
	void SetMaxLength(int nMaxLength = -1);
  // cuts the selection to the clipboard
	void Cut();
  // paste text from the clipboard
	void Paste();
  // copy the selection to the clipboard
	void Copy();
  // draw method called from the windows system
  virtual void Draw(CDDrawSurface *pSurface, CRect *pBoundRect);

  // deletes the obect
  virtual void Delete();
	// creates the edit box (with empty text)
  BOOL Create(CRect &rcBound, CEditBoxLayout *pLayout, CWindow *pParent, int nMaxLength = -1);

  // sets new text into the editbox
	void SetText(CString strText);
  // returns the edited text
	CString GetText();

  // constructor
	CEditBox();
  // destructor
	virtual ~CEditBox();

  enum Events{
    E_CHANGE = 1 // the text had changed
  };

protected:
  // maximu characters in the edit box
	int m_nMaxLength;
  // inserts test into the position
	virtual void InsertText(CString strText, int nPosition);
  // returns the selection
	virtual CString GetSelection();
  // delets the selection
	virtual void DeleteSelection();
  // returns TRUE if there is no selection
	virtual BOOL NoSelection();
  // sets the start of the selection
	virtual void SetSelectionStart(int nPosition);
	// sets the edges of the selection
  // parameter is current position of the user selection
  // and in the variable m_nSelectionStartPosition should be
  // the start index of the selection
  virtual void SetSelectionEdge(int nPosition);
  // index of the character where the user started the selection
	int m_nSelectionStartPosition;
  // returns the index of the characetr under given point
	virtual int GetCharOn(CPoint point);

  // reactions on mouse events
  void OnMouseMove(CPoint point);
  void OnLButtonDblClk(CPoint point);
  void OnLButtonUp(CPoint point);
  void OnLButtonDown(CPoint point);
  // scrolls the text so the caret is visible
  // returns TRUE if the editbox needs repainting
	virtual BOOL ScrollToCaret();
  // returns the text which is currently visible in the edit box
  // (includes the last character which is partialy visible)
	virtual int GetVisibleText(CString &strVisible);
  // reaction on char event
  BOOL OnChar(UINT nChar, DWORD dwFlags);
  // reaction on key down event
  BOOL OnKeyDown(UINT nChar, DWORD dwFlags);
	// the caret is ahead of the character with this index
  int m_nCaretPosition;
  // TRUE -> if the selection is to be drawn
	BOOL m_bSelectionVisible;

  // reaction for request to repaint some animation
  void OnAnimsRepaint(CAnimationInstance *pAnimation);
  // rectangle for clipping the text in it
	CRect m_rcInner;
  // index of first visible character
  int m_nFirstVisible;
  // end of the selection (index of the character beond the end)
  int m_nSelectionEnd;
  // start of the selection (index of the first character)
  // if -1 -> no selection
  int m_nSelectionStart;

	// reaction on activate event from window system
  virtual BOOL OnActivate();
  // reaction on lose focus
	virtual void OnLoseFocus();
  // reaction on set focus
	virtual void OnSetFocus();

  // the text in the edit box (edited text)
	CString m_strText;

  // TRUE - if in password mode
  BOOL m_bPassword;

private:
	BOOL m_bMouseDrag;
  // animation for the selection
  CAnimationInstance m_Selection;
  // animation for cursor
  CAnimationInstance m_Caret;

  // pointer to the layout object for this editbox
	CEditBoxLayout * m_pLayout;
};

#endif // !defined(AFX_EDITBOX_H__F729F994_4B42_11D3_A024_FDE940773438__INCLUDED_)
