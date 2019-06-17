// EUnitAppearanceType.cpp: implementation of the CEUnitAppearanceType class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "EUnitAppearanceType.h"

#include "..\MapFormats.h"

#include "EUnitAppearance.h"
#include "EUnitType.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CEUnitAppearanceType, CObject)

CEUnitAppearanceType::CEUnitAppearanceType()
{
  m_pDefaultAppearance = NULL;
  int i;
  for(i = 0; i < Cache_Size; i++){
    m_pInstance[i] = NULL;
  }
}

CEUnitAppearanceType::~CEUnitAppearanceType()
{
  Delete();
}

#ifdef _DEBUG

void CEUnitAppearanceType::AssertValid() const
{
  CObject::AssertValid();
}

void CEUnitAppearanceType::Dump(CDumpContext &dc) const
{
  CObject::Dump(dc);
}

#endif


BOOL CEUnitAppearanceType::Create(DWORD dwID, CEUnitType *pUnitType)
{
  m_dwID = dwID;

  m_pUnitType = pUnitType;

  // empty the strings
  {
    int i;
    for(i = 0; i < 8; i++){
      m_aDirectionFileNames[i].Empty();
    }
  }
  return TRUE;
}

BOOL CEUnitAppearanceType::Create(CArchiveFile file, CEUnitType *pUnitType)
{
  if(g_dwUnitTypeFileVersion < MAP_FILE_VERSION(1, 0)){
    SAppearance_Old h;

    m_pUnitType = pUnitType;

    // let's read the file
    file.Read(&h, sizeof(h));
    // copy some info
    m_dwID = h.m_dwID;
    m_strName = h.m_szName;

    // now copy the direction strings
    {
      int i;
      for(i = 0; i < 8; i++){
        m_aDirectionFileNames[i] = h.m_szDirections[i];
      }
    }
  }
  else{
    SAppearance h;

    m_pUnitType = pUnitType;

    // let's read the file
    file.Read(&h, sizeof(h));
    // copy some info
    m_dwID = h.m_dwID;
    m_strName = h.m_szName;

    // now copy the direction strings
    {
      int i;
      for(i = 0; i < 8; i++){
        m_aDirectionFileNames[i] = h.m_szDirections[i];
      }
    }
  }

  return TRUE;
}


void CEUnitAppearanceType::Delete()
{
  // just empty strings (it not necesary, but it frees some memory .. should be done here)
  int i;

  for(i = 0;i < 8; i++){
    m_aDirectionFileNames[i].Empty();
  }

  ClearCache();
}

void CEUnitAppearanceType::SaveToFile(CArchiveFile file)
{
  SAppearance h;

  // first set all NULL
  memset(&h, 0, sizeof(h));
  // now copy the data to the structure
  h.m_dwID = m_dwID;
  strncpy(h.m_szName, m_strName, 30);

  {
    int i;
    for(i = 0;i < 8; i++){
      strncpy(h.m_szDirections[i], m_aDirectionFileNames[i], 50);
    }
  }

  // OK write it to file
  file.Write(&h, sizeof(h));
}

CEUnitAppearance * CEUnitAppearanceType::GetDefaultInstance()
{
  // if no instance -> create one
  if(m_pDefaultAppearance == NULL){
//    if(!m_Archive.IsOpened()) return NULL;
    m_pDefaultAppearance = new CEUnitAppearance();
    if(!m_pDefaultAppearance->Create(this, UNITAPPEARANCETYPE_DEFAULTCOLOR, m_Archive)){
      delete m_pDefaultAppearance;
      m_pDefaultAppearance = NULL;
      return NULL;
    }
  }

  return m_pDefaultAppearance;
}

void CEUnitAppearanceType::ClearCache()
{
  if(m_pDefaultAppearance != NULL){
    m_pDefaultAppearance->Delete();
    delete m_pDefaultAppearance;
    m_pDefaultAppearance = NULL;
  }
  int i;
  for(i = 0; i < Cache_Size; i++){
    if(m_pInstance[i] != NULL){
      m_pInstance[i]->Delete();
      delete m_pInstance[i];
      m_pInstance[i] = NULL;
    }
  }
}

CEUnitAppearance * CEUnitAppearanceType::CreateInstance(DWORD dwColor)
{
  static int nLastUsed = 0;
  int i;
  for(i = 0; i < Cache_Size; i++){
    if((m_pInstance[i] != NULL) && (m_pInstance[i]->GetColor() == dwColor)){
      return m_pInstance[i]; // do nothing it's already there
    }
  }
  for(i = 0; i < Cache_Size; i++){
    if(m_pInstance[i] == NULL){
      nLastUsed = i;
      goto Found;
    }
  }
  nLastUsed++; if(nLastUsed >= Cache_Size) nLastUsed = 0;
Found:
  ;

  if(m_pInstance[nLastUsed] != NULL){
    m_pInstance[nLastUsed]->Delete();
    delete m_pInstance[nLastUsed];
    m_pInstance[nLastUsed] = NULL;
  }

  m_pInstance[nLastUsed] = new CEUnitAppearance();
  m_pInstance[nLastUsed]->Create(this, dwColor, m_Archive);

  return m_pInstance[nLastUsed];
}

BOOL CEUnitAppearanceType::CheckValid()
{
  if(m_aDirectionFileNames[0].IsEmpty()){
    CString str;
    str = "Vzhled '";
    str += m_strName;
    str += "' jednotky '";
    str += m_pUnitType->GetName();
    str += "' musí mít vyplnìn smìr N.";
    AfxMessageBox(str);
    return FALSE;
  }
  return TRUE;
}