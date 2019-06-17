// EUnitSkillType.cpp: implementation of the CEUnitSkillType class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\MapEditor.h"
#include "EUnitSkillType.h"

#include "..\MapFormats.h"
#include "EUnitType.h"

#include "..\UnitEditor\UnitLibrariesDlg.h"
#include "..\UnitEditor\UnitSkillTypeDlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CEUnitSkillType, CObject)

CEUnitSkillType::CEUnitSkillType()
{
  m_pData = NULL;
  m_dwDataSize = 0;
  m_pUnitType = NULL;
  m_bEnabled = TRUE;
}

CEUnitSkillType::~CEUnitSkillType()
{
  ASSERT(m_pData == NULL);
}

#ifdef _DEBUG

void CEUnitSkillType::AssertValid() const
{
  CObject::AssertValid();
}

void CEUnitSkillType::Dump(CDumpContext &dc) const
{
  CObject::Dump(dc);
}

#endif


BOOL CEUnitSkillType::Create(CString strName, CEUnitType *pUnitType)
{
  ASSERT(pUnitType != NULL);
  m_strName = strName;
  m_bEnabled = TRUE;
  m_nAllowedModes = 0;
  m_pUnitType = pUnitType;

  m_pData = NULL;
  m_dwDataSize = 0;

  return TRUE;
}

BOOL CEUnitSkillType::Create(CArchiveFile file, CEUnitType *pUnitType)
{
  ASSERT(pUnitType != NULL);
  
  if(g_dwUnitTypeFileVersion < MAP_FILE_VERSION(1, 0)){
    SSkillTypeHeader_Old h;

    file.Read(&h, sizeof(h));

    m_strName = h.m_pName;
    m_bEnabled = h.m_bEnabled;
    m_nAllowedModes = h.m_nAllowedModes;
    m_dwDataSize = h.m_dwDataSize;
    m_pUnitType = pUnitType;

    AllocateData(h.m_dwDataSize);
    file.Read(m_pData, h.m_dwDataSize);

    CUnitSkillTypeDlg *pTypeDlg = g_pUnitSkillTypeCollection->GetByName(m_strName);
    if(pTypeDlg != NULL){
      pTypeDlg->OnLoadSkill(this);
    }
  }
  else{
    SSkillTypeHeader h;

    file.Read(&h, sizeof(h));

    m_strName = h.m_pName;
    m_bEnabled = h.m_bEnabled;
    m_nAllowedModes = h.m_nAllowedModes;
    m_dwDataSize = h.m_dwDataSize;
    m_pUnitType = pUnitType;

    AllocateData(h.m_dwDataSize);
    file.Read(m_pData, h.m_dwDataSize);
    CUnitSkillTypeDlg *pTypeDlg = g_pUnitSkillTypeCollection->GetByName(m_strName);
    if(pTypeDlg != NULL){
      pTypeDlg->OnLoadSkill(this);
    }
  }

  return TRUE;
}

void CEUnitSkillType::Delete()
{
  FreeData();
}

void CEUnitSkillType::SaveToFile(CArchiveFile file)
{
  SSkillTypeHeader h;

  memset(&h, 0, sizeof(h));

  strncpy(h.m_pName, m_strName, 30);
  h.m_bEnabled = m_bEnabled;
  h.m_nAllowedModes = m_nAllowedModes;
  h.m_dwDataSize = m_dwDataSize;

  file.Write(&h, sizeof(h));
  if(m_dwDataSize > 0)
    file.Write(m_pData, m_dwDataSize);
}

BYTE * CEUnitSkillType::AllocateData(DWORD dwSize)
{
  FreeData();

  if(dwSize == 0) return NULL;

  m_pData = new BYTE[dwSize];
  m_dwDataSize = dwSize;
  return m_pData;
}

BYTE * CEUnitSkillType::ReallocateData(DWORD dwNewSize)
{
  BYTE *pNewData = NULL;
  if(dwNewSize != NULL){
    pNewData = new BYTE[dwNewSize];
  }
  if(m_pData != NULL){
    DWORD dwMin = m_dwDataSize;
    if(dwMin > dwNewSize) dwMin = dwNewSize;
    memcpy(pNewData, m_pData, dwMin);
    delete m_pData;
  }
  m_pData = pNewData;
  m_dwDataSize = dwNewSize;
  return m_pData;
}

void CEUnitSkillType::FreeData()
{
  if(m_pData != NULL){
    CUnitSkillTypeDlg *pTypeDlg = g_pUnitSkillTypeCollection->GetByName(m_strName);
    if(pTypeDlg != NULL){
      pTypeDlg->DeleteSkill(this);
    }
    delete m_pData;
    m_pData = NULL;
  }
  m_dwDataSize = 0;
}

BYTE * CEUnitSkillType::GetData()
{
  return m_pData;
}

BOOL CEUnitSkillType::CheckValid()
{
  CUnitSkillTypeDlg *pTypeDlg = g_pUnitSkillTypeCollection->GetByName(m_strName);
  if(pTypeDlg == NULL){
    AfxMessageBox("Neznámá dovednost. (vnitøní chyba)");
    return FALSE;
  }
  return pTypeDlg->CheckValid(this);
}