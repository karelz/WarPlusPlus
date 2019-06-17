// ListControlLayout.cpp: implementation of the CListControlLayout class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ListControlLayout.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CListControlLayout, CFrameWindowLayout);

CListControlLayout::CListControlLayout()
{
  m_pFont = NULL;

  m_dwLeftMargin = 0;
  m_dwTopMargin = 0;
  m_dwRightMargin = 0;
  m_dwBottomMargin = 0;

  m_dwNormalTextColor = 0;
  m_dwSelectedTextColor = 0;
  m_dwFocusTextColor = 0;

  m_pSelection = NULL;
  m_pFocus = NULL; m_pSelectionFocus = NULL;
  m_bHasHorizontalScroll = FALSE;
}

CListControlLayout::~CListControlLayout()
{

}

#ifdef _DEBUG

void CListControlLayout::AssertValid() const
{
  CFrameWindowLayout::AssertValid();
}

void CListControlLayout::Dump(CDumpContext &dc) const
{
  CFrameWindowLayout::Dump(dc);
}

#endif

BOOL CListControlLayout::Create(CArchiveFile CfgFile)
{
  return CFrameWindowLayout::Create(CfgFile);
}

BOOL CListControlLayout::Create(CConfigFile *pCfgFile)
{
  CFG_BEGIN(pCfgFile);

  CString strScrollConfig;
  strScrollConfig = cfg_Section.GetString("ScrollControl");
  m_ScrollControlLayout.Create(cfg_Archive.CreateFile(cfg_strPath + strScrollConfig));

  if(cfg_Section.IsVariable("HorizontalScrollControl")){
    strScrollConfig = cfg_Section.GetString("HorizontalScrollControl");
    m_HorizontalScrollControlLayout.Create(cfg_Archive.CreateFile(cfg_strPath + strScrollConfig));
    m_bHasHorizontalScroll = TRUE;
  }

  CFG_FONT_O("Font", m_pFont, 100);

  CFG_LONG_O("LeftMargin", m_dwLeftMargin);
  CFG_LONG_O("RightMargin", m_dwRightMargin);
  CFG_LONG_O("TopMargin", m_dwTopMargin);
  CFG_LONG_O("BottomMargin", m_dwBottomMargin);

  CFG_COLOR("Normal_TextColor", m_dwNormalTextColor);
  CFG_COLOR("Selected_TextColor", m_dwSelectedTextColor);
  CFG_COLOR("Focus_TextColor", m_dwFocusTextColor);
  CFG_COLOR("SelectedFocus_TextColor", m_dwSelectedFocusTextColor);

  CFG_ANIMATION_O("Selection", m_pSelection);
  CFG_ANIMATION_O("Focus", m_pFocus);
  CFG_ANIMATION_O("SelectionFocus", m_pSelectionFocus);

  CFG_END();

  if(!CFrameWindowLayout::Create(pCfgFile)) return FALSE;

  m_nDragLeft = -1; m_nDragTop = -1;
  return TRUE;
}

void CListControlLayout::Delete()
{
  CFG_DELETE(m_pFont);

  CFG_DELETE(m_pSelection); CFG_DELETE(m_pFocus); CFG_DELETE(m_pSelectionFocus);

  m_ScrollControlLayout.Delete();
  if(m_bHasHorizontalScroll){
    m_HorizontalScrollControlLayout.Delete();
  }

  CFrameWindowLayout::Delete();
}

CFontObject * CListControlLayout::GetFont()
{
  if(m_pFont) return m_pFont;
  return g_pSystemFont;
}
