// ZConnectedClient.cpp: implementation of the CZConnectedClient class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "ZConnectedClient.h"
#include "..\ZCivilization.h"

#include "ClientCommunication\ZClientUnitInfoSender.h"

#include "Common\MemoryPool\SelfPooledStack.h"
#include "Common\ServerClient\ControlConnection.h"
#include "Common\ServerClient\MapViewConnection.h"
#include "Common\ServerClient\UnitHierarchyConnection.h"

#include "TripleS\Interpret\Src\SysObjs\ISOSet.h"
#include "TripleS\Interpret\Src\SysObjs\ISOString.h"
#include "TripleS\Interpret\Src\SysObjs\ISOClient.h"
#include "TripleS\Interpret\Src\InterpretEvents.h"

#include "GameServer\GameServer\SCivilization.h"
#include "GameServer\GameServer\SMap.h"
#include "GameServer\GameServer\SUnit.inl"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CZConnectedClient, CNotifier);    

BEGIN_OBSERVER_MAP(CZConnectedClient, CNotifier)
  BEGIN_NOTIFIER(ID_NetworkServer)
    EVENT(E_NEWCLIENTCONNECTED)
      OnNewClientConnected((CNetworkConnection *)dwParam);
  END_NOTIFIER()

  BEGIN_NOTIFIER(ID_NetworkConnection)
    EVENT(E_NETWORKERROR)
	  EVENT(E_MEMORYERROR)
	  EVENT(E_ABORTIVECLOSE)
		  OnNetworkError(); return FALSE;
    EVENT(E_NORMALCLOSE)
      OnNormalClose(); return FALSE;
    EVENT(E_NEWVIRTUALCONNECTION)
      OnNewVirtualConnection((CNetworkConnection::SNewVirtualConnectionUserData *) dwParam); return FALSE;
  END_NOTIFIER()

  BEGIN_NOTIFIER ( ID_Civilization )
    EVENT ( E_DisconnectClient )
      OnDisconnectClient ( (CEvent *)dwParam ); return FALSE;
  END_NOTIFIER ()

  BEGIN_NOTIFIER ( ID_Self )
    EVENT ( E_ActionDone )
      ActionDone (); return FALSE;
  END_NOTIFIER ()

END_OBSERVER_MAP(CZConnectedClient, CNotifier)


// COnstructor
CZConnectedClient::CZConnectedClient()
{
  m_pCivilization = NULL;
  m_pConnection = NULL;
  m_pNext = NULL;
  m_pNetworkEventManager = NULL;
  m_pNetworkEventThread = NULL;

  m_ControlConnectionObserver.m_pConnectedClient = this;
  m_MapViewConnectionObserver.m_pConnectedClient = this;
  m_UnitHierarchyConnectionObserver.m_pConnectedClient = this;

  m_dwGlobalFunctionProcessID = 0;

  m_pISOClients = NULL;
  m_pQuestionPositionSyscall = NULL;
  m_pQuestionUnitSyscall = NULL;
}

// Destructor
CZConnectedClient::~CZConnectedClient()
{
  ASSERT(m_pNetworkEventManager == NULL);
  ASSERT(m_pNetworkEventThread == NULL);
	ASSERT(m_pCivilization == NULL);
	ASSERT(m_pConnection == NULL);
}


// Debug functions
#ifdef _DEBUG

void CZConnectedClient::AssertValid() const
{
  CNotifier::AssertValid();
  ASSERT(m_pCivilization != NULL);
  ASSERT_VALID(m_pConnection);
}

void CZConnectedClient::Dump(CDumpContext &dc) const
{
  CNotifier::Dump(dc);
}

#endif


// Creation

// Creates the object
void CZConnectedClient::Create(CString strUserName, CZCivilization *pCivilization)
{
  ASSERT(pCivilization != NULL);

  CNotifier::Create();

  m_strUserName = strUserName;

  // copy the civ
  m_pCivilization = pCivilization;

	// start the new thread
  m_pNetworkEventThread = AfxBeginThread(_NetworkEventThread, this);
  m_pNetworkEventThread->m_bAutoDelete = FALSE;

  VERIFY(m_eventNetworkEventThreadStarted.Lock());
  m_pCivilization->Connect ( this, ID_Civilization, m_pNetworkEventThread->m_nThreadID );
  // Connect us to us
  Connect ( this, ID_Self, m_pNetworkEventThread->m_nThreadID );
}


void CZConnectedClient::Delete()
{
	if(m_pNetworkEventThread != NULL){
		// stop the network event thread
    m_pNetworkEventThread->m_bAutoDelete = TRUE;
    if (m_pNetworkEventManager != NULL)
    {
		  m_pNetworkEventManager->InlayQuitEvent(0);
      m_pNetworkEventManager = NULL;
    }
		m_pNetworkEventThread = NULL;
	}

  m_pCivilization->LockISOClients();

  // Mark all ISOClients as dummy
  {
    CISOClient *pClient = m_pISOClients;
    while(pClient != NULL){
      VERIFY(pClient->m_lockLock.Lock());
      pClient->m_bActive = FALSE;
      pClient->m_pConnectedClient = NULL;
      VERIFY(pClient->m_lockLock.Unlock());

      pClient = pClient->m_pNext;
    }

    // And forget them
    m_pISOClients = NULL;
  }

  m_pCivilization->UnlockISOClients();

  // cancel all question syscalls
  {
    // Lock the connection
    VERIFY(m_lockMapViewConnection.Lock());

    if(m_pQuestionPositionSyscall != NULL){
      m_pQuestionPositionSyscall->FinishWaiting ( false, 0, 0 );
      m_pQuestionPositionSyscall = NULL;
    }

    if(m_pQuestionUnitSyscall != NULL){
      m_pQuestionUnitSyscall->FinishWaiting ( NULL );
      m_pQuestionUnitSyscall = NULL;
    }

    VERIFY(m_lockMapViewConnection.Unlock());
  }

  // Close the mapview connection
  m_MapViewConnection.DeleteVirtualConnection();
  m_UnitHierarchyConnection.DeleteVirtualConnection();
  m_ControlConnection.DeleteVirtualConnection();

  // delete all virt. connections object
  m_ScriptEditor.Delete();
  m_LogOutput.Delete();
  m_MapDownload.Delete();

  m_UnitInfoSender.Delete();	

  // destroy the connection
  if(m_pConnection != NULL){
    m_pConnection->Disconnect(this);
    m_pConnection->Delete(3000);
    delete m_pConnection;
    m_pConnection = NULL;
  }

  Disconnect ( this );

  // forget the civilization
  m_pCivilization->Disconnect ( this );
  m_pCivilization = NULL;

  CNotifier::Delete();
}


UINT CZConnectedClient::_NetworkEventThread(LPVOID pParam)
{
	CZConnectedClient *pConnectedClient = (CZConnectedClient *)pParam;
	pConnectedClient->NetworkEventThread();
	return 0;
}

void CZConnectedClient::NetworkEventThread()
{
	CEventManager *pEventManager = new CEventManager;
  pEventManager->Create();
  m_pNetworkEventManager = pEventManager;
	m_eventNetworkEventThreadStarted.SetEvent();

	pEventManager->DoEventLoop();

	pEventManager->Delete();
  delete pEventManager;
}

// Reactions

#ifdef _DEBUG
extern BOOL g_bUDPAlive;
#endif

void CZConnectedClient::OnNewClientConnected(CNetworkConnection *pConnection)
{
  // copy the connection
  m_pConnection = pConnection;

  // now connect us to the connection
  pConnection->Connect(this, ID_NetworkConnection, m_pNetworkEventThread->m_nThreadID);

#ifdef _DEBUG
  pConnection->SetUDPAliveChecking(g_bUDPAlive);
#endif
}

void CZConnectedClient::OnDisconnectClient ( CEvent * pEvent )
{
  // Delete us
  Delete ();
  delete this;

  // Set the event
  pEvent->SetEvent ();
}

void CZConnectedClient::OnNormalClose()
{
  // just say it to our civ
  ASSERT(m_pCivilization != NULL);

  m_pCivilization->DisconnectClient ( this );
}

void CZConnectedClient::OnNetworkError()
{
  // just say it to our civ
  ASSERT(m_pCivilization != NULL);

  m_pCivilization->DisconnectClient ( this );
}

void CZConnectedClient::OnNewVirtualConnection(CNetworkConnection::SNewVirtualConnectionUserData *pUserData)
{
  ASSERT(pUserData->dwSize >= 4);

  // lookup the first DWORD -> that's the ID
  switch(*((DWORD *)pUserData->pBuffer)){

  // The control connection
  case VirtualConnection_Control:
    m_ControlConnection = pUserData->VirtualConnection;
    m_ControlConnection.SetVirtualConnectionObserver(&m_ControlConnectionObserver, ID_ControlConnection, m_pNetworkEventThread->m_nThreadID);
    VERIFY(pUserData->pWakeUpEvent->SetEvent());
    break;

  // The map view connection
  case VirtualConnection_MapView:
    m_MapViewConnection = pUserData->VirtualConnection;
    m_MapViewConnection.SetVirtualConnectionObserver(&m_MapViewConnectionObserver, ID_MapViewConnection, m_pNetworkEventThread->m_nThreadID);
    VERIFY(pUserData->pWakeUpEvent->SetEvent());
    break;

  // The unit hierarchy connection
  case VirtualConnection_UnitHierarchy:
    m_UnitHierarchyConnection = pUserData->VirtualConnection;
    m_UnitHierarchyConnection.SetVirtualConnectionObserver(&m_UnitHierarchyConnectionObserver, ID_UnitHierarchyConnection, m_pNetworkEventThread->m_nThreadID);
    VERIFY(pUserData->pWakeUpEvent->SetEvent());
    break;

  // The script editor connection
  case VirtualConnection_ScriptEditor:
    m_ScriptEditor.Create(pUserData->VirtualConnection, m_pCivilization->GetCompiler(), m_pCivilization->GetCodeManager());
    VERIFY(pUserData->pWakeUpEvent->SetEvent());
    break;

  // The script error output
  case VirtualConnection_ScriptErrorOutput:
    m_ScriptEditor.CreateErrorOutput(pUserData->VirtualConnection);
    VERIFY(pUserData->pWakeUpEvent->SetEvent());
    break;

  case VirtualConnection_LogOutput:
    m_LogOutput.Create(pUserData->VirtualConnection, m_pCivilization->GetLogFile());
    VERIFY(pUserData->pWakeUpEvent->SetEvent());
    break;

  case VirtualConnection_MapDownload:
    m_MapDownload.Create(pUserData->VirtualConnection);
    VERIFY(pUserData->pWakeUpEvent->SetEvent());
    break;

// *ROMAN v--v

  case VirtualConnection_ClientControl:
  case VirtualConnection_ClientRequests:
  case VirtualConnection_ClientBriefInfos:
  case VirtualConnection_ClientCheckPointInfos:
  case VirtualConnection_ClientFullInfos:
  case VirtualConnection_ClientEnemyFullInfos:
  case VirtualConnection_ClientPing:
  case VirtualConnection_Resources:
  case VirtualConnection_MiniMap:
      m_UnitInfoSender.AddConnection(pUserData->VirtualConnection, *((DWORD *)pUserData->pBuffer), m_pCivilization);
	  VERIFY(pUserData->pWakeUpEvent->SetEvent());
	  break;

  // *ROMAN ^--^

  default:
    TRACE0("Unknown new virtual connection create attempt recieved.\n");
    ASSERT(FALSE);
  }
}

BEGIN_OBSERVER_MAP(CZConnectedClient::CControlConnectionObserver, CObserver)
  BEGIN_NOTIFIER(ID_ControlConnection)
    EVENT(E_PACKETAVAILABLE)
      OnPacketAvailable(); return FALSE;
  END_NOTIFIER()
END_OBSERVER_MAP(CZConnectedClient::CControlConnectionObserver, CObserver)

void CZConnectedClient::OnControlConnectionPacketAvailable()
{
  DWORD dwRequest, dwSize;

  // Recieve the request
  dwSize = sizeof(dwRequest);
  VERIFY(m_ControlConnection.BeginReceiveCompoundBlock());
  VERIFY(m_ControlConnection.ReceiveBlock(&dwRequest, dwSize));
  ASSERT(dwSize == 4);

  switch(dwRequest){
  case ControlRequest_Init:
    // Init request
    {

      VERIFY(m_ControlConnection.BeginSendCompoundBlock());

      // Send the answer ID
      DWORD dwAnswer = ControlAnswer_Init;
      m_ControlConnection.SendBlock(&dwAnswer, sizeof(dwAnswer));

      // Send the structure
      SControlAnswerInit h;
      h.m_dwCivilizationID = m_pCivilization->GetSCivilization()->GetCivilizationID();
		  h.m_dwTimeslice = g_cMap.GetTimeSlice();
      h.m_dwTimesliceInterval = g_cMap.GetTimeSliceLength();
      h.m_dwStartPositionX = m_pCivilization->GetSCivilization()->GetClientStartMapPosition().x;
      h.m_dwStartPositionY = m_pCivilization->GetSCivilization()->GetClientStartMapPosition().y;
		  VERIFY(m_ControlConnection.SendBlock(&h, sizeof(h)));

      VERIFY(m_ControlConnection.EndSendCompoundBlock());
    }
    break;
  default:
    // Ooops unknown packet
    TRACE0("ConnectedClient - ControlConnection - Unknown request.\n");
    break;
  }

  VERIFY(m_ControlConnection.EndReceiveCompoundBlock());
}

BEGIN_OBSERVER_MAP(CZConnectedClient::CMapViewConnectionObserver, CObserver)
  BEGIN_NOTIFIER(ID_MapViewConnection)
    EVENT(E_PACKETAVAILABLE)
      OnPacketAvailable(); return FALSE;
  END_NOTIFIER()
END_OBSERVER_MAP(CZConnectedClient::CMapViewConnectionObserver, CObserver)

void CZConnectedClient::OnMapViewConnectionPacketAvailable()
{
  DWORD dwRequest, dwSize;

  // Lock the mapview connection
  VERIFY(m_lockMapViewConnection.Lock());

  dwSize = sizeof(dwRequest);
  VERIFY(m_MapViewConnection.BeginReceiveCompoundBlock());
  VERIFY(m_MapViewConnection.ReceiveBlock(&dwRequest, dwSize));
  ASSERT(dwSize == sizeof(dwRequest));

  switch(dwRequest){
  
  // Run global function request
  case MapViewRequest_RunFunction:
    try
    {
      ASSERT(m_dwGlobalFunctionProcessID == 0);

      // Receive the header
      SMapViewRequest_RunFunction sRequest;
      dwSize = sizeof(sRequest);
      VERIFY(m_MapViewConnection.ReceiveBlock(&sRequest, dwSize));
      ASSERT(dwSize == sizeof(sRequest));

      CString strFunctionName;
      // Read the function name
      {
        char *pTxt = new char[sRequest.m_dwFunctionNameLength];
        dwSize = sRequest.m_dwFunctionNameLength;
        VERIFY(m_MapViewConnection.ReceiveBlock(pTxt, dwSize));
        ASSERT(dwSize == sRequest.m_dwFunctionNameLength);
        strFunctionName = pTxt;
        delete pTxt;
      }

      // Create the params bag
      CIBag cBag;
      cBag.Create();

      // First param will be the client object, so create it
      CISOClient *pISOClient = (CISOClient *)m_pCivilization->GetInterpret()->CreateSystemObject(SO_CLIENT);
      if(pISOClient == NULL){
        // Ooops error creating the client object
        cBag.Delete();
        throw (int)1;
      }
      // Fill it with our pointer
      pISOClient->Set( this, m_pCivilization );

      // and add it to bag
      cBag.AddSystemObject(pISOClient);

      // The ISO client will be added to the list of them 
      // after we'll unlock the virtual connection

      // Make the set of selected units
      CISOSet<CZUnit *> *pSelectedUnitsSet;
      CType cType;
      cType.SetUnit(g_StringTable.AddItem(DEFAULT_UNIT_PARENT, false));
      pSelectedUnitsSet = (CISOSet<CZUnit *> *)m_pCivilization->GetInterpret()->CreateSet(&cType, true);
      if(pSelectedUnitsSet == NULL){
        pISOClient->Release();
        cBag.Delete();
        throw (int)1;
      }

      {
        // Read units
        DWORD dwUnit, dwUnitID;
        CZUnit *pUnit;
        CSUnit *pSUnit;
        for(dwUnit = 0; dwUnit < sRequest.m_dwSelectedUnitsCount; dwUnit++){
          dwSize = sizeof(dwUnitID);
          VERIFY(m_MapViewConnection.ReceiveBlock(&dwUnitID, dwSize));
          ASSERT(dwSize == sizeof(dwUnitID));
          pSUnit = g_cMap.GetUnitByID(dwUnitID);
          if(pSUnit != NULL){
            pSUnit->AddRef();
            pUnit = pSUnit->GetZUnit();
            pUnit->AddRef();
            ASSERT(pUnit != NULL);

            pSelectedUnitsSet->AddItemToSet(pUnit);

            pUnit->Release();
            pSUnit->Release();
          }
        }
      }

      // Add the set to the bag
      cBag.AddSystemObject( pSelectedUnitsSet );
      pSelectedUnitsSet->Release();

      // Read params and add them to the bag
      {
        DWORD dwParam;
        SMapViewRequest_RunFunctionParam sParam;
        for(dwParam = 0; dwParam < sRequest.m_dwParamsCount; dwParam++){
          dwSize = sizeof(sParam);
          VERIFY(m_MapViewConnection.ReceiveBlock(&sParam, dwSize));
          ASSERT(dwSize == sizeof(sParam));

          switch(sParam.m_eParamType){
          case SMapViewRequest_RunFunctionParam::ParamType_Integer:
            cBag.AddConstantInt(sParam.m_nValue);
            break;
          case SMapViewRequest_RunFunctionParam::ParamType_Real:
            cBag.AddConstantFloat(sParam.m_dbValue);
            break;
          case SMapViewRequest_RunFunctionParam::ParamType_String:
            // Read the string
            {
              char *pTxt = new char[sParam.m_dwStringLength];
              dwSize = sParam.m_dwStringLength;
              VERIFY(m_MapViewConnection.ReceiveBlock(pTxt, dwSize));
              ASSERT(dwSize == sParam.m_dwStringLength);

              CISOString *pString = (CISOString *)m_pCivilization->GetInterpret()->CreateSystemObject(SO_STRING);
              if(pString == NULL){
                pISOClient->Release();
                cBag.Delete();
                throw (int)1;
              }
              pString->Set(pTxt);
              cBag.AddSystemObject(pString);
              pString->Release();
              delete pTxt;
            }
            break;
          case SMapViewRequest_RunFunctionParam::ParamType_Bool:
            cBag.AddConstantBool(sParam.m_bValue ? true : false);
            break;
          case SMapViewRequest_RunFunctionParam::ParamType_Position:
            {
              CISOPosition * pPosition = (CISOPosition *)m_pCivilization->GetInterpret ()->CreateSystemObject ( SO_POSITION );
              if ( pPosition == NULL )
              {
                pISOClient->Release ();
                cBag.Delete ();
                throw (int)1;
              }
              pPosition->Set ( sParam.m_sPosition.m_dwX, sParam.m_sPosition.m_dwY, sParam.m_sPosition.m_dwZ );
              cBag.AddSystemObject ( pPosition );
              pPosition->Release ();
            }
            break;
          case SMapViewRequest_RunFunctionParam::ParamType_Unit:
            {
              CSUnit * pUnit = g_cMap.GetUnitByID ( sParam.m_dwSUnitID );
              cBag.AddUnit ( pUnit->GetZUnit (), CCodeManager::m_pDefaultUnitType );
            }
            break;
          default:
            ASSERT(FALSE);
            break;
          }
        }
      }

      // OK that's all
      // now call the function
      CIRunGlobalFunctionData cRunGlobalFunctionData( strFunctionName, cBag );

      cRunGlobalFunctionData.m_ProcessEvent.ResetEvent();

      // Unlock the connection
      VERIFY(m_lockMapViewConnection.Unlock());

      // Send event to interpret
      m_pCivilization->GetInterpret()->InlayEvent(RUN_GLOBAL_FUNCTION, (DWORD)&cRunGlobalFunctionData,
        m_pCivilization->GetInterpret());

      // Wait for the interpret to start the process
      VERIFY(cRunGlobalFunctionData.m_ProcessEvent.Lock());

      // copy the process ID
      m_dwGlobalFunctionProcessID = cRunGlobalFunctionData.m_nProcessID;

      // If some error calling the function -> forget the action
      if(m_dwGlobalFunctionProcessID == 0){
        pISOClient->Release();
        cBag.Delete();
        // We must lock it here, the catch reaction needs it
        VERIFY(m_lockMapViewConnection.Lock());
        throw (int)1;
      }

      // Set the PID to the ISOClient
      pISOClient->m_dwProcessID = m_dwGlobalFunctionProcessID;

      DWORD dwPID = m_dwGlobalFunctionProcessID;

      // Also close the bag
      cBag.Delete();

      m_pCivilization->LockISOClients();
      
      // add the ISOClient to our list of them
      pISOClient->m_pNext = m_pISOClients;
      m_pISOClients = pISOClient;
      m_pCivilization->UnlockISOClients();

      // Release the ISOClient
      pISOClient->Release();

      // Must exit immidietly
      VERIFY ( m_MapViewConnection.EndReceiveCompoundBlock () );
      return;
    }
    catch(int){
      // Some error occured -> send error to the client
      VERIFY(m_MapViewConnection.BeginSendCompoundBlock());
      DWORD dwAnswer = MapViewAnswer_RunFunctionError;
      VERIFY(m_MapViewConnection.SendBlock(&dwAnswer, sizeof(dwAnswer)));
      VERIFY(m_MapViewConnection.EndSendCompoundBlock());
    }
    break;

  case MapViewRequest_KillFunction:
    {
      // Some function must be running
      if(m_dwGlobalFunctionProcessID == 0) break;

      DWORD dwPID = m_dwGlobalFunctionProcessID;
      // Unlock the connection
      VERIFY(m_MapViewConnection.EndReceiveCompoundBlock());
      VERIFY(m_lockMapViewConnection.Unlock());

      // Just kill the process of the function
      // It means send an event
      SIKillProcessData cKillProcessData;
      cKillProcessData.nProcessID = dwPID;
      cKillProcessData.Status = STATUS_EMPTYING_QUEUE;
	  cKillProcessData.ProcessKilled.ResetEvent ();
	  cKillProcessData.m_bDelete = false;

      m_pCivilization->GetInterpret()->InlayEvent(KILL_PROCESS, (DWORD)&cKillProcessData,
        m_pCivilization->GetInterpret());

      // Here wait for the interpret to kill the function
      VERIFY(cKillProcessData.ProcessKilled.Lock());

      // Set the pISOClient for this PID to Dummy
      m_pCivilization->LockISOClients();
      CISOClient *pClient = m_pISOClients;
      while(pClient != NULL){
        VERIFY(pClient->m_lockLock.Lock());
        if(pClient->m_dwProcessID == dwPID){
          // Just a small hmm I'd say error
          // This function locks the mapview connection
          // But we already have it locked, so, it's not .. clear.
          ActionDone ();
          pClient->m_bActive = FALSE;
        }
        VERIFY(pClient->m_lockLock.Unlock());

        pClient = pClient->m_pNext;
      }

      m_pCivilization->UnlockISOClients();

      return;
    }
    break;

  case MapViewRequest_AskForPosition:
    {
      if( m_pQuestionPositionSyscall == NULL ){
        TRACE("Unexpected question answer.\n");
        ASSERT(FALSE);
        break;
      }

      // recieve the header
      SMapViewRequest_AskForPosition header;
      dwSize = sizeof(header);
      VERIFY(m_MapViewConnection.ReceiveBlock( &header, dwSize ));
      ASSERT(dwSize == sizeof(header));

      // end the syscall successfully
      m_pQuestionPositionSyscall->FinishWaiting( true, header.m_dwPositionX, header.m_dwPositionY );
      m_pQuestionPositionSyscall = NULL;
    }
    break;

  case MapViewRequest_AskForPositionCanceled:
    {
      if( m_pQuestionPositionSyscall == NULL ){
        TRACE("Unexpected question answer.\n");
        ASSERT(FALSE);
        break;
      }

      // return false
      m_pQuestionPositionSyscall->FinishWaiting( false, 0, 0 );
      m_pQuestionPositionSyscall = NULL;
    }
    break;

  case MapViewRequest_AskForUnit:
    {
      if( m_pQuestionUnitSyscall == NULL ){
        TRACE("Unexpected question answer.\n");
        ASSERT(FALSE);
        break;
      }

      // recieve the header
      SMapViewRequest_AskForUnit header;
      dwSize = sizeof(header);
      VERIFY(m_MapViewConnection.ReceiveBlock( &header, dwSize ));
      ASSERT(dwSize == sizeof(header));

      // end the syscall successfully
      CSUnit *pSUnit = g_cMap.GetUnitByID ( header.m_dwUnitID );
      if ( pSUnit == NULL )
      {
        m_pQuestionUnitSyscall->FinishWaiting ( NULL );
      }
      else{
        CZUnit *pUnit = pSUnit->GetZUnit();
        m_pQuestionUnitSyscall->FinishWaiting( pUnit );
      }
      m_pQuestionUnitSyscall = NULL;
    }
    break;

  case MapViewRequest_AskForUnitCanceled:
    {
      if( m_pQuestionUnitSyscall == NULL ){
        TRACE("Unexpected question answer.\n");
        ASSERT(FALSE);
        break;
      }

      // return false
      m_pQuestionUnitSyscall->FinishWaiting( NULL );
      m_pQuestionUnitSyscall = NULL;
    }
    break;

  case MapViewRequest_ToolbarFileUpdate:
    {
      // Download the toolbar file to the client
      // We'll call a recursive function to do the job
      SendToolbarFiles ( m_pCivilization->GetCodeManager ()->GetToolbarsDirectory (), "" );

      // Now send that the update is done
      {
        VERIFY ( m_MapViewConnection.BeginSendCompoundBlock () );
        DWORD dwAnswer = MapViewAnswer_ToolbarUpdateComplete;
        VERIFY ( m_MapViewConnection.SendBlock ( &dwAnswer, sizeof ( dwAnswer ) ) );
        VERIFY ( m_MapViewConnection.EndSendCompoundBlock () );
      }
    }
    break;

  default:
    // Ooops unknown packet
    TRACE0("ConnectedClient - MapViewConnection - Unknown request.\n");
    break;
  }

  VERIFY(m_MapViewConnection.EndReceiveCompoundBlock());

  // Unlock the connection
  VERIFY(m_lockMapViewConnection.Unlock());
}

// This function can be run only under the civilization's ISOClients lock
void CZConnectedClient::UnregisterISOClient( CISOClient *pISOClient )
{
  // Remove it from our list
  CISOClient **p = &m_pISOClients;
  while( *p != NULL ){
    if( *p == pISOClient ){
      *p = (*p)->m_pNext;
      break;
    }
    p = &((*p)->m_pNext);
  }
  pISOClient->m_pNext = NULL;

  // If it called this method, it means, that the process is shutting down
  // So no more processing is needed
}

void CZConnectedClient::ActionDone()
{
  // Firs lock the mapview connection
  VERIFY(m_lockMapViewConnection.Lock());

  // If there is some global function in action send an end message to client
  if(m_dwGlobalFunctionProcessID != 0){
    VERIFY(m_MapViewConnection.BeginSendCompoundBlock());
    DWORD dwAnswer = MapViewAnswer_FunctionActionDone;
    VERIFY(m_MapViewConnection.SendBlock(&dwAnswer, sizeof(dwAnswer)));
    VERIFY(m_MapViewConnection.EndSendCompoundBlock());

    // And forget the global function
    m_dwGlobalFunctionProcessID = 0;
  }

  // Unlock the connection
  VERIFY(m_lockMapViewConnection.Unlock());
}

void CZConnectedClient::InternalSendMessage( CString strText, EMessageType eType,
                                             DWORD dwXPos, DWORD dwYPos, DWORD dwUnitID )
{
  VERIFY(m_lockMapViewConnection.Lock());

  VERIFY(m_MapViewConnection.BeginSendCompoundBlock());
  DWORD dwAnswer = MapViewAnswer_Message;
  VERIFY(m_MapViewConnection.SendBlock(&dwAnswer, sizeof(dwAnswer)));

  SMapViewAnswer_Message msg;

  switch(eType){
  case MsgType_User:
    msg.m_eMessageType = SMapViewAnswer_Message::MsgType_User;
    break;
  case MsgType_Info:
    msg.m_eMessageType = SMapViewAnswer_Message::MsgType_Info;
    break;
  case MsgType_Warning:
    msg.m_eMessageType = SMapViewAnswer_Message::MsgType_Warning;
    break;
  case MsgType_Error:
    msg.m_eMessageType = SMapViewAnswer_Message::MsgType_Error;
    break;
  case MsgType_Admin:
    msg.m_eMessageType = SMapViewAnswer_Message::MsgType_Admin;
    break;
  default:
    msg.m_eMessageType = SMapViewAnswer_Message::MsgType_Info;
    break;
  }

  msg.m_dwTextLength = strText.GetLength() + 1;
  msg.m_dwXPos = dwXPos;
  msg.m_dwYPos = dwYPos;
  msg.m_dwUnitID = dwUnitID;

  // Send the structure
  VERIFY(m_MapViewConnection.SendBlock(&msg, sizeof(msg)));

  // Send the text
  VERIFY(m_MapViewConnection.SendBlock((LPCSTR)strText, msg.m_dwTextLength));

  VERIFY(m_MapViewConnection.EndSendCompoundBlock());

  VERIFY(m_lockMapViewConnection.Unlock());
}

void CZConnectedClient::AskForPosition ( CString strQuestion, CISOClientAskForPositionSyscall *pSyscall )
{
  // Send the question to the client
  VERIFY(m_lockMapViewConnection.Lock());
  ASSERT(m_pQuestionPositionSyscall == NULL);

  // Copy the syscall
  m_pQuestionPositionSyscall = pSyscall;

  VERIFY(m_MapViewConnection.BeginSendCompoundBlock());
  // Send the ID
  DWORD dwAnswer = MapViewAnswer_AskForPosition;
  VERIFY(m_MapViewConnection.SendBlock( &dwAnswer, sizeof(dwAnswer) ));

  // send the header
  SMapViewAnswer_AskForPosition ask;
  ask.m_dwQuestionLength = strQuestion.GetLength() + 1;
  VERIFY(m_MapViewConnection.SendBlock( &ask, sizeof(ask) ));

  // send the question text
  VERIFY(m_MapViewConnection.SendBlock( (LPCSTR)strQuestion, ask.m_dwQuestionLength ));

  VERIFY(m_MapViewConnection.EndSendCompoundBlock());

  VERIFY(m_lockMapViewConnection.Unlock());
}

void CZConnectedClient::AskForUnit ( CString strQuestion, CISOClientAskForUnitSyscall::EUnitType eUnitType,
                                     CISOClientAskForUnitSyscall *pSyscall )
{
  // Send the question to the client
  VERIFY(m_lockMapViewConnection.Lock());
  ASSERT(m_pQuestionUnitSyscall == NULL);

  // Copy the syscall
  m_pQuestionUnitSyscall = pSyscall;

  VERIFY(m_MapViewConnection.BeginSendCompoundBlock());
  // Send the ID
  DWORD dwAnswer = MapViewAnswer_AskForUnit;
  VERIFY(m_MapViewConnection.SendBlock( &dwAnswer, sizeof(dwAnswer) ));

  // send the header
  SMapViewAnswer_AskForUnit ask;
  ask.m_dwQuestionLength = strQuestion.GetLength() + 1;
  switch( eUnitType )
  {
  case CISOClientAskForUnitSyscall::UnitType_Any:
    ask.m_eUnitType = SMapViewAnswer_AskForUnit::UnitType_Any;
    break;
  case CISOClientAskForUnitSyscall::UnitType_Enemy:
    ask.m_eUnitType = SMapViewAnswer_AskForUnit::UnitType_Enemy;
    break;
  case CISOClientAskForUnitSyscall::UnitType_My:
    ask.m_eUnitType = SMapViewAnswer_AskForUnit::UnitType_My;
    break;
  default:
    TRACE("ConnectedClient - unknown unit type question.\n");
    ASSERT(FALSE);
    // set some default
    ask.m_eUnitType = SMapViewAnswer_AskForUnit::UnitType_Any;
    break;
  }
  VERIFY(m_MapViewConnection.SendBlock( &ask, sizeof(ask) ));

  // send the question text
  VERIFY(m_MapViewConnection.SendBlock( (LPCSTR)strQuestion, ask.m_dwQuestionLength ));

  VERIFY(m_MapViewConnection.EndSendCompoundBlock());

  VERIFY(m_lockMapViewConnection.Unlock());
}


BEGIN_OBSERVER_MAP(CZConnectedClient::CUnitHierarchyConnectionObserver, CObserver)
  BEGIN_NOTIFIER(ID_UnitHierarchyConnection)
    EVENT(E_PACKETAVAILABLE)
      OnPacketAvailable(); return FALSE;
  END_NOTIFIER()
END_OBSERVER_MAP(CZConnectedClient::CUnitHierarchyConnectionObserver, CObserver)

void CZConnectedClient::OnUnitHierarchyConnectionPacketAvailable()
{
  DWORD dwRequest, dwSize;

  dwSize = sizeof(dwRequest);
  VERIFY(m_UnitHierarchyConnection.BeginReceiveCompoundBlock());
  VERIFY(m_UnitHierarchyConnection.ReceiveBlock(&dwRequest, dwSize));
  ASSERT(dwSize == sizeof(dwRequest));

  switch(dwRequest){
  case UnitHierarchyRequest_GetWhole:
    // Get whole unit hierarchy request
    // (no more data to receive)
    {
      // Send the answer
      DWORD dwAnswer = UnitHierarchyAnswer_GetWhole;
      VERIFY(m_UnitHierarchyConnection.BeginSendCompoundBlock( TRUE, PACKETPRIORITY_IDLE ));
      VERIFY(m_UnitHierarchyConnection.SendBlock(&dwAnswer, sizeof(dwAnswer)));

      // Now send the header
      SUnitHierarchyAnswer_GetWhole header;

      // First we will build a hierarchy tree in memory (under all locks)
      // Then we'll ask for attributes (like script name)
      // Don't forget to AddRef units

      // Our node
      SUnitHierarchyNode * pRootNode;
      // Pool for our nodes
      CTypedMemoryPool < SUnitHierarchyNode > cPool ( 50 );

      // Lock the unit hierarchy for our civilization
      m_pCivilization->LockHierarchyAndEvents();
      // And also lock the unit misc data
      m_pCivilization->LockUnitData();

      // Get the unit count
      header.m_dwUnitCount = m_pCivilization->GetUnitCount();
      VERIFY(m_UnitHierarchyConnection.SendBlock(&header, sizeof(header)));

      // Get the general commander of our civilization
      pRootNode = cPool.Allocate ();
      pRootNode->m_pUnit = m_pCivilization->GetGeneralCommander();
      if ( pRootNode->m_pUnit != NULL )
      {
        pRootNode->m_pUnit->AddRef ();
        pRootNode->m_pFirstInferior = NULL;
        pRootNode->m_pNextSibling = NULL;
        pRootNode->m_pCommander = NULL;

        BuildUnitHierarchyNodeRecursive ( pRootNode, &cPool );
      }

      // unlock misc data
      m_pCivilization->UnlockUnitData();
      // Unlock the hierarchy for our civilization
      m_pCivilization->UnlockHierarchyAndEvents();

      // OK we have the hierarchy in memory
      // Now go through it again (without locks) and send it to network
      // On the way we'll also delete it
      SendUnitHierarchyRecordRecursive ( pRootNode, &cPool );

      // Close the sending connection
      VERIFY(m_UnitHierarchyConnection.EndSendCompoundBlock());
    }
    break;

  case UnitHierarchyRequest_ChangeScript:
    // Change some scripts
    {
      // read the header
      SUnitHierarchyRequest_ChangeScript header;
      dwSize = sizeof(header);
      VERIFY(m_UnitHierarchyConnection.ReceiveBlock(&header, dwSize));
      ASSERT(dwSize == sizeof(header));

      ASSERT(header.m_dwUnitCount > 0);

      // send the reply
      VERIFY(m_UnitHierarchyConnection.BeginSendCompoundBlock());
      DWORD dwAnswer = UnitHierarchyAnswer_ChangeScript;
      VERIFY(m_UnitHierarchyConnection.SendBlock(&dwAnswer, sizeof(dwAnswer)));
      // Send the reply header
      SUnitHierarchyAnswer_ChangeScript h;
      h.m_dwUnitCount = header.m_dwUnitCount;
      VERIFY(m_UnitHierarchyConnection.SendBlock(&h, sizeof(h)));
      
      CString strNewScriptName;
      // receive the new script name
      VERIFY(m_UnitHierarchyConnection.ReceiveBlock(strNewScriptName.GetBuffer(header.m_dwScriptNameLength),
        header.m_dwScriptNameLength));
      strNewScriptName.ReleaseBuffer();

      // Go through units
      DWORD dwUnit, dwZUnitID;
      SUnitHierarchyAnswer_ChangeScript_UnitRecord record;
      CZUnit *pUnit;

      // First build a list of units
      CZUnit * * pUnits = (CZUnit * * ) new BYTE [ sizeof ( CZUnit * ) * header.m_dwUnitCount ];
      DWORD * aZUnitIDs = (DWORD *) new DWORD [ header.m_dwUnitCount ];

      m_pCivilization->LockHierarchyAndEvents();

      for(dwUnit = 0; dwUnit < header.m_dwUnitCount; dwUnit++){

        // receive the unit ID
        dwSize = sizeof(dwZUnitID);
        VERIFY(m_UnitHierarchyConnection.ReceiveBlock(&dwZUnitID, dwSize));
        ASSERT(dwSize == sizeof(dwZUnitID));

        aZUnitIDs [ dwUnit ] = dwZUnitID;
        // find the unit
        pUnit = m_pCivilization->GetUnitByID(dwZUnitID);
        if ( pUnit == NULL )
        {
          pUnits [ dwUnit ] = NULL;
        }
        else
        {
          pUnit->AddRef ();
          pUnits [ dwUnit ] = pUnit;
        }
      }

      m_pCivilization->UnlockHierarchyAndEvents();

      // Go through our list and change the scripts

      for(dwUnit = 0; dwUnit < header.m_dwUnitCount; dwUnit++){

        pUnit = pUnits [ dwUnit ];

        record.m_dwZUnitID = aZUnitIDs [ dwUnit ];
        record.m_dwResult = 0;

        do{
          // If dead -> reply
          if(pUnit == NULL){
            record.m_dwResult = 2;
            break;
          }

          // change the script
          SIChangeUnitScript sChangeUnitScript;
          sChangeUnitScript.m_pUnit = pUnit;
          sChangeUnitScript.m_strScriptName = strNewScriptName;
          m_pCivilization->GetInterpret()->InlayEvent(CHANGE_UNIT_SCRIPT, (DWORD)&sChangeUnitScript, m_pCivilization->GetInterpret());
          VERIFY(sChangeUnitScript.m_Done.Lock());

          if(!sChangeUnitScript.m_bSuccess){
            record.m_dwResult = 1;
          }

          // Release the unit
          pUnit->Release ();

        }while(FALSE);

        // send the result
        VERIFY(m_UnitHierarchyConnection.SendBlock(&record, sizeof(record)));
      }

      // Free arrays
      delete aZUnitIDs;
      delete pUnits;

      VERIFY(m_UnitHierarchyConnection.EndSendCompoundBlock());
    }
    break;


  case UnitHierarchyRequest_ChangeName:
    // Change some name
    {
      // read the header
      SUnitHierarchyRequest_ChangeName header;
      dwSize = sizeof(header);
      VERIFY(m_UnitHierarchyConnection.ReceiveBlock(&header, dwSize));
      ASSERT(dwSize == sizeof(header));

      ASSERT(header.m_dwUnitCount > 0);

      // send the reply
      VERIFY(m_UnitHierarchyConnection.BeginSendCompoundBlock());
      DWORD dwAnswer = UnitHierarchyAnswer_ChangeName;
      VERIFY(m_UnitHierarchyConnection.SendBlock(&dwAnswer, sizeof(dwAnswer)));
      // Send the reply header
      SUnitHierarchyAnswer_ChangeName h;
      h.m_dwUnitCount = header.m_dwUnitCount;
      VERIFY(m_UnitHierarchyConnection.SendBlock(&h, sizeof(h)));
      
      CString strNewName;
      // receive the new script name
      dwSize = header.m_dwNameLength;
      VERIFY(m_UnitHierarchyConnection.ReceiveBlock(strNewName.GetBuffer(header.m_dwNameLength), dwSize));
      ASSERT(dwSize == header.m_dwNameLength);
      strNewName.ReleaseBuffer();

      // Go through units
      DWORD dwUnit, dwZUnitID;
      SUnitHierarchyAnswer_ChangeScript_UnitRecord record;
      CZUnit *pUnit;

      m_pCivilization->LockHierarchyAndEvents();

      for(dwUnit = 0; dwUnit < header.m_dwUnitCount; dwUnit++){

        // receive the unit ID
        dwSize = sizeof(dwZUnitID);
        VERIFY(m_UnitHierarchyConnection.ReceiveBlock(&dwZUnitID, dwSize));
        ASSERT(dwSize == sizeof(dwZUnitID));

        // find the unit
        pUnit = m_pCivilization->GetUnitByID(dwZUnitID);

        record.m_dwZUnitID = dwZUnitID;
        record.m_dwResult = 0;

        do{
          // If dead -> reply
          if(pUnit == NULL){
            record.m_dwResult = 2;
            break;
          }

          // change the name
          m_pCivilization->LockUnitData();
          pUnit->SetName(strNewName);
          m_pCivilization->UnlockUnitData();

        }while(FALSE);

        // send the result
        VERIFY(m_UnitHierarchyConnection.SendBlock(&record, sizeof(record)));
      }

      m_pCivilization->UnlockHierarchyAndEvents();

      VERIFY(m_UnitHierarchyConnection.EndSendCompoundBlock());
    }
    break;

  default:
    TRACE("Ooops - UnitHierarchyConnection - Unknown request arrived.\n");
    ASSERT(FALSE);
    break;
  }

  VERIFY(m_UnitHierarchyConnection.EndReceiveCompoundBlock());
}

// Builds unit hierarchy for given node
void CZConnectedClient::BuildUnitHierarchyNodeRecursive ( SUnitHierarchyNode * pNode, CTypedMemoryPool < SUnitHierarchyNode > * pPool )
{
  CZUnit * pInferior = pNode->m_pUnit->GetFirstInferior ();
  SUnitHierarchyNode * pNewNode, **pPrevNodePlace = &(pNode->m_pFirstInferior);
  while ( pInferior != NULL )
  {
    pNewNode = pPool->Allocate ();
    *pPrevNodePlace = pNewNode;
    pPrevNodePlace = &(pNewNode->m_pNextSibling);
    pNewNode->m_pCommander = pNode;
    pNewNode->m_pNextSibling = NULL;
    pNewNode->m_pFirstInferior = NULL;
    pNewNode->m_pUnit = pInferior;
    pNewNode->m_pUnit->AddRef ();

    BuildUnitHierarchyNodeRecursive ( pNewNode, pPool );

    pInferior = pInferior->GetNextSibling ();
  }
}

// Sends unit records for givven unit node and also for all children units
// Called recursively
void CZConnectedClient::SendUnitHierarchyRecordRecursive( SUnitHierarchyNode * pNode, CTypedMemoryPool < SUnitHierarchyNode > * pPool  )
{
  // Construct the unit record
  SUnitHierarchyAnswer_UnitRecord record;

  record.m_dwZUnitID = pNode->m_pUnit->GetID();
  if(pNode->m_pCommander == NULL)
    record.m_dwZCommanderID = 0;
  else
    record.m_dwZCommanderID = pNode->m_pCommander->m_pUnit->GetID ();

  if(pNode->m_pUnit->GetSUnit() == NULL){
    record.m_dwSUnitID = 0x0FFFFFFFF;
    record.m_dwSUnitTypeID = 0;
  }
  else{
    VERIFY(pNode->m_pUnit->GetSUnit()->ReaderLock());
    record.m_dwSUnitID = pNode->m_pUnit->GetSUnit()->GetID();
    record.m_dwSUnitTypeID = pNode->m_pUnit->GetSUnit()->GetUnitType()->GetID();
    pNode->m_pUnit->GetSUnit()->ReaderUnlock();
  }

  // Count inferior units
  DWORD dwInferiorCount = 0;
  {
    SUnitHierarchyNode * pN = pNode->m_pFirstInferior;
    while(pN != NULL){
      dwInferiorCount++;
      pN = pN->m_pNextSibling;
    }
  }
  record.m_dwInferiorCount = dwInferiorCount;

/* obsolete

  SIGetUnitInfo sGetUnitInfo;
  sGetUnitInfo.m_pUnit = pNode->m_pUnit;
  sGetUnitInfo.m_Done.ResetEvent();
  m_pCivilization->GetInterpret()->InlayEvent(GET_UNIT_INFO, (DWORD)&sGetUnitInfo, m_pCivilization->GetInterpret());
  VERIFY(sGetUnitInfo.m_Done.Lock());
*/
  m_pCivilization->LockUnitData();
  CString strScriptName = (CString)*pNode->m_pUnit->m_pIUnitType->m_stiName;
  CString strName = pNode->m_pUnit->GetName();
  m_pCivilization->UnlockUnitData();

  record.m_dwScriptNameLength = strScriptName.GetLength() + 1;
  record.m_dwUnitNameLength = strName.GetLength() + 1;

  // Now send the record
  VERIFY(m_UnitHierarchyConnection.SendBlock( &record, sizeof(record) ));

  // Send the script name
  VERIFY(m_UnitHierarchyConnection.SendBlock( (LPCSTR)strScriptName, record.m_dwScriptNameLength ));
  // Send the name
  VERIFY(m_UnitHierarchyConnection.SendBlock( (LPCSTR)strName, record.m_dwUnitNameLength ));

  // Now send all our child units recursively
  {
    SUnitHierarchyNode * pN = pNode->m_pFirstInferior, *pDoIt;
    while(pN != NULL){
      pDoIt = pN;
      pN = pN->m_pNextSibling;

      SendUnitHierarchyRecordRecursive( pDoIt, pPool );
    }
  }

  // No delete our node
  pNode->m_pUnit->Release ();
  pPool->Free ( pNode );

  // And that's all
}

// Send whole directory to the client (used for toolbars download)
void CZConnectedClient::SendToolbarFiles ( CString strDirectory, CString strRelativePath )
{
  // List all files in the directory
  CFileFind cFind;
  if ( !cFind.FindFile ( strDirectory + "*.*" ) ) return;

  bool bContinue = true;
  while ( bContinue )
  {
    bContinue = cFind.FindNextFile () ? true : false;

    if ( cFind.IsDots () )
      continue;
    if ( cFind.IsHidden () )
      continue;
    if ( cFind.IsSystem () )
      continue;
    if ( cFind.IsDirectory () )
    {
      // Send the directory name
      CString strDirPath = strRelativePath + cFind.GetFileName ();
      VERIFY ( m_MapViewConnection.BeginSendCompoundBlock () );
      DWORD dwAnswer = MapViewAnswer_ToolbarFileUpdate;
      VERIFY ( m_MapViewConnection.SendBlock ( &dwAnswer, sizeof ( dwAnswer ) ) );

      SMapViewAnswer_ToolbarFileUpdate sHeader;
      sHeader.m_dwFlags = 1;
      sHeader.m_dwPathLength = strDirPath.GetLength () + 1;
      sHeader.m_dwFileLength = 0;
      VERIFY ( m_MapViewConnection.SendBlock ( &sHeader, sizeof ( sHeader ) ) );

      VERIFY ( m_MapViewConnection.SendBlock ( (LPCSTR)strDirPath, sHeader.m_dwPathLength ) );
      VERIFY ( m_MapViewConnection.EndSendCompoundBlock () );

      // Now call us recursively to send the subdir
      SendToolbarFiles ( strDirectory + cFind.GetFileName () + "\\", strRelativePath + cFind.GetFileName () + "\\" );

      continue;
    }

    // Send one file

    VERIFY ( m_MapViewConnection.BeginSendCompoundBlock ( TRUE ) );
    DWORD dwAnswer = MapViewAnswer_ToolbarFileUpdate;
    VERIFY ( m_MapViewConnection.SendBlock ( &dwAnswer, sizeof ( dwAnswer ) ) );

    SMapViewAnswer_ToolbarFileUpdate sHeader;
    sHeader.m_dwFlags = 0;
    CString strFilePath = strRelativePath + cFind.GetFileName ();
    sHeader.m_dwPathLength = strFilePath.GetLength () + 1;
    sHeader.m_dwFileLength = cFind.GetLength ();
    VERIFY ( m_MapViewConnection.SendBlock ( &sHeader, sizeof ( sHeader ), TRUE ) );
    VERIFY ( m_MapViewConnection.SendBlock ( (LPCSTR)strFilePath, sHeader.m_dwPathLength ) );

    {
      // Allocate the buffer for the file
      BYTE * pBuffer = new BYTE [ sHeader.m_dwFileLength ];

      // Read it from the file
      CArchiveFile cFile = CDataArchive::GetRootArchive ()->CreateFile ( cFind.GetFilePath () );
      cFile.Read ( pBuffer, sHeader.m_dwFileLength );

      // Send it
      VERIFY ( m_MapViewConnection.SendBlock ( pBuffer, sHeader.m_dwFileLength, TRUE ) );

      delete pBuffer;
    }
    VERIFY ( m_MapViewConnection.EndSendCompoundBlock () );
  }

  // And that's all
}
