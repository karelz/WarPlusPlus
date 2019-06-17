#include "stdafx.h"
#include "CUnitTypeLibrary.h"

#include "..\Common\CommonExceptions.h"

#include "CMap.h"
#include "CGeneralUnitType.h"
#include "..\LoadException.h"

IMPLEMENT_DYNAMIC(CCUnitTypeLibrary, CObject);

// constructor
CCUnitTypeLibrary::CCUnitTypeLibrary()
{
}

// destructor
CCUnitTypeLibrary::~CCUnitTypeLibrary()
{
}

// debug functions
#ifdef _DEBUG

void CCUnitTypeLibrary::AssertValid() const
{
  CObject::AssertValid();
}

void CCUnitTypeLibrary::Dump(CDumpContext &dc) const
{
  CObject::Dump(dc);
}

#endif


// Implementation ------------------------------

// Creates the library from the archive
void CCUnitTypeLibrary::Create(CDataArchive Archive, CCMap *pMap)
{
  SUnitTypeLibraryHeader Header;
  CArchiveFile HeaderFile;
  CDataArchive cArchive_GraphicsClone;

  // Clone the given archive for graphics caches
  cArchive_GraphicsClone = Archive.CreateArchiveClone ();

  // load the header
  HeaderFile = Archive.CreateFile("Library.Header");

  // load the version header
  {
    SUnitTypeFileVersionHeader sFileVersionHeader;

    LOAD_ASSERT ( HeaderFile.Read(&sFileVersionHeader, sizeof(sFileVersionHeader)) == sizeof(sFileVersionHeader) );

    {
      BYTE aFileID[16] = UNITTYPE_FILE_IDENTIFIER;
      LOAD_ASSERT ( memcmp(aFileID, sFileVersionHeader.m_aFileID, 16) == 0 );
    }

    // Test if the version is OK
    LOAD_ASSERT ( sFileVersionHeader.m_dwFormatVersion >= COMPATIBLE_UNITTYPE_FILE_VERSION );
    LOAD_ASSERT ( sFileVersionHeader.m_dwCompatibleFormatVersion <= CURRENT_MAP_FILE_VERSION );
  }

  LOAD_ASSERT ( HeaderFile.Read ( &Header, sizeof ( Header ) ) == sizeof ( Header ) );

  // loop through all unit types
  DWORD dwUnitType;
  char pUnitTypeFileName[31];
  CCGeneralUnitType *pUnitType;

  for(dwUnitType = 0; dwUnitType < Header.m_dwUnitTypeCount; dwUnitType ++){
    // load the unit type filename
    LOAD_ASSERT ( HeaderFile.Read ( &pUnitTypeFileName, 30 ) == 30 );

    // allocate new unit type
    pUnitType = new CCGeneralUnitType();

    // load the unit type
    pUnitType->Create(Archive.CreateFile(pUnitTypeFileName), cArchive_GraphicsClone);

    // add it to the map
    pMap->AddGeneralUnitType(pUnitType);
  }
}

// Deletes the library
void CCUnitTypeLibrary::Delete()
{
}