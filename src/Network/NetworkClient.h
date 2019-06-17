// NetworkClient.h: interface for the CNetworkClient class
//////////////////////////////////////////////////////////////////////

#if !defined(_NETWORKCLIENT__HEADER_INCLUDED_)
#define _NETWORKCLIENT__HEADER_INCLUDED_

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

//////////////////////////////////////////////////////////////////////
// Network client class
//////////////////////////////////////////////////////////////////////

class CNetworkClient : public CNetwork  
{
    DECLARE_DYNAMIC(CNetworkClient)

public:
    // empty constructor
    CNetworkClient() : 
        m_bConnected(FALSE),
        m_pNetworkConnection(NULL)
    { };
    // destructor
    virtual ~CNetworkClient();

    // methods Create() and Delete() are inherited from CNetwork w/o change

    // connect to server (establish a control connection)
    virtual CNetworkConnection * ConnectToServer(const CString &strServerName, WORD wServerPort, const CString &strLoginName, const CString &strLoginPassword, CObserver *pObserver, DWORD dwNotifierID, DWORD dwThreadID = 0);

    // disconnect from server - parameter has some special values
    //    0  -  disconnect immediately (VERY dirty)
    //  INFINITE  -  block sending, abort all receiving,
    //               wait forever for all corrections and then 
    //               wait for all threads to finish
    //  other values  -  block sending, abort receiving, wait
    //                   for all corrections (the specified time in milliseconds)
    //                   and then wait for all threads to finish
    virtual void DisconnectFromServer(DWORD dwTimeOut = INFINITE);

#ifdef _DEBUG
    // check content of object
    virtual void AssertValid() const;
    // dump content of object
    virtual void Dump(CDumpContext &dc) const;
#endif

protected:
    // connection to server
    CNetworkConnection *m_pNetworkConnection;

    // logout function (called by CNetworkConnection)
    virtual void Logout(CNetworkConnection *);

	// clean this object after unsuccessful connect to server
    virtual void ConnectCancel();

    // client connected to server
    BOOL m_bConnected;
};

#endif // !defined(_NETWORKCLIENT__HEADER_INCLUDED_)
