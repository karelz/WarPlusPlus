// MapViewToolbar.h: interface for the CMapViewToolbar class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAPVIEWTOOLBAR_H__A650B843_5402_11D4_B0AA_004F49068BD6__INCLUDED_)
#define AFX_MAPVIEWTOOLBAR_H__A650B843_5402_11D4_B0AA_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMapViewToolbar : public CWindow  
{
  DECLARE_DYNAMIC(CMapViewToolbar);
  DECLARE_OBSERVER_MAP(CMapViewToolbar);

public:
  // Constructor & destructor
	CMapViewToolbar();
	virtual ~CMapViewToolbar();

  // Debug Functions
#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

  // Creates the toolbar (it will create it in the bottom right corner of the parent window)
  void Create(CWindow *pParent, CDataArchive MapViewArchive, CObserver *pObserver);
  // Deletes the toolbar
  virtual void Delete();

  void DisablePartial();
  void EnableAll();

  // Draws the window
  void Draw(CDDrawSurface *pDDSurface, CRect *pBoundRect);

  enum{
    IDC_SCRIPTEDITOR = 0x05000, // The ID of the script editor button
    IDC_MINIMAP = 0x05001,      // The ID of the minimap button
    IDC_SELECTION = 0x05002,    // The ID of the selection window button
  };

  typedef enum{
    State_None = 0,
    State_HourGlass = 1,
    State_HourGlassWarning = 2,
    State_Question = 3,
  } EState;

  // Sets new state
  void SetState(EState eState = State_None){ m_eState = eState; UpdateRect(); }

private:
  // The background image
  CImageSurface m_Background;

  // The Script editor button
  CImageButtonLayout m_ScriptEditorLayout;
  CImageButton m_wndScriptEditor;

  // The Minimap button
  CImageButtonLayout m_MinimapLayout;
  CImageButton m_wndMinimap;

  // The Selection window button
  CImageButtonLayout m_SelectionLayout;
  CImageButton m_wndSelection;

  CImageSurface m_HourGlass;
  CImageSurface m_HourGlassWarning;
  CImageSurface m_Question;

  // Current state
  EState m_eState;
};

#endif // !defined(AFX_MAPVIEWTOOLBAR_H__A650B843_5402_11D4_B0AA_004F49068BD6__INCLUDED_)
