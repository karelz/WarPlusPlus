#ifndef BACKGRONUDCACHEPIECE_H_
#define BACKGRONUDCACHEPIECE_H_

class CCMap;

class CBackgroundCachePiece : public CScratchSurface
{
  DECLARE_DYNAMIC(CBackgroundCachePiece);

public:
  // constructor & destructor
  CBackgroundCachePiece();
  ~CBackgroundCachePiece();

// debug functions
#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

// Creation
  // creates the object (no graphics will be drawn here)
  // Sizes are in pixels - size of the piece
  // Also must have the map object -> we will ask it for mapsquares
  void Create(DWORD dwXSize, DWORD dwYSize, CCMap *pMap);

  // deletes the object
  void Delete();

// cache functions
  // draws new graphics
  // object should be locked for this
  // validates the object
  void DrawContents();

  // set new position - this also invalidates us
  // the object should be locked for this
  void SetPosition(DWORD dwXPos, DWORD dwYPos);

// Get/Set functions
  // returns X position (in mapcells) of the top-left corner
  DWORD GetXPosition(){ ASSERT_VALID(this); return m_dwXPosition; }
  // returns Y position (in mapcells) of the top-left corner
  DWORD GetYPosition(){ ASSERT_VALID(this); return m_dwYPosition; }

  // returns position in pixels
  CRect *GetPosition(){ return &m_rcPosition; }


// Lock functions
  // locks the object
  void LockExclusive(){ ASSERT_VALID(this); VERIFY(m_lockAccess.Lock()); }
  // unlocks the object
  void UnlockExclusive(){ ASSERT_VALID(this); VERIFY(m_lockAccess.Unlock()); }

private:
  // our pointer to the map
  CCMap *m_pMap;

  // position on the map
  // in mapcells -> top-left corner of our piece
  DWORD m_dwXPosition;
  DWORD m_dwYPosition;

  // position in pixels
  CRect m_rcPosition;

  // mutex for accessing the object
  CSemaphore m_lockAccess;

  // pointer to next piece in the queue
  CBackgroundCachePiece *m_pNext;

  friend class CBackgroundCache;
};

#endif