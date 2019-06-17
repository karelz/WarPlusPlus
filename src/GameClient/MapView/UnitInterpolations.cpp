// UnitInterpolations.cpp: implementation of the CUnitInterpolations class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UnitInterpolations.h"

#include "..\DataObjects\CMap.h"
#include "..\DataObjects\UnitCache.h"

#include "..\GameClientGlobal.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define INTERPOLATION_TIME_SLICE 50

IMPLEMENT_DYNAMIC(CUnitInterpolations, CObserver)

BEGIN_OBSERVER_MAP(CUnitInterpolations, CObserver)
  BEGIN_TIMER()
    ON_TIMETICK()
  END_TIMER()
END_OBSERVER_MAP(CUnitInterpolations, CObserver)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUnitInterpolations::CUnitInterpolations()
{
  m_pMap = NULL;
  m_pUnitCache = NULL;
  m_pInterpolationsThread = NULL;
}

CUnitInterpolations::~CUnitInterpolations()
{
  ASSERT(m_pMap == NULL);
  ASSERT(m_pUnitCache == NULL);
  ASSERT(m_pInterpolationsThread == NULL);
}

#ifdef _DEBUG

void CUnitInterpolations::AssertValid() const
{
  CObserver::AssertValid();
  ASSERT(m_pMap != NULL);
  ASSERT(m_pUnitCache != NULL);
  ASSERT(m_pInterpolationsThread != NULL);
}

void CUnitInterpolations::Dump(CDumpContext &dc) const
{
  CObserver::Dump(dc);
}

#endif

void CUnitInterpolations::Create(CCMap *pMap, CUnitCache *pUnitCache)
{
  ASSERT(pMap != NULL);
  ASSERT(pUnitCache != NULL);

  // copy pointers
  m_pMap = pMap;
  m_pUnitCache = pUnitCache;

  // just start the interpolations thread
  m_pInterpolationsThread = AfxBeginThread(_InterpolationsThread, (LPVOID)this);
  m_pInterpolationsThread->m_bAutoDelete = FALSE;
}

void CUnitInterpolations::Delete()
{
  // stop the interpolations thread
  if(m_pInterpolationsThread != NULL){
    m_EventManager.InlayQuitEvent(0);
    if(WaitForSingleObject((HANDLE)(*m_pInterpolationsThread), 5000) != WAIT_OBJECT_0){
      TerminateThread((HANDLE)(*m_pInterpolationsThread), -1);
    }
    delete m_pInterpolationsThread;
    m_pInterpolationsThread = NULL;
  }

  // just clear the pointers
  m_pMap = NULL;
  m_pUnitCache = NULL;
}


// The system procedure for the interpolations thread
UINT CUnitInterpolations::_InterpolationsThread(LPVOID pParam)
{
  CUnitInterpolations *pThis = (CUnitInterpolations *) pParam;

  pThis->InterpolationsThread();

  return 0;
}

// The interpolations thread itself
void CUnitInterpolations::InterpolationsThread()
{
  // create event manager for us
  m_EventManager.Create();

  // connect us to the timer
  ASSERT(g_pTimer != NULL);
  m_dwLastTickTime = g_pTimer->GetTime();
  g_pTimer->Connect(this, INTERPOLATION_TIME_SLICE);

  // do the event loop
  m_EventManager.DoEventLoop();

  g_pTimer->Disconnect(this);

  // Delete the event manager
  m_EventManager.Delete();
}

DWORD g_dwInterpolationTime;
// The time tick routine
void CUnitInterpolations::OnTimeTick(DWORD dwTime)
{
  ASSERT_VALID(this);

  // Length of the last tick in animseconds
  double dbTickLength = ((double)GAMECLIENT_TIMESLICE_LENGTH * ((double)dwTime - (double)m_dwLastTickTime)) /
    (double)(m_pMap->GetTimesliceLength());
  m_dwLastTickTime = dwTime;
  g_dwInterpolationTime += (DWORD)dbTickLength;


  if ( dbTickLength == 0 ) return;

  // Go throught all units in the unit cache
  CCUnit *pUnit = m_pUnitCache->OpenUnits(), *pPrevUnit;
  while(pUnit != NULL){
    // Add reference to the unit
    pUnit->AddRef();

    // Lock the unit for writing (many times we wan't need writer lock,
    // but you can't lock it for writing if you have it locked for reading)
    VERIFY(pUnit->GetLock()->WriterLock());

      // If the unit is not on the map -> some kind of Zombie - ignore it
	  if ( pUnit->GetParentMapSquare () == NULL )
	  {
	    goto NextUnit;
	  }

    // here for each unit do...

    // Change random direction if any
    if(pUnit->HasRandomDirection()){
      pUnit->DecreaseNextDirectionChangeRemainingTime((DWORD)dbTickLength);
    }
    
    // Compute animations
    // If the remaining time is 0 -> no animation
    if(pUnit->GetNextFrameRemainingTime() != 0){
      // Yes it's an animation

      // Decrease the animation time
      if(pUnit->DecreaseNextFrameRemainingTime((DWORD)dbTickLength)){
        // Reached next frame -> step to it
        pUnit->SetNextFrame();
      }
    }

    // do the move interpolation
    if(pUnit->IsMoving()){
      pUnit->MoveUnit(dbTickLength, m_pMap);
    }

NextUnit:
	;
    // step to the next unit in the cache
    pPrevUnit = pUnit;
    pUnit = m_pUnitCache->GetNextUnit(pUnit);

    pPrevUnit->GetLock()->WriterUnlock();
    pPrevUnit->Release();
  }
  // unlock unit cache (Close the list)
  m_pUnitCache->CloseUnits();
}