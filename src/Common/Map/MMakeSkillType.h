#ifndef __MMAKESKILLTYPE_HEADER_INCLUDED__
#define __MMAKESKILLTYPE_HEADER_INCLUDED__

#pragma pack(push, 1)

/////////////////////////////////////////////////////////////////////
// Make skilla

#include "Common\Map\MapDataTypes.h"

#define MAKESKILLTYPE_SIZE(pData) (sizeof(SMMakeSkillType)+(pData)->m_nRecords*sizeof(SMMakeSkillTypeRecord))

struct SMMakeSkillType
{   
public:
    enum EMMakeSkillTypeConstants {
        maxTimeToMake=1000000, // Maximalni doba, za kterou se postavi jednotka
    };

// Data
public:
    // Pocet zaznamu o jednotlivych jednotkach, ktere umime vyrabet
    int m_nRecords;

    // Zde nasleduje m_nRecords jednotlivych zaznamu (SMMakeSkillRecord)
};

// Popis typu jednotek, ktere je skila schopna vyrabet
struct SMMakeSkillTypeRecord
{
// Data
public:
    // Identifikace typu jednotky
    DWORD m_dwUnitType;

    // Kolik timeslicu se tato jednotka vyrabi
    int m_nTimeToMake;

    // Kolik resourcu vyroba sezere
    int m_ResourcesNeeded[RESOURCE_COUNT];

    // Jaky appearance se ma nastavit behem buildeni
    DWORD m_dwAppearance;
};

#pragma pack(pop)

#endif // __MMAKESKILLTYPE_HEADER_INCLUDED__