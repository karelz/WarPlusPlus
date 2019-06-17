// EUnitAnimation.cpp: implementation of the CEUnitAnimation class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "EUnitAnimation.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CEUnitAnimation, CObject)

CEUnitAnimation::CEUnitAnimation()
{
}

CEUnitAnimation::~CEUnitAnimation()
{
}

#ifdef _DEBUG

void CEUnitAnimation::AssertValid() const
{
  CObject::AssertValid();
}

void CEUnitAnimation::Dump(CDumpContext &dc) const
{
  CObject::Dump(dc);
}

#endif

BOOL CEUnitAnimation::Create(DWORD dwColor)
{
  m_dwSpeed = 0;
  m_bLoop = TRUE;
  m_aFrames.RemoveAll();
  m_dwDestColor = dwColor;
  m_nXGraphicalOffset = 0;
  m_nYGraphicalOffset = 0;

  return TRUE;
}

void CEUnitAnimation::Delete()
{
  DeleteAnimation();
}

BOOL CEUnitAnimation::Create(CArchiveFile File, DWORD dwColor)
{
  CEUnitSurface::SColoringParams sColoringParams;
  CString strFileName;
  char *pExt;

  m_nXGraphicalOffset = 0;
  m_nYGraphicalOffset = 0;

  m_dwDestColor = dwColor;
  m_strFileName = File.GetFileName();

  // set default coloring params
  // it means the red color and some tolerances
  sColoringParams.m_dwSourceColor = RGB32(255, 0, 0);
  sColoringParams.m_dbHTolerance = 0.05;
  sColoringParams.m_dbSTolerance = 0.1;
  sColoringParams.m_dbVTolerance = 0.2;
  // use the dest color
  sColoringParams.m_dwDestColor = dwColor;

  strFileName = File.GetFileName();
  pExt = strrchr(strFileName, '.');
  if((pExt != NULL) && (!strcmp(pExt, ".anim"))){
    // ok it's an anim file -> parse it
    CConfigFile AnimFile;

    AnimFile.Create(File);

    Create(&AnimFile, &sColoringParams);

    AnimFile.Delete();
  }
  else{
    CEUnitSurface *pFrame;

    // try to open the file as static image
    try{
      pFrame = new CEUnitSurface();
      if(!pFrame->Create(File, &sColoringParams)){
        delete pFrame;
        return FALSE;
      }
    }
    catch(CException *){
      delete pFrame;
      throw;
    }

    // OK we succeded -> create one frame animation
    m_dwSpeed = 0;
    m_aFrames.SetSize(1);
    m_aFrames[0] = pFrame;

    m_bLoop = FALSE; // static images can't loop
  }

  return TRUE;
}

BOOL CEUnitAnimation::Create(CConfigFile *pConfig, CEUnitSurface::SColoringParams *pColoringParams)
{
  DWORD dwFrame;
  CString strPath, strFrame, strFrameTag;
  CDataArchive Archive;
  CEUnitSurface *pFrame = NULL;

  m_nXGraphicalOffset = 0;
  m_nYGraphicalOffset = 0;

  CConfigFileSection RootSection = pConfig->GetRootSection();

  try{

    // Get number of frames
    DWORD dwFrameNum = RootSection.GetInteger("FrameNum");
    if(dwFrameNum <= 0){
      CString str;
      str.Format("Illegal number of frames specified in '%s'.", pConfig->GetFile().GetFilePath());
      throw new CStringException(str);
    }

    // get the speed
    m_dwSpeed = RootSection.GetInteger("Speed");
    if(m_dwSpeed < 0){
      CString str;
      str.Format("Illegal speed specified in '%s'.", pConfig->GetFile().GetFilePath());
      throw new CStringException(str);
    }

    if(RootSection.IsVariable("Loop")){
      m_bLoop = RootSection.GetBoolean("Loop");
    }
    else{
      m_bLoop = TRUE;
    }

    // Get Graphical offsets
    if(RootSection.IsVariable("XOffset"))
      m_nXGraphicalOffset = RootSection.GetInteger("XOffset");
    if(RootSection.IsVariable("YOffset"))
      m_nYGraphicalOffset = RootSection.GetInteger("YOffset");

    if(RootSection.IsSubsection("Coloring")){
      CConfigFileSection ColoringSection = RootSection.GetSubsection("Coloring");
      // get source color
      if(ColoringSection.IsVariable("SourceColor")){
        pColoringParams->m_dwSourceColor = ColoringSection.GetInteger("SourceColor");
      }

      // get tolerances
      if(ColoringSection.IsVariable("HTolerance"))
      { pColoringParams->m_dbHTolerance = ColoringSection.GetReal("HTolerance"); }
      if(ColoringSection.IsVariable("STolerance"))
      { pColoringParams->m_dbSTolerance = ColoringSection.GetReal("STolerance"); }
      if(ColoringSection.IsVariable("VTolerance"))
      { pColoringParams->m_dbVTolerance = ColoringSection.GetReal("VTolerance"); }
    }

    // create the path
    strPath = pConfig->GetFile().GetFileDir();
    if(!strPath.IsEmpty()) strPath += "\\";

    // get the archive from the anim file
    Archive = pConfig->GetFile().GetDataArchive();

    // create frames
    m_aFrames.SetSize(dwFrameNum);
    for(dwFrame = 0; dwFrame < dwFrameNum; dwFrame++)
      m_aFrames[dwFrame] = NULL;
    
    // read frames
    for(dwFrame = 0; dwFrame < dwFrameNum; dwFrame++){
      
      // get the frame name
      strFrameTag.Format("Frame%d", dwFrame);
      strFrame = RootSection.GetString(strFrameTag);
      strFrame = strPath + strFrame;
      
      // try to open the frame
      pFrame = new CEUnitSurface();
      if(!pFrame->Create(Archive.CreateFile(strFrame), pColoringParams)){
        CString str;
        str.Format("Can't create the frame image '%s'.", strFrame);
        throw new CStringException(str);
      }

      strFrameTag.Format ( "Frame%d_XOffset", dwFrame );
      if ( RootSection.IsVariable ( strFrameTag ) )
        pFrame->SetXOffset ( RootSection.GetInteger ( strFrameTag ) );
      strFrameTag.Format ( "Frame%d_YOffset", dwFrame );
      if ( RootSection.IsVariable ( strFrameTag ) )
        pFrame->SetYOffset ( RootSection.GetInteger ( strFrameTag ) );
      strFrameTag.Format ( "Frame%d_Delay", dwFrame );
      if ( RootSection.IsVariable ( strFrameTag ) )
        pFrame->SetFrameDelay ( RootSection.GetInteger ( strFrameTag ) );

      m_aFrames[dwFrame] = pFrame;
      pFrame = NULL;
    }

  }
  catch(CException *){
    if(pFrame != NULL) delete pFrame;
    Delete();
    throw;
  }
  return TRUE;
}

void CEUnitAnimation::SetGraphics(DWORD dwUnitID, DWORD dwAppearanceID, DWORD dwDirection, CString strGraphicsFile, CDataArchive Archive)
{
  // first remove all previous graphic files
  RemoveGraphics(Archive);

  CString strExt;

  // get the extension
  {
    int nPos;

    strExt.Empty();
    nPos = strGraphicsFile.ReverseFind('.');
    if(nPos != -1){
      strExt = strGraphicsFile.Mid(nPos + 1);
    }
  }

  // create the filename for the archive
  CString strInnerFileName;
  strInnerFileName.Format("Unit%010dApp%010dDir%01dGraphics.%s", dwUnitID, dwAppearanceID, dwDirection, strExt);

  m_strFileName = strInnerFileName;

  // get temp path
  CString strTempPath;
  strTempPath = CDataArchive::GetTempArchive()->GetArchivePath();

  // if the extension is not anim -> all is done
  if(strExt.CompareNoCase("anim") != 0){
    // copy the graphics file itself
    CString strFile = strTempPath + '\\' + strInnerFileName;
    if(!CopyFile(strGraphicsFile, strFile, FALSE)){
      CString str;
      str.Format("Can't copy the file '%s'.", strGraphicsFile);
      throw new CStringException(str);
    }
    // move it to the archive
    try{ Archive.RemoveFile(strInnerFileName); } catch(CException *e){ e->Delete(); }
    Archive.AppendFile(strFile, strInnerFileName, appendCompressed);

    CArchiveFile file;
    file = Archive.CreateFile(strInnerFileName);
    DeleteAnimation();
    Create(file, m_dwDestColor);

    return;
  }

  // Create new animation file
  CArchiveFile NewAnimFile;
  CString strNewAnimFileName = strTempPath + '\\' + strInnerFileName;
  NewAnimFile = CDataArchive::GetRootArchive()->CreateFile(strNewAnimFileName, CArchiveFile::modeWrite | CArchiveFile::modeCreate | CArchiveFile::modeUncompressed);
  CString strHelp;

  // the graphics is animation -> read all frames
  // if the file is animation -> read it and delete all frames
  CConfigFile AnimFile;

  AnimFile.Create(CDataArchive::GetRootArchive()->CreateFile(strGraphicsFile));

  CConfigFileSection RootSection = AnimFile.GetRootSection();

  // create the path
  CString strPath;
  strPath = AnimFile.GetFile().GetFileDir();
  if(!strPath.IsEmpty()) strPath += "\\";

  CString strFrameTag, strFrame;
  CString strInnerFrame, strTempFile, strFrameExt;

  DWORD dwFrameNum = RootSection.GetInteger("FrameNum");
  // write it to the new anim file
  strHelp.Format("FrameNum = %d;\r\n", dwFrameNum);
  NewAnimFile.Write((const char *)strHelp, strHelp.GetLength());
  // write the speed
  DWORD dwSpeed = RootSection.GetInteger("Speed");
  strHelp.Format("Speed = %d;\r\n", dwSpeed);
  NewAnimFile.Write((const char *)strHelp, strHelp.GetLength());

  // write the looping (only if included in original file)
  {
    if(RootSection.IsVariable("Loop")){
      BOOL bLoop;
      bLoop = RootSection.GetBoolean("Loop");
      if(bLoop)
        strHelp = "Loop = True;\r\n";
      else
        strHelp = "Loop = False;\r\n";
      NewAnimFile.Write((const char *)strHelp, strHelp.GetLength());
    }
  }

  // write graphicsl offsets
  if(RootSection.IsVariable("XOffset")){
    int nXOff = RootSection.GetInteger("XOffset");
    strHelp.Format("XOffset = %d;\r\n", nXOff);
    NewAnimFile.Write((const char *)strHelp, strHelp.GetLength());
  }
  if(RootSection.IsVariable("YOffset")){
    int nYOff = RootSection.GetInteger("YOffset");
    strHelp.Format("YOffset = %d;\r\n", nYOff);
    NewAnimFile.Write((const char *)strHelp, strHelp.GetLength());
  }

  // go through all frames
  CStringArray aFrames;
  DWORD dwFrame;
  DWORD dwFrameDelay;
  int nXOff, nYOff;
  for(dwFrame = 0; dwFrame < dwFrameNum; dwFrame++){
    // get the frame name
    strFrameTag.Format("Frame%d", dwFrame);
    strFrame = RootSection.GetString(strFrameTag);

    // get the extension
    {
      int nPos;

      strFrameExt.Empty();
      nPos = strFrame.ReverseFind('.');
      if(nPos != -1){
        strFrameExt = strFrame.Mid(nPos + 1);
      }
    }

    // test if there is the same framed copyied
    {
      DWORD i;
      for(i = 0; i < (DWORD)aFrames.GetSize(); i++){
        if(aFrames[i].CompareNoCase(strFrame) == 0){
          // yes there is allready some
          // -> so just add the new name to the anim file
          // no copying
      
          // make new file name
          strInnerFrame.Format("Unit%010dApp%010dDir%01dFrame%03d.%s", dwUnitID, dwAppearanceID, dwDirection, i, strFrameExt);
          goto FrameDone;
        }
      }
    }

    strFrame = strPath + strFrame;

    // make new file name
    strInnerFrame.Format("Unit%010dApp%010dDir%01dFrame%03d.%s", dwUnitID, dwAppearanceID, dwDirection, dwFrame, strFrameExt);

    strTempFile = strTempPath + '\\' + strInnerFrame;
    // copy the frame
    if(!CopyFile(strFrame, strTempFile, FALSE)){
      CString str;
      str.Format("Can't copy the file '%s'.", strFrame);
      throw new CStringException(str);
    }

    // append to the archive
    try{ Archive.RemoveFile(strInnerFrame); } catch(CException *e){ e->Delete(); }
    Archive.AppendFile(strTempFile, strInnerFrame, appendCompressed);

FrameDone:
    // write the frame to the anim file
    strHelp.Format("Frame%d = \"%s\";\r\n", dwFrame, strInnerFrame);
    NewAnimFile.Write((const char *)strHelp, strHelp.GetLength());

    strFrameTag.Format ( "Frame%d_XOffset", dwFrame );
    if ( RootSection.IsVariable ( strFrameTag ) )
    {
      nXOff = RootSection.GetInteger ( strFrameTag );
      strHelp.Format ( "Frame%d_XOffset = %d;\r\n", dwFrame, nXOff );
      NewAnimFile.Write((const char *)strHelp, strHelp.GetLength());
    }

    strFrameTag.Format ( "Frame%d_YOffset", dwFrame );
    if ( RootSection.IsVariable ( strFrameTag ) )
    {
      nYOff = RootSection.GetInteger ( strFrameTag );
      strHelp.Format ( "Frame%d_YOffset = %d;\r\n", dwFrame, nYOff );
      NewAnimFile.Write((const char *)strHelp, strHelp.GetLength());
    }

    strFrameTag.Format ( "Frame%d_Delay", dwFrame );
    if ( RootSection.IsVariable ( strFrameTag ) )
    {
      dwFrameDelay = RootSection.GetInteger ( strFrameTag );
      strHelp.Format  ( "Frame%d_Delay = %d;\r\n", dwFrame, dwFrameDelay );
      NewAnimFile.Write((const char *)strHelp, strHelp.GetLength());
    }

    // add it to done frames
    aFrames.Add(strInnerFrame);
  }

  if(RootSection.IsSubsection("Coloring"))
  {
    // start the coloring section
    strHelp = "[Coloring]\r\n";
    NewAnimFile.Write((const char *)strHelp, strHelp.GetLength());

    CConfigFileSection ColoringSection = RootSection.GetSubsection("Coloring");
    // write the source color
    {
      DWORD dwSourceColor;
      // get source color
      if(ColoringSection.IsVariable("SourceColor")){
        dwSourceColor = ColoringSection.GetInteger("SourceColor");
        strHelp.Format("SourceColor = 0x0%06X;\r\n", dwSourceColor);
        NewAnimFile.Write((const char *)strHelp, strHelp.GetLength());
      }
    }
    // write tolerances
    {
      double dbValue;
      if(ColoringSection.IsVariable("HTolerance")){
        dbValue = ColoringSection.GetReal("HTolerance");
        strHelp.Format("HTolerance = %0.1f;\r\n", dbValue);
        NewAnimFile.Write((const char *)strHelp, strHelp.GetLength());
      }
      if(ColoringSection.IsVariable("STolerance")){
        dbValue = ColoringSection.GetReal("STolerance");
        strHelp.Format("STolerance = %0.1f;\r\n", dbValue);
        NewAnimFile.Write((const char *)strHelp, strHelp.GetLength());
      }
      if(ColoringSection.IsVariable("VTolerance")){
        dbValue = ColoringSection.GetReal("VTolerance");
        strHelp.Format("VTolerance = %0.1f;\r\n", dbValue);
        NewAnimFile.Write((const char *)strHelp, strHelp.GetLength());
      }
    }
  }

  NewAnimFile.Close();

  // the new anim file is done
  try{ Archive.RemoveFile(strInnerFileName); } catch(CException *e){ e->Delete(); }
  Archive.AppendFile(strNewAnimFileName, strInnerFileName, appendCompressed);

  CArchiveFile file;
  file = Archive.CreateFile(strInnerFileName);
  DeleteAnimation();
  Create(file, m_dwDestColor);
}

void CEUnitAnimation::RemoveGraphics(CDataArchive Archive)
{
  CString strExt;
  
  if(m_strFileName.IsEmpty()) return;

  {
    int nPos;

    strExt.Empty();
    nPos = m_strFileName.ReverseFind('.');
    if(nPos != -1){
      strExt = m_strFileName.Mid(nPos + 1);
    }
  }

  if(strExt.CompareNoCase("anim") != 0){ // if not anim

    // just delete the file
    try{
      Archive.RemoveFile(m_strFileName);
    }
    catch(CException *e){
      e->Delete();
    }
    return;
  }

  // if the file is animation -> read it and delete all frames
  CConfigFile AnimFile;

  AnimFile.Create(Archive.CreateFile(m_strFileName));

  CConfigFileSection RootSection = AnimFile.GetRootSection();

  // create the path
  CString strPath;
  strPath = AnimFile.GetFile().GetFileDir();
  if(!strPath.IsEmpty()) strPath += "\\";

  CString strFrameTag, strFrame;

  DWORD dwFrameNum = RootSection.GetInteger("FrameNum");
  DWORD dwFrame;
  for(dwFrame = 0; dwFrame < dwFrameNum; dwFrame++){
    // get the frame name
    strFrameTag.Format("Frame%d", dwFrame);
    strFrame = RootSection.GetString(strFrameTag);
    strFrame = strPath + strFrame;

    // remove the frame file
    try{
      Archive.RemoveFile(strFrame);
    }
    catch(CException *e){
      e->Delete();
    }
  }

  // remove anim file
  try{
    Archive.RemoveFile(m_strFileName);
  }
  catch(CException *e){
    e->Delete();
  }

  // delete the animation
  DeleteAnimation();
  m_strFileName.Empty();
}

void CEUnitAnimation::DeleteAnimation()
{
  int i;
  CEUnitSurface *pSurface;
  for(i = 0; i < m_aFrames.GetSize(); i++){
    pSurface = m_aFrames[i];
    pSurface->Delete();
    delete pSurface;
  }
  m_aFrames.RemoveAll();
  m_dwSpeed = 0;
}
