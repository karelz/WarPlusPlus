/***********************************************************
 * 
 * Projekt: Strategicka hra
 *    Cast: Server hry - skilly
 *   Autor: Tomas Karban
 * 
 *   Popis: Abstraktni Attack skilla -- struktura z mapeditoru
 *          (presneji: tato struktura samozrejme nikde
 *           v mapeditoru videt neni, pouziva se jen jako
 *           spolecny predek pro vsechny attack skilly)
 * 
 ***********************************************************/

#ifndef __MAPEDITOR_ABSTRACTATTACK_SKILLTYPE__HEADER_INCLUDED__
#define __MAPEDITOR_ABSTRACTATTACK_SKILLTYPE__HEADER_INCLUDED__

//////////////////////////////////////////////////////////////////////
// Struktura na prenos dat mezi mapeditorem a CreateGame na serveru

struct SMAbstractAttackSkillType
{
    // akcni polomer (maximalni dostrel)
    DWORD m_dwAttackRadius;
    
    // vyskovy interval, kam muze attack skilla utocit
    DWORD m_dwAttackMinAltitude;
    DWORD m_dwAttackMaxAltitude;
};

// konstanty pro omezeni platnosti dat z mapeditoru si urci
// kazda konkretni attack skilla zvlast

#endif  // __MAPEDITOR_ABSTRACTATTACK_SKILLTYPE__HEADER_INCLUDED__
