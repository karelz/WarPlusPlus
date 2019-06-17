#include "stdafx.h"
#include "CMapexLibrary.h"

#include "Common\Map\Map.h"

#include "..\Common\CommonExceptions.h"

#include "CMap.h"
#include "..\LoadException.h"

IMPLEMENT_DYNAMIC(CCMapexLibrary, CObject);

// constructor
CCMapexLibrary::CCMapexLibrary()
{
}

// destructor
CCMapexLibrary::~CCMapexLibrary()
{
}


// debug functions
#ifdef _DEBUG

void CCMapexLibrary::AssertValid() const
{
  CObject::AssertValid();
}

void CCMapexLibrary::Dump(CDumpContext &dc) const
{
  CObject::Dump(dc);
}

#endif


// Implementation --------------------------------------------

// Loads the library from the archive.
// Allocates and loads all mapexes in the library.
// Add all newly created mapexes to the map list.
void CCMapexLibrary::Create(CDataArchive archive, CCMap *pMap)
{
  ASSERT(pMap != NULL);

  // load the library header
  SMapexLibraryHeader Header;
  CArchiveFile HeaderFile = archive.CreateFile("Library.Header");

  // load the version header
  {
    SMapexFileVersionHeader sFileVersionHeader;

    LOAD_ASSERT ( HeaderFile.Read(&sFileVersionHeader, sizeof(sFileVersionHeader))
		== sizeof(sFileVersionHeader) );

    {
      BYTE aFileID[16] = MAPEX_FILE_IDENTIFIER;
      LOAD_ASSERT ( memcmp(aFileID, sFileVersionHeader.m_aFileID, 16) == 0 );
    }

    // Test if the version is OK
    LOAD_ASSERT ( sFileVersionHeader.m_dwFormatVersion >= COMPATIBLE_MAPEX_FILE_VERSION );
    LOAD_ASSERT ( sFileVersionHeader.m_dwCompatibleFormatVersion <= CURRENT_MAP_FILE_VERSION );
  }

  LOAD_ASSERT ( HeaderFile.Read ( &Header, sizeof ( Header ) ) == sizeof ( Header ) );

  // go through all the mapexes and load them
  DWORD dwMapex;
  CCMapex *pMapex;
  char aMapexFileName[31];
  aMapexFileName[30] = 0;

  for(dwMapex = 0; dwMapex < Header.m_dwMapexCount; dwMapex ++){
    // allocate the object
    pMapex = new CCMapex();

    // load the mapex file name
    LOAD_ASSERT ( HeaderFile.Read(aMapexFileName, 30) == 30);

    // load the mapex
    pMapex->Create(archive.CreateFile(aMapexFileName));

    // add it to the map
    pMap->AddMapex(pMapex);
  }
}

void CCMapexLibrary::Delete()
{
  // do nothing
}