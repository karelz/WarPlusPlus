#ifndef BACKGROUNDCACHE_H_
#define BACKGROUNDCACHE_H_

#include "BackgroundCachePiece.h"

class CCMap;

// Background cache for one viewport
class CBackgroundCache : public CObject
{
  DECLARE_DYNAMIC(CBackgroundCache);

public:
  // Constructor & destructor
  CBackgroundCache();
  ~CBackgroundCache();

// Debug functions
#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif


// Creation
  // Creates the cache
  // Need size of the viewport in pixels
  // Also must have a pointer to map object (mapsquares will be get from there in the future)
  void Create(DWORD dwXSize, DWORD dwYSize, CCMap *pMap);

  // Deletes the object
  void Delete();

// Operations
  // Sets new position of the viewport on the map
  // this can result in recomputation of some bkg pieces
  // take it as an advice for the cache
  // You should call it every time the viewport moves
  // position is in mapcells (absolute for the map)
  void SetViewportPosition(DWORD dwXPos, DWORD dwYPos);

// Drawing  
  // Draws some part of the cache to destination surface
  void Draw(CRect *pDestRect, CDDrawSurface *pDDSurface, CRect *pSourceRect);

private:
// Cache data
  // array of pieces, no sorting here, just remeber pointers
  CBackgroundCachePiece **m_aPieces;

  // another array, here we will keep a table of pieces
  // here it will be sorted as on the screen
  CBackgroundCachePiece **m_aTable;
  // and a helper table (used during computations)
  CBackgroundCachePiece **m_aHelperTable;

// Position on the map
  // sizes
  // Width in pieces
  DWORD m_dwWidth;
  // height in pieces
  DWORD m_dwHeight;

  // the map object
  CCMap *m_pMap;

  // position on the map (in mapcells)
  DWORD m_dwXPosition;
  DWORD m_dwYPosition;

  // position on the map (in pieces)
  CRect m_rcPosition;

  // last known position of the viewport on the map in mapcells
  CRect m_rcLastViewportPosition;

  // worker thread -> there we will compute the graphics
  CWinThread *m_pWorkerThread;

// Queue
  // queue of pieces to be drawn in the worker thread
  CBackgroundCachePiece *m_pQueue;

  // and semaphore which works as counter
  CSemaphore m_semaphoreQueue;
  // and event for exiting
  CEvent m_eventExit;

  // mutex for locking the queue
  CMutex m_lockQueue;

  // Access to the object as whole
  CMutex m_lockAccess;

  // the worker thread proc, which will be executed
  static UINT _WorkerThreadProc(LPVOID pParam);
  // the real worker thread proc
  void WorkerThreadProc();
};

#endif