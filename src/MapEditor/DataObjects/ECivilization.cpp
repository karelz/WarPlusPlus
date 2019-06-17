// ECivilization.cpp: implementation of the CECivilization class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\MapEditor.h"
#include "ECivilization.h"

#include "..\MapFormats.h"
#include "EMap.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CECivilization, CObject)

CECivilization::CECivilization()
{
  m_pScriptSet = NULL;
  m_dwStartPosX = 0;
  m_dwStartPosY = 0;
}

CECivilization::~CECivilization()
{
  Delete();
}

#ifdef _DEBUG

void CECivilization::AssertValid() const
{
  CObject::AssertValid();
}

void CECivilization::Dump(CDumpContext &dc) const
{
  CObject::Dump(dc);
}

#endif


BOOL CECivilization::Create(DWORD dwID, CString strName, DWORD dwColor)
{
  m_dwID = dwID;
  m_strName = strName;
  m_dwColor = dwColor;
  m_pScriptSet = NULL;

  memset(m_aResources, 0, RESOURCE_COUNT * sizeof(int));

  return TRUE;
}

void CECivilization::Delete()
{
}

BOOL CECivilization::Create(CArchiveFile file, CEMap *pMap)
{
  if(g_dwMapFileVersion < MAP_FILE_VERSION(1,0)){
    SCivilizationHeader_Old h;

    file.Read(&h, sizeof(h));
    m_dwID = h.m_dwID;
    m_dwColor = h.m_dwColor;
    m_strName = h.m_aName;
    m_pScriptSet = NULL;
    memcpy(m_aResources, h.m_aResources, RESOURCE_COUNT * sizeof(int));
  }
  else if(g_dwMapFileVersion == MAP_FILE_VERSION(1, 0)){
    SCivilizationHeader1p0 h;

    file.Read(&h, sizeof(h));
    m_dwID = h.m_dwID;
    m_dwColor = h.m_dwColor;
    m_strName = h.m_aName;
    m_pScriptSet = NULL; // No script set can be loaded
    memcpy(m_aResources, h.m_aResources, RESOURCE_COUNT * sizeof(int));
  }
  else{
    SCivilizationHeader h;

    file.Read(&h, sizeof(h));
    m_dwID = h.m_dwID;
    m_dwColor = h.m_dwColor;
    m_strName = h.m_aName;
    m_pScriptSet = pMap->GetScriptSet(h.m_dwScriptSetID);
    memcpy(m_aResources, h.m_aResources, RESOURCE_COUNT * sizeof(int));

    m_dwStartPosX = h.m_dwStartPositionX;
    m_dwStartPosY = h.m_dwStartPositionY;
  }

  return TRUE;
}

void CECivilization::SaveToFile(CArchiveFile file)
{
  SCivilizationHeader h;

  memset(&h, 0, sizeof(h));
  h.m_dwID = m_dwID;
  h.m_dwColor = m_dwColor;
  strncpy(h.m_aName, m_strName, 255);
  if(m_pScriptSet != NULL){
    h.m_dwScriptSetID = m_pScriptSet->GetID();
  }
  else{
    h.m_dwScriptSetID = 0;
  }
  memcpy(h.m_aResources, m_aResources, RESOURCE_COUNT * sizeof(int));
  h.m_dwStartPositionX = m_dwStartPosX;
  h.m_dwStartPositionY = m_dwStartPosY;
  file.Write(&h, sizeof(h));
}

void CECivilization::SetColor(DWORD dwColor)
{
  m_dwColor = dwColor;
}

void CECivilization::SetName(CString strName)
{
  m_strName = strName;
}

BOOL CECivilization::CheckValid()
{
  if(m_strName.IsEmpty()){
    AfxMessageBox("Civilizace nemùže mít prázdné jméno.");
    return FALSE;
  }
  {
    int nPos, nLen = m_strName.GetLength();
    for(nPos = 0; nPos < nLen; nPos++){
      if((m_strName[nPos] < 32) || (m_strName[nPos] >= 128)){
        AfxMessageBox("Jméno civilizace nesmí obsahovat diakritiku ani kontrolní znaky.\nDoporuèujeme použít anglická jména");
        return FALSE;
      }
    }
  }

  if(m_pScriptSet == NULL){
    CString str;
    str = "Musíte zadat sadu skriptù pro civilizaci '";
    str += m_strName;
    str += "'.";
    AfxMessageBox(str);
    return FALSE;
  }

  if((m_dwStartPosX == 0) && (m_dwStartPosY == 0)){
    POSITION pos = AppGetActiveMap()->GetFirstUnitPosition();
    CEUnit *pUnit;
    while(pos != NULL){
      pUnit = AppGetActiveMap()->GetNextUnit(pos);
      if(pUnit->GetCivilization() == this){
        m_dwStartPosX = pUnit->GetXPos();
        m_dwStartPosY = pUnit->GetYPos();
        break;
      }
    }
  }
  return TRUE;
}