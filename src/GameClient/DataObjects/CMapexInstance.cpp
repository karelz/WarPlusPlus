#include "stdafx.h"
#include "CMapexInstance.h"

#include "CMap.h"
#include "..\LoadException.h"

// Constructor
CCMapexInstance::CCMapexInstance()
{
  m_pNext = NULL;
  m_pMapex = NULL;
}

// Destructor
CCMapexInstance::~CCMapexInstance()
{
  Delete();
}

// Implementation ------------------------------------

// Creates the object from file (need a map object)
void CCMapexInstance::Create(CArchiveFile MapFile, CCMap *pMap)
{
  SMapexInstanceHeader Header;

  // load the header
  LOAD_ASSERT ( MapFile.Read ( &Header, sizeof ( Header ) ) == sizeof ( Header ) );

  // copy the data
  m_dwXPosition = Header.m_dwX;
  m_dwYPosition = Header.m_dwY;

  // try to find the mapex in the map
  m_pMapex = pMap->GetMapex(Header.m_dwMapexID);
  LOAD_ASSERT ( m_pMapex != NULL );
}

// Deletes the object
void CCMapexInstance::Delete()
{
  m_pNext = NULL;
  m_pMapex = NULL;
}


// Drawing functions --------------------------

// Draws the mapex
void CCMapexInstance::Draw(DWORD dwXPos, DWORD dwYPos, CDDrawSurface *pSurface)
{
  // Get the mapex
  CCMapex *pMapex = GetMapex();
  if(pMapex == NULL) return;

  // lock it into the memory
  pMapex->LoadAndLock();


  // first get the frame
  CDDrawSurface *pFrame = pMapex->GetGraphics()->GetFrame(0);

  // compute the dest rect
  CRect rcDest = pFrame->GetAllRect();
  rcDest.OffsetRect(((int)m_dwXPosition - (int)dwXPos) * MAPCELL_WIDTH,
    ((int)m_dwYPosition - (int)dwYPos) * MAPCELL_HEIGHT);

  // and draw it
  pSurface->Paste(rcDest.TopLeft(), pFrame);

  // unlock the mapex
  pMapex->Unlock();
}