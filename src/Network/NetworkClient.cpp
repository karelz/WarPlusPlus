// NetworkClient.cpp: implementation of the CNetworkClient class
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NetworkClient.h"

IMPLEMENT_DYNAMIC(CNetworkClient, CNetwork)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// destructor
CNetworkClient::~CNetworkClient()
{
    // check if client is not connected
    // if this assertion fails, you forgot
    // to call DisconnectFromServer() (prior to Delete())
    ASSERT(!m_bConnected);
    ASSERT(m_pNetworkConnection == NULL);
}


//////////////////////////////////////////////////////////////////////
// Connect to server
//////////////////////////////////////////////////////////////////////

CNetworkConnection * CNetworkClient::ConnectToServer(const CString &strServerName, WORD wServerPort, const CString &strLoginName, const CString &strLoginPassword, CObserver *pObserver, DWORD dwNotifierID, DWORD dwThreadID)
{
    ASSERT(m_bInitialized);
    ASSERT(!m_bConnected);
    ASSERT_KINDOF(CObserver, pObserver);

    m_pNetworkConnection = new CNetworkConnection();
    
    // get an address of server
    sockaddr addr;
    // clean up the address (sockaddr) structure 
    memset(&addr, 0, sizeof(sockaddr));
    // set address family
    ((sockaddr_in *) &addr)->sin_family = AF_INET;
    // copy server port
    ((sockaddr_in *) &addr)->sin_port = htons(wServerPort);
    
    {
        // check if strServerName contains an IP address in dotted form (not a human-readable name)
        ((sockaddr_in *) &addr)->sin_addr.s_addr = inet_addr(strServerName);
        if (((sockaddr_in *) &addr)->sin_addr.s_addr == INADDR_NONE)
        {
            // address in strServerName is probably not an IP address
            
            // get name by querying DNS
            hostent *host = gethostbyname(strServerName);

            // if there's problem with querying DNS
            if (host == NULL)
            {
                // can't proceed, don't know where to connect
                int nError = WSAGetLastError();

                switch (nError)
                {
                case WSAHOST_NOT_FOUND:
                    TRACE_NETWORK("CNetworkClient::ConnectToServer() - unknown host \"%s\"\n", strServerName);
                    ConnectCancel();
                    throw new CNetworkException(CNetworkException::eUnknownHostError, 0, _T(__FILE__), __LINE__);
                    break;
                default:
                    TRACE_NETWORK("CNetworkClient::ConnectToServer() - gethostbyname() error %i\n", nError);
                    ConnectCancel();
                    throw new CNetworkException(CNetworkException::eDNSError, nError, _T(__FILE__), __LINE__);
                }
            }

            // DNS query was successful

            // copy the IP address
            ((sockaddr_in *) &addr)->sin_addr.s_addr = *((u_long *) host->h_addr_list[0]);

            // copy information about server to member variables
            m_pNetworkConnection->m_strRemoteHostName = host->h_name;
            m_pNetworkConnection->m_strRemoteIPAddress = inet_ntoa(((sockaddr_in *) &addr)->sin_addr);
        }
        else
        {
            // address in strServerName looks like an IP address in dotted form
            
            // reverse DNS query
            hostent *host = gethostbyaddr((char *) &(((sockaddr_in *) &addr)->sin_addr.S_un.S_addr), 4, AF_INET);

            if (host != NULL)
            {
                // DNS query was successful

                // copy the IP address
                ((sockaddr_in *) &addr)->sin_addr.s_addr = *((u_long *) host->h_addr_list[0]);
                
                // copy information about server to member variables
                m_pNetworkConnection->m_strRemoteHostName = host->h_name;
                m_pNetworkConnection->m_strRemoteIPAddress = inet_ntoa(((sockaddr_in *) &addr)->sin_addr);
            }
            else
            {
                // DNS query was not successful, perhaps there is no DNS server, try to proceed with an IP address only

                // the IP address will stay the same as got from inet_addr() function

                // copy information about server to member variables
                m_pNetworkConnection->m_strRemoteIPAddress = inet_ntoa(((sockaddr_in *) &addr)->sin_addr);
                m_pNetworkConnection->m_strRemoteHostName = m_pNetworkConnection->m_strRemoteIPAddress;
            }
        }
    }
        
    // copy server port to network connection member variable
    m_pNetworkConnection->m_wRemoteTCPRecvPort = wServerPort;
    
    // addr is filled with information about server
    TRACE_NETWORK("CNetworkClient::ConnectToServer() - address of server translated to IP successfuly\n  \"%s\" %s:%i\n", m_pNetworkConnection->m_strRemoteHostName, m_pNetworkConnection->m_strRemoteIPAddress, m_pNetworkConnection->m_wRemoteTCPRecvPort);
    
    // create TCP sockets for control connection
    m_pNetworkConnection->m_TCPSendSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_pNetworkConnection->m_TCPSendSocket == INVALID_SOCKET)
    {
        // some problem?
        int nError = WSAGetLastError();
        TRACE_NETWORK("CNetworkClient::ConnectToServer() - socket() error %i while creating m_TCPSendSocket\n", nError);
        ConnectCancel();
        throw new CNetworkException(CNetworkException::eCreateSocketError, nError, _T(__FILE__), __LINE__);
    }
    
    m_pNetworkConnection->m_TCPRecvSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_pNetworkConnection->m_TCPSendSocket == INVALID_SOCKET)
    {
        // some problem?
        int nError = WSAGetLastError();
        TRACE_NETWORK("CNetworkClient::ConnectToServer() - socket() error %i while creating m_TCPRecvSocket\n", nError);
        ConnectCancel();
        throw new CNetworkException(CNetworkException::eCreateSocketError, nError, _T(__FILE__), __LINE__);
    }
    // sockets successfully created
    
    // connect a m_TCPSendSocket to server
    if (connect(m_pNetworkConnection->m_TCPSendSocket, &addr, sizeof(addr)) == SOCKET_ERROR)
    {
        int nError = WSAGetLastError();
        TRACE_NETWORK("CNetworkClient::ConnectToServer() - connect() error %i while connecting TCP send socket\n", nError);
        ConnectCancel();
        throw new CNetworkException(CNetworkException::eConnectError, nError, _T(__FILE__), __LINE__);
    }

    // TRACE_NETWORK("CNetworkClient::ConnectToServer() - connected to server\n");
    
    // consider client as connected NOW
    m_bConnected = TRUE;
    
    // find out local TCP socket ports
    {
        sockaddr temp_addr;
        int addrlen = sizeof(temp_addr);
        
        // find out local TCP port of sending socket
        if (getsockname(m_pNetworkConnection->m_TCPSendSocket, &temp_addr, &addrlen) == SOCKET_ERROR)
        {
            int nError = WSAGetLastError();
            TRACE_NETWORK("CNetworkClient::ConnectToServer() - getsockname() error %i\n", nError);
            ConnectCancel();
            throw new CNetworkException(CNetworkException::eGetPortError, nError, _T(__FILE__), __LINE__);
        }
        
        m_pNetworkConnection->m_wLocalTCPSendPort = ntohs(((sockaddr_in *) &temp_addr)->sin_port);
    }
    
    // bind TCP receive socket to some port (to let the server know)
    {
        sockaddr xaddr;
        memset(&xaddr, 0, sizeof(xaddr));
        ((sockaddr_in *) &xaddr)->sin_family = AF_INET;
        ((sockaddr_in *) &xaddr)->sin_addr.s_addr = htonl(INADDR_ANY);
        ((sockaddr_in *) &xaddr)->sin_port = htons(0);
        
        if (bind(m_pNetworkConnection->m_TCPRecvSocket, &xaddr, sizeof(xaddr)) == SOCKET_ERROR)
        {
            int nError = WSAGetLastError();
            TRACE_NETWORK("CNetworkClient::ConnectToServer() - can't bind TCP receive socket, error %i\n", nError);
            ConnectCancel();
            throw new CNetworkException(CNetworkException::eBindError, nError, _T(__FILE__), __LINE__);
        }
        
        int xsize = sizeof(xaddr);
        if (getsockname(m_pNetworkConnection->m_TCPRecvSocket, &xaddr, &xsize) == SOCKET_ERROR)
        {
            int nError = WSAGetLastError();
            TRACE_NETWORK("CNetworkClient::ConnectToServer() - can't get local name of TCP receive socket, error %i\n", nError);
            ConnectCancel();
            throw new CNetworkException(CNetworkException::eBindError, nError, _T(__FILE__), __LINE__);
        }
        
        m_pNetworkConnection->m_wLocalTCPRecvPort = ntohs(((sockaddr_in *) &xaddr)->sin_port);
    }
    
    // create connection object
    try 
    {
        m_pNetworkConnection->Create(this);
    }
    catch (CNetworkException *e)
    {
        UNUSED(e);
        
#ifdef _DEBUG
        TCHAR buf[200];
        e->GetErrorMessage(buf, 200);
        TRACE_NETWORK("CNetworkClient::ConnectToServer() - exception caught while creating CNetworkConnection object, throwing on...\n  %s\n", buf);
#endif  // _DEBUG
        
        ConnectCancel();
        throw;
    }
    
    // TRACE_NETWORK("CNetworkClient::ConnectToServer() - network connection object created\n");
    
    // prepare initialization TCP packet
    SInitTCPPacket sInit;
    memset(&sInit, 0, sizeof(sInit));
    sInit.dwChallenge1 = sInit.dwChallengeA;
    sInit.dwChallenge2 = sInit.dwChallengeB;
    strncpy(sInit.cClientName, m_strLocalHostName, 255);
    strncpy(sInit.cClientIP, m_strLocalIPAddress, 15);
    
#ifdef NETWORK_FORCESMALLPACKETS
    sInit.dwMaximumPacketSize = NETWORK_FORCESMALLPACKETS;
#else
    sInit.dwMaximumPacketSize = m_dwMaximumPacketSize;
#endif  // NETWORK_FORCESMALLPACKETS
    
    sInit.wTCPLocalSendPort = m_pNetworkConnection->m_wLocalTCPSendPort;
    sInit.wTCPLocalRecvPort = m_pNetworkConnection->m_wLocalTCPRecvPort;
    sInit.wUDPLocalSendPort = m_pNetworkConnection->m_wLocalUDPSendPort;
    sInit.wUDPLocalRecvPort = m_pNetworkConnection->m_wLocalUDPRecvPort;
    strncpy(sInit.cLoginName, strLoginName, 31);
    strncpy(sInit.cLoginPassword, strLoginPassword, 31);
    
    // TRACE_NETWORK("Local TCP send port %i\n", m_pNetworkConnection->m_wLocalTCPSendPort);
    // TRACE_NETWORK("Local TCP receive port %i\n", m_pNetworkConnection->m_wLocalTCPRecvPort);
    // TRACE_NETWORK("Local UDP send port %i\n", m_pNetworkConnection->m_wLocalUDPSendPort);
    // TRACE_NETWORK("Local UDP receive port %i\n", m_pNetworkConnection->m_wLocalUDPRecvPort);
    
    try
    {
        // send initial TCP packet to server
        // TRACE_NETWORK("Sending initial packet to server...\n");
        SendTCP(m_pNetworkConnection->m_TCPSendSocket, &sInit, sizeof(sInit));
        
        // receive the answer from server (the same format)
        // TRACE_NETWORK("Receiving initial packet from server...\n");
        ReceiveTCP(m_pNetworkConnection->m_TCPSendSocket, &sInit, sizeof(sInit));
    }
    catch (CNetworkException *e)
    {
        UNUSED(e);
        
#ifdef _DEBUG
        TCHAR buf[200];
        e->GetErrorMessage(buf, 200);
        TRACE_NETWORK("CNetworkClient::ConnectToServer() - exception caught while exchanging initial TCP packet, throwing on...\n  %s\n", buf);
#endif  // _DEBUG
        
        m_pNetworkConnection->CreateCancel();
        ConnectCancel();
        throw;
    }
    
    // TRACE_NETWORK("Initial TCP packet sent and received again...\n");
    
    // check challenge
    if (sInit.dwChallenge1 == 0)
    {
        // prepare CNetworkException
        CNetworkException *e = new CNetworkException(CNetworkException::eUnknownError, 0, _T(__FILE__), __LINE__);
        
        switch (sInit.dwChallenge2)
        {
        case sInit.Bad_Challenge:
            TRACE_NETWORK("CNetworkClient::ConnectToServer() - bad challenge\n");
            e->SetError(CNetworkException::eBadChallenge);
            break;
        case sInit.Server_Error:
            TRACE_NETWORK("CNetworkClient::ConnectToServer() - server error\n");
            e->SetError(CNetworkException::eServerError);
            break;
        case sInit.Login_Failed:
            TRACE_NETWORK("CNetworkClient::ConnectToServer() - unknown login error\n");
            e->SetError(CNetworkException::eLoginFailed);
            break;
        case sInit.Bad_Password:
            TRACE_NETWORK("CNetworkClient::ConnectToServer() - bad password\n");
            e->SetError(CNetworkException::eLoginBadPassword);
            break;
        case sInit.Unknown_User:
            TRACE_NETWORK("CNetworkClient::ConnectToServer() - unknown username\n");
            e->SetError(CNetworkException::eLoginUnknownUser);
            break;
        case sInit.Already_Logged_On:
            TRACE_NETWORK("CNetworkClient::ConnectToServer() - user is already logged on\n");
            e->SetError(CNetworkException::eLoginAlreadyLoggedOn);
            break;
        case sInit.Game_Not_Running:
            TRACE_NETWORK("CNetworkClient::ConnectToServer() - gameserver is not running any game\n");
            e->SetError(CNetworkException::eLoginGameNotRunning);
            break;
        case sInit.User_Disabled:
            TRACE_NETWORK("CNetworkClient::ConnectToServer() - user is disabled\n");
            e->SetError(CNetworkException::eLoginUserDisabled);
            break;
        default:
            TRACE_NETWORK("CNetworkClient::ConnectToServer() - unknown server response\n");
            ASSERT(FALSE);
        }
        
        // common action
        m_pNetworkConnection->CreateCancel();
        ConnectCancel();        

        // throw an exception
        throw e;
    }
    
    // check challenge
    if ((sInit.dwChallenge1 != sInit.dwChallengeB) ||
        (sInit.dwChallenge2 != sInit.dwChallengeA))
    {
        TRACE_NETWORK("CNetworkClient::ConnectToServer() - bad server challenge response\n");
        m_pNetworkConnection->CreateCancel();
        ConnectCancel();
        throw new CNetworkException(CNetworkException::eBadChallenge, 0, _T(__FILE__), __LINE__);
    }
    
    TRACE_NETWORK("CNetworkClient::ConnectToServer() - logged in\n");
    
    // copy remote port numbers to members of CNetworkConnection
    m_pNetworkConnection->m_wRemoteUDPSendPort = sInit.wUDPLocalSendPort;
    m_pNetworkConnection->m_wRemoteUDPRecvPort = sInit.wUDPLocalRecvPort;
    m_pNetworkConnection->m_wRemoteTCPSendPort = sInit.wTCPLocalSendPort;
    // TRACE_NETWORK("Remote UDP send port %i\n", sInit.wUDPLocalSendPort);
    // TRACE_NETWORK("Remote UDP receive port %i\n", sInit.wUDPLocalRecvPort);
    
    // copy maximum packet size
    m_pNetworkConnection->m_dwMaximumPacketSize = sInit.dwMaximumPacketSize;
    
    // connect TCP receive port to server
    ((sockaddr_in *) &addr)->sin_port = htons(m_pNetworkConnection->m_wRemoteTCPSendPort);
    if (connect(m_pNetworkConnection->m_TCPRecvSocket, &addr, sizeof(addr)) == SOCKET_ERROR)
    {
        int nError = WSAGetLastError();
        TRACE_NETWORK("CNetworkClient::ConnectToServer() - connect() error %i while connecting TCP receive socket\n", nError);
        m_pNetworkConnection->CreateCancel();
        ConnectCancel();
        throw new CNetworkException(CNetworkException::eConnectError, nError, _T(__FILE__), __LINE__);
    }
    
    // connect user-specified observer to CNetworkConnection
    m_pNetworkConnection->Connect(pObserver, dwNotifierID, dwThreadID);
    
    // copy pointer to pObserver to member variable
    m_pNetworkConnection->m_pObserver = pObserver;
    
    // do additional initializations, pass in the address of the server
    try
    {
        m_pNetworkConnection->PostCreate(addr);
    }
    catch (CNetworkException *e)
    {
        UNUSED(e);
        
#ifdef _DEBUG
        TCHAR buf[200];
        e->GetErrorMessage(buf, 200);
        TRACE_NETWORK("CNetworkClient::ConnectToServer() - exception caught while making final initializations, throwing on...\n  %s\n", buf);
#endif  // _DEBUG
        
        m_pNetworkConnection->CreateCancel();
        ConnectCancel();
        throw;
    }
    
    // success
    TRACE_NETWORK("CNetworkClient::ConnectToServer() - connect successful\n");
    return m_pNetworkConnection;
}


//////////////////////////////////////////////////////////////////////
// Cancel the ConnectToServer() method
//////////////////////////////////////////////////////////////////////

void CNetworkClient::ConnectCancel()
{
    if (m_pNetworkConnection->m_TCPSendSocket != INVALID_SOCKET)
        closesocket(m_pNetworkConnection->m_TCPSendSocket);
    
    if (m_pNetworkConnection->m_TCPRecvSocket != INVALID_SOCKET)
        closesocket(m_pNetworkConnection->m_TCPRecvSocket);

    m_bConnected = FALSE;
    
    // delete the CNetworkConnection object
    delete m_pNetworkConnection;
    m_pNetworkConnection = NULL;
}


//////////////////////////////////////////////////////////////////////
// Disconnect from server
//////////////////////////////////////////////////////////////////////

void CNetworkClient::DisconnectFromServer(DWORD dwTimeOut)
{
    ASSERT_VALID(this);

    // try to disconnect (ie. call CNetworkConnection::Delete())
    m_pNetworkConnection->Delete(dwTimeOut);
    
    delete m_pNetworkConnection;
    m_pNetworkConnection = NULL;
    m_bConnected = FALSE;
}


//////////////////////////////////////////////////////////////////////
// Logout function
//////////////////////////////////////////////////////////////////////

void CNetworkClient::Logout(CNetworkConnection *)
{
    // there's nothing to do in CNetworkClient,
    // this method is needed only in in CNetworkServer
}


//////////////////////////////////////////////////////////////////////
// Debugging methods
//////////////////////////////////////////////////////////////////////

#ifdef _DEBUG

void CNetworkClient::AssertValid() const
{
    CNetwork::AssertValid();
    ASSERT(m_bConnected);
    ASSERT_KINDOF(CNetworkConnection, m_pNetworkConnection);
}


void CNetworkClient::Dump(CDumpContext &dc) const
{
    CNetwork::Dump(dc);
    dc << "CNetworkClient object:" << endl;

    if (m_bInitialized)
    {
        if (m_pNetworkConnection->m_bInitialized)
        {
            dc << "Local name: " << m_strLocalHostName << endl;
            dc << "Local IP: " << m_strLocalIPAddress << endl;
            dc << "Local TCP send port: " << m_pNetworkConnection->m_wLocalTCPSendPort << endl;
            dc << "Local TCP receive port: " << m_pNetworkConnection->m_wLocalTCPRecvPort << endl;
            dc << "Local UDP send port: " << m_pNetworkConnection->m_wLocalUDPSendPort << endl;
            dc << "Local UDP receive port: " << m_pNetworkConnection->m_wLocalUDPRecvPort << endl;
            dc << "Server name: " << m_pNetworkConnection->m_strRemoteHostName << endl;
            dc << "Server IP: " << m_pNetworkConnection->m_strRemoteIPAddress << endl;
            dc << "Server TCP send port: " << m_pNetworkConnection->m_wRemoteTCPSendPort << endl;
            dc << "Server TCP receive port: " << m_pNetworkConnection->m_wRemoteTCPRecvPort << endl;
            dc << "Server UDP send port: " << m_pNetworkConnection->m_wRemoteUDPSendPort << endl;
            dc << "Server UDP receive port: " << m_pNetworkConnection->m_wRemoteUDPRecvPort << endl;
            
            // TODO: add more dumping here...
        }
        else
        {
            dc << "Client is not connected to server" << endl;
        }
    }
    else
    {
        dc << "CNetworkClient is not initialized" << endl;
    }
}

#endif  // _DEBUG
