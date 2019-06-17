/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: Notifikace serveru do skriptù
 * 
 ***********************************************************/

#ifndef __SERVER_NOTIFICATIONS__HEADER_INCLUDED__
#define __SERVER_NOTIFICATIONS__HEADER_INCLUDED__

#include "SUnit.h"
#include "GameServer\Civilization\Src\ZUnit.h"

#ifdef _DEBUG
#include "SCivilization.h"
#include "SSkillType.h"
#include "GameServer\Civilization\ZCivilization.h"
#endif //_DEBUG

//////////////////////////////////////////////////////////////////////
// Dopøedná deklarace tøíd

// tøída položky tabulky øetìzcù
class CStringTableItem;

//////////////////////////////////////////////////////////////////////
// Datové typy

// datový typ ID notifikace
enum ENotification 
{
	NOTIFICATION_ENEMY_UNIT_SEEN = 0,
	NOTIFICATION_ENEMY_UNIT_SEEN_BY_CIVILIZATION,
	NOTIFICATION_ENEMY_UNIT_DISAPPEARED,
	NOTIFICATION_SYSTEM_UNIT_SEEN,
	NOTIFICATION_SYSTEM_UNIT_SEEN_BY_CIVILIZATION,
	NOTIFICATION_SYSTEM_UNIT_DISAPPEARED,
	NOTIFICATION_BEING_ATTACKED,

	NOTIFICATION_COUNT		// poèet notifikací
};

//////////////////////////////////////////////////////////////////////
// Globální data

// tabulka ID jmen notifikací
extern CStringTableItem *g_aNotificationNameID[NOTIFICATION_COUNT];
// tabulka jmen notifikací
extern char *g_aNotificationName[NOTIFICATION_COUNT];

//////////////////////////////////////////////////////////////////////
// Globální funkce

// inicializuje notifikace do skriptù s pøíznakem vytváøení hry "bCreate"
void InitializeNotifications ( BOOL bCreate );
// znièí notifikace do skriptù
void DestructNotifications ();

// jednotce "pZUnit" pošle notifikaci "eNotification" s parametrem bagu "pBag"
inline void SendNotification ( CZUnit *pZUnit, enum ENotification eNotification, 
	CIBag *pBag ) 
{
	ASSERT ( ( pZUnit != NULL ) && ( pBag != NULL ) );
	ASSERT ( eNotification < NOTIFICATION_COUNT );
	ASSERT ( g_aNotificationNameID[eNotification] != NULL );

	// pošle jednotce notifikaci
	pZUnit->SendNotification ( g_aNotificationNameID[eNotification], pBag );

#ifdef _DEBUG
	TRACE_NOTIFICATION ( 
		". %s: on unit (ID=%d, CivilizationID=%d) with %d arguments\n", 
		g_aNotificationName[eNotification], ( pZUnit->GetSUnit () == NULL ) ? -1 : 
		pZUnit->GetSUnit ()->GetID (), 
		pZUnit->GetCivilization ()->GetSCivilization ()->GetCivilizationIndex (), 
		pBag->GetArgCount () );
#endif //_DEBUG
}

// jednotce "pZUnit" pošle notifikaci "eNotification" s parametrem jednotky "pUnit"
inline void SendNotification ( CZUnit *pZUnit, enum ENotification eNotification, 
	CSUnit *pUnit ) 
{
	ASSERT ( ( pZUnit != NULL ) && ( pUnit != NULL ) );
	ASSERT ( eNotification < NOTIFICATION_COUNT );
	ASSERT ( g_aNotificationNameID[eNotification] != NULL );

	// pøipraví parametry notifikace
	CIBag cBag;
	cBag.Create ();
	cBag.AddUnit ( pUnit->GetZUnit (), NULL );
	// pošle jednotce notifikaci
	pZUnit->SendNotification ( g_aNotificationNameID[eNotification], &cBag );
	cBag.Delete ();

#ifdef _DEBUG
	TRACE_NOTIFICATION ( 
		". %s: on unit (ID=%d, CivilizationID=%d) with unit parameter (ID=%d)\n", 
		g_aNotificationName[eNotification], ( pZUnit->GetSUnit () == NULL ) ? -1 : 
		pZUnit->GetSUnit ()->GetID (), 
		pZUnit->GetCivilization ()->GetSCivilization ()->GetCivilizationIndex (), 
		pUnit->GetID () );
#endif //_DEBUG
}

#endif //__SERVER_NOTIFICATIONS__HEADER_INCLUDED__
