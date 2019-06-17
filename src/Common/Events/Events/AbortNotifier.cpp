// AbortNotifier.cpp: implementation of the CAbortNotifier class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AbortNotifier.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CAbortNotifier g_AbortNotifier;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CAbortNotifier, CNotifier)

CAbortNotifier::CAbortNotifier()
{

}

CAbortNotifier::~CAbortNotifier()
{

}

#ifdef _DEBUG

void CAbortNotifier::AssertValid() const
{
  CNotifier::AssertValid();
}

void CAbortNotifier::Dump(CDumpContext &dc) const
{
  CNotifier::Dump(dc);
}

#endif

void CAbortNotifier::Connect(CObserver *pObserver, DWORD dwNotID)
{
  CNotifier::Connect(pObserver, dwNotID);
}

// This function will abort whole application
// It won't exit the application
// it just close all graphics and sound and so on ...
void AbortApplication(DWORD dwExitCode)
{
  g_AbortNotifier.SendEvent(CAbortNotifier::E_ABORTEVENT, dwExitCode, NULL);
}