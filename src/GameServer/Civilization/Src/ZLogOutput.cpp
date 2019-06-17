// ZLogOutput.cpp: implementation of the CZLogOutput class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ZLogOutput.h"

#include "Common\ServerClient\LogOutput.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CZLogOutput, CObserver)

BEGIN_OBSERVER_MAP(CZLogOutput, CObserver)
  BEGIN_NOTIFIER(ID_VirtualConnection)
    EVENT(E_PACKETAVAILABLE)
      OnPacketAvailable(); return FALSE;
    EVENT(E_VIRTUALCONNECTIONCLOSED)
      OnVirtualConnectionClosed(); return FALSE;
    EVENT(E_NETWORKERROR)
      OnNetworkError(); return FALSE;
  END_NOTIFIER()
END_OBSERVER_MAP(CZLogOutput, CObserver)

CZLogOutput::CZLogOutput()
{
  m_pLogFile = NULL;
}

CZLogOutput::~CZLogOutput()
{
  ASSERT(m_pLogFile == NULL);
}

#ifdef _DEBUG

void CZLogOutput::AssertValid() const
{
  CObserver::AssertValid();
  ASSERT(m_pLogFile != NULL);
}

void CZLogOutput::Dump(CDumpContext &dc) const
{
  CObserver::Dump(dc);
}

#endif


void CZLogOutput::Create(CVirtualConnection VirtualConnection, CLogFile *pLogFile)
{
  ASSERT(pLogFile != NULL);

  m_VirtualConnection = VirtualConnection;
  m_VirtualConnection.SetVirtualConnectionObserver(this, ID_VirtualConnection);

  m_pLogFile = pLogFile;
  m_pLogFile->List(m_VirtualConnection, TRUE);
}

void CZLogOutput::Delete()
{
  if(m_pLogFile != NULL){
    m_pLogFile->StopAutoUpdate(m_VirtualConnection);
    m_pLogFile = NULL;
  }
  m_VirtualConnection.DeleteVirtualConnection();
}


void CZLogOutput::OnPacketAvailable()
{
  DWORD dwCommand;
  DWORD dwSize;

  VERIFY(m_VirtualConnection.BeginReceiveCompoundBlock());
  dwSize = sizeof(dwCommand);
  VERIFY(m_VirtualConnection.ReceiveBlock(&dwCommand, dwSize));
  ASSERT(dwSize == sizeof(dwCommand));

  switch(dwCommand){
  case LogOutput_Command_Clear:
    // no more data
    // just clean the log
    m_pLogFile->Clear();
    break;
  }

  VERIFY(m_VirtualConnection.EndReceiveCompoundBlock());
}

void CZLogOutput::OnNetworkError()
{
  Delete();
}

void CZLogOutput::OnVirtualConnectionClosed()
{
  VERIFY(m_VirtualConnection.DeleteVirtualConnection());
  Delete();
}