// ToolbarInfo.cpp: implementation of the CToolbarInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ToolbarInfo.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CToolbarInfo, CObject);

// Constructor
CToolbarInfo::CToolbarInfo()
{
  // Set empty data
  m_dwButtonsCount = 0;
  m_pButtons = NULL;
  m_pNext = NULL;

  m_pRightClickPositionButton = NULL;
  m_pRightClickMyUnitButton = NULL;
  m_pRightClickEnemyUnitButton = NULL;
  m_pShiftRightClickPositionButton = NULL;
  m_pShiftRightClickMyUnitButton = NULL;
  m_pShiftRightClickEnemyUnitButton = NULL;
}

// Destructor
CToolbarInfo::~CToolbarInfo()
{
  // Assert the data is empty
  ASSERT ( m_dwButtonsCount == 0 );
  ASSERT ( m_pButtons == NULL );
  ASSERT ( m_pRightClickPositionButton == NULL );
  ASSERT ( m_pRightClickMyUnitButton == NULL );
  ASSERT ( m_pRightClickEnemyUnitButton == NULL );
  ASSERT ( m_pShiftRightClickPositionButton == NULL );
  ASSERT ( m_pShiftRightClickMyUnitButton == NULL );
  ASSERT ( m_pShiftRightClickEnemyUnitButton == NULL );
}

#ifdef _DEBUG

// Asserts object validity
void CToolbarInfo::AssertValid() const
{
  CObject::AssertValid();

  if ( m_dwButtonsCount > 0 )
  {
    ASSERT ( m_pButtons != NULL );
  }
}

// Dump object data
void CToolbarInfo::Dump(CDumpContext &dc) const
{
  CObject::Dump(dc);

  dc << "Button count : " << m_dwButtonsCount << "\n";
}

#endif

// Helper function for creating buttons from config files
CToolbarButtonInfo * CToolbarInfo::CreateButton ( CDataArchive Archive, CConfigFileSection SubSection,
                                                  CToolbarsErrorOutput * pErrorOutput, bool bCanBeSeparator )
{
  CToolbarButtonInfo * pButton = new CToolbarButtonInfo();

  try{
    // If some error occured during creation -> delete the button and ignore it
    if(!pButton->Create(Archive, &SubSection, pErrorOutput, bCanBeSeparator)){
      pButton->Delete();
      delete pButton;
      // Write our unit type to the error log
      CString strHlp;
      strHlp.Format("Previous error occured in definition of unit type %s.", m_strUnitTypeName);
      pErrorOutput->WriteError(strHlp);
    }
    else{
      // Successfull -> remember the button
      return pButton;
    }
  }
  catch(CException *e){
    pButton->Delete();
    delete pButton;
    
    // write the exception to the output
    pErrorOutput->WriteError(e);
    e->Delete();
    // Write our unit type to the error log
    CString strHlp;
    strHlp.Format("Previous error occured in definition of unit type %s.", m_strUnitTypeName);
    pErrorOutput->WriteError(strHlp);
  }

  return NULL;
}

// creation
// Creates the toolbar info from config file section
BOOL CToolbarInfo::Create(CDataArchive Archive, CString strUnitTypeName, CConfigFileSection *pSection, CToolbarsErrorOutput *pErrorOutput)
{
  // copy the unit type name
  m_strUnitTypeName = strUnitTypeName;

  // Create the right click buttons
  {
    CString strSubSection;

    // Right click on position
    {
      strSubSection = "RightClickPosition";
      if ( pSection->IsSubsection ( strSubSection ) )
      {
        m_pRightClickPositionButton = CreateButton ( Archive, pSection->GetSubsection ( strSubSection ), pErrorOutput, false );
      }
    }

    // Right click on my unit
    {
      strSubSection = "RightClickMyUnit";
      if ( pSection->IsSubsection ( strSubSection ) )
      {
        m_pRightClickMyUnitButton = CreateButton ( Archive, pSection->GetSubsection ( strSubSection ), pErrorOutput, false );
      }
    }

    // Right click on enemy unit
    {
      strSubSection = "RightClickEnemyUnit";
      if ( pSection->IsSubsection ( strSubSection ) )
      {
        m_pRightClickEnemyUnitButton = CreateButton ( Archive, pSection->GetSubsection ( strSubSection ), pErrorOutput, false );
      }
    }

    // Shift Right click on position
    {
      strSubSection = "ShiftRightClickPosition";
      if ( pSection->IsSubsection ( strSubSection ) )
      {
        m_pShiftRightClickPositionButton = CreateButton ( Archive, pSection->GetSubsection ( strSubSection ), pErrorOutput, false );
      }
    }

    // Shift Right click on my unit
    {
      strSubSection = "ShiftRightClickMyUnit";
      if ( pSection->IsSubsection ( strSubSection ) )
      {
        m_pShiftRightClickMyUnitButton = CreateButton ( Archive, pSection->GetSubsection ( strSubSection ), pErrorOutput, false );
      }
    }

    // Shift Right click on enemy unit
    {
      strSubSection = "ShiftRightClickEnemyUnit";
      if ( pSection->IsSubsection ( strSubSection ) )
      {
        m_pShiftRightClickEnemyUnitButton = CreateButton ( Archive, pSection->GetSubsection ( strSubSection ), pErrorOutput, false );
      }
    }
  }

  // Read number of the buttons on the toolbar
  if(!pSection->IsVariable("ButtonsCount")){
    CString strHlp;
    strHlp.Format("Variable ButtonsCount not found in section for unit type %s.", strUnitTypeName);
    pErrorOutput->WriteError(strHlp);
    return FALSE;
  }
  m_dwButtonsCount = pSection->GetInteger("ButtonsCount");

  if(m_dwButtonsCount > 0){
    // Allocate buttons array
    m_pButtons = (CToolbarButtonInfo **)new BYTE[sizeof(CToolbarButtonInfo *) * m_dwButtonsCount];
    memset(m_pButtons, 0, sizeof(CToolbarButtonInfo *) * m_dwButtonsCount);

    // go through all buttons and read them
    {
      DWORD dwButton;
      CString strSectionName;

      for(dwButton = 0; dwButton < m_dwButtonsCount; dwButton++){
        strSectionName.Format("Button%d", dwButton + 1);

        // If there is no such section -> ignore the button
        if(!pSection->IsSubsection(strSectionName)){
          CString strHlp;
          strHlp.Format("Missing subsection %s in definition of unit type %s.", strSectionName, strUnitTypeName);
          pErrorOutput->WriteError(strHlp);
          continue;
        }

        // Create the button
        // Copy the NULL as well
        m_pButtons [ dwButton ]  = CreateButton ( Archive, pSection->GetSubsection(strSectionName), pErrorOutput );
      }
    }
  }

  return TRUE;
}

// Deletes the toolbar info
void CToolbarInfo::Delete()
{
  // Delete buttons
  if(m_pButtons != NULL){
    DWORD dwButton;
    CToolbarButtonInfo *pButton;
    for(dwButton = 0; dwButton < m_dwButtonsCount; dwButton++){
      pButton = m_pButtons[dwButton];
      if(pButton == NULL) continue;

      pButton->Delete();
      delete pButton;
    }

    delete m_pButtons;
    m_pButtons = NULL;
    m_dwButtonsCount = 0;
  }

  // Delete right click buttons
  {
    if ( m_pRightClickPositionButton != NULL )
    {
      m_pRightClickPositionButton->Delete ();
      delete m_pRightClickPositionButton;
      m_pRightClickPositionButton = NULL;
    }
    if ( m_pRightClickMyUnitButton != NULL )
    {
      m_pRightClickMyUnitButton->Delete ();
      delete m_pRightClickMyUnitButton;
      m_pRightClickMyUnitButton = NULL;
    }
    if ( m_pRightClickEnemyUnitButton != NULL )
    {
      m_pRightClickEnemyUnitButton->Delete ();
      delete m_pRightClickEnemyUnitButton;
      m_pRightClickEnemyUnitButton = NULL;
    }
    if ( m_pShiftRightClickPositionButton != NULL )
    {
      m_pShiftRightClickPositionButton->Delete ();
      delete m_pShiftRightClickPositionButton;
      m_pShiftRightClickPositionButton = NULL;
    }
    if ( m_pShiftRightClickMyUnitButton != NULL )
    {
      m_pShiftRightClickMyUnitButton->Delete ();
      delete m_pShiftRightClickMyUnitButton;
      m_pShiftRightClickMyUnitButton = NULL;
    }
    if ( m_pShiftRightClickEnemyUnitButton != NULL )
    {
      m_pShiftRightClickEnemyUnitButton->Delete ();
      delete m_pShiftRightClickEnemyUnitButton;
      m_pShiftRightClickEnemyUnitButton = NULL;
    }
  }
}
