// ZRequestVirtualConnection.cpp: implementation of the CZRequestVirtualConnection class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ZRequestVirtualConnection.h"
#include "Common/ServerClient/VirtualConnectionIDs.h"
#include "Common/ServerClient/UnitInfoStructures.h"
#include "ZClientUnitInfoSender.h"

BEGIN_OBSERVER_MAP(CZRequestVirtualConnection, CObserver)
    BEGIN_NOTIFIER(VirtualConnection_ClientRequests)
        EVENT(E_PACKETAVAILABLE)
            OnPacketAvailable((WORD) dwParam);
            return TRUE;
    END_NOTIFIER()
END_OBSERVER_MAP(CZRequestVirtualConnection, CObserver)

//////////////////////////////////////////////////////////////////////

CZRequestVirtualConnection::CZRequestVirtualConnection()
{
    Create();
}

CZRequestVirtualConnection::CZRequestVirtualConnection(CVirtualConnection VirtualConnection)
{
	Create();
    SetConnection(VirtualConnection);
}

CZRequestVirtualConnection::~CZRequestVirtualConnection()
{
}

void CZRequestVirtualConnection::Create()
{
   	m_pSender=NULL;
}

void CZRequestVirtualConnection::Delete()
{
    m_VirtualConnection.DeleteVirtualConnection();
}

void CZRequestVirtualConnection::SetConnection(CVirtualConnection VirtualConnection)
{
	m_VirtualConnection=VirtualConnection;
	m_VirtualConnection.SetVirtualConnectionObserver(this, VirtualConnection_ClientRequests);
}

void CZRequestVirtualConnection::SetClientUnitInfoSender(CZClientUnitInfoSender *pSender)
{
	m_pSender=pSender;
}

void CZRequestVirtualConnection::OnPacketAvailable(WORD wConnectionNumber)
{
    // ASSERT(wConnectionNumber == VirtualConnection_ClientRequests);

	// Zpracovani zpravy
	// Zprava ma format:
    SUnitRequestHeaderFormat reqHeader;
	BYTE buffer[ReceiveBufSize];
    DWORD size;

    // Zacneme prenos
	m_VirtualConnection.BeginReceiveCompoundBlock();

	// Jedeme, dokud se nevyskoci uprostred
	while(TRUE) {
		size=sizeof(reqHeader);
		m_VirtualConnection.ReceiveBlock((void *)&reqHeader, size);
		if(size==0) {
			// Uz neni zadna hlavicka
			break;
		}
		
		// Alespon hlavicka MUSELA prijit
		ASSERT(size==sizeof(reqHeader));

		// Z hlavicky zjistime velikost a nacteme data do bufferu
		ASSERT(reqHeader.dwRequestDataLength<=ReceiveBufSize);
		size=reqHeader.dwRequestDataLength;
		m_VirtualConnection.ReceiveBlock(&buffer, size);

		// Ty data by tam mely byt
		ASSERT(size==reqHeader.dwRequestDataLength);
		
		if(m_pSender) {
			m_pSender->RequestArrived(reqHeader.dwRequestType, size, &buffer);
		}
	}

	// Skoncime prijem
	m_VirtualConnection.EndReceiveCompoundBlock();
}
