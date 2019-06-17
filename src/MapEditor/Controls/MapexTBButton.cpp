// MapexTBButton.cpp: implementation of the CMapexTBButton class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\MapEditor.h"
#include "MapexTBButton.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMapexTBButton::CMapexTBButton()
{
  m_pMapex = NULL;
}

CMapexTBButton::~CMapexTBButton()
{
}

void CMapexTBButton::SetMapex(CEMapex *pMapex)
{
  m_pMapex = pMapex;
  if(m_pMapex == NULL){
    m_pImage = NULL;
    m_dwColor = RGB32(0, 0, 0);
    return;
  }
  m_pImage = m_pMapex->GetGraphics()->GetFrame(0);
  if(m_pImage == NULL){
    m_dwColor = RGB32(0, 0, 0);
  }

  Invalidate();
}

CEMapex * CMapexTBButton::GetMapex()
{
  return m_pMapex;
}
