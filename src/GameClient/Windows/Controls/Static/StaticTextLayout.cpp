// StaticTextLayout.cpp: implementation of the CStaticTextLayout class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "StaticTextLayout.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CStaticTextLayout, CObject)

CStaticTextLayout::CStaticTextLayout()
{
  m_pFont = NULL;
  m_eAlign = 0;
  m_dwTextColor = 0x0FFFFFF;
}

CStaticTextLayout::~CStaticTextLayout()
{

}

#ifdef _DEBUG

void CStaticTextLayout::AssertValid() const
{
  CObject::AssertValid();
}

void CStaticTextLayout::Dump(CDumpContext &dc) const
{
  CObject::Dump(dc);
}

#endif

BOOL CStaticTextLayout::Create(CArchiveFile CfgFile)
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

BOOL CStaticTextLayout::Create(CConfigFile *pCfgFile)
{
  CFG_BEGIN(pCfgFile);

  CFG_FONT_O("Font", m_pFont, 100);

	CFG_LONG_O("Align", m_eAlign);

  CFG_COLOR_O("TextColor", m_dwTextColor);

  CFG_END();

  return TRUE;
}

void CStaticTextLayout::Delete()
{
  CFG_DELETE(m_pFont);
}

CFontObject * CStaticTextLayout::GetFont()
{
  ASSERT_VALID(this);

  if(m_pFont != NULL) return m_pFont;
  return g_pSystemFont;
}
