// DataArchiveFilePrefixed.cpp: implementation of the CDataArchiveFilePrefixed class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DataArchiveFilePrefixed.h"
#include "DataArchive.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

BOOL CDataArchiveFilePrefixed::Create()
{
	strPrefix="";
	m_pArchiveFile=NULL;
	return TRUE;
}

BOOL CDataArchiveFilePrefixed::Create(CString strDirPath, CDataArchiveFile *pHostArchive)
{
	// Orezeme cestu
	strDirPath.TrimRight();
	strDirPath.TrimRight('\\');
	strDirPath.TrimRight('/');
	strPrefix=strDirPath;
		
	CDataArchiveInfo::Create(strDirPath, pHostArchive->m_dwFlags, CDataArchiveInfo::archiveFile);
	
	// Zapamatujeme si archiv
	m_pArchiveFile=pHostArchive;

	// Pripojeni k neznamemu archivu
	m_pArchiveFile->AttachToDataArchive(NULL);
	return TRUE;
}

CDataArchiveFilePrefixed::CDataArchiveFilePrefixed()
{
	Create();
}

CDataArchiveFilePrefixed::~CDataArchiveFilePrefixed()
{
	if(m_pArchiveFile) {
		// Odpojeni od neznameho archivu
		m_pArchiveFile->DetachFromDataArchive(NULL);
	}
}

CArchiveFile CDataArchiveFilePrefixed::CreateFile(LPCTSTR lpcszFileName, DWORD dwFlags, DWORD dwInitialSize) 
{ 
	ASSERT(m_pArchiveFile); 
	
	CArchiveFile f;
	f=m_pArchiveFile->CreateFile(CArchiveUtils::ConcatenatePaths(strPrefix, lpcszFileName), dwFlags, dwInitialSize);
	f.m_pFileInfo->ChangeDataArchive(this, lpcszFileName);
	
	return f;
}

CDataArchiveInfo *CDataArchiveFilePrefixed::CreateArchiveInfoClone()
{
	CDataArchiveFile *pInfo=(CDataArchiveFile*)m_pArchiveFile->CreateArchiveInfoClone();
	if(pInfo==m_pArchiveFile) {
		// Vnitrek se neklonoval, my se tez neklonujeme
		return this;
	} else {
		CDataArchiveFilePrefixed *pNewInfo=new CDataArchiveFilePrefixed();
		pNewInfo->m_pArchiveFile=pInfo;
		pInfo->AttachToDataArchive(NULL);
		pNewInfo->strPrefix=strPrefix;		
		return pNewInfo;
	}
}

void CDataArchiveFilePrefixed::AdoptFile(CArchiveFile &file)
{
	ASSERT(m_pArchiveFile!=NULL);

	CArchiveFileInfo *pInfo=file.m_pFileInfo;

	CDataArchiveFilePrefixed *pDA=(CDataArchiveFilePrefixed*)pInfo->m_pArchiveInfo;
	
	// Soubor musi byt ze stejne rodiny archivu
	// ASSERT(pDA->GetMappedFile()==GetMappedFile());

	// Prehakujeme si info
	pInfo->ChangeDataArchive(this, pInfo->m_strFileName);
}