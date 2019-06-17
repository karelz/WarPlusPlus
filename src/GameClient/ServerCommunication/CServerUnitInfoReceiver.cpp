// CServerUnitInfoReceiver.cpp: implementation of the CCServerUnitInfoReceiver class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CServerUnitInfoReceiver.h"
#include "CWatchedRectangle.h"
#include "Common\ServerClient\VirtualConnectionIDs.h"
#include "..\GameClientNetwork.h"
#include "..\Windows\Timer\Timer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCServerUnitInfoReceiver::CCServerUnitInfoReceiver()
{
	m_pMap=NULL;
	m_pRect=NULL;
	m_pUnitCache = NULL;

  m_ResourceObserver.m_pParent = this;
  m_bResourceConnectionCreated = FALSE;
}

CCServerUnitInfoReceiver::~CCServerUnitInfoReceiver()
{
	ASSERT(m_pMap == NULL);
	ASSERT(m_pRect == NULL);
	ASSERT(m_pUnitCache == NULL);
}

void CCServerUnitInfoReceiver::Create(CNetworkConnection *pConnection, CCMap *pMap, CResourcesBar *pResourcesBar)
{
	m_pMap=pMap;
	m_dwServerTime=0;
	m_dwTime=0;
  m_pResourcesBar = pResourcesBar;

	m_pUnitCache=new CUnitCache();
	m_pUnitCache->Create();

// Spolehliva spojeni
	// Pozadavky
	DWORD dwUserData = VirtualConnection_ClientRequests;
	m_RequestConnection.Create(pConnection->CreateVirtualConnectionEx(PACKET_TYPE1, (CObserver *)&m_RequestConnection, CCRequestVirtualConnection::ID_RequestVirtualConnection,
    g_dwNetworkEventThreadID, &dwUserData, sizeof(dwUserData)));

	// Ridici spojeni
	dwUserData = VirtualConnection_ClientControl;
	m_ControlConnection.Create(pConnection->CreateVirtualConnectionEx(PACKET_TYPE1, (CObserver *)&m_ControlConnection, CCTimestampedVirtualConnection::ID_TimestampedVirtualConnection,
    g_dwNetworkEventThreadID, &dwUserData, sizeof(dwUserData)), VirtualConnection_ClientControl, this);

// Nespolehliva spojeni	
	// Brief infa
	dwUserData = VirtualConnection_ClientBriefInfos;
	m_BriefInfoConnection.Create(pConnection->CreateVirtualConnectionEx(PACKET_TYPE4, (CObserver *)&m_BriefInfoConnection, CCTimestampedVirtualConnection::ID_TimestampedVirtualConnection, 
    g_dwNetworkEventThreadID, &dwUserData, sizeof(dwUserData)), VirtualConnection_ClientBriefInfos, this);

	// Checkpoint infa
	dwUserData = VirtualConnection_ClientCheckPointInfos;
	m_CheckPointInfoConnection.Create(pConnection->CreateVirtualConnectionEx(PACKET_TYPE4, (CObserver *)&m_CheckPointInfoConnection, CCTimestampedVirtualConnection::ID_TimestampedVirtualConnection, 
    g_dwNetworkEventThreadID, &dwUserData, sizeof(dwUserData)), VirtualConnection_ClientCheckPointInfos, this);

	// Full infa
	dwUserData = VirtualConnection_ClientFullInfos;
	m_FullInfoConnection.Create(pConnection->CreateVirtualConnectionEx(PACKET_TYPE4, (CObserver *)&m_FullInfoConnection, CCTimestampedVirtualConnection::ID_TimestampedVirtualConnection, 
    g_dwNetworkEventThreadID, &dwUserData, sizeof(dwUserData)), VirtualConnection_ClientFullInfos, this);

	// Enemy full infa
	dwUserData = VirtualConnection_ClientEnemyFullInfos;
	m_EnemyFullInfoConnection.Create(pConnection->CreateVirtualConnectionEx(PACKET_TYPE4, (CObserver *)&m_EnemyFullInfoConnection, CCTimestampedVirtualConnection::ID_TimestampedVirtualConnection, 
    g_dwNetworkEventThreadID, &dwUserData, sizeof(dwUserData)), VirtualConnection_ClientEnemyFullInfos, this);

	// Ping
	dwUserData = VirtualConnection_ClientPing;
	m_PingConnection.Create(pConnection->CreateVirtualConnectionEx(PACKET_TYPE4, (CObserver *)&m_PingConnection, CCTimestampedVirtualConnection::ID_TimestampedVirtualConnection, 
    g_dwNetworkEventThreadID, &dwUserData, sizeof(dwUserData)), VirtualConnection_ClientPing, this);

	// Resources
	dwUserData = VirtualConnection_Resources;
	m_ResourceConnection = pConnection->CreateVirtualConnectionEx(PACKET_TYPE1, (CObserver *)&m_ResourceObserver, ID_ResourceConnection,
	g_dwNetworkEventThreadID, &dwUserData, sizeof(dwUserData));
	m_bResourceConnectionCreated = TRUE;

	// Minimapa
	dwUserData = VirtualConnection_MiniMap;
	m_MiniMapConnection = pConnection->CreateVirtualConnectionEx(PACKET_TYPE1, (CObserver *)&m_MiniMapClip, CCMiniMapClip::ID_MiniMapVirtualConnection,
	g_dwNetworkEventThreadID, &dwUserData, sizeof(dwUserData));
	m_MiniMapClip.Create(&m_MiniMapConnection, &m_RequestConnection);
}

void CCServerUnitInfoReceiver::Delete()
{
	m_pMap = NULL;

	if(m_bResourceConnectionCreated){
		m_ResourceConnection.DeleteVirtualConnection();
		m_bResourceConnectionCreated = FALSE;
	}

	// Nejprve zrusime vyrez, ktery zapomene sitova spojeni
	m_MiniMapClip.Delete();
	
	m_PingConnection.Delete();
	m_EnemyFullInfoConnection.Delete();
	m_FullInfoConnection.Delete();
	m_CheckPointInfoConnection.Delete();
	m_BriefInfoConnection.Delete();
	m_ControlConnection.Delete();
	m_RequestConnection.Delete();
	m_MiniMapConnection.DeleteVirtualConnection();
	
	if(m_pUnitCache) {
		m_pUnitCache->Delete();
    delete m_pUnitCache;
		m_pUnitCache=NULL;
	}

	ASSERT(m_pRect == NULL);
}

void CCServerUnitInfoReceiver::AddWatchedRectangle(CCWatchedRectangle *pRect)
{
	if(m_pRect) {
		m_pRect->Append(pRect);
	} else {
		m_pRect=pRect;
	}
	
	m_RequestConnection.SendStartWatchingRectangle(pRect);
	pRect->Watched();
}

void CCServerUnitInfoReceiver::RemoveWatchedRectangle(CCWatchedRectangle *pRect)
{
	pRect->Watched(FALSE);
	if(pRect!=m_pRect) {
		pRect->Disconnect();
	} else {
    if(m_pRect->Next() == m_pRect)
      // Nasledujici obdelnik jsem ja sam - jsem tedy posledni ve spojaku
      m_pRect = NULL;
    else
		  m_pRect = m_pRect->Next();
		pRect->Disconnect();
	}

	m_RequestConnection.SendStopWatchingRectangle(pRect);
}

void CCServerUnitInfoReceiver::StartReceivingFullInfo(CCUnit *pUnit)
{
	TRACE("StartReceivingFullInfo: ID=%d\n", pUnit->GetID());
	if(!pUnit->GetFullInfoFlag()) {
		pUnit->SetFullInfoFlag(TRUE);
		m_RequestConnection.SendStartFullInfo(pUnit);
	}
}

void CCServerUnitInfoReceiver::StopReceivingFullInfo(CCUnit *pUnit)
{
	TRACE("StopReceivingFullInfo: ID=%d\n", pUnit->GetID());
	if(pUnit->GetFullInfoFlag()) {
		pUnit->SetFullInfoFlag(FALSE);
		m_RequestConnection.SendStopFullInfo(pUnit);
	}
}

void CCServerUnitInfoReceiver::NotifyPacketArrived(DWORD dwTimeStamp) {
	DWORD dwCurrentTime, dwInterval, i;
	
	/* Prave prisla prvni notifikace */
	if(m_dwServerTime==0 && m_dwTime==0) 
	{
		m_dwTime=g_pTimer->GetTime();
		m_dwServerTime=dwTimeStamp;
		return;
	}

	if(dwTimeStamp>m_dwServerTime) {
		dwCurrentTime=g_pTimer->GetTime();
		if(dwCurrentTime>m_dwTime) {
			dwInterval=dwCurrentTime-m_dwTime;
		} else {
			/* Doslo k preteceni casovace */
			dwInterval=m_dwTime-dwCurrentTime;
		}
		/* Jeden casovy usek */
		dwInterval/=dwTimeStamp-m_dwServerTime;
		
		for(i=1; i<=dwTimeStamp-m_dwServerTime; i++) {
			TimeIncrement(dwInterval, m_dwServerTime+i-1);
		}

		m_dwTime=dwCurrentTime;
		m_dwServerTime=dwTimeStamp;
	}
}

void CCServerUnitInfoReceiver::TimeIncrement(DWORD dwTimeInterval, DWORD dwServerTime) {
  m_pMap->NewTimesliceArrivedFromServer(dwServerTime, dwTimeInterval);
}

BEGIN_OBSERVER_MAP(CCServerUnitInfoReceiver::CResourceObserver, CObserver)
  BEGIN_NOTIFIER(ID_ResourceConnection)
    EVENT(E_PACKETAVAILABLE)
      OnPacketAvailable(); return FALSE;
  END_NOTIFIER()
END_OBSERVER_MAP(CCServerUnitInfoReceiver::CResourceObserver, CObserver)

void CCServerUnitInfoReceiver::OnResourcePacketAvailable()
{
  VERIFY(m_ResourceConnection.BeginReceiveCompoundBlock());

  int aResources[RESOURCE_COUNT];
  DWORD dwSize = sizeof(int) * RESOURCE_COUNT;
  VERIFY(m_ResourceConnection.ReceiveBlock(aResources, dwSize));
  ASSERT(dwSize == sizeof(int) * RESOURCE_COUNT);
  DWORD dwIndex;
  for(dwIndex = 0; dwIndex < RESOURCE_COUNT; dwIndex++){
    m_pResourcesBar->SetResource(dwIndex, aResources[dwIndex]);
  }

  VERIFY(m_ResourceConnection.EndReceiveCompoundBlock());
}