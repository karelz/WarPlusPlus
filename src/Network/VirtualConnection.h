// VirtualConnection.h: interface for the CVirtualConnection class
//////////////////////////////////////////////////////////////////////

#if !defined(_VIRTUALCONNECTION__HEADER_INCLUDED_)
#define _VIRTUALCONNECTION__HEADER_INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PacketTypes.h"

// forward declaration
class CNetworkConnection;

class CVirtualConnection : public CObject  
{
    DECLARE_DYNAMIC(CVirtualConnection)

public:
    // default empty constructor
    CVirtualConnection() : m_pNetworkConnection(NULL), m_wVirtualConnection(0) { }
    // constructor taking virtual connection number
    inline CVirtualConnection(CNetworkConnection *pNetworkConnection, WORD wVirtualConnection);
    // copy constructor
    inline CVirtualConnection(const CVirtualConnection &Source);
    // empty destructor
    virtual ~CVirtualConnection() { }
    // operator =
    inline const CVirtualConnection & operator = (const CVirtualConnection &Source);

    // operator ==
    inline BOOL operator == (const CVirtualConnection &Source) const;
    // operator !=
    inline BOOL operator != (const CVirtualConnection &Source) const;

    // delete this virtual connection
    inline BOOL DeleteVirtualConnection();

    // returns the virtual connection number (word)
    WORD GetVirtualConnectionNumber() const { return m_wVirtualConnection; }

    // set observer for this virtual connection
    inline BOOL SetVirtualConnectionObserver(CObserver *pObserever, DWORD dwNotID, DWORD dwThreadID = 0);

    // send single block or add data to compound block
    inline BOOL SendBlock(const void *pBuffer, DWORD dwBlockSize, BOOL bCautiousSend = FALSE, BYTE btPacketPriority = PACKETPRIORITY_ACTUAL) const;
    // receive single block or get part of data from compound block
    inline BOOL ReceiveBlock(void *pBuffer, DWORD &dwBlockSize) const;

    // check the network status
    inline BOOL IsNetworkReady() const;
    
    // begin send compound block
    inline BOOL BeginSendCompoundBlock(BOOL bCautiousSend = FALSE, BYTE btPacketPriority = PACKETPRIORITY_ACTUAL) const;
    // end send compound block
    inline BOOL EndSendCompoundBlock() const;
    // begin receive compound block
    inline BOOL BeginReceiveCompoundBlock() const;
    // end receive compound block
    inline BOOL EndReceiveCompoundBlock() const;

    // get maximum message size
    // this can be used as maximum block size for blocks sent over
    // unsecure connections (PACKET_TYPE3 or PACKET_TYPE4)
    inline DWORD GetMaximumMessageSize() const;

    // get the size of data in the last packet of actual send compound block
    // (when send compound block is not enabled, returns 0)
    // (does not make any sense for secure connections (PACKET_TYPE1 or PACKET_TYPE2)
    inline DWORD GetActualDataSize() const;

    // get base priority for virtual connection
    inline BYTE GetVirtualConnectionPriority() const;
    // set base priority for virtual connection
    inline BOOL SetVirtualConnectionPriority(BYTE btBasePriority = PACKETPRIORITY_DEFAULT);


#ifdef _DEBUG
    // check content of object
    virtual void AssertValid() const;
    // dump content of object
    virtual void Dump(CDumpContext &dc) const;
#endif  // _DEBUG

protected:
    // pointer to CNetworkConnection object (our creator)
    CNetworkConnection *m_pNetworkConnection;
    // virtual connection nubmer
    WORD m_wVirtualConnection;

    // CNetworkConnection should be friend
    friend class CNetworkConnection;
};


#endif  // !defined(_VIRTUALCONNECTION__HEADER_INCLUDED_)
