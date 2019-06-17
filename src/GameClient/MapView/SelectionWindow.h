// SelectionWindow.h: interface for the CSelectionWindow class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SELECTIONWINDOW_H__D1255AD3_5B39_11D4_B0BD_004F49068BD6__INCLUDED_)
#define AFX_SELECTIONWINDOW_H__D1255AD3_5B39_11D4_B0BD_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "UnitSelection.h"

// Windows showing selected units
class CSelectionWindow : public CFrameWindow  
{
  DECLARE_DYNAMIC(CSelectionWindow);
  DECLARE_OBSERVER_MAP(CSelectionWindow);

public:
  // constructor & destructor
	CSelectionWindow();
	virtual ~CSelectionWindow();

  // debug functions
#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

  // Creates the window
  void Create(CRect &rcRect, CWindow *pParent, CUnitSelection *pSelection, CDataArchive MapViewArchive, CWindow *pDesktopWindow);
  // Deletes it
  virtual void Delete();

  // Partialy disables the window (some buttons on the window)
  void DisablePartial();
  // Enables the whole window
  void EnableAll();

  enum{ E_Close = 1, };

protected:
  // reaction on size change
  virtual void OnSize(CSize size);

  // reaction on events
  void OnClose();

private:
  // The selection object (we represent it)
  CUnitSelection *m_pSelection;
  enum{ ID_Selection = 0x01000, };

  // The layout of the window
  CFrameWindowLayout m_Layout;
  // The layout of the list control
  CListControlLayout m_ListLayout;
  // The list control
  CListControl m_wndList;

  // The close button layout
  CImageButtonLayout m_CloseButtonLayout;
  // The close button
  CImageButton m_wndCloseButton;

  enum{ ID_CloseButton = 0x01001, };

  // Desktop window
  CWindow *m_pDesktopWindow;

  // returns the rectangle of the list control in the window
  void GetListRect(CRect *pRect);
  // Fills the list with items from the selection
  void FillList();
};

#endif // !defined(AFX_SELECTIONWINDOW_H__D1255AD3_5B39_11D4_B0BD_004F49068BD6__INCLUDED_)
