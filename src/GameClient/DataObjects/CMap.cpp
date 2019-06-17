#include "stdafx.h"
#include "CMap.h"

#include "Common\Map\Map.h"

#include "CMapexLibrary.h"
#include "CUnitTypeLibrary.h"

#include "..\Common\CommonExceptions.h"

#include "..\LoadException.h"

// Dynamic creation
IMPLEMENT_DYNAMIC(CCMap, CObject);

// Constructor
CCMap::CCMap()
{
  m_pMapexes = NULL;
  m_pGeneralUnitTypes = NULL;
  m_pCivilizations = NULL;

  m_dwWidth = 0;
  m_dwHeight = 0;

  m_dwVersionNumber = 0;

  m_aMapSquares = NULL;
  m_aMapSquareOffsets = NULL;

  m_dwCurrentTime = 0;

  m_dwCurrentTimeslice = 0;
  m_dwLastKnownTimeslice = 0;
  m_TimesliceIntervalObserver.m_pMap = this;

  memset(m_aResources, 0, sizeof(CCResource *) * RESOURCE_COUNT);
  memset(m_aInvisibilities, 0, sizeof(CCInvisibility *) * INVISIBILITY_COUNT);
}

// Destructor
CCMap::~CCMap()
{
  ASSERT(m_aMapSquares == NULL);
}

// debug functions
#ifdef _DEBUG

void CCMap::AssertValid() const
{
  CObject::AssertValid();

  ASSERT(m_aMapSquares != NULL);
}

void CCMap::Dump(CDumpContext &dc) const
{
  CObject::Dump(dc);
}

#endif

// Implementation ----------------------------------------------

// Creates the map from the map file (saved by the map editor)
// must know the paths to libraries
void CCMap::Create(CDataArchive TheMapArchive)
{
  // empty all lists
  m_pMapexes = NULL;
  m_pGeneralUnitTypes = NULL;
  m_pCivilizations = NULL;

  // store the archive
  m_TheMapArchive = TheMapArchive;
  // store the file
  m_MapFile = TheMapArchive.CreateFile("Map", CArchiveFile::modeRead | CFile::shareDenyWrite);

  // init caches
  CCMapex::InitCache();
  CCUnitAnimation::InitManager();
  CCUnitAnimation::InitCache();

  // load the version header
  {
    SFileVersionHeader sFileVersionHeader;

    if(m_MapFile.Read(&sFileVersionHeader, sizeof(sFileVersionHeader)) != sizeof(sFileVersionHeader))
      throw new CStringException("Map file corrupted.\n");

    {
      BYTE aFileID[16] = MAP_FILE_IDENTIFIER;
      if(memcmp(aFileID, sFileVersionHeader.m_aMapFileID, 16) != 0){
        throw new CStringException("Map file corrupted.\n");
      }
    }

    // Test if the version is OK
    if(sFileVersionHeader.m_dwFormatVersion < COMPATIBLE_MAP_FILE_VERSION)
      throw new CStringException("Incorrect version of map file.\n");
    if(sFileVersionHeader.m_dwCompatibleFormatVersion > CURRENT_MAP_FILE_VERSION)
      throw new CStringException("Incorrect version of map file.\n");
  }

  // load the header
  SMapHeader Header;

  LOAD_ASSERT ( m_MapFile.Read(&Header, sizeof(Header)) == sizeof ( Header ) );

  // copy data
  m_dwWidth = Header.m_dwWidth / MAPSQUARE_WIDTH;
  m_dwHeight = Header.m_dwHeight / MAPSQUARE_HEIGHT;

  m_dwVersionNumber = Header.m_dwMapVersion;

  m_strName = Header.m_pName;
  m_strDescription = Header.m_pDescription;

  // Skip Script sets
  {
    m_MapFile.Seek(Header.m_dwScriptSetCount * sizeof(SScriptSetHeader), CFile::current);
  }

// Load mapexes
  {
    DWORD dwLibrary;
    CCMapexLibrary *pMapexLibrary;
    SMapexLibraryNode MapexLibraryNode;
    CDataArchive Archive, cOriginalArchive;

    // Loop through all libraries in the map
    for(dwLibrary = 0; dwLibrary < Header.m_dwMapexLibraryCount; dwLibrary ++){
      // first load the library node
      LOAD_ASSERT ( m_MapFile.Read(&MapexLibraryNode, sizeof(MapexLibraryNode)) == sizeof ( MapexLibraryNode ) );

      // get the mapex library file name
      // must cat the path to mapex libraries with its file name
      // Create the library archive
      cOriginalArchive = TheMapArchive.CreateArchive(MapexLibraryNode.m_pFileName);
	  // Clone the archive, cause mapexes will be loaded by other thread (cache)
	  Archive = cOriginalArchive.CreateArchiveClone ();
	  cOriginalArchive.Delete ();

      // Allocate the library
      pMapexLibrary = new CCMapexLibrary();

      // Create the library
      // This will also add all mapexes in the library to the map (uses AddMapex method)
      pMapexLibrary->Create(Archive, this);
      
      // all was done -> delete the library, we won't need it anymore
      pMapexLibrary->Delete();
      delete pMapexLibrary;

      // delete the archive
      Archive.Delete();
    }
  }

// Load civilizations
  {
    DWORD dwCivilization;
    CCCivilization *pCivilization;
    m_mapCivilizations.InitHashTable ( 53 );

    // Loop through all civilizations in the map
    for(dwCivilization = 0; dwCivilization < Header.m_dwCivilizationsCount; dwCivilization ++){
      // allocate new civilization
      pCivilization = new CCCivilization();

      // add it to our list
      pCivilization->m_pMapNext = m_pCivilizations;
      m_pCivilizations = pCivilization;

      // load it from the file
      pCivilization->Create ( m_MapFile, dwCivilization );
      m_mapCivilizations.SetAt ( pCivilization->GetID (), pCivilization );
    }
  }

// Load unit types
  {
    DWORD dwUnitType;
    CCUnitTypeLibrary *pUnitTypeLibrary;
    SUnitTypeLibraryNode UnitTypeLibraryNode;
    CDataArchive Archive, cOriginalArchive;
    m_mapGeneralUnitTypes.InitHashTable(101);

    // Loop through all unit types in the map
    for(dwUnitType = 0; dwUnitType < Header.m_dwUnitTypeLibraryCount; dwUnitType ++){
      // load the library node
      LOAD_ASSERT ( m_MapFile.Read(&UnitTypeLibraryNode, sizeof(UnitTypeLibraryNode)) == sizeof ( UnitTypeLibraryNode ) );

      // Get the file name
      // UnitType Libraries path + \ + filename

      // Create the archive
      cOriginalArchive = TheMapArchive.CreateArchive(UnitTypeLibraryNode.m_pFileName);
	  // Duplicate it, case we'll use it from another thread simultaneously
	  // with other threads using the main archive
	  Archive = cOriginalArchive.CreateArchiveClone ();
	  cOriginalArchive.Delete ();

      // allocate new unit type library
      pUnitTypeLibrary = new CCUnitTypeLibrary();

      // Create the library
      pUnitTypeLibrary->Create(Archive, this);

      // all done -> delete the library
      pUnitTypeLibrary->Delete();
      delete pUnitTypeLibrary;

      // delete archive
      Archive.Delete();
    }
  }

  // create the mapsquare array
  m_aMapSquares = (CCMapSquare **)(new BYTE[sizeof(CCMapSquare *) * m_dwWidth * m_dwHeight]);
  memset(m_aMapSquares, 0, sizeof(CCMapSquare *) * m_dwWidth * m_dwHeight);

// Load map squares
  {
    // Note that no mapsquares will be really loaded here -> they are cached
    // but we will read the table of offsets to help finding the mapsquare in the file

    // allocate the offsets array
    m_aMapSquareOffsets = new DWORD[m_dwWidth * m_dwHeight];

    // load it from file
    LOAD_ASSERT ( m_MapFile.Read ( m_aMapSquareOffsets, sizeof ( DWORD ) * m_dwWidth * m_dwHeight) ==
		sizeof ( DWORD ) * m_dwWidth * m_dwHeight );

    // store current position in the file
    // it's the offset of the begining of the mapsquares
    m_dwMapSquareStart = m_MapFile.GetPosition ();

	// Create the clone archive and open the map file again there
	// thus create the mapsquare clone of the MapFile
	CDataArchive cClone = m_MapFile.GetDataArchive().CreateArchiveClone ();
	m_cMapFile_MapSquareClone = m_MapFile.CreateFileClone ();
	// and adopt the file
	cClone.AdoptFile ( m_cMapFile_MapSquareClone );
  }

// Create all unit types
  {
    CCCivilization *pCivilization;
    CCGeneralUnitType *pGeneralUnitType;

    for(pGeneralUnitType = m_pGeneralUnitTypes; pGeneralUnitType != NULL; pGeneralUnitType = pGeneralUnitType->m_pMapNext){
      for(pCivilization = m_pCivilizations; pCivilization != NULL; pCivilization = pCivilization->m_pMapNext){
        pCivilization->CreateUnitType(pGeneralUnitType);
      }
    }
  }

// Skip masquares offset table
  {
    // It means that we must read the mapsquares and skip them
    DWORD dwX, dwY;
    SMapSquareHeader SquareHeader;
    for(dwY = 0; dwY < Header.m_dwHeight / MAPSQUARE_HEIGHT; dwY++){
      for(dwX = 0; dwX < Header.m_dwWidth / MAPSQUARE_WIDTH; dwX++){
        LOAD_ASSERT ( m_MapFile.Read ( &SquareHeader, sizeof ( SquareHeader ) ) == sizeof ( SquareHeader ) );

        m_MapFile.Seek(SquareHeader.m_dwBL1MapexesNum * sizeof(SMapexInstanceHeader), CFile::current);
        m_MapFile.Seek(SquareHeader.m_dwBL2MapexesNum * sizeof(SMapexInstanceHeader), CFile::current);
        m_MapFile.Seek(SquareHeader.m_dwBL3MapexesNum * sizeof(SMapexInstanceHeader), CFile::current);
        m_MapFile.Seek(SquareHeader.m_dwULMapexesNum * sizeof(SULMapexInstanceHeader), CFile::current);
      }
    }
  }

// Skip unit instances
  {
    m_MapFile.Seek(sizeof(SUnitHeader) * Header.m_dwUnitCount, CFile::current);
  }

// Load resource types
  {
    CCResource *pResource;
    DWORD i;

    for(i = 0; i < RESOURCE_COUNT; i++){
      pResource = new CCResource();
      pResource->Create(m_MapFile);
      m_aResources[i] = pResource;
    }
  }

// Load invisibilities
  {  
    CCInvisibility *pInvisibility;
    DWORD i;

    for(i = 0; i < INVISIBILITY_COUNT; i++){
      pInvisibility = new CCInvisibility();
      pInvisibility->Create(m_MapFile);
      m_aInvisibilities[i] = pInvisibility;
    }
  }

// Skip find path graphs
  {
	  m_MapFile.Seek ( sizeof ( SFindPathGraphHeader ) * Header.m_dwFindPathGraphCount, CFile::current );
  }

}

// deletes the map
void CCMap::Delete()
{
  // clear all lists

  // delete all mapexes
  {
    CCMapex *pMapex, *pDel;

    pMapex = m_pMapexes;
    while(pMapex != NULL){
      pDel = pMapex;
      pMapex = pMapex->m_pMapNext;

      pDel->Delete();
      delete pDel;
    }

    m_pMapexes = NULL;
  }

  // delete all civilizations
  {
    CCCivilization *pCivilization, *pDel;

    pCivilization = m_pCivilizations;
    while(pCivilization != NULL){
      pDel = pCivilization;
      pCivilization = pCivilization->m_pMapNext;

      pDel->Delete();
      delete pDel;
    }

    m_pCivilizations = NULL;
  }

  // delete all general unit types
  {
    CCGeneralUnitType *pUnitType, *pDel;

    pUnitType = m_pGeneralUnitTypes;
    while(pUnitType != NULL){
      pDel = pUnitType;
      pUnitType = pUnitType->m_pMapNext;

      pDel->Delete();
      delete pDel;
    }

    m_pGeneralUnitTypes = NULL;
  }

  // delete all mapsquares
  if ( m_aMapSquares != NULL )
  {
    DWORD dwX, dwY;
    for(dwY = 0; dwY < m_dwHeight; dwY++){
      for(dwX = 0; dwX < m_dwWidth; dwX++){
        if(m_aMapSquares[dwX + dwY * m_dwWidth] != NULL){
          CCMapSquare *pMapSquare = m_aMapSquares[dwX + dwY * m_dwWidth];
          pMapSquare->Delete();
          delete pMapSquare;
        }
      }
    }

      // delete map squares array
    delete m_aMapSquares;
    m_aMapSquares = NULL;
  }

  // delete the map square offsets array
  if(m_aMapSquareOffsets != NULL){
    delete m_aMapSquareOffsets;
    m_aMapSquareOffsets = NULL;
  }

  // Delete resource types
  {
    DWORD i;
    for(i = 0; i < RESOURCE_COUNT; i++){
      if(m_aResources[i] == NULL) continue;
      m_aResources[i]->Delete();
      delete m_aResources[i];
      m_aResources[i] = 0;
    }
  }

  // Delete visibilities
  {
    DWORD i;
    for(i = 0; i < INVISIBILITY_COUNT; i++){
      if(m_aInvisibilities[i] == NULL) continue;
      m_aInvisibilities[i]->Delete();
      delete m_aInvisibilities[i];
      m_aInvisibilities[i] = NULL;
    }
  }

  // close caches
  CCUnitAnimation::CloseCache();
  CCUnitAnimation::CloseManager();
  CCMapex::CloseCache();
}


// Mapex functions ---------

// Add mapex to the map list
void CCMap::AddMapex(CCMapex *pMapex)
{
  ASSERT(pMapex != NULL);

  // add it to the head of the list
  pMapex->m_pMapNext = m_pMapexes;
  m_pMapexes = pMapex;

  // OK all was done
}

// Finds mapex by its ID
CCMapex *CCMap::GetMapex(DWORD dwID)
{
  ASSERT_VALID(this);

  // go through our list
  CCMapex *pMapex;
  pMapex = m_pMapexes;

  while(pMapex != NULL){
    if(pMapex->GetID() == dwID)
      return pMapex; // found
    pMapex = pMapex->m_pMapNext;
  }

  // not found
  ASSERT(FALSE);
  return NULL;
}


// Map square functions -------------------------------

// creates new map square fo rgiven coords
CCMapSquare *CCMap::CreateMapSquare(DWORD dwX, DWORD dwY)
{
  ASSERT_VALID(this);
  ASSERT(dwX < m_dwWidth); ASSERT(dwY < m_dwHeight);

  CCMapSquare *pMapSquare;

  // create new map square
  pMapSquare = new CCMapSquare();

  // find it in the file
  DWORD dwOffset = m_aMapSquareOffsets[dwX + dwY * m_dwWidth];

  // seek to that position
  m_MapFile.Seek(dwOffset + m_dwMapSquareStart, CFile::begin);

  // load the mapsquare
  pMapSquare->Create(m_MapFile, this);

  // add it to our structures

  return pMapSquare;
}


// returns the map square for given coords
CCMapSquare *CCMap::GetMapSquare(DWORD dwXPos, DWORD dwYPos, BOOL bWriterLocked)
{
  ASSERT_VALID(this);
  ASSERT(dwXPos < m_dwWidth); ASSERT(dwYPos < m_dwHeight);

  DWORD dwOff = dwXPos + dwYPos * m_dwWidth;
  CCMapSquare *pMapSquare = m_aMapSquares[dwOff];

  if(pMapSquare == NULL){
    if(!bWriterLocked){
      // unlock the reader lock
      m_lockGlobal.ReaderUnlock();
      // and lock it as a writer lock
      VERIFY(m_lockGlobal.WriterLock());
    }
    // test if its still free
    if(m_aMapSquares[dwOff] != NULL){
      pMapSquare = m_aMapSquares[dwOff];
    }
    else{
      m_aMapSquares[dwOff] = CreateMapSquare(dwXPos, dwYPos);
      pMapSquare = m_aMapSquares[dwOff];
    }

    if(!bWriterLocked){
      // change the lock to reader lock
      m_lockGlobal.WriterUnlock();
      VERIFY(m_lockGlobal.ReaderLock());
    }
  }

  return pMapSquare;
}


// General unit type functions ------------------------

// Add general unit type to the list
void CCMap::AddGeneralUnitType(CCGeneralUnitType *pUnitType)
{
  ASSERT(pUnitType != NULL);

  // add it to the head of the list
  pUnitType->m_pMapNext = m_pGeneralUnitTypes;
  m_pGeneralUnitTypes = pUnitType;

  m_mapGeneralUnitTypes.SetAt(pUnitType->GetID(), pUnitType);

  // OK
}


// Timeslice functions ---------------------------------

BEGIN_OBSERVER_MAP(CCMap::CTimesliceIntervalObserver, CObserver)
  BEGIN_TIMER()
    ON_TIMETICK()
  END_TIMER()
END_OBSERVER_MAP(CCMap::CTimesliceIntervalObserver, CObserver)

// Initializes the timeslice interval estimation
void CCMap::InitTimesliceIntervalEstimation(DWORD dwTimeslice, DWORD dwTimesliceInterval)
{
  ASSERT(dwTimeslice > 0);

  // Fill our history table with known value of interval
  {
    DWORD i;
    for(i = 0; i < MAP_TIMESLICE_HISTORY_COUNT; i++){
      m_aTimesliceIntervalHistory[i] = dwTimesliceInterval;
    }
  }

  // Init out estimation
  m_dwLastTimesliceIntervalModified = 0;
  m_dwCurrentEstimateTimesliceInterval = dwTimesliceInterval;
  m_dwLastKnownTimeslice = dwTimeslice;
  m_dwEstimationTimesliceIntervalSum = dwTimesliceInterval * MAP_TIMESLICE_HISTORY_COUNT;

  // Connect out observer to the timer
  ASSERT(g_pTimer != NULL);
  g_pTimer->Connect(&m_TimesliceIntervalObserver, m_dwCurrentEstimateTimesliceInterval);

  // Copy the timeslice
  m_dwCurrentTimeslice = dwTimeslice;
}

// Computes new estimation from new timeslice interval
void CCMap::ComputeTimesliceIntervalEstimation(DWORD dwNewTimesliceInterval)
{
  // subtract the last interval in the table
  m_dwEstimationTimesliceIntervalSum -= m_aTimesliceIntervalHistory[m_dwLastTimesliceIntervalModified];

  // Add new interval to the table and to the sum
  m_aTimesliceIntervalHistory[m_dwLastTimesliceIntervalModified] = dwNewTimesliceInterval;
  m_dwEstimationTimesliceIntervalSum += dwNewTimesliceInterval;

  // Move to the next position on history table
  m_dwLastTimesliceIntervalModified = (m_dwLastTimesliceIntervalModified + 1) % MAP_TIMESLICE_HISTORY_COUNT;

  // Compute new estimation
  m_dwCurrentEstimateTimesliceInterval = m_dwEstimationTimesliceIntervalSum / MAP_TIMESLICE_HISTORY_COUNT;

//  TRACE("Timeslice - interval %d, new estimation %d\n", dwNewTimesliceInterval, m_dwCurrentEstimateTimesliceInterval);
}

extern DWORD g_dwInterpolationTime;
// Reaction on the time tick from the timeslice interval timer
void CCMap::OnTimesliceIntervalTimeTick(DWORD dwTime)
{
  VERIFY(m_lockTimesliceEstimation.Lock());

  // Increase the timeslice
  m_dwCurrentTimeslice++;

//  TRACE("Timeslice %d - real %d, interpolation %d\n", m_dwCurrentTimeslice,
//    m_dwCurrentEstimateTimesliceInterval, g_dwInterpolationTime);
  g_dwInterpolationTime = 0;


  VERIFY(m_lockTimesliceEstimation.Unlock());
  // Just restart the timer with the same interval (it means, do nothing)
}

// Notifies map that new timeslice arrived from server
// Params are the number of the timeslice and interval since the last timeslice arrived
void CCMap::NewTimesliceArrivedFromServer(DWORD dwTimeslice, DWORD dwTimesliceInterval)
{
  // If the estimation didn't started yet -> do nothing
  if(m_dwCurrentTimeslice == 0) return;

  VERIFY(m_lockTimesliceEstimation.Lock());

  m_dwLastKnownTimeslice = dwTimeslice;

  if(dwTimeslice > m_dwCurrentTimeslice){
    // If the new timeslice is greater
    // Just go to that timeslice
    m_dwCurrentTimeslice = dwTimeslice;
    // Recompute estimations
    ComputeTimesliceIntervalEstimation(dwTimesliceInterval);
    // And restart the timer with new estimation
    g_pTimer->SetElapseTime(&m_TimesliceIntervalObserver, m_dwCurrentEstimateTimesliceInterval);

//    TRACE("Timeslice %d - real %d, interpolation %d\n", m_dwCurrentTimeslice,
//      m_dwCurrentEstimateTimesliceInterval, g_dwInterpolationTime);
    g_dwInterpolationTime = 0;
  }
  else{
    // It's current or less
    // Recompute estimations
    ComputeTimesliceIntervalEstimation(dwTimesliceInterval);
    // And restart timer with some special value
    g_pTimer->SetElapseTime(&m_TimesliceIntervalObserver,
      ((m_dwCurrentTimeslice - dwTimeslice) + 1) * m_dwCurrentEstimateTimesliceInterval);
  }

  VERIFY(m_lockTimesliceEstimation.Unlock());
}

CCCivilization *CCMap::GetCivilizationByIndex(DWORD dwIndex)
{
  ASSERT_VALID(this);
  CCCivilization *pCiv = NULL;
  DWORD dwID = 0;

  POSITION pos = m_mapCivilizations.GetStartPosition();
  
  while(pos != NULL){
    m_mapCivilizations.GetNextAssoc(pos, dwID, pCiv);
    if(pCiv->GetIndex() == dwIndex) return pCiv;
  }
  return NULL;
};