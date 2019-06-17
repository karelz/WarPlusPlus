// ScriptEdit.h: interface for the CScriptEdit class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SCRIPTEDIT_H__7C4CD784_5A18_11D3_A037_B2F8384EE131__INCLUDED_)
#define AFX_SCRIPTEDIT_H__7C4CD784_5A18_11D3_A037_B2F8384EE131__INCLUDED_

#include "..\SCROLLCONTROL\ScrollControl.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\..\Window.h"
#include "ScriptEditLayout.h"
#include "ScriptEditScroll.h"

#include "..\..\..\ScriptLex\ScEditLex.h"
#include "ScEdLexInput.h"

class CScriptEdit : public CFrameWindow  
{
  DECLARE_DYNAMIC(CScriptEdit)
  DECLARE_OBSERVER_MAP(CScriptEdit)

protected:
  // struct for one style node
  struct tagSStyleNode{
    int m_nColumn;     // the first column of this style
    DWORD m_dwStyle;   // the style ID
    struct tagSStyleNode *m_pNext; // next b;ock in the list on the line
  };
  typedef struct tagSStyleNode SStyleNode;

  // struct for one line
  struct tagSLine{
    CString m_strText;    // the text of the line
    SStyleNode *m_pStyles; // the style list of the line
    int m_nWidth; // width of the line in pixels
  };
  typedef struct tagSLine SLine;

  enum{ HorizontalScrollID = 1, VerticalScrollID = 2 };

public:
  // sets the readonly flag
  void SetReadOnly(BOOL bReadOnly = TRUE){ ASSERT_VALID(this); m_bReadOnly = bReadOnly; }

  // appends given text to the end of the edited text
  void AppendText(CString &strText);

  // deletes selected text
	void DeleteSelectedText();

  // returns number of lines
  DWORD GetLinesCount(){ ASSERT_VALID(this); return m_aLines.GetSize(); }

  // returns one line
  CString GetLine(DWORD dwLine){ ASSERT_VALID(this); ASSERT(dwLine < (DWORD)m_aLines.GetSize()); return m_aLines[dwLine]->m_strText; }

  // returns position of the caret
  DWORD GetCaretLine(){ ASSERT_VALID(this); return (DWORD)m_nCaretLine; }
  DWORD GetCaretColumn(){ ASSERT_VALID(this); return (DWORD) m_nCaretRealColumn; }
  // sets new caret position
	virtual void SetCaretPosition(int nLine, int nColumn);
  // scrolls to given position
  // makes it visible
  void ScrollToPosition(int nLine, int nCol);

  // sets the selection
	void SetSelection(int nStartLine, int nStartColumn, int nEndLine, int nEndColumn);
  void SetSelectionLine(int nStartLine, int nEndLine);

  // sets new window position
	void SetWindowPos(CRect *pRect);

  // returns the whole contents
  CString GetAll();
  // sets the whole contents
  void SetAll(CString strAll);
  // deletes all text
  void DeleteAll();

  // disk operations
	void LoadFromFile(CArchiveFile file);
	void SaveToFile(CArchiveFile file);

  // clipboard operations
	void Paste();
	void Cut();
	void Copy();
  // draws the script edit
	virtual void Draw(CDDrawSurface *pSurface, CRect *pRectBound);
  // deletes the object
	virtual void Delete();
  // creates the object
	BOOL Create(CRect &rcBound, CScriptEditLayout *pLayout, CWindow *pParent);

  // constructor
	CScriptEdit();
  // desctructor
	virtual ~CScriptEdit();

  enum{
    E_MODIFIED = 1,
  };

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

private:
	void DrawSelectionLine(int nLine, int nStartCol, int nEndCol, CDDrawSurface *pSurface, CRect *pRectBound, BOOL bEOLSelected);
  // layout of this control
	CScriptEditLayout * m_pLayout;

  // some animations
  CAnimationInstance m_Caret;

  // the rect of the text
  CRect m_rcInner;

  // the lex input
//  CScriptEditLexInput m_LexInput;
  // the lex acceptor
//  CLexAcceptor m_LexAcceptor;

  // the lex
  // counted pointer
  static CScEditLex *m_pScEditLex;
  static DWORD m_dwScEditLexReferenceCount;

  // the input object for the lex
  CScEdLexInput m_ScEdLexInput;

protected:
	virtual void OnSize(CSize size);

  // computes position from the point
  // character under the point
	void PositionFromPoint(CPoint point, int &nLine, int &nColumn);

  // clears the selection (No selection)
	void ClearSelection();
  // returns TRUE if the selection is empty
	BOOL IsSelectionEmpty();
  // sets the user start of the selection
	void SetSelectionStart(int nLine, int nColumn);
  // sets the user edge of the selection
	void SetSelectionEdge(int nLine, int nColumn);
  // selects whole text
	void SelectAll();
  // returns selected text
	CString GetSelection();
  // deletes selected text
	void DeleteSelection();

  // sets new first visible position (top left corener)
	virtual void SetFirstVisible(int nLine, int nColumn);

  // returns the indentation of given line
	int GetIndentation(int nLine);

  // inserts text at given position
  // the text can't containt EOLs
	void InsertLineText(int nLine, int nColumn, CString strText);
  // insert one line
	void InsertLine(int nLine, CString strLine, BOOL bUpdate = TRUE);
  // inserts multiline text
	void Insert(int nLine, int nColumn, CString strText);
  // deletes line
	void DeleteLine(int nLine);
  // deletes text one one line
	void DeleteLineText(int nLine, int nStartColumn, int nCount);

  // updates scroll bars to fit the position
	void UpdateScrolls();
  // updates given range
	void UpdateRange(int nStartLine, int nStartCol, int nEndLine, int nEndCol);
  // updates one line
	void UpdateLine(int nLine);

  // (re)computes highlighting on given lines range
	void ComputeHighlighting(int nStartLine, int nStartColumn, int nEndLine, int nEndColumn);

  // reactions on events from keyboard
	BOOL OnChar(int nChar, DWORD dwFlags);
	BOOL OnKeyDown(int nChar, DWORD dwFlags);

  // reactions on event from mouse
	void OnLButtonUp(CPoint point);
	void OnLButtonDown(CPoint point);
	void OnMouseMove(CPoint point);

  // reaction on event from timer
	void OnTimeTick(DWORD dwTime);

  // reactions on events from scroll bars
	void OnHorizontalPosition(int nNewPosition);
	void OnHorizontalPageDown();
	void OnHorizontalPageUp();
	void OnHorizontalStepDown();
	void OnHorizontalStepUp();
	void OnVerticalPosition(int nNewPosition);
	void OnVerticalPageDown();
	void OnVerticalPageUp();
	void OnVerticalStepDown();
	void OnVerticalStepUp();

  // reaction on repaint rtequest
	void OnAnimsRepaint(CAnimationInstance *pAnim);

  // reaction on activation of some scroll bar
  // -> active us
	void OnScrollActivated();


  // deletes whole lines array
	virtual void DeleteLines();
	// deletes the SLine structure
  virtual void DeleteLine(SLine *pLine);

  // scroll bars
	CScriptEditScroll m_VerticalScroll;
	CScriptEditScroll m_HorizontalScroll;

  // reactions on some notifications
	virtual void OnLoseFocus();
	virtual void OnSetFocus();
	virtual BOOL OnActivate();

  // the height of one line (in pixels)
  int m_nLineHeight;

  // the first visible line (at the top of the window)
  int m_nFirstVisibleLine;
  int m_nFirstVisibleColumn; // this is in pixels
  int m_nLastVisibleLine;

  // position of the caret
  int m_nCaretLine;   // line o nwhich the caret is
  int m_nCaretColumn; // column before which the caret is
  int m_nCaretRealColumn; // the really drawn column

  // selection
  int m_nSelectionStartLine;
  int m_nSelectionStartColumn;
  int m_nSelectionUserStartLine;
  int m_nSelectionUserStartColumn;
  int m_nSelectionEndLine;
  int m_nSelectionEndColumn;

  // mouse is draging in the window
  BOOL m_bMouseDrag;
  int m_nLastMouseLine;
  int m_nLastMouseColumn;

  // timer ticked and was not used
  BOOL m_bTimerTicked;
  static DWORD m_dwScrollTickDelay;

  // the array of lines in the editor
  CTypedPtrArray<CPtrArray, SLine *> m_aLines;

  // TRUE - if readonly
  BOOL m_bReadOnly;

  friend class CScriptEditLexInput;
  friend class CScEdLexInput;
};

#endif // !defined(AFX_SCRIPTEDIT_H__7C4CD784_5A18_11D3_A037_B2F8384EE131__INCLUDED_)
