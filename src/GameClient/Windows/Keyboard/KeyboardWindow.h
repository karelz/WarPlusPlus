#if !defined(AFX_KEYBOARDWINDOW_H__8F2050C3_7C95_11D2_AB60_D89D541E9E61__INCLUDED_)
#define AFX_KEYBOARDWINDOW_H__8F2050C3_7C95_11D2_AB60_D89D541E9E61__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// KeyboardWindow.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CKeyboardWindow frame

class CKeyboard;

class CKeyboardWindow : public CEventWindow
{
	DECLARE_DYNCREATE(CKeyboardWindow)
protected:
  // constructor
  CKeyboardWindow();           // protected constructor used by dynamic creation

public:
#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CKeyboardWindow)
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CKeyboardWindow();

	// Generated message map functions
	//{{AFX_MSG(CKeyboardWindow)
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
  afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	// pointer to the keyboard object
  // we will inlay events instead of this object
  CKeyboard * m_pKeyboard;

  friend CKeyboard;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_KEYBOARDWINDOW_H__8F2050C3_7C95_11D2_AB60_D89D541E9E61__INCLUDED_)
