// Observer.cpp: implementation of the CObserver class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Observer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CObserver, CObject);

CObserver::CObserver()
{

}

CObserver::~CObserver()
{

}

////////////////////////////////////////////////////////////////////
// Debug methods

#ifdef _DEBUG

void CObserver::AssertValid() const
{
  CPersistentObject::AssertValid();
}

void CObserver::Dump(CDumpContext &dc) const
{
  CPersistentObject::Dump(dc);
}

#endif


// return TRUE if you process the event
// elseway return FALSE
BOOL CObserver::Perform(DWORD NotID, DWORD Event, DWORD Param)
{
  return FALSE;
}
