// ToolbarButtonInfo.cpp: implementation of the CToolbarButtonInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ToolbarButtonInfo.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CToolbarButtonInfo::CToolbarImageManager *CToolbarButtonInfo::m_pImageManager = NULL;

IMPLEMENT_DYNAMIC(CToolbarButtonInfo, CObject);

CToolbarButtonInfo::CToolbarButtonInfo()
{
  m_dwParamsCount = 0;
  m_pParameters = NULL;
  m_pButtonIcon = NULL;
  m_nKeyAccChar = 0;
  m_dwKeyAccFlags = 0;
}

CToolbarButtonInfo::~CToolbarButtonInfo()
{
  ASSERT(m_pParameters == NULL);
  ASSERT(m_dwParamsCount == 0);
}

#ifdef _DEBUG

void CToolbarButtonInfo::AssertValid() const
{
  CObject::AssertValid();
}

void CToolbarButtonInfo::Dump(CDumpContext &dc) const
{
  CObject::Dump(dc);
}

#endif

// Creates the button info from config file section
// Returns TRUE if the creation was successfull
BOOL CToolbarButtonInfo::Create(CDataArchive Archive, CConfigFileSection *pSection, CToolbarsErrorOutput *pErrorOutput,
                                bool bCanBeSeparator)
{
  // read the caption
  if(pSection->IsVariable("Caption")){
    m_strCaption = pSection->GetString("Caption");
  }
  else{
    m_strCaption.Empty();
  }

  // If empty caption -> we're separator
  if(m_strCaption.IsEmpty() && bCanBeSeparator) return TRUE;

  // read the function name
  m_strFunctionName = pSection->GetString("Function");

  // read name of the image file
  if(!pSection->IsVariable("Icon")){
    m_pButtonIcon = NULL;
  }
  else{
    CString strFileName;

    strFileName = pSection->GetString("Icon");

    // Add image to image manager
    m_pButtonIcon = m_pImageManager->AddImage(Archive, strFileName);
  }

  // read the accelerator
  if ( pSection->IsVariable ( "Accelerator" ) )
  {
    CString strAcc = pSection->GetString ( "Accelerator" );

    if ( strAcc.Left ( 7 ).CompareNoCase ( "Ctrl + " ) == 0 )
    {
      m_dwKeyAccFlags = CKeyboard::CtrlDown;
      CString strHlp = strAcc.Mid ( 7, 1 );
      strHlp.MakeUpper ();
      m_nKeyAccChar = strHlp [ 0 ];
    }
    else if ( strAcc.Left ( 8 ).CompareNoCase ( "Shift + " ) == 0 )
    {
      m_dwKeyAccFlags = CKeyboard::ShiftDown;
      CString strHlp = strAcc.Mid ( 8, 1 );
      strHlp.MakeUpper ();
      m_nKeyAccChar = strHlp [ 0 ];
    }
    else
    {
      m_dwKeyAccFlags = 0;
      CString strHlp = strAcc.Mid ( 0, 1 );
      strHlp.MakeUpper ();
      m_nKeyAccChar = strHlp [ 0 ];
    }
  }
  else
  {
    m_nKeyAccChar = 0;
    m_dwKeyAccFlags = 0;
  }

  // read number of parameters
  if(pSection->IsVariable("ParamCount")){
    m_dwParamsCount = pSection->GetInteger("ParamCount");
  }
  else{
    m_dwParamsCount = 0;
  }

  // read parameters
  if(m_dwParamsCount > 0){
    // Allocate array
    m_pParameters = new SParameter[m_dwParamsCount];
    // Fill it with empty data
    {
      DWORD dwParam;
      for(dwParam = 0; dwParam < m_dwParamsCount; dwParam++){
        m_pParameters[dwParam].m_eParamType = ParamType_None;
      }
    }

    // Read the params
    {
      CString strParamVariable;
      DWORD dwParam;
      SParameter *pParameter;
      for(dwParam = 0; dwParam < m_dwParamsCount; dwParam++){
        // Format the name of the variable
        strParamVariable.Format("Param%d", dwParam + 1);

        // Does it exists
        if(!pSection->IsVariable(strParamVariable)){
          // If no -> error
          pErrorOutput->WriteError("Missing parameter for function");
          return FALSE;
        }

        pParameter = &(m_pParameters[dwParam]);

        // Try to read it as string
        try{
          CString strValue;
          strValue = pSection->GetString(strParamVariable);

          pParameter->m_eParamType = ParamType_String;
          pParameter->m_lpszValue = new char[strValue.GetLength() + 1];
          strcpy(pParameter->m_lpszValue, strValue);

          continue;
        }
        catch(CException *e){
          e->Delete();
        }

        // Try to read it as integer
        try{
          pParameter->m_nValue = pSection->GetInteger(strParamVariable);
          pParameter->m_eParamType = ParamType_Integer;

          continue;
        }
        catch(CException *e){
          e->Delete();
        }

        // Try to read it as boolean
        try{
          pParameter->m_bValue = pSection->GetBoolean(strParamVariable);
          pParameter->m_eParamType = ParamType_Bool;

          continue;
        }
        catch(CException *e){
          e->Delete();
        }

        // Try to read it as real
        try{
          pParameter->m_dbValue = pSection->GetReal(strParamVariable);
          pParameter->m_eParamType = ParamType_Real;

          continue;
        }
        catch(CException *e){
          e->Delete();
        }
      }
    }
  }

  return TRUE;
}

// Deletes the object
void CToolbarButtonInfo::Delete()
{
  // Delete parameters
  {
    DWORD dwParam;
    SParameter *pParameter;
    for(dwParam = 0; dwParam < m_dwParamsCount; dwParam++){
      pParameter = &(m_pParameters[dwParam]);
      
      if(pParameter->m_eParamType == ParamType_String){
        delete pParameter->m_lpszValue;
      }
    }

    if(m_pParameters != NULL){
      delete m_pParameters;
      m_pParameters = NULL;
    }

    m_dwParamsCount = 0;
  }
}

// Inits the image manager
void CToolbarButtonInfo::Init()
{
  ASSERT(m_pImageManager == NULL);

  m_pImageManager = new CToolbarImageManager();
  m_pImageManager->Create();
}

// Closes the image manager
void CToolbarButtonInfo::Close()
{
  if(m_pImageManager != NULL){
    m_pImageManager->Delete();
    delete m_pImageManager;
    m_pImageManager = NULL;
  }
}

// Creates the image manager
void CToolbarButtonInfo::CToolbarImageManager::Create()
{
  m_pNodes = NULL;
}

// Deletes the image manager
void CToolbarButtonInfo::CToolbarImageManager::Delete()
{
  // Go through all nodes and delete them
  struct SImageNode *pNode = m_pNodes, *pDel;
  while(pNode){
    pDel = pNode;
    pNode = pNode->m_pNext;
    pDel->m_Animation.Delete();
    delete pDel;
  }
  m_pNodes = NULL;
}

// Adds new image to the manager
CAnimation *CToolbarButtonInfo::CToolbarImageManager::AddImage(CDataArchive Archive, CString strImageFileName)
{
  // Make the image file path (small hack, we'll just concatenate the strings)
  CString strFilePath;
  strFilePath = Archive.GetArchivePath() + strImageFileName;

  // Try to find the image in the list
  struct SImageNode *pNode = m_pNodes;
  while(pNode != NULL){
    if(pNode->m_strFilePath == strFilePath){
      // Found -> return existing
      return &(pNode->m_Animation);
    }
    pNode = pNode->m_pNext;
  }
  // not found -> create new
  struct SImageNode *pNewNode;

  pNewNode = new SImageNode;
  pNewNode->m_strFilePath = strFilePath;

  // try to open the image
  try{
    CArchiveFile file = Archive.CreateFile(strImageFileName);
    pNewNode->m_Animation.Create(file);
  }
  catch(CException *e){
    e->Delete();
    delete pNewNode;
    return NULL;
  }

  // OK success -> add it to the list
  pNewNode->m_pNext = m_pNodes;
  m_pNodes = pNewNode;

  // and return the surface
  return &(pNewNode->m_Animation);
}