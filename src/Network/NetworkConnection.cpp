// NetworkConnection.cpp: implementation of the CNetworkConnection class
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NetworkException.h"
#include "NetworkEvents.h"
#include "Network.h"
#include "PacketTypes.h"
#include "VirtualConnection.h"
#include "NetworkConnection.h"
#include "VirtualConnection.inl"
#include "NetworkConstants.h"


#ifdef _DEBUG_MUTEXSENDING
#define TRACE_MUTEXSENDING TRACE_NETWORK
#else  // not _DEBUG_MUTEXSENDING
#define TRACE_MUTEXSENDING(x, y)
#endif  // _DEBUG_MUTEXSENDING


// INTERNAL PACKET TYPE DEFINITIONS
#define PACKET_FRAGMENTED ((BYTE) 0x04)
#define PACKET_LAST_FRAGMENT ((BYTE) 0x08)
#define PACKET_SYNCHRONIZATION ((BYTE) 0x80)
#define PACKET_TYPE1F ((BYTE) (PACKET_GUARANTEED_DELIVERY | PACKET_GUARANTEED_ORDER | PACKET_FRAGMENTED))
#define PACKET_TYPE2F ((BYTE) (PACKET_GUARANTEED_DELIVERY | PACKET_FRAGMENTED))


IMPLEMENT_DYNAMIC(CNetworkConnection, CMultithreadNotifier)

//////////////////////////////////////////////////////////////////////
// operator new for all types of UDP packets
//////////////////////////////////////////////////////////////////////

// common operator delete for all types of packets
// (it is never used for common header as it is
// never allocated separately)
void CNetworkConnection::SUDPPacketCommonHeader::operator delete(void *p)
{
    free(p);
}

// common part for all new operators
static inline void *UDPPacketOperatorNew(size_t stAllocationSize, DWORD dwBufferSize)
{
    void *pTemporary = malloc(stAllocationSize + dwBufferSize);
    if (pTemporary == NULL)
        AfxThrowMemoryException();
    return pTemporary;
}

void *CNetworkConnection::SUDPPacketType1::operator new(size_t stAllocationSize, DWORD dwBufferSize)
{
	return UDPPacketOperatorNew(stAllocationSize, dwBufferSize);
}

void *CNetworkConnection::SUDPPacketType1F::operator new(size_t stAllocationSize, DWORD dwBufferSize)
{
	return UDPPacketOperatorNew(stAllocationSize, dwBufferSize);
}

void *CNetworkConnection::SUDPPacketType2::operator new(size_t stAllocationSize, DWORD dwBufferSize)
{
	return UDPPacketOperatorNew(stAllocationSize, dwBufferSize);
}

void *CNetworkConnection::SUDPPacketType2F::operator new(size_t stAllocationSize, DWORD dwBufferSize)
{
	return UDPPacketOperatorNew(stAllocationSize, dwBufferSize);
}

void *CNetworkConnection::SUDPPacketType3::operator new(size_t stAllocationSize, DWORD dwBufferSize)
{
	return UDPPacketOperatorNew(stAllocationSize, dwBufferSize);
}

void *CNetworkConnection::SUDPPacketType4::operator new(size_t stAllocationSize, DWORD dwBufferSize)
{
	return UDPPacketOperatorNew(stAllocationSize, dwBufferSize);
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// empty constructor is inline

// destructor
CNetworkConnection::~CNetworkConnection() 
{ 
    ASSERT(!m_bInitialized);
}


void CNetworkConnection::Create(CNetwork *pNetworkObject)
{
    // can't call this method if already initialized
    ASSERT(!m_bInitialized);

    // copy pointer to main network object
    m_pNetworkObject = pNetworkObject;
    
    // create UDP send socket
    m_UDPSendSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (m_UDPSendSocket == INVALID_SOCKET)
	{
        int nError = WSAGetLastError();
		TRACE_NETWORK("CNetworkConnection::Create() - can't make UDP send socket, error %i\n", nError);
        throw new CNetworkException(CNetworkException::eCreateSocketError, nError, _T(__FILE__), __LINE__);
	}

    // create UDP receive socket
    m_UDPRecvSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (m_UDPRecvSocket == INVALID_SOCKET)
    {
        int nError = WSAGetLastError();
		TRACE_NETWORK("CNetworkConnection::Create() - can't make UDP receive socket, error %i\n", nError);
        closesocket(m_UDPSendSocket);
        throw new CNetworkException(CNetworkException::eCreateSocketError, nError, _T(__FILE__), __LINE__);
    }

    // prepare address to bind sockets to local ports
    sockaddr addr;
    memset(&addr, 0, sizeof(addr));
    ((sockaddr_in *) &addr)->sin_family = AF_INET;
    ((sockaddr_in *) &addr)->sin_addr.s_addr = htonl(INADDR_ANY);
    ((sockaddr_in *) &addr)->sin_port = htons(0);

    // bind m_UDPSendSocket to local port
    // TRACE_NETWORK("CNetworkConnection::Create() - binding UDP send socket to %s:%i\n", inet_ntoa(((sockaddr_in *) &addr)->sin_addr), ntohs(((sockaddr_in *) &addr)->sin_port));
    if (bind(m_UDPSendSocket, &addr, sizeof(addr)) == SOCKET_ERROR)
    {
        int nError = WSAGetLastError();
        TRACE_NETWORK("CNetworkConnection::Create() - can't bind UDP send socket, error %i\n", nError);
		closesocket(m_UDPSendSocket);
        closesocket(m_UDPRecvSocket);
        throw new CNetworkException(CNetworkException::eBindError, nError, _T(__FILE__), __LINE__);
    }

    // bind m_UDPRecvSocket to local port
    // TRACE_NETWORK("CNetworkConnection::Create() - binding UDP receive socket to %s:%i\n", inet_ntoa(((sockaddr_in *) &addr)->sin_addr), ntohs(((sockaddr_in *) &addr)->sin_port));
    if (bind(m_UDPRecvSocket, &addr, sizeof(addr)) == SOCKET_ERROR)
    {
        int nError = WSAGetLastError();
        closesocket(m_UDPSendSocket);
        closesocket(m_UDPRecvSocket);
		TRACE_NETWORK("CNetworkConnection::Create() - can't bind UDP receive socket, error %i\n", nError);
        throw new CNetworkException(CNetworkException::eBindError, nError, _T(__FILE__), __LINE__);
    }

    // prepare variable for length of address
    int addrlen;

    // find out port number of sending socket
    addrlen = sizeof(addr);
    if (getsockname(m_UDPSendSocket, &addr, &addrlen) == SOCKET_ERROR)
    {
        int nError = WSAGetLastError();
		TRACE_NETWORK("CNetworkConnection::Create() - getsockname() error %i\n", nError);
        closesocket(m_UDPSendSocket);
        closesocket(m_UDPRecvSocket);
        throw new CNetworkException(CNetworkException::eGetPortError, nError, _T(__FILE__), __LINE__);
    }
    // convert port number to host order and copy to member variable
    m_wLocalUDPSendPort = ntohs(((sockaddr_in *) &addr)->sin_port);

    // find out port number of receiving socket
    addrlen = sizeof(addr);
    if (getsockname(m_UDPRecvSocket, &addr, &addrlen) == SOCKET_ERROR)
    {
        int nError = WSAGetLastError();
		TRACE_NETWORK("CNetworkConnection::Create() - getsockname() error %i\n", nError);
        closesocket(m_UDPSendSocket);
        closesocket(m_UDPRecvSocket);
        throw new CNetworkException(CNetworkException::eGetPortError, nError, _T(__FILE__), __LINE__);
    }
    // convert port number to host order and copy to member variable
    m_wLocalUDPRecvPort = ntohs(((sockaddr_in *) &addr)->sin_port);

    // initialize UDP alive checking
    m_bPerformUDPAliveChecking = TRUE;
    m_bUDPPacketReceived = FALSE;
    m_dwTicksFromLastUDPPacket = 0;
}


void CNetworkConnection::CreateCancel()
{
    ASSERT(!m_bInitialized);

    if (m_UDPSendSocket != INVALID_SOCKET)
    {
        closesocket(m_UDPSendSocket);
        m_UDPSendSocket = INVALID_SOCKET;
    }
    if (m_UDPRecvSocket != INVALID_SOCKET)
    {
        closesocket(m_UDPRecvSocket);
        m_UDPRecvSocket = INVALID_SOCKET;
    }

    // reset UDP alive checking
    m_bPerformUDPAliveChecking = TRUE;
    m_bUDPPacketReceived = FALSE;
    m_dwTicksFromLastUDPPacket = 0;
}


void CNetworkConnection::PostCreate(sockaddr sAddr)
{
    // can't call this method if already initialized
    ASSERT(!m_bInitialized);

    // consider network connection initialized
    m_bInitialized = TRUE;

    // prepare address to connect to remote UDP receive port
    ((sockaddr_in *) &sAddr)->sin_port = htons(m_wRemoteUDPRecvPort);
    // TRACE_NETWORK("CNetworkConnection::PostCreate() - connecting UDP send socket to %s:%i\n", inet_ntoa(((sockaddr_in *) &sAddr)->sin_addr), ntohs(((sockaddr_in *) &sAddr)->sin_port));
    if (connect(m_UDPSendSocket, &sAddr, sizeof(sAddr)) == SOCKET_ERROR)
    {
        int nError = WSAGetLastError();
		TRACE_NETWORK("CNetworkConnection::PostCreate() - connect() error %i\n", nError);
        CreateCancel();
        throw new CNetworkException(CNetworkException::eConnectError, nError, _T(__FILE__), __LINE__);
    }
    
    // prepare address to connect to remote UDP send port
    ((sockaddr_in *) &sAddr)->sin_port = htons(m_wRemoteUDPSendPort);
    // TRACE_NETWORK("CNetworkConnection::PostCreate() - connecting UDP receive socket to %s:%i\n", inet_ntoa(((sockaddr_in *) &sAddr)->sin_addr), ntohs(((sockaddr_in *) &sAddr)->sin_port));
    if (connect(m_UDPRecvSocket, &sAddr, sizeof(sAddr)) == SOCKET_ERROR)
    {
        int nError = WSAGetLastError();
		TRACE_NETWORK("CNetworkConnection::PostCreate() - connect() error %i\n", nError);
        CreateCancel();
        throw new CNetworkException(CNetworkException::eConnectError, nError, _T(__FILE__), __LINE__);
    }
    
    // left enough space for packet header
    // (packet of type 1F has the biggest header)
    m_dwMaximumMessageSize = m_dwMaximumPacketSize - sizeof(SUDPPacketType1F);

    // initialize network statistics (this method is a bit piggy,
    // but better than to zero million of DWORD variables individually)
    memset(&m_sNetworkStatistics, 0, sizeof(m_sNetworkStatistics));

    // initialize total serial number
    m_dwTotalSerial = 1;

    // serial number of last packet in last completed block number
    m_dwLastReceivedTotalSerial1 = 0;
    // serial number of last packet in "time-out" area
    m_dwLastReceivedTotalSerial2 = 0;
    // serial number of very last received packet
    m_dwLastReceivedTotalSerial3 = 0;

    // prepare queue for sent packets
    m_qPacketsSent.first = NULL;
    m_qPacketsSent.last = &(m_qPacketsSent.first);
    m_qPacketsSent.dwSentPacketsNumber = 0;
    m_qPacketsSent.dwSentPacketsSize = 0;

    // prepare queue for packets prepared to send
    {
        for (BYTE i = PACKETPRIORITY_LOWEST; i <= PACKETPRIORITY_HIGHEST; i++)
        {
            m_qPacketsPreparedToSend[i].first = NULL;
            m_qPacketsPreparedToSend[i].last = &(m_qPacketsPreparedToSend[i].first);
        }
    }

    // create virtual connection 0 for synchronization packets
    m_pVirtualConnectionList = new SVirtualConnection;
    m_pVirtualConnectionList->btSendPacketFlags = PACKET_SYNCHRONIZATION | PACKET_TYPE4;
    m_pVirtualConnectionList->btSendConnectionPriority = PACKETPRIORITY_HIGHEST;
    m_pVirtualConnectionList->wVirtualConnection = 0;

    // initialize new connection numbers
    // (0 reserved for synchronization packets)
    m_wNewConnectionNumber = 1;

    // create received packets bitfield
    m_pReceivedBitfield = new SUDPPacketsReceivedBitfield;
    memset(m_pReceivedBitfield->aPacketsDelivered, 0, sizeof(DWORD) * RECEIVEDPACKETS_BITFIELD_SIZE);
    m_pReceivedBitfield->pNext = NULL;
    m_pReceivedBitfield->dwFirst = 0;
    m_pReceivedBitfield->aPacketsDelivered[0] = 0x00000001;

    // create buffer for receiver thread
    m_pReceiverThreadBuffer = new BYTE[m_dwMaximumPacketSize];

    // synchronization packets timer's elapsed time (milliseconds)
    m_uSynPacketsPeriod = INIT_SYNTIMER_PERIOD;
    // lost packets timer's elapsed time (milliseconds)    
    m_uLostPacketsPeriod = INIT_LOSTTIMER_PERIOD;

    // set initial request packet size
    m_dwRequestPacketSize = INIT_LOSTPACKET_REQUEST_LEN;

    // prepare control thread
    m_aThreadHandles[eControlThread] = ::AfxBeginThread(NetworkControlThread, this, THREAD_PRIORITY_HIGHEST, 0, CREATE_SUSPENDED);
    if (m_aThreadHandles[eControlThread] == NULL)
    {
        TRACE_NETWORK("CNetworkConnection::PostCreate() - cannot create new thread!!!\n");
        ASSERT(FALSE);
        throw 0;  // fatal error
    }
    m_aThreadHandles[eControlThread]->m_bAutoDelete = FALSE;
    // TRACE_NETWORK("Control thread 0x%X created\n", m_aThreadHandles[eControlThread]->m_nThreadID);

    // prepare receiver thread
    m_aThreadHandles[eReceiverThread] = ::AfxBeginThread(NetworkUDPReceiverThread, this, THREAD_PRIORITY_TIME_CRITICAL, 0, CREATE_SUSPENDED);
    if (m_aThreadHandles[eReceiverThread] == NULL)
    {
        TRACE_NETWORK("CNetworkConnection::PostCreate() - cannot create new thread!!!\n");
        ASSERT(FALSE);
        throw 0;  // fatal error
    }
    m_aThreadHandles[eReceiverThread]->m_bAutoDelete = FALSE;
    // TRACE_NETWORK("Receiver thread 0x%X created\n", m_aThreadHandles[eReceiverThread]->m_nThreadID);

    // prepare sender thread
    m_aThreadHandles[eSenderThread] = ::AfxBeginThread(NetworkUDPSenderThread, this, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
    if (m_aThreadHandles[eSenderThread] == NULL)
    {
        TRACE_NETWORK("CNetworkConnection::PostCreate() - cannot create new thread!!!\n");
        ASSERT(FALSE);
        throw 0;  // fatal error
    }
    m_aThreadHandles[eSenderThread]->m_bAutoDelete = FALSE;
    // TRACE_NETWORK("Sender thread 0x%X created\n", m_aThreadHandles[eSenderThread]->m_nThreadID);

    // prepare timer thread
    m_aThreadHandles[eTimerThread] = ::AfxBeginThread(NetworkTimerThread, this, THREAD_PRIORITY_ABOVE_NORMAL, 0, CREATE_SUSPENDED);
    if (m_aThreadHandles[eTimerThread] == NULL)
    {
        TRACE_NETWORK("CNetworkConnection::PostCreate() - cannot create new thread!!!\n");
        ASSERT(FALSE);
        throw 0;  // fatal error
    }
    m_aThreadHandles[eTimerThread]->m_bAutoDelete = FALSE;
    // TRACE_NETWORK("Timer thread 0x%X created\n", m_aThreadHandles[eTimerThread]->m_nThreadID);

    // start all threads
    m_aThreadHandles[eControlThread]->ResumeThread();
    m_aThreadHandles[eReceiverThread]->ResumeThread();
    m_aThreadHandles[eSenderThread]->ResumeThread();
    m_aThreadHandles[eTimerThread]->ResumeThread();

    // consider network ready NOW
    m_bIsNetworkReady = TRUE;
}


void CNetworkConnection::Delete(DWORD dwTimeOut)
{
    // check if Create() was called
    if (!m_bInitialized)
        return;

    // set closing to the 1st stage
    m_bClosing1 = TRUE;
    
    // should the "last info" be sent
    BOOL bSendCloseInfo = TRUE;

    // is this Delete() called after some error?
    if (!m_bIsNetworkReady)
    {
        bSendCloseInfo = FALSE;
        dwTimeOut = 0;
    }

    // graceful close?
    if (dwTimeOut > 0)
    {
        // do additional cleanup (wait for corrections etc.)

        // now we are sure, that no new data will come,
        // only synchronization packets, which are not important

        // now wait for all sending to finish
        {
            // pretend there is a packet prepared to send
            VERIFY(m_semPacketsPreparedToSend.Unlock());

            // wait
            switch (WaitForSingleObject(m_eventClosingSender, dwTimeOut))
            {
            case WAIT_OBJECT_0:
                // this is OK
                break;

            case WAIT_TIMEOUT:
                // report it
                TRACE_NETWORK("CNetworkConnection::Delete() - WaitForSingleObject() timeout (%u ms)!\n", dwTimeOut);
                break;

            default:
                // what the hell is wrong?
                TRACE_NETWORK("CNetworkConnection::Delete() - WaitForSingleObject() unexpected return code\n");
                ASSERT(FALSE);
            }
        }

        // now it is clear, that all data were sent (at least once),
        // it means that no new packets will be added to
        // the queue of sent packets

        // set the second stage of closing
        m_bClosing2 = TRUE;

        // now wait for all corrections to finish
        {
            // lock the queue of sent packets
            VERIFY(m_qPacketsSent.lock.Lock());

            if (m_qPacketsSent.first != NULL)
            {
                // unlock the queue
                VERIFY(m_qPacketsSent.lock.Unlock());

                // wait for event
                switch (WaitForSingleObject(m_eventClosingSentPackets, dwTimeOut))
                {
                case WAIT_OBJECT_0:
                    {
                        // OK
                    }
                    break;

                case WAIT_TIMEOUT:
                    {
                        // this is bad...
                        TRACE_NETWORK("CNetworkConnection::Delete() - corrections timed out\n");
                        // we can do NOTHING about it...
                    }
                    break;

                default:
                    {
                        // this is REALLY BAD...
                        TRACE_NETWORK("CNetworkConnection::Delete() - unexpected WaitForSingleObject() return code\n");
                        ASSERT(FALSE);
                    }
                    break;
                }
            }
            else
            {
                // unlock the queue
                VERIFY(m_qPacketsSent.lock.Unlock());
            }

        }
        
        // at this point all corrections are done, there's nothing
        // more to do, just continue with abortive close, but send
        // another type of close notification

        {
            // prepare "TCP packet"
            STCPPacket sPacket;
            sPacket.eType = STCPPacket::NormalClose;
            
            // send the data
            if (!SendTCP(&sPacket, sizeof(sPacket)))
            {
                // error? just report it...
                TRACE_NETWORK("CNetworkConnection::Delete() - the peer could not been informed about closing the connection\n");
            }
        }
    }
    else
    {
        // abortive close

        // send abort info to peer
        if (bSendCloseInfo)
        {
            // prepare "TCP packet"
            STCPPacket sPacket;
            sPacket.eType = STCPPacket::AbortiveClose;

            // send the data
            if (!SendTCP(&sPacket, sizeof(sPacket)))
            {
                // error?
                TRACE_NETWORK("CNetworkConnection::Delete() - the peer could not been informed about closing the connection\n");
            }
        }
    }

    // both types of closing now continue with the same...

    // stop timer thread (kill timers)
    VERIFY(m_eventTimerKill.SetEvent());
    // stop sender thread
    VERIFY(m_eventSenderKill.SetEvent());
    // stop receiver thread
    VERIFY(m_eventReceiverKill.SetEvent());
    // stop controller thread
    VERIFY(m_eventControllerKill.SetEvent());
    
    // wait for all threads stopped, they should stop quickly....
    {
        HANDLE aHandles[4];
        {
            for (int i = 0; i < 4; i++)
                aHandles[i] = m_aThreadHandles[i]->m_hThread;
        }
        
        switch (WaitForMultipleObjects(4, aHandles, TRUE, NETWORKCONNECTION_THREAD_TIMEOUT))
        {
        case WAIT_FAILED:
            {
                // error
                TRACE_NETWORK("CNetworkConnection::Delete() - WaitForMultipleObjects() error %i\n", GetLastError());
                ASSERT(FALSE);                
            }
            break;
            
        case WAIT_OBJECT_0 + 0:
        case WAIT_OBJECT_0 + 1:
        case WAIT_OBJECT_0 + 2:
        case WAIT_OBJECT_0 + 3:
            // good, all threads finished
            break;
            
        case WAIT_TIMEOUT:
            {
                // huh, why? can't help myself, kill 'em all...
                TRACE_NETWORK("CNetworkConnection::Delete() - WaitForMultipleObjects() time-out %i milliseconds\n", NETWORKCONNECTION_THREAD_TIMEOUT);
                
                // kill those zombies!
                for (int i = 0; i < 4; i++)
                {
                    DWORD dwExitCode;
                    if (!::GetExitCodeThread(aHandles[i], &dwExitCode) || (dwExitCode == STILL_ACTIVE))
                    {
                        TRACE_NETWORK("CNetworkConnection::Delete() - thread %i did not stop in the specified time-out, kill...\n", i);
                        if (::TerminateThread(aHandles[i], (DWORD) -1) != 0)
                        {
                            // error terminating thread, report only...
                            TRACE_NETWORK("CNetworkConnection::Delete() - TerminateThread() error %i\n", GetLastError());
                        }
                    }
                }       
            }
            break;
            
        default:
            // it is suspicious, report warning...
            TRACE_NETWORK("CNetworkConnection::Delete() - suspicious WaitForMultipleObjects() return code\n");
            ASSERT(FALSE);
            break;
        }
        
        // delete all thread objects
        {
            for (int i = 0; i < 4; i++)
                delete m_aThreadHandles[i];
        }
    }
    
    // delete receiver thread buffer
    if (m_pReceiverThreadBuffer)
        delete [] m_pReceiverThreadBuffer;
    
    // delete received packet bitfield
    VERIFY(m_lockReceivedBitfield.Lock());
    while (m_pReceivedBitfield != NULL)
    {
        SUDPPacketsReceivedBitfield *p = m_pReceivedBitfield;
        m_pReceivedBitfield = m_pReceivedBitfield->pNext;
        delete p;
    }
    VERIFY(m_lockReceivedBitfield.Unlock());
    
    // lock virtual connection list
    VERIFY(m_lockVirtualConnectionList.Lock());
    ASSERT(m_pVirtualConnectionList != NULL);
    ASSERT(m_pVirtualConnectionList->wVirtualConnection == 0);
    
    // delete virtual connections
    while (m_pVirtualConnectionList != NULL)
    {
        // store pointer to the first item
        SVirtualConnection *p = m_pVirtualConnectionList;
        // remove the first item from the list
        m_pVirtualConnectionList = m_pVirtualConnectionList->pNext;
        
#ifdef _DEBUG
        if (p->wVirtualConnection > 0)
        {
            TRACE_NETWORK("CNetworkConnection::Delete() - network connection %i was not deleted correctly\n", p->wVirtualConnection);
        }
#endif  // _DEBUG

        // connection must be at least "inactive"
        ASSERT(p->bSendCompoundBlockActive == FALSE);
        ASSERT(p->bReceiveCompoundBlockActive == FALSE);
        
        // remove unread packets
        VERIFY(p->qReceivePackets.lock.Lock());
        while (p->qReceivePackets.first != NULL)
        {
            // copy pointer to the first item
            SReceivePacketList *q = p->qReceivePackets.first;
            // remove the first item from the list
            p->qReceivePackets.first = p->qReceivePackets.first->pNext;
            // delete the first item
            delete [] q->pBuffer;
            delete q;
        }
        // let the pointer to the last item point to first pointer
        p->qReceivePackets.last = &(p->qReceivePackets.first);
        // paranoia
        ASSERT(AfxIsValidAddress(p->qReceivePackets.last, 4, TRUE) && ((*(p->qReceivePackets.last) == NULL) || AfxIsMemoryBlock(*(p->qReceivePackets.last), sizeof(SReceivePacketList))));
        VERIFY(p->qReceivePackets.lock.Unlock());

        if (p->wVirtualConnection != 0)
        {
            // wake up the waiting CreateVirtualConnection()
            VERIFY(p->eventCreateAcknowledged.SetEvent());

            // send info to peer (if necessary)
            if (!p->bDeleted)
            {
                // prepare "TCP packet"
                STCPPacket sPacket;
                sPacket.eType = STCPPacket::CloseVirtualConnection;
                sPacket.dwData1 = p->wVirtualConnection;
                
                // prepare error variable
                int nError;
                // send data
                if (!SendTCP(&sPacket, sizeof(sPacket), &nError))
                {
                    // report warning
                    TRACE_NETWORK("CNetworkConnection::Delete() - could not send notification about closing virtual connection to peer, error %i\n", nError);
                }
            }

            // disconnect obserever from notifier (this)
            if (p->pObserver != NULL)
            {
                Disconnect(p->pObserver);
            }
        }
        
        // pay attention to cautious sending
        VERIFY(p->eventWaitForSend.SetEvent());
        VERIFY(p->lockWaitForSend.Lock());
        VERIFY(p->lockWaitForSend.Unlock());

        delete p;
    }
    
    // unlock the virtual connection list
    VERIFY(m_lockVirtualConnectionList.Unlock());

    // delete queues of packets prepared to send
    for (BYTE i = PACKETPRIORITY_LOWEST; i <= PACKETPRIORITY_HIGHEST; i++)
    {
        VERIFY(m_qPacketsPreparedToSend[i].lock.Lock());
        while (m_qPacketsPreparedToSend[i].first != NULL)
        {
            // copy the first item from the list
            SSendPacketList *q = m_qPacketsPreparedToSend[i].first;
            // remove the first item from the list
            m_qPacketsPreparedToSend[i].first = m_qPacketsPreparedToSend[i].first->pNext;
            // delete that first item
            delete q->pPacket;
            delete q;
        }
        m_qPacketsPreparedToSend[i].last = &(m_qPacketsPreparedToSend[i].first);
        VERIFY(m_qPacketsPreparedToSend[i].lock.Unlock());
    }
    
    // delete queue of sent packets
    VERIFY(m_qPacketsSent.lock.Lock());
    while (m_qPacketsSent.first != NULL)
    {
        SSendPacketList *q = m_qPacketsSent.first;
        m_qPacketsSent.first = m_qPacketsSent.first->pNext;
        delete q->pPacket;
        delete q;
    }
    m_qPacketsSent.last = &(m_qPacketsSent.first);
    VERIFY(m_qPacketsSent.lock.Unlock());
    
    // call Logout() of main network object
    m_pNetworkObject->Logout(this);
    
    // disconnect all observers, clean everything regarding to events...
    CMultithreadNotifier::Delete();

    // free the memory allocated by CString objects
    m_strRemoteHostName.Empty();
    m_strRemoteIPAddress.Empty();
    
    // close all sockets
    closesocket(m_TCPSendSocket);
    closesocket(m_TCPRecvSocket);
    closesocket(m_UDPSendSocket);
    closesocket(m_UDPRecvSocket);
    
    // reset UDP alive checking
    m_bUDPPacketReceived = FALSE;
    m_dwTicksFromLastUDPPacket = 0;
    m_bPerformUDPAliveChecking = TRUE;
    
    // consider connection object as uninitialized
    m_bInitialized = FALSE;

    // that's all!
}


//////////////////////////////////////////////////////////////////////
// Send only ONE error event
//////////////////////////////////////////////////////////////////////

void CNetworkConnection::SendErrorEvent(ENetworkEvents eEvent, DWORD dwEventParam)
{
    // get "the exclusive right"
    VERIFY(m_lockErrorEvent.Lock());

    if (m_bSendErrorActivated == FALSE)
    {
        TRACE_NETWORK("CNetworkConnection::SendErrorEvent() - error event is being sent (event %i, param %u)\n", eEvent, dwEventParam);

        // set the boolean, it means the event was sent and will never be sent again
        m_bSendErrorActivated = TRUE;

        // consider network as not ready now
        m_bIsNetworkReady = FALSE;

        // send the event
        InlayEvent(eEvent, dwEventParam);

        // cancel all pending CreateVirtualConnection()
        // cancel all waiting cautious senders
        {
            // lock the virtual connection list
            VERIFY(m_lockVirtualConnectionList.Lock());
            ASSERT(m_pVirtualConnectionList != NULL);
            ASSERT(m_pVirtualConnectionList->wVirtualConnection == 0);

            // enumerate all connections
            SVirtualConnection *p = m_pVirtualConnectionList;
            while (p != NULL)
            {
                if (p->wVirtualConnection > 0)
                {
                    VERIFY(p->eventCreateAcknowledged.SetEvent());
                    VERIFY(p->eventWaitForSend.SetEvent());
                }
                p = p->pNext;
            }

            // unlock the virtual connection list
            VERIFY(m_lockVirtualConnectionList.Unlock());
        }

        // set event for the waiting Delete() with
        // nonzero wait time
        VERIFY(m_eventClosingSentPackets.SetEvent());
    }
    else
    {
        // error event was sent already, do nothing
        TRACE_NETWORK("CNetworkConnection::SendErrorEvent() - error event was sent already, do nothing\n");
    }

    // unlock the critical section
    VERIFY(m_lockErrorEvent.Unlock());

    // that's all
}


//////////////////////////////////////////////////////////////////////
// Create virtual connection
//////////////////////////////////////////////////////////////////////

WORD CNetworkConnection::CreateVirtualConnection(BYTE btConnectionFlags, CObserver *pObserver, DWORD dwNotID, DWORD dwThreadID, const void *pUserData, DWORD dwSize, BYTE btBasePriority)
{
    // object must be valid
    ASSERT_VALID(this);

    // check the connection flags, priority and user data
    ASSERT((btConnectionFlags & ~(PACKET_GUARANTEED_DELIVERY | PACKET_GUARANTEED_ORDER)) == 0);
    ASSERT(btBasePriority <= PACKETPRIORITY_HIGHEST);
    ASSERT((pUserData == NULL) || ((dwSize > 0) && ::AfxIsValidAddress(pUserData, dwSize, FALSE)));
    if (((btConnectionFlags & ~(PACKET_GUARANTEED_DELIVERY | PACKET_GUARANTEED_ORDER)) != 0) ||
        (btBasePriority > PACKETPRIORITY_HIGHEST) ||
        ((pUserData != NULL) && (dwSize == 0)))
        throw new CNetworkException(CNetworkException::eCreateVirtualConnectionError, 0, __FILE__, __LINE__);

    // check the observer
#ifdef _DEBUG
    if (pObserver != NULL)
    {
        // ASSERT_VALID(pObserver);
        ASSERT_KINDOF(CObserver, pObserver);
    }
#endif  // _DEBUG

    ASSERT(!m_bClosing1);
    if (!m_bIsNetworkReady || m_bClosing1)
    {
        TRACE_NETWORK("CNetworkConnection::CreateVirtualConnection() - network is down! throwing an exception\n");
        throw new CNetworkException(CNetworkException::eCreateVirtualConnectionError, 0, __FILE__, __LINE__);
    }

    // lock virtual connection list
    VERIFY(m_lockVirtualConnectionList.Lock());
    ASSERT(m_pVirtualConnectionList != NULL);
    ASSERT(m_pVirtualConnectionList->wVirtualConnection == 0);
    
    // find suitable virtual connection number, create new structure
    SVirtualConnection **p = &m_pVirtualConnectionList;
    for ( ; ; )
    {
        while (((*p) != NULL) && ((*p)->wVirtualConnection < m_wNewConnectionNumber))
            p = &((*p)->pNext);
        
        if ((*p) != NULL)
        {
            // not the end of list
            if ((*p)->wVirtualConnection == m_wNewConnectionNumber)
            {
                // connection with m_wNewConnectionNumber number exists already
                m_wNewConnectionNumber++;
                // don't let connection number be zero (reserved for synchronization packets)
                if (m_wNewConnectionNumber == 0)
                    m_wNewConnectionNumber++;
                continue;
            }
            else
            {
                // can create connection with number m_wNewConnectionNumber
                // and insert it in front of (**p)
                SVirtualConnection *q = *p;
                *p = new SVirtualConnection;
                (*p)->pNext = q;
            }
        }
        else
        {
            // at the end of list
            *p = new SVirtualConnection;
            (*p)->pNext = NULL;
        }

        break;
    } // endless loop

    //  fill created structure
    (*p)->btSendPacketFlags = btConnectionFlags;
    (*p)->pObserver = pObserver;
    (*p)->btSendConnectionPriority = btBasePriority;
    (*p)->wVirtualConnection = m_wNewConnectionNumber;

    // prepare a return value (number of newly created connection)
    WORD return_value = m_wNewConnectionNumber;
    
    // increase the number for new connection
    // don't let it be zero (reserved for synchronization)
    m_wNewConnectionNumber++;
    if (m_wNewConnectionNumber == 0)
        m_wNewConnectionNumber++;
        
    // send information about new virtual connection to peer
    {
        // prepare "TCP packet"
        STCPPacket sPacket;
        sPacket.eType = STCPPacket::NewVirtualConnection;
        sPacket.dwData1 = 
            ((DWORD) (*p)->btSendPacketFlags) | 
            ((DWORD) (*p)->btSendConnectionPriority << 8) |
            ((DWORD) (*p)->wVirtualConnection << 16);
        sPacket.dwData2 = dwSize;

        int nError;

        // send data
        if (!SendTCP(&sPacket, sizeof(sPacket), &nError))
        {
            // error, can't create connection
            TRACE_NETWORK("CNetworkConnection::CreateVirtualConnection() - can't send new virtual connection request\n");

            // delete virtual connection
            SVirtualConnection *q = *p;
            *p = (*p)->pNext;
            delete q;

            // unlock virtual connection list
            VERIFY(m_lockVirtualConnectionList.Unlock());

            // fail
            throw new CNetworkException(CNetworkException::eTCPSendError, nError, __FILE__, __LINE__);
        }

        // send user data (if provided)
        if (dwSize > 0)
        {
            if (!SendTCP(pUserData, dwSize, &nError))
            {
                // error, can't create connection
                TRACE_NETWORK("CNetworkConnection::CreateVirtualConnection() - can't send user data with new virtual connection request, error %i\n", nError);
                
                // delete virtual connection
                SVirtualConnection *q = *p;
                *p = (*p)->pNext;
                delete q;
                
                // unlock virtual connection list
                VERIFY(m_lockVirtualConnectionList.Unlock());
                
                // fail
                throw new CNetworkException(CNetworkException::eTCPSendError, nError, __FILE__, __LINE__);
            }
        }    
    }

    // connect new virtual connection observer to notifier (this)
    if (pObserver != NULL)
    {
        Connect(pObserver, dwNotID, dwThreadID);
    }

    // unlock virtual connection list
    VERIFY(m_lockVirtualConnectionList.Unlock());

    // wait for creation acknowledge
    VERIFY((*p)->eventCreateAcknowledged.Lock());

    // return the number of the new virtual connection
    return return_value;
}


CVirtualConnection CNetworkConnection::CreateVirtualConnectionEx(BYTE btConnectionFlags, CObserver *pObserver, DWORD dwNotID, DWORD dwThreadID, const void *pUserData, DWORD dwSize, BYTE btBasePriority)
{
    CVirtualConnection temp;
    temp.m_wVirtualConnection = CreateVirtualConnection(btConnectionFlags, pObserver, dwNotID, dwThreadID, pUserData, dwSize, btBasePriority);
    temp.m_pNetworkConnection = this;
    return temp;
}


//////////////////////////////////////////////////////////////////////
// Set virtual connection observer
//////////////////////////////////////////////////////////////////////

// set observer for virtual connection (by number)
BOOL CNetworkConnection::SetVirtualConnectionObserver(WORD wConnectionNumber, CObserver *pObserver, DWORD dwNotID, DWORD dwThreadID)
{
    // object must be valid
    ASSERT_VALID(this);

    // check connection number
    ASSERT(wConnectionNumber != 0);
    if (wConnectionNumber == 0)
        return FALSE;

    // check given observer
    ASSERT(pObserver != NULL);
    // ASSERT_VALID(pObserver);
    ASSERT_KINDOF(CObserver, pObserver);

    // test the closing state
    ASSERT(!m_bClosing1);
    if (m_bClosing1)
        return FALSE;

    // lock virtual connection list
    VERIFY(m_lockVirtualConnectionList.Lock());
    ASSERT(m_pVirtualConnectionList != NULL);
    ASSERT(m_pVirtualConnectionList->wVirtualConnection == 0);

    // find connection
    SVirtualConnection *p = m_pVirtualConnectionList;
    while ((p != NULL) && (p->wVirtualConnection < wConnectionNumber))
        p = p->pNext;

    // end of list or next item has greater connection number, connection does not exist
    // => connection does not exist 
    if ((p == NULL) || (p->wVirtualConnection > wConnectionNumber))
    {
        // unlock virtual connection list
        VERIFY(m_lockVirtualConnectionList.Unlock());
        return FALSE;
    }

    // paranoia
    ASSERT(p->wVirtualConnection == wConnectionNumber);
    
    // pObserver must not be set
    ASSERT(p->pObserver == NULL);

    // set the observer
    p->pObserver = pObserver;

    // connect virtual connection observer to notifier (this)
    Connect(pObserver, dwNotID, dwThreadID);

    // unlock virtual connection list
    VERIFY(m_lockVirtualConnectionList.Unlock());

    // success
    return TRUE;
}


//////////////////////////////////////////////////////////////////////
// Delete virtual connection
//////////////////////////////////////////////////////////////////////

BOOL CNetworkConnection::DeleteVirtualConnection(WORD wConnectionNumber)
{
    // object must be valid
    ASSERT_VALID(this);

    // check parameter
    ASSERT(wConnectionNumber != 0);
    if (wConnectionNumber == 0)
        return FALSE;

    // lock virtual connection list
    VERIFY(m_lockVirtualConnectionList.Lock());
    ASSERT(m_pVirtualConnectionList != NULL);
    ASSERT(m_pVirtualConnectionList->wVirtualConnection == 0);
    
    // find connection
    SVirtualConnection **p = &m_pVirtualConnectionList;
    while (((*p) != NULL) && ((*p)->wVirtualConnection < wConnectionNumber))
        p = &((*p)->pNext);

    // end of the list or the next item has greater connection number => connection does not exist
    if (((*p) == NULL) || ((*p)->wVirtualConnection > wConnectionNumber))
    {
        // unlock virtual connection list
        VERIFY(m_lockVirtualConnectionList.Unlock());
        return FALSE;
    }

    // paranoia
    ASSERT((*p)->wVirtualConnection == wConnectionNumber);

    // make variable for connection being deleted
    SVirtualConnection *q = *p;

    // delete *q from list
	*p = (*p)->pNext;

	// unlock virtual connection list
	VERIFY(m_lockVirtualConnectionList.Unlock());

    // wake up the waiting CreateVirtualConnection()
    VERIFY(q->eventCreateAcknowledged.SetEvent());
	
    // lock sending (thread checking)
    TRACE_MUTEXSENDING("CNetworkConnection::DeleteVirtualConnection() - trying to lock mutexSending (%i)\n", q->wVirtualConnection);
    VERIFY(q->mutexSending.Lock());
    TRACE_MUTEXSENDING("CNetworkConnection::DeleteVirtualConnection() - mutexSending locked (%i)\n", q->wVirtualConnection);
    
    // disconnect obserever from notifier (this)
    if (q->pObserver != NULL)
    {
        Disconnect(q->pObserver);
    }

    // close compound block if active
    ASSERT(!q->bSendCompoundBlockActive);
    if (q->bSendCompoundBlockActive)
        EndSendCompoundBlockUnsafely(q);

    ASSERT(!q->bReceiveCompoundBlockActive);

    // lock received packets list
    VERIFY(q->qReceivePackets.lock.Lock());

    // delete unread data
    while (q->qReceivePackets.first != NULL)
    {
        SReceivePacketList *r = q->qReceivePackets.first;
        q->qReceivePackets.first = q->qReceivePackets.first->pNext;
        delete [] r->pBuffer;
        delete r;
    }
    q->qReceivePackets.last = &(q->qReceivePackets.first);
    ASSERT(::AfxIsValidAddress(q->qReceivePackets.last, 4, TRUE) && ((*(q->qReceivePackets.last) == NULL) || ::AfxIsMemoryBlock(*(q->qReceivePackets.last), sizeof(SReceivePacketList))));

    // unlock received packets list
    VERIFY(q->qReceivePackets.lock.Unlock());

    // send information about closing to peer (if it is not him who conducted this action)
    if (!q->bDeleted)
    {
        // prepare "TCP packet"
        STCPPacket sPacket;
        sPacket.eType = STCPPacket::CloseVirtualConnection;
        sPacket.dwData1 = q->wVirtualConnection;
        if (q->btSendPacketFlags & PACKET_GUARANTEED_DELIVERY)
        {
            sPacket.dwData2 = q->dwSendSerial - 1;
        }

        // prepare error variable
        int nError;

        // send data
        if (!SendTCP(&sPacket, sizeof(sPacket), &nError))
        {
            // report warning
            TRACE_NETWORK("CNetworkConnection::DeleteVirtualConnection() - could not send notification to peer, error %i\n", nError);
        }
    }

    // delete virtual connection object
    delete q;

	return TRUE;
}


BOOL CNetworkConnection::DeleteVirtualConnection(const CVirtualConnection &VirtualConnection)
{
    if ((VirtualConnection.m_pNetworkConnection == NULL) || 
        (VirtualConnection.m_wVirtualConnection == 0))
        return TRUE;

    return DeleteVirtualConnection(VirtualConnection.m_wVirtualConnection); 
}


//////////////////////////////////////////////////////////////////////
// Compound block methods -- sending
//////////////////////////////////////////////////////////////////////

BOOL CNetworkConnection::BeginSendCompoundBlock(WORD wConnectionNumber, BOOL bCautiousSend, BYTE btPacketPriority)
{
	// object must be valid
    ASSERT_VALID(this);

    // check the connection number
    ASSERT(wConnectionNumber > 0);
    if (wConnectionNumber == 0)
        return FALSE;

    // test the closing state
    ASSERT(!m_bClosing1);
    if (m_bClosing1)
        return FALSE;

    // lock virtual connection list
	VERIFY(m_lockVirtualConnectionList.Lock());
    ASSERT(m_pVirtualConnectionList != NULL);
    ASSERT(m_pVirtualConnectionList->wVirtualConnection == 0);

    // find given virtual connection
    SVirtualConnection *p = m_pVirtualConnectionList;
    while ((p != NULL) && (p->wVirtualConnection < wConnectionNumber))
        p = p->pNext;
    if ((p == NULL) || (p->wVirtualConnection > wConnectionNumber))
    {
        // unlock virtual connection list
        VERIFY(m_lockVirtualConnectionList.Unlock());
        return FALSE;
    }

    // paranoia
    ASSERT(p->wVirtualConnection == wConnectionNumber);

    // check packet priority
    if (btPacketPriority == PACKETPRIORITY_ACTUAL)
        btPacketPriority = p->btSendConnectionPriority;
	// check the validity of priority
    ASSERT(btPacketPriority <= PACKETPRIORITY_HIGHEST);
    if (btPacketPriority > PACKETPRIORITY_HIGHEST)
    {
        VERIFY(m_lockVirtualConnectionList.Unlock());
        return FALSE;
    }

    // if there is already active compound block
    if (p->bSendCompoundBlockActive)
    {
		// unlock
        VERIFY(m_lockVirtualConnectionList.Unlock());
        return FALSE;
    }

    // set compound block active
    p->bSendCompoundBlockActive = TRUE;
    // copy bCautiousSend parameter to virtual connection structure member
    p->bSendCompoundBlockCautious = bCautiousSend;
    // if bCautiousSend is TRUE => reset event EndSendCompoundBlock() will wait for...
    if (bCautiousSend)
    {
        VERIFY(p->lockWaitForSend.Lock());
        VERIFY(p->eventWaitForSend.ResetEvent());
    }

    // copy packet priority
    p->btSendCompoundBlockPriority = btPacketPriority;
    // prepare buffer
    p->pSendCompoundBlockBuffer = new BYTE[m_dwMaximumMessageSize];
    // buffer is empty now
    p->dwSendCompoundBlockSize = 0;
	// set compound block fragment number to zero
	p->dwSendFragmentNumber = 0;

    // unlock
	VERIFY(m_lockVirtualConnectionList.Unlock());
    
    // thread checking
    TRACE_MUTEXSENDING("CNetworkConnection::BeginSendCompoundBlock() - trying to lock mutexSending (%i)\n", p->wVirtualConnection);
    VERIFY(p->mutexSending.Lock());    
    TRACE_MUTEXSENDING("CNetworkConnection::BeginSendCompoundBlock() - mutexSending locked (%i)\n", p->wVirtualConnection);
       
    return TRUE;
}


BOOL CNetworkConnection::EndSendCompoundBlock(WORD wConnectionNumber)
{
    // object must be valid
    ASSERT_VALID(this);

    // test the connection number
    ASSERT(wConnectionNumber != 0);
    if (wConnectionNumber == 0)
        return FALSE;

    // test the closing state
    ASSERT(!m_bClosing1);
    if (m_bClosing1)
        return FALSE;

    // lock virtual connection list
    VERIFY(m_lockVirtualConnectionList.Lock());
    ASSERT(m_pVirtualConnectionList != NULL);
    ASSERT(m_pVirtualConnectionList->wVirtualConnection == 0);

    // find given virtual connection
    SVirtualConnection *p = m_pVirtualConnectionList;
    while ((p != NULL) && (p->wVirtualConnection < wConnectionNumber))
        p = p->pNext;
    if ((p == NULL) || (p->wVirtualConnection > wConnectionNumber))
	{
		// unlock virtual connection list
		VERIFY(m_lockVirtualConnectionList.Unlock());
        return FALSE;
	}

    // paranoia
    ASSERT(p->wVirtualConnection == wConnectionNumber);

    // if there is no compound block active
    if (!p->bSendCompoundBlockActive)
	{
		// unlock virtual connection list
		VERIFY(m_lockVirtualConnectionList.Unlock());
        return FALSE;
	}
    
    BOOL bReturnValue = EndSendCompoundBlockUnsafely(p);

	// release mutex (thread checking)
    TRACE_MUTEXSENDING("CNetworkConnection::EndSendCompoundBlock() - trying to unlock mutexSending (%i)\n", p->wVirtualConnection);
    VERIFY(p->mutexSending.Unlock());
    TRACE_MUTEXSENDING("CNetworkConnection::EndSendCompoundBlock() - mutexSending unlocked (%i)\n", p->wVirtualConnection);
    
    // unlock the virtual connection list
    VERIFY(m_lockVirtualConnectionList.Unlock());

    // wait for packet sent (if cautious send was enabled in BeginSendCompoundBlock())
    if (p->bSendCompoundBlockCautious)
    {
        if ((bReturnValue) && (m_bIsNetworkReady))
        {
            // there were some data and network is still alive, we will wait for sending
            VERIFY(p->eventWaitForSend.Lock());
        }

        // unlock the event
        VERIFY(p->lockWaitForSend.Unlock());
    }

	return TRUE;
}


BOOL CNetworkConnection::EndSendCompoundBlockUnsafely(CNetworkConnection::SVirtualConnection *pVirtualConnection)
{
    // add "last fragment" information
    if (pVirtualConnection->btSendPacketFlags & PACKET_FRAGMENTED)
        pVirtualConnection->btSendPacketFlags |= PACKET_LAST_FRAGMENT;

    // set compound block as inactive
    pVirtualConnection->bSendCompoundBlockActive = FALSE;

    // have we some data?
    if (pVirtualConnection->dwSendCompoundBlockSize == 0)
    {
      // delete compound block buffer
      delete [] pVirtualConnection->pSendCompoundBlockBuffer;
      
      return FALSE;
    }
    
    // send data
    SendBlockUnsafely(pVirtualConnection, pVirtualConnection->pSendCompoundBlockBuffer, pVirtualConnection->dwSendCompoundBlockSize, pVirtualConnection->btSendCompoundBlockPriority, pVirtualConnection->bSendCompoundBlockCautious);
    
    // clear fragment bits in flags
    pVirtualConnection->btSendPacketFlags &= ~(PACKET_FRAGMENTED | PACKET_LAST_FRAGMENT);
    
	// delete compound block buffer
    delete [] pVirtualConnection->pSendCompoundBlockBuffer;

    // success
    return TRUE;
}


//////////////////////////////////////////////////////////////////////
// Compound block methods -- receiving
//////////////////////////////////////////////////////////////////////

BOOL CNetworkConnection::BeginReceiveCompoundBlock(WORD wConnectionNumber)
{
    // object must be valid
    ASSERT_VALID(this);

    // check the parameter
    ASSERT(wConnectionNumber > 0);
    if (wConnectionNumber == 0)
        return FALSE;

    // check the closing state
    ASSERT(!m_bClosing1);
    if (m_bClosing1)
        return FALSE;

    // lock virtual connection list
    VERIFY(m_lockVirtualConnectionList.Lock());
    ASSERT(m_pVirtualConnectionList != NULL);
    ASSERT(m_pVirtualConnectionList->wVirtualConnection == 0);

    // find given virtual connection
    SVirtualConnection *p = m_pVirtualConnectionList;
    while ((p != NULL) && (p->wVirtualConnection < wConnectionNumber))
        p = p->pNext;
    if ((p == NULL) || (p->wVirtualConnection > wConnectionNumber))
	{
		// unlock virtual connection list
		VERIFY(m_lockVirtualConnectionList.Unlock());
        return FALSE;
	}

    // paranoia
    ASSERT(p->wVirtualConnection == wConnectionNumber);
    
    // if there's already active receive compound block
    if (p->bReceiveCompoundBlockActive)
    {
        // unlock, exit
        VERIFY(m_lockVirtualConnectionList.Unlock());
        return FALSE;
    }

    if ((p->btSendPacketFlags & (PACKET_GUARANTEED_DELIVERY | PACKET_GUARANTEED_ORDER)) == PACKET_TYPE3)
    {
        // wait for packet (to get his serial number), 
        // this is needed for connection of type 3 only
        VERIFY(p->semReceiveDataReady.Lock());
        
        // lock received packet list 
        VERIFY(p->qReceivePackets.lock.Lock());
        ASSERT(p->qReceivePackets.first != NULL);
        // copy connection serial number
        p->dwReceiveLastSerial = p->qReceivePackets.first->dwConnectionSerial;
        // unlock received packet list
        VERIFY(p->qReceivePackets.lock.Unlock());
        
        // increase semaphore (we did not copy any data, just one serial number)
        VERIFY(p->semReceiveDataReady.Unlock());
    }

    // set receive compound block active
    p->bReceiveCompoundBlockActive = TRUE;
    // set stopper
    p->bReceiveCompoundBlockStopper = TRUE;
    // set offset in received data to zero
    p->dwReceiveCompoundBlockOffset = 0;

    // unlock everything, exit
    VERIFY(m_lockVirtualConnectionList.Unlock());

    return TRUE;
}


BOOL CNetworkConnection::EndReceiveCompoundBlock(WORD wConnectionNumber)
{
    // object must be valid
    ASSERT_VALID(this);

    // test the connection number
    ASSERT(wConnectionNumber != 0);
    if (wConnectionNumber == 0)
        return FALSE;

    // check the closing state
    ASSERT(!m_bClosing1);
    if (m_bClosing1)
        return FALSE;

    // lock virtual connection list
    VERIFY(m_lockVirtualConnectionList.Lock());
    ASSERT(m_pVirtualConnectionList != NULL);
    ASSERT(m_pVirtualConnectionList->wVirtualConnection == 0);

    // find given virtual connection
    SVirtualConnection *p = m_pVirtualConnectionList;
    while ((p != NULL) && (p->wVirtualConnection < wConnectionNumber))
        p = p->pNext;
    if ((p == NULL) || (p->wVirtualConnection > wConnectionNumber))
	{
		// unlock virtual connection list
		VERIFY(m_lockVirtualConnectionList.Unlock());
        return FALSE;
	}

    // paranoia
    ASSERT(p->wVirtualConnection == wConnectionNumber);
    
    // if compound block functionality is not enabled
    if (!p->bReceiveCompoundBlockActive)
    {
        // unlock, exit
        VERIFY(m_lockVirtualConnectionList.Unlock());
        return FALSE;
    }

    // decrease semaphore (packet is considered as received NOW)
    VERIFY(p->semReceiveDataReady.Lock());
    
    EndReceiveCompoundBlockUnsafely(p);

    // unlock everything, exit
    VERIFY(m_lockVirtualConnectionList.Unlock());

    return TRUE;
}


void CNetworkConnection::EndReceiveCompoundBlockUnsafely(CNetworkConnection::SVirtualConnection *pVirtualConnection)
{
    ASSERT(pVirtualConnection);
    
    // lock queue of received packets
    VERIFY(pVirtualConnection->qReceivePackets.lock.Lock());
    
    if (pVirtualConnection->bReceiveCompoundBlockStopper)
    {
        // there are still some unread data...

        ASSERT(pVirtualConnection->qReceivePackets.first != NULL);

        if (pVirtualConnection->qReceivePackets.first->btPacketFlags & PACKET_FRAGMENTED)
        {
            // delete all fragments
            while (!(pVirtualConnection->qReceivePackets.first->btPacketFlags & PACKET_LAST_FRAGMENT))
            {
                // save actual fragment
                SReceivePacketList *pDelete = pVirtualConnection->qReceivePackets.first;

                // delete this fragment from the list
                pVirtualConnection->qReceivePackets.first = pVirtualConnection->qReceivePackets.first->pNext;

                // there's no need to move the last pointer, only after the delete of the last fragment
                // assert just to prove it...
                ASSERT(pVirtualConnection->qReceivePackets.last != &(pDelete->pNext));

                // delete fragment
                delete [] pDelete->pBuffer;
                delete pDelete;

                // assert next iteration
                ASSERT(pVirtualConnection->qReceivePackets.first != NULL);
            }

            // delete last fragment (it has PACKET_LAST_FRAGMENT set in it)
            ASSERT(pVirtualConnection->qReceivePackets.first->btPacketFlags & PACKET_LAST_FRAGMENT);
            {
                // save actual fragment
                SReceivePacketList *pDelete = pVirtualConnection->qReceivePackets.first;

                // delete this fragment from the list
                pVirtualConnection->qReceivePackets.first = pVirtualConnection->qReceivePackets.first->pNext;

                // take care of the "last" pointer
                if (pVirtualConnection->qReceivePackets.last == &(pDelete->pNext))
                {
                    pVirtualConnection->qReceivePackets.last = &(pVirtualConnection->qReceivePackets.first);
                }

                // delete fragment
                delete [] pDelete->pBuffer;
                delete pDelete;
            }
        }
        else
        {
            // packet was not fragmented, delete only one piece

            // save actual fragment
            SReceivePacketList *pDelete = pVirtualConnection->qReceivePackets.first;
            
            // delete this fragment from the list
            pVirtualConnection->qReceivePackets.first = pVirtualConnection->qReceivePackets.first->pNext;
            
            // take care of the "last" pointer
            if (pVirtualConnection->qReceivePackets.last == &(pDelete->pNext))
            {
                pVirtualConnection->qReceivePackets.last = &(pVirtualConnection->qReceivePackets.first);
            }
                
            // delete fragment
            delete [] pDelete->pBuffer;
            delete pDelete;
        }
    }

    ASSERT(AfxIsValidAddress(pVirtualConnection->qReceivePackets.last, 4, TRUE) && ((*(pVirtualConnection->qReceivePackets.last) == NULL) || AfxIsMemoryBlock(*(pVirtualConnection->qReceivePackets.last), sizeof(SReceivePacketList))));
    
    // unlock queue of received packets
    VERIFY(pVirtualConnection->qReceivePackets.lock.Unlock());
    
    // reset compound block flag
    pVirtualConnection->bReceiveCompoundBlockActive = FALSE;
}


//////////////////////////////////////////////////////////////////////
// Get data size in the last packet of send compound block
//////////////////////////////////////////////////////////////////////

DWORD CNetworkConnection::GetActualDataSize(WORD wConnectionNumber)
{
	// object must be valid
    ASSERT_VALID(this);

    // check the connection number
    ASSERT(wConnectionNumber != 0);
    if (wConnectionNumber == 0)
        return 0;

    // lock virtual connection list
	VERIFY(m_lockVirtualConnectionList.Lock());
    ASSERT(m_pVirtualConnectionList != NULL);
    ASSERT(m_pVirtualConnectionList->wVirtualConnection == 0);

    // find given virtual connection
    SVirtualConnection *p = m_pVirtualConnectionList;
    while ((p != NULL) && (p->wVirtualConnection < wConnectionNumber))
        p = p->pNext;
    // if the send compound block is not active, return 0
    if ((p == NULL) || (p->wVirtualConnection > wConnectionNumber) || (!p->bSendCompoundBlockActive))
    {
        // unlock virtual connection list
        VERIFY(m_lockVirtualConnectionList.Unlock());
        return 0;
    }

    // copy return value
    DWORD dwReturnValue = p->dwSendCompoundBlockSize;

    // unlock virtual connection list
    VERIFY(m_lockVirtualConnectionList.Unlock());

    // that's all
    return dwReturnValue;
}


//////////////////////////////////////////////////////////////////////
// Send and receive block over TCP connection
//////////////////////////////////////////////////////////////////////

BOOL CNetworkConnection::SendTCP(const void *pBuffer, DWORD dwSize, int *err)
{
    ASSERT_VALID(this);

    // lock TCP send socket
    VERIFY(m_lockTCPSendSocket.Lock());
    
    int nRetriesLeft = NETWORK_LOSTCONNECTION_RETRIES;

    // send buffer; copy error code of network function to "err" argument
    while (send(m_TCPSendSocket, (const char *) pBuffer, dwSize, 0) == SOCKET_ERROR)
    {
        // copy the error code of send() function
        int nError = WSAGetLastError();

        if ((nRetriesLeft--) > 0)
        {
            TRACE_NETWORK("CNetworkConnection::SendTCP() - send() error %i, trying to recover\n", WSAGetLastError());

            // call RecoverConnection()
            if (RecoverConnection(E_RECOVER_SEND))
            {
                // success!!! 
                continue;
            }

            // recovering was not successful
            TRACE_NETWORK("CNetworkConnection::SendTCP() - recovering was not successful, aborting\n");
        }
        else
        {
           TRACE_NETWORK("CNetworkConnection::SendTCP() - send() error %i, no retries left\n", nError);
        }

        // unlock TCP send socket
        VERIFY(m_lockTCPSendSocket.Unlock());
        
        // copy the error code (if possible)
        if (err != NULL)
            *err = nError;
        
        return FALSE;
    }

    // set error code to "success"
    if (err != NULL)
        *err = 0;

    // unlock TCP send socket
    VERIFY(m_lockTCPSendSocket.Unlock());

    return TRUE;
}


BOOL CNetworkConnection::ReceiveTCP(void *pBuffer, DWORD dwSize, int *err)
{
    // object must be valid
    ASSERT_VALID(this);

    // actual return value of recv()
    int i;  
    // actual size of data read from the net
    DWORD j = 0;  

    // lock the receiving socket
    VERIFY(m_lockTCPRecvSocket.Lock());
    
    do
    {
        // number of retries for the recover operation
        int nRetriesLeft = NETWORK_LOSTCONNECTION_RETRIES;
        
        for ( ; ; )
        {
            i = recv(m_TCPRecvSocket, ((char *) pBuffer) + j, dwSize - j, 0);
            
            // was recv() successful?
            if ((i != SOCKET_ERROR) && (i != 0))
                break;

            // get error number
            int nError = WSAGetLastError();

            // check WSAEWOULDBLOCK error
            if (nError == WSAEWOULDBLOCK)
            {
                // return error immediately, caller will handle it properly
                
                // assert that we have not read ANY DATA...
                ASSERT(j == 0);
            }
            else
            {
                // do we have another try
                if ((nRetriesLeft--) > 0)
                {
                    TRACE_NETWORK("CNetworkConnection::ReceiveTCP() - recv() error %i %i, trying to recover\n", i, WSAGetLastError());
                    
                    // call RecoverConnection()
                    if (RecoverConnection(E_RECOVER_RECEIVE))
                    {
                        // success!!!
                        continue;
                    }
                    
                    // recovering was not successful
                    TRACE_NETWORK("CNetworkConnection::ReceiveTCP() - recovering was not successful, aborting\n");
                }
                else
                {
                    TRACE_NETWORK("CNetworkConnection::ReceiveTCP() - recv() error %i, no retries left\n", nError);
                }
            }

            // unlock TCP receiving socket
            VERIFY(m_lockTCPRecvSocket.Unlock());
            
            // copy the error code (if possible)
            if (err != NULL)
                *err = nError;
            
            return FALSE;
        }
        
        j += i;
    } while (j < dwSize);

    // unlock the receiving socket
    VERIFY(m_lockTCPRecvSocket.Unlock());

    // set the error code to "success"
    if (err != NULL)
        *err = 0;

    return TRUE;
}


//////////////////////////////////////////////////////////////////////
// Send Block
//////////////////////////////////////////////////////////////////////

BOOL CNetworkConnection::SendBlock(WORD wConnectionNumber, const void *pBuffer, DWORD dwBlockSize, BOOL bCautiousSend, BYTE btPacketPriority)
{
    // object must be valid
    ASSERT_VALID(this);

    // check the connection number
    ASSERT(wConnectionNumber > 0);
    if (wConnectionNumber == 0)
        return FALSE;

    // test the closing state
    ASSERT(!m_bClosing1);
    if (m_bClosing1)
        return FALSE;

    // lock virtual connection link list
    VERIFY(m_lockVirtualConnectionList.Lock());
    ASSERT(m_pVirtualConnectionList != NULL);
    ASSERT(m_pVirtualConnectionList->wVirtualConnection == 0);

    // find given virtual connection
    SVirtualConnection *p = m_pVirtualConnectionList;
    while ((p != NULL) && (p->wVirtualConnection < wConnectionNumber))
        p = p->pNext;

    // unlock the link list
    VERIFY(m_lockVirtualConnectionList.Unlock());

    if ((p == NULL) || (p->wVirtualConnection > wConnectionNumber))
    {
		// virtual connection does not exist
        return FALSE;
    }

    // paranoia
    ASSERT(p->wVirtualConnection == wConnectionNumber);

/*
    if (p->bDeleted)
    {
        // the peer closed the connection, let's pretend everything is OK
        return TRUE;
    }
*/

    // thread checking
    TRACE_MUTEXSENDING("CNetworkConnection::SendBlock() - trying to lock mutexSending (%i)\n", p->wVirtualConnection);
    VERIFY(p->mutexSending.Lock());
    TRACE_MUTEXSENDING("CNetworkConnection::SendBlock() - mutexSending locked (%i)\n", p->wVirtualConnection);
    
	// if user left actual priority, read it from virtual connection list
    if (btPacketPriority == PACKETPRIORITY_ACTUAL)
        btPacketPriority = p->btSendConnectionPriority;
	// check the validity of priority
    ASSERT(btPacketPriority <= PACKETPRIORITY_HIGHEST);

    if (p->bSendCompoundBlockActive)
    {
        // compound block is acitve, special actions have to be taken

        if (dwBlockSize == 0)
        {
            // unlock the mutex (thread checking)
            TRACE_MUTEXSENDING("CNetworkConnection::SendBlock() - trying to unlock mutexSending (%i)\n", p->wVirtualConnection);
            VERIFY(p->mutexSending.Unlock());
            TRACE_MUTEXSENDING("CNetworkConnection::SendBlock() - mutexSending unlocked (%i)\n", p->wVirtualConnection);
            return TRUE;
        }
        
        // check the validity of buffer
        ASSERT(::AfxIsValidAddress(pBuffer, dwBlockSize, FALSE));

        DWORD dwBufferPos = 0;
        while (dwBlockSize + p->dwSendCompoundBlockSize > m_dwMaximumMessageSize)
        {
            if (!(p->btSendPacketFlags & PACKET_GUARANTEED_DELIVERY))
            {
                // packet is greater than maximum message size, but fragmentation
                // is not allowed
                
                // unlock the mutex (thread checking)
                TRACE_MUTEXSENDING("CNetworkConnection::SendBlock() - trying to unlock mutexSending (%i)\n", p->wVirtualConnection);
                VERIFY(p->mutexSending.Unlock());
                TRACE_MUTEXSENDING("CNetworkConnection::SendBlock() - mutexSending unlocked (%i)\n", p->wVirtualConnection);
                return FALSE;
            }

            // packet should be sent, enable fragmentation
            if (!(p->btSendPacketFlags & PACKET_FRAGMENTED))
            {
                p->btSendPacketFlags |= PACKET_FRAGMENTED;
                p->dwSendFragmentNumber = 0;
			
                // if connection is type 2, modify the number of fragments
                if ((p->btSendPacketFlags & (PACKET_GUARANTEED_DELIVERY | PACKET_GUARANTEED_ORDER)) == PACKET_TYPE2)
                {
                    // add to highest byte fragmentation number
                    p->dwSendFragmentNumber |= p->dwSendTotalFragments;
                    // increase this number
                    p->dwSendTotalFragments += 0x01000000;
                }
            }

            // fill up the buffer
			memcpy(
				p->pSendCompoundBlockBuffer + p->dwSendCompoundBlockSize,  // dest
				(BYTE *) pBuffer + dwBufferPos,  // src
				m_dwMaximumMessageSize - p->dwSendCompoundBlockSize);  // size

            // send packet
            SendBlockUnsafely(p, p->pSendCompoundBlockBuffer, m_dwMaximumMessageSize, p->btSendCompoundBlockPriority, p->bSendCompoundBlockCautious);

            // move possition in source buffer
            dwBufferPos += m_dwMaximumMessageSize - p->dwSendCompoundBlockSize;
            // clean internal buffer possition
            p->dwSendCompoundBlockSize = 0;
            // decrease block size
            dwBlockSize -= m_dwMaximumMessageSize - p->dwSendCompoundBlockSize;
        }

        // now we know, that all data will fit to buffer, nothing more to send
        // copy the rest of data to buffer
        memcpy(
            p->pSendCompoundBlockBuffer + p->dwSendCompoundBlockSize,  // dest
            (BYTE *) pBuffer + dwBufferPos,  // src
            dwBlockSize);  // size

        // increase compound block buffer possition
        p->dwSendCompoundBlockSize += dwBlockSize;

        // thread checking
        TRACE_MUTEXSENDING("CNetworkConnection::SendBlock() - trying to unlock mutexSending (%i)\n", p->wVirtualConnection);
        VERIFY(p->mutexSending.Unlock());
        TRACE_MUTEXSENDING("CNetworkConnection::SendBlock() - mutexSending unlocked (%i)\n", p->wVirtualConnection);
        
        // and that's all
    }
    else
    {
        // single (not compound) block

        // cautious sending...
        if (bCautiousSend)
        {
            VERIFY(p->lockWaitForSend.Lock());
            VERIFY(p->eventWaitForSend.ResetEvent());
        }

        // check block size
        ASSERT(dwBlockSize > 0);
        // assert validity of input buffer
        ASSERT(::AfxIsValidAddress(pBuffer, dwBlockSize, FALSE));

        // is fragmentation needed?
        if (dwBlockSize > m_dwMaximumMessageSize)
        {
            // is fragmentation possible?
            if (!(p->btSendPacketFlags & PACKET_GUARANTEED_DELIVERY))
			{
				// unlock virtual connection list

                // unlock the mutex (thread checking)
                TRACE_MUTEXSENDING("CNetworkConnection::SendBlock() - trying to unlock mutexSending (%i)\n", p->wVirtualConnection);
                VERIFY(p->mutexSending.Unlock());
                TRACE_MUTEXSENDING("CNetworkConnection::SendBlock() - mutexSending unlocked (%i)\n", p->wVirtualConnection);
                return FALSE;
			}

            // add "fragmented" flag
            p->btSendPacketFlags |= PACKET_FRAGMENTED;
            
            // total number of fragments
            DWORD dwTotalFragments = dwBlockSize / m_dwMaximumMessageSize;
            
            // initialize fragment counter
            p->dwSendFragmentNumber = 0;
            
            // if connection is type 2, modify the number of fragments
            if ((p->btSendPacketFlags & (PACKET_GUARANTEED_DELIVERY | PACKET_GUARANTEED_ORDER)) == PACKET_TYPE2)
            {
                // add to highest byte fragmentation number
                p->dwSendFragmentNumber |= p->dwSendTotalFragments;
                // add to highest byte fragmentation number
                dwTotalFragments |= p->dwSendTotalFragments;
                // increase this number
                p->dwSendTotalFragments += 0x01000000;
            }
            
            // prepare last fragment size
            DWORD dwLastFragmentSize = dwBlockSize % m_dwMaximumMessageSize;
            if (dwLastFragmentSize > 0)
                dwTotalFragments++;
            else
                dwLastFragmentSize = m_dwMaximumMessageSize;

            // prepare offset in input buffer
            DWORD dwBufferOffset = 0;
            
            while (p->dwSendFragmentNumber < dwTotalFragments)
            {
                // if this is the last fragment
                if (p->dwSendFragmentNumber + 1 == dwTotalFragments)
                {
                    // set "last fragment" bit
                    p->btSendPacketFlags |= PACKET_LAST_FRAGMENT;
                    // send packet
                    SendBlockUnsafely(p, (BYTE *) pBuffer + dwBufferOffset, dwLastFragmentSize, btPacketPriority, bCautiousSend);
                }
                else
                {
                    // send packet
                    SendBlockUnsafely(p, (BYTE *) pBuffer + dwBufferOffset, m_dwMaximumMessageSize, btPacketPriority, bCautiousSend);
                    // increase offset
                    dwBufferOffset += m_dwMaximumMessageSize;
                }
            }
                
            p->btSendPacketFlags &= ~(PACKET_FRAGMENTED | PACKET_LAST_FRAGMENT);
        }
        else
        {
            // no fragmentation
			SendBlockUnsafely(p, pBuffer, dwBlockSize, btPacketPriority, bCautiousSend);
        }

        // release mutex (thread checking)
        TRACE_MUTEXSENDING("CNetworkConnection::SendBlock() - trying to unlock mutexSending (%i)\n", p->wVirtualConnection);
        VERIFY(p->mutexSending.Unlock());
        TRACE_MUTEXSENDING("CNetworkConnection::SendBlock() - mutexSending unlocked (%i)\n", p->wVirtualConnection);
        
        // cautious sending
        if (bCautiousSend)
        {
            // check the status of the network
            if (m_bIsNetworkReady)
            {
                VERIFY(p->eventWaitForSend.Lock());
            }

            VERIFY(p->lockWaitForSend.Unlock());
        }
    }

    return TRUE;
}


void CNetworkConnection::SendBlockUnsafely(CNetworkConnection::SVirtualConnection *pVirtualConnection, const void *pBuffer, DWORD dwBlockSize, BYTE btPacketPriority, BOOL bCautiousSend)
{
    // check input parameters
    ASSERT(pVirtualConnection != NULL);
    ASSERT(dwBlockSize > 0);
    ASSERT(::AfxIsValidAddress(pBuffer, dwBlockSize, FALSE));
    ASSERT(btPacketPriority <= PACKETPRIORITY_HIGHEST);

    // create packet list item
	SSendPacketList *p = new SSendPacketList;
    p->pNext = NULL;
    p->bCautiousSend = bCautiousSend;

	// switch by connection type
	switch (pVirtualConnection->btSendPacketFlags & (PACKET_GUARANTEED_DELIVERY | PACKET_GUARANTEED_ORDER | PACKET_FRAGMENTED))
	{
	case PACKET_TYPE1:
        // create packet
		(p->pPacket) = new(dwBlockSize) SUDPPacketType1;
        // set packet size to list item
		p->dwPacketSize = dwBlockSize + sizeof(SUDPPacketType1);
        // copy data
		memcpy(((SUDPPacketType1 *) p->pPacket)->Buffer, pBuffer, dwBlockSize);
        // copy additional packet parameters
		((SUDPPacketType1 *) p->pPacket)->ConnectionSerial = pVirtualConnection->dwSendSerial++;
		break;

	case PACKET_TYPE1F:
        // create packet
		p->pPacket = new(dwBlockSize) SUDPPacketType1F;
        // set packet size to list item
		p->dwPacketSize = dwBlockSize + sizeof(SUDPPacketType1F);
        // copy data
		memcpy(((SUDPPacketType1F *) p->pPacket)->Buffer, pBuffer, dwBlockSize);
        // copy additional packet parameters
		((SUDPPacketType1F *) p->pPacket)->ConnectionSerial = pVirtualConnection->dwSendSerial;
        if (pVirtualConnection->btSendPacketFlags & PACKET_LAST_FRAGMENT)
        {
            pVirtualConnection->dwSendSerial++;
        }
        else
        {
            // reset the cautious flag!!!
            p->bCautiousSend = FALSE;
        }
		((SUDPPacketType1F *) p->pPacket)->FragmentNumber = pVirtualConnection->dwSendFragmentNumber++;
		break;

	case PACKET_TYPE2:
        // create packet
		p->pPacket = new(dwBlockSize) SUDPPacketType2;
        // set packet size to list item
		p->dwPacketSize = dwBlockSize + sizeof(SUDPPacketType2);
        // copy data
		memcpy(((SUDPPacketType2 *) p->pPacket)->Buffer, pBuffer, dwBlockSize);
        // increase the counter of sent packets on this virtual connection
        pVirtualConnection->dwSendSerial++;
		break;

	case PACKET_TYPE2F:
        // create packet
		p->pPacket = new(dwBlockSize) SUDPPacketType2F;
        // set packet size to list item
		p->dwPacketSize = dwBlockSize + sizeof(SUDPPacketType2F);
        // copy data
		memcpy(((SUDPPacketType2F *) p->pPacket)->Buffer, pBuffer, dwBlockSize);
        if (pVirtualConnection->btSendPacketFlags & PACKET_LAST_FRAGMENT)
        {
            // this is last fragment, increase the counter of sent packets on this virtual connection
            pVirtualConnection->dwSendSerial++;
        }
        else
        {
            // reset the cautious flag!!! (if not the last fragment)
            p->bCautiousSend = FALSE;
        }
        // copy additional packet parameters
		((SUDPPacketType2F *) p->pPacket)->FragmentNumber = pVirtualConnection->dwSendFragmentNumber++;
		break;

	case PACKET_TYPE3:
        // create packet
		p->pPacket = new(dwBlockSize) SUDPPacketType3;
        // set packet size to list item
		p->dwPacketSize = dwBlockSize + sizeof(SUDPPacketType3);
        // copy data
		memcpy(((SUDPPacketType3 *) p->pPacket)->Buffer, pBuffer, dwBlockSize);
        // copy additional packet parameters
		((SUDPPacketType3 *) p->pPacket)->ConnectionSerial = pVirtualConnection->dwSendSerial++;
		break;

	case PACKET_TYPE4:
        // create packet
		p->pPacket = new(dwBlockSize) SUDPPacketType4;
        // set packet size to list item
		p->dwPacketSize = dwBlockSize + sizeof(SUDPPacketType4);
        // copy data
		memcpy(((SUDPPacketType4 *) p->pPacket)->Buffer, pBuffer, dwBlockSize);
		break;

    default:
        // SHOULDN'T BE REACHED
        TRACE_NETWORK("Wrong packet type\n");
        ASSERT(FALSE);
	}

	// copy packet flags
	p->pPacket->PacketFlags = pVirtualConnection->btSendPacketFlags;
	
	// copy packet priority
	p->pPacket->PacketPriority = btPacketPriority;

	// copy virtual connection number
	p->pPacket->VirtualConnection = pVirtualConnection->wVirtualConnection;

	// lock queue for outgoing packets of given priority
	VERIFY(m_qPacketsPreparedToSend[btPacketPriority].lock.Lock());

	// add packet to the end of queue
    ASSERT(*(m_qPacketsPreparedToSend[btPacketPriority].last) == NULL);
	*(m_qPacketsPreparedToSend[btPacketPriority].last) = p;
    m_qPacketsPreparedToSend[btPacketPriority].last = &(p->pNext);
    ASSERT(*(m_qPacketsPreparedToSend[btPacketPriority].last) == NULL);

	// unlock queue for outgoing packets
	VERIFY(m_qPacketsPreparedToSend[btPacketPriority].lock.Unlock());

    // increase the value of semaphore
	VERIFY(m_semPacketsPreparedToSend.Unlock());
}


//////////////////////////////////////////////////////////////////////
// Receive Block
//////////////////////////////////////////////////////////////////////

BOOL CNetworkConnection::ReceiveBlock(WORD wConnectionNumber, void *pBuffer, DWORD &dwBlockSize)
{
    // object must be valid
    ASSERT_VALID(this);

    // check the connection number
    ASSERT(wConnectionNumber != 0);
    if (wConnectionNumber == 0)
        return FALSE;

    // check the closing state
    ASSERT(!m_bClosing1);
    if (m_bClosing1)
        return FALSE;

	// lock virtual connection list
    VERIFY(m_lockVirtualConnectionList.Lock());
    ASSERT(m_pVirtualConnectionList != NULL);
    ASSERT(m_pVirtualConnectionList->wVirtualConnection == 0);

    // find given virtual connection
    SVirtualConnection *p = m_pVirtualConnectionList;
    while ((p != NULL) && (p->wVirtualConnection < wConnectionNumber))
        p = p->pNext;

    // connection does not exist
    if ((p == NULL) || (p->wVirtualConnection > wConnectionNumber))
    {
        // unlock virtual connectoin list
        VERIFY(m_lockVirtualConnectionList.Unlock());
        TRACE_NETWORK("CNetworkConnection::ReceiveBlock() - virtual connection %i does not exist\n", wConnectionNumber);
        return FALSE;
    }

    // paranoia
    ASSERT(p->wVirtualConnection == wConnectionNumber);

    // wait for data (decrease semaphore)
    VERIFY(p->semReceiveDataReady.Lock());

    // lock queue of received packets 
    VERIFY(p->qReceivePackets.lock.Lock());

    if (p->bReceiveCompoundBlockActive)
    {
        // compound block enabled

        // it is possible to call receive repeatedly on one block
        // and get small pieces of this (probably huge and fragmented) block

        // function will success at the end of compound block but 
        // will return zero bytes
        
        // increase semaphore to allow next receive
        VERIFY(p->semReceiveDataReady.Unlock());

        // check the size the application wants to receive
        if (dwBlockSize == 0)
        {
            // unlock the queue of received packets
            VERIFY(p->qReceivePackets.lock.Unlock());

            // unlock the list of virtual connections
            VERIFY(m_lockVirtualConnectionList.Unlock());

            return TRUE;
        }

        // check the validity of receive buffer
        ASSERT(::AfxIsValidAddress(pBuffer, dwBlockSize, TRUE));
        
        // byte counter
        DWORD dwBytesCopied = 0;
        
        // assert the first packet
        ASSERT(!p->bReceiveCompoundBlockStopper || p->qReceivePackets.first != NULL);
        
        while (p->bReceiveCompoundBlockStopper && (dwBlockSize >= (p->qReceivePackets.first->dwDataSize - p->dwReceiveCompoundBlockOffset)))
        {
            // not enough data in this packet (or exactly equal to the desired amount)
            // => actual packet will be deleted from queue
            
            // reset stopper, if we stand on the last fragment
            if (!(p->qReceivePackets.first->btPacketFlags & PACKET_FRAGMENTED) ||
                (p->qReceivePackets.first->btPacketFlags & PACKET_LAST_FRAGMENT))
                p->bReceiveCompoundBlockStopper = FALSE;

            // store the pointer to the first item in the list
            SReceivePacketList *r = p->qReceivePackets.first;

            // delete the first from the list
            p->qReceivePackets.first = p->qReceivePackets.first->pNext;
            
            // take care of "last"
            if (p->qReceivePackets.last == &(r->pNext))
            {
                p->qReceivePackets.last = &(p->qReceivePackets.first);
            }

            // copy data
            memcpy((BYTE *) pBuffer + dwBytesCopied, r->pBuffer + p->dwReceiveCompoundBlockOffset, (r->dwDataSize - p->dwReceiveCompoundBlockOffset));
            // increase copied size
            dwBytesCopied += r->dwDataSize - p->dwReceiveCompoundBlockOffset;
            // set offset to zero
            p->dwReceiveCompoundBlockOffset = 0;
            // decrease number of bytes wanted
            dwBlockSize -= r->dwDataSize - p->dwReceiveCompoundBlockOffset;

            // delete packet
            delete [] r->pBuffer;
            delete r;

            // assert next iteration
            ASSERT(!p->bReceiveCompoundBlockStopper || (p->qReceivePackets.first != NULL));
        }

        if ((p->bReceiveCompoundBlockStopper) && (dwBlockSize > 0))
        {
            // check the validity of the pointer
            ASSERT(p->qReceivePackets.first != NULL);

            // actual packet will be left in the queue
            // copy part only
            memcpy((BYTE *) pBuffer + dwBytesCopied, p->qReceivePackets.first->pBuffer + p->dwReceiveCompoundBlockOffset, dwBlockSize);
            // increase copied size
            dwBytesCopied += dwBlockSize;
            // increase offset in actual packet
            p->dwReceiveCompoundBlockOffset += dwBlockSize;
        }

        // return proper size to user
        dwBlockSize = dwBytesCopied;
    }       
    else
    {
        // compound block disabled

        // check first item in list of packets
        ASSERT(p->qReceivePackets.first != NULL);

        // copy connection serial number (needed for type 3 connections ONLY)
        if (p->qReceivePackets.first->btPacketFlags == PACKET_TYPE3)
            p->dwReceiveLastSerial = p->qReceivePackets.first->dwConnectionSerial;

        if (p->qReceivePackets.first->btPacketFlags & PACKET_FRAGMENTED)
        {
            // fragmented packet, must compute size and copy all fragments to the user buffer
            DWORD dwTotalSize = 0;
            {
                SReceivePacketList *r = p->qReceivePackets.first;
                BOOL bStopper = TRUE;
                while (bStopper) 
                {
                    ASSERT(r != NULL);

                    // should be this packet last?
                    if (r->btPacketFlags & PACKET_LAST_FRAGMENT)
                        bStopper = FALSE;
                    
                    // increase total size
                    dwTotalSize += r->dwDataSize;
                    // jump to next packet
                    r = r->pNext;
                };
            }

            // compare to user buffer size
            if (dwTotalSize > dwBlockSize)
            {
                // copy size (user can prepare bigger buffer)
                dwBlockSize = dwTotalSize;

                // increase semaphore (no packet copied, everything left in queue)
                VERIFY(p->semReceiveDataReady.Unlock());
                
                // unlock everything
                VERIFY(p->qReceivePackets.lock.Unlock());

                // unlock the list of virtual connections
                VERIFY(m_lockVirtualConnectionList.Unlock());

                return FALSE;
            }

            // check the validity of buffer
            ASSERT(::AfxIsValidAddress(pBuffer, dwBlockSize, TRUE));
            
            // size is adequate; copy size
            dwBlockSize = dwTotalSize;

            // copy data, delete packets
            {
                DWORD dwOffset = 0;
                SReceivePacketList *q;
                BOOL bStopper = TRUE;

                while (bStopper)
                {
                    ASSERT(p->qReceivePackets.first);

                    // is this packet the last in the block?
                    if (p->qReceivePackets.first->btPacketFlags & PACKET_LAST_FRAGMENT)
                        bStopper = FALSE;

                    // copy data
                    memcpy((BYTE *) pBuffer + dwOffset, p->qReceivePackets.first->pBuffer, p->qReceivePackets.first->dwDataSize);
                    // increase offset in output buffer
                    dwOffset += p->qReceivePackets.first->dwDataSize;

                    // save pointer to the first item
                    q = p->qReceivePackets.first;
                    
                    // delete the first item from the list
                    p->qReceivePackets.first = p->qReceivePackets.first->pNext;

                    // take care of "last"
                    if (p->qReceivePackets.last == &(q->pNext))
                    {   
                        p->qReceivePackets.last = &(p->qReceivePackets.first);
                    }

                    // delete packet
                    delete [] q->pBuffer;
                    delete q;
                }
            }
        }
        else
        {
            // packet is not fragmented

            // check packet size
            if (p->qReceivePackets.first->dwDataSize > dwBlockSize)
            {
                // copy size (user can prepare bigger buffer)
                dwBlockSize = p->qReceivePackets.first->dwDataSize;

                // increase semaphore (no packet was copied, everything left in queue)
                VERIFY(p->semReceiveDataReady.Unlock());
                
                // unlock the queue of received packets
                VERIFY(p->qReceivePackets.lock.Unlock());

                // unlock the list of virtual connections
                VERIFY(m_lockVirtualConnectionList.Unlock());
                
                // not enough space, fail
                return FALSE;
            }

            // check the validity of buffer
            ASSERT(::AfxIsValidAddress(pBuffer, dwBlockSize, TRUE));

            // copy size
            dwBlockSize = p->qReceivePackets.first->dwDataSize;

            // copy data
            memcpy(pBuffer, p->qReceivePackets.first->pBuffer, dwBlockSize);

            // save the pointer to the first item
            SReceivePacketList *r = p->qReceivePackets.first;

            // delete the first item from the list
            p->qReceivePackets.first = p->qReceivePackets.first->pNext;
            
            // take care of the pointer to the last packet
            if (p->qReceivePackets.last == &(r->pNext))
            {
                p->qReceivePackets.last = &(p->qReceivePackets.first);
            }

            // delete the first packet
            delete [] r->pBuffer;
            delete r;
        }
    }

    ASSERT(::AfxIsValidAddress(p->qReceivePackets.last, 4, TRUE) && ((*(p->qReceivePackets.last) == NULL) || AfxIsMemoryBlock(*(p->qReceivePackets.last), sizeof(SReceivePacketList))));

    // unlock received packets queue
    VERIFY(p->qReceivePackets.lock.Unlock());

    // unlock virtual connection list
    VERIFY(m_lockVirtualConnectionList.Unlock());

    return TRUE;
}


//////////////////////////////////////////////////////////////////////
// Get/Set virtual connection priority
//////////////////////////////////////////////////////////////////////

BYTE CNetworkConnection::GetVirtualConnectionPriority(WORD wConnectionNumber)
{
    // object must be valid
    ASSERT_VALID(this);

    // check the connection number
    ASSERT(wConnectionNumber != 0);
    if (wConnectionNumber == 0)
        return 0;

    // lock virtual connection list
	VERIFY(m_lockVirtualConnectionList.Lock());
    ASSERT(m_pVirtualConnectionList != NULL);
    ASSERT(m_pVirtualConnectionList->wVirtualConnection == 0);

    // find the right virtual connection
    SVirtualConnection *p = m_pVirtualConnectionList;
    while ((p != NULL) && (p->wVirtualConnection < wConnectionNumber))
        p = p->pNext;
    if ((p == NULL) || (p->wVirtualConnection > wConnectionNumber))
	{
		// unlock virtual connection list
		VERIFY(m_lockVirtualConnectionList.Unlock());
        return PACKETPRIORITY_ACTUAL;
	}

    // paranoia
    ASSERT(p->wVirtualConnection == wConnectionNumber);

	// copy return value from list element
	BYTE return_value = p->btSendConnectionPriority;

	// unlock virtual connection list
	VERIFY(m_lockVirtualConnectionList.Unlock());

    return return_value;
}


BOOL CNetworkConnection::SetVirtualConnectionPriority(WORD wConnectionNumber, BYTE btBasePriority)
{
    // object must be valid
    ASSERT_VALID(this);

    // check the connection number
    ASSERT(wConnectionNumber != NULL);
    if (wConnectionNumber == 0)
        return FALSE;

    // check the priority
    ASSERT(btBasePriority <= PACKETPRIORITY_HIGHEST);
    if (btBasePriority > PACKETPRIORITY_HIGHEST)
        return FALSE;

    // test the closing state
    ASSERT(!m_bClosing1);
    if (m_bClosing1)
        return FALSE;

    // lock virtual connection list
	VERIFY(m_lockVirtualConnectionList.Lock());
    ASSERT(m_pVirtualConnectionList != NULL);
    ASSERT(m_pVirtualConnectionList->wVirtualConnection == 0);
	
    // find the right virtual connection
	SVirtualConnection *p = m_pVirtualConnectionList;
    while ((p != NULL) && (p->wVirtualConnection < wConnectionNumber))
        p = p->pNext;
    if ((p == NULL) || (p->wVirtualConnection > wConnectionNumber))
	{
		// unlock virtual connection list
		VERIFY(m_lockVirtualConnectionList.Unlock());
        return FALSE;
	}

    // paranoia
    ASSERT(p->wVirtualConnection == wConnectionNumber);

    p->btSendConnectionPriority = btBasePriority;

	// unlock virtual connection list
	VERIFY(m_lockVirtualConnectionList.Unlock());
    
	return TRUE;
}


//////////////////////////////////////////////////////////////////////
// Recover lost network connection
//////////////////////////////////////////////////////////////////////

BOOL CNetworkConnection::RecoverConnection(enum CNetworkConnection::ERecoverConnection /*dwWhatRecover*/)
{
    // this method is just a dream...
    // imagine all those TCP packets cut in half...
    // imagine all those threads confused...
    // imagine all those thousands lines of code written here to save the world...

    // we were not successful
    TRACE_NETWORK("CNetworkConnection::RecoverConnection() - not implemented\n");
    return FALSE;
}


//////////////////////////////////////////////////////////////////////
// Statistic methods
//////////////////////////////////////////////////////////////////////

void CNetworkConnection::GetNetworkStatistics(CNetworkConnection::SNetworkStatistics &sStats)
{
    ASSERT_VALID(this);
    VERIFY(m_lockNetworkStatistics.Lock());
    sStats = m_sNetworkStatistics;
    VERIFY(m_lockNetworkStatistics.Unlock());
}


//////////////////////////////////////////////////////////////////////
// Debug methods
//////////////////////////////////////////////////////////////////////

#ifdef _DEBUG

void CNetworkConnection::AssertValid() const
{
    CObject::AssertValid();

    ASSERT(m_bInitialized);
    ASSERT_KINDOF(CNetwork, m_pNetworkObject);
}


void CNetworkConnection::Dump(CDumpContext &dc) const
{
    CObject::Dump(dc);
    if (m_bInitialized)
    {
        dc << _T("CNetworkConnection object:") << endl;
        
        // TODO: add dumping here...
    }
    else
    {
        dc << _T("CNetworkConnection object is not initialized") << endl;
    }
}

#endif  //  _DEBUG


//////////////////////////////////////////////////////////////////////
// Thread function -- UDP sender
//////////////////////////////////////////////////////////////////////

// return values:
//    0: normal exit
//    1: WaitForMultipleObjects() failure

UINT CNetworkConnection::NetworkUDPSenderThread(LPVOID pParam)
{
    CNetworkConnection *p = (CNetworkConnection *) pParam;
    ASSERT(p);

    HANDLE aHandles[2];
    aHandles[0] = p->m_eventSenderKill;
    aHandles[1] = p->m_semPacketsPreparedToSend;

    DWORD dwSleepTime = MIN_SENDER_SLEEP_TIME;

    for ( ; ; )
    {
        switch (WaitForMultipleObjects(2, aHandles, FALSE, INFINITE))
        {
        case WAIT_FAILED:
            // error
            {
                TRACE_NETWORK("CNetworkConnection::NetworkUDPSenderThread() - wait function failed, error %i\n", GetLastError());

                // send event to the application
                p->SendErrorEvent(E_WAITERROR, eSenderThread);
            }
            return 1;

        case WAIT_OBJECT_0:
            // kill event activated
            {
                TRACE_NETWORK("CNetworkConnection::NetworkUDPSenderThread() - exiting...\n");
            }
            return 0;

        case WAIT_OBJECT_0 + 1:
            // something to send
            {
                // variable i must be signed! because PACKETPRIORITY_LOWEST is 0
                for (int i = PACKETPRIORITY_HIGHEST; i >= PACKETPRIORITY_LOWEST; i--)
                {
                    // paranoia?
                    ASSERT((i <= PACKETPRIORITY_HIGHEST) && (i >= PACKETPRIORITY_LOWEST));

                    // lock the queue
                    VERIFY(p->m_qPacketsPreparedToSend[i].lock.Lock());
                    
                    // if the queue is empty
                    if (p->m_qPacketsPreparedToSend[i].first == NULL)
                    {
                        // unlock it
                        VERIFY(p->m_qPacketsPreparedToSend[i].lock.Unlock());
                        // and continue on the next queue (with lower priority)
                        continue;
                    }
                    
                    // queue is not empty, test the length of the sent packets link list 
                    // (don't test it for the highest priority)
                    if ((i < PACKETPRIORITY_HIGHEST) &&
                        ((p->m_qPacketsSent.dwSentPacketsNumber > INIT_SENTPACKETS_NUMBER) ||
                        (p->m_qPacketsSent.dwSentPacketsSize > INIT_SENTPACKETS_SIZE)))
                    {
                        // sent packets link list is too long, pause sending for a while...

#ifdef _DEBUG
                        // TRACE the reason why it is necessary to sleep
                        if (p->m_qPacketsSent.dwSentPacketsNumber > INIT_SENTPACKETS_NUMBER)
                        {
                            TRACE_NETWORK("CNetworkConnection::NetworkUDPSenderThread() - too many sent packets in the link list, sleeping...\n");
                        }
                        else
                        {
                            TRACE_NETWORK("CNetworkConnection::NetworkUDPSenderThread() - the link list of sent packets is too big, sleeping...\n");
                        }
#endif

                        // unlock the queue
                        VERIFY(p->m_qPacketsPreparedToSend[i].lock.Unlock());
                        // increase semaphore (packet will not be send)
                        VERIFY(p->m_semPacketsPreparedToSend.Unlock());
                        
                        // send event to application
                        ASSERT(p->m_pObserver != NULL);
                        p->InlayEvent(E_NETWORKCONGESTION, eSenderThread, p->m_pObserver);
                        
                        // sleep some time
                        Sleep(dwSleepTime);
                        // get out
                        break;
                    }

                    // create pointer to packet being processed
                    SSendPacketList *q = p->m_qPacketsPreparedToSend[i].first;
                    
                    // delete processed packet from queue
                    p->m_qPacketsPreparedToSend[i].first = p->m_qPacketsPreparedToSend[i].first->pNext;
                    if (p->m_qPacketsPreparedToSend[i].first == NULL)
                        p->m_qPacketsPreparedToSend[i].last = &(p->m_qPacketsPreparedToSend[i].first);
                    
                    // just to be sure...
                    ASSERT(*(p->m_qPacketsPreparedToSend[i].last) == NULL);
                    
                    // unlock queue
                    VERIFY(p->m_qPacketsPreparedToSend[i].lock.Unlock());
                    
                    if (q->pPacket->PacketFlags & PACKET_GUARANTEED_DELIVERY)
                    {
                        // actualize serial number in packet (if needed)
                        switch (q->pPacket->PacketFlags & (PACKET_GUARANTEED_DELIVERY | PACKET_GUARANTEED_ORDER | PACKET_FRAGMENTED))
                        {
                        case PACKET_TYPE1:
                            VERIFY(p->m_lockTotalSerial.Lock());
                            q->dwTotalSerialNumber = p->m_dwTotalSerial;
                            ((SUDPPacketType1 *) q->pPacket)->TotalSerial = p->m_dwTotalSerial++;
                            VERIFY(p->m_lockTotalSerial.Unlock());
                            break;
                            
                        case PACKET_TYPE1F:
                            VERIFY(p->m_lockTotalSerial.Lock());
                            q->dwTotalSerialNumber = p->m_dwTotalSerial;
                            ((SUDPPacketType1F *) q->pPacket)->TotalSerial = p->m_dwTotalSerial++;
                            VERIFY(p->m_lockTotalSerial.Unlock());
                            break;
                            
                        case PACKET_TYPE2:
                            VERIFY(p->m_lockTotalSerial.Lock());
                            q->dwTotalSerialNumber = p->m_dwTotalSerial;
                            ((SUDPPacketType2 *) q->pPacket)->TotalSerial = p->m_dwTotalSerial++;
                            VERIFY(p->m_lockTotalSerial.Unlock());
                            break;
                            
                        case PACKET_TYPE2F:
                            VERIFY(p->m_lockTotalSerial.Lock());
                            q->dwTotalSerialNumber = p->m_dwTotalSerial;
                            ((SUDPPacketType2F *) q->pPacket)->TotalSerial = p->m_dwTotalSerial++;
                            VERIFY(p->m_lockTotalSerial.Unlock());
                            break;
                        }
                    }
                    
                    // send packet over network
                    VERIFY(p->m_lockUDPSendSocket.Lock());
                    int send_result = send(p->m_UDPSendSocket, (char *) q->pPacket, q->dwPacketSize, 0);
                    VERIFY(p->m_lockUDPSendSocket.Unlock());
                    
                    if (send_result == SOCKET_ERROR)
                    {
                        // this error should be ignored, we can do NOTHING
                        // this is equivalent to the lost packet on the network

                        // report it for debugging...
                        TRACE_NETWORK("CNetworkConnection::NetworkUDPSenderThread() - send() error %i\n", WSAGetLastError());

                        // and continue in a good job
                    }

                    // wake up the sleeping SendBlock()
                    if (q->bCautiousSend)
                    {
                        VERIFY(p->m_lockVirtualConnectionList.Lock());
                        ASSERT(p->m_pVirtualConnectionList != NULL);
                        ASSERT(p->m_pVirtualConnectionList->wVirtualConnection == 0);

                        SVirtualConnection *r = p->m_pVirtualConnectionList;
                        while ((r != NULL) && (r->wVirtualConnection < q->pPacket->VirtualConnection))
                        {
                            r = r->pNext;
                        }
                        if ((r != NULL) && (r->wVirtualConnection == q->pPacket->VirtualConnection))
                        {
                            // set event
                            VERIFY(r->eventWaitForSend.SetEvent());
                        }
/*
                        else
                        {
                            // that's not goot, we didn't find the virtual connection!
                            TRACE_NETWORK("CNetworkConnection::NetworkUDPSenderThread() - can't find virtual connection, can't wake up waiting SendBlock()\n");
                            ASSERT(FALSE);
                        }
*/
                        VERIFY(p->m_lockVirtualConnectionList.Unlock());
                    }

                    // has packet guaranteed delivery?
                    if (q->pPacket->PacketFlags & PACKET_GUARANTEED_DELIVERY)
                    {
                        // yes, packet has to be added to queue of sent packets
                        VERIFY(p->m_qPacketsSent.lock.Lock());
                        ASSERT(*(p->m_qPacketsSent.last) == NULL);  // non-NULL value means that we overwrite some packet...
                        *(p->m_qPacketsSent.last) = q;
                        q->pNext = NULL;
                        p->m_qPacketsSent.last = &(q->pNext);

                        // actualize the length of sent packets link list
                        p->m_qPacketsSent.dwSentPacketsNumber++;
                        p->m_qPacketsSent.dwSentPacketsSize += q->dwPacketSize;

                        VERIFY(p->m_qPacketsSent.lock.Unlock());
                    }
                    else
                    {
                        // no, delete packet
                        switch (q->pPacket->PacketFlags)
                        {
                        case PACKET_TYPE3:
                            delete q->pPacket;
                            break;
                            
                        case PACKET_TYPE4:
                        case PACKET_SYNCHRONIZATION:
                            delete q->pPacket;
                            break;
                            
                        default:
                            // this code shouldn't be reached
                            TRACE_NETWORK("CNetworkConnection::NetworkUDPSenderThread() - wrong packet type!\n");
                            ASSERT(FALSE);
                            break;
                        }
                        
                        delete q;
                    }
                    
                    // break the for-cycle, because the packet was sent
                    // (ie. let's start sending packets of the highest
                    // priority again)
                    break;
                }

                // test the closing state of the network connection
                if (i < PACKETPRIORITY_LOWEST)
                {
                    // assert it is TRUE (otherwise the semaphore was not handled properly)
                    ASSERT(p->m_bClosing1);

                    // set the event
                    VERIFY(p->m_eventClosingSender.SetEvent());
                }
            }
            break;

        default:
            // SHOULD NOT BE REACHED
            TRACE_NETWORK("CNetworkConnection::NetworkUDPSenderThread() - invalid return code for WaitForMultipleObjects()\n");
            ASSERT(FALSE);
        }
    } // endless loop

    // this code should never be reached
    TRACE_NETWORK("CNetworkConnection::NetworkUDPSenderThread() - wrong code reached\n");
    ASSERT(FALSE);
    return 0;
}


//////////////////////////////////////////////////////////////////////
// Thread function -- UDP receiver
//////////////////////////////////////////////////////////////////////

// return values:
//    0: normal exit
//    1: network error (incl. WSAEventSelect() error)
//    2: memory allocation failure
//    3: WaitForMultipleObjects() failure

UINT CNetworkConnection::NetworkUDPReceiverThread(LPVOID pParam)
{
    // pointer to CNetworkConnection object
    CNetworkConnection *p = (CNetworkConnection *) pParam;
    ASSERT(p);

    // pointer to a receiver buffer
    BYTE *pBuffer = p->m_pReceiverThreadBuffer;
    ASSERT(::AfxIsValidAddress(pBuffer, p->m_dwMaximumPacketSize, TRUE));

    // prepare an event variable
    CEvent eventUDPPacketArrived;

    // attach socket to event
    if (WSAEventSelect(p->m_UDPRecvSocket, eventUDPPacketArrived, FD_READ) == SOCKET_ERROR)
    {
        // error?
        int nError = WSAGetLastError();
        TRACE_NETWORK("CNetworkConnection::NetworkUDPReceiverThread() - WSAEventSelect() error %i\n", nError);

        // send event to the application
        p->SendErrorEvent(E_NETWORKEVENTSELECTERROR, eReceiverThread);

        // end thread
        return 1;
    }

    HANDLE aHandles[2];
    aHandles[0] = p->m_eventReceiverKill;
    aHandles[1] = eventUDPPacketArrived;

    for ( ; ; )
    {
        switch (WaitForMultipleObjects(2, aHandles, FALSE, INFINITE))
        {
        case WAIT_FAILED:
            // error
            {
                TRACE_NETWORK("CNetworkConnection::NetworkUDPReceiverThread() - wait function failed\n");
                
                // send event to the application
                p->SendErrorEvent(E_WAITERROR, eReceiverThread);
            }
            return 3;

        case WAIT_OBJECT_0:
            // kill event was set
            {
                TRACE_NETWORK("CNetworkConnection::NetworkUDPReceiverThread() - exiting...\n");
            }
            return 0;

        case WAIT_OBJECT_0 + 1:
            // data on network ready (probably)
            {
                // prepare network events structure                
                WSANETWORKEVENTS sNetEvents;

                // find out the exact reason of event
                if (WSAEnumNetworkEvents(p->m_UDPRecvSocket, aHandles[1], &sNetEvents) == SOCKET_ERROR)
                {
                    // some error occured

                    // TODO: what to do with this???
                    TRACE_NETWORK("CNetworkConnection::NetworkUDPReceiverThread() - WSAEnumNetworkEvents() error %i\n", WSAGetLastError());
                    ASSERT(FALSE);
                }

                if (sNetEvents.lNetworkEvents & FD_READ)
                {
                    
                    int recv_result = recv(p->m_UDPRecvSocket, (char *) pBuffer, p->m_dwMaximumPacketSize, 0);
                    
                    // if receive has failed
                    if (recv_result == SOCKET_ERROR)
                    {
                        // get error number
                        int nError = WSAGetLastError();
                        
                        if (nError == WSAEWOULDBLOCK)
                        {
                            // data not ready, just wait...
                            break;
                        }
                        else
                        {
                            // some other error
                            TRACE_NETWORK("CNetworkConnection::NetworkUDPReceiverThread() - recv() error %i\n", nError);
                            
                            // send event to application 
                            p->SendErrorEvent(E_NETWORKERROR, eReceiverThread);
                            
                            // end thread
                            return 1;
                        }
                    }
                    
                    // check minimal packet size        
                    if (recv_result < sizeof(SUDPPacketCommonHeader))
                    {
                        // packet is too small, discard it and receive something else...
                        TRACE_NETWORK("CNetworkConnection::NetworkUDPReceiverThread() - too small packet arrived, discarding...\n");
                        ASSERT(FALSE);
                        continue;
                    }
                    
                    // UDP alive checking
                    p->m_bUDPPacketReceived = TRUE;
                    
                    // check if it is synchronization packet
                    if (((SUDPPacketCommonHeader *) pBuffer)->PacketFlags == (PACKET_SYNCHRONIZATION | PACKET_TYPE4))
                    {
                        // copy the content of the synchronization packet
                        SSynchronizationPacket *pSynchroPacket = (SSynchronizationPacket *) (((SUDPPacketType4 *) pBuffer)->Buffer);
                        // TRACE_NETWORK("CNetworkConnection::NetworkUDPReceiverThread() - synchronization packet received (last awaited packet %i, last good delivered packet %i)\n", pSynchroPacket->dwTotalSerial, pSynchroPacket->dwLastGoodSerial);
                        
                        // copy the serial number of the last awaited packet
                        VERIFY(p->m_lockLastReceivedTotalSerial.Lock());
                        if (pSynchroPacket->dwTotalSerial > p->m_dwLastReceivedTotalSerial3)
                        {
                            // TRACE_NETWORK("CNetworkConnection::NetworkUDPReceiverThread() - the number of the last awaited packet was adjusted\n");
                            p->m_dwLastReceivedTotalSerial3 = pSynchroPacket->dwTotalSerial;
                        }
                        VERIFY(p->m_lockLastReceivedTotalSerial.Unlock());
                        
                        // now delete the beginning of the link list of sent packets
                        
                        // lock sent packets queue
                        VERIFY(p->m_qPacketsSent.lock.Lock());

                        while ((p->m_qPacketsSent.first != NULL) && (p->m_qPacketsSent.first->dwTotalSerialNumber <= pSynchroPacket->dwLastGoodSerial))
                        {
                            // copy a pointer to the first packet
                            SSendPacketList *q = p->m_qPacketsSent.first;

                            // move the pointer to the second packet in the link list
                            p->m_qPacketsSent.first = q->pNext;

                            // the length of sent packets link list has changed, adjust counters
                            ASSERT(p->m_qPacketsSent.dwSentPacketsNumber > 0);
                            p->m_qPacketsSent.dwSentPacketsNumber--;
                            ASSERT(p->m_qPacketsSent.dwSentPacketsSize >= q->dwPacketSize);
                            p->m_qPacketsSent.dwSentPacketsSize -= q->dwPacketSize;
                            
                            // delete the packet
                            delete q->pPacket;
                            delete q;
                        }

                        // take care of the "last" pointer
                        if (p->m_qPacketsSent.first == NULL)
                        {
                            // let the "last" pointer point to the first
                            p->m_qPacketsSent.last = &(p->m_qPacketsSent.first);

                            // are we in the closing state?
                            if (p->m_bClosing2)
                            {
                                // set the event
                                VERIFY(p->m_eventClosingSentPackets.SetEvent());
                            }
                        }

                        // "last" must point to "NULL" always...
                        ASSERT(*(p->m_qPacketsSent.last) == NULL);
                        
                        // unlock sent packets queue
                        VERIFY(p->m_qPacketsSent.lock.Unlock());

                        // receive next packet
                        continue;
                    }
                    
                    if (((SUDPPacketCommonHeader *) pBuffer)->PacketFlags & PACKET_GUARANTEED_DELIVERY)
                    {
                        // set bit in the bitfield; discard packet if it is duplicit
                        
                        // copy total serial number of received packet
                        DWORD dwSerial = 0;
                        switch (((SUDPPacketCommonHeader *) pBuffer)->PacketFlags & (PACKET_GUARANTEED_DELIVERY | PACKET_GUARANTEED_ORDER | PACKET_FRAGMENTED))
                        {
                        case PACKET_TYPE1:
                            dwSerial = ((SUDPPacketType1 *) pBuffer)->TotalSerial;
                            break;
                        case PACKET_TYPE1F:
                            dwSerial = ((SUDPPacketType1F *) pBuffer)->TotalSerial;
                            break;
                        case PACKET_TYPE2:
                            dwSerial = ((SUDPPacketType2 *) pBuffer)->TotalSerial;
                            break;
                        case PACKET_TYPE2F:
                            dwSerial = ((SUDPPacketType2F *) pBuffer)->TotalSerial;
                            break;
                        default:
                            // SHOULDN'T BE REACHED
                            TRACE_NETWORK("CNetworkConnection::NetworkUDPReceiverThread() - wrong packet type!\n");
                            ASSERT(FALSE);
                        }
                        
                        // lock bitfield
                        VERIFY(p->m_lockReceivedBitfield.Lock());
                        
                        ASSERT(p->m_pReceivedBitfield);
                        
                        // locate bitfield block
                        SUDPPacketsReceivedBitfield *q = p->m_pReceivedBitfield;
                        while ((q->dwFirst + RECEIVEDPACKETS_BITFIELD_SIZE * 32) <= dwSerial)
                        {
                            if (q->pNext == NULL)
                            {
                                // allocate next block
                                try 
                                {
                                    q->pNext =  new SUDPPacketsReceivedBitfield;
                                }
                                catch (CMemoryException *)
                                {
                                    // allocation error!
                                    TRACE_NETWORK("CNetworkConnection::NetworkUDPReceiverThread() - failed to allocate memory!\n");
                                    // unlock bitfield
                                    VERIFY(p->m_lockReceivedBitfield.Unlock());

                                    // send message to application
                                    p->SendErrorEvent(E_MEMORYERROR, eReceiverThread);
                                    
                                    // end thread
                                    return 2;
                                }
                                
                                // fill its members
                                q->pNext->dwFirst = q->dwFirst + RECEIVEDPACKETS_BITFIELD_SIZE * 32;
                                q->pNext->pNext = NULL;
                                memset(q->pNext->aPacketsDelivered, 0, sizeof(DWORD) * RECEIVEDPACKETS_BITFIELD_SIZE);
                            }
                            
                            q = q->pNext;
                        }
                        
                        // set the right bit
                        DWORD dwIndex = (dwSerial - q->dwFirst) / 32;
                        DWORD dwBitMask = 0x00000001 << ((dwSerial - q->dwFirst) % 32);
                        if (!(q->aPacketsDelivered[dwIndex] & dwBitMask))
                        {
                            // packet is not duplicate, proceed
                            
                            // set the bit
                            q->aPacketsDelivered[dwIndex] |= dwBitMask;
                            
                            // unlock bitfield
                            VERIFY(p->m_lockReceivedBitfield.Unlock());
                        }
                        else
                        {
                            // duplicate packet, discard it (ie. do nothing with it)
                            
                            // TRACE_NETWORK("CNetworkConnection::NetworkUDPReceiverThread() - duplicate packet %i\n", dwSerial);
                            
                            // unlock bitfield
                            VERIFY(p->m_lockReceivedBitfield.Unlock());
                            
                            // receive next packet
                            continue;
                        }
                        
                        // increase last total serial number
                        VERIFY(p->m_lockLastReceivedTotalSerial.Lock());
                        if (dwSerial > p->m_dwLastReceivedTotalSerial3)
                            p->m_dwLastReceivedTotalSerial3 = dwSerial;
                        VERIFY(p->m_lockLastReceivedTotalSerial.Unlock());
                        
                        // TRACE_NETWORK("CNetworkConnection::NetworkUDPReceiverThread() - packet %i has arrived\n", dwSerial);
                    }
                    
                    // find the right virtual connection
                    VERIFY(p->m_lockVirtualConnectionList.Lock());
                    ASSERT(p->m_pVirtualConnectionList != NULL);
                    ASSERT(p->m_pVirtualConnectionList->wVirtualConnection == 0);

                    SVirtualConnection *q = p->m_pVirtualConnectionList;
                    while ((q != NULL) && (q->wVirtualConnection < ((SUDPPacketCommonHeader *) pBuffer)->VirtualConnection))
                        q = q->pNext;
                    
                    // if virtual connection does not exist
                    if ((q == NULL) || (q->wVirtualConnection > ((SUDPPacketCommonHeader *) pBuffer)->VirtualConnection))
                    {
                        TRACE_NETWORK("CNetworkConnection::NetworkUDPReceiverThread() - received packet does not have the virtual connection (%i)\n", ((SUDPPacketCommonHeader *) pBuffer)->VirtualConnection);

                        // discard packet (ie. do nothing with it)

                        // unlock virtual connection list
                        VERIFY(p->m_lockVirtualConnectionList.Unlock());

                        // receive next packet
                        continue;
                    }

                    // paranoia
                    ASSERT(q->wVirtualConnection == ((SUDPPacketCommonHeader *) pBuffer)->VirtualConnection);
                    
                    // check the virtual connection flags against the flags received in the packet
                    ASSERT((((SUDPPacketCommonHeader *) pBuffer)->PacketFlags & (PACKET_GUARANTEED_DELIVERY | PACKET_GUARANTEED_ORDER)) ==
                        (q->btSendPacketFlags & (PACKET_GUARANTEED_DELIVERY | PACKET_GUARANTEED_ORDER)));
                    
                    // lock receive packets queue
                    VERIFY(q->qReceivePackets.lock.Lock());
                    
                    // discard packet of type 3?
                    if ((((SUDPPacketCommonHeader *) pBuffer)->PacketFlags == PACKET_TYPE3) &&
                        (((SUDPPacketType3 *) pBuffer)->ConnectionSerial <= q->dwReceiveLastSerial))
                    {
                        // TRACE_NETWORK("CNetworkConnection::NetworkUDPReceiverThread() - delayed packet of type 3\n");
                        
                        // discard it, it is to late to insert it to the queue, because
                        // the application has read packet with higher connection serial
                        // number already
                        
                        // unlock receive packets queue
                        VERIFY(q->qReceivePackets.lock.Unlock());
                        // unlock virtual connection list
                        VERIFY(p->m_lockVirtualConnectionList.Unlock());
                        
                        // receive next packet
                        continue;
                    }
                    
                    // create received packet list item
                    SReceivePacketList *pPacketList;
                    try {
                        pPacketList = new SReceivePacketList;
                    }
                    catch (CMemoryException *)
                    {
                        // memory error!
                        TRACE_NETWORK("CNetworkConnection::NetworkUDPReceiverThread() - failed to allocate memory!\n");
                        VERIFY(q->qReceivePackets.lock.Unlock());
                        VERIFY(p->m_lockVirtualConnectionList.Unlock());
                        
                        // send event to the application (no way to recorver)
                        p->SendErrorEvent(E_MEMORYERROR, eReceiverThread);
                        
                        // end thread
                        return 2;
                    }
                    
                    // fill the received packet list item
                    pPacketList->pNext = NULL;
                    pPacketList->btPacketFlags = ((SUDPPacketCommonHeader *) pBuffer)->PacketFlags;
                    
                    switch (((SUDPPacketCommonHeader *) pBuffer)->PacketFlags & (PACKET_GUARANTEED_DELIVERY | PACKET_GUARANTEED_ORDER | PACKET_FRAGMENTED))
                    {
                    case PACKET_TYPE1:
                        pPacketList->dwConnectionSerial = ((SUDPPacketType1 *) pBuffer)->ConnectionSerial;
                        pPacketList->dwDataSize = recv_result - sizeof(SUDPPacketType1);
                        try 
                        {
                            pPacketList->pBuffer = new BYTE[pPacketList->dwDataSize];
                        }
                        catch (CMemoryException *)
                        {
                            // memory error!
                            TRACE_NETWORK("CNetworkConnection::NetworkUDPReceiverThread() - failed to allocate memory!\n");
                            
                            // delete packet list item
                            delete pPacketList;

                            // unlock received packets list and virtual connection list
                            VERIFY(q->qReceivePackets.lock.Unlock());
                            VERIFY(p->m_lockVirtualConnectionList.Unlock());
                            
                            // send  event to the application (no way to recover)
                            p->SendErrorEvent(E_MEMORYERROR, eReceiverThread);
                            
                            // end thread
                            return 2;
                        }
                        memcpy(pPacketList->pBuffer, ((SUDPPacketType1 *) pBuffer)->Buffer, pPacketList->dwDataSize);
                        break;
                        
                    case PACKET_TYPE1F:
                        pPacketList->dwConnectionSerial = ((SUDPPacketType1F *) pBuffer)->ConnectionSerial;
                        pPacketList->dwFragmentNumber = ((SUDPPacketType1F *) pBuffer)->FragmentNumber;
                        pPacketList->dwDataSize = recv_result - sizeof(SUDPPacketType1F);
                        try
                        {
                            pPacketList->pBuffer = new BYTE[pPacketList->dwDataSize];
                        }
                        catch (CMemoryException *)
                        {
                            // memory error!
                            TRACE_NETWORK("CNetworkConnection::NetworkUDPReceiverThread() - failed to allocate memory!\n");
                            
                            // delete packet list item
                            delete pPacketList;

                            // unlock received packets list and virtual connection list
                            VERIFY(q->qReceivePackets.lock.Unlock());
                            VERIFY(p->m_lockVirtualConnectionList.Unlock());

                            // send event to the application (no way to recover)
                            p->SendErrorEvent(E_MEMORYERROR, eReceiverThread);

                            // end thread
                            return 2;
                        }
                        memcpy(pPacketList->pBuffer, ((SUDPPacketType1F *) pBuffer)->Buffer, pPacketList->dwDataSize);
                        break;
                        
                    case PACKET_TYPE2:
                        pPacketList->dwDataSize = recv_result - sizeof(SUDPPacketType2);
                        try
                        {
                            pPacketList->pBuffer = new BYTE[pPacketList->dwDataSize];
                        }
                        catch (CMemoryException *)
                        {
                            // memory error!
                            TRACE_NETWORK("CNetworkConnection::NetworkUDPReceiverThread() - failed to allocate memory!\n");

                            // delete packet list item
                            delete pPacketList;

                            // unlock received packet list and virtual connection list
                            VERIFY(q->qReceivePackets.lock.Unlock());
                            VERIFY(p->m_lockVirtualConnectionList.Unlock());
                            
                            // send event to the application (no way to recover)
                            p->SendErrorEvent(E_MEMORYERROR, eReceiverThread);
                            
                            // end thread
                            return 2;
                        }
                        memcpy(pPacketList->pBuffer, ((SUDPPacketType2 *) pBuffer)->Buffer, pPacketList->dwDataSize);
                        break;
                        
                    case PACKET_TYPE2F:
                        pPacketList->dwFragmentNumber = ((SUDPPacketType2F *) pBuffer)->FragmentNumber;
                        pPacketList->dwDataSize = recv_result - sizeof(SUDPPacketType2F);
                        try
                        {
                            pPacketList->pBuffer = new BYTE[pPacketList->dwDataSize];
                        }
                        catch (CMemoryException *)
                        {
                            // memory error!
                            TRACE_NETWORK("CNetworkConnection::NetworkUDPReceiverThread() - failed to allocate memory!\n");

                            // delete packet list item
                            delete pPacketList;

                            // unlock received packet list and virtual connection list
                            VERIFY(q->qReceivePackets.lock.Unlock());
                            VERIFY(p->m_lockVirtualConnectionList.Unlock());

                            // send event to the application (no way to recover)
                            p->SendErrorEvent(E_MEMORYERROR, eReceiverThread);
                            
                            // end thread
                            return 2;
                        }
                        memcpy(pPacketList->pBuffer, ((SUDPPacketType2F *) pBuffer)->Buffer, pPacketList->dwDataSize);
                        break;
                        
                    case PACKET_TYPE3:
                        pPacketList->dwConnectionSerial = ((SUDPPacketType3 *) pBuffer)->ConnectionSerial;
                        pPacketList->dwDataSize = recv_result - sizeof(SUDPPacketType3);
                        try
                        {
                            pPacketList->pBuffer = new BYTE[pPacketList->dwDataSize];
                        }
                        catch (CMemoryException *)
                        {
                            // memory error!
                            TRACE_NETWORK("CNetworkConnection::NetworkUDPReceiverThread() - failed to allocate memory!\n");

                            // delete packet list item
                            delete pPacketList;

                            // unlock received packet list and virtual connection list
                            VERIFY(q->qReceivePackets.lock.Unlock());
                            VERIFY(p->m_lockVirtualConnectionList.Unlock());

                            // send event to the application (no way to recover)
                            p->SendErrorEvent(E_MEMORYERROR, eReceiverThread);
                            
                            // end thread
                            return 2;
                        }
                        memcpy(pPacketList->pBuffer, ((SUDPPacketType3 *) pBuffer)->Buffer, pPacketList->dwDataSize);
                        break;
                        
                    case PACKET_TYPE4:
                        pPacketList->dwDataSize = recv_result - sizeof(SUDPPacketType4);
                        try
                        {
                            pPacketList->pBuffer = new BYTE[pPacketList->dwDataSize];
                        }
                        catch (CMemoryException *)
                        {
                            // memory error!
                            TRACE_NETWORK("CNetworkConnection::NetworkUDPReceiverThread() - failed to allocate memory!\n");

                            // delete packet list item
                            delete pPacketList;

                            // unlock received packet list and virtual connection list
                            VERIFY(q->qReceivePackets.lock.Unlock());
                            VERIFY(p->m_lockVirtualConnectionList.Unlock());

                            // send event to the application (no way to recover)
                            p->SendErrorEvent(E_MEMORYERROR, eReceiverThread);
                          
                            // end thread
                            return 2;
                        }
                        memcpy(pPacketList->pBuffer, ((SUDPPacketType4 *) pBuffer)->Buffer, pPacketList->dwDataSize);
                        break;
                        
                    default:
                        // SHOULDN'T BE REACHED
                        TRACE_NETWORK("CNetworkConnection::NetworkUDPReceiverThread() - wrong packet type\n");
                        ASSERT(FALSE);
                    }
                
                    // add packet to the queue
                    if (pPacketList->btPacketFlags & PACKET_GUARANTEED_ORDER)
                    {
                        // types 1, 1F and 3
                        // insert packet to (the middle of) the queue
                        
                        // create pointer to the beginning of the part of uncomplete packets in the list
                        SReceivePacketList **r = q->qReceivePackets.last;

                        ASSERT(AfxIsValidAddress(q->qReceivePackets.last, 4, TRUE) && ((*(q->qReceivePackets.last) == NULL) || AfxIsMemoryBlock(*(q->qReceivePackets.last), sizeof(SReceivePacketList))));

                        // find the right position in the link list
                        while (((*r) != NULL) && ((*r)->dwConnectionSerial < pPacketList->dwConnectionSerial))
                        {
                            r = &((*r)->pNext);
                            ASSERT(AfxIsValidAddress(r, 4, TRUE) && ((*r == NULL) || AfxIsValidAddress(*r, 4, TRUE)));
                        }
                        
                        if (*r == NULL)
                        {
                            // we're at the end of list
                            *r = pPacketList;
                            // TRACE_NETWORK("CNetworkConnection::NetworkUDPReceiverThread() - received packet added to the end of the queue\n");
                        }
                        else
                        {
                            // something's in the way....
                            if (pPacketList->btPacketFlags & PACKET_FRAGMENTED)
                            {
                                // packet is fragmented, take care to insert it to right place
                                while ((*r != NULL) &&
                                    ((*r)->dwConnectionSerial == pPacketList->dwConnectionSerial) &&
                                    ((*r)->dwFragmentNumber < pPacketList->dwFragmentNumber))
                                    r = &((*r)->pNext);
                                
                                if (*r == NULL)
                                {
                                    // we're at the end of list
                                    *r = pPacketList;
                                    // TRACE_NETWORK("CNetworkConnection::NetworkUDPReceiverThread() - received packet added to the end of the queue\n");
                                }
                                else
                                {
                                    // insert to list
                                    pPacketList->pNext = *r;
                                    *r = pPacketList;
                                    // TRACE_NETWORK("CNetworkConnection::NetworkUDPReceiverThread() - received packet inserted to the middle of the queue\n");
                                }
                            }
                            else
                            {
                                // packet is not fragmented

                                // check duplicity of packet type 3
                                if ((pPacketList->btPacketFlags == PACKET_TYPE3) &&
                                    ((*r)->dwConnectionSerial == pPacketList->dwConnectionSerial))
                                {
                                    // discard packet (ie. do nothing)
                                    // TRACE_NETWORK("CNetworkConnection::NetworkUDPReceiverThread() - duplicate packet of type 3\n");

                                    // unlock the received packet list and virtual connection list
                                    VERIFY(q->qReceivePackets.lock.Unlock());
                                    VERIFY(p->m_lockVirtualConnectionList.Unlock());
                                    
                                    // delete the packet
                                    delete [] pPacketList->pBuffer;
                                    delete pPacketList;
                                    
                                    // receive next packet
                                    continue;
                                }
                                
                                // packet is not fragmented, insert right here...
                                pPacketList->pNext = *r;
                                *r = pPacketList;
                                
                                // TRACE_NETWORK("CNetworkConnection::NetworkUDPReceiverThread() - received packet inserted to the middle of queue\n");
                            }
                        }
                    }
                    else
                    {
                        // types 2, 2F and 4; add packet to the end of queue
                        SReceivePacketList **r = q->qReceivePackets.last;

                        ASSERT(AfxIsValidAddress(q->qReceivePackets.last, 4, TRUE) && ((*(q->qReceivePackets.last) == NULL) || AfxIsMemoryBlock(*(q->qReceivePackets.last), sizeof(SReceivePacketList))));

                        if (pPacketList->btPacketFlags & PACKET_FRAGMENTED)
                        {
                            // find the right place
                            while ((*r != NULL) && ((*r)->dwFragmentNumber < pPacketList->dwFragmentNumber))
                                r = &((*r)->pNext);
                        }

                        // insert in front of (*r)
                        pPacketList->pNext = *r;
                        *r = pPacketList;
                    }
                    
                    // packet was inserted
                    // now increase semaphore (and move "last" pointer if apropriate),
                    // if something can be received by an application
                    switch (pPacketList->btPacketFlags & (PACKET_GUARANTEED_DELIVERY | PACKET_GUARANTEED_ORDER | PACKET_FRAGMENTED))
                    {
                    case PACKET_TYPE1:
                    case PACKET_TYPE1F:
                        {
                            for ( ; ; )
                            {
                                SReceivePacketList *r = *(q->qReceivePackets.last);
                                if (r == NULL)
                                {
                                    // end of list; do nothing
                                    break;
                                }
                                else
                                {
                                    // not the end of the list (we have something to check)

                                    if (r->btPacketFlags & PACKET_FRAGMENTED)
                                    {
                                        DWORD dwLastFragmentNumber = 0;
                                        while ((r != NULL) &&
                                            (r->dwConnectionSerial == q->dwReceiveLastSerial + 1) &&
                                            (r->dwFragmentNumber == dwLastFragmentNumber) &&
                                            !(r->btPacketFlags & PACKET_LAST_FRAGMENT))
                                        {
                                            r = r->pNext;
                                            dwLastFragmentNumber++;
                                        }
                                        
                                        if ((r != NULL) &&
                                            (r->dwConnectionSerial == q->dwReceiveLastSerial + 1) &&
                                            (r->dwFragmentNumber == dwLastFragmentNumber) &&
                                            (r->btPacketFlags & PACKET_LAST_FRAGMENT))
                                        {
                                            // GOOD, fragmented block is complete
                                            q->dwReceiveLastSerial++;
                                            q->qReceivePackets.last = &(r->pNext);
                                            VERIFY(q->semReceiveDataReady.Unlock());

                                            // send message to the application
                                            ASSERT(q->pObserver != NULL);
                                            p->InlayEvent(E_PACKETAVAILABLE, q->wVirtualConnection, q->pObserver);

                                            // check of closing this virtual connection
                                            if (q->dwReceiveLastSerial == q->dwReceiveLastAwaitedSerial)
                                            {
                                                // deleted flag must be set
                                                ASSERT(q->bDeleted);

                                                // send event to the virtual connection observer
                                                ASSERT(q->pObserver != NULL);
                                                p->InlayEvent(E_VIRTUALCONNECTIONCLOSED, q->wVirtualConnection, q->pObserver);
                                            }
                                            
                                            // TRACE_NETWORK("CNetworkConnection::NetworkUDPReceiverThread() - semaphore increased (fragmented packet complete), last serial %i\n", q->dwReceiveLastSerial);
                                        }
                                        else
                                        {
                                            // fragmented packet is not complete, have to wait...
                                            // TRACE_NETWORK("CNetworkConnection::NetworkUDPReceiverThread() - semaphore could not be increased, fragmented packet is not complete\n");
                                            break;
                                        }
                                    }
                                    else
                                    {
                                        if (r->dwConnectionSerial == q->dwReceiveLastSerial + 1)
                                        {
                                            // GOOD, we have one more packet
                                            q->dwReceiveLastSerial++;
                                            q->qReceivePackets.last = &(r->pNext);
                                            VERIFY(q->semReceiveDataReady.Unlock());
                                            
                                            // send message to the application
                                            ASSERT(q->pObserver != NULL);
                                            p->InlayEvent(E_PACKETAVAILABLE, q->wVirtualConnection, q->pObserver);

                                            // check of closing this virtual connection
                                            if (q->dwReceiveLastSerial == q->dwReceiveLastAwaitedSerial)
                                            {
                                                // deleted flag must be set
                                                ASSERT(q->bDeleted);

                                                // send event to the virtual connection observer
                                                ASSERT(q->pObserver != NULL);
                                                p->InlayEvent(E_VIRTUALCONNECTIONCLOSED, q->wVirtualConnection, q->pObserver);
                                            }

                                            // TRACE_NETWORK("CNetworkConnection::NetworkUDPReceiverThread() - semaphore increased, last serial %i\n", q->dwReceiveLastSerial);
                                        }
                                        else
                                        {
                                            // packet has bad serial number, have to wait...
                                            // TRACE_NETWORK("CNetworkConnection::NetworkUDPReceiverThread() - semaphore could not be increased, dwReceiveLastSerial %i, dwConnectionSerial %i\n", q->dwReceiveLastSerial, r->dwConnectionSerial);
                                            break;
                                        }
                                    }
                                }
                            } // endless loop
                        }
                        break;
                        
                    case PACKET_TYPE2F:
                        // check, if the fragmented block is complete
                        for ( ; ; )
                        {
                            SReceivePacketList *r = *(q->qReceivePackets.last);
                            if (r == NULL)
                            {
                                // on the end of the list
                                break;
                            }
                            else
                            {
                                // attention! connection serial is not applicable to this type of packets

                                // prepare the fragment number (derived from the first packet in the queue)
                                DWORD dwLastFragmentNumber = r->dwFragmentNumber & 0xff000000;

                                while ((r != NULL) &&
                                    (r->dwFragmentNumber == dwLastFragmentNumber) &&
                                    !(r->btPacketFlags & PACKET_LAST_FRAGMENT))
                                {
                                    r = r->pNext;
                                    dwLastFragmentNumber++;
                                }
                                
                                if ((r != NULL) &&
                                    (r->dwFragmentNumber == dwLastFragmentNumber) &&
                                    (r->btPacketFlags & PACKET_LAST_FRAGMENT))
                                {
                                    // GOOD, fragmented block complete

                                    // increase the counter of successfuly received blocks
                                    q->dwReceiveLastSerial++;

                                    // move "last" pointer
                                    q->qReceivePackets.last = &(r->pNext);
                                    // increase semaphore (packet is ready)
                                    VERIFY(q->semReceiveDataReady.Unlock());

                                    // send message to the application (packet is ready)
                                    ASSERT(q->pObserver != NULL);
                                    p->InlayEvent(E_PACKETAVAILABLE, q->wVirtualConnection, q->pObserver);

                                    // check of closing this virtual connection
                                    if (q->dwReceiveLastSerial == q->dwReceiveLastAwaitedSerial)
                                    {
                                        // deleted flag must be set
                                        ASSERT(q->bDeleted);
                                        
                                        // send event to the virtual connection observer
                                        ASSERT(q->pObserver != NULL);
                                        p->InlayEvent(E_VIRTUALCONNECTIONCLOSED, q->wVirtualConnection, q->pObserver);
                                    }
                                }
                                else
                                {
                                    break;
                                }
                            }
                        } // endless loop
                        break;
                        
                    case PACKET_TYPE2:
                        {
                            // increase the number of successfuly received blocks
                            q->dwReceiveLastSerial++;

                            // move "last" pointer
                            q->qReceivePackets.last = &((*(q->qReceivePackets.last))->pNext);
                            
                            // increase semaphore (packet is ready)
                            VERIFY(q->semReceiveDataReady.Unlock());

                            // send message to the application (packet is ready)
                            ASSERT(q->pObserver != NULL);
                            p->InlayEvent(E_PACKETAVAILABLE, q->wVirtualConnection, q->pObserver);

                            // check of closing this virtual connection
                            if (q->dwReceiveLastSerial == q->dwReceiveLastAwaitedSerial)
                            {
                                // deleted flag must be set
                                ASSERT(q->bDeleted);
                                
                                // send event to the virtual connection observer
                                ASSERT(q->pObserver != NULL);
                                p->InlayEvent(E_VIRTUALCONNECTIONCLOSED, q->wVirtualConnection, q->pObserver);
                            }
                        }
                        break;

                    case PACKET_TYPE4:
                        {
                            // move "last" pointer
                            q->qReceivePackets.last = &((*(q->qReceivePackets.last))->pNext);
                            
                            // increase semaphore (packet is ready)
                            VERIFY(q->semReceiveDataReady.Unlock());

                            // send message to the application (packet is ready)
                            ASSERT(q->pObserver != NULL);
                            p->InlayEvent(E_PACKETAVAILABLE, q->wVirtualConnection, q->pObserver);
                        }
                        break;
                        
                    case PACKET_TYPE3:
                        {
                            // increase semaphore only, "last" must point to the beginning of list permanently
                            // in fact the semaphore value is equal to the number of packets
                            // received but pending in the received packet link list (unread by application)
                            VERIFY(q->semReceiveDataReady.Unlock());

                            // send message to the application
                            ASSERT(q->pObserver != NULL);
                            p->InlayEvent(E_PACKETAVAILABLE, q->wVirtualConnection, q->pObserver);
                        }
                        break;
                        
                    default:
                        // SHOULDN'T BE REACHED
                        TRACE_NETWORK("CNetworkConnection::NetworkUDPReceiverThread() - wrong packet type\n");
                        ASSERT(FALSE);
                        break;
                    }
                
                    ASSERT(AfxIsValidAddress(q->qReceivePackets.last, 4, TRUE) && ((*(q->qReceivePackets.last) == NULL) || AfxIsMemoryBlock(*(q->qReceivePackets.last), sizeof(SReceivePacketList))));

                    // unlock received packet queue
                    VERIFY(q->qReceivePackets.lock.Unlock());
                    
                    // unlock virtual connection list                
                    VERIFY(p->m_lockVirtualConnectionList.Unlock());
                }                

                // other network events?
                if ((sNetEvents.lNetworkEvents & ~(FD_READ) & FD_ALL_EVENTS) != 0)
                {
                    // something's wrong...
                    TRACE_NETWORK("CNetworkConnection::NetworkConnectionRecieverThread() - unknown network event (WSAEnumNetworkEvents()) %x!\n", sNetEvents.lNetworkEvents);
                    ASSERT(FALSE);
                }
            }
            break;

        default:
            // SHOULD NOT BE REACHED
            TRACE_NETWORK("CNetworkConnection::NetworkUDPReceiverThread() - invalid return code for WaitForMultipleObjects()\n");
            ASSERT(FALSE);
        }

    }  // endless loop
    
    // end thread (should never be reached)
    TRACE_NETWORK("CNetworkConnection::NetworkUDPReceiverThread() - wrong code reached\n");
    ASSERT(FALSE);
    return 0;
}


//////////////////////////////////////////////////////////////////////
// Thread function -- TCP control connection
//////////////////////////////////////////////////////////////////////

// return values:
//    0: normal exit
//    1: network error
//    2: memory allocation failure
//    3: WaitForMultipleObjects() failure
//    4: abortive close 

UINT CNetworkConnection::NetworkControlThread(LPVOID pParam)
{
    // create a pointer to network connection object
    CNetworkConnection *p = (CNetworkConnection *) pParam;

    // prepare a TCP packet
    STCPPacket sPacket;

    // prepare an event variable
    CEvent eventTCPPacketArrived;
    
    // attach socket to event
    if (WSAEventSelect(p->m_TCPRecvSocket, eventTCPPacketArrived, FD_READ | FD_CLOSE) == SOCKET_ERROR)
    {
        // error?
        int nError = WSAGetLastError();
        TRACE_NETWORK("CNetworkConnection::NetworkControlThread() - WSAEventSelect() error %i\n", nError);

        // send event to application
        p->SendErrorEvent(E_NETWORKEVENTSELECTERROR, eControlThread);
        
        // exit thread
        TRACE_NETWORK("CNetworkConnection::NetworkControlThread() - exiting...\n");
        return 1;
    }

    // prepare handles for wait function
    HANDLE aHandles[2];
    aHandles[0] = p->m_eventControllerKill;
    aHandles[1] = eventTCPPacketArrived;

    for ( ; ; )
    {
        switch (WaitForMultipleObjects(2, aHandles, FALSE, INFINITE))
        {
        case WAIT_FAILED:
            // damn it...
            {
                TRACE_NETWORK("CNetworkConnection::NetworkControlThread() - wait function failed\n");
            
                // send event to the application
                p->SendErrorEvent(E_WAITERROR, eControlThread);
            }
            return 3;

        case WAIT_OBJECT_0:
            // kill event was set
            {
                TRACE_NETWORK("CNetworkConnection::NetworkControlThread() - thread finished\n");
            }
            return 0;

        case WAIT_OBJECT_0 + 1:
            // data on network ready
            {
                // prepare error code variable
                int nError;
                
                // receive TCP packet
                if (!(p->ReceiveTCP(&sPacket, sizeof(sPacket), &nError)))
                {
                    if (nError == WSAEWOULDBLOCK)
                    {
                        // break the switch, wait for something else (the socket is in correct state, nothing happened)
                        break;
                    }
                    else
                    {
                        // some error other than WSAEWOULDBLOCK
                        TRACE_NETWORK("CNetworkConnection::NetworkControlThread() - network error, thread finished\n");
                        
                        // send message to application
                        p->SendErrorEvent(E_NETWORKERROR, eControlThread);

                        // exit thread
                        return 1;
                    }
                }
                
                switch (sPacket.eType)
                {
                case STCPPacket::SendAgain:
                    {
                        TRACE_NETWORK("CNetworkConnection::NetworkControlThread() - request to send again %i packets received\n", sPacket.dwData1);

                        for (DWORD i = 0; i < sPacket.dwData1; i++)
                        {
                            DWORD dwLostPacketNumber;

                            for ( ; ; )
                            {
                                if (p->ReceiveTCP(&dwLostPacketNumber, sizeof(DWORD), &nError))
                                    break;

                                if (nError == WSAEWOULDBLOCK)
                                {
                                    // wait for a while
                                    Sleep(10);
                                    // try to receive again
                                    continue;
                                }
                                else
                                {
                                    // some error other than WSAEWOULDBLOCK
                                    TRACE_NETWORK("CNetworkConnection::NetworkControlThread() - network error, thread finished\n");
                                    
                                    // send message to application
                                    p->SendErrorEvent(E_NETWORKERROR, eControlThread);
                                    
                                    // exit thread
                                    return 1;
                                }
							}
                            
							// TRACE_NETWORK("CNetworkConnection::NetworkControlThread() - sending again packet %i\n", dwLostPacketNumber);

                            // lock sent packets queue
                            VERIFY(p->m_qPacketsSent.lock.Lock());
                            
                            // prepare iterator
                            SSendPacketList *q = p->m_qPacketsSent.first;

                            if ((q == NULL) || (dwLostPacketNumber < q->dwTotalSerialNumber))
                            {
                                // packet was probably delivered successfully, but request to send it again was delayed
                                TRACE_NETWORK("CNetworkConnection::NetworkControlThread() - delayed lost packet request\n");

                                // unlock the list of sent packets
                                VERIFY(p->m_qPacketsSent.lock.Unlock());

                                // ingore this request
                                continue;
                            }

                            // find the right packet
							while (dwLostPacketNumber > q->dwTotalSerialNumber)
							{
                                q = q->pNext;
                                ASSERT(q != NULL);  // this means, that request has higher number than the last packet sent
							}
                            
                            // paranoia (the hole in the link list??)
                            ASSERT(q->dwTotalSerialNumber == dwLostPacketNumber);

                            // send packet over network
                            VERIFY(p->m_lockUDPSendSocket.Lock());
                            int send_result = send(p->m_UDPSendSocket, (char *) q->pPacket, q->dwPacketSize, 0);
							if (send_result == SOCKET_ERROR)
							{
								// treat this like an error?
                                // send message to application?
								TRACE_NETWORK("CNetworkConnection::NetworkControlThread() - can't send lost packet again, error %i\n", WSAGetLastError());
							}
                            VERIFY(p->m_lockUDPSendSocket.Unlock());
                            
                            // unlock sent packets queue
                            VERIFY(p->m_qPacketsSent.lock.Unlock());
                        }
                    }
                    break;
                    
                case STCPPacket::LastGoodSerial:
                    {
                        // delete the beginning of sent packet list
                        // TRACE_NETWORK("CNetworkConnection::NetworkControlThread() - serial number of the last good packet on the other side %i\n", sPacket.dwData1);
                        
                        // check last packet serial number against zero (it means "I didn't send anything yet...")
                        if (sPacket.dwData1 == 0)
                            break;

                        // lock sent packets queue
                        VERIFY(p->m_qPacketsSent.lock.Lock());
                        
                        // delete the beginning of the list (up to the last good packet incl.)
                        while ((p->m_qPacketsSent.first != NULL) && (p->m_qPacketsSent.first->dwTotalSerialNumber <= sPacket.dwData1))
                        {
                            // save pointer to the first packet in the list
                            SSendPacketList *r = p->m_qPacketsSent.first;
                            // delete the first packet from the list
                            p->m_qPacketsSent.first = p->m_qPacketsSent.first->pNext;

                            // the length of sent packets link list has changed
                            ASSERT(p->m_qPacketsSent.dwSentPacketsNumber > 0);
                            p->m_qPacketsSent.dwSentPacketsNumber--;
                            ASSERT(p->m_qPacketsSent.dwSentPacketsSize >= r->dwPacketSize);
                            p->m_qPacketsSent.dwSentPacketsSize -= r->dwPacketSize;
                            
                            // delete the packet
                            delete r->pPacket;
                            delete r;
                        }
                        
                        // take care of the "last" pointer
                        if (p->m_qPacketsSent.first == NULL)
                        {
                            // let the "last" point to the first
                            p->m_qPacketsSent.last = &(p->m_qPacketsSent.first);

                            // are we in the closing state?
                            if (p->m_bClosing2)
                            {
                                // set the event
                                VERIFY(p->m_eventClosingSentPackets.SetEvent());
                            }
                        }

                        // "last" must point to "NULL" always...
                        ASSERT(*(p->m_qPacketsSent.last) == NULL);
                        
                        // unlock sent packets queue
                        VERIFY(p->m_qPacketsSent.lock.Unlock());
                    }
                    break;
                    
                case STCPPacket::AbortiveClose:
                    {
                        // the peer wants to abort
                        TRACE_NETWORK("CNetworkConnection::NetworkControlThread() - abortive close notification received from the peer, thread finished\n");
                        
                        // send event to main application
                        p->SendErrorEvent(E_ABORTIVECLOSE, eControlThread);

                        // abort this thread
                        return 4;
                    }

                case STCPPacket::NormalClose:
                    {
                        // the peer wants to end the connection
                        TRACE_NETWORK("CNetworkConnection::NetworkControlThread() - normal close notification received from the peer\n");

                        // this means to consider all sending (and corrections)
                        // as successful
                        
                        // lock sent packets queue
                        VERIFY(p->m_qPacketsSent.lock.Lock());

                        // delete the whole linked list
                        while (p->m_qPacketsSent.first != NULL)
                        {
                            SSendPacketList *q = p->m_qPacketsSent.first;
                            p->m_qPacketsSent.first = p->m_qPacketsSent.first->pNext;
                            delete q->pPacket;
                            delete q;
                        }
                        // let the "last" point to the first
                        p->m_qPacketsSent.last = &(p->m_qPacketsSent.first);
                        
                        // set the event
                        VERIFY(p->m_eventClosingSentPackets.SetEvent());

                        // unlock sent packets queue
                        VERIFY(p->m_qPacketsSent.lock.Unlock());
                        
                        // send event to main application
                        p->SendErrorEvent(E_NORMALCLOSE, eControlThread);

                        // exit thread
                        TRACE_NETWORK("CNetworkConnection::NetworkControlThread() - thread finished\n");
                        return 0;
                    }

                case STCPPacket::NewVirtualConnection:
                    {
                        // the peer wants to create new virtual connection

                        // prepare new values
                        WORD wNewConnectionNumber = (WORD) ((sPacket.dwData1 & 0xffff0000) >> 16);
                        ASSERT(wNewConnectionNumber != 0);
                        BYTE btNewConnectionPriority = (BYTE) ((sPacket.dwData1 & 0x0000ff00) >> 8);
                        ASSERT(btNewConnectionPriority <= PACKETPRIORITY_HIGHEST);
                        BYTE btNewConnectionFlags = (BYTE) (sPacket.dwData1 & 0x000000ff);
                        ASSERT((btNewConnectionFlags & ~(PACKET_GUARANTEED_ORDER | PACKET_GUARANTEED_DELIVERY)) == 0);

                        // TRACE_NETWORK("CNetworkConnection::NetworkControlThread() - new virtual connection %i request, priority %i, flags 0x%x\n", wNewConnectionNumber, btNewConnectionPriority, btNewConnectionFlags);
                        
                        // lock virtual connection list
                        VERIFY(p->m_lockVirtualConnectionList.Lock());
                        ASSERT(p->m_pVirtualConnectionList != NULL);
                        ASSERT(p->m_pVirtualConnectionList->wVirtualConnection == 0);
                        
                        // find connection
                        SVirtualConnection **q = &(p->m_pVirtualConnectionList);
                        while (((*q) != NULL) && ((*q)->wVirtualConnection < wNewConnectionNumber))
                            q = &((*q)->pNext);
                        
                        // if next item has the same connection number
                        if ((*q != NULL) && ((*q)->wVirtualConnection == wNewConnectionNumber))
                        {
                            // ignore creation...
                            TRACE_NETWORK("CNetworkConnection::NetworkControlThread() - new virtual connection request ignored (connection exists already)\n");

                            // send negative acknowledge to client
                            sPacket.eType = STCPPacket::NewVirtualConnectionNegAck;
                            
                            if (p->SendTCP(&sPacket, sizeof(sPacket), &nError))
                            {
                                // ignore this error
                                TRACE_NETWORK("CNetworkConnection::NetworkControlThread() - SendTCP() error %i, could not send new virtual connection negative acknowledge\n", nError);
                            }

                            // unlock virtual connection list
                            VERIFY(p->m_lockVirtualConnectionList.Unlock());
                            
                            // wait for something else
                            break;
                        }

                        // copy the number of newly created connection to the local member variable
                        p->m_wNewConnectionNumber = wNewConnectionNumber + 1;
                        if (p->m_wNewConnectionNumber == 0)
                            p->m_wNewConnectionNumber++;
                        
                        // create connection                        
                        try
                        {
                            // make pointer to actual virtual connection
                            SVirtualConnection *r = *q;
                            
                            *q = new SVirtualConnection;
                            (*q)->btSendPacketFlags = btNewConnectionFlags;
                            (*q)->btSendConnectionPriority = btNewConnectionPriority;
                            (*q)->wVirtualConnection = wNewConnectionNumber;
                            (*q)->pNext = r;
                        }
                        catch (CMemoryException *)
                        {
                            // unlock virtual connection list
                            VERIFY(p->m_lockVirtualConnectionList.Unlock());

                            TRACE_NETWORK("CNetworkConnection::NetworkControlThread() - memory exception caught\n");
                        
                            // send event to the application
                            p->SendErrorEvent(E_MEMORYERROR, eControlThread);
                            
                            // abort thread
                            return 2;
                        }
                        
                        // unlock virtual connection list
                        VERIFY(p->m_lockVirtualConnectionList.Unlock());

                        // prepare pointer to user data
                        SNewVirtualConnectionUserData *pUserData;
                        try
                        {
                            pUserData = new SNewVirtualConnectionUserData(p, (*q)->wVirtualConnection);
                            pUserData->dwSize = sPacket.dwData2;
                            pUserData->pBuffer = NULL;
                            pUserData->pWakeUpEvent = new CEvent();
                        }
                        catch (CMemoryException *)
                        {
                            // report error
                            TRACE_NETWORK("CNetworkConnection::NetworkControlThread() - memory exception caught\n");

                            // cleanup
                            if (pUserData != NULL)
                                delete pUserData;
                        
                            // send event to the application
                            p->SendErrorEvent(E_MEMORYERROR, eControlThread);
                            
                            // abort thread
                            return 2;
                        }
                        
                        // do we have some user data?
                        if (pUserData->dwSize > 0)
                        {
                            // prepare buffer for user data
                            try
                            {
                                pUserData->pBuffer = new BYTE[pUserData->dwSize];
                            }
                            catch (CMemoryException *)
                            {
                                // oops! that's not good
                                TRACE_NETWORK("CNetworkConnection::NetworkControlThread() - memory exception caught\n");
                                
                                // send event to the application
                                p->SendErrorEvent(E_MEMORYERROR, eControlThread);
                                
                                // abort thread
                                return 2;
                            }

                            for ( ; ; )
                            {
                                // receive user data
                                if (!(p->ReceiveTCP(pUserData->pBuffer, pUserData->dwSize, &nError)))
                                {
                                    if (nError == WSAEWOULDBLOCK)
                                    {
                                        // huh, user data are still not ready, wait a moment and try again...
                                        Sleep(10);
                                        
                                        // receive again
                                        continue;
                                    }
                                    else
                                    {
                                        // some error other than WSAEWOULDBLOCK
                                        TRACE_NETWORK("CNetworkConnection::NetworkControlThread() - network error, can't receive user data with new virtual connection request, thread finished\n");
                                        
                                        // send message to application
                                        p->SendErrorEvent(E_NETWORKERROR, eControlThread);
                                        
                                        // exit thread
                                        return 1;
                                    }
                                }
                                break;
                            }
                        }
                        
                        // send event to application (high priority, wait until processed)
                        ASSERT(p->m_pObserver != NULL);
                        p->InlayEvent((DWORD) E_NEWVIRTUALCONNECTION, (DWORD) pUserData, p->m_pObserver, (DWORD) CEventManager::Priority_High + 1);
                        
                        // wait until the application processes the user data,
                        // it is necessary for attaching CObserver to this virtual connection
                        // (but it is not necessary to attach it for "send-only" connections)
                        VERIFY(pUserData->pWakeUpEvent->Lock());

                        // delete user data received with new virtual connection request
                        if (pUserData->dwSize > 0) 
                            delete [] (BYTE *) (pUserData->pBuffer);
                        delete pUserData->pWakeUpEvent;
                        delete pUserData;
                        
                        // send acknowledge to the peer, that the request was successful

                        // prepare "TCP packet"
                        sPacket.eType = STCPPacket::NewVirtualConnectionAck;
                        sPacket.dwData1 = wNewConnectionNumber;
                        
                        // send data
                        if (!(p->SendTCP(&sPacket, sizeof(sPacket), &nError)))
                        {
                            // damn it, what the hell is happening??
                            TRACE_NETWORK("CNetworkConnection::NetworkControlThread() - can't send new virtual connection acknowledge, error %i\n", nError);
                            
                            // delete newly created connection
                            // (it is necessary to browse the link list
                            // again as it was unlocked for some time so
                            // the predecessor (we have a pointer into) could have been deleted)
                            VERIFY(p->m_lockVirtualConnectionList.Lock());
                            ASSERT(p->m_pVirtualConnectionList != NULL);
                            ASSERT(p->m_pVirtualConnectionList->wVirtualConnection == 0);

                            q = &(p->m_pVirtualConnectionList);
                            while (((*q) != NULL) && ((*q)->wVirtualConnection < wNewConnectionNumber))
                                q = &((*q)->pNext);
                            // paranoia
                            ASSERT(((*q) != NULL) && ((*q)->wVirtualConnection == wNewConnectionNumber));

                            // delete the item from the link list
                            SVirtualConnection *r = *q;
                            *q = (*q)->pNext;
                            VERIFY(p->m_lockVirtualConnectionList.Unlock());
                            
                            // if posiible, send event to virtual connection observer
                            if (r->pObserver != NULL)
                            {
                                p->InlayEvent(E_VIRTUALCONNECTIONCLOSED, wNewConnectionNumber, r->pObserver);
                            }
                            
                            delete r;

                            // send error event to application
                            p->SendErrorEvent(E_NETWORKERROR, eControlThread);
                        
                            // abort this thread
                            TRACE_NETWORK("CNetworkConnection::NetworkControlThread() - thread finished\n");
                            return 1;
                        }
                    }
                    break;

                case STCPPacket::NewVirtualConnectionAck:
                    {
                        // acknowledge new virtual connection

                        // lock virtual connection list
                        VERIFY(p->m_lockVirtualConnectionList.Lock());
                        ASSERT(p->m_pVirtualConnectionList != NULL);
                        ASSERT(p->m_pVirtualConnectionList->wVirtualConnection == 0);

                        // prepare connection number
                        WORD wVirtualConnection = (WORD) (sPacket.dwData1 & 0x0000ffff);
                        ASSERT(wVirtualConnection != 0);
                        
                        // find given virtual connection
                        SVirtualConnection *q = p->m_pVirtualConnectionList;

                        while ((q != NULL) && (q->wVirtualConnection < wVirtualConnection))
                            q = q->pNext;

                        if ((q != NULL) && (q->wVirtualConnection == wVirtualConnection))
                        {
                            // good number, set event
                            q->eventCreateAcknowledged.SetEvent();
                        }
                        else
                        {
                            TRACE_NETWORK("CNetworkConnection::NetworkControlThread() - acknowledge for non-existing virtual connection received\n");
                            ASSERT(FALSE);
                        }

                        // unlock virtual connection list
                        VERIFY(p->m_lockVirtualConnectionList.Unlock());
                    }
                    break;

                case STCPPacket::CloseVirtualConnection:
                    {
                        // prepare connection number
                        ASSERT((sPacket.dwData1 & 0xffff0000) == 0);
                        WORD wDeletedConnection = (WORD) (sPacket.dwData1 & 0x0000ffff);
                        ASSERT(wDeletedConnection != 0);

                        // lock virtual connection list
                        VERIFY(p->m_lockVirtualConnectionList.Lock());
                        ASSERT(p->m_pVirtualConnectionList != NULL);
                        ASSERT(p->m_pVirtualConnectionList->wVirtualConnection == 0);

                        // find connection
                        SVirtualConnection *q = p->m_pVirtualConnectionList;
                        while ((q != NULL) && (q->wVirtualConnection < wDeletedConnection))
                            q = q->pNext;
                        
                        // connection does not exist
                        if ((q == NULL) || (q->wVirtualConnection > wDeletedConnection))
                        {
                            TRACE_NETWORK("CNetworkConnection::NetworkControlThread() - virtual connection delete request was ignored (connection doesn't exist)\n");
                            
                            // unlock virtual connection list
                            VERIFY(p->m_lockVirtualConnectionList.Unlock());

                            break;
                        }

                        // paranoia
                        ASSERT(q->wVirtualConnection == wDeletedConnection);

                        // set deleted flag...
                        q->bDeleted = TRUE;
                            
                        if (q->pObserver == NULL)
                        {
                            // there is no observer, somebody will delete this connection later

                        }
                        else
                        {
                            // we have an observer, now consider guaranteed connection and missing packets

                            // send event if the connection has received all packets and has meaningful observer
                            if (q->btSendPacketFlags & PACKET_GUARANTEED_DELIVERY)
                            {
                                // paranoia (last received packet must have smaller or equal number than the expected one)
                                ASSERT(q->dwReceiveLastSerial <= sPacket.dwData2);
                                
                                if (q->dwReceiveLastSerial == sPacket.dwData2)
                                {
                                    // send event to application that virtual connection is being closed
                                    p->InlayEvent(E_VIRTUALCONNECTIONCLOSED, q->wVirtualConnection, q->pObserver);
                                }
                                else
                                {
                                    // copy the number of last awaited packet to the virtual connection member
                                    q->dwReceiveLastAwaitedSerial = sPacket.dwData2;
                                }
                            }
                            else
                            {
                                // send event to application that virtual connection is being closed
                                p->InlayEvent(E_VIRTUALCONNECTIONCLOSED, q->wVirtualConnection, q->pObserver);
                            }
                        }
                        
                        // unlock virtual connection list
                        VERIFY(p->m_lockVirtualConnectionList.Unlock());
                    }
                    break;
                    
                default:
                    // SHOULDN'T BE REACHED
                    TRACE_NETWORK("CNetworkConnection::NetworkControlThread() - wrong TCP packet type!\n");
                    ASSERT(FALSE);
                } // switch
            }
            break;

        default:
            // SHOULD NOT BE REACHED
            TRACE_NETWORK("CNetworkConnection::NetworkControlThread() - suspicious WaitForMultipleObjects() return code\n");
            ASSERT(FALSE);
        } // switch
    } // endless loop

    // end thread (should not be reached)
    TRACE_NETWORK("CNetworkConnection::NetworkControlThread() - wrong code reached!\n");
    ASSERT(FALSE);
    return 0;
}


//////////////////////////////////////////////////////////////////////
// Thread function -- timers
//////////////////////////////////////////////////////////////////////

// return values:
//    0: normal exit
//    1: GetMessage() error
//    2: UDP alive checking -- no UDP packets
//    3: TCP error
//    4: wrong code reached
//    5: cannot initialize timers

UINT CNetworkConnection::NetworkTimerThread(LPVOID pParam)
{
    // create pointer to network clien thread
    CNetworkConnection *p = (CNetworkConnection *) pParam;

    // create synchronization packets timer
    UINT uSynPacketsTimer = SetTimer(NULL, 0, p->m_uSynPacketsPeriod, NULL);
    if (uSynPacketsTimer == 0)
    {
        TRACE_NETWORK("CNetworkConnection::NetworkTimerThread() - synchronization packet timer could not be created, error %i\n", GetLastError());
        return 5;
    }

    // create lost packets timer
    UINT uLostPacketsTimer = SetTimer(NULL, 0, p->m_uLostPacketsPeriod, NULL);
    if (uLostPacketsTimer == 0)
    {
        TRACE_NETWORK("CNetworkConnection::NetworkTimerThread() - lost packet timer could not be created, error %i\n", GetLastError());
        KillTimer(NULL, uSynPacketsTimer);
        return 5;
    }

    // create handle from CEvent object
    HANDLE hTimerKill = p->m_eventTimerKill;

    for ( ; ; )
    {
        switch (MsgWaitForMultipleObjects(1, &hTimerKill, FALSE, INFINITE, QS_TIMER))
        {
        case WAIT_OBJECT_0:
            {
                // timer killer was activated
                
                // kill synchronization packets timer
                if (uSynPacketsTimer != 0)
                    KillTimer(NULL, uSynPacketsTimer);
                // kill lost packets timer
                if (uLostPacketsTimer != 0)
                    KillTimer(NULL, uLostPacketsTimer);

                TRACE_NETWORK("CNetworkConnection::NetworkTimerThread() - exiting...\n");
            }
            // end the thread
            return 0;

        case WAIT_OBJECT_0 + 1:
            {
                // timer message arrived

                // prepare MSG structure
                MSG msg;
            
                for ( ; ; )
                {
                    // get that message
                    switch (GetMessage(&msg, NULL, 0, 0))
                    {
                    case -1:
                        {
                            // error
                            TRACE_NETWORK("CNetworkConnection::NetworkTimerThread() - GetMessage() error!\n");
                            
                            // kill synchronization packets timer
                            if (uSynPacketsTimer != 0)
                                KillTimer(NULL, uSynPacketsTimer);
                            // create lost packets timer
                            if (uLostPacketsTimer != 0)
                                KillTimer(NULL, uLostPacketsTimer);
                        }
                        // end the thread
                        return 1;
                    

                    case 0:
                        // WM_QUIT message
                        TRACE_NETWORK("CNetworkConnection::NetworkTimerThread() - got suspicious WM_QUIT message, ignoring...\n");
                        ASSERT(FALSE);
                        goto endloop;
                        
                        
                    default:
                        // check the type of message
                        if (msg.message != WM_TIMER)
                        {
                            TRACE_NETWORK("CNetworkConnection::NetworkTimerThread() - got unexpected (unknown) message, ignoring...\n");
                            goto endloop;
                        }

                        if (msg.wParam == uLostPacketsTimer)
                        {
                            // first check UDP alive status
                            if (p->m_bUDPPacketReceived)
                            {
                                // UDP packet received
                                // reset the boolean
                                p->m_bUDPPacketReceived = FALSE;
                                // reset the tick counter
                                p->m_dwTicksFromLastUDPPacket = 0;
                            }
                            else
                            {
                                // no UDP packet received
                                // increase the tick counter
                                p->m_dwTicksFromLastUDPPacket++;

#ifdef UDP_ALIVE_CHECKING
                                if (p->m_bPerformUDPAliveChecking &&
                                    (p->m_dwTicksFromLastUDPPacket > UDP_ALIVE_CHECKING))
                                {
                                    // no UDP packets for long long ... long long time...
                                    // send network error and exit this thread
                                    TRACE_NETWORK("CNetworkConnection::NetworkTimerThread() - UDP connection died...\n");
                                    
                                    // kill synchronization packets timer
                                    if (uSynPacketsTimer != 0)
                                        KillTimer(NULL, uSynPacketsTimer);
                                    // kill lost packets timer
                                    if (uLostPacketsTimer != 0)
                                        KillTimer(NULL, uLostPacketsTimer);
                                    
                                    // send network error
                                    p->SendErrorEvent(E_NETWORKERROR, eTimerThread);

                                    TRACE_NETWORK("CNetworkConnection::NetworkTimerThread() - exiting...\n");
                                    return 2;
                                }
#endif  // UDP_ALIVE_CHECKING
                            }


                            // check missing packets, ask for repeating...
                            
                            // array of missing packets
                            DWORD aMissingPackets[MAX_LOSTPACKET_REQUEST_LEN];
                            // actual size of the request
                            VERIFY(p->m_lockRequestPacketSize.Lock());
                            DWORD dwActualRequestSize = p->m_dwRequestPacketSize;
                            VERIFY(p->m_lockRequestPacketSize.Unlock());
                            
                            // number of values in array
                            DWORD dwMissingPackets = 0;
                            
                            // lock received packets bitfield
                            VERIFY(p->m_lockReceivedBitfield.Lock());
                            
                            // lock last total serial numbers
                            VERIFY(p->m_lockLastReceivedTotalSerial.Lock());
                            
                            // check the number of packets in last time-slice
                            if (p->m_dwLastReceivedTotalSerial2 == p->m_dwLastReceivedTotalSerial1)
                            {
                                // no packets arrived, no work
                                
                                // set the last 2 value
                                p->m_dwLastReceivedTotalSerial2 = p->m_dwLastReceivedTotalSerial3;
                                // unlock everything
                                VERIFY(p->m_lockLastReceivedTotalSerial.Unlock());
                                VERIFY(p->m_lockReceivedBitfield.Unlock());
                                goto endloop;
                            }
                            
                            // TRACE_NETWORK("Timer thread - checking packets %i - %i\n", p->m_dwLastReceivedTotalSerial1 + 1, p->m_dwLastReceivedTotalSerial2);
                            
                            // actual index to received packets bitfield (may be greater than RECEIVEDPACKETS_BITFIELD_SIZE)
                            DWORD dwActualIndex = ((p->m_dwLastReceivedTotalSerial1 + 1) >> 5) - (p->m_pReceivedBitfield->dwFirst >> 5);
                            // total number of indexes to scan
                            DWORD dwTotalIndexes = (p->m_dwLastReceivedTotalSerial2 >> 5) - ((p->m_dwLastReceivedTotalSerial1 + 1) >> 5) + 1;
                            
                            // bitfield must be valid
                            ASSERT(p->m_pReceivedBitfield);
                            
                            // delete bitfield blocks from the beginning if possible
                            while (dwActualIndex >= RECEIVEDPACKETS_BITFIELD_SIZE)
                            {
                                // delete the first block of bitfield
                                SUDPPacketsReceivedBitfield *q = p->m_pReceivedBitfield;
                                p->m_pReceivedBitfield = p->m_pReceivedBitfield->pNext;
                                delete q;
                                
                                // check the validity of bitfield
                                ASSERT(p->m_pReceivedBitfield);
                                // decrease actual index
                                dwActualIndex -= RECEIVEDPACKETS_BITFIELD_SIZE;
                            }
                            
                            DWORD i;
                            // find the last good packet serial number
                            for (i = 0; i < dwTotalIndexes; i++, dwActualIndex++)
                            {
                                // stop if there is some missing packet
                                if (p->m_pReceivedBitfield->aPacketsDelivered[dwActualIndex] != 0xffffffff)
                                    break;
                            }
                            
                            // find exact number of the first missing packet
                            if (i == dwTotalIndexes)
                            {
                                // all packets good and complete
                                p->m_dwLastReceivedTotalSerial1 = p->m_dwLastReceivedTotalSerial2;
                                // copy the last serial number for next timer tick
                                p->m_dwLastReceivedTotalSerial2 = p->m_dwLastReceivedTotalSerial3;
                                
                                // send last good packet serial number to peer
                                STCPPacket sPacket;
                                sPacket.eType = STCPPacket::LastGoodSerial;
                                sPacket.dwData1 = p->m_dwLastReceivedTotalSerial1;
                                
                                // TRACE_NETWORK("CNetworkConnection::NetworkTimerThread() - sending the serial number of the last good packet %i\n", p->m_dwLastReceivedTotalSerial1);
                                
                                // send data
                                if (!p->SendTCP(&sPacket, sizeof(sPacket)))
                                {
                                    // error
                                    TRACE_NETWORK("CNetworkConnection::NetworkTimerThread() - network error, never mind\n");
                                }
                                
                                // unlock everything and exit
                                VERIFY(p->m_lockLastReceivedTotalSerial.Unlock());
                                VERIFY(p->m_lockReceivedBitfield.Unlock());
                                
                                // TRACE_NETWORK("CNetworkConnection::NetworkTimerThread() - all packets received, nothing to request\n");
                                goto endloop;
                            }
                            else
                            {
                                BYTE j;
                                DWORD dwMask = 0x00000001;
                                for (j = 0; j < 32; j++, dwMask <<= 1)
                                {
                                    // stop if the packet is missing
                                    if (!(p->m_pReceivedBitfield->aPacketsDelivered[dwActualIndex] & dwMask))
                                        break;
                                }
                                
                                // set the serial number of last good packet
                                p->m_dwLastReceivedTotalSerial1 = p->m_pReceivedBitfield->dwFirst + dwActualIndex * 32 + j - 1;
                                
                                // can't go over last serial 2
                                if (p->m_dwLastReceivedTotalSerial1 >= p->m_dwLastReceivedTotalSerial2)
                                {
                                    // all packets received successfuly
                                    p->m_dwLastReceivedTotalSerial1 = p->m_dwLastReceivedTotalSerial2;
                                    // copy the last serial number for next timer tick
                                    p->m_dwLastReceivedTotalSerial2 = p->m_dwLastReceivedTotalSerial3;
                                    
                                    // send serial number of the last good packet to peer
                                    STCPPacket sPacket;
                                    sPacket.eType = STCPPacket::LastGoodSerial;
                                    sPacket.dwData1 = p->m_dwLastReceivedTotalSerial1;
                                    
                                    // TRACE_NETWORK("CNetworkConnection::NetworkTimerThread() - sending the serial number of the last good packet %i\n", p->m_dwLastReceivedTotalSerial1);
                                    
                                    // send data
                                    if (!p->SendTCP(&sPacket, sizeof(sPacket)))
                                    {
                                        // error
                                        TRACE_NETWORK("CNetworkConnection::NetworkTimerThread() - network error, never mind\n");
                                    }
                                    // unlock everything and exit
                                    VERIFY(p->m_lockLastReceivedTotalSerial.Unlock());
                                    VERIFY(p->m_lockReceivedBitfield.Unlock());
                                    
                                    // TRACE_NETWORK("CNetworkConnection::NetworkTimerThread() - all packets received, nothing to request\n");
                                    goto endloop;
                                }
                            }
                            
                            // TRACE_NETWORK("CNetworkConnection::NetworkTimerThread() - some packet is missing...\n");
                            
                            // send last good packet serial number to peer
                            {
                                // prepare data
                                STCPPacket sPacket;
                                sPacket.eType = STCPPacket::LastGoodSerial;
                                sPacket.dwData1 = p->m_dwLastReceivedTotalSerial1;
                                
                                // TRACE_NETWORK("CNetworkConnection::NetworkTimerThread() - sending the serial number of the last good packet %i\n", p->m_dwLastReceivedTotalSerial1);
                                
                                // send data
                                if (!p->SendTCP(&sPacket, sizeof(sPacket)))
                                {
                                    // error
                                    TRACE_NETWORK("CNetworkConnection::NetworkTimerThread() - network error\n");
                                }
                            }
                            
                            // copy pointer to the begin of bitfield list
                            SUDPPacketsReceivedBitfield *q = p->m_pReceivedBitfield;
                            
                            // find all serial numbers of lost packets (variable i left from previous for-cycle)
                            for ( ; i < dwTotalIndexes; i++)
                            {
                                ASSERT(q);
                                
                                if (q->aPacketsDelivered[dwActualIndex] != 0xffffffff)
                                {
                                    // something's lost
                                    DWORD dwMask = 0x00000001;
                                    for (BYTE j = 0; j < 32; j++, dwMask <<= 1)
                                    {
                                        if (q->dwFirst + dwActualIndex * 32 + j <= p->m_dwLastReceivedTotalSerial1)
                                            continue;
                                        
                                        if (q->dwFirst + dwActualIndex * 32 + j > p->m_dwLastReceivedTotalSerial2)
                                            break;
                                        
                                        if (!(q->aPacketsDelivered[dwActualIndex] & dwMask))
                                        {
                                            // missing packet
                                            aMissingPackets[dwMissingPackets] = q->dwFirst + (dwActualIndex * 32) + j;
                                            dwMissingPackets++;
                                            
                                            if (dwMissingPackets == dwActualRequestSize)
                                            {
                                                // have dwActualRequestSize missing packets, finish searching
                                                TRACE_NETWORK("CNetworkConnection::NetworkTimerThread() - number of missing packets reached dwActualRequestSize (%i), skipping the rest...\n", dwActualRequestSize);
                                                goto finish_search;
                                            }
                                        }
                                    }
                                }
                                
                                dwActualIndex++;
                                if (dwActualIndex == RECEIVEDPACKETS_BITFIELD_SIZE)
                                {
                                    q = q->pNext;
                                    dwActualIndex = 0;
                                }
                            }
                            
                            // label to jump to when dwActualRequestSize reached                    
                        finish_search:
                            ;
                            
                            if (dwMissingPackets > 0)
                            {
                                // send TCP request
                                
                                // prepare data
                                STCPPacket sPacket;
                                sPacket.eType = STCPPacket::SendAgain;
                                sPacket.dwData1 = dwMissingPackets;
                                
                                // send header with missing packets count
                                if (!p->SendTCP(&sPacket, sizeof(sPacket)))
                                {
                                    // thread error, unlock everything, kill timers and exit
                                    TRACE_NETWORK("CNetworkConnection::NetworkTimerThread() - TCP error\n");

                                    VERIFY(p->m_lockLastReceivedTotalSerial.Unlock());
                                    VERIFY(p->m_lockReceivedBitfield.Unlock());

                                    // kill synchronization packets timer
                                    if (uSynPacketsTimer != 0)
                                        KillTimer(NULL, uSynPacketsTimer);
                                    // kill lost packets timer
                                    if (uLostPacketsTimer != 0)
                                        KillTimer(NULL, uLostPacketsTimer);

                                    // send error event
                                    p->SendErrorEvent(E_NETWORKERROR, eTimerThread);
                                    // exit thread
                                    return 3;
                                }

                                // send the numbers of missing packets
                                if (!p->SendTCP(aMissingPackets, sizeof(DWORD) * dwMissingPackets))
                                {
                                    // thread error, unlock everything and exit
                                    TRACE_NETWORK("CNetworkConnection::NetworkTimerThread() - TCP error\n");

                                    VERIFY(p->m_lockLastReceivedTotalSerial.Unlock());
                                    VERIFY(p->m_lockReceivedBitfield.Unlock());

                                    // kill synchronization packets timer
                                    if (uSynPacketsTimer != 0)
                                        KillTimer(NULL, uSynPacketsTimer);
                                    // kill lost packets timer
                                    if (uLostPacketsTimer != 0)
                                        KillTimer(NULL, uLostPacketsTimer);

                                    // send error event
                                    p->SendErrorEvent(E_NETWORKERROR, eTimerThread);
                                    // exit thread
                                    return 3;
                                }
                                
/*
#ifdef _DEBUG
                                // TRACE_NETWORK missing packets
                                {
                                    TRACE_NETWORK("CNetworkConnection::NetworkTimerThread() - requesting following packets:  ");
                                    CString string1, string2;
                                    for (DWORD iii = 0; iii < dwMissingPackets; iii++)
                                    {
                                        string1.Format("%i ", aMissingPackets[iii]);
                                        string2 += string1;
                                    }
                                    
                                    TRACE_NETWORK("%s\n", string2);
                                }
#endif  // _DEBUG
*/
                            }
                            
                            // copy m_dwLastReceivedTotalSerial3 to m_dwLastReceivedTotalSerial2
                            p->m_dwLastReceivedTotalSerial2 = p->m_dwLastReceivedTotalSerial3;
                            
                            // unlock total serial numbers
                            VERIFY(p->m_lockLastReceivedTotalSerial.Unlock());
                            // unlock received packets bitfield
                            VERIFY(p->m_lockReceivedBitfield.Unlock());
                            
                            goto endloop;
                        }
                    
                    
                        if (msg.wParam == uSynPacketsTimer)
                        {
                            // let's send a synchronization packet
                            SSynchronizationPacket sSynchroPacket;
                            
                            // copy the serial number of the last packet sent
                            VERIFY(p->m_lockTotalSerial.Lock());
                            sSynchroPacket.dwTotalSerial = p->m_dwTotalSerial - 1;
                            VERIFY(p->m_lockTotalSerial.Unlock());
                            
                            // copy the serial number of the last good packet
                            VERIFY(p->m_lockLastReceivedTotalSerial.Lock());
                            sSynchroPacket.dwLastGoodSerial = p->m_dwLastReceivedTotalSerial1;
                            VERIFY(p->m_lockLastReceivedTotalSerial.Unlock());
                            
                            // use virtual connection 0
                            VERIFY(p->m_lockVirtualConnectionList.Lock());
                            ASSERT(p->m_pVirtualConnectionList != NULL);
                            ASSERT(p->m_pVirtualConnectionList->wVirtualConnection == 0);
                            p->SendBlockUnsafely(p->m_pVirtualConnectionList, &sSynchroPacket, sizeof(sSynchroPacket), PACKETPRIORITY_HIGHEST, FALSE);
                            VERIFY(p->m_lockVirtualConnectionList.Unlock());
                            
                            goto endloop;
                        }
                        
                        // SHOULDN'T BE REACHED
                        TRACE_NETWORK("CNetworkConnection::NetworkTimerThread() - unexpected message parameter!\n");
                        ASSERT(FALSE);
                        goto endloop;
                    } // switch (GetMessage())
                
                } // endless loop
            endloop: ;
            
            } // case WAIT_OBJECT_0 + 1:  (timer message)
            break;
            
        default:
            {
                // this should never be reached
                TRACE_NETWORK("CNetworkConnection::NetworkTimerThread() - MsgWaitForMultipleObjects() unexpected return code\n");
                ASSERT(FALSE);
            }

        } // switch (MsgWaitForMultipleObjects())

    } // endless loop

    // should not be reached
	TRACE_NETWORK("CNetworkConnection::NetworkTimerThread() - wrong code reached\n");
	ASSERT(FALSE);
    return 4;
}
