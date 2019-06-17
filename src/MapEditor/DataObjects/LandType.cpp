// LandType.cpp: implementation of the CLandType class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LandType.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLandType::CLandType()
{
  m_pAnimation = NULL;
  m_dwColor = 0;
}

CLandType::~CLandType()
{
  Delete();
}

BOOL CLandType::Create(BYTE nID, CString strName, CAnimation *pAnimation, DWORD dwColor)
{
  m_nID = nID;
  m_strName = strName;
  m_pAnimation = pAnimation;
  m_dwColor = dwColor;
  return TRUE;
}

BOOL CLandType::Create(BYTE nID, CString strName, DWORD dwColor)
{
  m_nID = nID;
  m_strName = strName;
  m_pAnimation = NULL;
  m_dwColor = dwColor;
  return TRUE;
}

void CLandType::Delete()
{
  if(m_pAnimation != NULL){
    m_pAnimation->Delete();
    delete m_pAnimation;
    m_pAnimation = NULL;
  }
}

CAnimation * CLandType::GetAnimation()
{
  return m_pAnimation;
}

DWORD CLandType::GetColor()
{
  return m_dwColor;
}

BYTE CLandType::GetID()
{
  return m_nID;
}

CString CLandType::GetName()
{
  return m_strName;
}
