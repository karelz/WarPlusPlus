// Animation.cpp: implementation of the CAnimation class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Animation.h"
#include "..\Timer\Timer.h"

#include "..\..\Common\CommonExceptions.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CAnimation, CObject);

CAnimation::CAnimation()
{
  m_bLoop = TRUE;
  m_dwFrameNum = 0;
  m_dwSpeed = 0;
  m_szSize.cx = 0; m_szSize.cy = 0;
}

CAnimation::~CAnimation()
{
  ASSERT(m_dwFrameNum == 0);
  ASSERT(m_aFrames.GetSize() == 0);
}

#ifdef _DEBUG

void CAnimation::AssertValid() const
{
  CObject::AssertValid();
}

void CAnimation::Dump(CDumpContext &dc) const
{
  CObject::Dump(dc);
  dc << "Frame number : " << m_dwFrameNum << "  Speed : " << m_dwSpeed << "\n";
}

#endif

BOOL CAnimation::Create(DWORD dwFrameNum, DWORD dwSpeed, BOOL bLoop)
{
  DWORD i;
  
  Delete();

  if(dwFrameNum == 0) return TRUE;

  m_dwSpeed = dwSpeed;
  m_aFrames.SetSize(dwFrameNum);
  for(i = 0; i < dwFrameNum; i++){
    m_aFrames[i] = NULL;
  }
  m_dwFrameNum = dwFrameNum;

  m_bLoop = bLoop;

  RecomputeSize();

  return TRUE;
}

BOOL CAnimation::Create(CArchiveFile File)
{
  CString strFileName;
  char *pExt;

  strFileName = File.GetFileName();
  pExt = strrchr(strFileName, '.');
  if((pExt != NULL) && (!strcmp(pExt, ".anim"))){
    // ok it's an anim file -> parse it
    CConfigFile AnimFile;

    AnimFile.Create(File);

    Create(&AnimFile);
  }
  else{
    CImageSurface *pFrame;

    // try to open the file as static image
    try{
      pFrame = new CImageSurface();
      if(!pFrame->Create(File)){
        delete pFrame;
        return FALSE;
      }
    }
    catch(CException *){
      delete pFrame;
      throw;
    }

    // OK we succeded -> create one frame animation
    m_dwFrameNum = 1;
    m_dwSpeed = 0;
    m_aFrames.SetSize(1);
    m_aFrames[0] = pFrame;
  }

  RecomputeSize();

  return TRUE;
}

void CAnimation::Delete()
{
  int i;

  // delete all frames
  for(i = 0; i < m_aFrames.GetSize(); i++){
    if(m_aFrames[i] == NULL) continue;
    m_aFrames[i]->Delete();
    delete m_aFrames[i];
  }
  m_aFrames.RemoveAll();

  m_dwFrameNum = 0;
  m_dwSpeed = 0;
}

DWORD CAnimation::GetFrameNum()
{
  return m_dwFrameNum;
}

CDDrawSurface * CAnimation::GetFrame(DWORD dwIndex)
{
  if(dwIndex >= m_dwFrameNum) return NULL;
  return m_aFrames[dwIndex];
}

DWORD CAnimation::GetSpeed()
{
  return m_dwSpeed;
}

void CAnimation::SetFrame(DWORD dwIndex, CDDrawSurface *pSurface)
{
  if(dwIndex >= m_dwFrameNum) return;

  if(m_aFrames[dwIndex] != NULL){
    m_aFrames[dwIndex]->Delete();
    delete m_aFrames[dwIndex];
    m_aFrames[dwIndex] = NULL;
  }

#ifdef _DEBUG
  if(pSurface != NULL) ASSERT_VALID(pSurface);
#endif

  m_aFrames[dwIndex] = pSurface;

  RecomputeSize();
}

void CAnimation::SetSpeed(DWORD dwSpeed)
{
  m_dwSpeed = dwSpeed;
}

CSize CAnimation::GetSize()
{
  return m_szSize;
}

BOOL CAnimation::IsTransparent()
{
  DWORD i;

  for(i = 0; i < m_dwFrameNum; i++){
    if(m_aFrames[i] != NULL){
      if(m_aFrames[i]->IsTransparent()) return TRUE;
    }
  }

  return FALSE;
}

void CAnimation::RecomputeSize()
{
  CSize size;
  DWORD i;

  size.cx = 0; size.cy = 0;
  for(i = 0; i < m_dwFrameNum; i++){
    if(m_aFrames[i] != NULL){
      if(m_aFrames[i]->GetWidth() > (DWORD)size.cx)
        size.cx = m_aFrames[i]->GetWidth();
      if(m_aFrames[i]->GetHeight() > (DWORD)size.cy)
        size.cy = m_aFrames[i]->GetHeight();
    }
  }

  m_szSize = size;
}

IMPLEMENT_DYNAMIC(CAnimationInstance, CObserver)

BEGIN_OBSERVER_MAP(CAnimationInstance, CObserver)
  BEGIN_TIMER()
    ON_TIMETICK()
  END_TIMER()
END_OBSERVER_MAP(CAnimationInstance, CObserver)

CAnimationInstance::CAnimationInstance(){
  m_pAnimation = NULL; m_bLoop = TRUE; m_dwFrame = 0;
  m_pObserver = NULL; m_dwNotID = 0;
}

CAnimationInstance::CAnimationInstance(CAnimation *pAnimation, CObserver *pObserver, int nLoop, DWORD dwNotID)
{
  ASSERT_VALID(pAnimation);
  m_pAnimation = pAnimation; m_dwFrame = 0;
  if(nLoop == -1) m_bLoop = m_pAnimation->GetLoop();
  else m_bLoop = (nLoop == 0)?FALSE:TRUE;
  m_pObserver = pObserver; m_dwNotID = dwNotID;
}

CAnimationInstance::~CAnimationInstance()
{
  if(g_pTimer != NULL) g_pTimer->Disconnect(this);
}

#ifdef _DEBUG

void CAnimationInstance::AssertValid() const
{
  CObserver::AssertValid();
}

void CAnimationInstance::Dump(CDumpContext &dc) const
{
  CObserver::Dump(dc);
  dc << m_pAnimation;
}

#endif

BOOL CAnimationInstance::Create(CAnimation *pAnimation, CObserver *pObserver, int nLoop, DWORD dwNotID)
{
  m_pAnimation = pAnimation; m_dwFrame = 0;
  if((nLoop == -1) && (pAnimation != NULL)) m_bLoop = m_pAnimation->GetLoop();
  else m_bLoop = (nLoop == 0)?FALSE:TRUE;
  m_pObserver = pObserver; m_dwNotID = dwNotID;
  return TRUE;
}

void CAnimationInstance::Play()
{
  if(m_pAnimation == NULL) return;
  m_dwFrame = 0;
  if(m_pAnimation->GetSpeed() == 0) return;
  g_pTimer->Connect(this, m_pAnimation->GetSpeed());
}

void CAnimationInstance::Stop()
{
  if(m_pAnimation == NULL) return;
//  if(m_pAnimation->GetSpeed() == 0) return;
  if(g_pTimer != NULL) g_pTimer->Disconnect(this);
}

void CAnimationInstance::OnTimeTick(DWORD dwTime)
{
  DWORD dwFrame = m_dwFrame;
  
  NextFrame();
  
  if(m_dwFrame != dwFrame){
    if(m_pObserver != NULL){
      m_pObserver->Perform(m_dwNotID, E_REPAINT, (DWORD)this);
    }
  }
}

BOOL CAnimation::Create(CConfigFile *pAnimFile)
{
  DWORD dwFrame;
  CString strPath, strFrame, strFrameTag;
  CDataArchive Archive;
  CImageSurface *pFrame = NULL;

  CConfigFileSection Section = pAnimFile->GetRootSection();
  
  try{
    // read the frames number
    m_dwFrameNum = Section.GetInteger("FrameNum");
    if(m_dwFrameNum <= 0){
      CString str;
      str.Format("Illegal number of frames specified in '%s'.", pAnimFile->GetFile().GetFilePath());
      throw new CStringException(str);
    }

    if(Section.IsVariable("Loop")){
      m_bLoop = Section.GetBoolean("Loop");
    }
    
    // read the speed
    m_dwSpeed = Section.GetInteger("Speed");
    if(m_dwSpeed < 0){
      CString str;
      str.Format("Illegal speed specified in '%s'.", pAnimFile->GetFile().GetFilePath());
      throw new CStringException(str);
    }
    
    // create the path
    strPath = pAnimFile->GetFile().GetFileDir();
    if(!strPath.IsEmpty()) strPath += "\\";

    // get the archive from the anim file
    Archive = pAnimFile->GetFile().GetDataArchive();
    
    // create frames
    m_aFrames.SetSize(m_dwFrameNum);
    for(dwFrame = 0; dwFrame < m_dwFrameNum; dwFrame++)
      m_aFrames[dwFrame] = NULL;
    
    // read frames
    for(dwFrame = 0; dwFrame < m_dwFrameNum; dwFrame++){
      
      // get the frame name
      strFrameTag.Format("Frame%d", dwFrame);
      strFrame = Section.GetString(strFrameTag);
      strFrame = strPath + strFrame;
      
      // try to open the frame
      pFrame = new CImageSurface();
      if(!pFrame->Create(Archive.CreateFile(strFrame))){
        CString str;
        str.Format("Can't create the frame image '%s'.", strFrame);
        throw new CStringException(str);
      }
      m_aFrames[dwFrame] = pFrame;
      pFrame = NULL;
    }
    
  }
  catch(CException *){
    if(pFrame) delete pFrame;
    Delete();
    throw;
  }

  return TRUE;
}
