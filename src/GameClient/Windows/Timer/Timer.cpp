// Timer.cpp: implementation of the CTimer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Timer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CTimer *g_pTimer = NULL;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CTimer, CNotifier)

BEGIN_OBSERVER_MAP(CTimer, CNotifier)
  BEGIN_ABORT()
    ON_ABORT()
  END_ABORT()
END_OBSERVER_MAP(CTimer, CNotifier)


CTimer::CTimer()
{
  m_nTimerID = 0;
}

CTimer::~CTimer()
{

}


//////////////////////////////////////////////////////////////////////
// debug methods

#ifdef _DEBUG

void CTimer::AssertValid() const
{
  CNotifier::AssertValid();
  ASSERT(m_nTimerID != NULL);
}

void CTimer::Dump(CDumpContext &dc) const
{
  CNotifier::Dump(dc);
  dc << "Timer ID : " << m_nTimerID << "\n";
}

#endif

BOOL CTimer::Create(DWORD dwTimeSlice)
{
  if(!CNotifier::Create()) return FALSE;

  m_dwTimeSlice = dwTimeSlice;

  // assuming we are the only one timer in the system
  // we will write us to the global pointer

  ASSERT(g_pTimer == NULL);
  g_pTimer = this;

  m_dwTime = ::GetTickCount();

  // set the timer for our time slice
  m_nTimerID = ::SetTimer(NULL, TimerID, m_dwTimeSlice, CTimer::CallbackProc);
  if(m_nTimerID == 0){
    TRACE("CTimer::Create : Failed to create the timer. \n");
    return FALSE;
  }

  g_AbortNotifier.Connect(this);

  return TRUE;
}

void CTimer::Delete()
{
  g_AbortNotifier.Disconnect(this);

  CNotifier::Delete();
  if(m_nTimerID == 0) return; // no need to call the delete

  // kill the timer
  ::KillTimer(NULL, m_nTimerID);

  // we can clear the global value (we should be the only one timer)
  g_pTimer = NULL;

  CNotifier::Delete();
}


// the callback function for the timer
void CALLBACK CTimer::CallbackProc(HWND hWnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{
  if(g_pTimer == NULL) return;

  DWORD dwSlice;
  
  dwSlice = dwTime - g_pTimer->m_dwTime; // get the current slice
  g_pTimer->m_dwTime = dwTime; // update the time value

  // loop through all connections and increas the counters
  SObserverNode *pNode;
  STimerConnectionData *pConData;

  pNode = g_pTimer->CNotifier::OpenConnections();
  for(; pNode != NULL; pNode = g_pTimer->CNotifier::GetNextConnection(pNode)){
    // get the connection data
    pConData = (STimerConnectionData *)pNode->m_pConnectionData;
    if(pConData == NULL) continue;

    // if the infinite time -> no event
    if(pConData->m_dwElapseTime == 0) continue;

    // increase the elapsed time
    pConData->m_dwCurrentElapseTime += dwSlice;
    if(pConData->m_dwCurrentElapseTime > pConData->m_dwElapseTime){
      // we have reached the time value
      // so clear the counter and notify
      pConData->m_dwCurrentElapseTime = 0;

      // inlay the event
      g_pTimer->CNotifier::InlayEvent(CTimer::E_TICK, g_pTimer->m_dwTime, pNode->m_pObserver, CEventManager::Priority_Low, TRUE, TRUE);
    }
  }
  g_pTimer->CNotifier::CloseConnections();
}


BOOL CTimer::Init(DWORD dwTimeSlice)
{
  CTimer *pTimer;

  pTimer = new CTimer();
  return pTimer->Create(dwTimeSlice);
}

void CTimer::Close()
{
  CTimer *pTimer;

  pTimer = g_pTimer;
  if(pTimer == NULL) return;

  pTimer->Delete();
  delete pTimer;
}

void CTimer::Connect(CObserver * pObserver, DWORD dwElapseTime, DWORD dwNotifierID)
{
  CNotifier::Connect(pObserver, dwNotifierID);

  SObserverNode *pNode;
  STimerConnectionData *pConData;
  pNode = FindObserverNode(pObserver);

//  TRACE("Connecting to timer %s\n", pObserver->GetRuntimeClass()->m_lpszClassName);
//  pObserver->Dump(afxDump);

  // additional info about this connection
  pConData = new STimerConnectionData;
  pConData->m_dwElapseTime = dwElapseTime;
  pConData->m_dwCurrentElapseTime = 0;
  pNode->m_pConnectionData = pConData;
}

void CTimer::DeleteNode(SObserverNode * pNode)
{
  STimerConnectionData *pConData;

//  TRACE("Disconnection from timer %s\n", pNode->m_pObserver->GetRuntimeClass()->m_lpszClassName);
//  pNode->m_pObserver->Dump(afxDump);

  //remove and delete additional info about the connection
  pConData = (STimerConnectionData *)pNode->m_pConnectionData;
  pNode->m_pConnectionData = NULL;

  if(pConData != NULL)
    delete pConData;

  CNotifier::DeleteNode(pNode);
}

// sets new elapse time for connection
void CTimer::SetElapseTime(CObserver * pObserver, DWORD dwElapseTime)
{
  SObserverNode *pNode;
  STimerConnectionData *pConData;

  pNode = FindObserverNode(pObserver);
  pConData = (STimerConnectionData *)pNode->m_pConnectionData;
  pConData->m_dwElapseTime = dwElapseTime;
  pConData->m_dwCurrentElapseTime = 0;
}

void CTimer::OnAbort(DWORD dwExitCode)
{
  Delete();
}

double CTimer::GetPreciseTime()
{
  FILETIME t1, t2, t3, t4;

  GetThreadTimes(GetCurrentThread(), &t1, &t2, &t3, &t4);

  return (((double)t3.dwHighDateTime + (double)t4.dwHighDateTime) * (double)(0x0100000000)) +
    ((double)t3.dwLowDateTime + (double)t4.dwLowDateTime);
}
