// Viewport.h: interface for the CViewport class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VIEWPORT_H__A41B4843_113B_11D4_849E_004F4E0004AA__INCLUDED_)
#define AFX_VIEWPORT_H__A41B4843_113B_11D4_849E_004F4E0004AA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\DataObjects\CMap.h"
#include "BackgroundCache.h"
#include "..\ServerCommunication\CWatchedRectangle.h"
#include "..\ServerCommunication\CServerUnitInfoReceiver.h"

// class representing one viewport on the map
class CViewport : public CWindow  
{
  DECLARE_DYNAMIC(CViewport);

public:

  // call ComputeFrame on all registered viewports
  static void DoFrameLoop();

  // sets new position of the view
  // sets top left corner position in pixels on the map
  void SetPosition(DWORD dwXPos, DWORD dwYPos);
  // returns position
  void GetPosition(DWORD &dwXPos, DWORD &dwYPos){ dwXPos = m_dwNewXPosition; dwYPos = m_dwNewYPosition; }

  // returns the size of the viewport in mapcells
  CSize GetSize(){ CRect rcWnd; rcWnd = GetWindowPosition();
    return CSize(rcWnd.Width() / MAPCELL_WIDTH, rcWnd.Height() / MAPCELL_HEIGHT); }

  // Computes next frame
  virtual void ComputeFrame();

  // Draws the viewport
  virtual void Draw(CDDrawSurface *pDDSurface, CRect *pBoundRect);

  // creation
  // the size of the window can't be changed at runtime (it wan't affect the size of the view)
  // we also must get pointer to the map object (to get data from)
  // The pExternalFullscreenBuffer specifies the fullscreen video memory buffer
  // You must set this to NULL if the viewport is not the fullscreen viewport
  void Create(CRect &rcWindowRect, CCMap *pMap, CCServerUnitInfoReceiver *pServerUnitInfoReceiver, CWindow *pParent,
    CDDrawSurface *pExternalFullscreenBuffer = NULL);
  // this will delete the object
  virtual void Delete();

  // constructor & destructor
	CViewport();
	virtual ~CViewport();

  // debug functions
#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

  typedef struct tagSUnitNode{
    DWORD m_dwX, m_dwY, m_dwZ; // position on the map in pixels
    CCUnit *m_pUnit;           // pointer to the unit object
    CCUnitAnimation *m_pAnimation; // current animation
    DWORD m_dwCurrentFrame;        // current frame
    CPoint m_ptFrameTopLeft;      // top left corner of the frame
    CRect m_rcPosition;        // position on the map in pixels
    DWORD m_dwLives;  // number of lifes and its maximum
    DWORD m_dwMaxLives; 
    DWORD m_dwMarks;  // marks assigned to unit

    // Selection mark helpers
    CPoint m_ptSelectionMarkTopLeft; // top left corner of the selection mark
    // Life bar helpers
    CRect m_rcLifeBar; // Rectangle of the life bar

    struct tagSUnitNode *m_pNext; // pointer to next node in the list
    BOOL m_bProcessed; // TRUE - the node can be deleted from the old nodes list
  } SUnitNode;

  // Helper class for receiving timer ticks (to slowdown the framerate.. debuging tasks)
  class CViewportTimerObserver : public CObserver
  {
    DECLARE_OBSERVER_MAP(CViewport::CViewportTimerObserver);

  public:
    CViewportTimerObserver(){}
    ~CViewportTimerObserver(){}

    void WaitForNextFrame();
    enum{ ID_SlowdownTimer = 0x090000000, };

  protected:
    void OnTimeTick(DWORD dwTime);

    CEvent m_eventNextFrame;
  };

  // Class for receiving notifications about viewport position changes
  class CViewportPositionObserver
  {
  public:
    CViewportPositionObserver(){ m_pViewport = NULL; }
    virtual ~CViewportPositionObserver(){ ASSERT(m_pViewport == NULL); }

    // Creates the position observer for given viewport and registers it
    void Create(CViewport *pViewport){
      ASSERT(pViewport != NULL);
      m_pViewport = pViewport;
      m_pViewport->AddPositionObserver(this);
    }
    // Deletes the position observer (and deregister it)
    virtual void Delete(){
      if(m_pViewport != NULL){
        m_pViewport->RemovePositionObserver(this);
        m_pViewport = NULL;
      }
    }

    // Returns the viewport
    CViewport *GetViewport(){ return m_pViewport; }

    // Sets new position of the viewport (without notifications)
    void SetViewportPosition(DWORD dwX, DWORD dwY){ m_pViewport->InnerSetPosition(dwX, dwY); }

    // Called when the position has changed (or the size of the viewport)
    // by the viewport
    // You must NOT set new position from this function or any other action with the viewport
    // You can only call Get functions
    virtual void OnPositionChanged(){};

  private:
    // The associated viewport
    CViewport *m_pViewport;
    // Next position observer in the list of observers on the viewport
    CViewportPositionObserver *m_pNext;

    friend class CViewport;
  };

protected:
  // pointer to the map object
  CCMap *m_pMap;

  // Graphical surface - the real view on the map
  // We'll call it frame buffer (it does make some kind of sense)
  CScratchSurface m_FrameBuffer;
  CSemaphore m_lockFrameBuffer;

  // External fullscreen buffer
  CDDrawSurface *m_pFullscreenBuffer;

  // Our background cache
  CBackgroundCache m_BackgroundCache;

  // the network watched rectangle (for the server)
  CCWatchedRectangle m_WatchedRectangle;

  // pointer to list of all last drawn units (full info about them)
  // This list is sorted in a way that if you go through it
  // and draw the units in that order the result will be the right one
  // (It means that first is the unit with less z order and most left and top)
  SUnitNode *m_pUnitNodeList;

  // lock for this viewport
  CReadWriteLock m_lockViewport;


  // new position on the map (in pixels) (can differ when someone sets it, till next call to ComputeFrame)
  DWORD m_dwNewXPosition, m_dwNewYPosition;


  // the real position on the map in pixels
  DWORD m_dwXPosition, m_dwYPosition;
  // position on the map in mapcells (rounded down)
  DWORD m_dwXCellPosition, m_dwYCellPosition;

  // rect list to update
  // accessed only in ComputeFrame
  CUpdateRectList m_RectUpdateList;

// these can't be changed runtime ->
  // size of the viewport in pixels
  DWORD m_dwXSize, m_dwYSize;
  // size of the viewport in mapcells (rounded up)
  DWORD m_dwXCellSize, m_dwYCellSize;
// <- can't be changed runtime

  // Pool for unit node structures (non multithread - only one thread accesses it)
  // it's shared by all viewports
  static CTypedMemoryPool<SUnitNode> m_UnitNodePool;

// Viewport list
  // pointer to the next viewport in the list of viewports
  CViewport *m_pNextViewport;
  // list of all viewports
  static CViewport *m_pViewportList;
  // and lock for it
  static CMutex m_lockViewportList;
  // add this viewport to the list
  void AddViewport();
  // removes the viewport from the list
  void RemoveViewport();

  // draws new rectangle
  void DrawRect(CRect &rcUpdateRect);

// Timer
  static CViewportTimerObserver m_ViewportTimerObserver;

// Unit marks
  enum{
    UnitMark_None = 0,     // The unit has no mark on it
    UnitMark_Selected = 1, // The unit has selection mark (selected unit)
	  UnitMark_LifeBar = 2,  // The unit has a life bar drawn over it
  };

  // This functions is called before computing the frame updates
  // You can add any rectangles to given list and this is the way you
  // register some rects for updating
  // The viewport is locked
  virtual void AddOverlayUpdates ( CUpdateRectList * pUpdateRectList ) {}
  // Draw any overlay - it means here you can draw anything over
  // the viewport graphics
  // But if you want to change it, you must register a rect for
  // updating in AddOverlayUpdates
  // You should draw it to given surface, as if you'd draw it to the
  // frame buffer (usually it will be the frame buffer)
  // The rcUpdateRect is the rect that you should draw into
  // (in fact the surface will have set the clipping just for this rect)
  // The viewport is locked
  virtual void DrawOverlay ( CDDrawSurface * pSurface, CRect & rcUpdateRect ) {}

  // Called before the all units are locked
  // int the computing frames
  // If you need some lock in GetUnitMarks or AdjustNodeSize lock it here
  virtual void BeforeUnitsLocking () {}
  // Called after all units are unlocked
  // unlock everything you've locked in BeforeUnitsLocking
  virtual void AfterUnitsLocking () {}

  // Virtual function which returns unit marks for given unit
  // used by drawing functions
  // This function should be implemented if you want units in this view
  // to have some marks
  // Default implementation returns UnitMark_None
  virtual DWORD GetUnitMarks(CCUnit *pUnit){ return UnitMark_None; }

  // Virtual function that resizes the unit node rectangle to
  // contain whole unit with all marks
  // Modify the node structure itself
  virtual void AdjustNodeSize(SUnitNode *pNode);

// Helper function for minimaps to access this viewport
  // Sets position of the viewport without notifing the associated minimap
  void InnerSetPosition(DWORD dwXPos, DWORD dwYPos);
  // Adds new viewport position observer
  void AddPositionObserver(CViewportPositionObserver *pObserver);
  // Removes the viewport position observer
  void RemovePositionObserver(CViewportPositionObserver *pObserver);

  // List of all connected position observers
  CViewportPositionObserver *m_pPositionObservers;

  friend class CViewport::CViewportPositionObserver;
};

#endif // !defined(AFX_VIEWPORT_H__A41B4843_113B_11D4_849E_004F4E0004AA__INCLUDED_)
