// MappedFileCommon.cpp: implementation of the CMappedFileCommon class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MappedFileCommon.h"
#include "DataArchiveException.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMappedFileCommon::CMappedFileCommon()
{
	m_nRefCount=0;
}

CMappedFileCommon::~CMappedFileCommon()
{
	ASSERT(m_nRefCount==0);
}

void CMappedFileCommon::AddRef() {
	ASSERT(m_nRefCount>=0);
	m_nRefCount++;
}

void CMappedFileCommon::Release() {
	ASSERT(m_nRefCount>0);
	m_nRefCount--;
	if(m_nRefCount==0) {
		// Koncime
		delete this;
	}
}

UINT CMappedFileCommon::SeekRead(DWORD dwPos, void *pData, UINT nCount)
{
	UINT nDataRead;
	Lock();
	try {
		m_File.Seek(dwPos, CFile::begin);
		nDataRead=m_File.Read(pData, nCount);
		Unlock();
		return nDataRead;
	} catch(CFileException *fileException) { 
		Unlock();
		throw new CDataArchiveException(fileException); 
		return 0;
	}
}

void CMappedFileCommon::SeekForceRead(DWORD dwPos, void *pData, UINT nCount)
{
	UINT nDataRead;
	Lock();
	try {
		m_File.Seek(dwPos, CFile::begin);
		nDataRead=m_File.Read(pData, nCount);
		Unlock();
		if(nDataRead!=nCount) {
			throw new CDataArchiveException(m_File.GetFilePath(), 
					  CDataArchiveException::EDataArchiveExceptionTypes::endOfFile, 
				      CDataArchiveException::EDataArchiveExceptionLocations::archiveItself);
			return;
		}
		return;
	} catch(CFileException *fileException) { 
		Unlock();
		throw new CDataArchiveException(fileException); 
		return;
	}
}

void CMappedFileCommon::SeekWrite(DWORD dwPos, void *pData, UINT nCount)
{
}
