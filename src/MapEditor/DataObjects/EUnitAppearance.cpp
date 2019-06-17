// EUnitAppearance.cpp: implementation of the CEUnitAppearance class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "EUnitAppearance.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CEUnitAppearance, CObject)

CEUnitAppearance::CEUnitAppearance()
{
  int i;
  for(i = 0; i < 8; i++){
    m_aDirections[i] = NULL;
  }
}

CEUnitAppearance::~CEUnitAppearance()
{
  Delete();
}

#ifdef _DEBUG

void CEUnitAppearance::AssertValid() const
{
  CObject::AssertValid();
}

void CEUnitAppearance::Dump(CDumpContext &dc) const
{
  CObject::Dump(dc);
}

#endif

void CEUnitAppearance::Delete()
{
  // go through all animations
  int i;
  CEUnitAnimation *pAnim;
  for(i = 0; i < 8; i++){
    pAnim = m_aDirections[i];
    if(pAnim != NULL){
      // and delete it
      pAnim->Delete();
      delete pAnim;
    }
    m_aDirections[i] = NULL;
  }
}

BOOL CEUnitAppearance::Create(CEUnitAppearanceType *pType, DWORD dwColor, CDataArchive Archive)
{
  ASSERT(pType != NULL);

  m_dwColor = dwColor;

  int i;
  CEUnitAnimation *pAnim;
  for(i = 0; i < 8; i++){
    if(pType->m_aDirectionFileNames[i].IsEmpty()){
      m_aDirections[i] = NULL;
    }
    else{
      CArchiveFile AnimFile;
      pAnim = new CEUnitAnimation();
      AnimFile = Archive.CreateFile(pType->m_aDirectionFileNames[i]);
      pAnim->Create(AnimFile, m_dwColor);
      m_aDirections[i] = pAnim;
    }
  }

  return TRUE;
}

void CEUnitAppearance::SaveToType(CEUnitAppearanceType *pType)
{
  ASSERT(pType != NULL);

  int i;
  for(i = 0; i < 8; i++){
    if(m_aDirections[i] != NULL){
      pType->m_aDirectionFileNames[i] = m_aDirections[i]->GetFileName();
    }
    else{
      pType->m_aDirectionFileNames[i].Empty();
    }
  }
}

void CEUnitAppearance::RemoveGraphics(CDataArchive Archive)
{
  int i;
  CEUnitAnimation *pAnim;

  for(i = 0; i < 8;i ++){
    pAnim = m_aDirections[i];
    if(pAnim != NULL){
      pAnim->RemoveGraphics(Archive);
    }
  }
}

CEUnitAnimation * CEUnitAppearance::AddDirection(DWORD dwDirection)
{
  CEUnitAnimation *pAnim;
  
  if(m_aDirections[dwDirection] != NULL)
    return NULL;

  pAnim = new CEUnitAnimation();
  m_aDirections[dwDirection] = pAnim;
  pAnim->Create(m_dwColor);
  return pAnim;
}

void CEUnitAppearance::RemoveDirection(DWORD dwDirection, CDataArchive Archive)
{
  if(m_aDirections[dwDirection] == NULL) return;

  m_aDirections[dwDirection]->RemoveGraphics(Archive);
  m_aDirections[dwDirection]->Delete();
  delete m_aDirections[dwDirection];
  m_aDirections[dwDirection] = NULL;
}
