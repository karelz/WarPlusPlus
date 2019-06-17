/***********************************************************
 * 
 * Projekt: Strategicka hra
 *    Cast: Server hry - skilly
 *   Autor: Tomas Karban
 * 
 *   Popis: Bullet Attack skilla -- struktura z mapeditoru
 * 
 ***********************************************************/

#ifndef __MAPEDITOR_BULLETATTACK_SKILLTYPE__HEADER_INCLUDED__
#define __MAPEDITOR_BULLETATTACK_SKILLTYPE__HEADER_INCLUDED__

#include "Common\Map\MAbstractAttackSkillType.h"
#include "GameServer\GameServer\GameServerCompileSettings.h"

//////////////////////////////////////////////////////////////////////
// Struktura na prenos dat mezi mapeditorem a CreateGame na serveru

struct SMBulletAttackSkillType : public SMAbstractAttackSkillType
{
    // cas pred prvnim vystrelem (v timeslicech)
    DWORD m_dwLoadTime;
    
    // cas mezi dvema vystrely (v timeslicech)
    // (0 = strilej kazdy timeslice, n = mezi dvema vystrely n timeslicu ticho)
    DWORD m_dwFirePeriod;

    // sila utoku
    int m_nAttackIntensity;

    // vzhled jednotky pri strelbe
    DWORD m_dwAppearanceID;
    
    // offset startu animace (o kolik timeslicu drive zacne
    // animace pred samotnym vystrelem)
    DWORD m_dwAnimationOffset;

    // delka animace v timeslicech
    DWORD m_dwAnimationLength;
};

// doba pred prvnim vystrelem
#define SM_BULLETATTACK_SKILLTYPE_LOADTIME_MAX 1000
// doba mezi dvema vystrely
#define SM_BULLETATTACK_SKILLTYPE_FIREPERIOD_MAX 1000
// intenzita jednoho vystrelu
#define SM_BULLETATTACK_SKILLTYPE_INTENSITY_MIN 1
#define SM_BULLETATTACK_SKILLTYPE_INTENSITY_MAX 10000

// omezujici konstanty pro data z abstraktniho predka
// akcni polomer (128 -- dva mapsquary)
#define SM_BULLETATTACK_SKILLTYPE_RADIUS_MIN 1
#define SM_BULLETATTACK_SKILLTYPE_RADIUS_MAX 128
#if (SM_BULLETATTACK_SKILLTYPE_RADIUS_MAX > MAX_GET_UNITS_IN_AREA_RADIUS)
#error Konstanta "SM_BULLETATTACK_SKILLTYPE_RADIUS_MAX" zadana v souboru "Common\Map\MBulletAttackSkillType.h" je prilis velka!
#endif

// vyskovy interval
#define SM_BULLETATTACK_SKILLTYPE_ALTITUDE_MIN 1
#define SM_BULLETATTACK_SKILLTYPE_ALTITUDE_MAX UINT_MAX

// appearance ID nelze rozumne assertovat
// start offset muze byt cokoliv

// delka animace musi byt nenulova
#define SM_BULLETATTACK_SKILLTYPE_ANIMATIONLENGTH_MIN 1

#endif  // __MAPEDITOR_BULLETATTACK_SKILLTYPE__HEADER_INCLUDED__
