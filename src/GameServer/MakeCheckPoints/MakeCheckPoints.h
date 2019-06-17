/***********************************************************
 * 
 * Projekt: Strategicka hra
 *    Cast: Server hry - hledani cesty
 *   Autor: Tomas Karban
 * 
 *   Popis: vytvoreni seznamu checkpointu z vysledku vlny
 * 
 ***********************************************************/

#ifndef __SERVER_MAKECHECKPOINTS__HEADER_INCLUDED__
#define __SERVER_MAKECHECKPOINTS__HEADER_INCLUDED__

/////////////////////////////////////////////////////////////////////
// Exportovana funkce, ktera vytvori seznam checkpointu

void MakeCheckPoints(CPointDW pointStart, CPointDW pointEnd);
BOOL IsPathFree(CPointDW pointStart, CPointDW pointEnd);

#endif  // __SERVER_MAKECHECKPOINTS__HEADER_INCLUDED__
