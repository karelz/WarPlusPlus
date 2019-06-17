// IconButtonLayout.cpp: implementation of the CIconButtonLayout class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IconButtonLayout.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CIconButtonLayout, CObject);

CIconButtonLayout::CIconButtonLayout()
{
  m_pPassive = NULL; m_pActive = NULL; m_pPressed = NULL;
  m_pCursor = NULL;
  m_pHoverSound = NULL; m_pLeaveSound = NULL;
  m_pPressSound = NULL; m_pReleaseSound = NULL;
}

CIconButtonLayout::~CIconButtonLayout()
{
}

#ifdef _DEBUG

void CIconButtonLayout::AssertValid() const
{
  CObject::AssertValid();
}

void CIconButtonLayout::Dump(CDumpContext &dc) const
{
  CObject::Dump(dc);
}

#endif


BOOL CIconButtonLayout::Create(CArchiveFile CfgFile)
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

void CIconButtonLayout::Delete()
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

BOOL CIconButtonLayout::Create(CConfigFile *pCfgFile)
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

  CFG_POINT_O("IconOffset_Passive", m_ptPassiveOffset);
  CFG_POINT_O("IconOffset_Active", m_ptActiveOffset);
  CFG_POINT_O("IconOffset_Pressed", m_ptPressedOffset);

  if(pCfgFile->GetRootSection().IsVariable("IconCenter")){
    m_bCenter = pCfgFile->GetRootSection().GetBoolean("IconCenter");
  }
  else{
    m_bCenter = FALSE;
  }

  CFG_END();

  return TRUE;
}

BOOL CIconButtonLayout::AreAnimsTransparent()
{
  if(!m_pPassive) return TRUE;
  else if(m_pPassive->IsTransparent()) return TRUE;
  if(!m_pActive) return TRUE;
  else if(m_pActive->IsTransparent()) return TRUE;
  if(!m_pPressed) return TRUE;
  else if(m_pPressed->IsTransparent()) return TRUE;
 
  return FALSE;
}
