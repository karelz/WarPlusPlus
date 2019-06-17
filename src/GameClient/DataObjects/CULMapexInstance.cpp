// CULMapexInstance.cpp: implementation of the CCULMapexInstance class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CULMapexInstance.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#include "CMap.h"
#include "..\LoadException.h"

// Constructor
CCULMapexInstance::CCULMapexInstance()
{
  m_pNext = NULL;
  m_pMapex = NULL;
}

// Destructor
CCULMapexInstance::~CCULMapexInstance()
{
  Delete();
}

// Implementation ------------------------------------

// Creates the object from file (need a map object)
void CCULMapexInstance::Create(CArchiveFile MapFile, CCMap *pMap)
{
  SULMapexInstanceHeader Header;

  // load the header
  LOAD_ASSERT ( MapFile.Read ( &Header, sizeof ( Header ) ) == sizeof ( Header ) );

  // copy the data
  m_dwXPosition = Header.m_dwX;
  m_dwYPosition = Header.m_dwY;
  m_dwZPosition = Header.m_dwZ;

  // try to find the mapex in the map
  m_pMapex = pMap->GetMapex(Header.m_dwMapexID);
  LOAD_ASSERT ( m_pMapex != NULL );
}

// Deletes the object
void CCULMapexInstance::Delete()
{
  m_pNext = NULL;
  m_pMapex = NULL;
}


// Drawing functions --------------------------

// Draws the mapex
void CCULMapexInstance::Draw(DWORD dwXPos, DWORD dwYPos, CDDrawSurface *pSurface)
{
  // Get the mapex
  CCMapex *pMapex = GetMapex();

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