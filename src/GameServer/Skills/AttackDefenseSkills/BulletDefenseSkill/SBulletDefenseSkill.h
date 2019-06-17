/***********************************************************
 * 
 * Projekt: Strategicka hra
 *    Cast: Server hry - skilly
 *   Autor: Tomas Karban
 * 
 *   Popis: Bullet Defense skilla -- data skilly u jednotky
 * 
 ***********************************************************/

#ifndef __SERVER_BULLETDEFENSE_SKILL__HEADER_INCLUDED__
#define __SERVER_BULLETDEFENSE_SKILL__HEADER_INCLUDED__

/////////////////////////////////////////////////////////////////////
// Data skilly u jednotky

struct SSBulletDefenseSkill
{
    // data uzitecna pro "slizy" -- skilly nepratel oslabujici obranu

    // linearni koeficient (vynasobeny 1000)
    // (v normalnim stavu bude roven 1000 -- obrana beze zmeny,
    // sliz muze snizit az na nulu -- vypnout tuto defense skillu)
    DWORD m_nPostLinearCoefficient;

    // absolutni koeficient
    // (v normalnim stavu bude roven 0)
    DWORD m_nPostAbsoluteCoefficient;
};

#endif  // __SERVER_BULLETDEFENSE_SKILL__HEADER_INCLUDED__
