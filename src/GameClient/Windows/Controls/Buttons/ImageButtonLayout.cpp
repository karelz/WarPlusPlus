// ImageButtonLayout.cpp: implementation of the CImageButtonLayout class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ImageButtonLayout.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CImageButtonLayout, CObject)

CImageButtonLayout::CImageButtonLayout()
{
  m_pPassive = NULL; m_pActive = NULL; m_pPressed = NULL;

  m_pCursor = NULL;

  m_pPressSound = NULL; m_pReleaseSound = NULL; m_pHoverSound = NULL; m_pLeaveSound = NULL;
}

CImageButtonLayout::~CImageButtonLayout()
{
}

#ifdef _DEBUG

void CImageButtonLayout::AssertValid() const
{
  CObject::AssertValid();
}

void CImageButtonLayout::Dump(CDumpContext &dc) const
{
  CObject::Dump(dc);
}

#endif


BOOL CImageButtonLayout::Create(CConfigFile *pCfgFile)
{
  CFG_BEGIN(pCfgFile);

  CFG_ANIMATION("Passive", m_pPassive);
  CFG_ANIMATION("Active", m_pActive);
  CFG_ANIMATION("Pressed", m_pPressed);

  CFG_CURSOR_O("Cursor", m_pCursor);

  CFG_SOUND_O("HoverSound", m_pHoverSound);
  CFG_SOUND_O("LeaveSound", m_pLeaveSound);
  CFG_SOUND_O("PressSound", m_pPressSound);
  CFG_SOUND_O("ReleaseSound", m_pReleaseSound);

  CFG_END();

  return TRUE;
}

void CImageButtonLayout::Delete()
{
  CFG_DELETE(m_pPassive);
  CFG_DELETE(m_pActive);
  CFG_DELETE(m_pPressed);

  CFG_DELETE(m_pCursor);

  CFG_DELETE(m_pHoverSound);
  CFG_DELETE(m_pLeaveSound);
  CFG_DELETE(m_pPressSound);
  CFG_DELETE(m_pReleaseSound);
}


BOOL CImageButtonLayout::Create(CArchiveFile CfgFile)
{
  CConfigFile Config;
  BOOL bResult = TRUE;

  // create the config file
  Config.Create(CfgFile);

  if(!Create(&Config)){
    bResult = FALSE;
  }

  Config.Delete();

  return bResult; 
}

BOOL CImageButtonLayout::AreAnimsTransparent()
{
  if(!m_pPassive) return TRUE;
  else if(m_pPassive->IsTransparent()) return TRUE;
  if(!m_pActive) return TRUE;
  else if(m_pActive->IsTransparent()) return TRUE;
  if(!m_pPressed) return TRUE;
  else if(m_pPressed->IsTransparent()) return TRUE;

  return FALSE;
}
