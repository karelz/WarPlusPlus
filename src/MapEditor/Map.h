// Map file structures
// Version 1.0

#ifndef MAP_H_
#define MAP_H_

#pragma pack(1)

/*

*.map file
  SFileVersionHeader
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
    SMapexFileVersionHeader
    SMapexLibraryHeader  
    char szMapexFileName[m_dwMapexCount][30]

  Mapex<ID>.Header file (szMapexFileName)
    SMapexHeader
    BYTE aLandTypes[m_dwYSize][m_dwXSize]

*.ulb archive
  Library.Header file
    SUnitTypeFileVersionHeader
    SUnitTypeLibraryHeader
    char szUnitTypeFileName[m_dwUnitTypeCount][30]

  Unit<ID>.Header file (szUnitTypeFileName)
    SUnitTypeHeader
    SAppearance[m_dwAppearancesNumber]
    SSkillTypeHeader[m_dwSkillTypesCount]
      BYTE aSkillTypeData[m_dwDataSize]
    
*/

#include "MapDataTypes.h"

// -----------------------------------------------------------------------
// *.map file

#define MAP_FILE_IDENTIFIER {0x6b,0x10,0x09,0xc1,0x70,0xf9,0x11,0xd4,0xb0,0xdb,0x00,0x4f,0x49,0x06,0x8b,0xd6}
#define MAP_FILE_VERSION(MajorVersion, MinorVersion) ((((DWORD)MajorVersion) << 8) | (((DWORD)MinorVersion) & 0x0FF))
#define MAP_FILE_MAJORVERSION(Version) (((DWORD)Version) >> 8)
#define MAP_FILE_MINORVERSION(Version) (((DWORD)Version) & 0x0FF)

#define CURRENT_MAP_FILE_VERSION MAP_FILE_VERSION(1, 0)
#define COMPATIBLE_MAP_FILE_VERSION MAP_FILE_VERSION(1, 0)
#define COMPATIBLE_MAPEX_FILE_VERSION MAP_FILE_VERSION(1, 0)
#define COMPATIBLE_UNITTYPE_FILE_VERSION MAP_FILE_VERSION(1, 0)

struct tagSFileVersionHeader{
  BYTE m_aMapFileID[16]; // Special ID for our files
  DWORD m_dwFormatVersion; // Format version of this file
  DWORD m_dwCompatibleFormatVersion; // Compatible format version (the smallest possible)
  BYTE m_aReserved[8];   // Some reserved bytes
};
typedef struct tagSFileVersionHeader SFileVersionHeader;

struct tagSMapHeader{
  DWORD m_dwWidth;   // size of the map in mapcells
  DWORD m_dwHeight;
  char m_pName[64];  // name of the map
  char m_pDescription[1024]; // description
  DWORD m_dwMapexLibraryCount; // number of mapex libraries
  char m_pLandTypesFile[256]; // path to the land types file
  DWORD m_dwCivilizationsCount; // number of civilizations
  DWORD m_dwUnitTypeLibraryCount; // number of unit libraries in the map
  DWORD m_dwUnitCount; // number of unit instances in the map
  DWORD m_dwMapVersion; // unique version number of this map
  DWORD m_aUsedLandTypes[8]; // Bit array of used land types (256 bits)
  BYTE m_aReserved[644];
};
typedef struct tagSMapHeader SMapHeader;

struct tagSMapexLibraryNode{
  char m_pFileName[256];  // only the name of the file
  char m_pFullName[1024]; // full path
};
typedef struct tagSMapexLibraryNode SMapexLibraryNode;

struct tagSCivilizationHeader{
  DWORD m_dwID;
  DWORD m_dwColor;
  char m_aName[256];
  char m_aScriptSet[64];
  int m_aResources[RESOURCE_COUNT];
  char m_aReserved[184 - (sizeof(int) * RESOURCE_COUNT)];
};
typedef struct tagSCivilizationHeader SCivilizationHeader;

struct tagSUnitTypeLibraryNode{
  char m_pFileName[256]; // only the name of the file
  char m_pFullName[1024]; // full path
};
typedef struct tagSUnitTypeLibraryNode SUnitTypeLibraryNode;

struct tagSMapSquareHeader{ // struct for saving this object to file
  DWORD m_dwBL1MapexesNum;
  DWORD m_dwBL2MapexesNum;
  DWORD m_dwBL3MapexesNum;
  DWORD m_dwULMapexesNum;
};
typedef struct tagSMapSquareHeader SMapSquareHeader;

struct tagSMapexInstanceHeader{  // struct for saving this object to file
  DWORD m_dwX;
  DWORD m_dwY;
  DWORD m_dwMapexID;
};
typedef struct tagSMapexInstanceHeader SMapexInstanceHeader;

struct tagSULMapexInstanceHeader{ // struct to save this object to file
  DWORD m_dwX;
  DWORD m_dwY;
  DWORD m_dwMapexID;
  DWORD m_dwZ;
};
typedef struct tagSULMapexInstanceHeader SULMapexInstanceHeader;

struct tagSUnitHeader{
  DWORD m_dwUnitType;
  DWORD m_dwCivilization;
  DWORD m_dwX;
  DWORD m_dwY;
  DWORD m_dwZ;
  DWORD m_nDirection;
  BYTE m_pReserved[8];
};
typedef struct tagSUnitHeader SUnitHeader;

// -----------------------------------------------------------------------
// *.mlb archive

#define MAPEX_FILE_IDENTIFIER {0xfe,0x10,0x09,0xc1,0x70,0xf9,0x11,0xd4,0xb0,0xdb,0x00,0x4f,0x49,0x06,0x8b,0xd6}

struct tagSMapexFileVersionHeader{
  BYTE m_aFileID[16];  // File identifier
  DWORD m_dwFormatVersion; // Format verion of this file
  DWORD m_dwCompatibleFormatVersion; // Last compatible format version
  BYTE m_aReserved[8];
};
typedef struct tagSMapexFileVersionHeader SMapexFileVersionHeader;

struct tagSMapexLibraryHeader{
  char m_pName[64]; // name of the library
  DWORD m_dwMapexCount; // number of mapexes in the library
  BYTE m_aReserved[60];
};
typedef struct tagSMapexLibraryHeader SMapexLibraryHeader;

struct tagSMapexHeader{
  DWORD m_dwID;
  DWORD m_dwXSize, m_dwYSize;
  char m_pGraphicsFileName[32]; // standard file name (max len 29)
  char m_pName[64]; // name of the mapex
  BYTE m_aReserved[20];
};
typedef struct tagSMapexHeader SMapexHeader;

// -----------------------------------------------------------------------
// *.ulb archive

#define UNITTYPE_FILE_IDENTIFIER {0xfd,0x10,0x09,0xc1,0x70,0xf9,0x11,0xd4,0xb0,0xdb,0x00,0x4f,0x49,0x06,0x8b,0xd6}

struct tagSUnitTypeFileVersionHeader{
  BYTE m_aFileID[16];  // File identifier
  DWORD m_dwFormatVersion; // Format verion of this file
  DWORD m_dwCompatibleFormatVersion; // Last compatible format version
  BYTE m_aReserved[8];
};
typedef struct tagSUnitTypeFileVersionHeader SUnitTypeFileVersionHeader;

struct tagSUnitTypeLibraryHeader{
  char m_pName[64]; // name of the library
  DWORD m_dwUnitTypeCount; // number of units in the library
  BYTE m_aReserved[60];
};
typedef struct tagSUnitTypeLibraryHeader SUnitTypeLibraryHeader;

struct tagSUnitTypeHeader{
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
typedef struct tagSUnitTypeHeader SUnitTypeHeader;

struct tagSSkillTypeHeader{
  char m_pName[32]; // the name of the skill
  BOOL m_bEnabled; // the skill is initialy enabled
  BYTE m_nAllowedModes; // bit array of allowed modes for this skill
  DWORD m_dwDataSize; // size (in bytes) of skill parameters
  
  BYTE m_aReserved[23];
};
typedef struct tagSSkillTypeHeader SSkillTypeHeader;

// struct for saving us in the file
struct tagSAppearance{
  DWORD m_dwID; // id of the appearance
  char m_szName[32]; // the name
  char m_szDirections[8][64]; // file names of direction animations (if empty -> no anim)
  
  BYTE m_aReserved[476]; // reserved
};
typedef struct tagSAppearance SAppearance;

struct tagSResource{
  char m_szName[32]; // the name
  
  BYTE m_aReserved[96];
}; // After this is saved the image which is RESOURCE_ICON_WIDTH * RESOURCE_ICON_HEIGHT * 4 bytes long
typedef struct tagSResource SResource;

struct tagSInvisibility{
  char m_szName[32];

  BYTE m_aReserved[32];
};
typedef struct tagSInvisibility SInvisibility;

#pragma pack()
 
#endif
