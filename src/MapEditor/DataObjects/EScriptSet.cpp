// EScriptSet.cpp: implementation of the CEScriptSet class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "EScriptSet.h"

#include "Common\Map\Map.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CEScriptSet::CEScriptSet()
{
  DWORD m_dwID = 0;
}

CEScriptSet::~CEScriptSet()
{

}

#ifdef _DEBUG

void CEScriptSet::AssertValid() const
{
  CObject::AssertValid();

  ASSERT(m_dwID != 0);
}

void CEScriptSet::Dump(CDumpContext &dc) const
{
  CObject::Dump(dc);
}

#endif

void CEScriptSet::Create(DWORD dwID, CDataArchive Archive)
{
  m_strFullName = Archive.GetArchivePath();
  int nPos = m_strFullName.ReverseFind('\\');
  if(nPos == -1){
    m_strFileName = m_strFullName;
  }
  else{
    m_strFileName = m_strFullName.Mid(nPos + 1);
  }
  m_dwID = dwID;
}

void CEScriptSet::Create(CArchiveFile MapFile)
{
  SScriptSetHeader h;

  VERIFY(MapFile.Read(&h, sizeof(h)) == sizeof(h));

  m_dwID = h.m_dwID;
  m_strFileName = h.m_pFileName;
  m_strFullName = h.m_pFullName;
  m_strName = h.m_pName;

  CString strFileName = m_strFullName;
  // try to open the archive
RetryIt:;
  try{
    CDataArchive archive;
    archive.Create(strFileName, CArchiveFile::modeRead | CFile::shareDenyWrite, CDataArchiveInfo::archiveFile);
  }
  catch(CException *e){
    CString strAsk;
    strAsk.Format("SkriptSet '%s' nelze najít. Chcete ho najít sami?\n(Ne - SkriptSet se vymaže z mapy)", m_strFileName);
    if(AfxMessageBox(strAsk, MB_YESNO) == IDYES){
      CFileDialog dlg(TRUE, "ScriptSet", m_strFileName, OFN_HIDEREADONLY, "SkriptSety (*.ScriptSet)|*.ScriptSet|Všechny soubory (*.*)|*.*||");
      CString strTitle;
      strTitle.Format("Najít SkriptSet %s", m_strFileName);
      dlg.m_ofn.lpstrTitle = strTitle;

      if(dlg.DoModal() != IDOK){
        throw;
      }

      e->Delete();

      strFileName = dlg.GetPathName();

      goto RetryIt;
    }

    throw;
  }

  m_strFullName = strFileName;
  int nPos = m_strFullName.ReverseFind('\\');
  if(nPos == -1){
    m_strFileName = m_strFullName;
  }
  else{
    m_strFileName = m_strFullName.Mid(nPos + 1);
  }
}

void CEScriptSet::Delete()
{
  m_dwID = 0;
}

void CEScriptSet::Save(CArchiveFile MapFile)
{
  SScriptSetHeader h;

  memset(&h, 0, sizeof(h));
  strncpy(h.m_pFileName, m_strFileName, 255);
  strncpy(h.m_pFullName, m_strFullName, 1023);
  strncpy(h.m_pName, m_strName, 63);
  h.m_dwID = m_dwID;

  MapFile.Write(&h, sizeof(h));
}

void CEScriptSet::SetPath(CString strPath)
{
  m_strFullName = strPath;
  int nPos = m_strFullName.ReverseFind('\\');
  if(nPos == -1){
    m_strFileName = m_strFullName;
  }
  else{
    m_strFileName = m_strFullName.Mid(nPos + 1);
  }
}

void CEScriptSet::Export(CDataArchive TheMap)
{
  CDataArchive source;

  source.Create(m_strFullName, CArchiveFile::modeRead | CFile::shareDenyWrite, CDataArchiveInfo::archiveFile);
  
  TheMap.MakeDirectory(m_strFileName);
  TheMap.AppendDir(m_strFileName, source, "");
}