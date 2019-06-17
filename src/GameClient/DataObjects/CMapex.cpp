
#include "stdafx.h"
#include "CMapex.h"

#include "Common\Map\Map.h"
#include "CMap.h"
#include "..\GameClientGlobal.h"
#include "..\LoadException.h"

// constructor
CCMapex::CCMapex()
{
}

// destructor
CCMapex::~CCMapex()
{
}

// Implementation ----------------------------------------

CMapexCache *CCMapex::m_pMapexCache = NULL;

// initializes the mapex cache
void CCMapex::InitCache()
{
  ASSERT(m_pMapexCache == NULL);

  // create new cache object
  m_pMapexCache = new CMapexCache();
}

// closes the mapex cache
void CCMapex::CloseCache()
{
  // delete the cache object
  if(m_pMapexCache != NULL){
    delete m_pMapexCache;
    m_pMapexCache = NULL;
  }
}

// loads mapex from the file
// doesn't load eny graphics, just stores the location
void CCMapex::Create(CArchiveFile MapexFile)
{
	SMapexHeader Header;

	// load the header from the file
	LOAD_ASSERT ( MapexFile.Read ( &Header, sizeof ( Header ) ) == sizeof ( Header ) );

	// copy data to our structures
	m_dwID = Header.m_dwID;
	m_dwXSize = Header.m_dwXSize;
	m_dwYSize = Header.m_dwYSize;

	// get the filename of the graphics
	CString strFileName;
	strFileName = Header.m_pGraphicsFileName;
	// create the file for the graphics
	m_fileGraphics = MapexFile.GetDataArchive ().CreateFile ( strFileName );

	// load the land type array
	m_pLandTypes = new BYTE[m_dwXSize * m_dwYSize];
	LOAD_ASSERT ( MapexFile.Read ( m_pLandTypes, m_dwXSize * m_dwYSize ) ==
		m_dwXSize * m_dwYSize );

	// now add us to the cache
	m_pMapexCache->InsertObject ( this );
}

// deletes the mapex object
void CCMapex::Delete()
{
  // remove us from the cache
  m_pMapexCache->RemoveObject(this);

	// delete the land type array
	if(m_pLandTypes != NULL){
		delete m_pLandTypes;
		m_pLandTypes = NULL;
	}

	// close the graphics file
	m_fileGraphics.Close();

	// delete all loaded graphics
	m_Animation.Delete();
}

// loads the mapex graphics (this is called by the cache)
void CCMapex::Load()
{
	// load the graphics
  if(!m_Animation.Create(m_fileGraphics)){
    ASSERT(FALSE);
  }
}

// unloads the mapex graphics (this is called by the cache)
void CCMapex::Unload()
{
	// delete the graphics
	m_Animation.Delete();
}

// returns the size in bytes (called by the cache)
DWORD CCMapex::GetSize()
{
  if(m_Animation.GetSize().cx == 0) return m_dwXSize * m_dwYSize * MAPCELL_WIDTH * MAPCELL_HEIGHT * 3 + sizeof(this);
  else{
    return sizeof(this) + m_Animation.GetSize().cx * m_Animation.GetSize().cy * m_Animation.GetFrameNum() * 3;
  }
}
