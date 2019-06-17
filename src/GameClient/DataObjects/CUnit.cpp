#include "stdafx.h"
#include "CUnit.h"
#include "CMap.h"

#include "Common\Map\Map.h"
#include "UnitCache.h"
#include "..\GameClientGlobal.h"

#define WAIT_FOR_CACHE_DELAY 50
#define RANDOMDIRECTIONCHANGE_VARIATION 4000
#define RANDOMDIRECTIONCHANGE_MEAN 10000

//#define _BRUTAL_ASSERTS // BRUTAAAAL

#ifdef _BRUTAL_ASSERTS
#define BRUTAL_ASSERT(f) ASSERT(f)
#else
#define BRUTAL_ASSERT(f) ((void)0)
#endif

#define BRUTAL_ASSERT_PTR(f) BRUTAL_ASSERT(((DWORD)f != 0x0cccccccc) && ((DWORD)f != 0x0cdcdcdcd) && ((DWORD)f != 0x0dddddddd) && ((DWORD)f != 0x0fdfdfdfd) && ((DWORD)f != 0x0bfbfbfbf) && ((DWORD)f != 0x0ffffffff))

IMPLEMENT_DYNAMIC(CCUnit, CNotifier);

CCUnit::CCUnit()
{
  m_pUnitType = NULL;
  m_pUnitCacheNext = NULL;
  m_pUnitCachePrev = NULL;

  m_pUnitCache = NULL;

  m_dwID = 0x0ffffffff;
  m_nMode = 0;

  m_dwLives = 0;
  m_dwMaxLives = 0;
  m_dwViewRadius = 0;

  m_pLastGoodAnimation = NULL;
  m_pCurrentAnimation = NULL;
  m_pCurrentAppearance = NULL;
  m_nCurrentDirection = 0;
  m_nWantedDirection = 0;
  m_dwCurrentFrame = 0;
  m_dwNextFrameRemainingTime = 0;

  m_dwNextDirectionChangeRemainingTime = 0;
  m_dwNextFrameRemainingTime = 0;

  m_dwTimeStampBrief = 0;
  m_dwTimeStampCheckPoint = 0;
  m_dwTimeStampFull = 0;

  m_pParentMapSquare = NULL;
  m_pMapSquareNext = NULL;
  m_pMapSquarePrev = NULL;

  m_dwXPixelPos = 0x0ffffffff;
  m_dwYPixelPos = 0x0ffffffff;
  m_dbXSpeed = 0;
  m_dbYSpeed = 0;
  m_dbXPosition = m_dwXPixelPos;
  m_dbYPosition = m_dwYPixelPos;
  m_rcPosition.SetRect(0x0ffffffff, 0x0ffffffff, 0x0ffffffff, 0x0ffffffff);

  m_dwXPos = 0x0ffffffff;
  m_dwYPos = 0x0ffffffff;
  m_dwZPos = 0x0ffffffff;

  m_CurrentCheckPoint.m_dwX = m_dwXPos;
  m_CurrentCheckPoint.m_dwY = m_dwYPos;
  m_CurrentCheckPoint.m_dwTime = 0;

  m_NextCheckPoint.m_dwX = m_dwXPos;
  m_NextCheckPoint.m_dwY = m_dwYPos;
  m_NextCheckPoint.m_dwTime = 0;

  m_bFullInfoFlag = FALSE;
  m_dwFullInfoLength = 0;
  m_pFullInfo = NULL;

  m_pViewportNext = NULL;

  m_dwReferenceCount = 0;
}

CCUnit::~CCUnit()
{
  ASSERT(m_pUnitType == NULL);
  ASSERT(m_pUnitCache == NULL);
}

#ifdef _DEBUG

void CCUnit::AssertValid() const
{
  CNotifier::AssertValid();

  ASSERT(m_pUnitType != NULL);
  ASSERT(m_pUnitCache != NULL);
  ASSERT ( m_nMode < 8 );
  BRUTAL_ASSERT(m_dwCurrentFrame < 1000);
  BRUTAL_ASSERT(m_dwFullInfoLength < 10000);
  BRUTAL_ASSERT(m_dwID < 10000);
  BRUTAL_ASSERT(m_dwLives < 1000);
  BRUTAL_ASSERT(m_dwMaxLives < 1000);
  BRUTAL_ASSERT(m_dwReferenceCount < 100);
  BRUTAL_ASSERT(m_dwTimeStampBrief < 1000000);
  BRUTAL_ASSERT(m_dwTimeStampCheckPoint < 1000000);
  BRUTAL_ASSERT(m_dwTimeStampFull < 1000000);
  BRUTAL_ASSERT(m_dwViewRadius < 1000);
  BRUTAL_ASSERT(m_nWantedDirection < 9);
  BRUTAL_ASSERT_PTR(m_pCurrentAnimation);
  BRUTAL_ASSERT_PTR(m_pCurrentAppearance);
  BRUTAL_ASSERT_PTR(m_pFullInfo);
  BRUTAL_ASSERT_PTR(m_pLastGoodAnimation);
  BRUTAL_ASSERT_PTR(m_pMapSquareNext);
  BRUTAL_ASSERT_PTR(m_pMapSquarePrev);
  BRUTAL_ASSERT_PTR(m_pParentMapSquare);
  BRUTAL_ASSERT_PTR(m_pUnitCache);
  BRUTAL_ASSERT_PTR(m_pUnitCacheNext);
  BRUTAL_ASSERT_PTR(m_pUnitCachePrev);
  BRUTAL_ASSERT_PTR(m_pViewportNext);
}

void CCUnit::Dump(CDumpContext &dc) const
{
  CNotifier::Dump(dc);
}

#endif



void CCUnit::Create(CCUnitType *pUnitType, CUnitCache *pUnitCache)
{
  ASSERT_VALID(pUnitType);
  ASSERT_VALID(pUnitCache);

  CNotifier::Create ();

  m_pUnitType = pUnitType;
  // Lock the unit type in the cache (it must remain there while this unit exists)
  m_pUnitType->LoadAndLock();

  m_pUnitCache = pUnitCache;
  m_nCurrentDirection = 0;
}

void CCUnit::Delete()
{
  // Release the unit type from the cache
  m_pUnitType->Unlock();

  m_pUnitType = NULL;
  m_pUnitCache = NULL;

  // Delete full info
  if ( m_pFullInfo != NULL )
  {
	  delete m_pFullInfo;
	  m_pFullInfo = NULL;
  }

  CNotifier::Delete ();
}

void CCUnit::AddRef()
{  
  InterlockedIncrement((LPLONG)&m_dwReferenceCount);
}

void CCUnit::Release()
{
  ASSERT_VALID(this);
  DWORD dwReferenceCount = InterlockedDecrement((LPLONG)&m_dwReferenceCount);
  if(dwReferenceCount == 0){
    // and remove us from the cache (this will call delete on us)
    m_pUnitCache->RemoveUnit(this);
    return;
  }
}

void CCUnit::UpdateMapSquare(CCMap *pMap)
{
  ASSERT_VALID(this);
  CCMapSquare *pMS;

  VERIFY(pMap->GetGlobalLock()->WriterLock());

  if ( m_pParentMapSquare == NULL )
  {
    pMS = pMap->GetMapSquare ( m_dwXPos / MAPSQUARE_WIDTH, m_dwYPos / MAPSQUARE_HEIGHT, TRUE );
	m_pParentMapSquare = pMS;
    pMS->AppendUnit ( this );
  }
  else
  {
    pMS = pMap->GetMapSquare ( m_dwXPos / MAPSQUARE_WIDTH, m_dwYPos / MAPSQUARE_HEIGHT, TRUE );
  }

  if ( m_pParentMapSquare != pMS )
  {
    AddRef ();
    m_pParentMapSquare->RemoveUnit ( this );
    pMS->AppendUnit ( this );
    Release ();
  }

  pMap->GetGlobalLock()->WriterUnlock();
}

void CCUnit::RemoveFromMap(CCMap *pMap)
{
  ASSERT_VALID(this);
  VERIFY(pMap->GetGlobalLock()->WriterLock());

  if(m_pParentMapSquare != NULL){
    m_pParentMapSquare->RemoveUnit(this);
  }

  pMap->GetGlobalLock()->WriterUnlock();
}

void CCUnit::SetPosition(DWORD dwX, DWORD dwY, DWORD dwZ, CCMap *pMap)
{
  ASSERT_VALID(this);

  if ( (m_dwXPos != dwX) || (m_dwYPos != dwY) || (m_dwZPos != dwZ) )
	  InlayEvent ( E_PositionChanged, (DWORD)this );
  // set the position in mapcells
  m_dwXPos=dwX;
  m_dwYPos=dwY;
  m_dwZPos=dwZ;
  // place us to the correct mapsquare
  UpdateMapSquare(pMap);

  // compute the correct position of the unit's square in pixels
  m_dwXPixelPos = m_dwXPos * MAPCELL_WIDTH;
  m_dwYPixelPos = m_dwYPos * MAPCELL_HEIGHT;
  // also copy the position to our interpolation counters
  m_dbXPosition = m_dwXPixelPos;
  m_dbYPosition = m_dwYPixelPos;

  if(m_CurrentCheckPoint.m_dwX == 0x0ffffffff){
    // This is the first time we set the position, so set the check points
    // in such a way, that the unit will stand on the place it's standing
    m_CurrentCheckPoint.m_dwX = m_dwXPos;
    m_CurrentCheckPoint.m_dwY = m_dwYPos;
    m_CurrentCheckPoint.m_dwTime = 0;
    
    m_NextCheckPoint.m_dwX = m_dwXPos;
    m_NextCheckPoint.m_dwY = m_dwYPos;
    m_NextCheckPoint.m_dwTime = 0;
  }

  // recompute interpolations
  RecomputeInterpolations(pMap->GetCurrentTimeslice());

  // recompute the position of the graphics on the map
  RecomputePixelPosition();
}

void CCUnit::SetCheckPoints(SCCheckPoint *pFirstCheckPoint, SCCheckPoint *pSecondCheckPoint, CCMap *pMap)
{
  ASSERT_VALID(this);
  
  m_CurrentCheckPoint.m_dwX = pFirstCheckPoint->m_dwX;
  m_CurrentCheckPoint.m_dwY = pFirstCheckPoint->m_dwY;
  m_CurrentCheckPoint.m_dwTime = pFirstCheckPoint->m_dwTime;

  m_NextCheckPoint.m_dwX = pSecondCheckPoint->m_dwX;
  m_NextCheckPoint.m_dwY = pSecondCheckPoint->m_dwY;
  m_NextCheckPoint.m_dwTime = pSecondCheckPoint->m_dwTime;

  // We'll use the last known timeslice, cause it surely the one that arrived
  // with the notification, that called us
  RecomputeInterpolations(pMap->GetLastKnownTimeslice());
}

void CCUnit::SetDirection(BYTE nDirection)
{
  ASSERT_VALID(this);
  ASSERT(nDirection >= 0);
  ASSERT(nDirection <= 8);

  m_nWantedDirection = nDirection;
  if(nDirection == 8){
    SetRandomDirection();
    return;
  }

  if(m_nCurrentDirection == nDirection) return;
  m_nCurrentDirection = nDirection;

  m_pCurrentAnimation = m_pCurrentAppearance->GetDirection(m_nCurrentDirection);

  RestartAnimation();

  RecomputePixelPosition();
}

// Sets new random direction and also sets new direction change timeout
void CCUnit::SetRandomDirection()
{
  ASSERT_VALID(this);
  // first change the direction
  int nRandomDirection = ((rand() * 2) / (RAND_MAX - 1));
  if(nRandomDirection == 0){
    if(m_nCurrentDirection == 0) m_nCurrentDirection = 7;
    else
      m_nCurrentDirection = (m_nCurrentDirection - 1) % 8;
  }
  else{
    m_nCurrentDirection = (m_nCurrentDirection + 1) % 8;
  }

  m_pCurrentAnimation = m_pCurrentAppearance->GetDirection(m_nCurrentDirection);

  RestartAnimation();

  RecomputePixelPosition();

  // Then set new random direction change timeout
  m_dwNextDirectionChangeRemainingTime = (rand() * RANDOMDIRECTIONCHANGE_VARIATION) / RAND_MAX
    + RANDOMDIRECTIONCHANGE_MEAN;
}

void CCUnit::SetAppearance ( DWORD dwAppearance, BYTE nDirection )
{
  ASSERT_VALID(this);
  // get the appearance object from our unit type
  CCUnitAppearance *pApp = m_pUnitType->GetAppearance(dwAppearance);
  if ( (pApp == m_pCurrentAppearance) && (nDirection == m_nCurrentDirection)){
    RestartAnimation();
    return;
  }
  m_pCurrentAppearance = pApp;

  // Use the new direction
  if ( nDirection == 8 )
  {
    int nRandomDirection = ((rand() * 2) / (RAND_MAX - 1));
    if(nRandomDirection == 0){
      if(m_nCurrentDirection == 0) m_nCurrentDirection = 7;
      else m_nCurrentDirection = (m_nCurrentDirection - 1) % 8;
	}
    else{
      m_nCurrentDirection = (m_nCurrentDirection + 1) % 8;
	}
  }
  else
  {
    m_nCurrentDirection = nDirection;
  }
  m_pCurrentAnimation = m_pCurrentAppearance->GetDirection(m_nCurrentDirection);

  RestartAnimation();

  RecomputePixelPosition();
}

void CCUnit::RecomputePixelPosition(BOOL bIsLoaded)
{
  ASSERT_VALID(this);
  CCUnitAnimation *pAnim;
  DWORD dwFrame;
  BOOL bUnlock = FALSE;

  if(!bIsLoaded){
    // try to lock the appearance
    if((m_pCurrentAnimation == NULL) || (!m_pCurrentAnimation->TestLock())){
      // can't lock it -> use the last good
      if((m_pLastGoodAnimation == NULL) ||
        (!m_pLastGoodAnimation->TestLock())){
        // Ooops, we can't lock even this one -> nothing
        return;
      }
      if(!m_pLastGoodAnimation->IsLoaded()){
        m_pLastGoodAnimation->Unlock();
        return;
      }
      
      pAnim = m_pLastGoodAnimation;
      dwFrame = 0;
    }
    else{
      if(!m_pCurrentAnimation->IsLoaded()){
        m_pCurrentAnimation->Unlock();
        m_pCurrentAnimation->HintDataNeeded();
        
        if((m_pLastGoodAnimation == NULL) ||
          (!m_pLastGoodAnimation->TestLock())){
          // No one's in the cache -> do nothing
          return;
        }
        if(!m_pLastGoodAnimation->IsLoaded()){
          m_pLastGoodAnimation->Unlock();
          return;
        }
        
        pAnim = m_pLastGoodAnimation;
        dwFrame = 0;
      }
      else{
        m_pLastGoodAnimation = m_pCurrentAnimation;
        pAnim = m_pCurrentAnimation;
        dwFrame = m_dwCurrentFrame;
      }
    }
  }
  else{
    m_pLastGoodAnimation = m_pCurrentAnimation;
    pAnim = m_pCurrentAnimation;
    dwFrame = m_dwCurrentFrame;
  }

  // get current frame
  CCUnitSurface *pFrame = pAnim->GetFrame(dwFrame);

  // compute the pixel position
  m_rcPosition.left = m_dwXPixelPos - pAnim->GetXGraphicalOffset() - pFrame->GetXOffset ();
  m_rcPosition.top = m_dwYPixelPos - pAnim->GetYGraphicalOffset() - pFrame->GetYOffset ();
  m_rcPosition.right = m_rcPosition.left + pFrame->GetAllRect()->Width();
  m_rcPosition.bottom = m_rcPosition.top + pFrame->GetAllRect()->Height();

  // unlock the animation
  if(!bIsLoaded)
    pAnim->Unlock();
}

void CCUnit::RestartAnimation()
{
  ASSERT_VALID(this);
  // set the first frame
  m_dwCurrentFrame = 0;

  // Try to lock current animation
  if(!m_pCurrentAnimation->TestLock()){
    // if no succes -> set some timeout
    m_dwNextFrameRemainingTime = WAIT_FOR_CACHE_DELAY;
    return;
  }
  else{
    if(!m_pCurrentAnimation->IsLoaded()){
      m_pCurrentAnimation->Unlock();
      m_pCurrentAnimation->HintDataNeeded();
      m_dwNextFrameRemainingTime = WAIT_FOR_CACHE_DELAY;
      return;
    }
  }

  // Set the last good to this one
  m_pLastGoodAnimation = m_pCurrentAnimation;

  // next frame will arrive after "speed" milliseconds
  m_dwNextFrameRemainingTime = m_pCurrentAnimation->GetSpeed();

  // and recompute position
  RecomputePixelPosition(TRUE);

  m_pCurrentAnimation->Unlock();
}

void CCUnit::SetNextFrame()
{
  ASSERT_VALID(this);
  // try to lock current animation
  if((m_pCurrentAnimation == NULL) || (!m_pCurrentAnimation->TestLock())){
    // if no succes -> set some timeout
    m_dwNextFrameRemainingTime = WAIT_FOR_CACHE_DELAY;
    return;
  }
  if(!m_pCurrentAnimation->IsLoaded()){
    m_pCurrentAnimation->Unlock();
    m_pCurrentAnimation->HintDataNeeded();
    // if no succes -> set some timeout
    m_dwNextFrameRemainingTime = WAIT_FOR_CACHE_DELAY;
    return;
  }
  
  // move to the next frame
  // Only if the animation was loaded previously
  if ( m_pCurrentAnimation == m_pLastGoodAnimation )
	m_dwCurrentFrame++;
  else
    m_pLastGoodAnimation = m_pCurrentAnimation;

  if(m_dwCurrentFrame >= m_pCurrentAnimation->GetFramesCount()){
    // if we're at the end of animation
    if(m_pCurrentAnimation->GetLoop()){
      // If we're about to loop -> return back to the start of the animation
      m_dwCurrentFrame = 0;
    }
    else{
      // No looping -> stay at the last frame
      m_dwCurrentFrame--;
      m_dwNextFrameRemainingTime = 0;

      RecomputePixelPosition(TRUE);
      m_pCurrentAnimation->Unlock();
      return;
    }
  }
  // next frame will arrive after "speed" milliseconds
  CCUnitSurface * pFrame = m_pCurrentAnimation->GetFrame ( m_dwCurrentFrame );
  if ( pFrame->GetFrameDelay () == 0 )
    m_dwNextFrameRemainingTime = m_pCurrentAnimation->GetSpeed();
  else
    m_dwNextFrameRemainingTime = pFrame->GetFrameDelay ();

  RecomputePixelPosition(TRUE);
  m_pCurrentAnimation->Unlock();
}

void CCUnit::RecomputeInterpolations(DWORD dwCurrentTimeslice)
{
  ASSERT_VALID(this);
  // Get the time difference
  // amount of time to next checkpoint
  double dbTimeDif = (m_CurrentCheckPoint.m_dwTime - dwCurrentTimeslice) * GAMECLIENT_TIMESLICE_LENGTH;
  // If we are about to be already there -> move there in one animsecond
  // This is protection from errors
  // But also this is needed for teleports
  if(dbTimeDif <= 0) dbTimeDif = 1;
  
  // compute the speeds (it's in pixels so be carefull)
  m_dbXSpeed = ((double)(m_CurrentCheckPoint.m_dwX * MAPCELL_WIDTH) - m_dbXPosition) / dbTimeDif;
  m_dbYSpeed = ((double)(m_CurrentCheckPoint.m_dwY * MAPCELL_HEIGHT) - m_dbYPosition) / dbTimeDif;
}

// Moves unit along its path by given amount of animseconds
void CCUnit::MoveUnit(double dbAmount, CCMap *pMap)
{
  ASSERT_VALID(this);
  // First we must compute the distance to our destination
  double dbXDist, dbYDist, dbXMove, dbYMove;

  dbXDist = (double)(m_CurrentCheckPoint.m_dwX * MAPCELL_WIDTH) - m_dbXPosition;
  dbYDist = (double)(m_CurrentCheckPoint.m_dwY * MAPCELL_HEIGHT) - m_dbYPosition;

  dbXMove = m_dbXSpeed * dbAmount;
  dbYMove = m_dbYSpeed * dbAmount;

  if ( ( dbXDist != 0 ) || ( dbYDist != 0 ) )
    InlayEvent ( E_PositionChanged, (DWORD)this );

  if(dbXDist == 0){
    if(dbYDist < 0){
      if(dbYMove <= dbYDist){
        // We would step farer then we're about -> just go to next checkpoint
        NextCheckPoint(pMap);
        return;
      }
    }
    else{
      if(dbYMove >= dbYDist){
        // We would step farer then we're about -> just go to next checkpoint
        NextCheckPoint(pMap);
        return;
      }
    }
  }
  else{
    if(dbXDist < 0){
      if((dbXMove <= dbXDist) || (dbXMove > 0)){
        // We would step farer then we're about -> just go to next checkpoint
        NextCheckPoint(pMap);
        return;
      }
    }
    else{
      if((dbXMove >= dbXDist) || (dbXMove < 0)){
        // We would step farer then we're about -> just go to next checkpoint
        NextCheckPoint(pMap);
        return;
      }
    }
  }

  // just another small move on the long path (no checkpoint reached)

  // do the step
  m_dbXPosition += dbXMove;
  m_dbYPosition += dbYMove;

  // recompute pixel position 
  m_dwXPixelPos = (DWORD)m_dbXPosition;
  m_dwYPixelPos = (DWORD)m_dbYPosition;

  // recompute mapcell position
  DWORD dwXPos, dwYPos, dwSXPos, dwSYPos;;
  dwXPos = m_dwXPixelPos / MAPCELL_WIDTH;
  dwYPos = m_dwYPixelPos / MAPCELL_HEIGHT;
  dwSXPos = dwXPos / MAPSQUARE_WIDTH;
  dwSYPos = dwYPos / MAPSQUARE_HEIGHT;

  // If the unit moved accross mapsquare boundaries -> change the mapsquare
  if((dwSXPos != (m_dwXPos / MAPSQUARE_WIDTH)) || (dwSYPos != (m_dwYPos / MAPSQUARE_HEIGHT))){
    m_dwXPos = dwXPos;
    m_dwYPos = dwYPos;

    UpdateMapSquare(pMap);
  }
  else{
    m_dwXPos = dwXPos;
    m_dwYPos = dwYPos;
  }

  // recompute the graphics position
  RecomputePixelPosition();
}

void CCUnit::NextCheckPoint(CCMap *pMap)
{
  ASSERT_VALID(this);
  // remeber the checkpoint position
  SCCheckPoint ReachedCheckPoint = m_CurrentCheckPoint;

  // copy the next checkpoint to the current one
  m_CurrentCheckPoint = m_NextCheckPoint;
  // Left the next checkpoint as it was (this will do that the unit will stay there)

  // set unit position to the current checkpoint
  SetPosition(ReachedCheckPoint.m_dwX, ReachedCheckPoint.m_dwY, m_dwZPos, pMap);
}