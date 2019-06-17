#include "stdafx.h"
#include "BackgroundCache.h"

#include "..\DataObjects\CMap.h"

// size of cache piece in mapcells
#define BKGCACHE_PIECE_WIDTH 8
#define BKGCACHE_PIECE_HEIGHT 8

IMPLEMENT_DYNAMIC(CBackgroundCache, CObject);

// Constructor
CBackgroundCache::CBackgroundCache() :
  m_semaphoreQueue(0, 0xF000000)
{
  m_aPieces = NULL;
  m_aTable = NULL;
  m_aHelperTable = NULL;

  m_pQueue = NULL;
  m_pWorkerThread = NULL;

  // set the last position to nonsense value
  m_rcLastViewportPosition.SetRect(-1000, -1000, -1000, -1000);
}

// Destructor
CBackgroundCache::~CBackgroundCache()
{
  ASSERT(m_pQueue == NULL);
  ASSERT(m_pWorkerThread == NULL);
  ASSERT(m_aPieces == NULL);
  ASSERT(m_aTable == NULL);
  ASSERT(m_aHelperTable == NULL);
}

// Debug functions
#ifdef _DEBUG

void CBackgroundCache::AssertValid() const
{
  CObject::AssertValid();

  ASSERT(m_aPieces != NULL);
  ASSERT(m_aTable != NULL);
  ASSERT(m_aHelperTable != NULL);
}

void CBackgroundCache::Dump(CDumpContext &dc) const
{
  CObject::Dump(dc);
}

#endif


// Implementation -----------------------------------------

// Creation ------------------------

// Creates the cache, need to know the size of the view in pixels
void CBackgroundCache::Create(DWORD dwXSize, DWORD dwYSize, CCMap *pMap)
{
  // first compute the size of the case
  // it means - number of pieces to hold
  m_dwWidth = ((dwXSize / MAPCELL_WIDTH) + (BKGCACHE_PIECE_WIDTH / 2)) / BKGCACHE_PIECE_WIDTH
    + 1 + 1; // one 1 needed necessary, second 1 for better performance
  m_dwHeight = ((dwYSize / MAPCELL_HEIGHT) + (BKGCACHE_PIECE_HEIGHT / 2)) / BKGCACHE_PIECE_HEIGHT
    + 1 + 1;

  // compute the size of the view in mapcells
  m_rcLastViewportPosition.right = m_rcLastViewportPosition.left + dwXSize / MAPCELL_WIDTH;
  m_rcLastViewportPosition.bottom = m_rcLastViewportPosition.top + dwYSize / MAPCELL_HEIGHT;

  // copy the map pointer
  ASSERT(pMap != NULL);
  m_pMap = pMap;

  ASSERT(m_aPieces == NULL);

  // allocate array for pieces
  m_aPieces = (CBackgroundCachePiece **) new BYTE[sizeof(CBackgroundCachePiece *) * m_dwWidth * m_dwHeight];

  // go through it and create pieces
  {
    DWORD dwCount = m_dwWidth * m_dwHeight;
    DWORD i;

    CBackgroundCachePiece *pPiece;

    for(i = 0; i < dwCount; i++){
      // allocate new piece
      pPiece = new CBackgroundCachePiece();

      // create it
      pPiece->Create(BKGCACHE_PIECE_WIDTH * MAPCELL_WIDTH, BKGCACHE_PIECE_HEIGHT * MAPCELL_HEIGHT, pMap);

      // write it to the array
      m_aPieces[i] = pPiece;
    }
  }

  // allocate our table (I know that they're the same)
  m_aTable = (CBackgroundCachePiece **) new BYTE[sizeof(CBackgroundCachePiece *) * m_dwWidth * m_dwHeight];
  m_aHelperTable = (CBackgroundCachePiece **) new BYTE[sizeof(CBackgroundCachePiece *) * m_dwWidth * m_dwHeight];

  // fill it in some way - it doesn't matter in which way
  {
    // for example we can fill it like the array
    DWORD dwCount = m_dwWidth * m_dwHeight;
    DWORD i;

    for(i = 0; i < dwCount; i++){
      m_aTable[i] = m_aPieces[i];
    }
  }

  // set our position to nonsense (but be carefull, no negative numbers (for ints))
  m_dwXPosition = 0x7F000000;
  m_dwYPosition = 0x7F000000;
  m_rcPosition.left = m_dwXPosition / BKGCACHE_PIECE_WIDTH;
  m_rcPosition.right = m_rcPosition.left + m_dwWidth;
  m_rcPosition.top = m_dwYPosition / BKGCACHE_PIECE_HEIGHT;
  m_rcPosition.bottom = m_rcPosition.top + m_dwHeight;

  // reset the exit event
  m_eventExit.ResetEvent();

  // start the worker thread
  m_pWorkerThread = AfxBeginThread(_WorkerThreadProc, this, THREAD_PRIORITY_BELOW_NORMAL,
    0, CREATE_SUSPENDED);
  m_pWorkerThread->m_bAutoDelete = FALSE;
  m_pWorkerThread->ResumeThread();
}

// Deletes the object
void CBackgroundCache::Delete()
{
  // Stop the worker thread
  if(m_pWorkerThread != NULL){
    m_eventExit.SetEvent();
    if(::WaitForSingleObject(m_pWorkerThread->m_hThread, INFINITE) != WAIT_OBJECT_0){
      TerminateThread(m_pWorkerThread->m_hThread, -1);
    }
    delete m_pWorkerThread;
    m_pWorkerThread = NULL;
  }

  // go through all pieces and delete them
  if(m_aPieces != NULL){
    {
      DWORD dwCount = m_dwWidth * m_dwHeight;
      DWORD i;
  
      for(i = 0; i < dwCount; i++){
        m_aPieces[i]->Delete();
        delete m_aPieces[i];
      }
    }

    // free the array
    delete m_aPieces;

    m_aPieces = NULL;
  }

  // free the table
  if(m_aTable != NULL){
    delete m_aTable;

    m_aTable = NULL;
  }
  if(m_aHelperTable != NULL){
    delete m_aHelperTable;
    m_aHelperTable = NULL;
  }
}


// Queue worker thread

// the thread procedure
UINT CBackgroundCache::_WorkerThreadProc(LPVOID pParam)
{
  // extract the this pointer
  CBackgroundCache *pThis = (CBackgroundCache *)pParam;
  pThis->WorkerThreadProc();

  return 0;
}

// real worker thread proc
void CBackgroundCache::WorkerThreadProc()
{
  // init array of handles
  HANDLE aHandles[2];
  aHandles[0] = (HANDLE)m_semaphoreQueue;
  aHandles[1] = (HANDLE)m_eventExit;

  // loop in endless cycle
  while(TRUE){
    // wait for those objects
    switch(WaitForMultipleObjects(2, aHandles, FALSE, INFINITE)){
    case WAIT_OBJECT_0:
      // the semaphore is up -> some piece is invalid
      // get it from the queue
      {
        CBackgroundCachePiece *pPiece;

        // lock the queue
        VERIFY(m_lockQueue.Lock());

        // remove the first item in the queue
        pPiece = m_pQueue;
        m_pQueue = pPiece->m_pNext;
        pPiece->m_pNext = NULL;

        // unlock the queue
        VERIFY(m_lockQueue.Unlock());

        // and drawit -> this makes it valid
        pPiece->DrawContents();
      }
      break;

    case WAIT_OBJECT_0 + 1:
      // exit event set
      // OK -> exit
      return;

    default:
      // oops some error
      // exit with error
      ASSERT(FALSE);
      return;
    }
  }
}


// Operations -----------------------------

// Sets new position of the viewport
void CBackgroundCache::SetViewportPosition(DWORD dwXPos, DWORD dwYPos)
{
  ASSERT_VALID(this);

  // lock the queue
  VERIFY(m_lockAccess.Lock());

  if((dwXPos == (DWORD)m_rcLastViewportPosition.left) && (dwYPos == (DWORD)m_rcLastViewportPosition.top)){
    // unlock the queue
    VERIFY(m_lockAccess.Unlock());
    return;
  }

  // now compute new rect on the map
  CRect rcNew;
  rcNew.left = dwXPos / BKGCACHE_PIECE_WIDTH;
  rcNew.right = rcNew.left + m_dwWidth;
  rcNew.top = dwYPos / BKGCACHE_PIECE_HEIGHT;
  rcNew.bottom = rcNew.top + m_dwHeight;

  // new view rect
  CRect rcNewViewport(m_rcLastViewportPosition);
  rcNewViewport.OffsetRect(dwXPos - m_rcLastViewportPosition.left, dwYPos - m_rcLastViewportPosition.top);

  // if they are equal -> just compute some special movements
  // this code works like prediction of the movement
  // it means if it moved to the right it's better to precompute the pieces
  // to the right than to the left (possible the movement will continue to the right)
  if(rcNew.EqualRect(&m_rcPosition)){
    // first compute it in the X direction
    {
      // determine the movement direction
      if(rcNewViewport.left != m_rcLastViewportPosition.left){

        if(rcNewViewport.left > m_rcLastViewportPosition.left){
          // here it's for the right movement
          // if we cross with our right edge (viewport edge) the half of the last piece in the cache
          // (the rightmost piece) -> move the cache to the right for one piece
          if((rcNewViewport.right + (BKGCACHE_PIECE_WIDTH / 2)) >=
            (int)(m_dwXPosition + m_dwWidth * BKGCACHE_PIECE_WIDTH)){
            rcNew.OffsetRect(1, 0);
          }
        }
        else{
          // and the left movement
          // the same thing as for the right movement but to the left side
          if((rcNewViewport.left - (BKGCACHE_PIECE_WIDTH / 2)) <= (int)m_dwXPosition){
            rcNew.OffsetRect(-1, 0);
          }
        }
      }
    }

    {
      // determine the movement direction
      if(rcNewViewport.top != m_rcLastViewportPosition.top){

        if(rcNewViewport.top > m_rcLastViewportPosition.top){
          // the down movement
          if((rcNewViewport.bottom + (BKGCACHE_PIECE_HEIGHT / 2)) >=
            (int)(m_dwYPosition + m_dwHeight * BKGCACHE_PIECE_HEIGHT)){
            rcNew.OffsetRect(0, 1);
          }
        }
        else{
          // the up movement
          if((rcNewViewport.top - (BKGCACHE_PIECE_HEIGHT / 2)) <= (int)m_dwYPosition){
            rcNew.OffsetRect(0, -1);
          }
        }
      }
    }

    // if we decide to move the cache -> compute new position to rcNew
    // and don't return (just leave the if)
    // elseway return (no work)
  }

  // clipp the new to the map
  if(rcNew.left < 0){ rcNew.OffsetRect(-rcNew.left, 0); }
  if(rcNew.top < 0){ rcNew.OffsetRect(0, -rcNew.top); }
  if(rcNew.right >= (int)m_pMap->GetWidthInCells()){ rcNew.OffsetRect(m_pMap->GetWidthInCells() - rcNew.right - 1, 0); }
  if(rcNew.bottom >= (int)m_pMap->GetHeightInCells()){ rcNew.OffsetRect(0, m_pMap->GetHeightInCells() - rcNew.bottom - 1); }

  // it remained the same -> no work
  if(rcNew.EqualRect(&m_rcPosition)){
    m_rcLastViewportPosition = rcNewViewport;
    return;
  }

  // compute intersection of the old and new one
  CRect rcIntersect;
  rcIntersect.IntersectRect(&rcNew, &m_rcPosition);

  // if the intersection is not empty -> do not recompute those pieces
  if(!rcIntersect.IsRectEmpty()){
    // first copy the table to new one
    memcpy(m_aHelperTable, m_aTable, sizeof(CBackgroundCachePiece *) * m_dwWidth * m_dwHeight);

    // zero the new table to mark pieces we didn't remake
    memset(m_aTable, 0, sizeof(CBackgroundCachePiece *) * m_dwWidth * m_dwHeight);

    // now copy the pieces which wan't change
    {
      DWORD x, y;

      for(y = rcIntersect.top; y < (DWORD)rcIntersect.bottom; y++){
        for(x = rcIntersect.left; x < (DWORD)rcIntersect.right; x++){
          // copy the piece
          m_aTable[(x - rcNew.left) + (y - rcNew.top) * m_dwWidth] =
            m_aHelperTable[(x - m_rcPosition.left) + (y - m_rcPosition.top) * m_dwWidth];

          // cleare the position -> mark it as empty
          m_aHelperTable[(x - m_rcPosition.left) + (y - m_rcPosition.top) * m_dwWidth] = NULL;
        }
      }
    }

    m_dwXPosition = rcNew.left * BKGCACHE_PIECE_WIDTH;
    m_dwYPosition = rcNew.top * BKGCACHE_PIECE_HEIGHT;

    // and go through the rest and fill it
    {
      DWORD xNew, yNew; // position in the new table
      DWORD xOld = 0, yOld = 0; // position in the old table
      DWORD dwOff;
      CBackgroundCachePiece *pPiece;

      // go through whole new table
      for(yNew = 0; yNew < m_dwHeight; yNew ++){
        for(xNew = 0; xNew < m_dwWidth; xNew ++){
          dwOff = yNew * m_dwWidth + xNew;

          // if the piece is already set -> no work
          if(m_aTable[dwOff] != NULL){
            continue;
          }

          // elseway find some free piece

          // go through the helper table
          while(m_aHelperTable[xOld + yOld * m_dwWidth] == NULL){
            // go to next piece (lines and rows then)
            xOld ++;
            if(xOld >= m_dwWidth){
              xOld = 0;
              yOld++;
              // oops some error -> we went out of the table
              // (that shouldn't happen)
              ASSERT(yOld < m_dwHeight);
            }
          }
          
          // now we have a free piece
          // so copy it to the new table
          pPiece = m_aTable[dwOff] = m_aHelperTable[xOld + yOld * m_dwWidth];
          
          // and mark the old piece position as empty
          m_aHelperTable[xOld + yOld * m_dwWidth] = NULL;
          
          // and set the new piece position
          pPiece->SetPosition(m_dwXPosition + xNew * BKGCACHE_PIECE_WIDTH,
            m_dwYPosition + yNew * BKGCACHE_PIECE_HEIGHT);
          
          VERIFY(m_lockQueue.Lock());

          // add it to queue
          pPiece->m_pNext = m_pQueue;
          m_pQueue = pPiece;
          m_semaphoreQueue.Unlock();

          VERIFY(m_lockQueue.Unlock());
        }
      }
    }

    // OK that's it
  }
  else{
    // elseway just recompute all pieces

    m_dwXPosition = rcNew.left * BKGCACHE_PIECE_WIDTH;
    m_dwYPosition = rcNew.top * BKGCACHE_PIECE_HEIGHT;

    // go through all pieces and reposition them
    {
      DWORD x, y;
      CBackgroundCachePiece *pPiece;

      for(y = 0; y < m_dwHeight; y++){
        for(x = 0; x < m_dwWidth; x++){
          pPiece = m_aTable[x + y * m_dwWidth];

          // set new position (this invalidates the piece)
          pPiece->SetPosition(m_dwXPosition + x * BKGCACHE_PIECE_WIDTH,
            m_dwYPosition + y * BKGCACHE_PIECE_HEIGHT);
          
          VERIFY(m_lockQueue.Lock());

          // add it to queue
          pPiece->m_pNext = m_pQueue;
          m_pQueue = pPiece;
          m_semaphoreQueue.Unlock();
        
          // unlock the queue
          VERIFY(m_lockQueue.Unlock());
        }
      }
    }
  }

  m_rcPosition = rcNew;

  // copy the viewport position
  m_rcLastViewportPosition = rcNewViewport;

  // unlock the queue
  VERIFY(m_lockAccess.Unlock());
}

void CBackgroundCache::Draw(CRect *pDestRect, CDDrawSurface *pDDSurface, CRect *pSourceRect)
{
  VERIFY(m_lockAccess.Lock());

  // go through the rect and find all pieces to draw from
  int x, y;
  CBackgroundCachePiece *pPiece;
  CRect rcPiece, rcDest;
  for(y = 0; y < (int)m_dwHeight; y++){
    for(x = 0; x < (int)m_dwWidth; x++){
      pPiece = m_aTable[x + y * m_dwWidth];
      
      rcPiece.IntersectRect(pSourceRect, pPiece->GetPosition());
      if(!rcPiece.IsRectEmpty()){
        pPiece->LockExclusive();
        
        // compute the destination rect
        rcDest = rcPiece;
        rcDest.OffsetRect(-pSourceRect->left, -pSourceRect->top);
        rcDest.OffsetRect(pDestRect->TopLeft());

        // offset the intersection to the piece coords
        rcPiece.OffsetRect(-pPiece->GetPosition()->left, - pPiece->GetPosition()->top);

        // draw it
        pDDSurface->Paste(rcDest.TopLeft(), pPiece, &rcPiece);
          
        pPiece->UnlockExclusive();
      }
    }
  }

  VERIFY(m_lockAccess.Unlock());
}