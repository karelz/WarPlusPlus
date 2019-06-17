// CRequestVirtualConnection.cpp: implementation of the CCRequestVirtualConnection class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CRequestVirtualConnection.h"
#include "Common/ServerClient/CommunicationInfoStructures.h"
#include "CWatchedRectangle.h"
#include "../DataObjects/CUnit.h"
#include "../GameClientNetwork.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

BEGIN_OBSERVER_MAP(CCRequestVirtualConnection, CObserver)
    BEGIN_NOTIFIER(ID_RequestVirtualConnection)
        EVENT(E_PACKETAVAILABLE)
			ASSERT(FALSE);
            return TRUE;
    END_NOTIFIER()
END_OBSERVER_MAP(CCRequestVirtualConnection, CObserver)

void CCRequestVirtualConnection::Create(CVirtualConnection Connection)
{ 
	m_Connection=Connection;
	m_bVirtualConnectionCreated = TRUE;
}

void CCRequestVirtualConnection::Delete()
{
	if(m_bVirtualConnectionCreated){
		m_Connection.DeleteVirtualConnection();
	}
}

CCRequestVirtualConnection::CCRequestVirtualConnection()
{
	m_bVirtualConnectionCreated = FALSE;
}

CCRequestVirtualConnection::CCRequestVirtualConnection(CVirtualConnection Connection)
{
	Create(Connection);
}

CCRequestVirtualConnection::~CCRequestVirtualConnection()
{
}

void CCRequestVirtualConnection::SendStartWatchingRectangle(CCWatchedRectangle *pRect)
{
	SUnitRequestHeaderFormat hdr;
	SUserRequestStartWatchingRectangle swr;

    VERIFY(m_lockConnection.Lock());

	hdr.dwRequestType=UR_StartWatchingRectangle;
	hdr.dwRequestDataLength=sizeof(SUserRequestStartWatchingRectangle);

	swr.dwID=pRect->ID();
	swr.dwLeft=pRect->LeftMapSquare();
	swr.dwTop=pRect->TopMapSquare();
	swr.dwWidth=pRect->WidthMapSquare();
	swr.dwHeight=pRect->HeightMapSquare();

	TRACE("## SendStartWatchingRectangle: ID=%d, X=%d, Y=%d, W=%d, H=%d\n", swr.dwID, swr.dwLeft, swr.dwTop, swr.dwWidth, swr.dwHeight);

	VERIFY(m_Connection.BeginSendCompoundBlock());
	VERIFY(m_Connection.SendBlock(&hdr, sizeof(hdr)));
	VERIFY(m_Connection.SendBlock(&swr, sizeof(swr)));
	VERIFY(m_Connection.EndSendCompoundBlock());

    VERIFY(m_lockConnection.Unlock());
}

void CCRequestVirtualConnection::SendStopWatchingRectangle(CCWatchedRectangle *pRect)
{
	SUnitRequestHeaderFormat hdr;
	SUserRequestStopWatchingRectangle swr;
	
    VERIFY(m_lockConnection.Lock());

	hdr.dwRequestType=UR_StopWatchingRectangle;
	hdr.dwRequestDataLength=sizeof(SUserRequestStopWatchingRectangle);

	swr.dwID=pRect->ID();

	TRACE("## SendStopWatchingRectangle: ID=%d\n", swr.dwID);

	VERIFY(m_Connection.BeginSendCompoundBlock());
	VERIFY(m_Connection.SendBlock(&hdr, sizeof(hdr)));
	VERIFY(m_Connection.SendBlock(&swr, sizeof(swr)));
	VERIFY(m_Connection.EndSendCompoundBlock());

    VERIFY(m_lockConnection.Unlock());
}

void CCRequestVirtualConnection::SendRectangleMoved(CCWatchedRectangle *pRect)
{
	SUnitRequestHeaderFormat hdr;
	SUserRequestRectangleMoved srm;
	
    VERIFY(m_lockConnection.Lock());

	hdr.dwRequestType=UR_RectangleMoved;
	hdr.dwRequestDataLength=sizeof(SUserRequestRectangleMoved);

	srm.dwID=pRect->ID();
	srm.dwLeft=pRect->LeftMapSquare();
	srm.dwTop=pRect->TopMapSquare();
	srm.dwWidth=pRect->WidthMapSquare();
	srm.dwHeight=pRect->HeightMapSquare();

	TRACE("## SendRectangleMoved: ID=%d, X=%d, Y=%d, W=%d, H=%d\n", srm.dwID, srm.dwLeft, srm.dwTop, srm.dwWidth, srm.dwHeight);

	VERIFY(m_Connection.BeginSendCompoundBlock());
	VERIFY(m_Connection.SendBlock(&hdr, sizeof(hdr)));
	VERIFY(m_Connection.SendBlock(&srm, sizeof(srm)));
	VERIFY(m_Connection.EndSendCompoundBlock());

    VERIFY(m_lockConnection.Unlock());
}

void CCRequestVirtualConnection::SendStartFullInfo(CCUnit *pUnit)
{
	SUnitRequestHeaderFormat hdr;
	SUserRequestStartFullInfo sfi;

    VERIFY(m_lockConnection.Lock());

	hdr.dwRequestType=UR_StartFullInfo;
	hdr.dwRequestDataLength=sizeof(SUserRequestStartFullInfo);

	sfi.dwID=pUnit->GetID();

	TRACE("## SendStartFullInfo: ID=%d\n", sfi.dwID);

	VERIFY(m_Connection.BeginSendCompoundBlock());
	VERIFY(m_Connection.SendBlock(&hdr, sizeof(hdr)));
	VERIFY(m_Connection.SendBlock(&sfi, sizeof(sfi)));
	VERIFY(m_Connection.EndSendCompoundBlock());

    VERIFY(m_lockConnection.Unlock());
}

void CCRequestVirtualConnection::SendStopFullInfo(CCUnit *pUnit)
{
	SUnitRequestHeaderFormat hdr;
	SUserRequestStopFullInfo sfi;

    VERIFY(m_lockConnection.Lock());

	hdr.dwRequestType=UR_StopFullInfo;
	hdr.dwRequestDataLength=sizeof(SUserRequestStopFullInfo);

	sfi.dwID=pUnit->GetID();

	TRACE("## SendStopFullInfo: ID=%d\n", sfi.dwID);

	VERIFY(m_Connection.BeginSendCompoundBlock());
	VERIFY(m_Connection.SendBlock(&hdr, sizeof(hdr)));
	VERIFY(m_Connection.SendBlock(&sfi, sizeof(sfi)));
	VERIFY(m_Connection.EndSendCompoundBlock());

    VERIFY(m_lockConnection.Unlock());
}

void CCRequestVirtualConnection::SendCloseComms()
{
	SUnitRequestHeaderFormat hdr;
	
    VERIFY(m_lockConnection.Lock());

	hdr.dwRequestType=UR_CloseComms;
	hdr.dwRequestDataLength=0;

	TRACE("## SendCloseComms\n");

	VERIFY(m_Connection.SendBlock(&hdr, sizeof(hdr)));
    
    VERIFY(m_lockConnection.Unlock());
}

void CCRequestVirtualConnection::SendMiniMapClip(SMiniMapClip Clip)
{
	SUnitRequestHeaderFormat hdr;

    VERIFY(m_lockConnection.Lock());

	hdr.dwRequestType=UR_MiniMap;
	hdr.dwRequestDataLength=sizeof(Clip);

	TRACE("## SendMiniMapClip\n");

	VERIFY(m_Connection.BeginSendCompoundBlock());
	VERIFY(m_Connection.SendBlock(&hdr, sizeof(hdr)));
	VERIFY(m_Connection.SendBlock(&Clip, sizeof(Clip)));
	VERIFY(m_Connection.EndSendCompoundBlock());

    VERIFY(m_lockConnection.Unlock());
}
