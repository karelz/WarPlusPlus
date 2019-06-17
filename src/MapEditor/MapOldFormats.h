#ifndef MAPEDITOR_MAP_OLD_FORMATS_H_
#define MAPEDITOR_MAP_OLD_FORMATS_H_

// Map file structures
// Version 1.3

// Same as version 1.4
// except
struct tagSFindPathGraphHeader1p3{
  char m_pName[64];
  DWORD m_aAllowedLandTypes[8];
  BYTE m_pReserved[56];
};
typedef struct tagSFindPathGraphHeader1p3 SFindPathGraphHeader1p3;

struct tagSUnitTypeHeader1p3{
  DWORD m_dwID; // unit ID
  char m_pName[32]; // name of the unit
  DWORD m_dwLifeMax; // maximum life number
  DWORD m_dwViewRadius; // the view radius
  DWORD m_aMoveWidth[8]; // the width for move and standing size (it's square)
  DWORD m_dwAppearancesNumber; // number of appearances in the unit
  char m_aModeNames[8][32]; // names of modes
  DWORD m_aAllowedLandTypes[8][8]; // eight 256 bit arrays of allowed land types
  DWORD m_aDefaultAppearances[8]; // default appearance IDs for all modes
  DWORD m_dwSkillTypesCount; // the number of skill types for this unit
  DWORD m_dwFlags; // Some special flags (see EUnitTypeFlags)
  DWORD m_dwZPos; // Z pos of the unit after its creation
  DWORD m_aInvisibilityFlags[8]; // Invisibility flags for the modes
  DWORD m_aDetectionFlags[8];  // Detection flags for the modes
  
  BYTE m_aReserved[324]; // reserved place
};
typedef struct tagSUnitTypeHeader1p3 SUnitTypeHeader1p3;

// Map file structures
// Version 1.0

// Same as version 1.1
// Except
struct tagSCivilizationHeader1p0{
  DWORD m_dwID;
  DWORD m_dwColor;
  char m_aName[256];
  char m_aScriptSet[64];
  int m_aResources[RESOURCE_COUNT];
  char m_aReserved[184 - (sizeof(int) * RESOURCE_COUNT)];
};
typedef struct tagSCivilizationHeader1p0 SCivilizationHeader1p0;

// Map file structures (OLD)
// Version 0.9

/*

*.map file
  SMapHeader
  SMapexLibraryNode[m_dwMapexLibraryCount] -> *.mlb
  SCivilizationHeader[m_dwCivilizationsCount]
  SUnitTypeLibraryNode[m_dwUnitTypeLibraryCount] -> *.ulb
  DWORD aMapSquareOffsets[m_dwHeight][m_dwWidth]
  SMapSquareHeader[m_dwHeight][m_dwWidth]
    SMapexInstanceHeader[m_dwBL1MapexesNum]
    SMapexInstanceHeader[m_dwBL2MapexesNum]
    SMapexInstanceHeader[m_dwBL3MapexesNum]
    SULMapexInstanceHeader[m_dwULMapexesNum]
  SUnitHeader[m_dwUnitCount]
  [RESOURCE_COUNT]
	 SResource
    DWORD aResourceIcon[RESOURCE_ICON_WIDTH * RESOURCE_ICON_HEIGHT]
  SInvisibility[INVISIBILITY_COUNT]

*.mlb archive
  Library.Header file
    SMapexLibraryHeader  
    char szMapexFileName[m_dwMapexCount][30]

  Mapex<ID>.Header file (szMapexFileName)
    SMapexHeader
    BYTE aLandTypes[m_dwYSize][m_dwXSize]

*.ulb archive
  Library.Header file
    SUnitTypeLibraryHeader
    char szUnitTypeFileName[m_dwUnitTypeCount][30]

  Unit<ID>.Header file (szUnitTypeFileName)
    SUnitTypeHeader
    SAppearance[m_dwAppearancesNumber]
    SSkillTypeHeader[m_dwSkillTypesCount]
      BYTE aSkillTypeData[m_dwDataSize]
    
*/

#include "Common\Map\MapDataTypes.h"

// -----------------------------------------------------------------------
// *.map file
struct tagSMapHeader_Old{
  DWORD m_dwWidth; // size of the map
  DWORD m_dwHeight;
  char m_pName[51]; // name of the map
  char m_pDescription[1001]; // description
  DWORD m_dwMapexLibraryCount; // number of mapex libraries
  char m_pLandTypesFile[256]; // path to the land types file
  DWORD m_dwCivilizationsCount; // number of civilizations
  DWORD m_dwUnitTypeLibraryCount; // number of unit libraries in the map
  DWORD m_dwUnitCount; // number of unit instances in the map
  DWORD m_dwMapVersion; // unique version number of this map
  DWORD m_aUsedLandTypes[8]; // Bit array of used land types
  BYTE m_pReserved[682];
};
typedef struct tagSMapHeader_Old SMapHeader_Old;

struct tagSMapexLibraryNode_Old{
  char m_pFileName[256]; // only the name of the file
  char m_pFullName[1024]; // full path
};
typedef struct tagSMapexLibraryNode_Old SMapexLibraryNode_Old;

struct tagSCivilizationHeader_Old{
  DWORD m_dwID;
  DWORD m_dwColor;
  char m_aName[256];
  char m_aScriptSet[52];
  int m_aResources[RESOURCE_COUNT];
  char m_aReserved[196 - (RESOURCE_COUNT * sizeof(int))];
};
typedef struct tagSCivilizationHeader_Old SCivilizationHeader_Old;

struct tagSUnitTypeLibraryNode_Old{
  char m_pFileName[256]; // only the name of the file
  char m_pFullName[1024]; // full path
};
typedef struct tagSUnitTypeLibraryNode_Old SUnitTypeLibraryNode_Old;

struct tagSMapSquareHeader_Old{ // struct for saving this object to file
  DWORD m_dwBL1MapexesNum;
  DWORD m_dwBL2MapexesNum;
  DWORD m_dwBL3MapexesNum;
  DWORD m_dwULMapexesNum;
};
typedef struct tagSMapSquareHeader_Old SMapSquareHeader_Old;

struct tagSMapexInstanceHeader_Old{  // struct for saving this object to file
  DWORD m_dwX;
  DWORD m_dwY;
  DWORD m_dwMapexID;
};
typedef struct tagSMapexInstanceHeader_Old SMapexInstanceHeader_Old;

struct tagSULMapexInstanceHeader_Old{ // struct to save this object to file
  DWORD m_dwX;
  DWORD m_dwY;
  DWORD m_dwMapexID;
  DWORD m_dwZ;
};
typedef struct tagSULMapexInstanceHeader_Old SULMapexInstanceHeader_Old;

struct tagSUnitHeader_Old{
  DWORD m_dwUnitType;
  DWORD m_dwCivilization;
  DWORD m_dwX;
  DWORD m_dwY;
  DWORD m_dwZ;
  BYTE m_nDirection;
  BYTE m_pReserved[11];
};
typedef struct tagSUnitHeader_Old SUnitHeader_Old;

// -----------------------------------------------------------------------
// *.mlb archive
struct tagSMapexLibraryHeader_Old{
  char m_pName[51]; // name of the library
  DWORD m_dwMapexCount; // number of mapexes in the library
  BYTE m_pReserved[9];
};
typedef struct tagSMapexLibraryHeader_Old SMapexLibraryHeader_Old;

struct tagSMapexHeader_Old{
  DWORD m_dwID;
  DWORD m_dwXSize, m_dwYSize;
  char m_pGraphicsFileName[30]; // standard file name (max len 29)
  char m_pName[51]; // name of the mapex
  BYTE m_pReserved[35];
};
typedef struct tagSMapexHeader_Old SMapexHeader_Old;

// -----------------------------------------------------------------------
// *.ulb archive
struct tagSUnitTypeLibraryHeader_Old{
  char m_pName[51]; // name of the library
  DWORD m_dwUnitTypeCount; // number of units in the library
  BYTE m_pReserved[9];
};
typedef struct tagSUnitTypeLibraryHeader_Old SUnitTypeLibraryHeader_Old;

struct tagSUnitTypeHeader_Old{
  DWORD m_dwID; // unit ID
  char m_pName[31]; // name of the unit
  DWORD m_dwLifeMax; // maximum life number
  DWORD m_dwViewRadius; // the view radius
  DWORD m_aMoveWidth[8]; // the width for move and standing size (it's square)
  DWORD m_dwAppearancesNumber; // number of appearances in the unit
  char m_aModeNames[8][31]; // names of modes
  BYTE m_aAllowedLandTypes[8][8]; // eight 256 bit arrays of allowed land types
  DWORD m_aDefaultAppearances[8]; // default appearance IDs for all modes
  DWORD m_dwSkillTypesCount; // the number of skill types for this unit
  DWORD m_dwFlags; // Some special flags (see EUnitTypeFlags)
  DWORD m_dwZPos; // Z pos of the unit after its creation
  DWORD m_aInvisibilityFlags[8]; // Invisibility flags for the modes
  DWORD m_aDetectionFlags[8];  // Detection flags for the modes
  
  BYTE m_aReserved[17]; // reserved place
};
typedef struct tagSUnitTypeHeader_Old SUnitTypeHeader_Old;

struct tagSSkillTypeHeader_Old{
  char m_pName[31]; // the name of the skill
  BOOL m_bEnabled; // the skill is initialy enabled
  BYTE m_nAllowedModes; // bit array of allowed modes for this skill
  DWORD m_dwDataSize; // size (in bytes) of skill parameters
  
  BYTE m_aReserved[24];
};
typedef struct tagSSkillTypeHeader_Old SSkillTypeHeader_Old;

// struct for saving us in the file
struct tagSAppearance_Old{
  DWORD m_dwID; // id of the appearance
  char m_szName[31]; // the name
  char m_szDirections[8][51]; // file names of direction animations (if empty -> no anim)
  
  BYTE m_aReserved[69]; // reserved
};
typedef struct tagSAppearance_Old SAppearance_Old;

struct tagSResource_Old{
  char m_szName[31]; // the name
  
  BYTE m_aReserved[97];
}; // After this is saved the image which is RESOURCE_ICON_WIDTH * RESOURCE_ICON_HEIGHT * 4 bytes long
typedef struct tagSResource_Old SResource_Old;

struct tagSInvisibility_Old{
  char m_szName[32];

  BYTE m_aReserved[32];
};
typedef struct tagSInvisibility_Old SInvisibility_Old;

#endif