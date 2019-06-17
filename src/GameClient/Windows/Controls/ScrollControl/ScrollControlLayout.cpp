// ScrollControlLayout.cpp: implementation of the CScrollControlLayout class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ScrollControlLayout.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CScrollControlLayout, CObject)

CScrollControlLayout::CScrollControlLayout()
{
  m_pPassiveMiddle = NULL;

  m_pPassiveTop = NULL; m_pPassiveBottom = NULL; m_pPassiveMiddle = NULL;
  m_pActiveTop = NULL; m_pActiveBottom = NULL; m_pActiveMiddle = NULL;
  m_pPressedTop = NULL; m_pPressedBottom = NULL; m_pPressedMiddle = NULL;

  m_pPassiveButtonTop = NULL; m_pPassiveButtonMiddle = NULL; m_pPassiveButtonBottom = NULL;
  m_pActiveButtonTop = NULL; m_pActiveButtonMiddle = NULL; m_pActiveButtonBottom = NULL;
  m_pPressedButtonTop = NULL; m_pPressedButtonMiddle = NULL; m_pPressedButtonBottom = NULL;
}

CScrollControlLayout::~CScrollControlLayout()
{
  Delete();
}

#ifdef _DEBUG

void CScrollControlLayout::AssertValid() const
{
  CObject::AssertValid();
}

void CScrollControlLayout::Dump(CDumpContext &dc) const
{
  CObject::Dump(dc);
}

#endif

BOOL CScrollControlLayout::Create(CArchiveFile CfgFile)
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

BOOL CScrollControlLayout::Create(CConfigFile *pCfgFile)
{
  CFG_BEGIN(pCfgFile);
  
  CFG_ANIMATION("Passive_Top", m_pPassiveTop);
  CFG_ANIMATION("Passive_Middle", m_pPassiveMiddle);
  CFG_ANIMATION("Passive_Bottom", m_pPassiveBottom);
  CFG_ANIMATION("Active_Top", m_pActiveTop);
  CFG_ANIMATION("Active_Middle", m_pActiveMiddle);
  CFG_ANIMATION("Active_Bottom", m_pActiveBottom);
  CFG_ANIMATION("Pressed_Top", m_pPressedTop);
  CFG_ANIMATION("Pressed_Middle", m_pPressedMiddle);
  CFG_ANIMATION("Pressed_Bottom", m_pPressedBottom);

  CFG_ANIMATION("Passive_Button_Top", m_pPassiveButtonTop);
  CFG_ANIMATION("Passive_Button_Middle", m_pPassiveButtonMiddle);
  CFG_ANIMATION("Passive_Button_Bottom", m_pPassiveButtonBottom);
  CFG_ANIMATION("Active_Button_Top", m_pActiveButtonTop);
  CFG_ANIMATION("Active_Button_Middle", m_pActiveButtonMiddle);
  CFG_ANIMATION("Active_Button_Bottom", m_pActiveButtonBottom);
  CFG_ANIMATION("Pressed_Button_Top", m_pPressedButtonTop);
  CFG_ANIMATION("Pressed_Button_Middle", m_pPressedButtonMiddle);
  CFG_ANIMATION("Pressed_Button_Bottom", m_pPressedButtonBottom);

  CFG_LONG("Top_ButtonSize", m_nTopButtonSize);
  CFG_LONG("Bottom_ButtonSize", m_nBottomButtonSize);

  CFG_LONG("MinButtonSize", m_nMinButtonSize);

  CFG_END();

  return TRUE;
}

void CScrollControlLayout::Delete()
{
  CFG_DELETE(m_pPassiveTop); CFG_DELETE(m_pPassiveBottom); CFG_DELETE(m_pPassiveMiddle);
  CFG_DELETE(m_pActiveTop); CFG_DELETE(m_pActiveBottom); CFG_DELETE(m_pActiveMiddle);
  CFG_DELETE(m_pPressedTop); CFG_DELETE(m_pPressedBottom); CFG_DELETE(m_pPressedMiddle);

  CFG_DELETE(m_pPassiveButtonTop); CFG_DELETE(m_pPassiveButtonMiddle); CFG_DELETE(m_pPassiveButtonBottom);
  CFG_DELETE(m_pActiveButtonTop); CFG_DELETE(m_pActiveButtonMiddle); CFG_DELETE(m_pActiveButtonBottom);
  CFG_DELETE(m_pPressedButtonTop); CFG_DELETE(m_pPressedButtonMiddle); CFG_DELETE(m_pPressedButtonBottom);
}

int CScrollControlLayout::GetWidth()
{
  ASSERT_VALID(this);

  return m_pPassiveTop->GetSize().cx;
}

int CScrollControlLayout::GetHeight()
{
  ASSERT_VALID(this);

  return m_pPassiveTop->GetSize().cy;
}
