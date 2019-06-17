// EMapexLibrary.cpp: implementation of the CEMapexLibrary class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "EMapexLibrary.h"

#include "EMap.h"
#include "..\MapFormats.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CEMapexLibrary, CObject);

CEMapexLibrary::CEMapexLibrary()
{
  m_pMap = NULL;
}

CEMapexLibrary::~CEMapexLibrary()
{
//  Delete();
  ASSERT(m_pMap == NULL);
}

#ifdef _DEBUG

void CEMapexLibrary::AssertValid() const
{
  CObject::AssertValid();

  ASSERT(m_pMap != NULL);
}

void CEMapexLibrary::Dump(CDumpContext &dc) const
{
  CObject::Dump(dc);
}

#endif

BOOL CEMapexLibrary::Create(CDataArchive Archive, BOOL bCreateNew, CEMap *pMap)
{
  ASSERT(pMap != NULL);

  m_pMap = pMap;

  if(!bCreateNew){
    LoadLibrary(Archive);
  }
  else{
    m_Archive = Archive;
  }
  return TRUE;
}

void CEMapexLibrary::Delete()
{
//  m_Archive.Delete();

  DeleteAll();
  m_pMap = NULL;
}

void CEMapexLibrary::DeleteAll()
{
  POSITION pos;
  CEMapex *pMapex;
  pos = m_listMapexes.GetHeadPosition();
  while(pos != NULL){
    pMapex = m_listMapexes.GetNext(pos);
    if(m_pMap != NULL)
      m_pMap->DeleteMapex(pMapex);
    pMapex->Delete();
    delete pMapex;
  }
  m_listMapexes.RemoveAll();
}

void CEMapexLibrary::LoadLibrary(CDataArchive Archive)
{
  m_Archive = Archive;

  CArchiveFile HeaderFile;
  HeaderFile = Archive.CreateFile("Library.Header");

  // First load the version header
  {
    SMapexFileVersionHeader sFileVersionHeader;

    if(HeaderFile.Read(&sFileVersionHeader, sizeof(sFileVersionHeader)) < sizeof(sFileVersionHeader)){
      // File is too small -> old version
      g_dwMapexFileVersion = MAP_FILE_VERSION(0, 9);
      goto FormatDone;
    }

    // Compare ID
    {
      BYTE aFileID[16] = MAPEX_FILE_IDENTIFIER;
      if(memcmp(aFileID, sFileVersionHeader.m_aFileID, 16) != 0){
        // Not our ID -> old format
        g_dwMapexFileVersion = MAP_FILE_VERSION(0, 9);
        goto FormatDone;
      }
    }

    // Compare version numbers
    if(sFileVersionHeader.m_dwFormatVersion > CURRENT_MAP_FILE_VERSION){
      // It's newer - look at compatability
      if(sFileVersionHeader.m_dwCompatibleFormatVersion > CURRENT_MAP_FILE_VERSION){
        // Ooops too new
        throw new CStringException("Neznámá verze souboru knihovny mapexù");
      }
      // Compatible - load it as our version
      g_dwMapexFileVersion = CURRENT_MAP_FILE_VERSION;
    }
    else{
      // Older version
      g_dwMapexFileVersion = sFileVersionHeader.m_dwFormatVersion;
    }

FormatDone:
    ;
  }

  if(g_dwMapexFileVersion < MAP_FILE_VERSION(1, 0)){
    HeaderFile.SeekToBegin();

    SMapexLibraryHeader_Old h;
    HeaderFile.Read(&h, sizeof(h));

    m_strName = h.m_pName;

    DWORD dwMapex;
    CEMapex *pMapex;
    char pMapexName[30];
    CString strMapexFile;
    for(dwMapex = 0; dwMapex < h.m_dwMapexCount; dwMapex++){
      HeaderFile.Read(pMapexName, 30);
      strMapexFile = pMapexName;

      pMapex = new CEMapex();
      if(!pMapex->Create(Archive.CreateFile(strMapexFile))){
        CString str;
        str.Format("Error loading mapex '%s'.", strMapexFile);
        throw new CStringException(str);
      }
      m_listMapexes.AddHead(pMapex);

      m_pMap->AddMapex(pMapex);
    }
  }
  else{
    SMapexLibraryHeader h;
    HeaderFile.Read(&h, sizeof(h));

    m_strName = h.m_pName;

    DWORD dwMapex;
    CEMapex *pMapex;
    char pMapexName[30];
    CString strMapexFile;
    for(dwMapex = 0; dwMapex < h.m_dwMapexCount; dwMapex++){
      HeaderFile.Read(pMapexName, 30);
      strMapexFile = pMapexName;

      pMapex = new CEMapex();
      if(!pMapex->Create(Archive.CreateFile(strMapexFile))){
        CString str;
        str.Format("Error loading mapex '%s'.", strMapexFile);
        throw new CStringException(str);
      }
      m_listMapexes.AddHead(pMapex);

      m_pMap->AddMapex(pMapex);
    }
  }
}

void CEMapexLibrary::SaveLibrary()
{
  SMapexLibraryHeader h;
  CString strTempPath, strTempFile;

  strTempPath = CDataArchive::GetTempArchive()->GetArchivePath();

  CArchiveFile HeaderFile;

  strTempFile = strTempPath + '\\' + "Library.Header";
  HeaderFile = CDataArchive::GetRootArchive()->CreateFile(strTempFile, CArchiveFile::modeReadWrite | CArchiveFile::modeCreate | CArchiveFile::modeUncompressed);
  HeaderFile.Open();

  // First save the version header
  {
    SMapexFileVersionHeader sFileVersionHeader;
    BYTE aFileID[16] = MAPEX_FILE_IDENTIFIER;

    memset(&sFileVersionHeader, 0, sizeof(sFileVersionHeader));
    memcpy(sFileVersionHeader.m_aFileID, aFileID, 16);
    sFileVersionHeader.m_dwFormatVersion = CURRENT_MAP_FILE_VERSION;
    sFileVersionHeader.m_dwCompatibleFormatVersion = COMPATIBLE_MAPEX_FILE_VERSION;

    HeaderFile.Write(&sFileVersionHeader, sizeof(sFileVersionHeader));
  }

  memset(&h, 0, sizeof(h));
  h.m_dwMapexCount = (DWORD)m_listMapexes.GetCount();
  strncpy(h.m_pName, m_strName, 50);
  HeaderFile.Write(&h, sizeof(h));

  CEMapex *pMapex;
  CString strMapex;
  char pMapexName[30];
  POSITION pos;
  pos = m_listMapexes.GetHeadPosition();
  while(pos != NULL){
    pMapex = m_listMapexes.GetNext(pos);

    if(pMapex->IsModified())
      pMapex->SaveMapex(m_Archive);
    strMapex = pMapex->GetMapexFileName();
    strncpy(pMapexName, strMapex, 29);
    pMapexName[29] = 0;
    HeaderFile.Write(pMapexName, 30);
  }

  HeaderFile.Close();

  // add it to archive
  try{ m_Archive.RemoveFile("Library.Header"); } catch(CException *e){ e->Delete(); }
  m_Archive.AppendFile(strTempFile, "Library.Header", appendCompressed);
}

CString CEMapexLibrary::GetFullPath()
{
  return m_Archive.GetArchivePath();
}

CString CEMapexLibrary::GetFileName()
{
  CString strFull = m_Archive.GetArchivePath();
  int nPos;
  nPos = strFull.ReverseFind('\\');
  if(nPos > -1){
    return strFull.Mid(nPos + 1);
  }
  return strFull;
}

CString CEMapexLibrary::GetName()
{
  return m_strName;
}

void CEMapexLibrary::SetName(CString strName)
{
  m_strName = strName;
}

void CEMapexLibrary::FillMapexesListCtrl(CListCtrl *pListCtrl)
{
  int i, nItem;
  CEMapex *pMapex;

  pListCtrl->DeleteAllItems();
  POSITION pos;
  i = 0;
  pos = m_listMapexes.GetHeadPosition();
  while(pos != NULL){
    pMapex = m_listMapexes.GetNext(pos);
    nItem = pListCtrl->InsertItem(i, pMapex->GetName());
    pListCtrl->SetItemData(nItem, (DWORD)pMapex);
    i++;
  }
}

CEMapex * CEMapexLibrary::NewMapex(CSize sizeMapex)
{
  CEMapex *pMapex;
  pMapex = new CEMapex();

  pMapex->Create(m_pMap->GetNewMapexID(), sizeMapex);
  m_listMapexes.AddHead(pMapex);
  m_pMap->AddMapex(pMapex);

  return pMapex;
}

CDataArchive CEMapexLibrary::GetArchive()
{
  return m_Archive;
}

void CEMapexLibrary::DeleteMapex(CEMapex *pMapex)
{
  POSITION pos;
  // remove it from our list
  pos = m_listMapexes.GetHeadPosition();
  while(pos != NULL){
    if(m_listMapexes.GetAt(pos) == pMapex){
      m_listMapexes.RemoveAt(pos);
      break;
    }
    m_listMapexes.GetNext(pos);
  }

  // remove it from the map list
  if(m_pMap != NULL){
    m_pMap->DeleteMapex(pMapex);
  }
  // remove the mapex graphics from the archive
  pMapex->RemoveGraphics(m_Archive);

  // remove the mapex file from the archive
  // ignore all exceptions (if doesn't exists well, that's what we want)
  try{
    m_Archive.RemoveFile(pMapex->GetMapexFileName());
  }
  catch(CException *e){
    e->Delete();
  }

  // delete the object
  pMapex->Delete();
  delete pMapex;
}

void CEMapexLibrary::Rebuild()
{
  m_Archive.Rebuild();
}
