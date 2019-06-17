// VirtualConnection.inl: inline functions of the CVirtualConnection class
//////////////////////////////////////////////////////////////////////////

#if !defined(_VIRTUALCONNECTION__INLINE_INCLUDED_)
#define _VIRTUALCONNECTION__INLINE_INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


//////////////////////////////////////////////////////////////////////////
inline CVirtualConnection::CVirtualConnection(CNetworkConnection *pNetworkConnection, WORD wVirtualConnection)
{ 
    m_pNetworkConnection = pNetworkConnection; 
    m_wVirtualConnection = wVirtualConnection; 
}

//////////////////////////////////////////////////////////////////////////
inline CVirtualConnection::CVirtualConnection(const CVirtualConnection &Source)
{ 
    m_pNetworkConnection = Source.m_pNetworkConnection; 
    m_wVirtualConnection = Source.m_wVirtualConnection; 
}

//////////////////////////////////////////////////////////////////////////
inline const CVirtualConnection & CVirtualConnection::operator = (const CVirtualConnection &Source)
{ 
    m_pNetworkConnection = Source.m_pNetworkConnection;
    m_wVirtualConnection = Source.m_wVirtualConnection; 
    return (*this);
}

//////////////////////////////////////////////////////////////////////////
inline BOOL CVirtualConnection::operator == (const CVirtualConnection &Source) const
{
    return ((Source.m_pNetworkConnection == m_pNetworkConnection) && (Source.m_wVirtualConnection == m_wVirtualConnection));
}

//////////////////////////////////////////////////////////////////////////
inline BOOL CVirtualConnection::operator != (const CVirtualConnection &Source) const
{
    return ((Source.m_pNetworkConnection != m_pNetworkConnection) || (Source.m_wVirtualConnection != m_wVirtualConnection));
}

//////////////////////////////////////////////////////////////////////////
inline BOOL CVirtualConnection::DeleteVirtualConnection()
{
    if ((m_pNetworkConnection == NULL) || (m_wVirtualConnection == 0))
    {
        // do nothing, it is not initialized

        // paranoia
        ASSERT((m_pNetworkConnection == NULL) && (m_wVirtualConnection == 0));
        return TRUE;
    }
    else
    {
        return m_pNetworkConnection->DeleteVirtualConnection(m_wVirtualConnection); 
    }
}

//////////////////////////////////////////////////////////////////////////
inline CVirtualConnection::SetVirtualConnectionObserver(CObserver *pObserver, DWORD dwNotID, DWORD dwThreadID)
{
    ASSERT_VALID(this);
    ASSERT_KINDOF(CObserver, pObserver);
    return m_pNetworkConnection->SetVirtualConnectionObserver(m_wVirtualConnection, pObserver, dwNotID, dwThreadID);
}

//////////////////////////////////////////////////////////////////////////
inline BOOL CVirtualConnection::SendBlock(const void *pBuffer, DWORD dwBlockSize, BOOL bCautiousSend, BYTE btPacketPriority) const
{ 
    ASSERT_VALID(this);
    return m_pNetworkConnection->SendBlock(m_wVirtualConnection, pBuffer, dwBlockSize, bCautiousSend, btPacketPriority); 
}

//////////////////////////////////////////////////////////////////////////
inline BOOL CVirtualConnection::ReceiveBlock(void *pBuffer, DWORD &dwBlockSize) const
{ 
    ASSERT_VALID(this);
    return m_pNetworkConnection->ReceiveBlock(m_wVirtualConnection, pBuffer, dwBlockSize); 
}

//////////////////////////////////////////////////////////////////////////
inline BOOL CVirtualConnection::IsNetworkReady() const
{ 
    ASSERT_VALID(this);
    return m_pNetworkConnection->IsNetworkReady();
}

//////////////////////////////////////////////////////////////////////////
inline BOOL CVirtualConnection::BeginSendCompoundBlock(BOOL bCautiousSend, BYTE btPacketPriority) const
{ 
    ASSERT_VALID(this);
    return m_pNetworkConnection->BeginSendCompoundBlock(m_wVirtualConnection, bCautiousSend, btPacketPriority);
}

//////////////////////////////////////////////////////////////////////////
inline BOOL CVirtualConnection::EndSendCompoundBlock() const
{ 
    ASSERT_VALID(this);
    return m_pNetworkConnection->EndSendCompoundBlock(m_wVirtualConnection); 
}

//////////////////////////////////////////////////////////////////////////
inline BOOL CVirtualConnection::BeginReceiveCompoundBlock() const
{ 
    ASSERT_VALID(this);
    return m_pNetworkConnection->BeginReceiveCompoundBlock(m_wVirtualConnection); 
}

//////////////////////////////////////////////////////////////////////////
inline BOOL CVirtualConnection::EndReceiveCompoundBlock() const
{ 
    ASSERT_VALID(this);
    return m_pNetworkConnection->EndReceiveCompoundBlock(m_wVirtualConnection);
}

//////////////////////////////////////////////////////////////////////////
inline BYTE CVirtualConnection::GetVirtualConnectionPriority() const
{ 
    ASSERT_VALID(this);
    return m_pNetworkConnection->GetVirtualConnectionPriority(m_wVirtualConnection);
}

//////////////////////////////////////////////////////////////////////////
inline BOOL CVirtualConnection::SetVirtualConnectionPriority(BYTE btBasePriority)
{ 
    ASSERT_VALID(this);
    return m_pNetworkConnection->SetVirtualConnectionPriority(m_wVirtualConnection, btBasePriority);
}

//////////////////////////////////////////////////////////////////////////
inline DWORD CVirtualConnection::GetMaximumMessageSize() const
{ 
    // CVirtualConnection doesn't need to be valid
    // just call CNetworkConnection member function
    return m_pNetworkConnection->GetMaximumMessageSize(); 
}

//////////////////////////////////////////////////////////////////////////
inline DWORD CVirtualConnection::GetActualDataSize() const
{
    ASSERT_VALID(this);
    return m_pNetworkConnection->GetActualDataSize(m_wVirtualConnection);
}


#endif  // !defined(_VIRTUALCONNECTION__INLINE_INCLUDED_)
