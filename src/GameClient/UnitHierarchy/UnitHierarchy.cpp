// UnitHierarchy.cpp: implementation of the CUnitHierarchy class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UnitHierarchy.h"

#include "..\GameClientNetwork.h"

#include "Common\ServerClient\VirtualConnectionIDs.h"
#include "Common\ServerClient\UnitHierarchyConnection.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CUnitHierarchy, CNotifier);

BEGIN_OBSERVER_MAP(CUnitHierarchy, CNotifier)
  BEGIN_NOTIFIER(ID_Connection)
    EVENT(E_PACKETAVAILABLE)
      OnPacketAvailable(); return FALSE;
    EVENT(E_NETWORKERROR)
      OnNetworkError(); return FALSE;
  END_NOTIFIER()
END_OBSERVER_MAP(CUnitHierarchy, CNotifier)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUnitHierarchy::CUnitHierarchy()
{
  m_pGeneralCommander = NULL;
  m_bUpdating = FALSE;
}

CUnitHierarchy::~CUnitHierarchy()
{
  ASSERT(m_pGeneralCommander == NULL);
  ASSERT(m_bUpdating == FALSE);
}


#ifdef _DEBUG

void CUnitHierarchy::AssertValid() const
{
  CNotifier::AssertValid();
}

void CUnitHierarchy::Dump(CDumpContext &dc) const
{
  CNotifier::Dump(dc);
}

#endif

void CUnitHierarchy::Create()
{
  // Create our virtual connection
  DWORD dwUserData = VirtualConnection_UnitHierarchy;
  m_Connection = g_pNetworkConnection->CreateVirtualConnectionEx( PACKET_TYPE4, this, ID_Connection,
    g_dwNetworkEventThreadID, &dwUserData, sizeof(dwUserData));
}

void CUnitHierarchy::Delete()
{
  Lock();
  if(m_bUpdating) m_bUpdating = FALSE;

  // Delete the hierarchy of units
  Clear();

  // delete the virtual connection
  m_Connection.DeleteVirtualConnection();
}

void CUnitHierarchy::Clear()
{
  if(m_pGeneralCommander != NULL){
    m_pGeneralCommander->Delete();
    delete m_pGeneralCommander;
    m_pGeneralCommander = NULL;
  }
}

void CUnitHierarchy::OnNetworkError()
{
  // Just say that we're done
  Lock();
  Clear();
  m_bUpdating = FALSE;
  InlayEvent(E_HierarchyUpdated, 0xFFFFFFFF);
  Unlock();
}

void CUnitHierarchy::OnPacketAvailable()
{
  Lock();

  DWORD dwAnswer, dwSize;
  dwSize = sizeof(dwAnswer);

  VERIFY(m_Connection.BeginReceiveCompoundBlock());
  VERIFY(m_Connection.ReceiveBlock(&dwAnswer, dwSize));
  ASSERT(dwSize == sizeof(dwAnswer));

  switch(dwAnswer){
  case UnitHierarchyAnswer_GetWhole:
    {
      // Whole hierarchy arrived
      ASSERT(m_bUpdating);

      // So clear us
      Clear();

      // Receive the header
      SUnitHierarchyAnswer_GetWhole header;
      dwSize = sizeof(header);
      VERIFY(m_Connection.ReceiveBlock(&header, dwSize));
      ASSERT(dwSize == sizeof(header));

      if(header.m_dwUnitCount > 0){
        // Some unit in it -> load it
        m_pGeneralCommander = new CUnitHierarchyUnit();
        m_pGeneralCommander->Create(&m_Connection, NULL);
      }
      m_bUpdating = FALSE;

      InlayEvent(E_HierarchyUpdated, 0);
    }
    break;

  case UnitHierarchyAnswer_ChangeScript:
    {
      // Answer to our change script request
      ASSERT(m_bUpdating);

      // recieve the header
      SUnitHierarchyAnswer_ChangeScript header;
      dwSize = sizeof(header);
      VERIFY(m_Connection.ReceiveBlock(&header, dwSize));
      ASSERT(dwSize == sizeof(header));

      ASSERT(header.m_dwUnitCount > 0);
      
      DWORD dwUnit, dwErrorCount = 0;
      SUnitHierarchyAnswer_ChangeScript_UnitRecord record;
      CUnitHierarchyUnit *pUnit;
      for(dwUnit = 0; dwUnit < header.m_dwUnitCount; dwUnit++){
        dwSize = sizeof(record);
        VERIFY(m_Connection.ReceiveBlock(&record, dwSize));
        ASSERT(dwSize == sizeof(record));

        pUnit = FindZUnit(record.m_dwZUnitID);

        switch(record.m_dwResult){
        case 0:
          // success -> update data
          pUnit->m_strScriptName = m_strChangeScriptName;
          break;
        case 1:
          // some error -> no update
          dwErrorCount++;
          break;
        case 2:
          // unit died -> remove it
          DeleteUnit(pUnit);
          break;
        }
      }

      m_bUpdating = FALSE;

      // Notify window
      InlayEvent(E_ChangedScriptName, dwErrorCount);
    }
    break;

  case UnitHierarchyAnswer_ChangeName:
    {
      // Answer to our change script request
      ASSERT(m_bUpdating);

      // recieve the header
      SUnitHierarchyAnswer_ChangeName header;
      dwSize = sizeof(header);
      VERIFY(m_Connection.ReceiveBlock(&header, dwSize));
      ASSERT(dwSize == sizeof(header));

      ASSERT(header.m_dwUnitCount > 0);
      
      DWORD dwUnit, dwErrorCount = 0;
      SUnitHierarchyAnswer_ChangeScript_UnitRecord record;
      CUnitHierarchyUnit *pUnit;
      for(dwUnit = 0; dwUnit < header.m_dwUnitCount; dwUnit++){
        dwSize = sizeof(record);
        VERIFY(m_Connection.ReceiveBlock(&record, dwSize));
        ASSERT(dwSize == sizeof(record));

        pUnit = FindZUnit(record.m_dwZUnitID);

        switch(record.m_dwResult){
        case 0:
          // success -> update data
          pUnit->m_strName = m_strChangeName;
          break;
        case 1:
          // some error -> no update
          dwErrorCount++;
          break;
        case 2:
          // unit died -> remove it
          DeleteUnit(pUnit);
          break;
        }
      }

      m_bUpdating = FALSE;

      // Notify window
      InlayEvent(E_ChangedName, dwErrorCount);
    }
    break;

  default:
    TRACE("Ooops - UnitHierarchy unknown answer recieved.\n");
    ASSERT(FALSE);
    break;
  }

  VERIFY(m_Connection.EndReceiveCompoundBlock());

  Unlock();
}

BOOL CUnitHierarchy::Refresh()
{
  Lock();

  if(!m_Connection.IsNetworkReady()){
    Unlock();
    return FALSE;
  }
  // Allready in progress
  if(m_bUpdating){
    Unlock();
    return TRUE;
  }

  // Send a request to the server
  DWORD dwRequest = UnitHierarchyRequest_GetWhole;
  VERIFY(m_Connection.BeginSendCompoundBlock());
  VERIFY(m_Connection.SendBlock(&dwRequest, sizeof(dwRequest)));
  VERIFY(m_Connection.EndSendCompoundBlock());

  m_bUpdating = TRUE;

  Unlock();

  return TRUE;
}

BOOL CUnitHierarchy::ChangeName(CString strNewName, DWORD *pZUnitIDs, DWORD dwZUnitCount)
{
  ASSERT(m_bUpdating == FALSE);
  ASSERT(dwZUnitCount > 0);
  ASSERT(pZUnitIDs != NULL);

  if(!m_Connection.IsNetworkReady()) return FALSE;

  m_strChangeName = strNewName;

  DWORD dwRequest = UnitHierarchyRequest_ChangeName;
  VERIFY(m_Connection.BeginSendCompoundBlock());
  VERIFY(m_Connection.SendBlock(&dwRequest, sizeof(dwRequest)));

  SUnitHierarchyRequest_ChangeName header;
  header.m_dwUnitCount = dwZUnitCount;
  header.m_dwNameLength = strNewName.GetLength() + 1;
  VERIFY(m_Connection.SendBlock(&header, sizeof(header)));

  VERIFY(m_Connection.SendBlock((LPCSTR)strNewName, header.m_dwNameLength));
  VERIFY(m_Connection.SendBlock(pZUnitIDs, dwZUnitCount * sizeof(DWORD)));

  VERIFY(m_Connection.EndSendCompoundBlock());
  m_bUpdating = TRUE;

  return TRUE;
}

BOOL CUnitHierarchy::ChangeScript(CString strNewScript, DWORD *pZUnitIDs, DWORD dwZUnitCount)
{
  ASSERT(m_bUpdating == FALSE);
  ASSERT(dwZUnitCount > 0);
  ASSERT(pZUnitIDs != NULL);

  if(!m_Connection.IsNetworkReady()) return FALSE;

  m_strChangeScriptName = strNewScript;

  DWORD dwRequest = UnitHierarchyRequest_ChangeScript;
  VERIFY(m_Connection.BeginSendCompoundBlock());
  VERIFY(m_Connection.SendBlock(&dwRequest, sizeof(dwRequest)));

  SUnitHierarchyRequest_ChangeScript header;
  header.m_dwUnitCount = dwZUnitCount;
  header.m_dwScriptNameLength = strNewScript.GetLength() + 1;
  VERIFY(m_Connection.SendBlock(&header, sizeof(header)));

  VERIFY(m_Connection.SendBlock((LPCSTR)strNewScript, header.m_dwScriptNameLength));
  VERIFY(m_Connection.SendBlock(pZUnitIDs, dwZUnitCount * sizeof(DWORD)));

  VERIFY(m_Connection.EndSendCompoundBlock());
  m_bUpdating = TRUE;

  return TRUE;
}

CUnitHierarchyUnit * CUnitHierarchy::FindZUnit(DWORD dwZUnitID, CUnitHierarchyUnit *pParent)
{
  if(pParent == NULL){
    return FindZUnit(dwZUnitID, m_pGeneralCommander);
  }

  if(pParent->GetZUnitID() == dwZUnitID) return pParent;

  CUnitHierarchyUnit *pUnit = pParent->GetFirstInferior(), *pRet;
  while(pUnit != NULL){
    pRet = FindZUnit(dwZUnitID, pUnit);
    if(pRet != NULL){
      return pRet;
    }
    pUnit = pUnit->GetNextSibbling();
  }

  return NULL;
}

void CUnitHierarchy::DeleteUnit(CUnitHierarchyUnit *pUnit)
{
  CUnitHierarchyUnit *pParent, **p;

  pParent = pUnit->GetCommander();
  if(pParent == NULL){
    Clear();
    return;
  }

  p = &(pParent->m_pFirstInferior);
  while((*p) != NULL){
    if((*p) == pUnit){
      *p = pUnit->GetNextSibbling();
      break;
    }
    p = &((*p)->m_pNextSibbling);
  }

  pUnit->Delete();
  delete pUnit;
}