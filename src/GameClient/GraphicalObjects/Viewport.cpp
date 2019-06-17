// Viewport.cpp: implementation of the CViewport class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Viewport.h"
#include "SpecialBlt.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define MAX_FRAME_RATE 1000

#define LIFEBAR_HEIGHT 4
#define LIFEBAR_BACKGROUNDCOLOR RGB32 ( 0, 0, 0 )
// These are coef for color computing
// If the unit has more than one half of lives -> BARCOLOR2 is used
// else the BARCOLOR1 is used
// The expression is as follows
// If the unit has 75% of lives
// nRed = LIFEBAR_BARCOLOR2_RED_ABS + ( LIFEBRA_BARCOLOR2_RED_MULT * 0.75 )
#define LIFEBAR_BARCOLOR1_RED_ABS 255
#define LIFEBAR_BARCOLOR1_RED_MULT 0
#define LIFEBAR_BARCOLOR1_GREEN_ABS 0
#define LIFEBAR_BARCOLOR1_GREEN_MULT 240
#define LIFEBAR_BARCOLOR1_BLUE_ABS 0
#define LIFEBAR_BARCOLOR1_BLUE_MULT 0
#define LIFEBAR_BARCOLOR2_RED_ABS 255
#define LIFEBAR_BARCOLOR2_RED_MULT -255
#define LIFEBAR_BARCOLOR2_GREEN_ABS 240
#define LIFEBAR_BARCOLOR2_GREEN_MULT -40
#define LIFEBAR_BARCOLOR2_BLUE_ABS 0
#define LIFEBAR_BARCOLOR2_BLUE_MULT 0

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CViewport, CWindow)

BEGIN_OBSERVER_MAP(CViewport::CViewportTimerObserver, CObserver)
  BEGIN_NOTIFIER(ID_SlowdownTimer)
    ON_TIMETICK();
  END_NOTIFIER();
END_OBSERVER_MAP(CViewport::CViewportTimerObserver, CObserver)

// the slowdown observer
CViewport::CViewportTimerObserver CViewport::m_ViewportTimerObserver;

// create the memory pool
CTypedMemoryPool<CViewport::SUnitNode> CViewport::m_UnitNodePool(100);

// List of all viewports
CViewport *CViewport::m_pViewportList = NULL;
// Lock for that list
CMutex CViewport::m_lockViewportList;

// constructor
CViewport::CViewport()
{
  m_pUnitNodeList = NULL;
  m_pMap = NULL;
  m_pPositionObservers = NULL;
}

// destructor
CViewport::~CViewport()
{
  ASSERT(m_pMap == NULL);
}


// Debug functions
#ifdef _DEBUG

void CViewport::AssertValid() const
{
  CWindow::AssertValid();

  ASSERT(m_pMap != NULL);
}

void CViewport::Dump(CDumpContext &dc) const
{
  CWindow::Dump(dc);
}

#endif


// Creation
void CViewport::Create(CRect &rcWindowRect, CCMap *pMap, CCServerUnitInfoReceiver *pServerUnitInfoReceiver, CWindow *pParent,
                       CDDrawSurface *pExternalFullscreenBuffer)
{
  ASSERT(pMap != NULL);
  ASSERT(pParent != NULL); // we should have some parent window

  // Copy pointer to the map
  m_pMap = pMap;

  m_RectUpdateList.Create(1, 1);

  // Create the frame buffer
  m_FrameBuffer.SetWidth(rcWindowRect.Width());
  m_FrameBuffer.SetHeight(rcWindowRect.Height());
  m_FrameBuffer.Create();
  // clear it
  m_FrameBuffer.Fill(0);

  // Copy pointer to fullscreen buffer
  m_pFullscreenBuffer = pExternalFullscreenBuffer;

  // init the background cache
  // must say beigger size (bkg cache computes everything in mapcells, but we do it in pixels ... )
  m_BackgroundCache.Create(rcWindowRect.Width() + MAPCELL_WIDTH, rcWindowRect.Height() + MAPCELL_HEIGHT, m_pMap);

  // Create the window
  m_bTransparent = FALSE;
  CWindow::Create(&rcWindowRect, pParent);

  // set our size and position
  m_dwNewXPosition = 0;
  m_dwNewYPosition = 0;
  m_dwXPosition = 0x07F000000;
  m_dwYPosition = 0x07F000000;
  m_dwXSize = rcWindowRect.Width();
  m_dwYSize = rcWindowRect.Height();
  m_dwXCellSize = m_dwXSize / MAPCELL_WIDTH + 1;
  m_dwYCellSize = m_dwYSize / MAPCELL_HEIGHT + 1;

  // register our watched rectangle
  m_WatchedRectangle.Create(pServerUnitInfoReceiver);
  m_WatchedRectangle.SetSize(m_dwXCellSize, m_dwYCellSize);
  pServerUnitInfoReceiver->AddWatchedRectangle(&m_WatchedRectangle);

  // register us as new viewport
  AddViewport();
}

// Delete the object
void CViewport::Delete()
{
  // unregister us
  RemoveViewport();

  // unregister watched rectangle in the network
  m_WatchedRectangle.Delete();

  // clear the pointer
  m_pMap = NULL;

  // delete the window
  CWindow::Delete();

  // close the background cache
  m_BackgroundCache.Delete();

  // forget the fullscreen buffer
  m_pFullscreenBuffer = NULL;

  // delete the frame buffer
  m_FrameBuffer.Delete();

  // Delete unit node list
  {
    SUnitNode *pNode = m_pUnitNodeList, *pDel;
    while(pNode != NULL){
      pDel = pNode;
      pNode = pNode->m_pNext;
      pDel->m_pUnit->Release ();
      m_UnitNodePool.Free(pDel);
    }
  }

  // Delete the rect update list
  m_RectUpdateList.Delete();
}


// Draws the viewport
void CViewport::Draw(CDDrawSurface *pDDSurface, CRect *pBoundRect)
{
  VERIFY(m_lockFrameBuffer.Lock());
  try{
    // just copy our frame buffer to the destination
    pDDSurface->Paste(0, 0, &m_FrameBuffer);
  }
  catch(...){
    VERIFY(m_lockFrameBuffer.Unlock());
    throw;
  }
  VERIFY(m_lockFrameBuffer.Unlock());
}


void CViewport::SetPosition(DWORD dwXPos, DWORD dwYPos)
{
  // First call the position setting
  InnerSetPosition(dwXPos, dwYPos);

  VERIFY(m_lockViewport.ReaderLock());

  // Then notify all position observers
  CViewportPositionObserver *pObserver;
  pObserver = m_pPositionObservers;
  while(pObserver != NULL){
    pObserver->OnPositionChanged();
    pObserver = pObserver->m_pNext;
  }

  m_lockViewport.ReaderUnlock();
}

void CViewport::InnerSetPosition(DWORD dwXPos, DWORD dwYPos)
{
  ASSERT_VALID(this);

  // lock us
  VERIFY(m_lockViewport.WriterLock());

  // first set new position in pixels
  m_dwNewXPosition = dwXPos;
  m_dwNewYPosition = dwYPos;
  if((int)m_dwNewXPosition < 0) m_dwNewXPosition = 0;
  if((int)m_dwNewYPosition < 0) m_dwNewYPosition = 0;
  if((m_dwNewXPosition + m_dwXSize) >= (m_pMap->GetWidth() * MAPSQUARE_WIDTH * MAPCELL_WIDTH))
    m_dwNewXPosition = m_pMap->GetWidth() * MAPSQUARE_WIDTH * MAPCELL_WIDTH - m_dwXSize - 1;
  if((m_dwNewYPosition + m_dwYSize) >= (m_pMap->GetHeight() * MAPSQUARE_HEIGHT * MAPCELL_HEIGHT))
    m_dwNewYPosition = m_pMap->GetHeight() * MAPSQUARE_HEIGHT * MAPCELL_HEIGHT - m_dwYSize - 1;


  // Here we can advise the network caches to load some data
  // means -> set new position in cache
  m_WatchedRectangle.SetPos(m_dwNewXPosition / MAPCELL_WIDTH, m_dwNewYPosition / MAPCELL_HEIGHT);

  m_lockViewport.WriterUnlock();
}

// Computes new frame
void CViewport::ComputeFrame()
{
  ASSERT_VALID(this);

  CRect rcFrameBufferSource;
  CPoint ptFrameBufferDest;
  BOOL bMoveFrameBuffer = FALSE;

  // first copy out position information for this viewport
  {
    VERIFY(m_lockViewport.ReaderLock());

    // Also give a chance to overlays
    AddOverlayUpdates ( &m_RectUpdateList );
  
    // determine if the position changed
    {
      // the intersection rectangle between the old and new position
      CRect rcOld(m_dwXPosition, m_dwYPosition, m_dwXPosition + m_dwXSize, m_dwYPosition + m_dwYSize);
      CRect rcNew(m_dwNewXPosition, m_dwNewYPosition, m_dwNewXPosition + m_dwXSize, m_dwNewYPosition + m_dwYSize);

      // if they're equal -> no move
      if(!rcOld.EqualRect(&rcNew)){
        
        // get the intersection of them
        CRect rcIntersection;
        rcIntersection.IntersectRect(&rcOld, &rcNew);
        
        // if empty -> brand new position
        if(rcIntersection.IsRectEmpty()){
          // empty intersection
          // no copying just invalidate all view
          rcNew.OffsetRect(-(int)m_dwNewXPosition, -(int)m_dwNewYPosition);
          m_RectUpdateList.AddRect(&rcNew);
        }
        else{
          // intersection is valid -> copy it
          ptFrameBufferDest.x = rcIntersection.left - rcNew.left;
          ptFrameBufferDest.y = rcIntersection.top - rcNew.top;
          rcFrameBufferSource = rcIntersection;
          rcFrameBufferSource.OffsetRect(-rcOld.left, -rcOld.top);
          
          // set the move frame buffer flag
          bMoveFrameBuffer = TRUE;
          
          // mark the rest as invalid 
          // first mark the left or right piece (without the cross section)
          CRect rcUpdate(rcNew.left, rcIntersection.top, rcNew.right, rcIntersection.bottom);
          if(rcIntersection.left > rcNew.left)
            rcUpdate.right = rcIntersection.left;
          else
            rcUpdate.left = rcIntersection.right;
          // transform it to the viewport coords
          rcUpdate.OffsetRect(-rcNew.left, -rcNew.top);
          m_RectUpdateList.AddRect(&rcUpdate);
          
          // then mark the top/bottom belt with the crossing section
          rcUpdate = rcNew;
          if(rcIntersection.top > rcNew.top)
            rcUpdate.bottom = rcIntersection.top;
          else
            rcUpdate.top = rcIntersection.bottom;
          // transform the rect to the viewport coords
          rcUpdate.OffsetRect(-rcNew.left, -rcNew.top);
          m_RectUpdateList.AddRect(&rcUpdate);
        }
      }
    }

    // we'll copy it to our member variables
    m_dwXPosition = m_dwNewXPosition;
    m_dwYPosition = m_dwNewYPosition;

    m_lockViewport.ReaderUnlock();

    // and compute the position in mapcells
    m_dwXCellPosition = m_dwXPosition / MAPCELL_WIDTH;
    m_dwYCellPosition = m_dwYPosition / MAPCELL_HEIGHT;

    // set new position in background caches
    m_BackgroundCache.SetViewportPosition(m_dwXCellPosition, m_dwYCellPosition);
  }

  // we must go through all interesting mapsquares and create a list of units in them
  CCUnit *pAllUnits = NULL;
  {
    // lock the map
    VERIFY(m_pMap->GetGlobalLock()->ReaderLock());

    // go through all interesting mapsquares
    int nX, nY;
    // we have to start one square above the affected one
    // becauese units can override square boundaries
    // and one square under (they can override it in every direction)
    for(nY = m_dwYCellPosition / MAPSQUARE_HEIGHT - 1; nY <= (int)((m_dwYCellPosition + m_dwYCellSize) / MAPSQUARE_HEIGHT + 1); nY++){
      if((nY < 0) || (nY >= (int)m_pMap->GetHeight())) continue;
      // the same trick with starting index as for Y coord
      for(nX = m_dwXCellPosition / MAPSQUARE_WIDTH - 1; nX <= (int)((m_dwXCellPosition + m_dwXCellSize) / MAPSQUARE_WIDTH + 1); nX++){
        if((nX < 0) || (nX >= (int)m_pMap->GetWidth())) continue;

        // go through all units in the mapex and add them
        CCUnit *pFirstUnit = m_pMap->GetMapSquare(nX, nY)->GetUnitList();
        CCUnit *pHlpUnit = pFirstUnit;
        if(pHlpUnit != NULL){
          do{
            ASSERT_VALID(pHlpUnit);
            pHlpUnit->AddRef();

            // append the unit into our list
            pHlpUnit->m_pViewportNext = pAllUnits;
            pAllUnits = pHlpUnit;

            // step to next unit in mapsquare
            pHlpUnit = pHlpUnit->GetMapSquareNext();
          }while(pHlpUnit != pFirstUnit);
        }
      }
    }

    // unlock the map
    m_pMap->GetGlobalLock()->ReaderUnlock();
  }

  // Now we have in pAllUnits list of all units from interesting mapexes
  // it's about to go through it and filter out those who doesn't intersect with our viewport
  // and for those who does, we will create the SUnitNode structure and add it to our new list

  // new list of units in the view
  SUnitNode *pNewUnitNodes = NULL;
  {
    // rect of the viewport on the map
    CRect rcViewport(m_dwXPosition, m_dwYPosition, m_dwXPosition + m_dwXSize, m_dwYPosition + m_dwYSize);
    CRect rcIntersection;

    BeforeUnitsLocking ();

    // go through all units in the pAllUnits list
    CCUnit *pUnit = pAllUnits, *pLastUnit;
    while(pUnit != NULL){
      ASSERT_VALID(pUnit);
      // lock the unit for reading
      VERIFY(pUnit->GetLock()->ReaderLock());

      // first check if it intersects our rectangle
      rcIntersection.IntersectRect(&(pUnit->GetPositionRect()), &rcViewport);
      if(!rcIntersection.IsRectEmpty()){
        // it's visible from the viewport
        // create the info structure
        SUnitNode *pUnitNode = m_UnitNodePool.Allocate();

        // fill it
        pUnitNode->m_dwX = pUnit->GetXPixelPosition();
        pUnitNode->m_dwY = pUnit->GetYPixelPosition();
        pUnitNode->m_dwZ = pUnit->GetZPosition();

        pUnitNode->m_dwLives = pUnit->GetLives();
        pUnitNode->m_dwMaxLives = pUnit->GetMaxLives ();

        pUnitNode->m_pAnimation = pUnit->GetCurrentAnimation();
        pUnitNode->m_dwCurrentFrame = pUnit->GetCurrentFrameNum();

        pUnit->AddRef();
        pUnitNode->m_pUnit = pUnit;
        pUnitNode->m_rcPosition = pUnit->GetPositionRect();
        pUnitNode->m_ptFrameTopLeft = pUnitNode->m_rcPosition.TopLeft();

        pUnitNode->m_bProcessed = FALSE;

        pUnitNode->m_dwMarks = GetUnitMarks(pUnit);

        AdjustNodeSize(pUnitNode);

        // add it to our list
        // Here we must sort the list right
        {
          // Find node to insert our new node before
          SUnitNode **pNode = &pNewUnitNodes;
          while((*pNode) != NULL){
            if(((*pNode)->m_dwZ > pUnitNode->m_dwZ) ||
              (((*pNode)->m_dwZ == pUnitNode->m_dwZ) && (((*pNode)->m_dwY > pUnitNode->m_dwY) ||
              (((*pNode)->m_dwY == pUnitNode->m_dwY) && ((*pNode)->m_dwX >= pUnitNode->m_dwX)) ) ) ){
              // OK Insert our unit before this one
              break;
            }
 
            // Go to the next node
            pNode = &((*pNode)->m_pNext);
          }

          // Now we have in (*pNode) no to insert our before
          // and in pNode address of the pointer which is pointing on it
          // So just insert our new node
          pUnitNode->m_pNext = *pNode;
          *pNode = pUnitNode;
        }
      }

      // unlock the unit
      pUnit->GetLock()->ReaderUnlock();
      pLastUnit = pUnit;
      pUnit = pUnit->m_pViewportNext;
      pLastUnit->Release();
    }

    AfterUnitsLocking ();
  }

  // OK that's it, now we have a list of unit nodes for this view
  // so go through it and find the corresponding node in the old list
  VERIFY(m_lockViewport.WriterLock());
  {
    // point to offset the rect to transform it from the map coords to view ones
    CPoint ptMapToView(-(int)m_dwXPosition, -(int)m_dwYPosition);

    SUnitNode *pUnitNode = pNewUnitNodes;
    while(pUnitNode != NULL){
      // try to find corresponding node
      {
        SUnitNode *pOldUnitNode = m_pUnitNodeList;
        while(pOldUnitNode != NULL){
          if(pOldUnitNode->m_pUnit == pUnitNode->m_pUnit){
            // we've found it
            break;
          }
          pOldUnitNode = pOldUnitNode->m_pNext;
        }

        if(pOldUnitNode == NULL){
          // there wasn't any node -> the unit is new in the viewport
          // update the rect for the new node
          CRect rc(pUnitNode->m_rcPosition);
          rc.OffsetRect(ptMapToView);
          m_RectUpdateList.AddRect(&rc);
        }
        else{
          // there was some node -> compare them and if they're different update rectangles
          if((pUnitNode->m_dwLives != pOldUnitNode->m_dwLives) ||
            (pUnitNode->m_pAnimation != pOldUnitNode->m_pAnimation) ||
            (pUnitNode->m_dwCurrentFrame != pOldUnitNode->m_dwCurrentFrame) ||
            (!pUnitNode->m_rcPosition.EqualRect(&pOldUnitNode->m_rcPosition)) ||
            (pUnitNode->m_dwMarks != pOldUnitNode->m_dwMarks) ||
            (pUnitNode->m_dwMaxLives != pOldUnitNode->m_dwMaxLives) ){
            // they're different -> update old rect and new one as well
            CRect rc(pUnitNode->m_rcPosition);
            rc.OffsetRect(ptMapToView);
            m_RectUpdateList.AddRect(&rc);

            rc = pOldUnitNode->m_rcPosition;
            rc.OffsetRect(ptMapToView);
            m_RectUpdateList.AddRect(&rc);
          }

          // anyway, mark the old node as proccesed
          pOldUnitNode->m_bProcessed = TRUE;
        }
      }
      pUnitNode = pUnitNode->m_pNext;
    }

    // now go through the old ones and delete them
    // if there's some unprocessed -> mark it for updating
    pUnitNode = m_pUnitNodeList;
    SUnitNode *pDelNode;
    while(pUnitNode != NULL){
      pDelNode = pUnitNode;
      if(!pUnitNode->m_bProcessed){
        // unprocessed update it's rect
        // it's unit which dissapiered fromthe viewport
        CRect rc(pUnitNode->m_rcPosition);
        rc.OffsetRect(ptMapToView);
        m_RectUpdateList.AddRect(&rc);
      }
      pUnitNode = pUnitNode->m_pNext;

      // delete it
      pDelNode->m_pUnit->Release();
      m_UnitNodePool.Free(pDelNode);
    }
  }

  // Now everything is done, just exchange the lists of unit nodes
  // make the new one the current (old) one
  // We must lock the viewport for writing to do so
  m_pUnitNodeList = pNewUnitNodes;
  m_lockViewport.WriterUnlock();

  
  ///////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////


  // Now let's draw changes
  {
    CUpdateRectList::SRectNode *pRectNode;

    VERIFY(m_lockFrameBuffer.Lock());

    try{

    // Move the frame if needed
    if(bMoveFrameBuffer){
      // First move our internal frame buffer
      CRect rcDest(ptFrameBufferDest, rcFrameBufferSource.Size());
      LPBYTE pData;
      DWORD dwPitch;
      pData = (LPBYTE)m_FrameBuffer.Lock(m_FrameBuffer.GetAllRect(), 0, dwPitch);
      Special_Blt(pData, dwPitch, rcFrameBufferSource, rcDest, g_pDirectDraw->Is32BitMode());
      m_FrameBuffer.Unlock(pData);

      // Then move the fullscreen buffer if we're the special fullscreen view
      if(m_pFullscreenBuffer != NULL){
        // First just roughly copy the data
        m_pFullscreenBuffer->Paste(ptFrameBufferDest, m_pFullscreenBuffer, &rcFrameBufferSource);

        // Then add some rectangles cto updateing
        // We must add all windows (toolbars) above us
        // and also we must add old parts of windows which moved
        CRect rcWindow;
        CRect rcUnion;
        CWindow *pWindow;
        pWindow = GetFirstChild();
        DWORD dwCount = 0;
        while(pWindow != NULL){
          // If the window is hidden -> nothing to do
          if(pWindow->IsVisible()){

            // We'll get the screen position cause we must be the fullscreen view
            rcWindow = pWindow->GetWindowScreenPosition();
            rcUnion = rcWindow;
            rcUnion.OffsetRect(ptFrameBufferDest.x - rcFrameBufferSource.left, ptFrameBufferDest.y - rcFrameBufferSource.top);
            rcUnion.UnionRect(&rcUnion, &rcWindow);

            // Update the union of this ractangles
            UpdateRect(&rcUnion);
          
          }
          // Go to the next child window
          pWindow = GetNextChild(pWindow);
        }
      }
    }

    // If it's full screen view, or the view has not moved
    // -> draw only changes
    if ( ( m_pFullscreenBuffer != NULL ) || ( !bMoveFrameBuffer ) )
    {
      // Go through all rect nodes
      pRectNode = m_RectUpdateList.GetFirstRectNode();
      while(pRectNode != NULL){
        // Draw the rectangle
        DrawRect(pRectNode->m_rcRect);

        // Mark the rect as the one needed to redraw
        UpdateRect(&(pRectNode->m_rcRect));

        // go to the next one
        pRectNode = pRectNode->m_pNext;
      }
    }
    else
    {
      // Go through all rect nodes
      pRectNode = m_RectUpdateList.GetFirstRectNode();
      while(pRectNode != NULL){
        // Draw the rectangle
        DrawRect(pRectNode->m_rcRect);

        // go to the next one
        pRectNode = pRectNode->m_pNext;
      }

      // Here we must update the whole view
      UpdateRect ( NULL );
    }

    // Clear the update list
    m_RectUpdateList.RemoveAll();

    }
    catch(...){
      VERIFY(m_lockFrameBuffer.Unlock());
      throw;
    }

    VERIFY(m_lockFrameBuffer.Unlock());
  }
}

// Updates given rect in the FrameBuffer
void CViewport::DrawRect(CRect &rcUpdateRect)
{
  // first draw the background from cache
  CRect rcSource = rcUpdateRect;
  rcSource.OffsetRect(m_dwXPosition, m_dwYPosition);
  m_BackgroundCache.Draw(&rcUpdateRect, &m_FrameBuffer, &rcSource);

  // Draw units in the rect
  // We have the list of interesting units in the m_pUnitNodeList
  // So go through it and check if it intersects our rectangle
  {
    CRect rcUpdateMapRect;
    rcUpdateMapRect = rcUpdateRect;
    rcUpdateMapRect.OffsetRect(m_dwXPosition, m_dwYPosition);

    CRect rcIntersect, rcOldClip;

    // Set new clipping for the frame buffer
    m_FrameBuffer.SetClipRect(&rcUpdateRect, &rcOldClip);

    SUnitNode *pNode = m_pUnitNodeList;
    while(pNode != NULL){

      // Get the graphics
      CCUnitSurface *pGraphics;

      // try to lock the animation
      if(pNode->m_pAnimation->TestLock()){
        // OK locked -> try if it's in the cache
        if(pNode->m_pAnimation->IsLoaded()){
          // OK loaded -> so draw it
          pGraphics = pNode->m_pAnimation->GetFrame(pNode->m_dwCurrentFrame);

		  // Draw the selection under the unit
          if(pNode->m_dwMarks & UnitMark_Selected){
            // Get the animation and frame
            CCUnitAnimation *pSelectionAnimation = pNode->m_pUnit->GetUnitType()->GetSelectionMarkAppearance()->GetDirection(pNode->m_pUnit->GetCurrentDirection());

            // Lock the animation
            pSelectionAnimation->LoadAndLock();
            CCUnitSurface *pFrame = pSelectionAnimation->GetFrame(0);
            
            // Draw it
            m_FrameBuffer.Paste(pNode->m_ptSelectionMarkTopLeft.x - m_dwXPosition,
              pNode->m_ptSelectionMarkTopLeft.y - m_dwYPosition, pFrame);

            // Unlock the animation
            pSelectionAnimation->Unlock();
          }

          // draw the unit
          m_FrameBuffer.Paste(pNode->m_ptFrameTopLeft.x - m_dwXPosition,
            pNode->m_ptFrameTopLeft.y - m_dwYPosition, pGraphics);

          // Draw the life bar over the unit
          if ( pNode->m_dwMarks & UnitMark_LifeBar )
          {
            // Draw the life bar background
            CRect rcBackground ( pNode->m_rcLifeBar );
            rcBackground.OffsetRect ( -(int)m_dwXPosition, -(int)m_dwYPosition );
            m_FrameBuffer.Fill ( LIFEBAR_BACKGROUNDCOLOR, &rcBackground );

            // Compute the rectangle of the life bar
            CRect rcLifeBar;
            rcLifeBar.left = rcBackground.left + 1;
            rcLifeBar.top = rcBackground.top + 1;
            rcLifeBar.bottom = rcBackground.bottom - 1;
            rcLifeBar.right = rcLifeBar.left +
              ( ( ( rcBackground.Width () - 2 ) * (int)pNode->m_dwLives ) /
              (int)pNode->m_dwMaxLives );

            // Compute the color of the life bar
            int nRed, nGreen, nBlue;
            if ( pNode->m_dwLives >= ( pNode->m_dwMaxLives >> 1 ) )
            {
              int nVal = (int)(pNode->m_dwLives - ( pNode->m_dwMaxLives >> 1 ));
              int nMax = (int)(pNode->m_dwMaxLives >> 1);
              nRed = LIFEBAR_BARCOLOR2_RED_ABS + ( ( LIFEBAR_BARCOLOR2_RED_MULT * nVal ) / nMax );
              nGreen = LIFEBAR_BARCOLOR2_GREEN_ABS + ( ( LIFEBAR_BARCOLOR2_GREEN_MULT * nVal ) / nMax );
              nBlue = LIFEBAR_BARCOLOR2_BLUE_ABS + ( ( LIFEBAR_BARCOLOR2_BLUE_MULT * nVal ) / nMax );
            }
            else
            {
              int nVal = (int)(pNode->m_dwLives);
              int nMax = (int)(pNode->m_dwMaxLives >> 1);
              nRed = LIFEBAR_BARCOLOR1_RED_ABS + ( ( LIFEBAR_BARCOLOR1_RED_MULT * nVal ) / nMax );
              nGreen = LIFEBAR_BARCOLOR1_GREEN_ABS + ( ( LIFEBAR_BARCOLOR1_GREEN_MULT * nVal ) / nMax );
              nBlue = LIFEBAR_BARCOLOR1_BLUE_ABS + ( ( LIFEBAR_BARCOLOR1_BLUE_MULT * nVal ) / nMax );
            }
            DWORD dwColor = RGB32 ( nRed, nGreen, nBlue );

            // Draw the life bar
            m_FrameBuffer.Fill ( dwColor, &rcLifeBar );
          }
        }

        // unlock the animation
        pNode->m_pAnimation->Unlock();
      }

      // jump to the next unit
      pNode = pNode->m_pNext;
    }

    // Draw any overlay
    DrawOverlay ( &m_FrameBuffer, rcUpdateRect );

    // restore the previous clipping
    m_FrameBuffer.SetClipRect(&rcOldClip, NULL);
  }
}


// adds viewport to the list of all viewports
void CViewport::AddViewport()
{
  VERIFY(m_lockViewportList.Lock());

  if(m_pViewportList == NULL){
    // if the list was empty -> start the slowdown timer
    ASSERT_VALID(g_pTimer);
    g_pTimer->Connect(&m_ViewportTimerObserver, 1000 / MAX_FRAME_RATE, CViewportTimerObserver::ID_SlowdownTimer);
  }

  this->m_pNextViewport = m_pViewportList;
  m_pViewportList = this;

  VERIFY(m_lockViewportList.Unlock());
}

// removes viewport from the list
void CViewport::RemoveViewport()
{
  VERIFY(m_lockViewportList.Lock());

  CViewport *pPrev, *pCur;

  pCur = m_pViewportList; pPrev = NULL;
  while(pCur != NULL){
    if(pCur == this) break;
    pPrev = pCur;
    pCur = pCur->m_pNextViewport;
  }
  if(pPrev == NULL){
    m_pViewportList = this->m_pNextViewport;
    this->m_pNextViewport = NULL;
  }
  else{
    pPrev->m_pNextViewport = this->m_pNextViewport;
    this->m_pNextViewport = NULL;
  }

  if(m_pViewportList == NULL){
    // if we were the last viewport -> stop the slowdown timer
    g_pTimer->Disconnect(&m_ViewportTimerObserver);
  }

  VERIFY(m_lockViewportList.Unlock());
}

// do the frame loop
void CViewport::DoFrameLoop()
{
  CViewport *pViewport;

  VERIFY(m_lockViewportList.Lock());
  if(m_pViewportList == NULL) return;
  // wait for next frame
  m_ViewportTimerObserver.WaitForNextFrame();

  pViewport = m_pViewportList;
  while(pViewport != NULL){
    pViewport->ComputeFrame();
    pViewport = pViewport->m_pNextViewport;
  }
  VERIFY(m_lockViewportList.Unlock());
}

// reaction on slowdown timer event
void CViewport::CViewportTimerObserver::OnTimeTick(DWORD dwTime)
{
  // just set the next frame event
  m_eventNextFrame.SetEvent();
}

// wait for next frame
void CViewport::CViewportTimerObserver::WaitForNextFrame()
{
  // wait for the event
#ifdef _DEBUG
//  m_eventNextFrame.Lock(1000 / MAX_FRAME_RATE);
#endif
}

// Adds new viewport position observer
void CViewport::AddPositionObserver(CViewportPositionObserver *pObserver)
{
  ASSERT(pObserver != NULL);

  // lock the viewport for writing
  VERIFY(m_lockViewport.WriterLock());

  // Add the new position observer to the list
  pObserver->m_pNext = m_pPositionObservers;
  m_pPositionObservers = pObserver;

  m_lockViewport.WriterUnlock();
}

// Removes the viewport position observer
void CViewport::RemovePositionObserver(CViewportPositionObserver *pObserver)
{
  // Lock the viewport for writing
  VERIFY(m_lockViewport.WriterLock());

  // Find and remove the position from the list
  CViewportPositionObserver *pObs, **pPrev;
  pPrev = &m_pPositionObservers;
  pObs = m_pPositionObservers;
  while(pObs != NULL){
    if(pObs == pObserver) break;
    pPrev = &pObs->m_pNext;
    pObs = pObs->m_pNext;
  }
  if(*pPrev == pObserver){
    *pPrev = pObserver->m_pNext;
  }

  m_lockViewport.WriterUnlock();
}

// Resizes the unit node to include all marks
void CViewport::AdjustNodeSize( SUnitNode *pNode )
{
  // Selection mark on the unit
  if(pNode->m_dwMarks & UnitMark_Selected){
    CCUnitAppearance *pSelectionAppearance = pNode->m_pUnit->GetUnitType()->GetSelectionMarkAppearance();
    CCUnitAnimation *pSelectionAnimation = pSelectionAppearance->GetDirection(pNode->m_pUnit->GetCurrentDirection());

    // Wait for the selection mark to load
    pSelectionAnimation->LoadAndLock();

    CCUnitSurface *pFrame = pSelectionAnimation->GetFrame(0);

    // compute the pixel position
    CRect rcSelection;
    rcSelection.left = (int)(pNode->m_dwX) - pSelectionAnimation->GetXGraphicalOffset() - pFrame->GetXOffset ();
    rcSelection.top = (int)(pNode->m_dwY) - pSelectionAnimation->GetYGraphicalOffset() - pFrame->GetYOffset ();
    rcSelection.right = rcSelection.left + pFrame->GetAllRect()->Width();
    rcSelection.bottom = rcSelection.top + pFrame->GetAllRect()->Height();

    // Unlock the selection animation
    pSelectionAnimation->Unlock();

    // remeber the topleft of the selection mark
    pNode->m_ptSelectionMarkTopLeft = rcSelection.TopLeft();

    // union rect of the unit and of the selection
    pNode->m_rcPosition.UnionRect(&(pNode->m_rcPosition), &rcSelection);
  }

  // Life bar mark on the unit
  if ( pNode->m_dwMarks & UnitMark_LifeBar )
  {
    DWORD dwMoveWidthInPixels;

    // Get the move width of the unit
    dwMoveWidthInPixels = pNode->m_pUnit->GetUnitType ()->GetGeneralUnitType ()
      ->GetMoveWidth ( pNode->m_pUnit->GetMode () ) * MAPCELL_WIDTH;

    // Now compute the rect of the life bar
    pNode->m_rcLifeBar.left = pNode->m_dwX - ( dwMoveWidthInPixels / 2 );
    pNode->m_rcLifeBar.right = pNode->m_rcLifeBar.left + dwMoveWidthInPixels;
    pNode->m_rcLifeBar.top = pNode->m_dwY + ( dwMoveWidthInPixels / 2 );
    pNode->m_rcLifeBar.bottom = pNode->m_rcLifeBar.top + LIFEBAR_HEIGHT;

    // union the rect of the unit and the life bar
    pNode->m_rcPosition.UnionRect ( &( pNode->m_rcPosition ), & ( pNode->m_rcLifeBar ) );
  }
}
