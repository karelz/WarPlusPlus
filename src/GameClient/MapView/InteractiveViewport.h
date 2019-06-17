// InteractiveViewport.h: interface for the CInteractiveViewport class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_INTERACTIVEVIEWPORT_H__B40B79C3_5A33_11D4_B0BB_004F49068BD6__INCLUDED_)
#define AFX_INTERACTIVEVIEWPORT_H__B40B79C3_5A33_11D4_B0BB_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\GraphicalObjects\Viewport.h"
#include "UnitSelection.h"

class CInteractiveViewport : public CViewport
{
  DECLARE_DYNAMIC(CInteractiveViewport);
  DECLARE_OBSERVER_MAP(CInteractiveViewport);

public:
  // Constructor & destructor
	CInteractiveViewport();
	virtual ~CInteractiveViewport();

  // Creates the viewport
  void Create(CRect &rcWindowRect, CCMap *pMap, CCServerUnitInfoReceiver *pServerUnitInfoReceiver,
    CWindow *pParent, CUnitSelection *pUnitSelection, CDataArchive *pGraphicsArchive, CDDrawSurface *pExternalFullScreenBuffer = NULL);
  // Deletes the viewport
  virtual void Delete();

  // Returns current rectangle selection
  // If there is none - the returned rect will be empty
  // coords are in pixels on the map
  CRect GetRectangleSelection () { ASSERT_VALID ( this );
    VERIFY ( m_lockRectangleSelection.Lock () ); CRect rcRectangleSelection = m_rcCurrentRectangleSelection;
    VERIFY ( m_lockRectangleSelection.Unlock () ); return rcRectangleSelection; }
  // Sets new rectangle selection
  // coords are in pixels on the map
  void SetRectangleSelection ( CRect & rcSelection );
  // Enables/Disables rectangle selection
  void EnableRectangleSelection ( bool bEnable = true ) { m_bRectangleSelectionEnabled = bEnable; }

  // Returns life bar marking flag
  bool GetLifeBarMarking () { ASSERT_VALID ( this ); return m_bLifeBarMarking; }
  // Set the life bar marking flag
  void SetLifeBarMarking ( bool bLifeBarMarking = true ) {
    ASSERT_VALID ( this ); m_bLifeBarMarking = bLifeBarMarking; }

  typedef enum{
    ViewportState_Normal = 0, // Normal state - selection is enabled
    ViewportState_Disabled = 1, // Disabled - no mouse actions - just Cancel (RButton)
    ViewportState_Asking = 2,  // Asking - send event if the mouse clicks
  } EViewportState;

  // Sets new viewport state
  void SetViewportState(EViewportState eState);

  typedef enum{
    ViewportQuestion_None = 0,
    ViewportQuestion_Position = 1,  // Asking for position
    ViewportQuestion_AnyUnit = 2,   // asking for any unit
    ViewportQuestion_EnemyUnit = 3, // asking for enemy unit
    ViewportQuestion_MyUnit = 4,    // asking for my unit
  } EViewportQuestion;

  // Sets viewport question
  // To set some reasonable question, the viewport must be in Asking state
  // And also before setting new state - clear the question
  void SetViewportQuestion(EViewportQuestion eQuestion);

  // Debug functions
#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

  enum{
                  // If the view is in some asking state and user cancel the action
                  // (by right click)
    E_Cancel = 1, // Sent when the user clicked the right button

                    // If the view is in the Asking state for Position then if the user
                    // selects some position then
    E_Position = 2, // in param is (X << 16) | (Y) in mapcells

                  // If the view in in Asking state for some unit (Any, My, Enemy)
                  // and user selects the apropriate unit then this event is sent
                  // !!!!!!  The unit has AddRef called on it
    E_Unit = 3,   // in param is (DWORD)(CCUnit *)pUnit

                      // If the view is in normal state and user selects some unit
                      // for watching (doubleclick it)
                      // !!!!!! The unit has AddRef called on it
    E_WatchUnit = 4,  // in param is (DWORD)(CCUnit *)pUnit which was selected to watch

                              // If the view is in the normal state then if the user
                              // selects some position with right click then
    E_RightClickPosition = 5, // in param is (X << 16) | (Y) in mapcells

                          // If the view is in normal state and user right clicks
                          // some unit
    E_RightClickUnit = 6, // then in the param is (DWORD)(CCUnit *)pUnit

                              // If the view is in the normal state then if the user
                              // selects some position with right click then
    E_ShiftRightClickPosition = 7, // in param is (X << 16) | (Y) in mapcells

                          // If the view is in normal state and user right clicks
                          // some unit
    E_ShiftRightClickUnit = 8, // then in the param is (DWORD)(CCUnit *)pUnit
  };

protected:
  // This functions is called before computing the frame updates
  // You can add any rectangles to given list and this is the way you
  // register some rects for updating
  // The viewport is locked
  virtual void AddOverlayUpdates ( CUpdateRectList * pUpdateRectList );
  // Draw any overlay - it means here you can draw anything over
  // the viewport graphics
  // But if you want to change it, you must register a rect for
  // updating in AddOverlayUpdates
  // You should draw it to given surface, as if you'd draw it to the
  // frame buffer (usually it will be the frame buffer)
  // The rcUpdateRect is the rect that you should draw into
  // (in fact the surface will have set the clipping just for this rect)
  // The viewport is locked
  virtual void DrawOverlay ( CDDrawSurface * pSurface, CRect & rcUpdateRect );

  // Called before the all units are locked
  // int the computing frames
  // If you need some lock in GetUnitMarks or AdjustNodeSize lock it here
  virtual void BeforeUnitsLocking ();
  // Called after all units are unlocked
  // unlock everything you've locked in BeforeUnitsLocking
  virtual void AfterUnitsLocking ();

  // Returns unit marks (selection marks for units in selection)
  virtual DWORD GetUnitMarks(CCUnit *pUnit);

// Reactions on mouse events
  // LButton was clicked
  void OnLButtonDown ( CPoint pt );
  // LButton was released
  void OnLButtonUp ( CPoint pt );
  // LButton was doubleclicked
  void OnLButtonDblClk ( CPoint pt );
  // RButton was clicked
  void OnRButtonDown ( CPoint pt );
  // RButton was released
  void OnRButtonUp ( CPoint pt );
  // Mouse moved
  void OnMouseMove ( CPoint pt );

private:

  // Current state of the viewport
  EViewportState m_eViewportState;

  // Current viewport question
  EViewportQuestion m_eViewportQuestion;

  // Selection object to work with
  CUnitSelection *m_pUnitSelection;

// Cursors
  // Location cursor
  CCursor m_LocationCursor;
  // MyUnit cursor
  CCursor m_MyUnitCursor;
  // Enemy unit cursor
  CCursor m_EnemyUnitCursor;

  // My drag cursor
  CCursor m_MyDragCursor;
  // Enemy drag cursor
  CCursor m_EnemyDragCursor;

// Rectangle selection
  // true if the rectangle selection is enabled in this viewport
  bool m_bRectangleSelectionEnabled;
  // Actual rectangle selection (in pixels on the map)
  CRect m_rcCurrentRectangleSelection;
  // Last drawn rectangle selection (in pixels on the map)
  CRect m_rcLastDrawnRectangleSelection;
  // Lock for accesing the selection
  CCriticalSection m_lockRectangleSelection;
  // If true we will select all our units
  // If false we will select all enemy units
  bool m_bRectangleSelectOurUnits;
  // Selection of units, that were selected before the rectangle selection started
  CUnitSelection m_cKeepSelectionWhileDragging;

// Mouse status
  // true if the mouse is dragging
  bool m_bMouseDragging;
  // Here we'll remember the drag start point 
  // (in pixels on the map)
  CPoint m_ptDragStart;

// Helper functions
  // Returns the unit which is selectable and given point
  // is on it
  // Given point is in pixel on the map !!!!
  CCUnit *FindSelectableUnitByPoint(CPoint pt);

// Unit marking
  // Variable which says if to use life bar marks
  bool m_bLifeBarMarking;
};

#endif // !defined(AFX_INTERACTIVEVIEWPORT_H__B40B79C3_5A33_11D4_B0BB_004F49068BD6__INCLUDED_)
