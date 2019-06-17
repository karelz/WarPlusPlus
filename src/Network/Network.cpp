// Network.cpp: implementation of the CNetwork class
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Network.h"
#include "NetworkException.h"

IMPLEMENT_DYNAMIC(CNetwork, CObject)

//////////////////////////////////////////////////////////////////////
// Initialization of static members
//////////////////////////////////////////////////////////////////////

const DWORD CNetwork::m_dwDefaultMaximumPacketSize = 1000;

const DWORD CNetwork::SInitTCPPacket::dwChallengeA = 0x01234567;
const DWORD CNetwork::SInitTCPPacket::dwChallengeB = 0xFEDCBA98;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction of CNetwork
//////////////////////////////////////////////////////////////////////

// destructor
CNetwork::~CNetwork()
{
    ASSERT(!m_bInitialized);
}

// create method
void CNetwork::Create()
{
    // can't call Create() method twice (because of WinSock initialization)
    ASSERT(!m_bInitialized);

    // output variable 
    WSADATA wsaData;
    // requested Winsock DLL version
    WORD wVersionRequested = MAKEWORD(2, 2);
    
    int nResult = WSAStartup(wVersionRequested, &wsaData);
    if (nResult != 0)
    {
        TRACE_NETWORK("CNetwork::Create() - WSAStartup() error %i\n", nResult);
        throw new CNetworkException(CNetworkException::eWinsockStartupError, nResult, _T(__FILE__), __LINE__);
    }

    if ((LOBYTE(wsaData.wVersion) < LOBYTE(wVersionRequested)) || 
        ((LOBYTE(wsaData.wVersion) == LOBYTE(wVersionRequested)) && (HIBYTE(wsaData.wVersion) < HIBYTE(wVersionRequested))))
    {
        // no error, bad version though
        TRACE_NETWORK("CNetwork::Create() - bad Winsock version %i.%i\n", LOBYTE(wsaData.wVersion), HIBYTE(wsaData.wVersion));
        throw new CNetworkException(CNetworkException::eWinsockStartupError, 0, _T(__FILE__), __LINE__);
    }
    
    // everything is OK
    // TRACE_NETWORK("CNetwork::Create() - Winsock2 initialized\n");
    
    // copy the highest WinSock version number supported
    m_wNetworkVersion = wsaData.wHighVersion;
    // copy WinSock description
    m_strNetworkDescription = wsaData.szDescription;

    // find out maximum packet size
    m_dwMaximumPacketSize = m_dwDefaultMaximumPacketSize;
    {
        // create UDP socket
        SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (sock != INVALID_SOCKET)
        {
            unsigned int max;
            int size = sizeof(unsigned int);

            if ((getsockopt(sock, SOL_SOCKET, SO_MAX_MSG_SIZE, (char *) &max, &size) != SOCKET_ERROR) &&
                (size == sizeof(unsigned int)) &&
                (max >= 500) &&
                (max <= 65536))
            {
                // information is correct, use it
                m_dwMaximumPacketSize = max;
            }
            // close UDP socket
            closesocket(sock);
        }
    }

    // TRACE_NETWORK("CNetwork::Create() - maximum packet size is %i\n", m_dwMaximumPacketSize);
    
    // find out information about local host name and IP address
    {
        // local host name
        char hostname[256];
        if (gethostname(hostname, 256) == SOCKET_ERROR)
        {
            int nError = WSAGetLastError();
            TRACE_NETWORK("CNetwork::Create() - gethostname() error %i\n", nError);
            throw new CNetworkException(CNetworkException::eDNSError, nError, _T(__FILE__), __LINE__);
        }
        
        // get official information
        hostent *temp_host = gethostbyname(hostname);
        if (temp_host == NULL)
        {
            int nError = WSAGetLastError();
            TRACE_NETWORK("CNetwork::Create() - gethostbyname() error %i\n", nError);
            throw new CNetworkException(CNetworkException::eDNSError, nError, _T(__FILE__), __LINE__);
        }
        
        // copy official fully qualified name
        m_strLocalHostName = temp_host->h_name;
        
        // copy IP address
        in_addr temp_addr;
        temp_addr.s_addr = *((u_long *) (void *) temp_host->h_addr_list[0]);
        m_strLocalIPAddress = inet_ntoa(temp_addr);
    }
    // TRACE_NETWORK("CNetwork::Create() - information about local host name was detected\n");

    // consider CNetwork as initialized
    m_bInitialized = TRUE;
}


void CNetwork::Delete()
{
    if (m_bInitialized)
    {
        // consider CNetwork as uninitialized again
        m_bInitialized = FALSE;
        
        CMultithreadNotifier::Delete();
        
        // do WinSock cleanup
        if (WSACleanup() == SOCKET_ERROR)
        {
            int nError = WSAGetLastError();
            TRACE_NETWORK("CNetwork::Delete() - WSACleanup() error %i\n", nError);
            throw new CNetworkException(CNetworkException::eWinsockCleanupError, nError, _T(__FILE__), __LINE__);
        }

        // delete memory occupied by CString classes
        m_strLocalHostName.Empty();
        m_strLocalIPAddress.Empty();
        m_strNetworkDescription.Empty();
    }
}


//////////////////////////////////////////////////////////////////////
// Send and receive block over TCP connection
//////////////////////////////////////////////////////////////////////

void CNetwork::SendTCP(SOCKET Sock, const void *pBuffer, DWORD dwSize)
{
    ASSERT_VALID(this);
    ASSERT(Sock != INVALID_SOCKET);

    // send block; if there was some error, throw an exception
    if (send(Sock, (const char *) pBuffer, dwSize, 0) == SOCKET_ERROR)
    {
        // send error
        int nError = WSAGetLastError();
        TRACE_NETWORK("CNetwork::SendTCP() - send() error %i\n", nError);
        throw new CNetworkException(CNetworkException::eTCPSendError, nError, _T(__FILE__), __LINE__);
    }
}


void CNetwork::ReceiveTCP(SOCKET Sock, void *pBuffer, DWORD dwSize)
{
    ASSERT_VALID(this);
    ASSERT(Sock != INVALID_SOCKET);

    int i;
    DWORD j = 0;
    do
    {
        i = recv(Sock, ((char *) pBuffer) + j, dwSize - j, 0);
        
        if (i == SOCKET_ERROR)
        {
            // recv() error
            int nError = WSAGetLastError();

            if (nError == WSAEWOULDBLOCK)
            {
                // huh? what a surprise!
                ASSERT(j == 0);

                Sleep(10);
                continue;
            }

            TRACE_NETWORK("CNetwork::ReceiveTCP() - recv() error %i\n", nError);
            throw new CNetworkException(CNetworkException::eTCPReceiveError, nError, _T(__FILE__), __LINE__);
        }

        if (i == 0)
        {
            // connection gracefully closed (why?!?!)
            TRACE_NETWORK("CNetwork::ReceiveTCP() - the peer closed the connection, received %i of %i bytes\n", j, dwSize);
            throw new CNetworkException(CNetworkException::eTCPReceiveError, 0, _T(__FILE__), __LINE__);
        }

        j += i;
    } while (j < dwSize);

    // paranoia
    ASSERT(j == dwSize);
}


//////////////////////////////////////////////////////////////////////
// Debug methods
//////////////////////////////////////////////////////////////////////

#ifdef _DEBUG

void CNetwork::AssertValid() const
{
    CObject::AssertValid();
    ASSERT(m_bInitialized);
}


void CNetwork::Dump(CDumpContext &dc) const
{
    CObject::Dump(dc);
    if (m_bInitialized)
    {
        dc << "Winsock DLL version: " << m_wNetworkVersion << endl;
        dc << "Decription: " << m_strNetworkDescription << endl;
        dc << "Maximum packet size: " << m_dwMaximumPacketSize << endl;
    }
    else
    {
        dc << "CNetwork object is not initialized" << endl;
    }
}

#endif  //  _DEBUG
