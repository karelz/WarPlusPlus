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

// Number of invisibility flags
#define INVISIBILITY_COUNT 32  // Must be less than 32

// Constants for use in the SUnitTypeHeader::m_dwFlags
enum EUnitTypeFlags{
  UnitTypeFlags_None = 0,
  UnitTypeFlags_Selectable = 1, // True if the unit can be selected on the client
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

#endif //__MAP_DATA_TYPES__H__
