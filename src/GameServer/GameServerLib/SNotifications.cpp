/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: Notifikace serveru do skriptù
 * 
 ***********************************************************/

#include "StdAfx.h"
#include "SNotifications.h"

//////////////////////////////////////////////////////////////////////
// Globální data

// tabulka ID jmen notifikací
CStringTableItem *g_aNotificationNameID[NOTIFICATION_COUNT] = 
	{
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
	};
// tabulka jmen notifikací
char *g_aNotificationName[NOTIFICATION_COUNT] = 
	{
		"ENEMY_UNIT_SEEN",
		"ENEMY_UNIT_SEEN_BY_CIVILIZATION",
		"ENEMY_UNIT_DISAPPEARED",
		"SYSTEM_UNIT_SEEN",
		"SYSTEM_UNIT_SEEN_BY_CIVILIZATION",
		"SYSTEM_UNIT_DISAPPEARED",
		"UNIT_BEING_ATTACKED",
	};

//////////////////////////////////////////////////////////////////////
// Globální funkce

// inicializuje notifikace do skriptù s pøíznakem vytváøení hry "bCreate"
void InitializeNotifications ( BOOL bCreate ) 
{
	// projede notifikace
	for ( int nNotificationIndex = NOTIFICATION_COUNT; nNotificationIndex-- > 0; )
	{
		ASSERT ( g_aNotificationNameID[nNotificationIndex] == NULL );
		// inicializuje ID jména notifikace
		g_aNotificationNameID[nNotificationIndex] = g_StringTable.AddItem ( 
			g_aNotificationName[nNotificationIndex], bCreate != FALSE );
		ASSERT ( g_aNotificationNameID[nNotificationIndex] != NULL );
	}
}

// znièí notifikace do skriptù
void DestructNotifications () 
{
	// projede notifikace
	for ( int nNotificationIndex = NOTIFICATION_COUNT; nNotificationIndex-- > 0; )
	{
		// zjistí, je-li ID jména notifikace platné
		if ( g_aNotificationNameID[nNotificationIndex] != NULL )
		{	// ID jména notifikace je platné
			// zruší ID jména notifikace
			(void)g_aNotificationNameID[nNotificationIndex]->Release ();
			g_aNotificationNameID[nNotificationIndex] = NULL;
		}
	}
}
