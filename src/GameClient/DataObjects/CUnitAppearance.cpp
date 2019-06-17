#include "stdafx.h"
#include "CUnitAppearance.h"

#include "CGeneralUnitAppearance.h"

IMPLEMENT_DYNAMIC(CCUnitAppearance, CObject);

// constructor
CCUnitAppearance::CCUnitAppearance()
{
  m_pNext = NULL;
  m_pGeneralAppearance = NULL;
  memset(m_aDirections, 0, sizeof(CCUnitAnimation *) * 8);
}

// destructor
CCUnitAppearance::~CCUnitAppearance()
{
  ASSERT(m_pGeneralAppearance == NULL);
}

// debug functions
#ifdef _DEBUG

void CCUnitAppearance::AssertValid() const
{
  CObject::AssertValid();
  ASSERT(m_pGeneralAppearance != NULL);
}

void CCUnitAppearance::Dump(CDumpContext &dc) const
{
  CObject::Dump(dc);
}

#endif

// Implementation -------------------------------------------

void CCUnitAppearance::Delete()
{
  // delete all directions
  // but we must be carefull, some directions can be the same
  CCUnitAnimation *pAnimation;

  int i,j;
  for(i = 0; i < 8; i++){
    pAnimation = m_aDirections[i];
    if(pAnimation == NULL) continue;
    for(j = i; j < 8; j++){
      if(m_aDirections[j] == pAnimation)
        m_aDirections[j] = NULL;
    }
    pAnimation->Delete();
    delete pAnimation;
  }

  m_pGeneralAppearance = NULL;
}