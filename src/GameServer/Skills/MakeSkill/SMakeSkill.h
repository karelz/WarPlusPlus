/***********************************************************
 * 
 * Projekt: Strategick· hra
 *    »·st: Server hry - skilly
 *   Autor: Roman Zenka
 * 
 *   Popis: Make skilla
 * 
 ***********************************************************/

#ifndef __SERVER_MAKE_SKILL__HEADER_INCLUDED__
#define __SERVER_MAKE_SKILL__HEADER_INCLUDED__

#include "GameServer/GameServer/SResources.h"

class CISysCallMake;
class CZUnit;

#pragma pack(push, 1)

// Popis typu jednotek, ktere je skila schopna vyrabet
struct SSMakeSkillRecord
{
// Data
public:
    // typ jednotky
    CSUnitType *m_pUnitType;

    // Kolik timeslicu se tato jednotka vyrabi
    int m_nTimeToMake;

    // Kolik resourcu vyroba sezere
    TSResources m_ResourcesNeeded;

    // Jaky appearance se ma nastavit behem buildeni
    DWORD m_dwAppearance;
};

/////////////////////////////////////////////////////////////////////
// Make skilla - data ukladana u jednotky
struct SSMakeSkillUnit
{   
// Data
public:
    // Typ jednotky, ktery zrovna stavime
    CSUnitType *m_pUnitType;

    // Skript, ktery jednotka dostane, az dostavime
    CISOString *m_pScriptName;

    // Za jak dlouho bude dostaveno
    int m_nTimeRemaining;

	// Jaky je celkovy cas stavby
	int m_nTimeTotal;
    
    // Ukazatel na commandera
    CZUnit *m_pCommander;

    // Syscall, na kterem spime
    CISysCallMake *m_pSysCall;
};

#pragma pack(pop)

#endif // __SERVER_MAKE_SKILL__HEADER_INCLUDED__
