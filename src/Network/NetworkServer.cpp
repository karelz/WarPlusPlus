// NetworkServer.cpp: implementation of the CNetworkServer class
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NetworkServer.h"

IMPLEMENT_DYNAMIC(CNetworkServer, CNetwork)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// empty constructor is inline

CNetworkServer::~CNetworkServer()
{
    // assert the server is not running
    // if this fails, you forgot to call CloseServer() before Delete()
    ASSERT(!m_bServerIsRunning);
}


//////////////////////////////////////////////////////////////////////
// Prepare server to accept client connections
//////////////////////////////////////////////////////////////////////

void CNetworkServer::PrepareServer(WORD wServerPort, CObserver * (* pLoginFunction)(CString strLogin, CString strPassword))
{
    // check the state of the object
#ifdef _DEBUG
    CNetwork::AssertValid();
#endif 
    ASSERT(m_bServerIsRunning == FALSE);
    ASSERT(m_TCPServerSocket == INVALID_SOCKET);
    
    // check parameters
    ASSERT(pLoginFunction != NULL);
    ASSERT(wServerPort >= 1000);

    // create TCP socket for control connection
    m_TCPServerSocket = socket(AF_INET, SOCK_STREAM, 0);
    
    // if there is some problem
    if (m_TCPServerSocket == INVALID_SOCKET)
	{
        int nError = WSAGetLastError();
		TRACE_NETWORK("CNetworkServer::PrepareServer() - socket() error %i\n", nError);
        throw new CNetworkException(CNetworkException::eCreateSocketError, nError, _T(__FILE__), __LINE__);
	}
    // socket successfuly created

    // prepare address
    sockaddr addr;
    memset(&addr, 0, sizeof(addr));
    ((sockaddr_in *) &addr)->sin_family = AF_INET;
    ((sockaddr_in *) &addr)->sin_addr.s_addr = htonl(INADDR_ANY);
    ((sockaddr_in *) &addr)->sin_port = htons(wServerPort);
    
    // bind socket to local port
    if (bind(m_TCPServerSocket, &addr, sizeof(addr)) == SOCKET_ERROR)
    {
        int nError = WSAGetLastError();
		TRACE_NETWORK("CNetworkServer::PrepareServer() - bind() error %i\n", nError);
        closesocket(m_TCPServerSocket);
        throw new CNetworkException(CNetworkException::eBindError, nError, _T(__FILE__), __LINE__);
    }

    // make socket listen on given port
    if (listen(m_TCPServerSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        int nError = WSAGetLastError();
		TRACE_NETWORK("CNetworkServer::PrepareServer() - listen() error %i\n", nError);
        closesocket(m_TCPServerSocket);
        throw new CNetworkException(CNetworkException::eListenError, nError, _T(__FILE__), __LINE__);
    }

    // copy pointer to login function
    m_pLoginFunction = pLoginFunction;

    // consider server as running NOW
    m_bServerIsRunning = TRUE;
    
    TRACE_NETWORK("CNetworkServer::PrepareServer() - server is listening on port %i\n", wServerPort);
    
    // start accepter thread function
    m_pAccepterThread = ::AfxBeginThread(NetworkServerAcceptThread, this, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
    if (m_pAccepterThread == NULL)
    {
        TRACE_NETWORK("CNetworkServer::PrepareServer() - cannot create new thread!!!\n");
        ASSERT(FALSE);
        throw 0;  // fatal error
    }
    m_pAccepterThread->m_bAutoDelete = FALSE;
    // TRACE_NETWORK("Thread 0x%X created (accepter)\n", m_pAccepterThread->m_nThreadID);
    m_pAccepterThread->ResumeThread();
}


//////////////////////////////////////////////////////////////////////
// Close server (and all connections to clients)
//////////////////////////////////////////////////////////////////////

BOOL CNetworkServer::CloseServer(BOOL bForcedClose)
{
    if (m_bServerIsRunning)
    {
        // object must be valid
        ASSERT_VALID(this);
        
        // lock the list of connected clients
        VERIFY(m_lockConnectedClients.Lock());
        
        if ((m_pFirstConnection != NULL) && !bForcedClose)
        {
            TRACE_NETWORK("CNetworkServer::CloseServer() - called in non-forced mode and there are still active connections\n");
            
            // unlock number of connected clients
            VERIFY(m_lockConnectedClients.Unlock());
            
            return FALSE;
        }
        
        // set killer event for accepter thread
        VERIFY(m_eventAccepterKiller.SetEvent());
        // wait for accepter's death...
        switch (WaitForSingleObject(*m_pAccepterThread, INFINITE))
        {
        case WAIT_FAILED:
            TRACE_NETWORK("CNetworkServer::CloseServer() - WaitForSingleObject() failed, error %i\n", GetLastError());
            ASSERT(FALSE);
            break;
            
        case WAIT_OBJECT_0:
            // OK
            break;
            
        default:
            TRACE_NETWORK("CNetworkServer::CloseServer() - unexpected WaitForSingleObject() return value\n");
            ASSERT(FALSE);
        }
        delete m_pAccepterThread;
        
        // consider server as not running NOW
        m_bServerIsRunning = FALSE;
        
        // set server socket to invalid (accepter has just closed it)
        m_TCPServerSocket = INVALID_SOCKET;
        
        {
            CNetworkConnection *pDelConnection;
            
            while(m_pFirstConnection != NULL)
            {
                // copy pointer to the deleted item
                pDelConnection = m_pFirstConnection;
                
                // delete item from the list
                m_pFirstConnection = m_pFirstConnection->m_pNextConnection;
                
                // close connection (forced disconnect)
                pDelConnection->Delete(0);
                
                // delete the connection
                delete pDelConnection;
            }
        }
        
        // unlock number of connected clients
        VERIFY(m_lockConnectedClients.Unlock());
        
        // if there is some WaitForAllClientsDisconnect() call, release it
        // by setting an event
        VERIFY(m_eventAllClientsDisconected.SetEvent());
        
        return TRUE;
    }
    return FALSE;
}


//////////////////////////////////////////////////////////////////////
// Wait for all clients disconnected
//////////////////////////////////////////////////////////////////////

void CNetworkServer::WaitForAllClientsDisconnect()
{
    // object must be valid
    ASSERT_VALID(this);

    // kill accepter thread
    VERIFY(m_eventAccepterKiller.SetEvent());
    // wait for it
    switch (WaitForSingleObject(*m_pAccepterThread, INFINITE))
    {
    case WAIT_FAILED:
        TRACE_NETWORK("CNetworkServer::WaitForAllClientsDisconnect() - WaitForSingleObject() failed, error %i\n", GetLastError());
        ASSERT(FALSE);
        break;

    case WAIT_OBJECT_0:
        // OK
        break;

    default:
        TRACE_NETWORK("CNetworkServer::WaitForAllClientsDisconnect() - unexpected WaitForSingleObject() return value\n");
        ASSERT(FALSE);
    }

    // let the some time for login threads to finish...
    Sleep(1000);

    // wait for the last client disconnected
    VERIFY(m_eventAllClientsDisconected.Lock());

    // success
}


//////////////////////////////////////////////////////////////////////
// Logout function
//////////////////////////////////////////////////////////////////////

void CNetworkServer::Logout(CNetworkConnection *p)
{
    ASSERT_VALID(this);

    // notify the main server, that one client was disconnected
    InlayEvent(E_CLIENTDISCONNECTED, (DWORD) p->m_pObserver);

    // delete client from the link list of all client connections
    DeleteConnection(p);
}


//////////////////////////////////////////////////////////////////////
// Debug methods
//////////////////////////////////////////////////////////////////////

#ifdef _DEBUG

void CNetworkServer::AssertValid() const
{
    CNetwork::AssertValid();
    ASSERT(m_bServerIsRunning);
    ASSERT(m_pLoginFunction != NULL);
    ASSERT(m_TCPServerSocket != INVALID_SOCKET);
    ASSERT_KINDOF(CWinThread, m_pAccepterThread);
}

void CNetworkServer::Dump(CDumpContext &dc) const
{
    CNetwork::Dump(dc);

    dc << "CNetworkServer object:" << endl;
    if (m_bServerIsRunning)
        dc << "  server is running" << endl;
    else
        dc << "  server is not running" << endl;
}

#endif  // _DEBUG


//////////////////////////////////////////////////////////////////////
// Thread function -- accept client connections
//////////////////////////////////////////////////////////////////////

// return codes:
//    0 : normal exit
//    1 : network error
//    2 : wait error
//    3 : memory allocation error

UINT CNetworkServer::NetworkServerAcceptThread(LPVOID pParam)
{
    CNetworkServer *p = (CNetworkServer *) pParam;
    ASSERT_VALID(p);
    
    if (WSAEventSelect(p->m_TCPServerSocket, p->m_eventAcceptPossible, FD_ACCEPT) == SOCKET_ERROR)
    {
        TRACE_NETWORK("CNetworkServer::NetworkServerAcceptThread() - WSAEventSelect() error %i\n", WSAGetLastError());

        // send message to application
        p->InlayEvent(E_ACCEPTERERROR, 1);

        // abort thread
        return 1;
    }
    
    HANDLE aHandles[2];
    aHandles[0] = p->m_eventAccepterKiller;
    aHandles[1] = p->m_eventAcceptPossible;
    
    for ( ; ; )
    {
        switch (WaitForMultipleObjects(2, aHandles, FALSE, INFINITE))
        {
        case WAIT_FAILED:
            {
                // wait function error
                TRACE_NETWORK("CNetworkServer::NetworkServerAcceptThread() - WaitForMultipleObjects() error %i\n", GetLastError());

                // send message to application
                p->InlayEvent(E_ACCEPTERERROR, 2);

                // exit thread...
                return 2;
            }

        case WAIT_OBJECT_0:
            // killer activated
            {
                // close accepting socket
                closesocket(p->m_TCPServerSocket);
            }
            return 0;

        case WAIT_OBJECT_0 + 1:
            // accept possible
            {
                // prepare structure for login thread
                SLoginData *q;
                try
                {
                    q = new SLoginData;
                }
                catch (CMemoryException *)
                {
                    // memory allocation error
                    TRACE_NETWORK("CNetworkServer::NetworkServerAcceptThread() - memory allocation error\n");

                    // do nothing (try to survive)
                    break;
                }

                // clear the structure
                memset(q, 0, sizeof(SLoginData));

                // copy the pointer to the CNetworkServer object
                q->pServer = p;

                // prepare the length of the addres for accept() function
                int nAddressLength = sizeof(q->addr);

                // accept new connection (gain new socket)
                q->sock = accept(p->m_TCPServerSocket, &(q->addr), &nAddressLength);

                // check the result of accept
                if (q->sock == INVALID_SOCKET)
                {
                    // accept() error
                    TRACE_NETWORK("CNetworkServer::NetworkServerAcceptThread() - accept() error %i\n", WSAGetLastError());
                    
                    // delete temporary structure
                    delete q;

                    // there's nothing more to do...
                }
                else
                {
                    // TRACE_NETWORK("CNetworkServer::NetworkServerAcceptThread() - accepted socket %i\n", q->sock);
                    
                    // create temporary CWinThread pointer
                    // call login thread function to login user
                    CWinThread *pTemp = ::AfxBeginThread(NetworkServerLoginThread, q);
                    if (pTemp == NULL)
                    {
                        TRACE_NETWORK("CNetworkServer::NetworkServerAcceptThread() - cannot create new thread!!!\n");

                        // close socket -- client will report an error
                        closesocket(q->sock);
                        
                        // delete temporary structure
                        delete q;

                        // Can't do more. Client was disconnected, hopefuly
                        // the next client will be more successful in this
                        // logging process. Accepter thread continues his
                        // job...
                    }

                    // TRACE_NETWORK("CNetworkServer::NetworkServerAcceptThread() - login thread 0x%X created\n", pTemp->m_nThreadID);
                    
                    // that's all; login thread will delete the temporary structure
                }
            }
            break;

        default:
            // SHOULD NOT BE REACHED
            TRACE_NETWORK("CNetworkServer::NetworkServerAcceptThread() - WaitForMultipleObjects() unexpected return code\n");
            ASSERT(FALSE);
        } // switch
         
    } // endless loop

    // exit thread (should not be reached)
    return 0;
}


//////////////////////////////////////////////////////////////////////
// Thread function -- client login
//////////////////////////////////////////////////////////////////////

// return codes:
//    0 : normal exit
//    1 : network error
//    2 : challenge error
//    3 : login error
//    4 : can't create connection object
//    5 : memory allocation error

UINT CNetworkServer::NetworkServerLoginThread(LPVOID pParam)
{
    SLoginData *p = (SLoginData *) pParam;
    ASSERT(::AfxIsValidAddress(p, sizeof(SLoginData), FALSE));
    ASSERT_VALID(p->pServer);

    TRACE_NETWORK("CNetworkServer::NetworkServerLoginThread() started\n");

    // receive initial TCP packet
    SInitTCPPacket sPacket;

    try
    {
        p->pServer->ReceiveTCP(p->sock, &sPacket, sizeof(sPacket));
    }
    catch (CNetworkException *e)
    {
        // error, abort thread, delete exception

#ifdef _DEBUG
        TCHAR buf[200];
        e->GetErrorMessage(buf, 200);
        TRACE_NETWORK("CNetworkServer::NetworkServerLoginThread() - network exception caught, aborting login\n  %s\n", buf);
#endif  // _DEBUG

        e->Delete();

        closesocket(p->sock);
        delete p;
        return 1;
    }

    // test challenge
    if ((sPacket.dwChallenge1 != SInitTCPPacket::dwChallengeA) ||
        (sPacket.dwChallenge2 != SInitTCPPacket::dwChallengeB))
    {
        // wrong challenge?
        TRACE_NETWORK("CNetworkServer::NetworkServerLoginThread() - wrong challenge, login failed, ignore client\n");

        // send reply to client (refuse connection)
        sPacket.dwChallenge1 = 0;
        sPacket.dwChallenge2 = SInitTCPPacket::Bad_Challenge;
        try
        {
            p->pServer->SendTCP(p->sock, &sPacket, sizeof(sPacket));
        }
        catch (CNetworkException *e)
        {
            // ignore all errors, just report to log

#ifdef _DEBUG
            TCHAR buf[200];
            e->GetErrorMessage(buf, 200);
            TRACE_NETWORK("CNetworkServer::NetworkServerLoginThread() - network exception caught while sending info to client, ignoring....\n  %s\n", buf);
#endif  // _DEBUG

            e->Delete();
        }

        closesocket(p->sock);
        delete p;
        return 2;
    }
    
    // call the login (callback) function
    CObserver *pObserver;
    VERIFY(p->pServer->m_lockLoginFunction.Lock());
    pObserver = p->pServer->m_pLoginFunction(sPacket.cLoginName, sPacket.cLoginPassword);
    VERIFY(p->pServer->m_lockLoginFunction.Unlock());

    // test login result
    if ((DWORD)pObserver < (DWORD)SInitTCPPacket::Last_Item)
    {
        sPacket.dwChallenge1 = 0;

        switch ((DWORD)pObserver)
        {
        case CNetworkServer::Login_Failed:
            TRACE_NETWORK("CNetworkServer::NetworkServerLoginThread() - login function: login failed\n");
            sPacket.dwChallenge2 = SInitTCPPacket::Login_Failed;
            break;
        case CNetworkServer::Login_BadPassword:
            TRACE_NETWORK("CNetworkServer::NetworkServerLoginThread() - login function: bad password\n");
            sPacket.dwChallenge2 = SInitTCPPacket::Bad_Password;
            break;
        case CNetworkServer::Login_UnknownUser:
            TRACE_NETWORK("CNetworkServer::NetworkServerLoginThread() - login function: unknown user\n");
            sPacket.dwChallenge2 = SInitTCPPacket::Unknown_User;
            break;
        case CNetworkServer::Login_AlreadyLoggedOn:
            TRACE_NETWORK("CNetworkServer::NetworkServerLoginThread() - login function: already logged on\n");
            sPacket.dwChallenge2 = SInitTCPPacket::Already_Logged_On;
            break;
        case CNetworkServer::Login_GameNotRunning:
            TRACE_NETWORK("CNetworkServer::NetworkServerLoginThread() - login function: game not running\n");
            sPacket.dwChallenge2 = SInitTCPPacket::Game_Not_Running;
            break;
        case CNetworkServer::Login_UserDisabled:
            TRACE_NETWORK("CNetworkServer::NetworkServerLoginThread() - login function: user disabled\n");
            sPacket.dwChallenge2 = SInitTCPPacket::User_Disabled;
            break;
        default:
            // Huh? Something is rotten in the state of Denmark...
            ASSERT(FALSE);
            sPacket.dwChallenge2 = SInitTCPPacket::Login_Failed;
        }

        try
        {
            p->pServer->SendTCP(p->sock, &sPacket, sizeof(sPacket));
        }
        catch (CNetworkException *e)
        {
            // ignore this exception, just report the error

#ifdef _DEBUG
            TCHAR buf[200];
            e->GetErrorMessage(buf, 200);
            TRACE_NETWORK("CNetworkServer::NetworkServerLoginThread() - network exception caught, ignoring...\n  %s\n", buf);
#endif  // _DEBUG

            e->Delete();
        }

        closesocket(p->sock);
        delete p;
        return 3;
    }

    ASSERT_KINDOF(CObserver, pObserver);
    TRACE_NETWORK("CNetworkServer::NetworkServerLoginThread() - login function successfuly called\n");

    // create new CNetworkConnection object
    CNetworkConnection *q;
    
    try
    {
        q = new CNetworkConnection();
    }
    catch (CMemoryException *)
    {
        // wow, report it...
        TRACE_NETWORK("CNetworkServer::NetworkServerLoginThread() - memory allocation error\n");

        // do some deletions
        closesocket(p->sock);
        delete p;

        // abort thread
        return 5;
    }
    
    // create network connection object
    try
    {
        // create the connection
        q->Create(p->pServer);
    }
    catch (CNetworkException *e)
    {
        // error, abort thread, delete exception

#ifdef _DEBUG
        {
            // report this exception
            TCHAR buf[200];
            e->GetErrorMessage(buf, 200);
            TRACE_NETWORK("CNetworkServer::NetworkServerLoginThread() - network exception caught while creating connection object, aborting login thread...\n  %s\n", buf);
        }
#endif  // _DEBUG

        e->Delete();
        
        // send reply to client (refuse connection)
        sPacket.dwChallenge1 = 0;
        sPacket.dwChallenge2 = SInitTCPPacket::Server_Error;
        try
        {
            p->pServer->SendTCP(p->sock, &sPacket, sizeof(sPacket));
        }
        catch (CNetworkException *e)
        {
            // ignore all errors, just report to log

#ifdef _DEBUG
            TCHAR buf[200];
            e->GetErrorMessage(buf, 200);
            TRACE_NETWORK("CNetworkServer::NetworkServerLoginThread() - network exception caught while sending info to client, ignoring....\n  %s\n", buf);
#endif  // _DEBUG

            e->Delete();
        }

        closesocket(p->sock);
        delete p;
        return 4;
    }

    // insert new CNetworkConnection object to the link list of connections
    {
        // lock the list of connected clients
        VERIFY(p->pServer->m_lockConnectedClients.Lock());
        
        // reset event
        VERIFY(p->pServer->m_eventAllClientsDisconected.ResetEvent());
        
        // insert newly created CNetworkConnection object into the list of connections
        if (p->pServer->m_pFirstConnection == NULL)
        {
            // insert to the beginning of empty list
            p->pServer->m_pFirstConnection = q;
        }
        else
        {
            // new connection will be the first in the list
            q->m_pNextConnection = p->pServer->m_pFirstConnection;
            p->pServer->m_pFirstConnection->m_pPrevConnection = q;
            p->pServer->m_pFirstConnection = q;
        }

        // unlock the list of connected clients
        VERIFY(p->pServer->m_lockConnectedClients.Unlock());
    }

    // copy temporary socket to CNetworkConnection object
    q->m_TCPRecvSocket = p->sock;

    // create second (temporary) TCP socket, it will be valid until accept() call
    SOCKET TempSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (TempSocket == INVALID_SOCKET)
	{
        // some problem?
		TRACE_NETWORK("CNetworkServer::NetworkServerLoginThread() - socket() error %i\n", WSAGetLastError());

        // send reply to client (refuse connection)
        sPacket.dwChallenge1 = 0;
        sPacket.dwChallenge2 = SInitTCPPacket::Server_Error;
        
        try
        {
            p->pServer->SendTCP(q->m_TCPRecvSocket, &sPacket, sizeof(sPacket));
        }
        catch (CNetworkException *e)
        {
            // ignore this exception, just report

#ifdef _DEBUG
            TCHAR buf[200];
            e->GetErrorMessage(buf, 200);
            TRACE_NETWORK("CNetworkServer::NetworkServerLoginThread() - network exception caught while sending info to client, ignoring...\n  %s\n", buf);
#endif  // _DEBUG

            e->Delete();
        }
        
        // do some deletions
        closesocket(q->m_TCPRecvSocket);
        q->CreateCancel();
        p->pServer->DeleteConnection(q);
        delete q;

        // abort thread
        return 1;
	}
    
    // prepare address for bind()
    sockaddr TempAddr1;
    memset(&TempAddr1, 0, sizeof(TempAddr1));
    ((sockaddr_in *) &TempAddr1)->sin_family = AF_INET;
    ((sockaddr_in *) &TempAddr1)->sin_addr.s_addr = htonl(INADDR_ANY);
    ((sockaddr_in *) &TempAddr1)->sin_port = htons(0);
    
    // bind socket to local port
    if (bind(TempSocket, &TempAddr1, sizeof(TempAddr1)) == SOCKET_ERROR)
    {
        TRACE_NETWORK("CNetworkServer::NetworkServerLoginThread() - bind() error %i\n", WSAGetLastError());

        // send reply to client (refuse connection)
        sPacket.dwChallenge1 = 0;
        sPacket.dwChallenge2 = SInitTCPPacket::Server_Error;
        
        try
        {
            p->pServer->SendTCP(q->m_TCPRecvSocket, &sPacket, sizeof(sPacket));
        }
        catch (CNetworkException *e)
        {
            // ignore this exception, just report to log

#ifdef _DEBUG
            TCHAR buf[200];
            e->GetErrorMessage(buf, 200);
            TRACE_NETWORK("CNetworkServer::NetworkServerLoginThread() - network exception caught: %s\n", buf);
#endif  // _DEBUG

            e->Delete();
        }
        
        // do some deletions
        closesocket(q->m_TCPRecvSocket);
        closesocket(TempSocket);
        q->CreateCancel();
        p->pServer->DeleteConnection(q);
        delete q;

        // abort thread
        return 1;
    }

    // get port number of the bound socket
    {
        sockaddr TempAddr2;
        memset(&TempAddr2, 0, sizeof(TempAddr2));
        ((sockaddr_in *) &TempAddr2)->sin_family = AF_INET;
        int TempAddr2Len = sizeof(TempAddr2);
        if (getsockname(TempSocket, &TempAddr2, &TempAddr2Len) == SOCKET_ERROR)
        {
            TRACE_NETWORK("CNetworkServer::NetworkServerLoginThread() - getsockname() error %i\n", WSAGetLastError());
            
            // send reply to client (deny access)
            sPacket.dwChallenge1 = 0;
            sPacket.dwChallenge2 = SInitTCPPacket::Server_Error;
            
            try 
            {
                p->pServer->SendTCP(q->m_TCPRecvSocket, &sPacket, sizeof(sPacket));
            }
            catch (CNetworkException *e)
            {
                // ignore this exception, just report to log

#ifdef _DEBUG
                TCHAR buf[200];
                e->GetErrorMessage(buf, 200);
                TRACE_NETWORK("CNetworkServer::NetworkServerLoginThread() - network exception caught: %s\n", buf);
#endif  // _DEBUG

                e->Delete();
            }
            
            // do some deletions
            closesocket(q->m_TCPRecvSocket);
            closesocket(TempSocket);
            q->CreateCancel();
            p->pServer->DeleteConnection(q);
            delete q;

            // abort thread
            return 1;
        }
        
        // convert port number to host order and copy to member variable
        q->m_wLocalTCPSendPort = ntohs(((sockaddr_in *) &TempAddr2)->sin_port);
    }
    
    // make the socket listen on given port
    if (listen(TempSocket, 1) == SOCKET_ERROR)
    {
        TRACE_NETWORK("CNetworkServer::NetworkServerLoginThread() - listen() error %i\n", WSAGetLastError());

        // send reply to client (refuse connection)
        sPacket.dwChallenge1 = 0;
        sPacket.dwChallenge2 = SInitTCPPacket::Server_Error;

        try 
        {
            p->pServer->SendTCP(q->m_TCPRecvSocket, &sPacket, sizeof(sPacket));
        }
        catch (CNetworkException *e)
        {
            // ignore this exception

#ifdef _DEBUG
            TCHAR buf[200];
            e->GetErrorMessage(buf, 200);
            TRACE_NETWORK("CNetworkSerer::NetworkServerLoginThread() - network exception caught: %s\n", buf);
#endif  // _DEBUG

            e->Delete();
        }
        
        // do some deletions
        closesocket(q->m_TCPRecvSocket);
        closesocket(TempSocket);
        q->CreateCancel();
        p->pServer->DeleteConnection(q);
        delete q;

        return 1;
    }
    
    // swap challenges
    sPacket.dwChallenge1 = SInitTCPPacket::dwChallengeB;
    sPacket.dwChallenge2 = SInitTCPPacket::dwChallengeA;
    
    // get information from init packet
    q->m_strRemoteHostName = sPacket.cClientName;
    q->m_strRemoteIPAddress = sPacket.cClientIP;
    q->m_wRemoteTCPSendPort = sPacket.wTCPLocalSendPort;
    q->m_wRemoteTCPRecvPort = sPacket.wTCPLocalRecvPort;
    q->m_wRemoteUDPRecvPort = sPacket.wUDPLocalRecvPort;
    q->m_wRemoteUDPSendPort = sPacket.wUDPLocalSendPort;

    // TRACE_NETWORK("Remote TCP send port %i\n", sPacket.wTCPLocalSendPort);
    // TRACE_NETWORK("Remote TCP receive port %i\n", sPacket.wTCPLocalRecvPort);
    // TRACE_NETWORK("Remote UDP send port %i\n", sPacket.wUDPLocalSendPort);
    // TRACE_NETWORK("Remote UDP receive port %i\n", sPacket.wUDPLocalRecvPort);
    
#ifdef NETWORK_FORCESMALLPACKETS
    // SMALL PACKETS FORCED - for testing purposes only!
    q->m_dwMaximumPacketSize = NETWORK_FORCESMALLPACKETS;
    sPacket.dwMaximumPacketSize = NETWORK_FORCESMALLPACKETS;
#else
    // set packet size 
    if (sPacket.dwMaximumPacketSize < p->pServer->m_dwMaximumPacketSize)
        q->m_dwMaximumPacketSize = sPacket.dwMaximumPacketSize;
    else
    {
        q->m_dwMaximumPacketSize = p->pServer->m_dwMaximumPacketSize;
        sPacket.dwMaximumPacketSize = p->pServer->m_dwMaximumPacketSize;
    }
#endif  // NETWORK_FORCESMALLPACKETS

    // set information to init packet
    sPacket.wTCPLocalSendPort = q->m_wLocalTCPSendPort;
    sPacket.wUDPLocalRecvPort = q->m_wLocalUDPRecvPort;
    sPacket.wUDPLocalSendPort = q->m_wLocalUDPSendPort;

    // TRACE_NETWORK("Local UDP send port %i\n", sPacket.wUDPLocalSendPort);
    // TRACE_NETWORK("Local UDP receive port %i\n", sPacket.wUDPLocalRecvPort);

    // question: why is this call performed directly?
    // answer: it would be difficult (and almost impossible)
    //         to send the message, because there's no way
    //         to connect the observer to the notifier
    //         so we will pretend the effect although there's
    //         no event at all...
    pObserver->Perform((DWORD) ID_NetworkServer, E_NEWCLIENTCONNECTED, (DWORD)q);

    // let's also notify the main server, that new client has just connected
    p->pServer->InlayEvent(E_NEWCLIENTCONNECTED, (DWORD)pObserver);

    // copy pObserver to CNetworkConnection object
    q->m_pObserver = pObserver;

    // send info back to client
    try
    {
        p->pServer->SendTCP(q->m_TCPRecvSocket, &sPacket, sizeof(sPacket));
    }
    catch (CNetworkException *e)
    {
        // send failed, crisis, what could be worse?

        // report this exception
        {
#ifdef _DEBUG
            TCHAR buf[200];
            e->GetErrorMessage(buf, 200);
            TRACE_NETWORK("CNetworkServer::NetworkServerLoginThread() - can't return init packet to client, network exception caught: %s\n", buf);
#endif  // _DEBUG
        }

        e->Delete();

        // do some deletions
        closesocket(q->m_TCPRecvSocket);
        closesocket(TempSocket);
        q->CreateCancel();
        p->pServer->DeleteConnection(q);
        delete q;

        // abort thread
        return 1;
    }

    // accept second TCP connection
    q->m_TCPSendSocket = accept(TempSocket, NULL, NULL);
    if (q->m_TCPSendSocket == INVALID_SOCKET)
    {
        // accept() failed
        TRACE_NETWORK("CNetworkServer::NetworkServerLoginThread() - accept() error %i\n", WSAGetLastError());

        // do some deletions
        closesocket(q->m_TCPRecvSocket);
        closesocket(TempSocket);
        q->CreateCancel();
        p->pServer->DeleteConnection(q);
        delete q;
        
        // abort thread
        return 1;
    }

    // close the listening (temporary) socket
    closesocket(TempSocket);
    // TRACE_NETWORK("CNetworkServer::NetworkServerLoginThread() - second TCP connection created\n");

    // make last initializations (for instance connect the UDP ports)
    try
    {
        q->PostCreate(p->addr);
    }
    catch (CNetworkException *e)
    {
        // damn it! report exception, abort thread

#ifdef _DEBUG
        TCHAR buf[200];
        e->GetErrorMessage(buf, 200);
        TRACE_NETWORK("CNetworkServer::NetworkServerLoginThread() - exception caught while doing final initializations, aborting login thread...\n  %s\n", buf);
#endif  // _DEBUG

        e->Delete();
        
        // do some deletions
        closesocket(q->m_TCPRecvSocket);
        q->CreateCancel();

        // abort thread
        return 5;
    }
    
    // delete temporary structure passed from accepter thread
    delete p;

    TRACE_NETWORK("CNetworkServer::NetworkServerLoginThread() - login successfuly completed\n");

    return 0;
}


//////////////////////////////////////////////////////////////////////
// Delete the client connection from the link list
//////////////////////////////////////////////////////////////////////

void CNetworkServer::DeleteConnection(CNetworkConnection *pConnection)
{
    // object must be valid
    ASSERT_VALID(this);

    // lock the list of connected clients
    VERIFY(m_lockConnectedClients.Lock());

    if (pConnection->m_pPrevConnection == NULL)
    {
        if (pConnection->m_pNextConnection == NULL)
        {
            m_pFirstConnection = NULL;
        }
        else
        {
            pConnection->m_pNextConnection->m_pPrevConnection = NULL;
            m_pFirstConnection = pConnection->m_pNextConnection;
        }
    }
    else
    {
        if (pConnection->m_pNextConnection == NULL)
        {
            pConnection->m_pPrevConnection->m_pNextConnection = NULL;
        }
        else
        {
            pConnection->m_pPrevConnection->m_pNextConnection = pConnection->m_pNextConnection;
            pConnection->m_pNextConnection->m_pPrevConnection = pConnection->m_pPrevConnection;
        }
    }

    // check if there are no connected clients
    if (m_pFirstConnection == NULL)
    {
        // set the event (all clients disconnected)
        VERIFY(m_eventAllClientsDisconected.SetEvent());
    }
    
    // unlock the list of connected clients
    VERIFY(m_lockConnectedClients.Unlock());
}
