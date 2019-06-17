/***********************************************************
 * 
 * Projekt: Strategicka hra
 *    Cast: Server hry - skilly
 *   Autor: Tomas Karban
 * 
 *   Popis: Bullet Defense skilla -- struktura z mapeditoru
 * 
 ***********************************************************/

#ifndef __MAPEDITOR_BULLETDEFENSE_SKILLTYPE__HEADER_INCLUDED__
#define __MAPEDITOR_BULLETDEFENSE_SKILLTYPE__HEADER_INCLUDED__

//////////////////////////////////////////////////////////////////////
// Struktura na prenos dat mezi mapeditorem a CreateGame na serveru

struct SMBulletDefenseSkillType
{
    // linearni koeficient vynasobeny 1000
    // (nulova obrana = 1000, absolutni obrana = 0)
    DWORD m_dwLinearCoefficient;
    // absolutni koeficient
    DWORD m_dwAbsoluteCoefficient;
};

#define SM_BULLETDEFENSE_SKILLTYPE_LIN_MIN 0
#define SM_BULLETDEFENSE_SKILLTYPE_LIN_MAX 1000
#define SM_BULLETDEFENSE_SKILLTYPE_ABS_MIN 0
#define SM_BULLETDEFENSE_SKILLTYPE_ABS_MAX 10000

#endif  // __MAPEDITOR_BULLETDEFENSE_SKILLTYPE__HEADER_INCLUDED__
