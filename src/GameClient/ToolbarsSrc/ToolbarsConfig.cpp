// ToolbarsConfig.cpp: implementation of the CToolbarsConfig class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ToolbarsConfig.h"
#include "ToolbarsErrorOutput.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CToolbarsConfig, CObject);

CToolbarsConfig::CToolbarsConfig()
{
  m_pToolbars = NULL;
  m_pDefaultToolbar = NULL;
  m_pEnemyToolbar = NULL;
  m_pNoSelectionToolbar = NULL;
}

CToolbarsConfig::~CToolbarsConfig()
{
  ASSERT(m_pToolbars == NULL);
  ASSERT(m_pDefaultToolbar == NULL);
  ASSERT(m_pEnemyToolbar == NULL);
  ASSERT(m_pNoSelectionToolbar == NULL);
}

#ifdef _DEBUG

void CToolbarsConfig::AssertValid() const
{
  CObject::AssertValid();
}

void CToolbarsConfig::Dump(CDumpContext &dc) const
{
  CObject::Dump(dc);
}

#endif


// creation
// Creates the toolbars objects from the archive
// The archive object must contain file named "Toolbars.txt"
void CToolbarsConfig::Create(CDataArchive Archive)
{
  CArchiveFile ToolbarsFile;
  CToolbarsErrorOutput ErrorOutput;
  CConfigFile CfgFile;

  // Open the main toolbars file
  ToolbarsFile = Archive.CreateFile("Toolbars.txt", CArchiveFile::modeRead | CFile::shareDenyWrite);
  CfgFile.Create(ToolbarsFile);

  // Open the error output file
  ErrorOutput.Create("Errors.log", Archive);

  // Initialize image manager
  CToolbarButtonInfo::Init();

  // now load the toolbars
  {
    CConfigFileSection RootSection = CfgFile.GetRootSection();
  
    DWORD dwSubSectionsCount = RootSection.GetSubsectionsCount();

    DWORD dwSubSection;
    CToolbarInfo *pInfo;
    CString strSubSection;

    for(dwSubSection = 0; dwSubSection < dwSubSectionsCount; dwSubSection++){
      strSubSection = RootSection.GetSubsectionName(dwSubSection);

      // Toolbar when no unit is selected
	  // All these functions will receive empty set of units
      if(strSubSection.CompareNoCase("noselection") == 0){
        // Read default toolbar
        CConfigFileSection SubSection = RootSection.GetSubsection(strSubSection);

        pInfo = new CToolbarInfo();

        try{
          if(!pInfo->Create(Archive, strSubSection, &SubSection, &ErrorOutput)){
            // Creation failed -> forget it
            pInfo->Delete();
            delete pInfo;
            pInfo = NULL;
          }
        }
        catch(CException *e){
          // Error -> forget that section
          // and write some message to the log
          ErrorOutput.WriteError(e);
          e->Delete();

          pInfo->Delete();
          delete pInfo;
          pInfo = NULL;
        }

        if ( pInfo != NULL )
        {
          // Remember the default toolbar
          m_pNoSelectionToolbar = pInfo;
        }

        continue;
      }

      // Default toolbar (when some more units with different types)
      if(strSubSection.CompareNoCase("default") == 0){
        // Read default toolbar
        CConfigFileSection SubSection = RootSection.GetSubsection(strSubSection);

        pInfo = new CToolbarInfo();

        try{
          if(!pInfo->Create(Archive, strSubSection, &SubSection, &ErrorOutput)){
            // Creation failed -> forget it
            pInfo->Delete();
            delete pInfo;
            pInfo = NULL;
          }
        }
        catch(CException *e){
          // Error -> forget that section
          // and write some message to the log
          ErrorOutput.WriteError(e);
          e->Delete();

          pInfo->Delete();
          delete pInfo;
          pInfo = NULL;
        }

        if ( pInfo != NULL )
        {
          // Remember the default toolbar
          m_pDefaultToolbar = pInfo;
        }

        continue;
      }

      // Enemy toolbar
      if(strSubSection.CompareNoCase("enemy") == 0){
        // Read default toolbar
        CConfigFileSection SubSection = RootSection.GetSubsection(strSubSection);

        pInfo = new CToolbarInfo();

        try{
          if(!pInfo->Create(Archive, strSubSection, &SubSection, &ErrorOutput)){
            // Creation failed -> forget it
            pInfo->Delete();
            delete pInfo;
            pInfo = NULL;
          }
        }
        catch(CException *e){
          // Error -> forget that section
          // and write some message to the log
          ErrorOutput.WriteError(e);
          e->Delete();

          pInfo->Delete();
          delete pInfo;
          pInfo = NULL;
        }

        if ( pInfo != NULL )
        {
          // Remember the enemy toolbar
          m_pEnemyToolbar = pInfo;
        }

        continue;
      }

      // If it isn't any special sub section -> read it as unit type sub section
      {
        CConfigFileSection SubSection = RootSection.GetSubsection(strSubSection);

        pInfo = new CToolbarInfo();

        try{
          if(!pInfo->Create(Archive, strSubSection, &SubSection, &ErrorOutput)){
            // Creation failed -> forget it
            pInfo->Delete();
            delete pInfo;
            pInfo = NULL;
          }
        }
        catch(CException *e){
          // Error -> forget that section
          // and write some message to the log
          ErrorOutput.WriteError(e);
          e->Delete();

          pInfo->Delete();
          delete pInfo;
          pInfo = NULL;
        }

        if ( pInfo != NULL )
        {
          // All was successfull -> add it to the list
          pInfo->m_pNext = m_pToolbars;
          m_pToolbars = pInfo;
        }
      }
    }
  }

  // Delete the config file
  CfgFile.Delete();

  // Close the error output
  ErrorOutput.Delete();
}

// Deletes the object
void CToolbarsConfig::Delete()
{
  // Delete toolbar infos
  {
    CToolbarInfo *pInfo, *pDel;

    pInfo = m_pToolbars;
    while(pInfo != NULL){
      pDel = pInfo;

      pInfo = pInfo->m_pNext;

      pDel->Delete();
      delete pDel;
    }

    m_pToolbars = NULL;
  }

  if(m_pNoSelectionToolbar != NULL){
    m_pNoSelectionToolbar->Delete();
    delete m_pNoSelectionToolbar;
    m_pNoSelectionToolbar = NULL;
  }
  
  if(m_pDefaultToolbar != NULL){
    m_pDefaultToolbar->Delete();
    delete m_pDefaultToolbar;
    m_pDefaultToolbar = NULL;
  }

  if(m_pEnemyToolbar != NULL){
    m_pEnemyToolbar->Delete();
    delete m_pEnemyToolbar;
    m_pEnemyToolbar = NULL;
  }

  // Close the image manager
  CToolbarButtonInfo::Close();
}

// Returns toolbar for given unit type name
CToolbarInfo *CToolbarsConfig::GetUnitToolbar(CString strUnitTypeName)
{
  // Go through the toolbars and find the righ one
  CToolbarInfo *pInfo = m_pToolbars;
  while(pInfo != NULL){
    if(strUnitTypeName.CompareNoCase(pInfo->GetUnitTypeName()) == 0)
      break;

    pInfo = pInfo->m_pNext;
  }

  if(pInfo == NULL){
    // Not found
    return m_pDefaultToolbar;
  }
  return pInfo;
}
