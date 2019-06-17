/***********************************************************
 * 
 * Projekt: Strategicka hra
 *    Cast: Server hry - skilly
 *   Autor: Tomas Karban
 * 
 *   Popis: Bullet Attack skilla -- data skilly u jednotky
 * 
 ***********************************************************/

#ifndef __SERVER_BULLETATTACK_SKILL__HEADER_INCLUDED__
#define __SERVER_BULLETATTACK_SKILL__HEADER_INCLUDED__

#include "..\AbstractAttackSkill\SAbstractAttackSkill.h"

/////////////////////////////////////////////////////////////////////
// Data skilly u jednotky

struct SSBulletAttackSkill : public SSAbstractAttackSkill
{
    // zbyvajici cas do dalsiho vystrelu
    // (kazdy timeslice snizuje o 1, pri rovno nule strelba)
    // pri deaktivaci se sem ulozi cislo timeslicu,
    // ve kterem je mozno skillu znova "aktivovat"
    // (presneji -- aktivace se samozrejme provede okamzite,
    // ale pocatecni counter bude prislusne zvetsen)
    DWORD m_dwRemainingTime;

    // pointer na jednotku, na kterou utocime
    CSUnit *m_pAttackedUnit;
};

#endif  // __SERVER_BULLETATTACK_SKILL__HEADER_INCLUDED__
