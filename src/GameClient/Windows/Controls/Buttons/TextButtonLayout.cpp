// TextButtonLayout.cpp: implementation of the CTextButtonLayout class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TextButtonLayout.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTextButtonLayout::CTextButtonLayout()
{
  m_pPassiveLeft = NULL; m_pPassiveMiddle = NULL; m_pPassiveRight = NULL;
  m_pPassiveKeyLeft = NULL; m_pPassiveKeyMiddle = NULL; m_pPassiveKeyRight = NULL;
  m_pActiveLeft = NULL; m_pActiveMiddle = NULL; m_pActiveRight = NULL;
  m_pActiveKeyLeft = NULL; m_pActiveKeyMiddle = NULL; m_pActiveKeyRight = NULL;
  m_pPressedLeft = NULL; m_pPressedMiddle = NULL; m_pPressedRight = NULL;

  m_pHoverSound = NULL; m_pLeaveSound = NULL;
  m_pPressSound = NULL; m_pReleaseSound = NULL;

  m_dwPassiveRightMargin = 0; m_dwActiveRightMargin = 0; m_dwPressedRightMargin = 0;
  m_dwPassiveTextColor = 0; m_dwActiveTextColor = 0; m_dwPressedTextColor = 0;

  m_pFont = NULL;
  m_pCursor = NULL;

  m_dwMinimalTextWidth = 0;
}

CTextButtonLayout::~CTextButtonLayout()
{

}

#ifdef _DEBUG

void CTextButtonLayout::AssertValid() const
{
  CObject::AssertValid();
}

void CTextButtonLayout::Dump(CDumpContext &dc) const
{
  CObject::Dump(dc);
}

#endif

BOOL CTextButtonLayout::Create(CConfigFile *pCfgFile)
{
  CFG_BEGIN(pCfgFile);

  CFG_ANIMATION("Passive_Left", m_pPassiveLeft);
  CFG_ANIMATION("Passive_Middle", m_pPassiveMiddle);
  CFG_ANIMATION("Passive_Right", m_pPassiveRight);

  CFG_ANIMATION("PassiveKey_Left", m_pPassiveKeyLeft);
  CFG_ANIMATION("PassiveKey_Middle", m_pPassiveKeyMiddle);
  CFG_ANIMATION("PassiveKey_Right", m_pPassiveKeyRight);

  CFG_ANIMATION("Active_Left", m_pActiveLeft);
  CFG_ANIMATION("Active_Middle", m_pActiveMiddle);
  CFG_ANIMATION("Active_Right", m_pActiveRight);

  CFG_ANIMATION("ActiveKey_Left", m_pActiveKeyLeft);
  CFG_ANIMATION("ActiveKey_Middle", m_pActiveKeyMiddle);
  CFG_ANIMATION("ActiveKey_Right", m_pActiveKeyRight);

  CFG_ANIMATION("Pressed_Left", m_pPressedLeft);
  CFG_ANIMATION("Pressed_Middle", m_pPressedMiddle);
  CFG_ANIMATION("Pressed_Right", m_pPressedRight);

  
  CFG_SOUND_O("HoverSound", m_pHoverSound);
  CFG_SOUND_O("LeaveSound", m_pLeaveSound);
  CFG_SOUND_O("PressSound", m_pPressSound);
  CFG_SOUND_O("ReleaseSound", m_pReleaseSound);

  CFG_CURSOR_O("Cursor", m_pCursor);

  CFG_FONT_O("Font", m_pFont, 100);

  CFG_POINT("Passive_TextOrigin", m_ptPassiveTextOrig); //CPoint(11, 2);
  CFG_POINT("Active_TextOrigin", m_ptActiveTextOrig);   //CPoint(10, 1);
  CFG_POINT("Pressed_TextOrigin", m_ptPressedTextOrig); //CPoint(11, 2);

  CFG_LONG("Passive_RightMargin", m_dwPassiveRightMargin); // 12
  CFG_LONG("Active_RightMargin", m_dwActiveRightMargin);   // 11
  CFG_LONG("Pressed_RightMargin", m_dwPressedRightMargin); // 12

  CFG_COLOR("Passive_TextColor", m_dwPassiveTextColor); //RGB32( 16, 208, 128);
  CFG_COLOR("Active_TextColor", m_dwActiveTextColor);   //RGB32(140, 240, 208);
  CFG_COLOR("Pressed_TextColor", m_dwPressedTextColor); //RGB32(140, 240, 208);

  CFG_SIZE_O("VirtualSize", m_sizeVirtual);

  CFG_LONG_O("MinimalTextWidth", m_dwMinimalTextWidth);

  CFG_END();

  return TRUE;
}

void CTextButtonLayout::Delete()
{
  CFG_DELETE(m_pPassiveLeft); CFG_DELETE(m_pPassiveMiddle); CFG_DELETE(m_pPassiveRight);
  CFG_DELETE(m_pPassiveKeyLeft); CFG_DELETE(m_pPassiveKeyMiddle); CFG_DELETE(m_pPassiveKeyRight);
  CFG_DELETE(m_pActiveLeft); CFG_DELETE(m_pActiveMiddle); CFG_DELETE(m_pActiveRight);
  CFG_DELETE(m_pActiveKeyLeft); CFG_DELETE(m_pActiveKeyMiddle); CFG_DELETE(m_pActiveKeyRight);
  CFG_DELETE(m_pPressedLeft); CFG_DELETE(m_pPressedMiddle); CFG_DELETE(m_pPressedRight);

  CFG_DELETE(m_pPressSound);
  CFG_DELETE(m_pHoverSound);
  CFG_DELETE(m_pReleaseSound);
  CFG_DELETE(m_pLeaveSound);

  CFG_DELETE(m_pCursor);

  CFG_DELETE(m_pFont);
}

BOOL CTextButtonLayout::AreAnimsTransparent()
{
  if(!m_pPassiveLeft) return TRUE;
  else if(m_pPassiveLeft->IsTransparent()) return TRUE;
  if(!m_pPassiveMiddle) return TRUE;
  else if(m_pPassiveMiddle->IsTransparent()) return TRUE;
  if(!m_pPassiveRight) return TRUE;
  else if(m_pPassiveRight->IsTransparent()) return TRUE;

  if(!m_pPassiveKeyLeft) return TRUE;
  else if(m_pPassiveKeyLeft->IsTransparent()) return TRUE;
  if(!m_pPassiveKeyMiddle) return TRUE;
  else if(m_pPassiveKeyMiddle->IsTransparent()) return TRUE;
  if(!m_pPassiveKeyRight) return TRUE;
  else if(m_pPassiveKeyRight->IsTransparent()) return TRUE;

  if(!m_pActiveLeft) return TRUE;
  else if(m_pActiveLeft->IsTransparent()) return TRUE;
  if(!m_pActiveMiddle) return TRUE;
  else if(m_pActiveLeft->IsTransparent()) return TRUE;
  if(!m_pActiveRight) return TRUE;
  else if(m_pActiveLeft->IsTransparent()) return TRUE;

  if(!m_pActiveKeyLeft) return TRUE;
  else if(m_pActiveKeyLeft->IsTransparent()) return TRUE;
  if(!m_pActiveKeyMiddle) return TRUE;
  else if(m_pActiveKeyMiddle->IsTransparent()) return TRUE;
  if(!m_pActiveKeyRight) return TRUE;
  else if(m_pActiveKeyRight->IsTransparent()) return TRUE;

  if(!m_pPressedLeft) return TRUE;
  else if(m_pPressedLeft->IsTransparent()) return TRUE;
  if(!m_pPressedMiddle) return TRUE;
  else if(m_pPressedLeft->IsTransparent()) return TRUE;
  if(!m_pPressedRight) return TRUE;
  else if(m_pPressedLeft->IsTransparent()) return TRUE;

  return FALSE;
}

CFontObject * CTextButtonLayout::GetFont()
{
  if(m_pFont == NULL)
    return g_pSystemFont;
  return m_pFont;
}

BOOL CTextButtonLayout::Create(CArchiveFile CfgFile)
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

BOOL CTextButtonLayout::Create()
{
  return TRUE;
}
