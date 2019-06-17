// UnitTBButton.cpp: implementation of the CUnitTBButton class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\MapEditor.h"
#include "UnitTBButton.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUnitTBButton::CUnitTBButton()
{
  m_pUnitType = NULL;
  m_pCivilization = NULL;
}

CUnitTBButton::~CUnitTBButton()
{

}

CEUnitType *CUnitTBButton::GetUnitType()
{
  return m_pUnitType;
}

void CUnitTBButton::SetUnitType(CEUnitType *pUnitType, CECivilization *pCivilization)
{
  m_AppInstance.Delete();

  m_pUnitType = pUnitType;
  m_pCivilization = pCivilization;

  if((pUnitType == NULL) || (pCivilization == NULL)){
    m_pImage = NULL;
    m_dwColor = RGB32(0, 0, 0);
    return;
  }

  CEUnitAppearanceType *pAppType = pUnitType->GetDefaultAppearanceType();

  if(pAppType == NULL){
    m_pImage = NULL;
    m_dwColor = RGB32(0, 0, 0);
    return;
  }
  
  m_AppInstance.Create(pAppType, pCivilization->GetColor(), pAppType->GetArchive());

  m_pImage = NULL;
  int i;
  for(i = 0; i < 8; i++){
    if(m_AppInstance.GetDirection(i) != NULL){
      m_pImage = m_AppInstance.GetDirection(i)->GetFrame(0);
      break;
    }
  }
  if(m_pImage == NULL){
    m_dwColor = RGB32(0, 0, 0);
  }

  Invalidate();
}

CECivilization * CUnitTBButton::GetUnitCivilization()
{
  return m_pCivilization;
}

void CUnitTBButton::BeforePaint()
{
  if(m_pCivilization == NULL) return;

  if(m_AppInstance.GetColor() != m_pCivilization->GetColor()){
    SetUnitType(m_pUnitType, m_pCivilization);
  }
}
