// LandTypeTBButton.cpp: implementation of the CLandTypeTBButton class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LandTypeTBButton.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLandTypeTBButton::CLandTypeTBButton()
{
  m_pLandType = NULL;
}

CLandTypeTBButton::~CLandTypeTBButton()
{
}

void CLandTypeTBButton::SetLandType(CLandType *pLandType)
{
  m_pLandType = pLandType;
  if(m_pLandType != NULL){
    if(m_pLandType->GetAnimation() != NULL){
      m_pImage = m_pLandType->GetAnimation()->GetFrame(0);
    }
    else{
      m_pImage = NULL;
      m_dwColor = m_pLandType->GetColor();
    }
  }
  else{
    m_pImage = NULL;
    m_dwColor = 0x0;
  }

  Invalidate();
}

CLandType * CLandTypeTBButton::GetLandType()
{
  return m_pLandType;
}
