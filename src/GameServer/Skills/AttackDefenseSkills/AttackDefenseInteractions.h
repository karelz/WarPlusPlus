/***********************************************************
 * 
 * Projekt: Strategicka hra
 *    Cast: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: Typy interakci mezi jednotkami (attack/defense)
 * 
 ***********************************************************/

#ifndef _SERVER_ATTACKDEFENSE_INTERACTIONS__HEADER_INCLUDED_
#define _SERVER_ATTACKDEFENSE_INTERACTIONS__HEADER_INCLUDED_


//////////////////////////////////////////////////////////////////////
// Typy interakci mezi attack a defense skillami
//////////////////////////////////////////////////////////////////////

enum 
{
    Interaction_None = 0x0000,
    Interaction_Bullet = 0x0001,
    Interaction_Explosive = 0x0002,
    Interaction_Fire = 0x0004,
    Interaction_Radioactive = 0x0008,
    Interaction_Laser = 0x0010,
    Interaction_Plasma = 0x0020,

    Interaction_All = 0x003F
};

#endif  // _SERVER_ATTACKDEFENSE_INTERACTIONS__HEADER_INCLUDED_