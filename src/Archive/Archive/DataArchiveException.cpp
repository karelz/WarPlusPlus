// DataArchiveException.cpp: implementation of the CDataArchiveException class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DataArchiveException.h"
#include "DataArchiveInfo.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDataArchiveException::CDataArchiveException()
{
	// Zadna chyba
	m_eDataArchiveExceptionCause=EDataArchiveExceptionTypes::none;
	// v zadnem souboru
	m_strDataArchiveExceptionFileName="";
	// v zadnem archivu
	m_strDataArchiveExceptionArchiveName="";
	// Nikde
	m_eDataArchiveExceptionLocation=EDataArchiveExceptionLocations::nowhere;
}

CDataArchiveException::CDataArchiveException(CFileException *pFileException, DWORD eWhereHappened, CDataArchiveInfo *pArchiveInfo) {
	ASSERT(pFileException);
	m_strDataArchiveExceptionFileName=pFileException->m_strFileName;
	m_eDataArchiveExceptionCause=(CDataArchiveException::EDataArchiveExceptionTypes)pFileException->m_cause;
	m_eDataArchiveExceptionLocation=(CDataArchiveException::EDataArchiveExceptionLocations)eWhereHappened;
	if(pArchiveInfo) m_strDataArchiveExceptionArchiveName=pArchiveInfo->GetArchivePath();
	pFileException->Delete(); // Mazeme starou vyjimku
}

CDataArchiveException::CDataArchiveException(CFileException *pFileException, CDataArchiveInfo *pArchiveInfo, DWORD eWhereHappened) {
	ASSERT(pFileException);
	m_strDataArchiveExceptionFileName=pFileException->m_strFileName;
	m_eDataArchiveExceptionCause=(CDataArchiveException::EDataArchiveExceptionTypes)pFileException->m_cause;
	m_eDataArchiveExceptionLocation=(CDataArchiveException::EDataArchiveExceptionLocations)eWhereHappened;
	if(pArchiveInfo) m_strDataArchiveExceptionArchiveName=pArchiveInfo->GetArchivePath();
	pFileException->Delete(); // Mazeme starou vyjimku
}

CDataArchiveException::CDataArchiveException(CString strFileName, DWORD eErrorCode, DWORD eWhereHappened, CDataArchiveInfo *pArchiveInfo) {
	m_strDataArchiveExceptionFileName=strFileName;
	m_eDataArchiveExceptionCause=(CDataArchiveException::EDataArchiveExceptionTypes)eErrorCode;
	m_eDataArchiveExceptionLocation=(CDataArchiveException::EDataArchiveExceptionLocations)eWhereHappened;
	if(pArchiveInfo) m_strDataArchiveExceptionArchiveName=pArchiveInfo->GetArchivePath();
}

CDataArchiveException::CDataArchiveException(CDataArchiveException *pArchiveException, CString strFileName, DWORD eWhereHappened, CDataArchiveInfo *pArchiveInfo) {
	ASSERT(pArchiveException);
	m_strDataArchiveExceptionFileName=strFileName;
	m_eDataArchiveExceptionCause=pArchiveException->GetErrorCode();
	m_eDataArchiveExceptionLocation=(CDataArchiveException::EDataArchiveExceptionLocations)eWhereHappened;
	if(pArchiveInfo) m_strDataArchiveExceptionArchiveName=pArchiveInfo->GetArchivePath();
	pArchiveException->Delete();
}

CDataArchiveException::~CDataArchiveException()
{
}

BOOL CDataArchiveException::GetErrorMessage(LPTSTR lpszError, UINT nMaxError, PUINT pnHelpContext) {
	strcpy(lpszError, (LPCTSTR)(GetErrorMessage().Left(nMaxError-1)));
	return TRUE;
}

CString CDataArchiveException::GetErrorMessage() {
	CString strErrorString;
	CString strErrorLocation;
	switch(m_eDataArchiveExceptionCause) {
		case none:
			strErrorString="No error."; break;
		
		case generic:
			strErrorString="Unspecified error."; break;
	
		case fileNotFound:
			strErrorString="File not found."; break;
		
		case badPath:
			strErrorString="All or part of path is invalid."; break;
		
		case tooManyOpenFiles:
			strErrorString="The permitted number of open files was exceeded."; break;

		case accessDenied:
			strErrorString="The file could not be accessed."; break;

		case invalidFile:
			strErrorString="Attempt to use an invalid file handle."; break;

		case removeCurrentDir:
			strErrorString="The current working directory cannot be removed."; break;

		case directoryFull:
			strErrorString="There are no more directory entries."; break;

		case badSeek:   
			strErrorString="There was an error trying to set the file pointer."; break;

		case hardIO:
			strErrorString="There was a hardware error."; break;

		case sharingViolation:
			strErrorString="SHARE.EXE was not loaded, or a shared region was locked."; break;
		
		case lockViolation:
			strErrorString="There was an attempt to lock a region that was already locked."; break;

		case diskFull:
			strErrorString="The disk is full."; break;

		case endOfFile:
			strErrorString="The end of file was reached."; break;

		case alreadyExists:
			strErrorString="Object already exists."; break;

		case notADirectory:
			strErrorString="Not a directory."; break;
		
		case makeDirFailed:
			strErrorString="Can't make directory."; break;

		case rmDirFailed:
			strErrorString="Can't remove directory."; break;

		case dirNotEmpty:
			strErrorString="Directory is not empty."; break;

		case fileNotAssigned:
			strErrorString="File was not assigned, yet."; break;

		case unknownArchiveFormat:
			strErrorString="Unknown archive format."; break;
		
		case archiveCorrupt:
			strErrorString="Archive structure is corrupt."; break;
		
		case decompressionFailed:
			strErrorString="Decompression failed."; break;

		case compressionFailed:
			strErrorString="Compression failed."; break;

		default:
			strErrorString="Unknown error."; break;
	}

	switch(m_eDataArchiveExceptionLocation) {
		case nowhere:
			strErrorLocation="?"; break;
		
		case fileInArchive:
			strErrorLocation="Archive file"; 
			if(!m_strDataArchiveExceptionArchiveName.IsEmpty()) {				
				strErrorLocation+=" ["+m_strDataArchiveExceptionArchiveName+"]";
			}			
			break;

		case archiveItself:
			strErrorLocation="Archive";
			if(!m_strDataArchiveExceptionArchiveName.IsEmpty()) {				
				strErrorLocation+=" ["+m_strDataArchiveExceptionArchiveName+"]";
			}
			break;

		case temporaryDirectory:
			strErrorLocation="Temporary directory"; break;

		case unknownLocation:
			strErrorLocation=""; break;
	}
	
	if(strErrorLocation.IsEmpty())
		return "\"" + m_strDataArchiveExceptionFileName+"\" : "+strErrorString;
	else
		return strErrorLocation+" error : \"" + m_strDataArchiveExceptionFileName+"\" : "+strErrorString;
}
