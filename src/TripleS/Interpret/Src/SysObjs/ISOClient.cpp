#include "stdafx.h"
#include "..\..\Interpret.h"
#include "ISOClient.h"

#include "Network\Network\NetworkServer.h"
#include "GameServer\Civilization\Src\ZConnectedClient.h"
#include "GameServer\Civilization\ZCivilization.h"
#include "TripleS\Interpret\Src\SysObjs\ISOString.h"
#include "TripleS\Interpret\Src\SysObjs\ISOPosition.h"

#include "GameServer\GameServer\SUnit.h"
#include "GameServer\Civilization\Src\ZUnit.h"

#include "..\Syscalls\ISOClientAskForPositionSyscall.h"
#include "..\Syscalls\ISOClientAskForUnitSyscall.h"

BEGIN_SYSCALL_INTERFACE_MAP(CISOClient)
  SYSCALL_METHOD(M_GetUserLogin),  // 0: String GetUserLogin()
  SYSCALL_METHOD(M_IsLoggedOn),    // 1: bool IsLoggedOn()

  SYSCALL_METHOD(M_MessageInfo),           // 2: void MessageInfo(String)
  SYSCALL_METHOD(M_MessageInfoLocation),   // 3: void MessageInfoLocation(String, Position)
  SYSCALL_METHOD(M_MessageInfoUnit),       // 4: void MessageInfo(String, Unit)

  SYSCALL_METHOD(M_MessageWarning),           // 5: void MessageWarning(String)
  SYSCALL_METHOD(M_MessageWarningLocation),   // 6: void MessageWarningLocation(String, Position)
  SYSCALL_METHOD(M_MessageWarningUnit),       // 7: void MessageWarning(String, Unit)

  SYSCALL_METHOD(M_MessageError),           // 8: void MessageError(String)
  SYSCALL_METHOD(M_MessageErrorLocation),   // 9: void MessageErrorLocation(String, Position)
  SYSCALL_METHOD(M_MessageErrorUnit),       // 10: void MessageError(String, Unit)

  SYSCALL_METHOD(M_CloseUserInput),    // 11: void CloseUserInput()

  SYSCALL_METHOD(M_AskForPosition),    // 12: bool AskForPosition(String strQuestion, Position posResult);
  SYSCALL_METHOD(M_AskForEnemyUnit),   // 13: Unit AskForEnemyUnit(String strQuestion);
  SYSCALL_METHOD(M_AskForMyUnit),      // 14: Unit AskForMyUnit(String strQuestion);
  SYSCALL_METHOD(M_AskForAnyUnit),     // 15: Unit AskForAnyUnit(String strQuestion);

END_SYSCALL_INTERFACE_MAP()

CISOClient::CISOClient()
{
  // Initialize the object type
  m_nSOType = SO_CLIENT;

  // NULL our object
  m_pConnectedClient = NULL;
  m_pNext = NULL;
  m_pCivilization = NULL;
  m_bActive = FALSE;
}

CISOClient::~CISOClient()
{
  ASSERT(m_pConnectedClient == NULL);
  ASSERT(m_pCivilization == NULL);
}

// SAVE
void CISOClient::PersistentSave( CPersistentStorage &storage )
{
  BRACE_BLOCK(storage);
    
  CISystemObject::PersistentSave(storage);

  // First we must say that this object can't be saved
  // Cause the client is not connected while saving
  // So we'll save NULL pointer ....
  ASSERT(m_pConnectedClient == NULL);

  // And hence we don't need to save the NULL pointer, cause we know
  // it will be NULL at load time :-)

  // Save pointer to Civilization
  storage << m_pCivilization;
}

// LOAD
void CISOClient::PersistentLoad( CPersistentStorage &storage )
{
  BRACE_BLOCK(storage);
  
  CISystemObject::PersistentLoad(storage);

  // Fill the client with NULL
  m_pConnectedClient = NULL;

  // Load the civilization
  void *pCiv;
  storage >> pCiv;
  m_pCivilization = (CZCivilization *)pCiv;
}

void CISOClient::PersistentTranslatePointers( CPersistentStorage &storage )
{
    CISystemObject::PersistentTranslatePointers(storage);  
    
    // Translate the civilization pointer
    m_pCivilization = (CZCivilization *)storage.TranslatePointer(m_pCivilization);
}

void CISOClient::PersistentInit()
{
    CISystemObject::PersistentInit();

    // Here do nothing
}

  
// Creation
bool CISOClient::Create( CInterpret *pInterpret )
{
  return CISystemObject::Create( pInterpret );
}

// Delete
void CISOClient::Delete( )
{
    if ( g_bGameLoading) return;

  // First lock the ISOClients lock on the civilization
  m_pCivilization->LockISOClients();

  // Then lock myself
  VERIFY(m_lockLock.Lock());

  if((m_pConnectedClient != NULL) && (m_dwProcessID != 0))
  {
    if(m_bActive){
      // Stop this action on the client
      // it means tell the fact we're done to our connected client
      // We have to use the event to serialize this events
      m_pConnectedClient->InlayEvent ( CZConnectedClient::E_ActionDone, 0, m_pConnectedClient );

      m_bActive = FALSE;
    }

    // Unregister us from the connected client
    m_pConnectedClient->UnregisterISOClient( this );

    // Forget the connected client
    m_pConnectedClient = NULL;
  }

  VERIFY(m_lockLock.Unlock());

  m_pCivilization->UnlockISOClients();

  m_pCivilization = NULL;

  CISystemObject::Delete();
}

// Sets the object
void CISOClient::Set(CZConnectedClient *pConnectedClient, CZCivilization *pCivilization)
{
  ASSERT(m_pConnectedClient == NULL);

  // Just copy the connected client
  m_pConnectedClient = pConnectedClient;
  m_pCivilization = pCivilization;

  m_bActive = TRUE;
}

// Load data member
bool CISOClient::LoadMember( CIDataStackItem& DSI, long nID)
{
  VERIFY(m_lockLock.Lock());
  if(m_pConnectedClient == NULL){
    VERIFY(m_lockLock.Unlock());
    return false;
  }
  VERIFY(m_lockLock.Unlock());

  return false;
}

// Store data member
bool CISOClient::StoreMember( CIDataStackItem *pDSI, long nID)
{
  VERIFY(m_lockLock.Lock());
  if(m_pConnectedClient == NULL){
    VERIFY(m_lockLock.Unlock());
    return false;
  }
  VERIFY(m_lockLock.Unlock());

  return false;
}


// Returns user login name
ESyscallResult CISOClient::M_GetUserLogin( CIProcess *pProcess, CISyscall **ppSyscall,
                                           CIDataStackItem *pRet )
{
  VERIFY(m_lockLock.Lock());
  if(m_pConnectedClient == NULL){
    VERIFY(m_lockLock.Unlock());
    return SYSCALL_ERROR;
  }

  // Create the string object to return the name in
  CISOString *pString = (CISOString *)pProcess->m_pInterpret->CreateSystemObject(SO_STRING);
  if(pString == NULL){
    VERIFY(m_lockLock.Unlock());
    return SYSCALL_ERROR;
  }
  pString->Set( m_pConnectedClient->GetUserName() );

  // Set it as return value
  pRet->Set( pString );

  // release it
  pString->Release();

  VERIFY(m_lockLock.Unlock());

  // And that's all
  return SYSCALL_SUCCESS;
}

// Returns TRUE if the user is logged on
ESyscallResult CISOClient::M_IsLoggedOn( CIProcess *pProcess, CISyscall **ppSyscall,
                                         CIDataStackItem *pRet )
{
  VERIFY(m_lockLock.Lock());

  if(m_pConnectedClient == NULL){
    pRet->Set(false);
  }
  else{
    pRet->Set(true);
  }

  VERIFY(m_lockLock.Unlock());

  return SYSCALL_SUCCESS;
}

// Displays info message on client
ESyscallResult CISOClient::M_MessageInfo( CIProcess *pProcess, CISyscall **ppSyscall,
                                          CISystemObject *pText )
{
  if(pText->GetObjectType() != SO_STRING) return SYSCALL_ARGUMENT_ERROR;

  VERIFY(m_lockLock.Lock());

  if(m_pConnectedClient != NULL){
    CISOString *pStrText = (CISOString *)pText;

    m_pConnectedClient->SendMessage( pStrText->Get(), CZConnectedClient::MsgType_Info );
  }

  VERIFY(m_lockLock.Unlock());

  return SYSCALL_SUCCESS;
}

// Displays info message on client with location
ESyscallResult CISOClient::M_MessageInfoLocation( CIProcess *pProcess, CISyscall **ppSyscall,
                                                  CISystemObject *pText, CISystemObject *pPos )
{
  if(pText->GetObjectType() != SO_STRING) return SYSCALL_ARGUMENT_ERROR;
  if(pPos->GetObjectType() != SO_POSITION) return SYSCALL_ARGUMENT_ERROR;

  VERIFY(m_lockLock.Lock());

  if(m_pConnectedClient != NULL){
    CISOString *pStrText = (CISOString *)pText;
    CISOPosition *pPosition = (CISOPosition *)pPos;

    m_pConnectedClient->SendMessage( pStrText->Get(), CZConnectedClient::MsgType_Info,
      pPosition->GetXPosition(), pPosition->GetYPosition() );
  }

  VERIFY(m_lockLock.Unlock());

  return SYSCALL_SUCCESS;
}

// Displays info message on client with unit
ESyscallResult CISOClient::M_MessageInfoUnit( CIProcess *pProcess, CISyscall **ppSyscall,
                                              CISystemObject *pText, CZUnit *pUnit )
{
  if(pText->GetObjectType() != SO_STRING) return SYSCALL_ARGUMENT_ERROR;

  VERIFY(m_lockLock.Lock());

  if(m_pConnectedClient != NULL){
    CISOString *pStrText = (CISOString *)pText;

    m_pConnectedClient->SendMessage( pStrText->Get(), CZConnectedClient::MsgType_Info,
      pUnit->GetSUnit()->GetID() );
  }

  VERIFY(m_lockLock.Unlock());

  return SYSCALL_SUCCESS;
}

// Displays warning message on client
ESyscallResult CISOClient::M_MessageWarning( CIProcess *pProcess, CISyscall **ppSyscall,
                                          CISystemObject *pText )
{
  if(pText->GetObjectType() != SO_STRING) return SYSCALL_ARGUMENT_ERROR;

  VERIFY(m_lockLock.Lock());

  if(m_pConnectedClient != NULL){
    CISOString *pStrText = (CISOString *)pText;

    m_pConnectedClient->SendMessage( pStrText->Get(), CZConnectedClient::MsgType_Warning );
  }

  VERIFY(m_lockLock.Unlock());

  return SYSCALL_SUCCESS;
}

// Displays warning message on client with location
ESyscallResult CISOClient::M_MessageWarningLocation( CIProcess *pProcess, CISyscall **ppSyscall,
                                                  CISystemObject *pText, CISystemObject *pPos )
{
  if(pText->GetObjectType() != SO_STRING) return SYSCALL_ARGUMENT_ERROR;
  if(pPos->GetObjectType() != SO_POSITION) return SYSCALL_ARGUMENT_ERROR;

  VERIFY(m_lockLock.Lock());

  if(m_pConnectedClient != NULL){
    CISOString *pStrText = (CISOString *)pText;
    CISOPosition *pPosition = (CISOPosition *)pPos;

    m_pConnectedClient->SendMessage( pStrText->Get(), CZConnectedClient::MsgType_Warning,
      pPosition->GetXPosition(), pPosition->GetYPosition() );
  }

  VERIFY(m_lockLock.Unlock());

  return SYSCALL_SUCCESS;
}

// Displays warning message on client with unit
ESyscallResult CISOClient::M_MessageWarningUnit( CIProcess *pProcess, CISyscall **ppSyscall,
                                              CISystemObject *pText, CZUnit *pUnit )
{
  if(pText->GetObjectType() != SO_STRING) return SYSCALL_ARGUMENT_ERROR;

  VERIFY(m_lockLock.Lock());

  if(m_pConnectedClient != NULL){
    CISOString *pStrText = (CISOString *)pText;

    m_pConnectedClient->SendMessage( pStrText->Get(), CZConnectedClient::MsgType_Warning,
      pUnit->GetSUnit()->GetID() );
  }

  VERIFY(m_lockLock.Unlock());

  return SYSCALL_SUCCESS;
}

// Displays error message on client
ESyscallResult CISOClient::M_MessageError( CIProcess *pProcess, CISyscall **ppSyscall,
                                          CISystemObject *pText )
{
  if(pText->GetObjectType() != SO_STRING) return SYSCALL_ARGUMENT_ERROR;

  VERIFY(m_lockLock.Lock());

  if(m_pConnectedClient != NULL){
    CISOString *pStrText = (CISOString *)pText;

    m_pConnectedClient->SendMessage( pStrText->Get(), CZConnectedClient::MsgType_Error );
  }

  VERIFY(m_lockLock.Unlock());

  return SYSCALL_SUCCESS;
}

// Displays error message on client with location
ESyscallResult CISOClient::M_MessageErrorLocation( CIProcess *pProcess, CISyscall **ppSyscall,
                                                  CISystemObject *pText, CISystemObject *pPos )
{
  if(pText->GetObjectType() != SO_STRING) return SYSCALL_ARGUMENT_ERROR;
  if(pPos->GetObjectType() != SO_POSITION) return SYSCALL_ARGUMENT_ERROR;

  VERIFY(m_lockLock.Lock());

  if(m_pConnectedClient != NULL){
    CISOString *pStrText = (CISOString *)pText;
    CISOPosition *pPosition = (CISOPosition *)pPos;

    m_pConnectedClient->SendMessage( pStrText->Get(), CZConnectedClient::MsgType_Error,
      pPosition->GetXPosition(), pPosition->GetYPosition() );
  }

  VERIFY(m_lockLock.Unlock());

  return SYSCALL_SUCCESS;
}

// Displays error message on client with unit
ESyscallResult CISOClient::M_MessageErrorUnit( CIProcess *pProcess, CISyscall **ppSyscall,
                                              CISystemObject *pText, CZUnit *pUnit )
{
  if(pText->GetObjectType() != SO_STRING) return SYSCALL_ARGUMENT_ERROR;

  VERIFY(m_lockLock.Lock());

  if(m_pConnectedClient != NULL){
    CISOString *pStrText = (CISOString *)pText;

    m_pConnectedClient->SendMessage( pStrText->Get(), CZConnectedClient::MsgType_Error,
      pUnit->GetSUnit()->GetID() );
  }

  VERIFY(m_lockLock.Unlock());

  return SYSCALL_SUCCESS;
}

// Stops client input
// And makes the object invalid
ESyscallResult CISOClient::M_CloseUserInput( CIProcess *pProcess, CISyscall **ppSyscall )
{
  VERIFY(m_lockLock.Lock());

  if((m_pConnectedClient != NULL) && (m_bActive)){
    // Send the event
    m_pConnectedClient->InlayEvent ( CZConnectedClient::E_ActionDone, 0, m_pConnectedClient );
    m_bActive = FALSE;
  }

  VERIFY(m_lockLock.Unlock());

  return SYSCALL_SUCCESS;
}

// Asks the user for position
ESyscallResult CISOClient::M_AskForPosition ( CIProcess *pProcess, CISyscall **ppSyscall, CIDataStackItem *pRet, CISystemObject *pQuestion, CISystemObject *pPosition )
{
  if( pPosition == NULL ) return SYSCALL_ARGUMENT_ERROR;
  if( pQuestion->GetObjectType() != SO_STRING ) return SYSCALL_ARGUMENT_ERROR;

  VERIFY(m_lockLock.Lock());

  if((m_pConnectedClient == NULL) || (m_bActive == FALSE)){
    VERIFY(m_lockLock.Unlock());
    return SYSCALL_ERROR;
  }

  CISOPosition *pPosPosition = (CISOPosition *)pPosition;
  // Create new syscall
  CISOClientAskForPositionSyscall *pSyscall = new CISOClientAskForPositionSyscall();
  pSyscall->Create ( pProcess, pRet, pPosPosition );
  *ppSyscall = pSyscall;

  CISOString *pStrQuestion = (CISOString *)pQuestion;
  m_pConnectedClient->AskForPosition ( pStrQuestion->Get(), pSyscall );

  VERIFY(m_lockLock.Unlock());

  return SYSCALL_SUCCESS_SLEEP;
}

// Asks the user for enemy unit
ESyscallResult CISOClient::M_AskForEnemyUnit ( CIProcess *pProcess, CISyscall **ppSyscall, CIDataStackItem *pRet, CISystemObject *pQuestion )
{
  if(pQuestion->GetObjectType() != SO_STRING) return SYSCALL_ARGUMENT_ERROR;

  VERIFY(m_lockLock.Lock());

  if((m_pConnectedClient == NULL) || (m_bActive == FALSE)){
    VERIFY(m_lockLock.Unlock());
    return SYSCALL_ERROR;
  }

  CISOClientAskForUnitSyscall *pSyscall = new CISOClientAskForUnitSyscall();
  pSyscall->Create( pProcess, pRet );
  *ppSyscall = pSyscall;

  CISOString *pStrQuestion = (CISOString *)pQuestion;
  m_pConnectedClient->AskForUnit ( pStrQuestion->Get(), CISOClientAskForUnitSyscall::UnitType_Enemy, pSyscall );

  VERIFY(m_lockLock.Unlock());

  return SYSCALL_SUCCESS_SLEEP;
}

// Asks the user for my unit
ESyscallResult CISOClient::M_AskForMyUnit ( CIProcess *pProcess, CISyscall **ppSyscall, CIDataStackItem *pRet, CISystemObject *pQuestion )
{
  if(pQuestion->GetObjectType() != SO_STRING) return SYSCALL_ARGUMENT_ERROR;

  VERIFY(m_lockLock.Lock());

  if((m_pConnectedClient == NULL) || (m_bActive == FALSE)){
    VERIFY(m_lockLock.Unlock());
    return SYSCALL_ERROR;
  }

  CISOClientAskForUnitSyscall *pSyscall = new CISOClientAskForUnitSyscall();
  pSyscall->Create( pProcess, pRet );
  *ppSyscall = pSyscall;

  CISOString *pStrQuestion = (CISOString *)pQuestion;
  m_pConnectedClient->AskForUnit ( pStrQuestion->Get(), CISOClientAskForUnitSyscall::UnitType_My, pSyscall );

  VERIFY(m_lockLock.Unlock());

  return SYSCALL_SUCCESS_SLEEP;
}

// Asks the user for any unit
ESyscallResult CISOClient::M_AskForAnyUnit ( CIProcess *pProcess, CISyscall **ppSyscall, CIDataStackItem *pRet, CISystemObject *pQuestion )
{
  if(pQuestion->GetObjectType() != SO_STRING) return SYSCALL_ARGUMENT_ERROR;

  VERIFY(m_lockLock.Lock());

  if((m_pConnectedClient == NULL) || (m_bActive == FALSE)){
    VERIFY(m_lockLock.Unlock());
    return SYSCALL_ERROR;
  }

  CISOClientAskForUnitSyscall *pSyscall = new CISOClientAskForUnitSyscall();
  pSyscall->Create( pProcess, pRet );
  *ppSyscall = pSyscall;

  CISOString *pStrQuestion = (CISOString *)pQuestion;
  m_pConnectedClient->AskForUnit ( pStrQuestion->Get(), CISOClientAskForUnitSyscall::UnitType_Any, pSyscall );
  
  VERIFY(m_lockLock.Unlock());

  return SYSCALL_SUCCESS_SLEEP;
}

