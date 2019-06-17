// EUnitLibrary.cpp: implementation of the CEUnitLibrary class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "EUnitLibrary.h"

#include "..\MapFormats.h"

#include "EMap.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CEUnitLibrary, CObject)

CEUnitLibrary::CEUnitLibrary()
{
  m_pMap = NULL;
}

CEUnitLibrary::~CEUnitLibrary()
{
  ASSERT(m_pMap == NULL);
}

#ifdef _DEBUG

void CEUnitLibrary::AssertValid() const
{
  CObject::AssertValid();
  ASSERT(m_pMap != NULL);
}

void CEUnitLibrary::Dump(CDumpContext &dc) const
{
  CObject::Dump(dc);
  dc << "Unit type count : " << m_listUnits.GetCount() << "\r\n";
}

#endif

BOOL CEUnitLibrary::Create(CDataArchive Archive, BOOL bNew, CEMap *pMap)
{
  ASSERT(pMap != NULL);

  m_pMap = pMap;

  if(bNew){
    m_Archive = Archive;
  }
  else{
    LoadLibrary(Archive);
  }
  return TRUE;
}

void CEUnitLibrary::Delete()
{
  DeleteAll();
  m_pMap = NULL;
}

void CEUnitLibrary::LoadLibrary(CDataArchive Archive)
{
  m_Archive = Archive;

  CArchiveFile HeaderFile;
  HeaderFile = Archive.CreateFile("Library.Header");

  // First read version header
  {
    SUnitTypeFileVersionHeader sFileVersionHeader;

    if(HeaderFile.Read(&sFileVersionHeader, sizeof(sFileVersionHeader)) < sizeof(sFileVersionHeader)){
      // File is too small -> old version
      g_dwUnitTypeFileVersion = MAP_FILE_VERSION(0, 9);
      goto FormatDone;
    }

    // Compare ID
    {
      BYTE aFileID[16] = UNITTYPE_FILE_IDENTIFIER;
      if(memcmp(aFileID, sFileVersionHeader.m_aFileID, 16) != 0){
        // Not our ID -> old format
        g_dwUnitTypeFileVersion = MAP_FILE_VERSION(0, 9);
        goto FormatDone;
      }
    }

    // Compare version numbers
    if(sFileVersionHeader.m_dwFormatVersion > CURRENT_MAP_FILE_VERSION){
      // It's newer - look at compatability
      if(sFileVersionHeader.m_dwCompatibleFormatVersion > CURRENT_MAP_FILE_VERSION){
        // Ooops too new
        throw new CStringException("Neznámá verze souboru knihovny jednotek");
      }
      // Compatible - load it as our version
      g_dwUnitTypeFileVersion = CURRENT_MAP_FILE_VERSION;
    }
    else{
      // Older version
      g_dwUnitTypeFileVersion = sFileVersionHeader.m_dwFormatVersion;
    }

FormatDone:
    ;
  }

  if(g_dwUnitTypeFileVersion < MAP_FILE_VERSION(1, 0)){
    HeaderFile.SeekToBegin();

    SUnitTypeLibraryHeader_Old h;
    HeaderFile.Read(&h, sizeof(h));

    m_strName = h.m_pName;

    DWORD dwUnit;
    CEUnitType *pUnit;
    char pUnitName[30];
    CString strUnitName;
    for(dwUnit = 0; dwUnit < h.m_dwUnitTypeCount; dwUnit++){
      HeaderFile.Read(pUnitName, 30);
      strUnitName = pUnitName;

      pUnit = new CEUnitType();
      pUnit->SetArchive(m_Archive);
      if(!pUnit->Create(Archive.CreateFile(strUnitName))){
        CString str;
        str.Format("Error loading unit '%s'.", strUnitName);
        throw new CStringException(str);
      }
      m_listUnits.AddHead(pUnit);

      m_pMap->AddUnitType(pUnit);
    }
  }
  else{
    SUnitTypeLibraryHeader h;
    HeaderFile.Read(&h, sizeof(h));

    m_strName = h.m_pName;

    DWORD dwUnit;
    CEUnitType *pUnit;
    char pUnitName[30];
    CString strUnitName;
    for(dwUnit = 0; dwUnit < h.m_dwUnitTypeCount; dwUnit++){
      HeaderFile.Read(pUnitName, 30);
      strUnitName = pUnitName;

      pUnit = new CEUnitType();
      pUnit->SetArchive(m_Archive);
      if(!pUnit->Create(Archive.CreateFile(strUnitName))){
        CString str;
        str.Format("Error loading unit '%s'.", strUnitName);
        throw new CStringException(str);
      }
      m_listUnits.AddHead(pUnit);

      m_pMap->AddUnitType(pUnit);
    }
  }
}

void CEUnitLibrary::SaveLibrary()
{
  SUnitTypeLibraryHeader h;
  CString strTempPath, strTempFile;

  strTempPath = CDataArchive::GetTempArchive()->GetArchivePath();

  CArchiveFile HeaderFile;

  strTempFile = strTempPath + '\\' + "Library.Header";
  HeaderFile = CDataArchive::GetRootArchive()->CreateFile(strTempFile, CArchiveFile::modeWrite | CArchiveFile::modeCreate | CArchiveFile::modeUncompressed);
  HeaderFile.Open();

  // Save the version header
  {
    SUnitTypeFileVersionHeader sFileVersionHeader;
    BYTE aFileID[16] = UNITTYPE_FILE_IDENTIFIER;

    memset(&sFileVersionHeader, 0, sizeof(sFileVersionHeader));
    memcpy(sFileVersionHeader.m_aFileID, aFileID, 16);
    sFileVersionHeader.m_dwFormatVersion = CURRENT_MAP_FILE_VERSION;
    sFileVersionHeader.m_dwCompatibleFormatVersion = COMPATIBLE_UNITTYPE_FILE_VERSION;

    HeaderFile.Write(&sFileVersionHeader, sizeof(sFileVersionHeader));
  }

  memset(&h, 0, sizeof(h));
  h.m_dwUnitTypeCount = m_listUnits.GetCount();
  strncpy(h.m_pName, m_strName, 50);
  HeaderFile.Write(&h, sizeof(h));

  CEUnitType *pUnit;
  CString strUnit;
  char pUnitName[30];
  POSITION pos;
  pos = m_listUnits.GetHeadPosition();
  while(pos != NULL){
    pUnit = m_listUnits.GetNext(pos);

    pUnit->SaveUnit(m_Archive);
    strUnit = pUnit->GetUnitFileName();
    strncpy(pUnitName, strUnit, 29);
    pUnitName[29] = 0;
    HeaderFile.Write(pUnitName, 30);
  }

  HeaderFile.Close();

  // add it to archive
  try{ m_Archive.RemoveFile("Library.Header"); } catch(CException *e){ e->Delete(); }
  m_Archive.AppendFile(strTempFile, "Library.Header", appendCompressed);
}

void CEUnitLibrary::DeleteAll()
{
  POSITION pos;
  CEUnitType *pUnit;

  pos = m_listUnits.GetHeadPosition();
  while(pos != NULL){
    pUnit = m_listUnits.GetNext(pos);

    pUnit->Delete();
    delete pUnit;
  }
  m_listUnits.RemoveAll();
}

CString CEUnitLibrary::GetFileName()
{
  CString strFull = m_Archive.GetArchivePath();
  int nPos;
  nPos = strFull.ReverseFind('\\');
  if(nPos > -1){
    return strFull.Mid(nPos + 1);
  }
  return strFull;
}

CString CEUnitLibrary::GetFullPath()
{
  return m_Archive.GetArchivePath();
}

void CEUnitLibrary::FillUnitTypesListCtrl(CListCtrl *pListCtrl)
{
  int i, nItem;
  CEUnitType *pUnitType;

  pListCtrl->DeleteAllItems();
  POSITION pos;
  i = 0;
  pos = m_listUnits.GetHeadPosition();
  while(pos != NULL){
    pUnitType = m_listUnits.GetNext(pos);
    nItem = pListCtrl->InsertItem(i, pUnitType->GetName());
    pListCtrl->SetItemData(nItem, (DWORD)pUnitType);
    i++;
  }
}

CEUnitType * CEUnitLibrary::NewUnitType()
{
  CEUnitType *pUnitType;
  pUnitType = new CEUnitType();

  pUnitType->SetArchive(m_Archive);
  pUnitType->Create(m_pMap->GetNewUnitTypeID());
  m_listUnits.AddHead(pUnitType);
  m_pMap->AddUnitType(pUnitType);

  return pUnitType;
}

void CEUnitLibrary::DeleteUnitType(CEUnitType *pUnitType)
{
  POSITION pos;
  // remove it from our list
  pos = m_listUnits.GetHeadPosition();
  while(pos != NULL){
    if(m_listUnits.GetAt(pos) == pUnitType){
      m_listUnits.RemoveAt(pos);
      break;
    }
    m_listUnits.GetNext(pos);
  }

  // remove it from the map list
  if(m_pMap != NULL){
    m_pMap->DeleteUnitType(pUnitType);
  }

  // remove the graphics
  pUnitType->RemoveGraphics(m_Archive);

  // remove the unit type file from the archive
  // ignore all exceptions
  try{
    m_Archive.RemoveFile(pUnitType->GetUnitFileName());
  }
  catch(CException *e){
    e->Delete();
  }

  // delete the object
  pUnitType->Delete();
  delete pUnitType;
}

void CEUnitLibrary::Rebuild()
{
  m_Archive.Rebuild();
}
