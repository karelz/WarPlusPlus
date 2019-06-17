#include "stdafx.h"
#include "CUnitAnimation.h"

#include "..\Common\CommonExceptions.h"

#include "CMap.h"
#include "..\GameClientGlobal.h"

IMPLEMENT_DYNAMIC(CCUnitAnimation, CCachedObject)

CUnitAnimationCache *CCUnitAnimation::m_pAnimationCache = NULL;
CCUnitSurfaceManager *CCUnitAnimation::m_pUnitSurfaceManager = NULL;

// Constructor
CCUnitAnimation::CCUnitAnimation()
{
  m_aFrames = NULL;
  m_dwFramesCount = 0;
  m_dwSpeed = 0;
  m_nXGraphicalOffset = 0;
  m_nYGraphicalOffset = 0;
}

// Destructor
CCUnitAnimation::~CCUnitAnimation()
{
  // just delete it
  ASSERT(m_dwFramesCount == 0);
}

#ifdef _DEBUG

void CCUnitAnimation::AssertValid() const
{
  CCachedObject::AssertValid();
}

void CCUnitAnimation::Dump(CDumpContext &dc) const
{
  CCachedObject::Dump(dc);
}

#endif

// Implementation --------------------------------------------------

// Creates the object enevelope (no graphics loaded)
void CCUnitAnimation::Create(CArchiveFile File, DWORD dwColor)
{
  // just copy the file -> no allocation and so on
  m_File = File;
  m_dwColor = dwColor;

  ASSERT_VALID(m_pAnimationCache);
  m_pAnimationCache->InsertObject(this);
}

// Deletes the object
void CCUnitAnimation::Delete()
{
  ASSERT_VALID(m_pAnimationCache);
  m_pAnimationCache->RemoveObject(this);

  // unload us
  Unload();

  // close the file
  m_File.Close();
}


// Loads the graphics
void CCUnitAnimation::Load()
{
  CString strFileName;
  char *pExt;

  // set default coloring params
  // it means the red color and some tolerances
  m_sColoringParams.m_dwSourceColor = RGB32(255, 0, 0);
  m_sColoringParams.m_dbHTolerance = 0.05;
  m_sColoringParams.m_dbSTolerance = 0.1;
  m_sColoringParams.m_dbVTolerance = 0.2;
  // use the dest color
  m_sColoringParams.m_dwDestColor = m_dwColor;

  try{

  strFileName = m_File.GetFileName();
  pExt = strrchr(strFileName, '.');
  if((pExt != NULL) && (!strcmp(pExt, ".anim"))){
    // ok it's an anim file -> parse it
    CConfigFile AnimFile;

    AnimFile.Create(m_File);

    LoadAnim(&AnimFile);

    AnimFile.Delete();
  }
  else{
    CCUnitSurface *pFrame = NULL;

    try{
      // try to open the file as static image
      ASSERT(m_pUnitSurfaceManager != NULL);
      pFrame = m_pUnitSurfaceManager->GetSurface(m_File, &m_sColoringParams, 0, 0, 0 );
      ASSERT(pFrame != NULL);

      // OK we succeded -> create one frame animation
      m_dwSpeed = 0;
    
      // allocate one frame array
      m_aFrames = (CCUnitSurface **)new LPVOID[1];
      m_aFrames[0] = pFrame;
      m_dwFramesCount = 1;

      m_bLoop = FALSE; // static images can't loop
    }
    catch(CException *){
      if(pFrame != NULL) m_pUnitSurfaceManager->Release(pFrame);
      throw;
    }
  }

  }
  catch(CException *){
    throw;
  }
}

// Load anim file
void CCUnitAnimation::LoadAnim(CConfigFile *pCfgFile)
{
  DWORD dwFrame;
  CString strPath, strFrame, strFrameTag;
  CDataArchive Archive;
  CCUnitSurface *pFrame = NULL;

  CConfigFileSection RootSection = pCfgFile->GetRootSection();

  try{

    // Get number of frames
    m_dwFramesCount = RootSection.GetInteger("FrameNum");
    if(m_dwFramesCount <= 0){
      CString str;
      str.Format("Illegal number of frames specified in '%s'.", pCfgFile->GetFile().GetFilePath());
      throw new CStringException(str);
    }

    // get the speed
    m_dwSpeed = RootSection.GetInteger("Speed");
    if(m_dwSpeed < 0){
      CString str;
      str.Format("Illegal speed specified in '%s'.", pCfgFile->GetFile().GetFilePath());
      throw new CStringException(str);
    }

    if(RootSection.IsVariable("Loop")){
      m_bLoop = RootSection.GetBoolean("Loop");
    }
    else{
      m_bLoop = FALSE;
    }

    // Get Graphical offsets
    if(RootSection.IsVariable("XOffset"))
      m_nXGraphicalOffset = RootSection.GetInteger("XOffset");
    if(RootSection.IsVariable("YOffset"))
      m_nYGraphicalOffset = RootSection.GetInteger("YOffset");

    if(RootSection.IsSubsection("Coloring"))
    {
      CConfigFileSection ColoringSection = RootSection.GetSubsection("Coloring");
      // get source color
      if(ColoringSection.IsVariable("SourceColor")){
        m_sColoringParams.m_dwSourceColor = ColoringSection.GetInteger("SourceColor");
      }

      // get tolerances
      if(ColoringSection.IsVariable("HTolerance"))
      { m_sColoringParams.m_dbHTolerance = ColoringSection.GetReal("HTolerance"); }
      if(ColoringSection.IsVariable("STolerance"))
      { m_sColoringParams.m_dbSTolerance = ColoringSection.GetReal("STolerance"); }
      if(ColoringSection.IsVariable("VTolerance"))
      { m_sColoringParams.m_dbVTolerance = ColoringSection.GetReal("VTolerance"); }
    }

    // create the path
    strPath = pCfgFile->GetFile().GetFileDir();
    if(!strPath.IsEmpty()) strPath += "\\";

    // get the archive from the anim file
    Archive = pCfgFile->GetFile().GetDataArchive();

    // create frames
    m_aFrames = (CCUnitSurface **)new LPVOID[m_dwFramesCount];

    for(dwFrame = 0; dwFrame < m_dwFramesCount; dwFrame++){
		m_aFrames[dwFrame]=NULL;
	 }
    
    int nXOffset, nYOffset;
    DWORD dwFrameDelay;
    // read frames
    for(dwFrame = 0; dwFrame < m_dwFramesCount; dwFrame++){
      
      // get the frame name
      strFrameTag.Format("Frame%d", dwFrame);
      strFrame = RootSection.GetString(strFrameTag);
      strFrame = strPath + strFrame;
      
      nXOffset = 0;
      strFrameTag.Format ( "Frame%d_XOffset", dwFrame );
      if ( RootSection.IsVariable ( strFrameTag ) )
        nXOffset = RootSection.GetInteger ( strFrameTag );

      nYOffset = 0;
      strFrameTag.Format ( "Frame%d_YOffset", dwFrame );
      if ( RootSection.IsVariable ( strFrameTag ) )
        nYOffset = RootSection.GetInteger ( strFrameTag );

      dwFrameDelay = 0;
      strFrameTag.Format ( "Frame%d_Delay", dwFrame );
      if ( RootSection.IsVariable ( strFrameTag ) )
        dwFrameDelay = RootSection.GetInteger ( strFrameTag );

      // try to open the frame
      ASSERT(m_pUnitSurfaceManager != NULL);
      pFrame = m_pUnitSurfaceManager->GetSurface(Archive.CreateFile(strFrame), &m_sColoringParams, nXOffset, nYOffset,
        dwFrameDelay );
      ASSERT(pFrame != NULL);

      m_aFrames[dwFrame] = pFrame;
      pFrame = NULL;
    }

  }
  catch(CException *){
    if(pFrame != NULL) m_pUnitSurfaceManager->Release(pFrame);
    Unload();
    throw;
  }
}

// Unload the graphics
void CCUnitAnimation::Unload()
{
  if(m_aFrames != NULL){
    // delete all frames
    {
      DWORD i;
      for(i = 0; i < m_dwFramesCount; i++){
        if(m_aFrames[i] != NULL){
          m_pUnitSurfaceManager->Release(m_aFrames[i]);
          m_aFrames[i] = NULL;
        }
      }
    }

    // delete the frames array
    delete m_aFrames;
    m_aFrames = NULL;
  }

  // no frames
  m_dwFramesCount = 0;
  m_dwSpeed = 0;
}

void CCUnitAnimation::InitCache()
{
  ASSERT(m_pAnimationCache == NULL);
  m_pAnimationCache = new CUnitAnimationCache();
  m_pAnimationCache->Create();
}

void CCUnitAnimation::CloseCache()
{
  if(m_pAnimationCache == NULL) return;
  ASSERT_VALID(m_pAnimationCache);
  m_pAnimationCache->Delete();
  delete m_pAnimationCache;
  m_pAnimationCache = NULL;
}

void CCUnitAnimation::InitManager()
{
  ASSERT(m_pUnitSurfaceManager == NULL);
  m_pUnitSurfaceManager = new CCUnitSurfaceManager();
  m_pUnitSurfaceManager->Create();
}

void CCUnitAnimation::CloseManager()
{
  if(m_pUnitSurfaceManager == NULL) return;
  m_pUnitSurfaceManager->Delete();
  delete m_pUnitSurfaceManager;
  m_pUnitSurfaceManager = NULL;
}