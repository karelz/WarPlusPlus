#include "stdafx.h"
#include "CGeneralUnitAppearance.h"

#include "Common\Map\Map.h"
#include "..\GameClientGlobal.h"
#include "CMap.h"

#include "CUnitAppearance.h"
#include "CCivilization.h"
#include "..\LoadException.h"

IMPLEMENT_DYNAMIC(CCGeneralUnitAppearance, CObject);

// constructor
CCGeneralUnitAppearance::CCGeneralUnitAppearance()
{
  m_dwID = 0;
  m_pNext = NULL;
}

// destructor
CCGeneralUnitAppearance::~CCGeneralUnitAppearance()
{
  ASSERT(m_dwID == 0);
  ASSERT(m_pNext == NULL);
}


// Debug functions
#ifdef _DEBUG

void CCGeneralUnitAppearance::AssertValid() const
{
  CObject::AssertValid();
}

void CCGeneralUnitAppearance::Dump(CDumpContext &dc) const
{
  CObject::Dump(dc);
}

#endif


// Implementation ------------------------------------------------------

void CCGeneralUnitAppearance::Create(CArchiveFile UnitTypeFile, CDataArchive cArchive_GraphicsClone)
{
  SAppearance Header;

  // load the header
  LOAD_ASSERT ( UnitTypeFile.Read(&Header, sizeof(Header)) == sizeof ( Header ) );

  // copy the data
  m_dwID = Header.m_dwID;

  {
    int i;
    for(i = 0; i < 8; i++){
      m_aDirections[i] = Header.m_szDirections[i];
    }
  }

  // store the archive
  m_Archive = UnitTypeFile.GetDataArchive();
  // Remember the clone archive
  m_cArchive_GraphicsClone = cArchive_GraphicsClone;
}

void CCGeneralUnitAppearance::Delete()
{
  m_dwID = 0;
}

// Creates an instance for this appearance type
CCUnitAppearance * CCGeneralUnitAppearance::CreateInstance(CCCivilization *pCivilization)
{
  CCUnitAppearance * pAppearance;

  // first allocate the object
  pAppearance = new CCUnitAppearance();
  pAppearance->m_pGeneralAppearance = this;

  // go through all directions and create that ones that includes some data
  int i;
  for(i = 0; i < 8; i++){
    if(!m_aDirections[i].IsEmpty()){
      // create the animation instance for this direction
      pAppearance->m_aDirections[i] = new CCUnitAnimation();

	  CArchiveFile cAnimationFile = m_Archive.CreateFile ( m_aDirections[i] );
	  // Adopt the file to the graphics clone
	  // We can't create the file in that archive directly, cause
	  // meanwhile some read opreation can be in progress on that archive
	  // from another thread
	  // But the adoption is multithread safe
	  m_cArchive_GraphicsClone.AdoptFile ( cAnimationFile );

	  // And now create the appearance
	  // The creation do NOT read anything from the archive
	  // It just copy the file envelope to its structures
	  // thus it's safe
      pAppearance->m_aDirections[i]->Create ( cAnimationFile, pCivilization->GetColor () );
    }
  }

  CCUnitAnimation *pAnim = NULL;

  // now go through all directions again and fill the empty ones with copies
  // assume that the first direction is always filled
  for(i = 0; i < 8; i++){
    if(pAppearance->m_aDirections[i] == NULL){
      pAppearance->m_aDirections[i] = pAnim;
    }
    else{
      pAnim = pAppearance->m_aDirections[i];
    }
  }

  return pAppearance;
}