// Network.h: interface for the CNetwork class
//////////////////////////////////////////////////////////////////////

#if !defined(_NETWORK__HEADER_INCLUDED_)
#define _NETWORK__HEADER_INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <winsock2.h>

// forward declaration
class CNetworkConnection;

//////////////////////////////////////////////////////////////////////
// basic network object
// used for attach to Winsock DLL
//////////////////////////////////////////////////////////////////////

/* 
   in fact it is not necessary to derive CNetwork from
   CMultithreadNotifier because CNetwork itself won't send
   any events, but both its successors - CNetworkClient and
   CNetworkServer - will send events...
*/

class CNetwork : public CMultithreadNotifier  
{
    DECLARE_DYNAMIC(CNetwork)

public:
    // constructor is protected, making instances of this class is not allowed

    // destructor calls Delete() member funciton if necessary
    virtual ~CNetwork();

    // initialize WinSock, query for maximum packet size
	virtual void Create();
    // disconnect from Winsock DLL
    virtual void Delete();

    // return highest supported version of Winsock DLL
    virtual WORD GetNetworkVersion() { ASSERT_VALID(this); return m_wNetworkVersion; }
    // return description of Winsock DLL
    virtual CString GetNetworkDescription() { ASSERT_VALID(this); return m_strNetworkDescription; }
    // return maximum size of packet
    virtual DWORD GetMaximumPacketSize() { ASSERT_VALID(this); return m_dwMaximumPacketSize; }

#ifdef _DEBUG
    // check content of object
    virtual void AssertValid() const;
    // dump content of object
    virtual void Dump(CDumpContext &dc) const;
#endif

protected:
    // empty constructor
    // protected => instance of this class can't be created, but
    //              it is possible to derive new class from this one
    CNetwork() : 
         m_bInitialized(FALSE)
    { }

    // logout function (called when connection finished)
    virtual void Logout(CNetworkConnection *p) = 0;

    // send a block over given TCP socket, in case of error throws an CNetworkException
    virtual void SendTCP(SOCKET Sock, const void *pBuffer, DWORD dwSize);
    // receive a block from given TCP socket, in case of error throws an CNetworkException
    virtual void ReceiveTCP(SOCKET Sock, void *pBuffer, DWORD dwSize);

/*
    // send a block over given TCP socket, report errors in the last parameter
    virtual BOOL SendTCPErr(SOCKET Sock, const void *pBuffer, DWORD dwSize, int *err = NULL);
    // receive a block over given TCP socket, report errors in the last parameter
    virtual BOOL ReceiveTCPErr(SOCKET Sock, void *pBuffer, DWORD dwSize, int *err = NULL);
*/

    // structure used while connection process
    struct SInitTCPPacket
    {
        DWORD dwChallenge1;
        DWORD dwChallenge2;
        char cClientName[256];
        char cClientIP[16];
        DWORD dwMaximumPacketSize;
        WORD wTCPLocalSendPort;
        WORD wTCPLocalRecvPort;
        WORD wUDPLocalSendPort;
        WORD wUDPLocalRecvPort;
        char cLoginName[32];
        char cLoginPassword[32];

        static const DWORD dwChallengeA;
        static const DWORD dwChallengeB;

        enum 
        { 
            Bad_Challenge, 
            Server_Error, 
            Login_Failed, 
            Bad_Password, 
            Unknown_User, 
            Already_Logged_On, 
            Game_Not_Running, 
            User_Disabled,

            // number of enum items
            Last_Item
        };
    };

    // TRUE if Create() was called successfuly
	BOOL m_bInitialized;

    // highest supported version of Winsock DLL
    WORD m_wNetworkVersion;

    // description of Winsock DLL
    CString m_strNetworkDescription;

    // default maximum packet size
    // used in case getsockopt(SO_MAX_MSG_SIZE) fails
    static const DWORD m_dwDefaultMaximumPacketSize;
    
    // size of largest packet in UDP (getsockopt())
    DWORD m_dwMaximumPacketSize;

    // local host name
    CString m_strLocalHostName;
    // local IP address
    CString m_strLocalIPAddress;

    // connection class has to be friend because of Logout() call,
    // which is called when closing connection
    friend class CNetworkConnection;
};

#endif // !defined(_NETWORK__HEADER_INCLUDED_)
