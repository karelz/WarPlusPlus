// Layouts.cpp: implementation of the CLayouts class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Layouts.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CLayouts, CObject)

CLayouts *CLayouts::m_pDefaults = NULL;

CLayouts::CLayouts()
{
  m_bLoaded = FALSE;
}

CLayouts::~CLayouts()
{

}

#ifdef _DEBUG

void CLayouts::AssertValid() const
{
  CObject::AssertValid();
  ASSERT(m_bLoaded);
}

void CLayouts::Dump(CDumpContext &dc) const
{
  CObject::Dump(dc);
}

#endif


BOOL CLayouts::Create(CArchiveFile CfgFile)
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

BOOL CLayouts::Create(CConfigFile *pCfgFile)
{
  CFG_BEGIN(pCfgFile);

  CString strConfig;
  strConfig = cfg_Section.GetString("CaptionWindowLayout");
  m_CaptionWindowLayout.Create(cfg_Archive.CreateFile(cfg_strPath + strConfig));

  strConfig = cfg_Section.GetString("DialogWindowLayout");
  m_DialogWindowLayout.Create(cfg_Archive.CreateFile(cfg_strPath + strConfig));

  strConfig = cfg_Section.GetString("EditBoxLayout");
  m_EditBoxLayout.Create(cfg_Archive.CreateFile(cfg_strPath + strConfig));

  strConfig = cfg_Section.GetString("ListControlLayout");
  m_ListControlLayout.Create(cfg_Archive.CreateFile(cfg_strPath + strConfig));

  strConfig = cfg_Section.GetString("VerticalScrollControlLayout");
  m_VerticalScrollControlLayout.Create(cfg_Archive.CreateFile(cfg_strPath + strConfig));

  strConfig = cfg_Section.GetString("HorizontalScrollControlLayout");
  m_HorizontalScrollControlLayout.Create(cfg_Archive.CreateFile(cfg_strPath + strConfig));

  strConfig = cfg_Section.GetString("StaticTextLayout");
  m_StaticTextLayout.Create(cfg_Archive.CreateFile(cfg_strPath + strConfig));

  strConfig = cfg_Section.GetString("TextButtonLayout");
  m_TextButtonLayout.Create(cfg_Archive.CreateFile(cfg_strPath + strConfig));

  strConfig = cfg_Section.GetString("PopupMenuLayout");
  m_PopupMenuLayout.Create(cfg_Archive.CreateFile(cfg_strPath + strConfig));

  strConfig = cfg_Section.GetString("ToolTipLayout");
  m_ToolTipLayout.Create(cfg_Archive.CreateFile(cfg_strPath + strConfig));

  CFG_END();

  m_bLoaded = TRUE;
  return TRUE;
}

void CLayouts::Delete()
{
  m_bLoaded = FALSE;

  m_CaptionWindowLayout.Delete();
  m_DialogWindowLayout.Delete();
  m_EditBoxLayout.Delete();
  m_ListControlLayout.Delete();
  m_VerticalScrollControlLayout.Delete();
  m_HorizontalScrollControlLayout.Delete();
  m_StaticTextLayout.Delete();
  m_TextButtonLayout.Delete();
  m_PopupMenuLayout.Delete();
  m_ToolTipLayout.Delete();
}

CCaptionWindowLayout * CLayouts::GetCaptionWindowLayout()
{
  ASSERT_VALID(this);
  return &m_CaptionWindowLayout;
}

CDialogWindowLayout * CLayouts::GetDialogWindowLayout()
{
  ASSERT_VALID(this);
  return &m_DialogWindowLayout;
}

CEditBoxLayout * CLayouts::GetEditBoxLayout()
{
  ASSERT_VALID(this);
  return &m_EditBoxLayout;
}

CListControlLayout * CLayouts::GetListControlLayout()
{
  ASSERT_VALID(this);
  return &m_ListControlLayout;
}

CScrollControlLayout * CLayouts::GetVerticalScrollControlLayout()
{
  ASSERT_VALID(this);
  return &m_VerticalScrollControlLayout;
}

CScrollControlLayout * CLayouts::GetHorizontalScrollControlLayout()
{
  ASSERT_VALID(this);
  return &m_HorizontalScrollControlLayout;
}

CStaticTextLayout * CLayouts::GetStaticTextLayout()
{
  ASSERT_VALID(this);
  return &m_StaticTextLayout;
}

CTextButtonLayout * CLayouts::GetTextButtonLayout()
{
  ASSERT_VALID(this);
  return &m_TextButtonLayout;
}

CFrameWindowLayout * CLayouts::GetPopupMenuLayout()
{
  ASSERT_VALID(this);
  return &m_PopupMenuLayout;
}

CToolTipLayout * CLayouts::GetToolTipLayout()
{
  ASSERT_VALID(this);
  return &m_ToolTipLayout;
}

BOOL CLayouts::Init(CArchiveFile CfgFile)
{
  ASSERT(m_pDefaults == NULL);

  m_pDefaults = new CLayouts();
  if(!m_pDefaults->Create(CfgFile)){
    delete m_pDefaults;
    return FALSE;
  }
  return TRUE;
}

void CLayouts::Close()
{
  if(m_pDefaults != NULL){
    m_pDefaults->Delete();

    delete m_pDefaults;
    m_pDefaults = NULL;
  }
}
