// ScriptView.h: interface for the CScriptView class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SCRIPTVIEW_H__4CBB5F01_AB35_11D3_99C2_99ADDD254479__INCLUDED_)
#define AFX_SCRIPTVIEW_H__4CBB5F01_AB35_11D3_99C2_99ADDD254479__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ScriptViewEdit.h"

class CScriptMDIFrame;

class CScriptView : public CCaptionWindow  
{
  DECLARE_DYNAMIC(CScriptView);
  DECLARE_OBSERVER_MAP(CScriptView);

public:
	void Close();
	void DeleteSelection();
	void Paste();
	void Cut();
	void Copy();

  // returns the script edit control in the view
  CScriptEdit *GetScriptEditCtrl(){ ASSERT_VALID(this); return &m_ScriptEdit; }
  
  // returns the filename
  CString GetFileName(){ ASSERT_VALID(this); return m_strFileName; }
  // sets the filename
  void SetFileName(CString strFileName);

  // returns position of the window for nonmaximized state
  CRect GetNormalPosition(){ ASSERT_VALID(this); return m_rcNormalPosition; }

  // returns if the view is marked as maximized
  BOOL GetMaximized(){ ASSERT_VALID(this); return m_bMaximized; }
  // sets the maximized flag
  void SetMaximized(BOOL bMaximized = TRUE){ ASSERT_VALID(this); m_bMaximized = bMaximized; }

  // returns the modified flag
  BOOL GetModified(){ ASSERT_VALID(this); return m_bModified; }
  // sets the modified flag
  void SetModified(BOOL bModified = TRUE){ ASSERT_VALID(this); m_bModified = bModified; UpdateCaption(); }

  // returns the saved flag
  BOOL GetSaved(){ ASSERT_VALID(this); return m_bSaved; }
  // sets the saved flag
  void SetSaved(BOOL bSaved = TRUE){ ASSERT_VALID(this); m_bSaved = bSaved; }

  // returns the local flag
  BOOL GetLocal(){ ASSERT_VALID(this); return m_bLocal; }
  // sets the local flag
  void SetLocal(BOOL bLocal = TRUE){ ASSERT_VALID(this); m_bLocal = bLocal; UpdateCaption(); }

  // sets caret line
  void SetCaretLine(DWORD dwLine);

  // destroys all layouts
	static void CloseLayout();
  // call this before creating any view -> sets their layout
	static void InitLayout(CCaptionWindowLayout *pCaptionWidnowLayout, CScriptEditLayout *pScriptEditLayout);

  // creates the view
	BOOL Create(CRect &rcBound, CWindow *pParent, CScriptMDIFrame *pMDIFrame);
  // deletes the object
	virtual void Delete();

  // constructor & destructor
	CScriptView();
	virtual ~CScriptView();

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

protected:
	void OnContextMenu(CPoint point);
  void OnScriptModified();
	virtual BOOL OnActivate();
	virtual void OnSize(CSize size);
  virtual void OnMove(CPoint point);

  void OnLButtonDblClk(CPoint point);

  enum{
    IDC_SCRIPTEDIT = 0x0200,
    IDC_CLOSE = 0x0201
  };
  // the script edit control
	CScriptViewEdit m_ScriptEdit;

  enum{
    IDC_CUT = 0x0100,
    IDC_COPY = 0x0101,
    IDC_PASTE = 0x0102,
    IDC_DELETE = 0x0103
  };
  // right click popup menu
  CPopupMenu m_ContextMenu;

  // the close button
  CImageButton m_CloseButton;

private:
  // sets caption text (takes care of all flags)
  void UpdateCaption();
  // returns the rect of the edit control
	void GetEditRect(CRect *pEditRect);
  // the layouts for the script view
	static CScriptEditLayout * m_pScriptEditLayout;
	static CCaptionWindowLayout * m_pCaptionWindowLayout;

  // the file name
  CString m_strFileName;

  // the saved flag
  BOOL m_bSaved;
  // set when the view was modified
  BOOL m_bModified;
  // the local flag
  BOOL m_bLocal;
  // maximized (the view is maximized)
  BOOL m_bMaximized;
  // position of the window in nonmaximized state
  CRect m_rcNormalPosition;

  // if TRUE - the view dosn't take care about position changes in context of maximizing
  BOOL m_bIgnoreReposition;

  // pointers for the list of all views
  CScriptView *m_pNextView;
  CScriptView *m_pPrevView;

  // pointer to the MDI frame - our parent
  CScriptMDIFrame *m_pMDIFrame;

  friend class CScriptMDIFrame;
};

#endif // !defined(AFX_SCRIPTVIEW_H__4CBB5F01_AB35_11D3_99C2_99ADDD254479__INCLUDED_)
