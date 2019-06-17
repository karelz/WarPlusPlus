// UnitToolbar.h: interface for the CUnitToolbar class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UNITTOOLBAR_H__75D881C3_2E21_11D4_84D5_004F4E0004AA__INCLUDED_)
#define AFX_UNITTOOLBAR_H__75D881C3_2E21_11D4_84D5_004F4E0004AA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// The unit toolbar window
class CUnitToolbar : public CFrameWindow  
{
  DECLARE_DYNAMIC(CUnitToolbar);
  DECLARE_OBSERVER_MAP(CUnitToolbar);

public:
  // Constructor & destructor
	CUnitToolbar();
	virtual ~CUnitToolbar();

  // Debug functions
#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

  // Sets toolbar button info for the toolbar
  void SetToolbarInfo(CToolbarInfo *pToolbarInfo);
  // Returns current toolbar button info
  CToolbarInfo * GetToolbarInfo () { ASSERT_VALID ( this ); return m_pToolbarInfo; }

  // Creation
  // Creates the window (toolbar) empty
  void Create(CRect &rcWindow, CWindow *pParent, CDataArchive MapViewArchive, CWindow *pDesktopWindow);

  // Deletes the toolbar
  virtual void Delete();

  void DisablePartial();
  void EnableAll();

  enum{ E_Command = 1, };

protected:
  void OnSize(CSize size);
  void OnMove(CPoint point);

  void OnButton(DWORD dwButton);

private:
  enum{
    ID_Buttons = 0x100,
  };

  // Sets the unit toolbar to empty (no unit)
  void SetEmpty();
  // Clears all controls on the toolbar
  void Clear();

  // Empty text control
  CStaticText m_EmptyText;

  // Current toolbar info
  CToolbarInfo *m_pToolbarInfo;

  // Array of pointers to currently displayed buttons
  CIconButton **m_aButtons;
  // Number of buttons on the toolbar
  DWORD m_dwButtonCount;
  // Layout for toolbar buttons
  CIconButtonLayout m_ButtonLayout;

  // Default button icon to use when none specified
  CAnimation m_DefaultButtonIcon;

  // Desktop window to use for our buttons
  CWindow *m_pDesktopWindow;

  // Layout for this window
  CFrameWindowLayout m_Layout;

// Some info about the resizing
  // Last size
  CRect m_rcLastPosition;
  // Size of one button
  CSize m_szButton;

  // TRUE if the toolbar is "line"
  BOOL m_bLine;

  // Number of lines (or columns)
  // This is the fixed direction size (in buttons)
  DWORD m_dwFixedNumber;

  // Size of one line/column toolbar
  CSize m_szOneLine;

  // Max and minimal size in the Variable direction
  // In this range the toolbar stays the same
  DWORD m_dwMin, m_dwMax;

  // TRUE if we're in OnSize method
  BOOL m_bSizing;

  CSize ComputeWindowSize(DWORD dwFixedCount);
  CSize ComputeOneLineSize();
  int CompareWithSquare(CSize sz);
  void PlaceButtons();
};

#endif // !defined(AFX_UNITTOOLBAR_H__75D881C3_2E21_11D4_84D5_004F4E0004AA__INCLUDED_)
