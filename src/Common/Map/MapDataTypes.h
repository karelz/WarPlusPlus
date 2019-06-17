// Map file data types

#ifndef __MAP_DATA_TYPES__H__
#define __MAP_DATA_TYPES__H__

// size of one mapsquare in mapcells
#define MAPSQUARE_WIDTH 64
#define MAPSQUARE_HEIGHT 64

// size of one mapcell in pixels
#define MAPCELL_WIDTH 16
#define MAPCELL_HEIGHT 16

// Size of the resource icon
#define RESOURCE_ICON_WIDTH 16
#define RESOURCE_ICON_HEIGHT 16

// Number of resources
#define RESOURCE_COUNT 16  // Must be less than 32 (I think)

// Number of resources accessible from scripts (less than or equal to RESOURCE_COUNT)
#define RESOURCE_USED_COUNT RESOURCE_COUNT

// Number of invisibility flags
#define INVISIBILITY_COUNT 32  // Must be less than 32

// Constants for use in the SUnitTypeHeader::m_dwFlags
enum EUnitTypeFlags{
  UnitTypeFlags_None = 0x00000000,
  UnitTypeFlags_Selectable = 0x00000001, // True if the unit can be selected on the client

  UnitTypeFlags_LifeBar_ShowAlways = 0x000000000, // Life bar is always visible
  UnitTypeFlags_LifeBar_ShowWhenSelected = 0x00000002, // Life bar is visible when the unit is selected
  UnitTypeFlags_LifeBar_ShowNever = 0x00000006, // Life bar is never visible
  UnitTypeFlags_LifeBar_Mask = 0x00000006, // Mask for accessing the lifebar information in the flags
};

// the directions in numbers
typedef enum{
  Dir_North = 0,
  Dir_NorthEast = 1,
  Dir_East = 2,
  Dir_SouthEast = 3,
  Dir_South = 4,
  Dir_SouthWest = 5,
  Dir_West = 6,
  Dir_NorthWest = 7,
	Dir_Random = 8,
} EDirections;

// transparent land type ID
#define TRANSPARENT_LAND_TYPE_ID		0x00

#endif //__MAP_DATA_TYPES__H__
