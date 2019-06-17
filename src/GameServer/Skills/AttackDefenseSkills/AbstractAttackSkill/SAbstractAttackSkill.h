/***********************************************************
 * 
 * Projekt: Strategicka hra
 *    Cast: Server hry - skilly
 *   Autor: Tomas Karban
 * 
 *   Popis: Abstraktni predek vsech attack skill -- data u jednotky
 * 
 ***********************************************************/

#ifndef __SERVER_ABSTRACTATTACK_SKILL__HEADER_INCLUDED__
#define __SERVER_ABSTRACTATTACK_SKILL__HEADER_INCLUDED__

// forward deklarace syscallu
class CISysCallAttack;

/////////////////////////////////////////////////////////////////////
// Data abstraktni attack skilly u jednotky

struct SSAbstractAttackSkill
{
    // pointer na syscall
    CISysCallAttack *m_pSysCall;
};

#endif  // __SERVER_ABSTRACTATTACK_SKILL__HEADER_INCLUDED__
