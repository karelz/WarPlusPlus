
#include "stdafx.h"
#include "PacketTypes.h"
#include "NetworkEvents.h"
#include "VirtualConnection.h"
#include "NetworkConnection.h"
#include "VirtualConnection.inl"

IMPLEMENT_DYNAMIC(CVirtualConnection, CObject)

//////////////////////////////////////////////////////////////////////
// Debug methods
//////////////////////////////////////////////////////////////////////

#ifdef _DEBUG

void CVirtualConnection::AssertValid() const
{
    CObject::AssertValid();
    ASSERT_KINDOF(CNetworkConnection, m_pNetworkConnection);
    ASSERT(m_wVirtualConnection != 0);
}

void CVirtualConnection::Dump(CDumpContext &dc) const
{
    CObject::Dump(dc);
    if (m_wVirtualConnection == 0)
    {
        dc << _T("Uninitialized CVirtualConnection object") << endl;
    }
    else
    {
        dc << _T("CVirtualConnection ") << m_wVirtualConnection << endl;
    }
}

#endif  // _DEBUG
