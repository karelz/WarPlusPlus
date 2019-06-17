/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Civilizace na serveru hry
 *   Autor: Roman Zenka
 * 
 *   Popis: Klient civilizace na serveru hry
 * 
 ***********************************************************/

#ifndef __CIVILIZATION_CLIENT_UNIT_INFO_SENDER__HEADER_INCLUDED__
#define __CIVILIZATION_CLIENT_UNIT_INFO_SENDER__HEADER_INCLUDED__

#include "Common/Map/MapDataTypes.h"
#include "Common/ServerClient/UnitInfoStructures.h"
#include "Common/ServerClient/CommunicationInfoStructures.h"
#include "Network/Network/VirtualConnection.h"
#include "ZTimestampedVirtualConnection.h"
#include "ZRequestVirtualConnection.h"
#include "ZWatchedRectangle.h"
#include "../MiniMap/ZMiniMapRequest.h"
#include "../MiniMap/MiniMapServer.h"

class CZCivilization;

//////////////////////////////////////////////////////////////////////
// Tøída klienta civilizace na serveru hry.
class CZClientUnitInfoSender 
{
// Konstanty
public:
	enum EZClientUnitInfoSenderConstants {
		// Pocet vsech spojeni, ktere trida potrebuje
		TotalConnections=9
	};

// Konstrukce & destukce
public:
	// Konstruktor
	CZClientUnitInfoSender();
	
	// Konstruktor
	CZClientUnitInfoSender(CZCivilization *pZCivilization);

	// Destruktor
	~CZClientUnitInfoSender();

	// Vytvoreni
	void Create(CZCivilization *pZCivilization);

	// Znièení
	virtual void Delete();

// Sitove spojeni
public:
	// Prida nove vznikle sitove spojeni
	void AddConnection(CVirtualConnection Connection, DWORD dwVirtualConnectionID, CZCivilization *pCivilization);

	// Prisla zadost od uzivatele!
	void RequestArrived(DWORD dwType, DWORD dwSize, void *pData);

// Metody pro komunikaci se serverem
public:
	// vrátí index civilizace
	DWORD GetCivilizationIndex();

	// Prijme brief info od serveru
	void BriefInfo(const SUnitBriefInfo *pBriefInfo);

	// Zacatek odesilani full infa od serveru
	void StartSendingFullInfo(const SUnitFullInfo *pFullInfo);
	// Pokracovani odesilani full infa od serveru
	void SendingFullInfo(const BYTE *pData, int nSize);
	// Konec odesilani full infa od serveru
	void StopSendingFullInfo();

	// Zacatek odesilani enemy full infa od serveru
	void StartSendingEnemyFullInfo(const SUnitEnemyFullInfo *pEnemyFullInfo);
	// Pokracovani odesilani enemy full infa od serveru
	void SendingEnemyFullInfo(const BYTE *pData, int nSize);
	// Konec odesilani enemy full infa od serveru
	void StopSendingEnemyFullInfo();

	// Zacatek odesilani start full infa od serveru
	void StartSendingStartFullInfo(const SUnitStartFullInfo *pStartFullInfo);
	// Pokracovani odesilani start full infa od serveru
	void SendingStartFullInfo(const BYTE *pData, int nSize);
	// Konec odesilani start full infa od serveru
	void StopSendingStartFullInfo();

	// Zacatek odesilani start enemy full infa od serveru
	void StartSendingStartEnemyFullInfo(const SUnitStartEnemyFullInfo *pStartEnemyFullInfo);
	// Pokracovani odesilani start enemy full infa od serveru
	void SendingStartEnemyFullInfo(const BYTE *pData, int nSize);
	// Konec odesilani start enemy full infa od serveru
	void StopSendingStartEnemyFullInfo();

	// Prijme checkpoint info od serveru
	// Prijme stop full info od serveru (sledovana jednotka se dostala mimo viditelnost 
	//		civilizace)
	void StopFullInfo(DWORD dwID);
	// Prijme stop enemy full info od serveru (sledovana jednotka se dostala mimo 
	//		viditelnost civilizace)
	void StopEnemyFullInfo(DWORD dwID);

	// Prijme checkpoint info od serveru
	void CheckPointInfo(const SUnitCheckPointInfo *pCheckPointInfo);

	// Prijme start info od serveru (jednotka je pro klienta zajimava)
	void StartInfo(const SUnitStartInfo *pStartInfo);
	// Prijme stop info od serveru (jednotka pro klienta uz neni zajimava)
	void StopInfo(DWORD dwID);

	// Zacatek odesilani informaci o jednotkach
	void StartSendingInfo();
	// Konec odesilani informaci o jednotkach
	void StopSendingInfo();

	// Prijme informace o surovinach civilizace
	void SendResourceInfo ( int (* pResources)[RESOURCE_COUNT] );

// Metody pro zpracovani minimapy
public:
	// Ziska request od uzivatele na minimapu
	CZMiniMapRequest *GetMiniMapRequest() { return &m_MiniMapRequest; }

	// Zacne renderovat jednotky
	void StartRenderingUnits() { m_MiniMapServer.StartRenderingUnits(&m_MiniMapRequest); }

	// Skonci s renderovanim jednotek
	void StopRenderingUnits() { m_MiniMapServer.StopRenderingUnits(); }

	// Vyrenderuje jednotku
	void RenderUnit(DWORD dwX, DWORD dwY, int nCivilization, int nSize=1) {
		m_MiniMapServer.RenderUnit(dwX, dwY, nCivilization, nSize);
	}

// Staticke metody pro praci s CMiniMapServerem
public:
	// Inicializace MiniMapServeru
	static void InitMiniMapServer() { 
        m_MiniMapServer.Create(); 
    }

	// Ukonceni minimap serveru
	static void DoneMiniMapServer() { m_MiniMapServer.Delete(); }

// Staticka data
private:
	// Server na zpracovavani minimapy
	static CMiniMapServer m_MiniMapServer;

// Data
private:	
	// Ukazatel na civilizaci
	CZCivilization *m_pZCivilization;

	// Priznak, ze se konci prace
	BOOL m_bFinishing;

	// Pozadavek klienta na vyrez minimapy
	CZMiniMapRequest m_MiniMapRequest;

// Ukazatele na jednotlive connections
private:
	// Kanal pro uzivatelske requesty
	CZRequestVirtualConnection m_RequestConnection; 

	// Jeden *bezpecny* kanal pro dulezite informace
	CZTimestampedVirtualConnection m_ControlConnection;

	// Kanal pro brief infa
	CZTimestampedVirtualConnection m_BriefInfoConnection;

	// Kanal pro checkpoint infa
	CZTimestampedVirtualConnection m_CheckPointInfoConnection;

	// Kanal pro full infa
	CZTimestampedVirtualConnection m_FullInfoConnection;

	// Kanal pro enemy full infa
	CZTimestampedVirtualConnection m_EnemyFullInfoConnection;

	// Kanal pro pingovani
	CZTimestampedVirtualConnection m_PingConnection;

	// Connection for sending resource counts
	CVirtualConnection m_ResourceConnection;

	// Spojeni pro posilani spakovanych minimap
	CVirtualConnection m_MiniMapConnection;

	// Pocet zatim otevrenych spojeni
	DWORD m_dwNumConnections;

    // Priznak, ze jsme se zaregistrovali
    BOOL m_bRegistered;

// Pomocna data
private:
	// Meric casu - inkrementuje se o 1 pri kazdem StartSendingInfo()
	DWORD m_dwTime;

	// List sledovanych obdelniku
	CZWatchedRectangle *m_pRectList;

// Debug
#ifdef _DEBUG
	// Mod, do ktereho jsme se prepnuli pomoci StartSending<?>
	// 0 - nic
	// 1 - FullInfo
	// 2 - EnemyFullInfo
	// 3 - StartFullInfo
	// 4 - StartEnemyFullInfo
	int m_nFullInfoMode;
#endif // _DEBUG
};

#endif //__CIVILIZATION_CIVILIZATION_CLIENT__HEADER_INCLUDED__
