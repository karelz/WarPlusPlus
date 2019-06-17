// EditBoxLayout.cpp: implementation of the CEditBoxLayout class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "EditBoxLayout.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CEditBoxLayout, CFrameWindowLayout);

CEditBoxLayout::CEditBoxLayout()
{
  m_nLeftMargin = 2;
  m_nRightMargin = 2;
  m_nTopMargin = 2;
  m_nBottomMargin = 2;

  m_dwActiveTextColor = 0;
  m_dwInactiveTextColor = 0;
  m_dwSelectedTextColor = 0xFFFFFFFF;

  m_pSelection = NULL;
  m_pCaret = NULL;

  m_pFont = NULL;
}

CEditBoxLayout::~CEditBoxLayout()
{

}

#ifdef _DEBUG

void CEditBoxLayout::AssertValid() const
{
  CFrameWindowLayout::AssertValid();
}

void CEditBoxLayout::Dump(CDumpContext &dc) const
{
  CFrameWindowLayout::Dump(dc);
}

#endif


BOOL CEditBoxLayout::Create(CConfigFile *pCfgFile)
{
  CFG_BEGIN(pCfgFile);

  CFG_COLOR("ActiveTextColor", m_dwActiveTextColor);
  CFG_COLOR("InactiveTextColor", m_dwInactiveTextColor);
  CFG_COLOR("SelectedTextColor", m_dwSelectedTextColor);

  CFG_FONT_O("Font", m_pFont, 100);

  CFG_LONG("LeftMargin", m_nLeftMargin);
  CFG_LONG("RightMargin", m_nRightMargin);
  CFG_LONG("TopMargin", m_nTopMargin);
  CFG_LONG("BottomMargin", m_nBottomMargin);

  CFG_ANIMATION("Selection", m_pSelection);
  CFG_ANIMATION("Caret", m_pCaret);

  CFG_END();

  if(!CFrameWindowLayout::Create(pCfgFile)) return FALSE;

  m_nDragLeft = -1; m_nDragTop = -1;
  return TRUE;
}

void CEditBoxLayout::Delete()
{
  CFG_DELETE(m_pSelection);
  CFG_DELETE(m_pCaret);
  
  CFG_DELETE(m_pFont);

  CFrameWindowLayout::Delete();
}

CFontObject * CEditBoxLayout::GetFont()
{
  if(m_pFont == NULL) return g_pSystemFont;
  return m_pFont;
}

BOOL CEditBoxLayout::Create(CArchiveFile CfgFile)
{
  return CFrameWindowLayout::Create(CfgFile);
}
