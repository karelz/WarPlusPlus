/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: Notifikace serveru do skript�
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
// Dop�edn� deklarace t��d

// t��da polo�ky tabulky �et�zc�
class CStringTableItem;

//////////////////////////////////////////////////////////////////////
// Datov� typy

// datov� typ ID notifikace
enum ENotification 
{
	NOTIFICATION_ENEMY_UNIT_SEEN = 0,
	NOTIFICATION_ENEMY_UNIT_SEEN_BY_CIVILIZATION,
	NOTIFICATION_ENEMY_UNIT_DISAPPEARED,
	NOTIFICATION_SYSTEM_UNIT_SEEN,
	NOTIFICATION_SYSTEM_UNIT_SEEN_BY_CIVILIZATION,
	NOTIFICATION_SYSTEM_UNIT_DISAPPEARED,
	NOTIFICATION_BEING_ATTACKED,

	NOTIFICATION_COUNT		// po�et notifikac�
};

//////////////////////////////////////////////////////////////////////
// Glob�ln� data

// tabulka ID jmen notifikac�
extern CStringTableItem *g_aNotificationNameID[NOTIFICATION_COUNT];
// tabulka jmen notifikac�
extern char *g_aNotificationName[NOTIFICATION_COUNT];

//////////////////////////////////////////////////////////////////////
// Glob�ln� funkce

// inicializuje notifikace do skript� s p��znakem vytv��en� hry "bCreate"
void InitializeNotifications ( BOOL bCreate );
// zni�� notifikace do skript�
void DestructNotifications ();

// jednotce "pZUnit" po�le notifikaci "eNotification" s parametrem bagu "pBag"
inline void SendNotification ( CZUnit *pZUnit, enum ENotification eNotification, 
	CIBag *pBag ) 
{
	ASSERT ( ( pZUnit != NULL ) && ( pBag != NULL ) );
	ASSERT ( eNotification < NOTIFICATION_COUNT );
	ASSERT ( g_aNotificationNameID[eNotification] != NULL );

	// po�le jednotce notifikaci
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

// jednotce "pZUnit" po�le notifikaci "eNotification" s parametrem jednotky "pUnit"
inline void SendNotification ( CZUnit *pZUnit, enum ENotification eNotification, 
	CSUnit *pUnit ) 
{
	ASSERT ( ( pZUnit != NULL ) && ( pUnit != NULL ) );
	ASSERT ( eNotification < NOTIFICATION_COUNT );
	ASSERT ( g_aNotificationNameID[eNotification] != NULL );

	// p�iprav� parametry notifikace
	CIBag cBag;
	cBag.Create ();
	cBag.AddUnit ( pUnit->GetZUnit (), NULL );
	// po�le jednotce notifikaci
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
