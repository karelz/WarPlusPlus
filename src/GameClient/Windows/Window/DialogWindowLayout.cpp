// DialogWindowLayout.cpp: implementation of the CDialogWindowLayout class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DialogWindowLayout.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CDialogWindowLayout, CCaptionWindowLayout)

CDialogWindowLayout::CDialogWindowLayout()
{
  m_dwCloseRightMargin = 20;
  m_dwCloseTopMargin = 5;

  m_dwCloseDummyLeft = 0;
  m_dwCloseDummyRight = 0;
  m_dwCloseDummyTop = 0;
  m_dwCloseDummyBottom = 0;
}

CDialogWindowLayout::~CDialogWindowLayout()
{
}

#ifdef _DEBUG

void CDialogWindowLayout::AssertValid() const
{
  CCaptionWindowLayout::AssertValid();
}

void CDialogWindowLayout::Dump(CDumpContext &dc) const
{
  CCaptionWindowLayout::Dump(dc);
}

#endif


BOOL CDialogWindowLayout::Create(CConfigFile *pCfgFile)
{
  CFG_BEGIN(pCfgFile);

  CString strCloseButtonConfig;
  strCloseButtonConfig = cfg_Section.GetString("CloseButton");
  m_CloseButtonLayout.Create(cfg_Archive.CreateFile(cfg_strPath + strCloseButtonConfig));

  CFG_LONG_O("CloseRightMargin", m_dwCloseRightMargin);
  CFG_LONG_O("CloseTopMargin", m_dwCloseTopMargin);

  CFG_LONG_O("CloseDummyLeft", m_dwCloseDummyLeft);
  CFG_LONG_O("CloseDummyRight", m_dwCloseDummyRight);
  CFG_LONG_O("CloseDummyTop", m_dwCloseDummyTop);
  CFG_LONG_O("CloseDummyBottom", m_dwCloseDummyBottom);

  CFG_END();

  return CCaptionWindowLayout::Create(pCfgFile);
}

void CDialogWindowLayout::Delete()
{
  m_CloseButtonLayout.Delete();

  CCaptionWindowLayout::Delete();
}

BOOL CDialogWindowLayout::Create(CArchiveFile CfgFile)
{
  return CCaptionWindowLayout::Create(CfgFile);
}
