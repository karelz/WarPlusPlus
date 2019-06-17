#include "stdafx.h"
#include "BackgroundCachePiece.h"

#include "..\DataObjects\CMap.h"
#include "..\DataObjects\CMapSquare.h"

IMPLEMENT_DYNAMIC(CBackgroundCachePiece, CScratchSurface);

// Constructor
CBackgroundCachePiece::CBackgroundCachePiece()
{
  m_pMap = NULL;

  m_pNext = NULL;
}

// Destructor
CBackgroundCachePiece::~CBackgroundCachePiece()
{
}

// debug functions
#ifdef _DEBUG

void CBackgroundCachePiece::AssertValid() const
{
  CScratchSurface::AssertValid();

  ASSERT(m_pMap != NULL);
}

void CBackgroundCachePiece::Dump(CDumpContext &dc) const
{
  CScratchSurface::Dump(dc);
}

#endif


// Implementation --------------------------------------------------------

// Creation
// Creates the object -> no graphics drawn
void CBackgroundCachePiece::Create(DWORD dwXSize, DWORD dwYSize, CCMap *pMap)
{
  ASSERT(pMap != NULL);
  ASSERT((dwXSize > 0) && (dwYSize > 0));

  // copy the map pointer
  m_pMap = pMap;

  // set sizes
  SetWidth(dwXSize);
  SetHeight(dwYSize);

  // create the surface
  VERIFY(CScratchSurface::Create());
}

// Deletes the object
void CBackgroundCachePiece::Delete()
{
  // delete the surface
  CScratchSurface::Delete();

  // clear the map pointer
  m_pMap = NULL;
}


// Cache functions

// sets new position -> invalidate us
void CBackgroundCachePiece::SetPosition(DWORD dwXPos, DWORD dwYPos)
{
  ASSERT_VALID(this);

  // lock the access - no one can access us if we're not in valid state
  VERIFY(m_lockAccess.Lock());

  m_dwXPosition = dwXPos;
  m_dwYPosition = dwYPos;

  m_rcPosition.left = m_dwXPosition * MAPCELL_WIDTH;
  m_rcPosition.top = m_dwYPosition * MAPCELL_HEIGHT;
  m_rcPosition.right = m_rcPosition.left + GetAllRect()->Width();
  m_rcPosition.bottom = m_rcPosition.top + GetAllRect()->Height();
}

// Draws the graphics for new top-left coordinates
// the position is given in mapcells
// validates the object
void CBackgroundCachePiece::DrawContents()
{
  ASSERT_VALID(this);

  // now draw the graphics

  // first clear the background (only in debug mode)
#ifdef _DEBUG
  Fill(0);
#endif
  
  // Compute the size in mapcells (the size should be multiply of the mapcell size)
  DWORD dwXSize = GetAllRect()->Width() / MAPCELL_WIDTH;
  DWORD dwYSize = GetAllRect()->Height() / MAPCELL_HEIGHT;

  // First get all mapsquares which are interesting to us
  CTypedPtrList<CPtrList, CCMapSquare *> listMapSquares;
  {
    int nX, nY;
    // We must lock the map to read mapsquares
    VERIFY(m_pMap->GetGlobalLock()->ReaderLock());
    // we have to start one square above the affected one
    // becauese mapexes can override square boundaries
    for(nY = m_dwYPosition / MAPSQUARE_HEIGHT - 1; nY <= (int)((m_dwYPosition + dwYSize) / MAPSQUARE_HEIGHT); nY++){
      if((nY < 0) || (nY >= (int)m_pMap->GetHeight())) continue;
      // the same trick with starting index as for Y coord
      for(nX = m_dwXPosition / MAPSQUARE_WIDTH - 1; nX <= (int)((m_dwXPosition + dwXSize) / MAPSQUARE_WIDTH); nX++){
        if((nX < 0) || (nX >= (int)m_pMap->GetWidth())) continue;

        listMapSquares.AddHead(m_pMap->GetMapSquare((DWORD)nX, (DWORD)nY));
      }
    }
    // Unlock the map
    m_pMap->GetGlobalLock()->ReaderUnlock();
  }
  
  // Now go through all levels and draw mapexes
  {
    DWORD dwLevel;
    CTypedPtrList<CPtrList, CCMapexInstance *> listMapexes;
    POSITION pos;
    CCMapSquare *pMapSquare;
    CCMapexInstance *pMapexInstance;


    for(dwLevel = 1; dwLevel <=3; dwLevel++){

      // clear the list of mapexes
      listMapexes.RemoveAll();

      // first fill the list of all mapexes we will draw
      // it means -> go through the mapsquares
      pos = listMapSquares.GetHeadPosition();

      while(pos != NULL){
        pMapSquare = listMapSquares.GetNext(pos);

        // lock the mapsquare in the memory
//        pMapSquare->LoadAndLock();

        // append its mapexes to our list
        pMapSquare->AppendBLMapexes(dwLevel, &listMapexes);

        // unlock the mapsquare
//        pMapSquare->Unlock();
      }


      // now go through the list and draw them
      pos = listMapexes.GetHeadPosition();

      while(pos != NULL){
        pMapexInstance = listMapexes.GetNext(pos);

        // draw it - we must say the coords of the top left of our piece
        pMapexInstance->Draw(m_dwXPosition, m_dwYPosition, this);
      }
    }
  }

  // OK
  m_lockAccess.Unlock();
}