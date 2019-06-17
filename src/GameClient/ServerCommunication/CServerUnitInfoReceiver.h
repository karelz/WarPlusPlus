// CServerUnitInfoReceiver.h: interface for the CCServerUnitInfoReceiver class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CSERVERUNITINFORECEIVER_H__3B651F74_105E_11D4_8040_0000B4A08F9A__INCLUDED_)
#define AFX_CSERVERUNITINFORECEIVER_H__3B651F74_105E_11D4_8040_0000B4A08F9A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Network/Network/NetworkConnection.h"
#include "Network/Network/VirtualConnection.h"
#include "../DataObjects/CUnit.h"
#include "CRequestVirtualConnection.h"
#include "CTimestampedVirtualConnection.h"
#include "CMiniMapClip.h"
#include "../DataObjects/UnitCache.h"
#include "../DataObjects/UnitTypeCache.h"
#include "../DataObjects/CMap.h"
#include "../MapView/ResourcesBar.h"

class CCWatchedRectangle;

// Tato trida prijima informace od serveru ohledne jednotek a UL mapexu
class CCServerUnitInfoReceiver  
{
// Konstrukce a destrukce
public:
	// Konstruktor
	CCServerUnitInfoReceiver();

	// Destruktor
	virtual ~CCServerUnitInfoReceiver();

// Metody
public:
	// Vytvoreni pripojeni na klienta
	void Create(CNetworkConnection *pConnection, CCMap *pMap, CResourcesBar *pResourcesBar);

	// Zruseni pripojeni na klienta
	void Delete();

	// Pridani obdelniku na sledovani
	void AddWatchedRectangle(CCWatchedRectangle *pRect);

	// Zruseni obdelniku na sledovani
	void RemoveWatchedRectangle(CCWatchedRectangle *pRect);

	// Projeveni zajmu o full info jednotky
	void StartReceivingFullInfo(CCUnit *pUnit);

	// Konec zajmu o full info jednotky
	void StopReceivingFullInfo(CCUnit *pUnit);

	// Vrati request spojeni
	CCRequestVirtualConnection *GetRequestConnection() { return &m_RequestConnection; }

	// Vrati mapu
	CCMap *GetMap() { return m_pMap; }

	// Vrati unit cache
	CUnitCache *GetUnitCache() { return m_pUnitCache; }

	// Vrati vyrez minimapy
	CCMiniMapClip *GetMiniMapClip() { return &m_MiniMapClip; }

	// Tuto funkci volaji TimestampedVirtualConnections, kdyz jim prijde paket
	void NotifyPacketArrived(DWORD dwTimeStamp);

	// Tato funkce se vola jednou pro kazdou inkrementaci casoveho udaje posilaneho
	// ze serveru. Dostava jako parametr pocet milisekund, jak dlouho dany tik serveru
	// trval, a cislo toho tiku. Informaci o tiku N dostanete az po prijeti zpravy o
	// tiku N+1 (pochopitelne)
	void TimeIncrement(DWORD dwTimeInterval, DWORD dwServerTime);

// Pripojeni na klienta
private:

	// Kanal pro uzivatelske requesty
	CCRequestVirtualConnection m_RequestConnection; 

	// Jeden *bezpecny* kanal pro dulezite informace
	CCTimestampedVirtualConnection m_ControlConnection;

	// Kanal pro brief infa
	CCTimestampedVirtualConnection m_BriefInfoConnection;

	// Kanal pro checkpoint infa
	CCTimestampedVirtualConnection m_CheckPointInfoConnection;

	// Kanal pro full infa
	CCTimestampedVirtualConnection m_FullInfoConnection;

	// Kanal pro enemy full infa
	CCTimestampedVirtualConnection m_EnemyFullInfoConnection;

	// Kanal pro pingovani klienta s aktualnim casem
	CCTimestampedVirtualConnection m_PingConnection;

	// Kanal pro zkomprimovanou minimapu
	CVirtualConnection m_MiniMapConnection;

	// Resource connection
	class CResourceObserver : public CObserver
	{
		DECLARE_OBSERVER_MAP(CResourceObserver);
		public:
		CResourceObserver(){};
		~CResourceObserver(){};

		void OnPacketAvailable() { m_pParent->OnResourcePacketAvailable(); }

		CCServerUnitInfoReceiver *m_pParent;
	};
	
	friend CResourceObserver;
	CResourceObserver m_ResourceObserver;
	CVirtualConnection m_ResourceConnection;
	BOOL m_bResourceConnectionCreated;
	void OnResourcePacketAvailable();
	enum{ ID_ResourceConnection = 0x0100, };

	// The resources bar to which we'll send informations
	CResourcesBar *m_pResourcesBar;

// Soukroma data
private:
	// Seznam sledovanych obdelniku
	CCWatchedRectangle *m_pRect;

	// Unit cache
	CUnitCache *m_pUnitCache;

	// Mapa
	CCMap *m_pMap;

	// Vyrez minimapy
	CCMiniMapClip m_MiniMapClip;

	// Aktualni severovy cas (v tikach)
	DWORD m_dwServerTime;

	// Cas prichodu posledniho paketu (v milisekundach od startu systemu)
	DWORD m_dwTime;
};

#endif // !defined(AFX_CSERVERUNITINFORECEIVER_H__3B651F74_105E_11D4_8040_0000B4A08F9A__INCLUDED_)
