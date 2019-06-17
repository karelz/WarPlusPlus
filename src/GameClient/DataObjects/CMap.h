#ifndef CMAP_H_
#define CMAP_H_

#include "CMapex.h"
#include "CCivilization.h"

#include "CGeneralUnitType.h"
#include "CMapSquare.h"
#include "CResource.h"
#include "CInvisibility.h"

#include "Common\Map\Map.h"

#define MAP_TIMESLICE_HISTORY_COUNT 30

// class for the map
class CCMap : public CObject
{
  DECLARE_DYNAMIC(CCMap);

public:
// constructor & destructor
  CCMap();
  virtual ~CCMap();

// debug functions
#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

// creation
  // Creates the map object from the map file
  // also must know paths to libraries
  void Create(CDataArchive TheMapArchive);

  // Deletes the map object
  void Delete();

// Global lock
  CReadWriteLock *GetGlobalLock(){ return &m_lockGlobal; }

// The TheMap archive global lock
  CMutex *GetArchiveLock(){ return &m_lockArchive; }

// mapexes
  // Adds mapex to the list of all mapexes in the map (used by libraries)
  void AddMapex(CCMapex *pMapex);
  // finds the mapex by its ID
  CCMapex *GetMapex(DWORD dwID);

// General unit types
  // returns general unit type by ID
  CCGeneralUnitType *GetGeneralUnitType(DWORD dwID){ ASSERT_VALID(this); CCGeneralUnitType *pGUT = NULL;
    VERIFY(m_mapGeneralUnitTypes.Lookup(dwID, pGUT)); return pGUT; }
  // Adds general unit type to the list of all general unit types (used by libraries)
  void AddGeneralUnitType(CCGeneralUnitType *pUnitType);

// Map squares
  // Creates new mapsquare for given coordinates (loads it from the disk)
  CCMapSquare *CreateMapSquare(DWORD dwX, DWORD dwY);

  // returns the map square for given coordinates in mapsquares
  // Map must have global lock locked at least for reading
  // If it's locked for writing -> set the bWriteLocked to TRUE
  CCMapSquare *GetMapSquare(DWORD dwXPos, DWORD dwYPos, BOOL bWriteLocked = FALSE);

// Get/Set functions
  // Returns the width in mapsquares
  DWORD GetWidth(){ ASSERT_VALID(this); return m_dwWidth; }
  // Returns the height in mapsquares
  DWORD GetHeight(){ ASSERT_VALID(this); return m_dwHeight; }

  // Returns the width in mapcells
  DWORD GetWidthInCells(){ ASSERT_VALID(this); return m_dwWidth * MAPSQUARE_WIDTH; }
  // Returns the height in mapcells
  DWORD GetHeightInCells(){ ASSERT_VALID(this); return m_dwHeight * MAPSQUARE_HEIGHT; }

  // Returns the unique version umber of this map
  DWORD GetVersion(){ ASSERT_VALID(this); return m_dwVersionNumber; }

  // Returns the name of the map
  CString GetName(){ ASSERT_VALID(this); return m_strName; }
  // Returns the description of the map
  CString GetDescription(){ ASSERT_VALID(this); return m_strDescription; }

  // Returns the archive object, from which the map was loaded
  CDataArchive GetMapArchive(){ ASSERT_VALID(this); return m_TheMapArchive; }

  // Returns current timeslice for the client
  DWORD GetCurrentTimeslice(){ return m_dwCurrentTimeslice; }
  // Returns current timeslice length in milliseconds (its just a guess)
  DWORD GetTimesliceLength(){ return m_dwCurrentEstimateTimesliceInterval; }
  // Returns last known timeslice from the server
  DWORD GetLastKnownTimeslice(){ return m_dwLastKnownTimeslice; }

  // Returns resource object for given index
  CCResource *GetResource(DWORD dwIndex){ ASSERT_VALID(this); ASSERT(dwIndex < RESOURCE_COUNT); return m_aResources[dwIndex]; }

  // Returns invisibility object for given index
  CCInvisibility *GetInvisibility(DWORD dwIndex){ ASSERT_VALID(this); ASSERT(dwIndex < INVISIBILITY_COUNT); return m_aInvisibilities[dwIndex]; }

// Civilizations
  // Returns civilization by ID
  CCCivilization *GetCivilization(DWORD dwID){ ASSERT_VALID(this); CCCivilization *pCiv = NULL;
    VERIFY(m_mapCivilizations.Lookup(dwID, pCiv)); return pCiv; }
  // Returns number of civilizations
  DWORD GetCivilizationCount(){ ASSERT_VALID(this); return m_mapCivilizations.GetCount(); }
  // Returns civilization by index
  CCCivilization *GetCivilizationByIndex(DWORD dwIndex);

// Timeslices
  // Initializes the timeslice interval estimation
  // Params are the current timeslice number and current timeslice interval
  void InitTimesliceIntervalEstimation(DWORD dwTimeslice, DWORD dwTimesliceInterval);
  // Notifies map that new timeslice arrived from server
  // Params are the number of the timeslice and interval since the last timeslice arrived
  void NewTimesliceArrivedFromServer(DWORD dwTimeslice, DWORD dwTimesliceInterval);

private:
// Global lock for the whole map
  // this one must be used when adding or deleting unit from mapsquares
  CReadWriteLock m_lockGlobal;

// mapexes
  // list of all mapexes used in the map
  CCMapex *m_pMapexes;

// General unit types
  // list of all general unit types in the map
  CCGeneralUnitType *m_pGeneralUnitTypes;
  CMap<DWORD, DWORD, CCGeneralUnitType *, CCGeneralUnitType *&> m_mapGeneralUnitTypes;

// civilizations
  // list of all civilizations used in the map
  CCCivilization *m_pCivilizations;
  CMap<DWORD, DWORD, CCCivilization *, CCCivilization *&> m_mapCivilizations;

// Map squares
  // array of offsets to map file, where are map squares stored
  DWORD *m_aMapSquareOffsets;
  // position in the file, where begins the first one of the mapsquares
  DWORD m_dwMapSquareStart;

  // array of all mapsquares
  CCMapSquare **m_aMapSquares;

  // Archive file from which the mapsquares are loaded
  // (In real, it's an archive clone of the m_MapFile, cause we'll use it
  // simultaneously from other thread
  CArchiveFile m_cMapFile_MapSquareClone;

// Time
  // Current time for the client (in milliseconds) (this is computed from arrived packets from server)
  // this time should be synchronized with the server
  DWORD m_dwCurrentTime;

// Timeslice estimation (all intervals are in milliseconds)
  // Current timeslice , if == 0 -> still no init
  DWORD m_dwCurrentTimeslice;
  // Last known timeslice form the server
  DWORD m_dwLastKnownTimeslice;
  // Table of last timeslice intervals (used for estimating the next interval)
  DWORD m_aTimesliceIntervalHistory[MAP_TIMESLICE_HISTORY_COUNT];
  // Helper variable (last modified index in the table)
  DWORD m_dwLastTimesliceIntervalModified;
  // Current estimate for next timeslice interval
  DWORD m_dwCurrentEstimateTimesliceInterval;
  // Helper variable for computing the extimation of the timeslice interval
  DWORD m_dwEstimationTimesliceIntervalSum;

  CMutex m_lockTimesliceEstimation;

  // Observer class for receiving timer events
  class CTimesliceIntervalObserver : public CObserver
  {
    DECLARE_OBSERVER_MAP(CTimesliceIntervalObserver);

  public:
    CTimesliceIntervalObserver(){};
    ~CTimesliceIntervalObserver(){};

    void OnTimeTick(DWORD dwTime){ m_pMap->OnTimesliceIntervalTimeTick(dwTime); }

    CCMap *m_pMap;
  };
  friend CTimesliceIntervalObserver;
  // And the only instance of it
  CTimesliceIntervalObserver m_TimesliceIntervalObserver;
  
  // Reactions on the events from timeslice interval timer
  void OnTimesliceIntervalTimeTick(DWORD dwTime);
  // Helper function (computes new estimate from given new value fo timeslice interval
  void ComputeTimesliceIntervalEstimation(DWORD dwNewTimesliceInterval);



// Files
  // Lock for the TheMap archive
  CMutex m_lockArchive;
  // The map archive
  CDataArchive m_TheMapArchive;
  // Map file, must be stored here for caching operations (on mapsquares)
  CArchiveFile m_MapFile;

// data
  // Size of the map in mapsquares
  DWORD m_dwWidth;
  DWORD m_dwHeight;

  // Map name
  CString m_strName;
  // description
  CString m_strDescription;

  // Unique version number
  DWORD m_dwVersionNumber;

// Resources
  // Array of resource objects (resource types on the map)
  CCResource *m_aResources[RESOURCE_COUNT];

// Invisibility flags
  // Array of invisibility objects (represents flags of the invisibility field)
  CCInvisibility *m_aInvisibilities[INVISIBILITY_COUNT];
};

#endif