// ScriptEditLayout.cpp: implementation of the CScriptEditLayout class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ScriptEditLayout.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CScriptEditLayout, CFrameWindowLayout)

CScriptEditLayout::CScriptEditLayout()
{
  m_pFont = NULL;
  m_pCaret = NULL;

  m_nLeftMargin = 0; m_nRightMargin = 0;
  m_nTopMargin = 0; m_nBottomMargin = 0;

  m_nTabSize = 2;
}

CScriptEditLayout::~CScriptEditLayout()
{

}

#ifdef _DEBUG

void CScriptEditLayout::AssertValid() const
{
  CFrameWindowLayout::AssertValid();
}

void CScriptEditLayout::Dump(CDumpContext &dc) const
{
  CFrameWindowLayout::Dump(dc);
}

#endif

BOOL CScriptEditLayout::Create(CConfigFile *pCfgFile)
{
  CFG_BEGIN(pCfgFile)

  CString strScrollConfig;
  strScrollConfig = cfg_Section.GetString("VerticalScrollControl");
  m_VerticalScrollControlLayout.Create(cfg_Archive.CreateFile(cfg_strPath + strScrollConfig));
  strScrollConfig = cfg_Section.GetString("HorizontalScrollControl");
  m_HorizontalScrollControlLayout.Create(cfg_Archive.CreateFile(cfg_strPath + strScrollConfig));

  CFG_ANIMATION("Caret", m_pCaret);

  CFG_COLOR("BackgroundColor", m_dwBackgroundColor);
  CFG_COLOR("SelectionColor", m_dwSelectionColor);

  CFG_LONG_O("LeftMargin", m_nLeftMargin);
  CFG_LONG_O("RightMargin", m_nRightMargin);
  CFG_LONG_O("TopMargin", m_nTopMargin);
  CFG_LONG_O("BottomMargin", m_nBottomMargin);

  CFG_LONG_O("TabSize", m_nTabSize);

  CString strFont;
  CGraphicFont *pFont;
  strFont = cfg_Section.GetString("Font");
  pFont = new CGraphicFont();
  pFont->Create(cfg_Archive.CreateFile(cfg_strPath + strFont));
  m_pFont = pFont;

  int nTokensNum = 0;
  CString strToken;
  STokenStyle *pStyle;
  m_aTokenStyles.SetSize(13);

  pStyle = new STokenStyle(); CFG_COLOR("Token_Comment", pStyle->m_dwColor);
  pStyle->m_pNormalFont = NULL;
  pStyle->m_pSelectionFont = NULL;
  pStyle->m_pNormalFont = new CGraphicCacheFont();
  pStyle->m_pNormalFont->Create(pFont, pStyle->m_dwColor, m_dwBackgroundColor);
  pStyle->m_pSelectionFont = new CGraphicCacheFont();
  pStyle->m_pSelectionFont->Create(pFont, pStyle->m_dwColor, m_dwSelectionColor);
  m_aTokenStyles[0] = pStyle;
  pStyle = new STokenStyle(); CFG_COLOR("Token_String", pStyle->m_dwColor);
  pStyle->m_pNormalFont = NULL;
  pStyle->m_pSelectionFont = NULL;
  pStyle->m_pNormalFont = new CGraphicCacheFont();
  pStyle->m_pNormalFont->Create(pFont, pStyle->m_dwColor, m_dwBackgroundColor);
  pStyle->m_pSelectionFont = new CGraphicCacheFont();
  pStyle->m_pSelectionFont->Create(pFont, pStyle->m_dwColor, m_dwSelectionColor);
  m_aTokenStyles[1] = pStyle;
  pStyle = new STokenStyle(); CFG_COLOR("Token_Integer", pStyle->m_dwColor);
  pStyle->m_pNormalFont = NULL;
  pStyle->m_pSelectionFont = NULL;
  pStyle->m_pNormalFont = new CGraphicCacheFont();
  pStyle->m_pNormalFont->Create(pFont, pStyle->m_dwColor, m_dwBackgroundColor);
  pStyle->m_pSelectionFont = new CGraphicCacheFont();
  pStyle->m_pSelectionFont->Create(pFont, pStyle->m_dwColor, m_dwSelectionColor);
  m_aTokenStyles[2] = pStyle;
  pStyle = new STokenStyle(); CFG_COLOR("Token_Float", pStyle->m_dwColor);
  pStyle->m_pNormalFont = NULL;
  pStyle->m_pSelectionFont = NULL;
  pStyle->m_pNormalFont = new CGraphicCacheFont();
  pStyle->m_pNormalFont->Create(pFont, pStyle->m_dwColor, m_dwBackgroundColor);
  pStyle->m_pSelectionFont = new CGraphicCacheFont();
  pStyle->m_pSelectionFont->Create(pFont, pStyle->m_dwColor, m_dwSelectionColor);
  m_aTokenStyles[3] = pStyle;
  pStyle = new STokenStyle(); CFG_COLOR("Token_Keyword", pStyle->m_dwColor);
  pStyle->m_pNormalFont = NULL;
  pStyle->m_pSelectionFont = NULL;
  pStyle->m_pNormalFont = new CGraphicCacheFont();
  pStyle->m_pNormalFont->Create(pFont, pStyle->m_dwColor, m_dwBackgroundColor);
  pStyle->m_pSelectionFont = new CGraphicCacheFont();
  pStyle->m_pSelectionFont->Create(pFont, pStyle->m_dwColor, m_dwSelectionColor);
  m_aTokenStyles[4] = pStyle;
  pStyle = new STokenStyle(); CFG_COLOR("Token_Constant", pStyle->m_dwColor);
  pStyle->m_pNormalFont = NULL;
  pStyle->m_pSelectionFont = NULL;
  pStyle->m_pNormalFont = new CGraphicCacheFont();
  pStyle->m_pNormalFont->Create(pFont, pStyle->m_dwColor, m_dwBackgroundColor);
  pStyle->m_pSelectionFont = new CGraphicCacheFont();
  pStyle->m_pSelectionFont->Create(pFont, pStyle->m_dwColor, m_dwSelectionColor);
  m_aTokenStyles[5] = pStyle;
  pStyle = new STokenStyle(); CFG_COLOR("Token_DataType", pStyle->m_dwColor);
  pStyle->m_pNormalFont = NULL;
  pStyle->m_pSelectionFont = NULL;
  pStyle->m_pNormalFont = new CGraphicCacheFont();
  pStyle->m_pNormalFont->Create(pFont, pStyle->m_dwColor, m_dwBackgroundColor);
  pStyle->m_pSelectionFont = new CGraphicCacheFont();
  pStyle->m_pSelectionFont->Create(pFont, pStyle->m_dwColor, m_dwSelectionColor);
  m_aTokenStyles[6] = pStyle;
  pStyle = new STokenStyle(); CFG_COLOR("Token_Operator", pStyle->m_dwColor);
  pStyle->m_pNormalFont = NULL;
  pStyle->m_pSelectionFont = NULL;
  pStyle->m_pNormalFont = new CGraphicCacheFont();
  pStyle->m_pNormalFont->Create(pFont, pStyle->m_dwColor, m_dwBackgroundColor);
  pStyle->m_pSelectionFont = new CGraphicCacheFont();
  pStyle->m_pSelectionFont->Create(pFont, pStyle->m_dwColor, m_dwSelectionColor);
  m_aTokenStyles[7] = pStyle;
  pStyle = new STokenStyle(); CFG_COLOR("Token_Text", pStyle->m_dwColor);
  pStyle->m_pNormalFont = NULL;
  pStyle->m_pSelectionFont = NULL;
  pStyle->m_pNormalFont = new CGraphicCacheFont();
  pStyle->m_pNormalFont->Create(pFont, pStyle->m_dwColor, m_dwBackgroundColor);
  pStyle->m_pSelectionFont = new CGraphicCacheFont();
  pStyle->m_pSelectionFont->Create(pFont, pStyle->m_dwColor, m_dwSelectionColor);
  m_aTokenStyles[8] = pStyle;
  pStyle = new STokenStyle(); CFG_COLOR("Token_Char", pStyle->m_dwColor);
  pStyle->m_pNormalFont = NULL;
  pStyle->m_pSelectionFont = NULL;
  pStyle->m_pNormalFont = new CGraphicCacheFont();
  pStyle->m_pNormalFont->Create(pFont, pStyle->m_dwColor, m_dwBackgroundColor);
  pStyle->m_pSelectionFont = new CGraphicCacheFont();
  pStyle->m_pSelectionFont->Create(pFont, pStyle->m_dwColor, m_dwSelectionColor);
  m_aTokenStyles[9] = pStyle;
  pStyle = new STokenStyle(); CFG_COLOR("Token_Hex", pStyle->m_dwColor);
  pStyle->m_pNormalFont = NULL;
  pStyle->m_pSelectionFont = NULL;
  pStyle->m_pNormalFont = new CGraphicCacheFont();
  pStyle->m_pNormalFont->Create(pFont, pStyle->m_dwColor, m_dwBackgroundColor);
  pStyle->m_pSelectionFont = new CGraphicCacheFont();
  pStyle->m_pSelectionFont->Create(pFont, pStyle->m_dwColor, m_dwSelectionColor);
  m_aTokenStyles[10] = pStyle;
  pStyle = new STokenStyle(); CFG_COLOR("Token_EndOfFile", pStyle->m_dwColor);
  pStyle->m_pNormalFont = NULL;
  pStyle->m_pSelectionFont = NULL;
  pStyle->m_pNormalFont = new CGraphicCacheFont();
  pStyle->m_pNormalFont->Create(pFont, pStyle->m_dwColor, m_dwBackgroundColor);
  pStyle->m_pSelectionFont = new CGraphicCacheFont();
  pStyle->m_pSelectionFont->Create(pFont, pStyle->m_dwColor, m_dwSelectionColor);
  m_aTokenStyles[11] = pStyle;
  pStyle = new STokenStyle(); CFG_COLOR("Token_Error", pStyle->m_dwColor);
  pStyle->m_pNormalFont = NULL;
  pStyle->m_pSelectionFont = NULL;
  pStyle->m_pNormalFont = new CGraphicCacheFont();
  pStyle->m_pNormalFont->Create(pFont, pStyle->m_dwColor, m_dwBackgroundColor);
  pStyle->m_pSelectionFont = new CGraphicCacheFont();
  pStyle->m_pSelectionFont->Create(pFont, pStyle->m_dwColor, m_dwSelectionColor);
  m_aTokenStyles[12] = pStyle;

  CFG_END()

  if(!CFrameWindowLayout::Create(pCfgFile)) return FALSE;

  m_nDragLeft = -1; m_nDragTop = -1;
  return TRUE;
}

BOOL CScriptEditLayout::Create(CArchiveFile CfgFile)
{
  return CFrameWindowLayout::Create(CfgFile);
}

void CScriptEditLayout::Delete()
{
  CFG_DELETE(m_pFont);
  CFG_DELETE(m_pCaret);

  m_VerticalScrollControlLayout.Delete();
  m_HorizontalScrollControlLayout.Delete();

  int i;
  for(i = 0; i < m_aTokenStyles.GetSize(); i++){
    if(m_aTokenStyles[i] == NULL) continue;
    if(m_aTokenStyles[i]->m_pNormalFont != NULL)
      delete m_aTokenStyles[i]->m_pNormalFont;
    if(m_aTokenStyles[i]->m_pSelectionFont != NULL)
      delete m_aTokenStyles[i]->m_pSelectionFont;
    delete m_aTokenStyles[i];
  }
  m_aTokenStyles.RemoveAll();

  CFrameWindowLayout::Delete();
}

CFontObject * CScriptEditLayout::GetFont()
{
  if(m_pFont != NULL) return m_pFont;
  return g_pSystemFont;
}
