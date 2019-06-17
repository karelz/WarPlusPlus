// PersistentStorage.cpp: implementation of the CPersistentStorage class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PersistentStorage.h"
#include "PersistentLoadException.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CPersistentStorage, CObject);

#ifdef PERSISTENT_STORAGE_TAGGED
DWORD CPersistentStorageAutoBracing::g_dwBraceNumber=0;
#endif

CPersistentStorage::CPersistentStorage()
{
    m_eStorageMode = NONE;
}

CPersistentStorage::~CPersistentStorage()
{
}

#ifdef _DEBUG

void CPersistentStorage::AssertValid() const
{
  CObject::AssertValid();
}

void CPersistentStorage::Dump(CDumpContext &dc) const
{
  CObject::Dump(dc);
}

#endif

bool CPersistentStorage::Open( LPCTSTR lpszFilename, CPersistentStorage::EStorageMode eMode)
{
#ifdef PERSISTENT_STORAGE_TAGGED
    CPersistentStorageAutoBracing::g_dwBraceNumber=0;
#endif

    bool bFileOpened = false, bArchiveOpened = false;
    try
    {
	    m_eStorageMode = NONE;
        m_tpPointers.RemoveAll();
	    
	    if (eMode == SAVE)
	    {
            if (!m_DataArchive.Create( lpszFilename, CArchiveFile::modeCreate | CArchiveFile::modeWrite, 
                                                CDataArchiveInfo::archiveDirectory)) return false;
            m_DataArchive.Open();
            bArchiveOpened = true;

            m_ArchiveFile = m_DataArchive.CreateFile("savegame", CArchiveFile::modeCreate | CArchiveFile::modeWrite | 
                                                         CArchiveFile::modeUncompressed);
            bFileOpened = true;

            m_eStorageMode = SAVE;
	    }
	    else if (eMode == LOAD)
	    {
            if (!m_DataArchive.Create( lpszFilename, CArchiveFile::modeRead | CFile::shareDenyWrite, CDataArchiveInfo::archiveDirectory)) return false;
            m_DataArchive.Open();
            bArchiveOpened = true;

            m_ArchiveFile = m_DataArchive.CreateFile("savegame", CArchiveFile::modeRead | CArchiveFile::modeUncompressed | CFile::shareDenyWrite);
            bFileOpened = true;

            m_eStorageMode = LOAD;
            m_tpPointers.SetAt( NULL, NULL);
        }
	    else ASSERT(false);
    }
    catch (...)
    {
        if (bFileOpened) m_ArchiveFile.Close();
        if (bArchiveOpened) m_DataArchive.Close();
        throw;
    }

    return true;
}

bool CPersistentStorage::Open( CDataArchive DataArchive, CPersistentStorage::EStorageMode eMode)
{
#ifdef PERSISTENT_STORAGE_TAGGED
    CPersistentStorageAutoBracing::g_dwBraceNumber=0;
#endif

    bool bFileOpened = false, bArchiveOpened = false;
    try
    {
	    m_eStorageMode = NONE;
        m_tpPointers.RemoveAll();
	    
        m_DataArchive=DataArchive;
        bArchiveOpened = true;
	    
        if (eMode == SAVE)
	    {
            m_ArchiveFile = m_DataArchive.CreateFile("savegame", CArchiveFile::modeCreate | CArchiveFile::modeWrite | 
                                                         CArchiveFile::modeUncompressed);
            bFileOpened = true;

            m_eStorageMode = SAVE;
	    }
	    else if (eMode == LOAD)
	    {
            m_ArchiveFile = m_DataArchive.CreateFile("savegame", CArchiveFile::modeRead | CArchiveFile::modeUncompressed | CFile::shareDenyWrite);
            bFileOpened = true;

            m_eStorageMode = LOAD;
            m_tpPointers.SetAt( NULL, NULL);
        }
	    else ASSERT(false);
    }
    catch (...)
    {
        if (bFileOpened) m_ArchiveFile.Close();
        if (bArchiveOpened) m_DataArchive.Close();
        throw;
    }

    return true;
}

void CPersistentStorage::Close()
{
    m_ArchiveFile.Close();
    m_DataArchive.Close();
}

void CPersistentStorage::Write(const void *pBuffer, DWORD dwLength)
{
  ASSERT( m_eStorageMode == SAVE);
#ifdef PERSISTENT_STORAGE_TAGGED
  m_ArchiveFile.Write( (void *)&dwLength, sizeof(dwLength));
#endif
  m_ArchiveFile.Write( (void*)pBuffer, dwLength);
}

void CPersistentStorage::Read(void *pBuffer, DWORD dwLength)
{
  ASSERT( m_eStorageMode == LOAD);
#ifdef PERSISTENT_STORAGE_TAGGED
  DWORD dwStoredLength;
  m_ArchiveFile.Read( (void *)&dwStoredLength, sizeof(dwStoredLength));
  
  ASSERT(dwStoredLength==dwLength);
#endif

	m_ArchiveFile.Read( pBuffer, dwLength);
}

void CPersistentStorage::RegisterPointer(void *pOldPointer, void *pNewPointer)
{
    ASSERT( m_eStorageMode == LOAD);
	m_tpPointers.SetAt( pOldPointer, pNewPointer);
}

void* CPersistentStorage::TranslatePointer(void *pPointer)
{
    ASSERT( m_eStorageMode == LOAD);

	void *pRet;
	if (!m_tpPointers.Lookup( pPointer, pRet)) LOAD_ASSERT(false);
	else return pRet;
}
