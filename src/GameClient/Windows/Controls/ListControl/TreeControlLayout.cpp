// TreeControlLayout.cpp: implementation of the CTreeControlLayout class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TreeControlLayout.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTreeControlLayout::CTreeControlLayout()
{
  m_pPlus = NULL; m_pMinus = NULL; m_dwIndentation = 5;
}

CTreeControlLayout::~CTreeControlLayout()
{
  ASSERT(m_pPlus == NULL); ASSERT(m_pMinus == NULL);
}

#ifdef _DEBUG

void CTreeControlLayout::AssertValid() const
{
  CListControlLayout::AssertValid();
}

void CTreeControlLayout::Dump(CDumpContext &dc) const
{
  CListControlLayout::Dump(dc);
}

#endif

BOOL CTreeControlLayout::Create(CArchiveFile CfgFile)
{
  return CListControlLayout::Create(CfgFile);
}

BOOL CTreeControlLayout::Create(CConfigFile *pCfgFile)
{
  CFG_BEGIN(pCfgFile);

  CFG_ANIMATION_O("Plus", m_pPlus);
  CFG_ANIMATION_O("Minus", m_pMinus);
  CFG_LONG_O("Indentation", m_dwIndentation);

  CFG_END();

  if(!CListControlLayout::Create(pCfgFile)) return FALSE;
  return TRUE;
}

void CTreeControlLayout::Delete()
{
  CListControlLayout::Delete();

  CFG_DELETE(m_pPlus);
  CFG_DELETE(m_pMinus);
}
