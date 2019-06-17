// ZConnectedClient.h: interface for the CZConnectedClient class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ZCONNECTEDCLIENT_H__AB144881_0220_11D4_8480_004F4E0004AA__INCLUDED_)
#define AFX_ZCONNECTEDCLIENT_H__AB144881_0220_11D4_8480_004F4E0004AA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ZScriptEditor.h"
#include "ZLogOutput.h"
#include "MapDownload\ZMapDownload.h"
// *ROMAN v--v
#include "ClientCommunication\ZClientUnitInfoSender.h"
// *ROMAN ^--^

#include "TripleS\Interpret\Src\SysObjs\ISOClient.h"
#include "TripleS\Interpret\Src\Syscalls\ISOClientAskForPositionSyscall.h"
#include "TripleS\Interpret\Src\Syscalls\ISOClientAskForUnitSyscall.h"

class CZCivilization;

// Represents one connected client
// It takes care about all the things around the client
class CZConnectedClient : public CNotifier
{
  DECLARE_DYNAMIC(CZConnectedClient);
  DECLARE_OBSERVER_MAP(CZConnectedClient);

public:
// Constructor & destructor
	CZConnectedClient();
	virtual ~CZConnectedClient();

// Debug functions
#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

// Creation
  // Creates the object
  void Create(CString strUserName, CZCivilization *pCivilization);
  // Deletes the object
  virtual void Delete();

  // returns teh user name
  CString GetUserName(){ ASSERT_VALID(this); return m_strUserName; }

  enum{ E_CLOSE = 1,
    E_DisconnectClient = 0x1000,
    E_ActionDone = 2, // Here is a small hack, this one is sent from us to us
                      // but it's don eby our ISOClients
  };

  // Unregisters the ISO client
  // This function can be run only under the civilization's ISOClients lock
  void UnregisterISOClient( CISOClient *pISOClient );

  // Sends info about the interactive action to the client
  void ActionDone();

  typedef enum{
    MsgType_User,
    MsgType_Info,
    MsgType_Warning,
    MsgType_Error,
    MsgType_Admin,
  } EMessageType;

  // Sends a message to client
  void SendMessage( CString strText, EMessageType eType ){ InternalSendMessage(strText, eType, 0x0F0000000, 0x0F0000000, 0x0FFFFFFFF); }
  void SendMessage( CString strText, EMessageType eType, DWORD dwXPos, DWORD dwYPos ){ InternalSendMessage(strText, eType, dwXPos, dwYPos, 0x0FFFFFFFF); }
  void SendMessage( CString strText, EMessageType eType, DWORD dwUnitID ){ InternalSendMessage(strText, eType, 0x0F0000000, 0x0F0000000, dwUnitID); }

  // Asks user for position
  void AskForPosition( CString strQuestion, CISOClientAskForPositionSyscall *pSyscall );
  // Asks user for unit
  void AskForUnit( CString strQuestion, CISOClientAskForUnitSyscall::EUnitType eUnitType, CISOClientAskForUnitSyscall *pSyscall );

protected:
  // Reactions on the network events
  void OnNewClientConnected(CNetworkConnection *pConnection);
  void OnNetworkError();
  void OnNormalClose();
  void OnNewVirtualConnection(CNetworkConnection::SNewVirtualConnectionUserData *pUserData);

  void OnDisconnectClient ( CEvent * pEvent );

private:
  // pointer to our civilization
  CZCivilization *m_pCivilization;
	
  // User name for whom we exist
  CString m_strUserName;


  // The network event thread
  CWinThread *m_pNetworkEventThread;

  // The network event thread functions
  static UINT _NetworkEventThread(LPVOID pParam);
  void NetworkEventThread();
	// network EM
  CEventManager *m_pNetworkEventManager;
  CEvent m_eventNetworkEventThreadStarted;

  // pointer to the connection
  CNetworkConnection *m_pConnection;

  enum{ ID_NetworkConnection = 0x0100,
    ID_Self = 0x0101,
    ID_Civilization = 0x0200, };

  // the script editor virtual connection end
  CZScriptEditor m_ScriptEditor;
  // the log output
  CZLogOutput m_LogOutput;
  // the map dowloader
  CZMapDownload m_MapDownload;

  // *ROMAN v--v
  CZClientUnitInfoSender m_UnitInfoSender;
  // *ROMAN ^--^

//-------------------------------------------
// Control connection
  // The virt. connection
  CVirtualConnection m_ControlConnection;
  // Reaction on its events
  void OnControlConnectionPacketAvailable();

  enum{ ID_ControlConnection = 0x0101, };

  class CControlConnectionObserver : public CObserver
  {
    DECLARE_OBSERVER_MAP(CControlConnectionObserver);

  public:
    CControlConnectionObserver(){};
    virtual ~CControlConnectionObserver(){};

    CZConnectedClient *m_pConnectedClient;

    void OnPacketAvailable(){ m_pConnectedClient->OnControlConnectionPacketAvailable(); }
  };
  friend CControlConnectionObserver;
  CControlConnectionObserver m_ControlConnectionObserver;


//-------------------------------------------
// MapView connection
  // The virt. connection
  CVirtualConnection m_MapViewConnection;
  // Reaction on its events
  void OnMapViewConnectionPacketAvailable();

  enum{ ID_MapViewConnection = 0x0102, };

  class CMapViewConnectionObserver : public CObserver
  {
    DECLARE_OBSERVER_MAP(CMapViewConnectionObserver);

  public:
    CMapViewConnectionObserver(){};
    virtual ~CMapViewConnectionObserver(){};

    CZConnectedClient *m_pConnectedClient;

    void OnPacketAvailable(){ m_pConnectedClient->OnMapViewConnectionPacketAvailable(); }
  };
  friend CMapViewConnectionObserver;
  CMapViewConnectionObserver m_MapViewConnectionObserver;

  // Process in which the global function is running
  DWORD m_dwGlobalFunctionProcessID;

  // Lock for accesing the mapview connection
  CMutex m_lockMapViewConnection;

  // Send whole directory to the client (used for toolbars download)
  void SendToolbarFiles ( CString strDirectory, CString strRelativePath );

// Locking hierarchy:
//   1. ISOClients on civilization
//   2. m_lockLock on the CISOClient

// the MapViewConnection must not be locked with any of the above ones

//-------------------------------------------
// ISOClient objects
  // List of ISOClient objects
  // This list has its lock on the civilization (ISOClients)
  CISOClient *m_pISOClients;

  // Send message to client
  void InternalSendMessage(CString strText, EMessageType eType, DWORD dwXPos, DWORD dwYPos, DWORD dwUnitID);


//-------------------------------------------
// Question in progress
  // Syscall for position question
  CISOClientAskForPositionSyscall *m_pQuestionPositionSyscall;
  // Syscall for unit question
  CISOClientAskForUnitSyscall *m_pQuestionUnitSyscall;

//-------------------------------------------
// UnitHierarchy connection
  // The connection
  CVirtualConnection m_UnitHierarchyConnection;

  // Node for building unit hierarchy
  struct SUnitHierarchyNode
  {
    CZUnit * m_pUnit;
    struct SUnitHierarchyNode * m_pCommander;
    struct SUnitHierarchyNode * m_pFirstInferior;
    struct SUnitHierarchyNode * m_pNextSibling;
  };

  // Reaction on arriving packet
  void OnUnitHierarchyConnectionPacketAvailable();

  // Helper functions
  // Builds unit hierarchy for given node
  void BuildUnitHierarchyNodeRecursive ( SUnitHierarchyNode * pNode, CTypedMemoryPool < SUnitHierarchyNode > * pPool );
  // Sends unit records for givven unit node and also for all children units
  // Called recursively
  void SendUnitHierarchyRecordRecursive( SUnitHierarchyNode * pNode, CTypedMemoryPool < SUnitHierarchyNode > * pPool  );

  enum{ ID_UnitHierarchyConnection = 0x0103, };

  // the observer class for it
  class CUnitHierarchyConnectionObserver : public CObserver
  {
    DECLARE_OBSERVER_MAP(CUnitHierarchyConnectionObserver);

  public:
    CUnitHierarchyConnectionObserver(){};
    ~CUnitHierarchyConnectionObserver(){};

    CZConnectedClient *m_pConnectedClient;

    void OnPacketAvailable(){ m_pConnectedClient->OnUnitHierarchyConnectionPacketAvailable(); }
  };
  friend CUnitHierarchyConnectionObserver;
  CUnitHierarchyConnectionObserver m_UnitHierarchyConnectionObserver;

  // next item in the connected client list
  CZConnectedClient *m_pNext;

  friend class CZCivilization;
};

#endif // !defined(AFX_ZCONNECTEDCLIENT_H__AB144881_0220_11D4_8480_004F4E0004AA__INCLUDED_)
