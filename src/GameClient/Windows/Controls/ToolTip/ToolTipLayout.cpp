// ToolTipLayout.cpp: implementation of the CToolTipLayout class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ToolTipLayout.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CToolTipLayout, CObject);

CToolTipLayout::CToolTipLayout()
{
  m_pFont = NULL;

  m_nOpaque = 255;
  m_dwBackgroundColor = 0;
  m_dwTextColor = 0xFFFFFF;
  m_sizeRBMargin = CSize(1, 1);
  m_ptTextOffset = CPoint(1, 1);
}

CToolTipLayout::~CToolTipLayout()
{
}

#ifdef _DEBUG

void CToolTipLayout::AssertValid() const
{
  CObject::AssertValid();
}

void CToolTipLayout::Dump(CDumpContext &dc) const
{
  CObject::Dump(dc);
}

#endif


BOOL CToolTipLayout::Create(CArchiveFile file)
{
  CConfigFile Config;
  BOOL bResult = TRUE;

  // create the config file
  Config.Create(file);

  if(!Create(&Config)){
    bResult = FALSE;
  }

  Config.Delete();

  return bResult;
}

BOOL CToolTipLayout::Create(CConfigFile *pCfgFile)
{
  CFG_BEGIN(pCfgFile);

  CFG_FONT_O("Font", m_pFont, 100);

  CFG_COLOR_O("Background_Color", m_dwBackgroundColor);
  CFG_COLOR_O("Text_Color", m_dwTextColor);

  CFG_LONG_O("Background_Opaque", m_nOpaque);

  CFG_POINT_O("TextOffset", m_ptTextOffset);
  CFG_SIZE_O("RBMargin", m_sizeRBMargin);

  CFG_END();

  return TRUE;
}

void CToolTipLayout::Delete()
{
  CFG_DELETE(m_pFont);
}

CFontObject * CToolTipLayout::GetFont()
{
  if(m_pFont == NULL) return g_pSystemFont;
  return m_pFont;
}
