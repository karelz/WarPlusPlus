// NetworkServer.h: interface for the CNetworkServer class
//////////////////////////////////////////////////////////////////////

#if !defined(_NETWORKSERVER__HEADER_INCLUDED_)
#define _NETWORKSERVER__HEADER_INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "NetworkException.h"
#include "NetworkEvents.h"
#include "Network.h"
#include "PacketTypes.h"
#include "VirtualConnection.h"
#include "NetworkConnection.h"
#include "VirtualConnection.inl"

// define fictive class CLock
#ifdef _DEBUG
#define CLock CSemaphore
#else
#define CLock CCriticalSection
#endif

//////////////////////////////////////////////////////////////////////
// Server class
//////////////////////////////////////////////////////////////////////

class CNetworkServer : public CNetwork  
{
    DECLARE_DYNAMIC(CNetworkServer)

public:
    enum ELoginStatus
    {
        Login_Failed = 0,
        Login_BadPassword = 0x000000001,
        Login_UnknownUser = 0x000000002,
        Login_AlreadyLoggedOn = 0x000000003,
        Login_GameNotRunning = 0x000000004,
        Login_UserDisabled = 0x000000005,
    };

public:
    // constructor
    CNetworkServer() : 
        m_pFirstConnection(NULL), 
        m_bServerIsRunning(FALSE),
        m_eventAllClientsDisconected(TRUE, TRUE),  // initially owned, manual
        m_TCPServerSocket(INVALID_SOCKET)
    { }
    // destructor
    virtual ~CNetworkServer();

    // methods Create() and Delete() are inherited w/o any changes

    // create listening socket on given port (called during initialization of server)
    virtual void PrepareServer(WORD wServerPort, CObserver * (* pLoginFunction)(CString strLogin, CString strPassword));
    // wait for client connection (called by civilizations)

    // wait for clients disconnect
    virtual void WaitForAllClientsDisconnect();
    
    // close server
    virtual BOOL CloseServer(BOOL bForcedClose = FALSE);

#ifdef _DEBUG
    // check content of object
    virtual void AssertValid() const;
    // dump content of object
    virtual void Dump(CDumpContext &dc) const;
#endif

protected:

    // indicates the status of server
    BOOL m_bServerIsRunning;

    // main server TCP socket
    SOCKET m_TCPServerSocket;

    // logout function (called by CNetworkConnection)
    virtual void Logout(CNetworkConnection *p);

	// this method deletes one network connection from the link list of all network connections
    void DeleteConnection(CNetworkConnection * pConnection);
    
    // list of clients
    CNetworkConnection *m_pFirstConnection;
    // lock for number of connected clients
    CLock m_lockConnectedClients;

    // login function
    CObserver * (* m_pLoginFunction)(CString strLogin, CString strPassword);
    // lock for login function (it doesn't need to be reentrant)
    CLock m_lockLoginFunction;

    // event is activated when the last client has disconnected
    CEvent m_eventAllClientsDisconected;

    // thread function -- accepting client connections
    static UINT NetworkServerAcceptThread(LPVOID pParam);
    // accepter thread function killer
    CEvent m_eventAccepterKiller;
    // accept possible on main TCP socket
    CEvent m_eventAcceptPossible;
    // handle to accepter thread
    CWinThread *m_pAccepterThread;

    // thread function -- login thread
    static UINT NetworkServerLoginThread(LPVOID pParam);

    struct SLoginData
    {
        CNetworkServer *pServer;
        SOCKET sock;
        sockaddr addr;
    };
};

// undefine CLock
#undef CLock


#endif // !defined(_NETWORKSERVER__HEADER_INCLUDED_)
