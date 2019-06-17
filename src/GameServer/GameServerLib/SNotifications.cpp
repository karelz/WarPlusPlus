/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: Notifikace serveru do skript�
 * 
 ***********************************************************/

#include "StdAfx.h"
#include "SNotifications.h"

//////////////////////////////////////////////////////////////////////
// Glob�ln� data

// tabulka ID jmen notifikac�
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
// tabulka jmen notifikac�
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
// Glob�ln� funkce

// inicializuje notifikace do skript� s p��znakem vytv��en� hry "bCreate"
void InitializeNotifications ( BOOL bCreate ) 
{
	// projede notifikace
	for ( int nNotificationIndex = NOTIFICATION_COUNT; nNotificationIndex-- > 0; )
	{
		ASSERT ( g_aNotificationNameID[nNotificationIndex] == NULL );
		// inicializuje ID jm�na notifikace
		g_aNotificationNameID[nNotificationIndex] = g_StringTable.AddItem ( 
			g_aNotificationName[nNotificationIndex], bCreate != FALSE );
		ASSERT ( g_aNotificationNameID[nNotificationIndex] != NULL );
	}
}

// zni�� notifikace do skript�
void DestructNotifications () 
{
	// projede notifikace
	for ( int nNotificationIndex = NOTIFICATION_COUNT; nNotificationIndex-- > 0; )
	{
		// zjist�, je-li ID jm�na notifikace platn�
		if ( g_aNotificationNameID[nNotificationIndex] != NULL )
		{	// ID jm�na notifikace je platn�
			// zru�� ID jm�na notifikace
			(void)g_aNotificationNameID[nNotificationIndex]->Release ();
			g_aNotificationNameID[nNotificationIndex] = NULL;
		}
	}
}
