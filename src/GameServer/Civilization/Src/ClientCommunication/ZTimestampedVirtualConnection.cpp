// ZTimestampedVirtualConnection.cpp: implementation of the CZTimestampedVirtualConnection class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ZTimestampedVirtualConnection.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CZTimestampedVirtualConnection::CZTimestampedVirtualConnection()
{
    Create();
}

CZTimestampedVirtualConnection::~CZTimestampedVirtualConnection()
{
    ASSERT(!m_bSending);
    ASSERT(m_dwState==0);
}

void CZTimestampedVirtualConnection::Create() 
{
#ifdef _DEBUG
    m_dwState=0;
#endif // _DEBUG
	m_dwActualTime=0;
	m_bCompoundWanted=FALSE;
	m_bSending=FALSE;
    m_bFirst=TRUE;
}

void CZTimestampedVirtualConnection::Delete()
{
    ASSERT(!m_bSending);
    ASSERT(m_dwState==0);
    m_VirtualConnection.DeleteVirtualConnection();
}


void CZTimestampedVirtualConnection::SetTime(DWORD dwTime)
{
	ASSERT(m_dwActualTime<=dwTime);
    m_dwActualTime=dwTime;
}
