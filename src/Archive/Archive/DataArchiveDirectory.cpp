// DataArchiveDirectory.cpp: implementation of the CDataArchiveDirectory class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DataArchiveDirectory.h"
#include "ArchiveFilePhysical.h"

#include <direct.h>
#include <errno.h>
#include "Definitions.h"
#include "DataArchiveFileDirectoryItem.h"
#include "DataArchiveDirContents.h"
#include "ArchiveFileCompressed.h"
#include "ArchiveUtils.h"

#include "../Compression/CompressEngine.h"
#include "DataArchive.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDataArchiveDirectory::CDataArchiveDirectory()
{
	Create();
}

CDataArchiveDirectory::~CDataArchiveDirectory()
{
}

void CDataArchiveDirectory::Open()
{
	m_bOpened=TRUE;
}

void CDataArchiveDirectory::Close()
{
	m_bOpened=FALSE;
}

BOOL CDataArchiveDirectory::Create()
{
	// Nechame predka delat co umi
	CDataArchiveInfo::Create();

	// Jenom si nastavime spravny typ
	m_eArchiveType=CDataArchiveInfo::archiveDirectory;

	return SUCCESS;
}

BOOL CDataArchiveDirectory::Create(CString lpcszFileName, DWORD dwFlags, DWORD dwArchiveType)
{
	// Musi po mne chtit udelat adresar
	ASSERT(dwArchiveType==archiveDirectory);
	
	// Volame si vlastni create
	CDataArchiveInfo::Create(lpcszFileName, dwFlags, dwArchiveType);

	if(dwFlags & CArchiveFile::modeCreate) { // Bude treba ho vytvorit
		int errMkdir=_mkdir(lpcszFileName);
		if(errMkdir==0) { // Nedoslo k chybe
			return SUCCESS;
		} else {
			if(errno!=EEXIST) {
				// Pokud chyba NEbyla ta, ze adresar jiz existoval... rveme
				throw new CDataArchiveException(lpcszFileName, CDataArchiveException::EDataArchiveExceptionTypes::makeDirFailed,
					CDataArchiveException::EDataArchiveExceptionLocations::archiveItself, this);
				return FAILURE;
			} else {
				// Adresar uz existoval, ale to nam nevadi
				return SUCCESS;
			}
		}
	} else { // Nic se nedeje
	}
	return SUCCESS;
}

void CDataArchiveDirectory::Rebuild()
{

}

CArchiveFile CDataArchiveDirectory::CreateFile(LPCTSTR lpcszFileName, DWORD dwFlags)
{
	CString strPhysName=lpcszFileName;
    if(!m_strArchiveName.IsEmpty()) { // Pokud neni jmeno archivu prazdne, rezeme lomitka na zacatku jmena souboru
        strPhysName.TrimLeft(); strPhysName.TrimLeft('\\');
    }
	CString strFileName=strPhysName; // Bez lomitek
	if(!m_strArchiveName.IsEmpty()) { // Pokud neni jmeno archivu prazdne, prilepime ho na zacatek cesty
		strPhysName=m_strArchiveName+'\\'+strPhysName;
	} 
	
	BOOL bCmp=FALSE; // Priznak otevirani pakovaneho souboru
	strPhysName.TrimRight();
	strPhysName.TrimRight('\\');
	CString strCmpName=strPhysName+CCompressEngine::GetDefaultArchiveExtension();

	if(dwFlags & CArchiveFile::modeCreate) { // Vytvarime novy
		if(!(dwFlags & CArchiveFile::modeUncompressed)) { // Pakovany soubor
			strPhysName=strCmpName;
			bCmp=TRUE;
		}
	} else { // Otvirame stary
		bCmp=CArchiveUtils::IsCompressed(strPhysName);
		if(bCmp) {
			strPhysName=strCmpName;
		}
	}

	if((dwFlags & CArchiveFile::modeCreate) && CArchiveUtils::FlagWrite(m_dwFlags)) {
		// Soubory se pri vytvareni delaji zapisovatelne
		// Pokud ma i sam archiv pravo na zapis
		if(!(dwFlags & CArchiveFile::modeReadWrite)) {
			dwFlags |=CArchiveFile::modeWrite;
		}
	}		

	if(bCmp) {
		if(dwFlags & CArchiveFile::modeWrite) {
			// Komprimovane soubory vyzaduji cteni i zapis, nikdy samotny zapis
			dwFlags &=~CArchiveFile::modeWrite;
			dwFlags |=CArchiveFile::modeReadWrite; 
		}		
	}
	CArchiveFilePhysical *PhysicalFile=new CArchiveFilePhysical(strFileName, strPhysName, dwFlags);

	if(bCmp && !(dwFlags & CArchiveFile::modeRaw)) {
		CArchiveFile wrap(PhysicalFile, this);
		CArchiveFileCompressed *pCompressedFile=new CArchiveFileCompressed(wrap);
		CArchiveFile ArchiveFileComp(pCompressedFile, this);
		return ArchiveFileComp;
	} else {
		CArchiveFile ArchiveFilePhys(PhysicalFile, this);
 		return ArchiveFilePhys;
	}
}

CDataArchive CDataArchiveDirectory::CreateArchive(LPCTSTR lpcszPath, BOOL bCreate) 
{
	CDataArchiveDirectory *pArchive;
	DWORD flags;
	
	pArchive = new CDataArchiveDirectory();
	
	// Zjistime priznaky, pokud bCreate nastavene na false, nulujeme modeCreate
	// jinak nastavujeme
	flags=m_dwFlags;
	if(bCreate) 
		flags|=CArchiveFile::modeCreate;
	else
		flags&=~CArchiveFile::modeCreate;

	pArchive->Create(CArchiveUtils::ConcatenatePaths(m_strArchiveName, lpcszPath), flags, CDataArchiveInfo::archiveDirectory);
	
	CDataArchive archive(pArchive);
	return archive;
}

void CDataArchiveDirectory::AppendFile(LPCTSTR lpcszFileName, LPCTSTR lpcszFilePath, DWORD dwFlags)
{
	CString strFilePath=lpcszFilePath;
	// Orezeme cestu uvnitr archivu
	strFilePath.Replace('/', '\\');
	strFilePath.TrimRight(); 
    if(!m_strArchiveName.IsEmpty()) {
        strFilePath.TrimLeft(); strFilePath.TrimLeft('\\');
    }
	// Stare jmeno souboru
	CString strOldName=lpcszFileName;
	strOldName.Replace('/', '\\');
	strOldName.TrimRight(); strOldName.TrimLeft(); strOldName.TrimLeft('\\');
	// Nove jmeno souboru
    
    CString strNewName=CArchiveUtils::ConcatenatePaths(m_strArchiveName, strFilePath);
	if(!CArchiveUtils::IsFile(strOldName) && !CArchiveUtils::IsFile(strOldName+CCompressEngine::GetDefaultArchiveExtension())) { // Neni takovy soubor
		throw new CDataArchiveException(strOldName, CDataArchiveException::fileNotFound, CDataArchiveException::archiveItself, this);
	} else {
		try { // Zkusime prekopirovat
			if(CArchiveUtils::IsFile(strOldName)) {
				CDataArchiveDirectory::CopyFile(strOldName, strNewName);
			} else {
				CDataArchiveDirectory::CopyFile(strOldName+CCompressEngine::GetDefaultArchiveExtension(), strNewName+CCompressEngine::GetDefaultArchiveExtension());
			}
		} catch (CFileException *fileException) {
			// Nepovedlo se, hazime vyjimku
			CDataArchiveException *archiveException=new CDataArchiveException(lpcszFileName, fileException->m_cause, CDataArchiveException::fileInArchive, this);
			fileException->Delete();
			throw archiveException;
		}
	}
}

void CDataArchiveDirectory::AppendFile(CArchiveFile file, LPCTSTR lpcszFilePath, DWORD dwFlags)
{
	char *pBuf;
	CFile dest;
	int nCount;
	CFileException *pError;

	pBuf=NULL;
	// Buffer.. alokace
	pBuf = new char[ARCHIVE_DIRECTORY_COPY_BUF_SIZE];

	CString strPath=lpcszFilePath;
	if(file.IsCompressed() && file.IsRaw())
		strPath+=CCompressEngine::GetDefaultArchiveExtension();
		
	pError=new CFileException();
	if(!dest.Open(CArchiveUtils::ConcatenatePaths(m_strArchiveName, strPath), CFile::modeCreate|CFile::modeWrite|CArchiveFile::modeRaw, pError)) { // Otevreme se
		if(pBuf)
			delete pBuf;		
		throw new CDataArchiveException(pError, this); // Chyba, hazime vyjimku
	} else {
		pError->Delete();  // Mazeme nepotrebnou vyjimku
	}

	try { 
		while((nCount=file.Read(pBuf, ARCHIVE_DIRECTORY_COPY_BUF_SIZE))>0) {
			dest.Write(pBuf, nCount);
		}

		if(file.IsReadOnly()) {
			CFileStatus st;
			CString path=CArchiveUtils::ConcatenatePaths(m_strArchiveName, strPath);
			dest.Close();
			if(CFile::GetStatus(path, st)) {
				st.m_attribute|=CFile::Attribute::readOnly;
				CFile::SetStatus(path, st);
			}		
		}

	} catch(CFileException *pCopyError) { 
		if(pBuf)
			delete pBuf;
		throw new CDataArchiveException(pCopyError, this); 
	} catch(...) {
		if(pBuf)
			delete pBuf;
		throw;
	}

	if(pBuf)
		delete pBuf;
}

void CDataArchiveDirectory::AppendDir(LPCTSTR lpcszDirPathTo, CDataArchive ArchiveFrom, LPCTSTR lpcszDirPathFrom, EAppendModes nFlags /* = appendRecursive */)
{
	CString strPathTo=lpcszDirPathTo;
	CString strPathFrom=lpcszDirPathFrom;
	CString newName, oldName;
    BOOL bRecursive=((nFlags & appendNonRecursive)==0);

	strPathTo.TrimRight();
	strPathTo.TrimRight('\\');
	strPathTo.TrimRight('/');	
	
	if(!strPathTo.IsEmpty()) {
		MakeDirNoExceptions(strPathTo);
	}

	CDataArchiveDirContents *pContents;
	pContents=ArchiveFrom.GetDirContents(strPathFrom);

	if(!pContents) return;
	
	BOOL bContinue=pContents->MoveFirst();
	while(bContinue) {
		newName=CArchiveUtils::ConcatenatePaths(strPathTo, pContents->GetInfo()->GetName());
		oldName=CArchiveUtils::ConcatenatePaths(strPathFrom, pContents->GetInfo()->GetName());
		if(pContents->GetInfo()->IsDirectory()) {
			// Je to adresar
            if(bRecursive) {
			    try {			
				    MakeDirectory(newName);
			    } catch(CDataArchiveException *ex) {
				    if(ex->GetErrorCode()==CDataArchiveException::EDataArchiveExceptionTypes::alreadyExists) {
					    // Adresar existuje? To lze ignorovat
					    ex->Delete();
				    } else {
					    // Nejaka zavaznejsi chyba
					    throw ex;
				    }
			    }
			    // Rekurzivne zavolame sami sebe
			    AppendDir(newName, ArchiveFrom, oldName, nFlags);
            }
		} else {
			// Neni to adresar
			CArchiveFile f;
            if((nFlags & appendUncompressed) != 0) {
			    f=ArchiveFrom.CreateFile(oldName, CFile::modeRead | CFile::shareDenyWrite);
            } else {
                f=ArchiveFrom.CreateFile(oldName, CFile::modeRead | CArchiveFile::modeRaw | CFile::shareDenyWrite);
            }
			AppendFile(f, newName, nFlags);
		}
		bContinue=pContents->MoveNext();
	}

	delete pContents;
}

void CDataArchiveDirectory::ExtractFile(LPCTSTR lpcszFileName, LPCTSTR lpcszFilePath)
{
	CString strFilePath=lpcszFilePath;
	// Orezeme cestu uvnitr archivu
	strFilePath.TrimRight(); 
    if(!m_strArchiveName.IsEmpty()) {
        strFilePath.TrimLeft(); strFilePath.TrimLeft('\\');
    }
	// Nove jmeno souboru
    CString strNewName=CArchiveUtils::ConcatenatePaths(m_strArchiveName, strFilePath);
	if(!CArchiveUtils::IsFile(strNewName) && !(CArchiveUtils::IsFile(strNewName+CCompressEngine::GetDefaultArchiveExtension()))) { // Neni takovy soubor
		throw new CDataArchiveException(strNewName, CDataArchiveException::fileNotFound, CDataArchiveException::archiveItself, this);
	} else {
		try { // Zkusime prekopirovat
			CString strOldName=lpcszFileName;
			if(!CArchiveUtils::IsFile(strNewName)) {
				strNewName+=CCompressEngine::GetDefaultArchiveExtension();
				strOldName+=CCompressEngine::GetDefaultArchiveExtension();
			}
			CDataArchiveDirectory::CopyFile(strNewName, strOldName);
		} catch (CFileException *fileException) {
			// Nepovedlo se, hazime vyjimku
			CDataArchiveException *archiveException=new CDataArchiveException(lpcszFileName, fileException->m_cause, CDataArchiveException::fileInArchive, this);
			fileException->Delete();
			throw archiveException;
		}
	}
}

void CDataArchiveDirectory::RemoveFile(LPCTSTR lpcszFileName)
{
	CString strFileName=m_strArchiveName+'\\'+lpcszFileName;
	if(!CArchiveUtils::IsFile(strFileName)  && !(CArchiveUtils::IsFile(strFileName+CCompressEngine::GetDefaultArchiveExtension()))) { // Neni takovy soubor
		throw new CDataArchiveException(strFileName, CDataArchiveException::fileNotFound, CDataArchiveException::archiveItself, this);
	} else {
		try { // Zkusime smazat
			if(CArchiveUtils::IsFile(strFileName))
				CFile::Remove(strFileName);
			else
				CFile::Remove(strFileName+CCompressEngine::GetDefaultArchiveExtension());
		} catch (CFileException *fileException) {
			// Nepovedlo se, hazime vyjimku
			CDataArchiveException *archiveException=new CDataArchiveException(strFileName, fileException->m_cause, CDataArchiveException::fileInArchive, this);
			fileException->Delete();
			throw archiveException;
		}
	}
}

int CDataArchiveDirectory::MakeDirNoExceptions(LPCTSTR lpcszDirectoryName)
{
    int nErrCode;
	CString strDirName=lpcszDirectoryName;
    if(!m_strArchiveName.IsEmpty()) {
        strDirName.TrimLeft(); strDirName.TrimLeft('\\');
    }
    nErrCode=_mkdir(CArchiveUtils::ConcatenatePaths(m_strArchiveName, strDirName));
    if(nErrCode!=0) { // Nastala chyba
        return errno;
	}
    return 0;
}

void CDataArchiveDirectory::MakeDirectory(LPCTSTR lpcszDirectoryName)
{
	switch(MakeDirNoExceptions(lpcszDirectoryName)) {
        case 0: // Vsechno v poradku
            // Nic nedelame
            break;
		case ENOENT: // Nelze vytvorit adresar
			throw new CDataArchiveException(lpcszDirectoryName, CDataArchiveException::makeDirFailed, CDataArchiveException::archiveItself, this);
			break;
		case EEXIST:
			throw new CDataArchiveException(lpcszDirectoryName, CDataArchiveException::alreadyExists, CDataArchiveException::archiveItself, this);
			break;
		default:
			throw new CDataArchiveException(lpcszDirectoryName, CDataArchiveException::generic, CDataArchiveException::archiveItself, this);
			break;		
	}
}

void CDataArchiveDirectory::RemoveDirectory(LPCTSTR lpcszDirectoryName)
{
	int nErrCode;
	CString strDirName=lpcszDirectoryName;
    if(!m_strArchiveName.IsEmpty()) {
        strDirName.TrimLeft(); strDirName.TrimLeft('\\');
    }
    nErrCode=_rmdir(CArchiveUtils::ConcatenatePaths(m_strArchiveName, strDirName));
	if(nErrCode!=0) { // Nastala chyba
		switch(errno) {
		case ENOENT:
			throw new CDataArchiveException(lpcszDirectoryName, CDataArchiveException::rmDirFailed, CDataArchiveException::archiveItself, this);
			break;
		case ENOTEMPTY:
			throw new CDataArchiveException(lpcszDirectoryName, CDataArchiveException::dirNotEmpty, CDataArchiveException::archiveItself, this);
			break;
		default:
			throw new CDataArchiveException(lpcszDirectoryName, CDataArchiveException::generic, CDataArchiveException::archiveItself, this);
			break;
		}
	}
}

CDataArchiveDirContents *CDataArchiveDirectory::GetDirContents(CString strPath) {
	CFileFind find;
	strPath.Replace('/', '\\');
    if(!m_strArchiveName.IsEmpty()) {
        strPath.TrimLeft();
	    strPath.TrimLeft('\\');
	    strPath.TrimRight();
	    strPath.TrimRight('\\');
    }

	// Pocet souboru?
	CString strFind;
    if(m_strArchiveName.IsEmpty()) {
   	    if(strPath.IsEmpty()) 
		    strFind="*.*";
	    else
            strFind=CArchiveUtils::ConcatenatePaths(strPath, "*.*");
    } else {
	    if(strPath.IsEmpty()) 
		    strFind=CArchiveUtils::ConcatenatePaths(m_strArchiveName, "*.*");
	    else
		    strFind=CArchiveUtils::ConcatenatePaths(CArchiveUtils::ConcatenatePaths(m_strArchiveName, strPath), "*.*");
    }

	BOOL bWorking=find.FindFile(strFind);
	int nFiles=0;

	if(!bWorking) return NULL;
	while(bWorking) {
		bWorking=find.FindNextFile();
		if(!find.IsDots()) {
			nFiles++;
		}
	}
	
	// Nyni plnime
	bWorking=find.FindFile(strFind);
	int nPos=0;

	CDataArchiveFileDirectoryItem **m_pItems;
	m_pItems=new CDataArchiveFileDirectoryItem *[nFiles];

	if(!bWorking) return NULL;
	while(bWorking && nPos<nFiles) {
		bWorking=find.FindNextFile();
		if(!find.IsDots()) {
			CString strExtName=find.GetFileName();
			CArchiveUtils::ClipExtension(strExtName);
			m_pItems[nPos]=new CDataArchiveFileDirectoryItem(strExtName, find.IsDirectory()?CDataArchiveFile::directoryFlag:CDataArchiveFile::fileFlag, 0);
			nPos++;
		}
	}

	ASSERT(nPos==nFiles);

	return new CDataArchiveDirContents(nFiles, m_pItems);
}

BOOL CDataArchiveDirectory::CopyFile(LPCTSTR lpcszSourceFileName, LPCTSTR lpcszDestFileName)
{
	char *pBuf;
	CFile source, dest;
	int nCount;

	pBuf=NULL;
	// Buffer.. alokace
	pBuf = new char[ARCHIVE_DIRECTORY_COPY_BUF_SIZE];

	CFileException *pError=new CFileException();
	if(!source.Open(lpcszSourceFileName, CFile::modeRead | CFile::shareDenyWrite, pError)) { // Otevreme se
		if(pBuf) delete pBuf;
		throw new CDataArchiveException(pError); // Chyba, hazime vyjimku
		return FAILURE;
	} else {
		pError->Delete();  // Mazeme nepotrebnou vyjimku
	}

	pError=new CFileException();
	if(!dest.Open(lpcszDestFileName, CFile::modeCreate|CFile::modeWrite, pError)) { // Otevreme se
		if(pBuf) delete pBuf;		
		throw new CDataArchiveException(pError); // Chyba, hazime vyjimku
		return FAILURE;
	} else {
		pError->Delete();  // Mazeme nepotrebnou vyjimku
	}

	try { 
		while((nCount=source.Read(pBuf, ARCHIVE_DIRECTORY_COPY_BUF_SIZE))>0) {
			dest.Write(pBuf, nCount);
		}

		CFileStatus st;		
		dest.Close();
		if(CFile::GetStatus(lpcszSourceFileName, st)) {
			if(st.m_attribute & CFile::Attribute::readOnly) {
				st.m_attribute|=CFile::Attribute::readOnly;
				st.m_mtime=st.m_atime=st.m_ctime=0;
				st.m_size=0;
				strncpy(st.m_szFullName, lpcszDestFileName, _MAX_PATH);
				CFile::SetStatus(lpcszDestFileName, st);
			}
		}		

	} catch(CFileException *pCopyError) { 
		if(pBuf) delete pBuf;
		throw new CDataArchiveException(pCopyError); 
	}

	delete pBuf;
	return SUCCESS;
}
