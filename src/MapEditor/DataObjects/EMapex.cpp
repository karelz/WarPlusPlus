// EMapex.cpp: implementation of the CEMapex class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "EMapex.h"

#include "..\MapFormats.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CEMapex::CEMapex()
{
  m_sizeMapex.cx = 0; m_sizeMapex.cy = 0;
  m_dwID = 0;
  m_pLandType = NULL;
  m_bModified = FALSE;
}

CEMapex::~CEMapex()
{
  Delete();
}

BOOL CEMapex::Create(DWORD dwID, CSize sizeMapex)
{
  ASSERT(m_pLandType == NULL);
  m_dwID = dwID;
  m_sizeMapex = sizeMapex;
  m_pLandType = new BYTE[m_sizeMapex.cx * m_sizeMapex.cy];
  memset(m_pLandType, 0, m_sizeMapex.cx * m_sizeMapex.cy);

  m_bModified = TRUE;

  return TRUE;
}

void CEMapex::Delete()
{
  if(m_pLandType != NULL){
    delete m_pLandType;
    m_pLandType = NULL;
  }
  m_Graphics.Delete();
}

void CEMapex::SetSize(CSize sizeMapex)
{
  ASSERT(m_pLandType != NULL);
  BYTE *pOldLandType;
  DWORD dwX, dwY;

  pOldLandType = m_pLandType;

  // allocate new landtype array
  m_pLandType = new BYTE[sizeMapex.cx * sizeMapex.cy];
  memset(m_pLandType, 0, sizeMapex.cx * sizeMapex.cy);

  // copy the land type to the new location
  // (copy only what can be copied)
  for(dwY = 0; dwY < (DWORD)sizeMapex.cy; dwY++){
    if(dwY >= (DWORD)m_sizeMapex.cy) break;
    for(dwX = 0; dwX < (DWORD)sizeMapex.cx; dwX++){
      if(dwX >= (DWORD)m_sizeMapex.cx) break;
      m_pLandType[dwX + dwY * sizeMapex.cx] =
        pOldLandType[dwX + dwY * m_sizeMapex.cx];
    }
  }

  // free old landtype array
  delete pOldLandType;

  m_sizeMapex = sizeMapex;

  m_bModified = TRUE;
}

BOOL CEMapex::Create(CArchiveFile MapexFile)
{
  BOOL bResult = TRUE;
  if(g_dwMapexFileVersion < MAP_FILE_VERSION(1, 0)){
    SMapexHeader_Old h;

    MapexFile.Open();
    MapexFile.Read(&h, sizeof(h));

    m_dwID = h.m_dwID;
    m_sizeMapex.cx = h.m_dwXSize;
    m_sizeMapex.cy = h.m_dwYSize;
    m_strGraphicsFile = h.m_pGraphicsFileName;
    m_strMapexFile = MapexFile.GetFilePath();
    m_strName = h.m_pName;

    m_pLandType = new BYTE[m_sizeMapex.cx * m_sizeMapex.cy];
    MapexFile.Read(m_pLandType, m_sizeMapex.cx * m_sizeMapex.cy);

    if(!m_strGraphicsFile.IsEmpty()){
      CArchiveFile file = MapexFile.GetDataArchive().CreateFile(m_strGraphicsFile);
      if(!m_Graphics.Create(file)) bResult = FALSE;
    }

    MapexFile.Close();
  }
  else{
    SMapexHeader h;

    MapexFile.Open();
    MapexFile.Read(&h, sizeof(h));

    m_dwID = h.m_dwID;
    m_sizeMapex.cx = h.m_dwXSize;
    m_sizeMapex.cy = h.m_dwYSize;
    m_strGraphicsFile = h.m_pGraphicsFileName;
    m_strMapexFile = MapexFile.GetFilePath();
    m_strName = h.m_pName;

    m_pLandType = new BYTE[m_sizeMapex.cx * m_sizeMapex.cy];
    MapexFile.Read(m_pLandType, m_sizeMapex.cx * m_sizeMapex.cy);

    if(!m_strGraphicsFile.IsEmpty()){
      CArchiveFile file = MapexFile.GetDataArchive().CreateFile(m_strGraphicsFile);
      if(!m_Graphics.Create(file)) bResult = FALSE;
    }

    MapexFile.Close();
  }

  return bResult;
}

void CEMapex::SaveMapex(CDataArchive Archive)
{
  // create the file name
  m_strMapexFile.Format("Mapex%010d.Header", m_dwID);

  CArchiveFile MapexFile;
  // first try to open it
  try{
    MapexFile = Archive.CreateFile(m_strMapexFile, CArchiveFile::modeWrite);
  }
  catch(CException *e){
    e->Delete();
    // it doesn't exists -> create it
    MapexFile = Archive.CreateFile(m_strMapexFile, CArchiveFile::modeWrite | CArchiveFile::modeCreate);
  }

  SMapexHeader h;

  memset(&h, 0, sizeof(h));
  h.m_dwID = m_dwID;
  h.m_dwXSize = m_sizeMapex.cx;
  h.m_dwYSize = m_sizeMapex.cy;
  strncpy(h.m_pGraphicsFileName, m_strGraphicsFile, 29);
  strncpy(h.m_pName, m_strName, 50);

  MapexFile.Open();
  MapexFile.Write(&h, sizeof(h));

  MapexFile.Write(m_pLandType, m_sizeMapex.cx * m_sizeMapex.cy);

  MapexFile.Close();

  m_bModified = FALSE;
}

void CEMapex::RemoveGraphics(CDataArchive Archive)
{
  CString strExt;
  
  {
    int nPos;

    strExt.Empty();
    nPos = m_strGraphicsFile.ReverseFind('.');
    if(nPos != -1){
      strExt = m_strGraphicsFile.Mid(nPos + 1);
    }
  }

  if(strExt.CompareNoCase("anim") != 0){ // if not anim

    // just delete the file
    try{
      Archive.RemoveFile(m_strGraphicsFile);
    }
    catch(CException *e){
      e->Delete();
    }
    return;
  }

  // if the file is animation -> read it and delete all frames
  CConfigFile AnimFile;

  AnimFile.Create(Archive.CreateFile(m_strGraphicsFile));
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

  AnimFile.Delete();

  // remove anim file
  try{
    Archive.RemoveFile(m_strGraphicsFile);
  }
  catch(CException *e){
    e->Delete();
  }

  // delete the animation
  m_Graphics.Delete();
  m_strGraphicsFile.Empty();

  m_bModified = TRUE;
}

void CEMapex::SetGraphics(CString strGraphicsFile, CDataArchive Archive)
{
  // first remove all previous graphic files
  RemoveGraphics(Archive);

  m_bModified = TRUE;

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
  strInnerFileName.Format("Mapex%010dGraphics.%s", m_dwID, strExt);

  m_strGraphicsFile = strInnerFileName;

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
    m_Graphics.Create(file);

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

  // go through all frames
  CStringArray aFrames;
  DWORD dwFrame;
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
          strInnerFrame.Format("Mapex%010dFrame%03d.%s", m_dwID, i, strFrameExt);
          goto FrameDone;
        }
      }
    }

    strFrame = strPath + strFrame;

    // make new file name
    strInnerFrame.Format("Mapex%010dFrame%03d.%s", m_dwID, dwFrame, strFrameExt);

    strTempFile = strTempPath + '\\' + strInnerFrame;
    // copy the frame
    if(!CopyFile(strFrame, strTempFile, FALSE)){
      CString str;
      str.Format("Can't copy the file '%s'.", strFrame);
      throw CStringException(str);
    }

    // append to the archive
    try{ Archive.RemoveFile(strInnerFrame); } catch(CException *e){ e->Delete(); }
    Archive.AppendFile(strTempFile, strInnerFrame, appendCompressed);

FrameDone:
    // write the frame to the anim file
    strHelp.Format("Frame%d = \"%s\";\r\n", dwFrame, strInnerFrame);
    NewAnimFile.Write((const char *)strHelp, strHelp.GetLength());
    // add it to done frames
    aFrames.Add(strInnerFrame);
  }

  AnimFile.Delete();

  // the new anim file is done
  NewAnimFile.Close();
  try{ Archive.RemoveFile(strInnerFileName); } catch(CException *e){ e->Delete(); }
  Archive.AppendFile(strNewAnimFileName, strInnerFileName, appendCompressed);

  CArchiveFile file;
  file = Archive.CreateFile(strInnerFileName);
  m_Graphics.Create(file);
}

CString CEMapex::GetMapexFileName()
{
  return m_strMapexFile;
}

CString CEMapex::GetName()
{
  return m_strName;
}

void CEMapex::SetName(CString strName)
{
  m_strName = strName;
  m_bModified = TRUE;
}

CAnimation * CEMapex::GetGraphics()
{
  return &m_Graphics;
}

BYTE CEMapex::GetLandType(DWORD dwX, DWORD dwY)
{
  ASSERT(dwX < (DWORD)m_sizeMapex.cx);
  ASSERT(dwY < (DWORD)m_sizeMapex.cy);

  return m_pLandType[dwY * m_sizeMapex.cx + dwX];
}

void CEMapex::SetLandType(DWORD dwX, DWORD dwY, BYTE nLandType)
{
  ASSERT(dwX < (DWORD)m_sizeMapex.cx);
  ASSERT(dwY < (DWORD)m_sizeMapex.cy);

  m_pLandType[dwY * m_sizeMapex.cx + dwX] = nLandType;

  m_bModified = TRUE;
}

BOOL CEMapex::CheckValid()
{
  return TRUE;
}