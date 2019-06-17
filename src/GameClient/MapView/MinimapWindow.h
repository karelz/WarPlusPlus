// MinimapWindow.h: interface for the CMinimapWindow class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MINIMAPWINDOW_H__6A29CEC7_4744_11D4_B519_00105ACA8325__INCLUDED_)
#define AFX_MINIMAPWINDOW_H__6A29CEC7_4744_11D4_B519_00105ACA8325__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\DataObjects\CMap.h"
#include "..\GraphicalObjects\Viewport.h"
#include "..\ServerCommunication\CMiniMapClip.h"

#define MINIMAP_CIVILIZATION_COUNT 32

class CMinimapWindow : public CWindow  
{
  DECLARE_DYNAMIC(CMinimapWindow);
  DECLARE_OBSERVER_MAP(CMinimapWindow);

public:
  // Constructor & destructor
	CMinimapWindow();
	virtual ~CMinimapWindow();

  // Debug functions
#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc ) const;
#endif

  // Creates the minimap window (given the size of the whole window (and position))
  void Create(CRect &rcRect, CWindow *pParent, CCMap *pMap, CViewport *pViewport, CCMiniMapClip *pMiniMapClip);
  // Deletes the window
  virtual void Delete();

  // Draws the window (internal)
  virtual void Draw(CDDrawSurface *pSurface, CRect *pBoundRect);

  // Sets new position of the minimap view
  // Specified coords will be new top-left corner of the view in mapcells
  void SetPosition(DWORD dwXPosition, DWORD dwYPosition);

  // Sets new position of the view rectangle
  void SetViewRect(CRect &rcViewRect);

  // Sets new zoom for the minimap
  void SetZoom(DWORD dwZoom);
  // Returns current zoom
  DWORD GetZoom(){ return m_dwZoom; }

protected:
  // Reactions on mouse events
  void OnMouseMove(CPoint pt);
  void OnLButtonDown(CPoint pt);
  void OnLButtonUp(CPoint pt);
  void OnRButtonDown(CPoint pt);
  void OnRButtonUp(CPoint pt);

  class CMinimapViewportPositionObserver : public CViewport::CViewportPositionObserver
  {
  public:
    CMinimapViewportPositionObserver(){};
    virtual ~CMinimapViewportPositionObserver(){};

    virtual void OnPositionChanged();

    void Create(CMinimapWindow *pWindow, CViewport *pViewport){
      m_pMinimapWindow = pWindow;
      CViewportPositionObserver::Create(pViewport);
    }

  private:
    // Pointer to the minimap window
    CMinimapWindow *m_pMinimapWindow;
  };

  void OnClipChanged();

private:

// Background graphics
  // Size of the background bitmap loaded from the disk (in minimap background pieces)
  DWORD m_dwBackgroundWidth;
  DWORD m_dwBackgroundHeight;
  // Position of the background bitmap on the map (in pieces)
  DWORD m_dwBackgroundPositionX;
  DWORD m_dwBackgroundPositionY;
  // The bitmap itself
  CScratchSurface m_Background;

// Position of the minimap
  // Position of the minimap window (view) on the whole map
  // This is the topleft corner position in mapcells
  DWORD m_dwPositionX;
  DWORD m_dwPositionY;

  // The zoom of the minimap (2, 4, 8, 16)
  DWORD m_dwZoom;

// Position of the view rectangle
  // The rectangle on the map which is in the associated view (in mapcells)
  CRect m_rcViewRect;

  // Position observer for associated viewport
  CMinimapViewportPositionObserver m_PositionObserver;

// Unit minimap
  // The minimap clip object (retrieved data from server)
  CCMiniMapClip *m_pMiniMapClip;
  enum{ ID_MiniMapClip = 0x0100, };

  // Array of civilization colors
  DWORD m_aCivilizationColors[MINIMAP_CIVILIZATION_COUNT];

// Some other data
  // The pointer to the map object
  CCMap *m_pMap;

  // The exclusive lock for this object
  CMutex m_lockExclusive;

  // Some helper variables for mouse draging
  enum EDragState{
    Drag_None = 0,
    Drag_Minimap = 1,
    Drag_ViewRect = 2,
  };
  EDragState m_eDragState;

  // Starting position of the minimap drag
  DWORD m_dwMinimapDragStartX, m_dwMinimapDragStartY;
  CPoint m_ptMinimapDragStart;


// Helper routines
  // Loads one minimap background piece from the disk
  // First coords are position of the piece on the whole map (in pieces)
  // Second coords are position of the piece on the background bitmap (in pieces)
  void LoadBackgroundPiece(DWORD dwX, DWORD dwY, DWORD dwBkgPosX, DWORD dwBkgPosY);

  // Sets new view rect position from mouse point in the window (used when clicked)
  void SetMouseViewRect(CPoint &pt);
};

#endif // !defined(AFX_MINIMAPWINDOW_H__6A29CEC7_4744_11D4_B519_00105ACA8325__INCLUDED_)
