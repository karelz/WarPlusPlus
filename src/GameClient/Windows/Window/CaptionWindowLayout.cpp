// CaptionWindowLayout.cpp: implementation of the CCaptionWindowLayout class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CaptionWindowLayout.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CCaptionWindowLayout, CFrameWindowLayout);

CCaptionWindowLayout::CCaptionWindowLayout()
{
  m_nRightMargin = 0;
  m_dwACaptionColor = 0;
  m_dwPCaptionColor = 0;
  m_pFont = NULL;
}

CCaptionWindowLayout::~CCaptionWindowLayout()
{

}

#ifdef _DEBUG

void CCaptionWindowLayout::AssertValid() const
{
  CFrameWindowLayout::AssertValid();
}

void CCaptionWindowLayout::Dump(CDumpContext &dc) const
{
  CFrameWindowLayout::Dump(dc);
}

#endif


BOOL CCaptionWindowLayout::Create(CConfigFile *pCfgFile)
{
  CFG_BEGIN(pCfgFile);

  CFG_COLOR_O("Active_CaptionColor", m_dwACaptionColor); //RGB32(140, 240, 208);
  CFG_COLOR_O("Passive_CaptionColor", m_dwPCaptionColor); //RGB32( 16, 208, 128);

  CFG_POINT_O("CaptionPos", m_ptCaptionPos); //CPoint(5, 6);

  CFG_FONT_O("CaptionFont", m_pFont, 100);
  
  CFG_END();

  return CFrameWindowLayout::Create(pCfgFile);
}

void CCaptionWindowLayout::Delete()
{
  CFG_DELETE(m_pFont);

  CFrameWindowLayout::Delete();
}

CFontObject * CCaptionWindowLayout::GetFont()
{
  if(m_pFont == NULL)
    return g_pSystemFont;
  return m_pFont;
}

BOOL CCaptionWindowLayout::Create(CArchiveFile CfgFile)
{
  return CFrameWindowLayout::Create(CfgFile);
}
