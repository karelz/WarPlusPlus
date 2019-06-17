// NetworkConnection.h: interface for the CNetworkConnection class
//////////////////////////////////////////////////////////////////////

#if !defined(_NETWORKCONNECTION__HEADER_INCLUDED_)
#define _NETWORKCONNECTION__HEADER_INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Network.h"

// forward declaration of CVirtualConnection class
class CVirtualConnection;

// define fictive class CLock
#ifdef _DEBUG
#define CLock CSemaphore
#else
#define CLock CCriticalSection
#endif

// constants for thread identification
enum tagENetworkConnectionThread
{
    eControlThread, eReceiverThread, eSenderThread, eTimerThread
};

// size of received packets bitfield
#define RECEIVEDPACKETS_BITFIELD_SIZE 512


//////////////////////////////////////////////////////////////////////
// basic connection object
//////////////////////////////////////////////////////////////////////

class CNetworkConnection : public CMultithreadNotifier
{
    DECLARE_DYNAMIC(CNetworkConnection);

protected:
    // UDP packet header
    // -----------------
    // There are four types of packets:
    //   1 .. guaranteed delivery and order (reliable connection)
    //   2 .. guaranteed delivery but order of incoming packets
    //   3 .. guaranteed order, packets may get lost
    //   4 .. neither delivery nor order guaranteed, just like ordinary UDP
    // In addition - packets of type 1 and 2 can be fragmented, then a new part
    // of header must appear
    
    // common part of header for all types of packets
    struct SUDPPacketCommonHeader
    {
        BYTE PacketFlags;
        BYTE PacketPriority;
        WORD VirtualConnection;

        // operator delete here is used for deleting types of packets
        // defined below, SUDPPacketCommonHeader is never
        // allocated separately
        void operator delete(void *p);
    };
    
// disable warning "nonstandard extension used : zero-sized array in struct/union"
#pragma warning(push)
#pragma warning(disable: 4200)
    
    // packet of type 1, not fragmented
    struct SUDPPacketType1 : public SUDPPacketCommonHeader
    {
        DWORD TotalSerial;
        DWORD ConnectionSerial;
        BYTE Buffer[];
        void *operator new(size_t stAllocationSize, DWORD dwBufferSize);
        // operator delete is defined in SUDPPacketCommonHeader
    };
    
    // packet of type 1, fragmented
    struct SUDPPacketType1F : public SUDPPacketCommonHeader
    {
        DWORD TotalSerial;
        DWORD ConnectionSerial;
        DWORD FragmentNumber;
        BYTE Buffer[];
        void *operator new(size_t stAllocationSize, DWORD dwBufferSize);
        // operator delete is defined in SUDPPacketCommonHeader
    };
    
    // packet of type 2, not fragmented
    struct SUDPPacketType2 : public SUDPPacketCommonHeader
    {
        DWORD TotalSerial;
        BYTE Buffer[];
        void *operator new(size_t stAllocationSize, DWORD dwBufferSize);
        // operator delete is defined in SUDPPacketCommonHeader
    };
    
    // packet of type 2, fragmented
    struct SUDPPacketType2F : public SUDPPacketCommonHeader
    {
        DWORD TotalSerial;
        DWORD FragmentNumber;
        BYTE Buffer[];
        void *operator new(size_t stAllocationSize, DWORD dwBufferSize);
        // operator delete is defined in SUDPPacketCommonHeader
    };
    
    // packet of type 3
    struct SUDPPacketType3 : public SUDPPacketCommonHeader
    {
        DWORD ConnectionSerial;
        BYTE Buffer[];
        void *operator new(size_t stAllocationSize, DWORD dwBufferSize);
        // operator delete is defined in SUDPPacketCommonHeader
    };
    
    // packet of type 4
    struct SUDPPacketType4 : public SUDPPacketCommonHeader
    {
        BYTE Buffer[];
        void *operator new(size_t stAllocationSize, DWORD dwBufferSize);
        // operator delete is defined in SUDPPacketCommonHeader
    };
    
// restore previous warning behavior
#pragma warning(pop)

    // structure for TCP packets
    struct STCPPacket
    {
        enum { SendAgain, LastGoodSerial, NewVirtualConnection, NewVirtualConnectionAck, NewVirtualConnectionNegAck, CloseVirtualConnection, NormalClose, AbortiveClose } eType;
        DWORD dwData1, dwData2;
    };
    
	// structure for holding received packets in lists
    struct SReceivePacketList
    {
        SReceivePacketList *pNext;
        BYTE btPacketFlags;
        DWORD dwConnectionSerial;
        DWORD dwTotalFragments;
        DWORD dwFragmentNumber;
        DWORD dwDataSize;
        BYTE *pBuffer;
    };
    
    // structure for holding received packets in queues    
    struct SReceivePacketQueue
    {
        CLock lock;
        SReceivePacketList *first, **last;
    };
    
    // structure for holding sent packets in lists
    struct SSendPacketList
    {
        SSendPacketList *pNext;
        DWORD dwPacketSize;
        DWORD dwTotalSerialNumber;
        BOOL bCautiousSend;
        SUDPPacketCommonHeader *pPacket;
    };
    
    // structure for holding sent packets in queues    
    struct SSendPacketQueue
    {
        CLock lock;
        SSendPacketList *first, **last;
        DWORD dwSentPacketsNumber;
        DWORD dwSentPacketsSize;
    };

    // structure for network connection statistics
    struct SNetworkStatistics
    {
        // reconsider sizes!! DWORD is sometimes too small!!!
        DWORD dwPacketsLost;
        DWORD dwPacketsDuplicated;
        DWORD dwPacketsBad;
        DWORD dwPacketsSent;
        DWORD dwPacketsSentT1;
        DWORD dwPacketsSentT1F;
        DWORD dwPacketsSentT2;
        DWORD dwPacketsSentT2F;
        DWORD dwPacketsSentT3;
        DWORD dwPacketsSentT4;
        DWORD dwBytesSent;
        DWORD dwPacketsReceived;
        DWORD dwPacketsReceivedT1;
        DWORD dwPacketsReceivedT1F;
        DWORD dwPacketsReceivedT2;
        DWORD dwPacketsReceivedT2F;
        DWORD dwPacketsReceivedT3;
        DWORD dwPacketsReceivedT4;
        DWORD dwBytesReceived;
        DWORD aPacketsOfPriority[PACKETPRIORITY_TOTAL_PRIORITIES];
        DWORD dwActiveConnections;
    };

    // structure holding information about virtual connection
    struct SVirtualConnection
    {
        // connection number
        WORD wVirtualConnection;
        
        // create event
        CEvent eventCreateAcknowledged;
        
        // information needed for sending
        BYTE btSendPacketFlags;
        BYTE btSendConnectionPriority;
        DWORD dwSendSerial;
        DWORD dwSendFragmentNumber;
        DWORD dwSendTotalFragments;

        CLock lockWaitForSend;
        CEvent eventWaitForSend;
        
        BOOL bSendCompoundBlockActive;
        BOOL bSendCompoundBlockCautious;
        BYTE btSendCompoundBlockPriority;
        BYTE *pSendCompoundBlockBuffer;
        DWORD dwSendCompoundBlockSize;

        // thread checking
        CMutex mutexSending;
        
        // information needed for receiving
        CSemaphore semReceiveDataReady;
        CObserver *pObserver;
        BOOL bDeleted;
        SReceivePacketQueue qReceivePackets;
        DWORD dwReceiveLastSerial;
        DWORD dwReceiveLastAwaitedSerial;

        BOOL bReceiveCompoundBlockActive;
        DWORD dwReceiveCompoundBlockOffset;
        BOOL bReceiveCompoundBlockStopper;

        // pointer to next virtual connection
        SVirtualConnection *pNext;
        
        // constructor - inline
        SVirtualConnection() : 
            semReceiveDataReady(0, LONG_MAX),
            dwSendSerial(1),
            dwReceiveLastSerial(0),
            dwReceiveLastAwaitedSerial(0),
            dwSendFragmentNumber(0),
            dwSendTotalFragments(0),
            bSendCompoundBlockActive(FALSE),
            bDeleted(FALSE),
            bReceiveCompoundBlockActive(FALSE),
            pObserver(NULL),
            pNext(NULL)
            // the rest is set by methods CreateVirtualConnection(),
            // BeginSendCompoundBlock(), BeginReceiveCompoundBlock(),
            // etc...
        { 
            qReceivePackets.first = NULL;
            qReceivePackets.last = &(qReceivePackets.first);
        };
    };

    // structure holding bit field of packets received
    struct SUDPPacketsReceivedBitfield
    {
        SUDPPacketsReceivedBitfield *pNext;
        DWORD dwFirst;
        DWORD aPacketsDelivered[RECEIVEDPACKETS_BITFIELD_SIZE];
    };

    // structure of synchronization packet (used on virtual connection 0)
    struct SSynchronizationPacket
    {
        DWORD dwTotalSerial;
        DWORD dwLastGoodSerial;
    };

public:
    // structure passed from control thread to application when
    // request to create new virtual connection was received
    struct SNewVirtualConnectionUserData
    {
        CVirtualConnection VirtualConnection;
        CEvent *pWakeUpEvent;
        DWORD dwSize;
        void *pBuffer;

        // constructor
        SNewVirtualConnectionUserData(CNetworkConnection *pNetworkConnection, WORD wVirtualConnection) :
            VirtualConnection(pNetworkConnection, wVirtualConnection)
        { }
    };

    // empty constructor
    CNetworkConnection() : 
        m_bInitialized(FALSE),
        m_bIsNetworkReady(FALSE),
        m_bSendErrorActivated(FALSE),
        m_bClosing1(FALSE),
        m_bClosing2(FALSE),
        m_semPacketsPreparedToSend(0, LONG_MAX),
        m_UDPRecvSocket(INVALID_SOCKET),
        m_UDPSendSocket(INVALID_SOCKET),
        m_TCPSendSocket(INVALID_SOCKET),
        m_TCPRecvSocket(INVALID_SOCKET),
        m_pPrevConnection(NULL),
        m_pNextConnection(NULL),
        m_bPerformUDPAliveChecking(TRUE),
        m_bUDPPacketReceived(FALSE),
        m_dwTicksFromLastUDPPacket(0),
        m_pObserver(NULL)
    { };
	// destructor calls Delete() if necessary
    virtual ~CNetworkConnection();

    // Create() is protected, this object can be constructed only by
    // CNetworkServer or by CNetworkClient classes
    
    // Delete()
    virtual void Delete(DWORD dwTimeOut = INFINITE);

    // returns actual state of the network connection
    virtual BOOL IsNetworkReady() { return m_bIsNetworkReady; }
    
    // create virtual connection, return its number
    virtual WORD CreateVirtualConnection(BYTE btConnectionFlags, CObserver *pObserver, DWORD dwNotID, DWORD dwThreadID = 0, const void *pUserData = NULL, DWORD dwSize = 0, BYTE btBasePriority = PACKETPRIORITY_DEFAULT);
    // create virtual connection, return pointer to CVirtualConnection object
    virtual CVirtualConnection CreateVirtualConnectionEx(BYTE btConnectionFlags, CObserver *pObserver, DWORD dwNotID, DWORD dwThreadID = 0, const void *pUserData = NULL, DWORD dwSize = 0, BYTE btBasePriority = PACKETPRIORITY_DEFAULT);

    // set observer for virtual connection (by number)
    virtual BOOL SetVirtualConnectionObserver(WORD wConnectionNumber, CObserver *pObserver, DWORD dwNotID, DWORD dwThreadID = 0);
    // set observer for virtual connection (by CVirtualConnection pointer)
    virtual BOOL SetVirtualConnectionObserver(const CVirtualConnection &VirtualConnection, CObserver *pObserver, DWORD dwNotID, DWORD dwThreadID = 0) { return SetVirtualConnectionObserver(VirtualConnection.m_wVirtualConnection, pObserver, dwNotID, dwThreadID); }

    // delete virtual connection (by its number)
    virtual BOOL DeleteVirtualConnection(WORD wConnectionNumber);
    // delete virtual connection (by the CVirtualConnection pointer)
    virtual BOOL DeleteVirtualConnection(const CVirtualConnection &VirtualConnection);
    
    // get base priority for virtual connection (by connection number)
    virtual BYTE GetVirtualConnectionPriority(WORD wConnectionNumber);
    // get base priority for virtual connection (by CVirtualConnection object)
    virtual BYTE GetVirtualConnectionPriority(const CVirtualConnection &VirtualConnection) { return GetVirtualConnectionPriority(VirtualConnection.m_wVirtualConnection); }
    // set base priority for virtual connection (by connection number)
    virtual BOOL SetVirtualConnectionPriority(WORD wConnectionNumber, BYTE btBasePriority = PACKETPRIORITY_DEFAULT);
    // set base priority for virtual connection (by CVirtualConnection object)
    virtual BOOL SetVirtualConnectionPriority(const CVirtualConnection &VirtualConnection, BYTE btBasePriority = PACKETPRIORITY_DEFAULT) { return SetVirtualConnectionPriority(VirtualConnection, btBasePriority); }

    // send single block or add data to compound block (by connection number)
    virtual BOOL SendBlock(WORD wConnectionNumber, const void *pBuffer, DWORD dwBlockSize, BOOL bCautiousSend = FALSE, BYTE btPacketPriority = PACKETPRIORITY_ACTUAL);
    // send single block or add data to compound block (by CVirtualConnection object)
    virtual BOOL SendBlock(const CVirtualConnection &VirtualConnection, const void *pBuffer, DWORD dwBlockSize, BOOL bCautiousSend = FALSE, BYTE btPacketPriority = PACKETPRIORITY_ACTUAL) { return SendBlock(VirtualConnection.m_wVirtualConnection, pBuffer, dwBlockSize, bCautiousSend, btPacketPriority); }
    // receive single block or get part of data from compound block (by connection number)
    virtual BOOL ReceiveBlock(WORD wConnectionNumber, void *pBuffer, DWORD &dwBlockSize);
    // receive single block or get part of data from compound block (by CVirtualConnection object)
    virtual BOOL ReceiveBlock(const CVirtualConnection &VirtualConnection, void *pBuffer, DWORD &dwBlockSize) { return ReceiveBlock(VirtualConnection.m_wVirtualConnection, pBuffer, dwBlockSize); }
    
    // begin send compound block (by connection number)
    virtual BOOL BeginSendCompoundBlock(WORD wConnectionNumber, BOOL bCautiousSend = FALSE, BYTE btPacketPriority = PACKETPRIORITY_ACTUAL);
    // begin send compound block (by CVirtualConnection object)
    virtual BOOL BeginSendCompoundBlock(const CVirtualConnection &VirtualConnection, BOOL bCautiousSend = FALSE, BYTE btPacketPriority = PACKETPRIORITY_ACTUAL) { return BeginSendCompoundBlock(VirtualConnection.m_wVirtualConnection, bCautiousSend, btPacketPriority); }
    // end send compound block (by connection number)
    virtual BOOL EndSendCompoundBlock(WORD wConnectionNumber);
    // end send compound block (by CVirtualConnection object)
    virtual BOOL EndSendCompoundBlock(const CVirtualConnection &VirtualConnection) { return EndSendCompoundBlock(VirtualConnection.m_wVirtualConnection); }

    // get maximum message size
    // this can be used as maximum block size for blocks sent over
    // unsecure connections (PACKET_TYPE3 or PACKET_TYPE4)
    virtual DWORD GetMaximumMessageSize() { return m_dwMaximumMessageSize; }

    // get the size of data in the last packet of actual send compound block
    // (when send compound block is not enabled, returns 0)
    // (does not make any sense for secure connections (PACKET_TYPE1 or PACKET_TYPE2))
    virtual DWORD GetActualDataSize(WORD wConnectionNumber);
    // get the size of data in the last packet of actual send compound block
    // (when send compound block is not enabled, returns 0)
    // (does not make any sense for secure connections (PACKET_TYPE1 or PACKET_TYPE2))
    virtual DWORD GetActualDataSize(const CVirtualConnection &VirtualConnection) { return GetActualDataSize(VirtualConnection.m_wVirtualConnection); }
    
    // begin receive compound block (by connection number)
    virtual BOOL BeginReceiveCompoundBlock(WORD wConnectionNumber);
    // begin receive compound block (by CVirtualConnection object)
    virtual BOOL BeginReceiveCompoundBlock(const CVirtualConnection &VirtualConnection) { return BeginReceiveCompoundBlock(VirtualConnection.m_wVirtualConnection); }
    // end receive compound block (by connection number)
    virtual BOOL EndReceiveCompoundBlock(WORD wConnectionNumber);
    // end receive compound block (by CVirtualConnection object)
    virtual BOOL EndReceiveCompoundBlock(const CVirtualConnection &VirtualConnection) { return EndReceiveCompoundBlock(VirtualConnection.m_wVirtualConnection); }

    // get network statistics
    virtual void GetNetworkStatistics(SNetworkStatistics &sStats);

    // set/reset UDP alive checking
    virtual void SetUDPAliveChecking(BOOL bEnable = TRUE) { m_bPerformUDPAliveChecking = bEnable; }

#ifdef _DEBUG
    // check content of object
    virtual void AssertValid() const;
    // dump content of object
    virtual void Dump(CDumpContext &dc) const;
#endif

protected:
    // creates an CNetworkConnection object 
    // (parameter is a pointer to either client or server creating the connection, ie. caller)
    virtual void Create(CNetwork *pNetworkObject);
    // function deletes contents of object initialized by Create()
    virtual void CreateCancel();
    // additional initializations common to client and server
    virtual void PostCreate(sockaddr sAddr);

    // error routine - sends safely ONE error event (not more...)
    virtual void SendErrorEvent(ENetworkEvents eEvent, DWORD dwEventParam);

    // send a block over TCP connection, indicate error in return value
    virtual BOOL SendTCP(const void *pBuffer, DWORD dwSize, int *err = NULL);
    // receive a block from TCP connection, indicate error in return value
    virtual BOOL ReceiveTCP(void *pBuffer, DWORD dwSize, int *err = NULL);

    // constants for recovery type
    enum ERecoverConnection { E_RECOVER_SEND, E_RECOVER_RECEIVE };

    // try to recover the network connection by connecting TCP sockets again
    virtual BOOL RecoverConnection(ERecoverConnection eWhatRecover);
    
    // send packet unsafely (do not lock locks etc.)
	virtual void SendBlockUnsafely(SVirtualConnection *pVirtualConnection, const void *pBuffer, DWORD dwBlockSize, BYTE btPacketPriority, BOOL bCautiousSend);
	// end send compound block unsafely (do not lock locks etc.)
    virtual BOOL EndSendCompoundBlockUnsafely(SVirtualConnection *pVirtualConnection);
	// end receive compound block unsafely (do not lock locks etc.)
    virtual void EndReceiveCompoundBlockUnsafely(SVirtualConnection *pVirtualConnection);

    // object is initialized (Create() was called)
    BOOL m_bInitialized;
    // actual status of the network connection
    BOOL m_bIsNetworkReady; 
    // object is closing gracefuly (sending blocked, receiving canceled,
    // waiting for corrections only)
    BOOL m_bClosing1, m_bClosing2;
    // event for sender thread
    // signaled when sendet thread has finished all sending
    CEvent m_eventClosingSender;
    // event for waiting for sent packets queue to be empty
    CEvent m_eventClosingSentPackets;

    // an error event was sent already, waiting for death...
    BOOL m_bSendErrorActivated;
    // lock for the the event boolean
    CLock m_lockErrorEvent;

    // pointer to main network object
    CNetwork *m_pNetworkObject;
    // pointer to observer, which will be informed about new virtual connections, etc.
    CObserver *m_pObserver;
    
    // remote host name
    CString m_strRemoteHostName;
    // remote IP address
    CString m_strRemoteIPAddress;
    // remote TCP send port
    WORD m_wRemoteTCPSendPort;
    // remote TCP receive port
    WORD m_wRemoteTCPRecvPort;
    // remote UDP send port
    WORD m_wRemoteUDPSendPort;
    // remote UDP receive port
    WORD m_wRemoteUDPRecvPort;
    // local TCP send port
    WORD m_wLocalTCPSendPort;
    // local TCP receive port
    WORD m_wLocalTCPRecvPort;
    // local UDP send port
    WORD m_wLocalUDPSendPort;
    // local UDP receive port
    WORD m_wLocalUDPRecvPort;

    // maximum size of data in packet
    DWORD m_dwMaximumMessageSize;
    // maximum size of packet
    DWORD m_dwMaximumPacketSize;
    
    // TCP socket for sending
    SOCKET m_TCPSendSocket;
    // TCP socket for receiving
    SOCKET m_TCPRecvSocket;
    // lock for TCP send socket (it is possible, that more than one thread will use it at one time)
    CLock m_lockTCPSendSocket;
    // lock for TCP receive socket (it is possible, that more than one thread will use it at one time)
    CLock m_lockTCPRecvSocket;
    // UDP socket for receiving (one thread use it only, no lock needed)
    SOCKET m_UDPRecvSocket;
    // UDP socket for sending
    SOCKET m_UDPSendSocket;
    // lock for UDP send socket
    CLock m_lockUDPSendSocket;

    // receiver thread buffer
    BYTE *m_pReceiverThreadBuffer;

    // statistic structure
    SNetworkStatistics m_sNetworkStatistics;
    // lock for network statistics structure
    CLock m_lockNetworkStatistics;

    // list of active virtual connections
    SVirtualConnection *m_pVirtualConnectionList;
    // lock for virtual connections
    CLock m_lockVirtualConnectionList;

    // number for new connection
    WORD m_wNewConnectionNumber;
    // serial number for outgoing packets
    DWORD m_dwTotalSerial;
    // lock for total serial
    CLock m_lockTotalSerial;

    // prepared packets event
    CSemaphore m_semPacketsPreparedToSend;
    // packet queue for packets prepared to send
    SSendPacketQueue m_qPacketsPreparedToSend[PACKETPRIORITY_TOTAL_PRIORITIES];
    // packet queue for sent packets (type 1 and 2 only)
    SSendPacketQueue m_qPacketsSent;
    
    // bitfield for received packets
    SUDPPacketsReceivedBitfield *m_pReceivedBitfield;
    // bitfield lock
    CLock m_lockReceivedBitfield;

    // next and previous connection in the list of connections
    CNetworkConnection * m_pNextConnection;
    CNetworkConnection * m_pPrevConnection;

    // serial number of last packet in last completed block number
    DWORD m_dwLastReceivedTotalSerial1;
    // serial number of last packet in "time-out" area
    DWORD m_dwLastReceivedTotalSerial2;
    // serial number of very last received packet
    DWORD m_dwLastReceivedTotalSerial3;
    // lock for last serial numbers
    CLock m_lockLastReceivedTotalSerial;

    // thread function -- sender
    static UINT NetworkUDPSenderThread(LPVOID pParam);
    // thread function -- receiver
    static UINT NetworkUDPReceiverThread(LPVOID pParam);
    // thread function -- control connection
    static UINT NetworkControlThread(LPVOID pParam);
    // thread function -- timer thread
    static UINT NetworkTimerThread(LPVOID pParam);

    // handles to all threads
    CWinThread *m_aThreadHandles[4];
    
    // sender thread function killer
    CEvent m_eventSenderKill;
    // receiver thread function killer
    CEvent m_eventReceiverKill;
    // control connection thread function killer
    CEvent m_eventControllerKill;
    // timer thread function killer
    CEvent m_eventTimerKill;

    // actual size of the request packet
    DWORD m_dwRequestPacketSize;
    // lock for the actual request packet size
    CLock m_lockRequestPacketSize;

    // synchronization packets timer frequency
    UINT m_uSynPacketsPeriod;
    // lost packets timer frequency
    UINT m_uLostPacketsPeriod;

    // UDP alive checking
    // TRUE if some UDP packet was received recently
    // (timer thread resets this bool)
    BOOL m_bUDPPacketReceived;
    // number of timer ticks from the last received UDP packet
    DWORD m_dwTicksFromLastUDPPacket;
    // perform alive checking?
    BOOL m_bPerformUDPAliveChecking;

    // cliend and server have to be friends, both access the members
    // of CNetworkConnection while connecting...
    friend class CNetworkClient;
    friend class CNetworkServer;
};

// undefine CLock
#undef CLock


#endif // !defined(_NETWORKCONNECTION__HEADER_INCLUDED_)
