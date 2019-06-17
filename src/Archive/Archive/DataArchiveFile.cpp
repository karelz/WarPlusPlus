// DataArchiveFile.cpp: implementation of the CDataArchiveFile class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DataArchive.h"
#include "DataArchiveDirectory.h"
#include "DataArchiveFile.h"
#include "DataArchiveFileDirectory.h"
#include "../Compression/CompressEngine.h"
#include "ArchiveUtils.h"
#include "DataArchiveFilePrefixed.h"
#include "DataArchiveFileCommon.h"

#include <direct.h>
#include <errno.h>
#include "Definitions.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDataArchiveFile::CDataArchiveFile()
{
	Create();
	m_pCache=NULL;
	m_pMappedFile=NULL;
	m_pRootDirectory=NULL;
	m_dwThreadID=0;

	m_pCommon=new CDataArchiveFileCommon();
	m_pCommon->AddRef(this);
}

CDataArchiveFile::~CDataArchiveFile()
{
	if(m_pCommon) {
		if(m_pCommon->m_bRebuildRunning) return;
		m_bDestructorRunning=TRUE;
		if(m_pCommon->m_bRebuildRequest) {
			m_pCommon->m_bRebuildRunning=TRUE;
			PerformRebuild();
		} else {
			CleanupDataMembers();
		}
	} else {
		CleanupDataMembers();
	}
	m_bDestructorRunning=FALSE;
}

BOOL CDataArchiveFile::Create()
{
	// Nechame predka delat co umi
	CDataArchiveInfo::Create();

	// Jenom si nastavime spravny typ
	m_eArchiveType=CDataArchiveInfo::archiveFile;

	return SUCCESS;
}

BOOL CDataArchiveFile::Create(CString strFileName, DWORD dwFlags, DWORD dwArchiveType)
{
	// Musi po mne chtit udelat soubor
	ASSERT(dwArchiveType==archiveFile);
	
	// Volame si vlastni create
	CDataArchiveInfo::Create(strFileName, dwFlags, dwArchiveType);

	// Jeste nas nikdo necreatnul
	ASSERT(!m_pMappedFile);

	// Zapamatujeme si, kdo nas otevrel
	m_dwThreadID=GetCurrentThreadId();

	// Zkusime vsechno udelat, kdyz ne - velka dealokace	
	try {
		// Zjistime, jestli nahodou archiv jiz neexistuje
		BOOL fileExisted;

		CFile f;
		CFileException ex;
		if(!f.Open(strFileName, CFile::modeRead | CFile::shareDenyWrite, &ex)) {
			fileExisted=FALSE;		
		} else {
			fileExisted=TRUE;
			f.Close();
		}

		if(fileExisted) {
			dwFlags&=~CArchiveFile::modeCreate;
		}

		// Vytvorime soubor
		m_pMappedFile=new CMappedFile();
		m_pMappedFile->Open(strFileName, dwFlags);

		// Alokace objektu
		m_pCommon->m_pMainHeader=new CDataArchiveFileMainHeader();
		m_pRootDirectory=new CDataArchiveFileDirectory();

		if((dwFlags & CArchiveFile::modeCreate) && !fileExisted) { // Vytvoreni archivu
			// Inicializujeme hlavicku archivu
			m_pCommon->m_pMainHeader->Init();
			m_pCommon->m_pMainHeader->RootOffset(sizeof(SDataArchiveFileMainHeader));

			// Zapiseme hlavicku do souboru
			m_pCommon->m_pMainHeader->Write(*m_pMappedFile);

			m_pRootDirectory->CreateRoot(*m_pMappedFile, m_pCommon->m_pMainHeader->RootOffset());
		} else {
			// Nacteme hlavicku ze souboru
			m_pCommon->m_pMainHeader->Read(*m_pMappedFile);

			// Nacteme root directory
			m_pRootDirectory->Open(*m_pMappedFile, m_pCommon->m_pMainHeader->RootOffset());
		}

		// Vytvorime cache na adresare
		m_pCache=new CDataArchiveFileDirectoryCache(m_pRootDirectory, m_pMappedFile);

	} catch(...) { // Neco se nepovedlo			
		// Delame velkou dealokaci toho, co jsme provedli
		CleanupDataMembers();
		
		// A hazime vyjimku vys
		throw;
	}

	return SUCCESS;
}

void CDataArchiveFile::Open() {
    ASSERT(m_pMappedFile!=NULL);
    m_pMappedFile->Reopen();
	m_bOpened=m_pMappedFile->IsOpened();
    ASSERT(m_bOpened==TRUE);
}

void CDataArchiveFile::Close() {
    ASSERT(m_pMappedFile!=NULL);
    m_pMappedFile->Close();
	m_bOpened=m_pMappedFile->IsOpened();
}

BOOL CDataArchiveFile::IsOpened() {
    if(m_pMappedFile==NULL) return FALSE;
    return m_pMappedFile->IsOpened();
}

void CDataArchiveFile::Rebuild() {
	ASSERT(m_pCommon!=NULL);
	m_pCommon->m_bRebuildRequest=TRUE;
}

void CDataArchiveFile::PerformRebuild() {
	CTime startTime, endTime; CTimeSpan elapsedTime; CString strTime;
	ASSERT(m_pCommon!=NULL);
	
	if(m_pCommon->m_bRebuildRequest) {
    	startTime = CTime::GetCurrentTime();

		m_pCommon->m_bRebuildRequest=FALSE;
		CString strName=CDataArchive::GetNewTemporaryPath();
		CDataArchive *pTemp;
		
		// Vytvorime temporary archiv
		pTemp=new CDataArchive();
		pTemp->Create(strName, CArchiveFile::modeReadWrite | CArchiveFile::modeCreate, CDataArchiveInfo::archiveFile);

		// Prehrajeme do nej sva data
		try {
			pTemp->AppendDirNoRef("", this, "");
			// FlushData(*pTemp, "");
		} catch(...) {
			delete pTemp;
			throw;
		}

		// Uzavreme oba archivy
		delete pTemp;
		CleanupDataMembers();

		// A nyni jen prekopirujeme vytvoreny archiv pres nas puvodni
		CDataArchiveDirectory::CopyFile(strName, GetArchivePath());

		if(remove(strName)==-1) {
			throw new CDataArchiveException(strName, CDataArchiveException::accessDenied, CDataArchiveException::fileInArchive, this);
		}

		endTime = CTime::GetCurrentTime();
		elapsedTime = endTime - startTime;
		strTime = elapsedTime.Format("%M mins, %S secs.");
		TRACE("Time for performing rebuild: %s\n", (const char *)strTime);
	}
}

CArchiveFile CDataArchiveFile::CreateFile(LPCTSTR lpcszFileName, DWORD dwFlags, DWORD dwInitialSize, BOOL bGonnaFill) {
	CString fileName=lpcszFileName;
	CString newFile;
	CString path;

	// Rozdelime si cestu na vlastni cestu a soubor
	SplitPath(fileName, path, newFile);	

	// Jmeno nesmi byt prazdne
	if(fileName.IsEmpty()) {
		TRACE("Prazdne jmeno vytvareneho souboru\n");
		throw new CDataArchiveException(m_pMappedFile->Path(), CDataArchiveException::badPath, CDataArchiveException::archiveItself, this);
	}

	// Cesta musi vest k adresari...
	ASSERT(m_pCache);
	CDataArchiveFileDirectory Dir=m_pCache->GetDirectory(path);

	// .. a zjevne vede, jinak bychom uz tady nebyli...
	
	CDataArchiveFileDirectoryItem *pItem;
	pItem=Dir.FileInfo(newFile);
	if(dwFlags & CArchiveFile::modeCreate) { // Vytvarime novy soubor
		if(pItem==NULL || pItem->IsDeleted() || pItem->IsFile()) { 
			// Soubor neexistuje vubec - to je dobre...
			// anebo... neco tam existuje, ale je to smazane...
			// anebo... neco tam existuje a smazane to neni a je to soubor
			return Dir.CreateFile(newFile, fileName, this, dwFlags, dwInitialSize, bGonnaFill);
		} else {
			throw new CDataArchiveException(fileName, CDataArchiveException::alreadyExists, CDataArchiveException::archiveItself, this);
		}
	} else { // Otevirame stary soubor
		if(pItem==NULL || pItem->IsDeleted() || pItem->IsDirectory()) { // Soubor neexistuje
			throw new CDataArchiveException(fileName, CDataArchiveException::fileNotFound, CDataArchiveException::archiveItself, this);
		} else { // Neco tam existuje a smazane to neni
			return Dir.CreateFile(newFile, fileName, this, dwFlags, dwInitialSize);
		}
	}
}

CDataArchive CDataArchiveFile::CreateArchive(LPCTSTR lpcszPath, BOOL bCreate)
{
	CDataArchiveFilePrefixed *archiveFile;
	
	if(bCreate) {
		try {
			this->MakeDirectory(lpcszPath);
		} catch (CDataArchiveException *ex) {
			if(ex->GetErrorCode()==CDataArchiveException::EDataArchiveExceptionTypes::alreadyExists) {
				// Adresar uz existuje
				// To je v pohode
				ex->Delete();
			} else {
				// Neco je spatne
				throw ex;
			}			
		}
	}
	
	archiveFile=new CDataArchiveFilePrefixed();
	archiveFile->Create(lpcszPath, this);
	
	CDataArchive archive(archiveFile);
	return archive;
}

void CDataArchiveFile::AppendFile(LPCTSTR lpcszFileName, LPCTSTR lpcszFilePath, DWORD dwFlags) {
	CFile file;
	CFileException *pError;
	CString strFileName=lpcszFileName;
	CString strFilePath=lpcszFilePath;
	int nCount;
	char *pBuf=NULL;
	BOOL bSourceCompressed;
	CArchiveFile archiveFile;

	strFileName=CArchiveUtils::AddExtension(strFileName);
	// Zdrojovy soubor je komprimovany, pokud jeho jmeno konci na .wz
	bSourceCompressed=CArchiveUtils::HasExtension(strFileName);
	
	if(bSourceCompressed && (dwFlags==appendUncompressed)) {
		// Zdrojovy soubor je pakovany, ale chceme ho pridat nepakovane...
		// Tohle by nikdy nemelo nastat, protoze nikdo neni tak sileny, aby tohle
		// mohl po archivu chtit
		TRACE("ZBLAZNIL SES??? Fakt chces, abych SPAKOVANEJ soubor ROZBALIL do ARCHIVU???\n");
		
		archiveFile=CDataArchive::GetRootArchive()->CreateFile(lpcszFileName, CArchiveFile::modeRead | CFile::shareDenyWrite);
		AppendFile(archiveFile, lpcszFilePath, dwFlags);
		return;
	}
	
	// Zkusime otevrit
	pError=new CFileException();
	if(!file.Open(strFileName, CFile::modeRead | CFile::shareDenyWrite, pError)) {
		throw new CDataArchiveException(pError, this, CDataArchiveException::unknownLocation);
	} else {
		pError->Delete();
	}
	// Zkusime prekopirovat
	try {
		if(dwFlags==appendAuto) {
			// Pridava se presne to, co se cte
			if(bSourceCompressed) {
				archiveFile=CreateFile(strFilePath, CArchiveFile::modeCreate | CArchiveFile::modeWrite | CArchiveFile::modeRaw, file.GetLength());
			} else {			
				archiveFile=CreateFile(strFilePath, CArchiveFile::modeCreate | CArchiveFile::modeWrite | CArchiveFile::modeRaw | CArchiveFile::modeUncompressed, file.GetLength());
			}
		} else {			
			if(dwFlags==appendCompressed) {
				// Chceme mit pridany soubor komprimovany
				if(bSourceCompressed) {
					// A pridavany soubor uz komprimovany je - pouzivame modeRaw
					archiveFile=CreateFile(strFilePath, CArchiveFile::modeCreate | CArchiveFile::modeRaw | CArchiveFile::modeWrite, file.GetLength());
				} else {
					// Jenze pridavany soubor komprimovany neni, musime pri zapisovani pakovat data
					// A nemuzeme odhadnout pocatecni velikost
					archiveFile=CreateFile(strFilePath, CArchiveFile::modeCreate | CArchiveFile::modeWrite /*, file.GetLength() nepouzitelne */ );
				}
			} else {
				// Chceme mit pridany soubor nekomprimovany
				if(!bSourceCompressed) {
					// A zdroj take neni komprimovany, to je fajn
					archiveFile=CreateFile(strFilePath, CArchiveFile::modeCreate | CArchiveFile::modeWrite | CArchiveFile::modeUncompressed, file.GetLength());
				} else {
					// Zdroj bohuzel komprimovany je
					// Musime ho rozpakovat!!!
										
					// Sem se to nikdy nedostane, protoze se tenhle pripad 
					// odhali uz pred otevrenim file
					// -- jestli ne, neco je HODNE spatne

					ASSERT(FALSE);					
				}
			}
		}
		
		// Buffer.. alokace
		pBuf = new char[ARCHIVE_DIRECTORY_COPY_BUF_SIZE];

		while((nCount=file.Read(pBuf, ARCHIVE_DIRECTORY_COPY_BUF_SIZE))>0) {
			archiveFile.Write(pBuf, nCount);
		}

		CFileStatus st;
		if(file.GetStatus(st)) {
			if(st.m_attribute & CFile::Attribute::readOnly) {
				archiveFile.SetReadOnly();
			}
		}

		if(pBuf) delete pBuf;
	} catch(CFileException *pFE) {
		if(pBuf) delete pBuf;
		throw new CDataArchiveException(pFE, this, CDataArchiveException::unknownLocation);
	}
}

void CDataArchiveFile::AppendFile(CArchiveFile file, LPCTSTR lpcszFilePath, DWORD dwFlags) {
	CFile fileTo;
	CString strFilePath=lpcszFilePath;
	int nCount;
	char *pBuf=NULL;
	BOOL bCompressed;

	// Zkusime prekopirovat
	try {
		CArchiveFile archiveFile;
		bCompressed=(dwFlags==appendCompressed);
		if(dwFlags==appendAuto) {
			// Uzivatel nechal na nas, jak chceme soubor pripojit
			// Zachovavame kompreseni podle toho, co jsme dostali
			if(file.IsCompressed()) 
				bCompressed=appendCompressed;
			else
				bCompressed=appendUncompressed;
		}
		if(file.IsCompressed()) {
			if(bCompressed && file.IsRaw()) {
				// Mame compresseny soubor, ktery je otevreny jako RAW
				// Takze ho muzeme prekopirovat 1-1
				archiveFile=CreateFile(strFilePath, CArchiveFile::modeCreate | CArchiveFile::modeWrite | CArchiveFile::modeRaw, file.GetLength(), TRUE);
			} else if(bCompressed) {
				// Mame compresseny soubor, ktery NENI otevreny jako RAW
				// Takze ho musime rozpakovavat a zase spakovavat
				archiveFile=CreateFile(strFilePath, CArchiveFile::modeCreate | CArchiveFile::modeWrite, file.GetLength());
			} else if(!bCompressed && !file.IsRaw()) {
				// Mame compresseny soubor, ktery NENI otevreny jako RAW
				// Takze ho musime rozpakovavat
				archiveFile=CreateFile(strFilePath, CArchiveFile::modeCreate | CArchiveFile::modeWrite | CArchiveFile::modeUncompressed, file.GetLength());
			} else {
				// Mame compresseny soubor, ktery je otevreny jako RAW a chceme ho spakovat
				// Tato operace NENI podporovana. sorry.
				TRACE("Nemohu rozpakovat soubor otevreny na raw cteni. Sorry.\n");
				ASSERT(FALSE);
			}
		} else {
			if(bCompressed==appendUncompressed) {
				// Chceme ho pridat jako nekompreseny
				archiveFile=CreateFile(strFilePath, CArchiveFile::modeCreate | CArchiveFile::modeWrite | CArchiveFile::modeUncompressed, file.GetLength(), TRUE);
			} else {
				// Chceme soubor zkompresit
				archiveFile=CreateFile(strFilePath, CArchiveFile::modeCreate | CArchiveFile::modeWrite /* , file.GetLength() - bohuzel nezname vyslednou delku souboru */);
			}
		}
		
		// Buffer.. alokace
		pBuf = new char[ARCHIVE_DIRECTORY_COPY_BUF_SIZE];

		while((nCount=file.Read(pBuf, ARCHIVE_DIRECTORY_COPY_BUF_SIZE))>0) {
			archiveFile.Write(pBuf, nCount);
		}
	
		archiveFile.SetReadOnly(file.IsReadOnly());

		if(pBuf) delete pBuf;
	} catch(CFileException *pFE) {
		if(pBuf) delete pBuf;
		throw new CDataArchiveException(pFE, this, CDataArchiveException::unknownLocation);
	} catch(...) {
		if(pBuf) delete pBuf;
		throw;
	}
}

void CDataArchiveFile::AppendDir(LPCTSTR lpcszDirPathTo, CDataArchive ArchiveFrom, LPCTSTR lpcszDirPathFrom, EAppendModes nFlags /* = appendRecursive */)
{
	AppendDirNoRef(lpcszDirPathTo, ArchiveFrom.m_pArchiveInfo, lpcszDirPathFrom, nFlags);
}

void CDataArchiveFile::AppendDirNoRef(LPCTSTR lpcszDirPathTo, CDataArchiveInfo *pArchiveFrom, LPCTSTR lpcszDirPathFrom, EAppendModes nFlags /* = appendRecursive */)
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
	pContents=pArchiveFrom->GetDirContents(strPathFrom);

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
		        AppendDirNoRef(newName, pArchiveFrom, oldName, nFlags);
            }
		} else {
			// Neni to adresar
			CArchiveFile f;
            if((nFlags & appendUncompressed)!=0) {
			    f=pArchiveFrom->CreateFile(oldName, CFile::modeRead | CFile::shareDenyWrite);
            } else {
			    f=pArchiveFrom->CreateFile(oldName, CFile::modeRead | CArchiveFile::modeRaw | CFile::shareDenyWrite);
            }
			AppendFile(f, newName, nFlags);
		}
		bContinue=pContents->MoveNext();
	}

	delete pContents;
}

void CDataArchiveFile::ExtractFile(LPCTSTR lpcszFileName, LPCTSTR lpcszFilePath) {
	CFile file;
	CFileException *pError=new CFileException();
	CString strFileName=lpcszFileName;
	CString strFilePath=lpcszFilePath;
	int nCount;
	char *pBuf=NULL;

	CArchiveFile archiveFile;
	archiveFile=CreateFile(strFilePath, CArchiveFile::modeRead | CArchiveFile::modeRaw | CFile::shareDenyWrite);
	if(archiveFile.IsCompressed()) 
		strFileName+=CCompressEngine::GetDefaultArchiveExtension();

	// Zkusime otevrit
	if(!file.Open(strFileName, CFile::modeWrite | CFile::modeCreate, pError)) {
		throw new CDataArchiveException(pError, this, CDataArchiveException::unknownLocation);
	} else {
		pError->Delete();
	}
	// Zkusime prekopirovat
	try {		
		// Buffer.. alokace
		pBuf = new char[ARCHIVE_DIRECTORY_COPY_BUF_SIZE];

		while((nCount=archiveFile.Read(pBuf, ARCHIVE_DIRECTORY_COPY_BUF_SIZE))>0) {
			file.Write(pBuf, nCount);
		}

		if(pBuf) delete pBuf;
	} catch(CFileException *pFE) {
		if(pBuf) delete pBuf;
		throw new CDataArchiveException(pFE, this, CDataArchiveException::unknownLocation);
	}
}

void CDataArchiveFile::RemoveFile(LPCTSTR lpcszFileName) {
	CString fileName=lpcszFileName;
	CString removeFile;
	CString path;

	// Rozdelime si cestu na vlastni cestu a soubor
	SplitPath(fileName, path, removeFile);

	// Jmeno nesmi byt prazdne
	if(fileName.IsEmpty()) {
		TRACE("Prazdne jmeno mazaneho souboru\n");
		throw new CDataArchiveException(m_pMappedFile->Path(), CDataArchiveException::badPath, CDataArchiveException::archiveItself, this);
	}

	// Cesta musi vest k adresari...
	ASSERT(m_pCache);
	CDataArchiveFileDirectory Dir=m_pCache->GetDirectory(path);

	// .. a zjevne vede, jinak bychom uz tady nebyli...
	
	// Mazany adresar MUSI existovat
	CDataArchiveFileDirectoryItem *pItem;
	pItem=Dir.FileInfo(removeFile);
	if(pItem==NULL) { // Soubor neexistuje vubec
		throw new CDataArchiveException(fileName, CDataArchiveException::fileNotFound, CDataArchiveException::archiveItself, this);
	} else if(pItem->IsDeleted()) { // Neco tam existuje, ale je to smazane...
		throw new CDataArchiveException(fileName, CDataArchiveException::fileNotFound, CDataArchiveException::archiveItself, this);
	} else { // Neco tam existuje a smazane to neni
		if(removeFile=="." || removeFile=="..") {	// je to '.' nebo '..'
			throw new CDataArchiveException(fileName, CDataArchiveException::badPath, CDataArchiveException::archiveItself, this);
		}
		if(!pItem->IsDirectory()) {	// Je to dokonce soubor
			Dir.RemoveFile(removeFile);		
		} else { // Neni to soubor
			throw new CDataArchiveException(fileName, CDataArchiveException::fileNotFound, CDataArchiveException::archiveItself, this);
		}
	}
}

int CDataArchiveFile::MakeDirNoExceptions(LPCTSTR lpcszDirectoryName)
{
	CString dirName=lpcszDirectoryName;
	CString newDir;
	CString path;

	// Rozdelime si cestu na vlastni cestu a adresar
	SplitPath(dirName, path, newDir);

	// Jmeno nesmi byt prazdne
	if(dirName.IsEmpty()) {
	    // Prazdne jmeno adresare
        return 2; 
    }

	// Cesta musi vest k adresari...
	ASSERT(m_pCache);
	CDataArchiveFileDirectory Dir=m_pCache->GetDirectory(path);

	// .. a zjevne vede, jinak bychom uz tady nebyli...
	
	// Vytvareny adresar NESMI existovat
	CDataArchiveFileDirectoryItem *pItem;
	pItem=Dir.FileInfo(newDir);
	if(pItem==NULL) { // Adresar neexistuje vubec - to je dobre
		Dir.CreateDirectory(newDir);
	} else if(pItem->IsDeleted()) { // Neco tam existuje, ale je to smazane...
		Dir.CreateDirectory(newDir);
	} else { // Neco tam existuje a smazane to neni
		return 1; // EXISTUJE
	}
    // Bez chyby
    return 0;
}

void CDataArchiveFile::MakeDirectory(LPCTSTR lpcszDirectoryName) {
    switch(MakeDirNoExceptions(lpcszDirectoryName)) {
        case 0: // Vsechno v poradku
            // Nic nedelame
            break;
		case 1: // Adresar existuje
			throw new CDataArchiveException(lpcszDirectoryName, CDataArchiveException::alreadyExists, CDataArchiveException::archiveItself, this);
			break;
        case 2: // Prazdne jmeno adresare
            TRACE("Prazdne jmeno vytvareneho adresare\n");
		    throw new CDataArchiveException(m_pMappedFile->Path(), CDataArchiveException::badPath, CDataArchiveException::archiveItself, this);
            break;
		default: // Neco je spatne
			throw new CDataArchiveException(lpcszDirectoryName, CDataArchiveException::generic, CDataArchiveException::archiveItself, this);
			break;
	}
}

void CDataArchiveFile::RemoveDirectory(LPCTSTR lpcszDirectoryName) {
	CString dirName=lpcszDirectoryName;
	CString removeDir;
	CString path;

	// Rozdelime si cestu na vlastni cestu a adresar
	SplitPath(dirName, path, removeDir);

	// Jmeno nesmi byt prazdne
	if(dirName.IsEmpty()) {
		TRACE("Prazdne jmeno mazaneho adresare\n");
		throw new CDataArchiveException(m_pMappedFile->Path(), CDataArchiveException::badPath, CDataArchiveException::archiveItself, this);
	}

	// Cesta musi vest k adresari...
	ASSERT(m_pCache);
	CDataArchiveFileDirectory Dir=m_pCache->GetDirectory(path);

	// .. a zjevne vede, jinak bychom uz tady nebyli...
	
	// Mazany adresar MUSI existovat
	CDataArchiveFileDirectoryItem *pItem;
	pItem=Dir.FileInfo(removeDir);
	if(pItem==NULL) { // Adresar neexistuje vubec
		throw new CDataArchiveException(dirName, CDataArchiveException::rmDirFailed, CDataArchiveException::archiveItself, this);
	} else if(pItem->IsDeleted()) { // Neco tam existuje, ale je to smazane...
		throw new CDataArchiveException(dirName, CDataArchiveException::rmDirFailed, CDataArchiveException::archiveItself, this);
	} else { // Neco tam existuje a smazane to neni
		if(removeDir=="." || removeDir=="..") {	// je to '.' nebo '..'
			throw new CDataArchiveException(dirName, CDataArchiveException::rmDirFailed, CDataArchiveException::archiveItself, this);
		}
		if(pItem->IsDirectory()) {	// Je to dokonce adresar
			// Test na prazdnost
			CDataArchiveFileDirectory DirRm=m_pCache->GetDirectory(dirName);
			if(DirRm.IsEmpty()) {			
				Dir.RemoveDirectory(removeDir);		
			} else {
				throw new CDataArchiveException(dirName, CDataArchiveException::dirNotEmpty, CDataArchiveException::archiveItself, this);
			}
		} else { // Neni to adresar
			throw new CDataArchiveException(dirName, CDataArchiveException::notADirectory, CDataArchiveException::archiveItself, this);
		}
	}
}

void CDataArchiveFile::CleanupDataMembers()
{
    if(m_pCache) { 
		delete m_pCache;
		m_pCache=NULL;
	}
	if(m_pRootDirectory) {
		delete m_pRootDirectory;
		m_pRootDirectory=NULL;
	}
	if(m_pCommon) {
		m_pCommon->Release(this);
		m_pCommon=NULL;
	}
	if(m_pMappedFile) {
		delete m_pMappedFile;
		m_pMappedFile=NULL;
	}
}

CDataArchiveDirContents *CDataArchiveFile::GetDirContents(CString strPath) {
	strPath.Replace('/', '\\');
	strPath.TrimRight(); strPath.TrimRight('\\');
    strPath.TrimLeft(); strPath.TrimLeft('\\');

	ASSERT(m_pCache);
	CDataArchiveFileDirectory Dir;

	try {		
		Dir=m_pCache->GetDirectory(strPath);
	} catch(CDataArchiveException *ex) {
		ex->Delete();
		return NULL;
	}
		
	return Dir.GetDirContents();
}

void CDataArchiveFile::FlushData(CDataArchive Archive, CString strDir) {
	ASSERT(m_pCache);

	CDataArchiveFileDirectory dir;
	dir=m_pCache->GetDirectory(strDir);

	CDataArchiveDirContents *contents;
	contents=NULL;
	try {
		contents=dir.GetDirContents();
		ASSERT(contents);

		BOOL bWorking=contents->MoveFirst();
		while(bWorking) {
			CDataArchiveFileDirectoryItem *info=contents->GetInfo();
			CString strItemName;
			if(strDir.IsEmpty()) {
				strItemName=info->GetName();
			} else {
				strItemName=strDir+'\\'+info->GetName();
			}
			if(info->IsDirectory()) {
				Archive.MakeDirectory(strItemName);
				FlushData(Archive, strItemName);
			} else {
				// Zkusime prekopirovat
				CArchiveFile fileFrom;
				fileFrom=CreateFile(strItemName, CArchiveFile::modeRead | CArchiveFile::modeRaw | CFile::shareDenyWrite);
				Archive.AppendFile(fileFrom, strItemName);
			}
			bWorking=contents->MoveNext();
		}
	} catch(...) {
		delete contents;
		throw;
	}

	delete contents;
}

void CDataArchiveFile::SplitPath(CString &strPath, CString &strDir, CString &strFile)
{
	strPath.Replace('/', '\\');
	strPath.TrimRight(); strPath.TrimRight('\\');
	strPath.TrimLeft(); strPath.TrimLeft('\\');

	// Oddelime jmeno od cesty
	int pos=strPath.ReverseFind('\\');
	if(pos>=0) { // Lomitko tam je
		strDir=strPath.Left(pos);
		strFile=strPath.Mid(pos+1);
	} else { // Lomitko tam neni, to co je zadane je cele jmeno adresare
		strDir="";
		strFile=strPath;
	}
}

CDataArchiveFileDirectoryItem *CDataArchiveFile::FileInfo(CString strName)
{
	CString path;
	CString fileName;

	SplitPath(strName, path, fileName);	
	CDataArchiveFileDirectory dir;
	dir=m_pCache->GetDirectory(path);
	return dir.FileInfo(fileName);
}

CDataArchiveFileDirectory CDataArchiveFile::DirInfo(CString strName)
{
	CString path;
	CString fileName;

	SplitPath(strName, path, fileName);	
	CDataArchiveFileDirectory dir;
	dir=m_pCache->GetDirectory(path);
	return dir;
}

CDataArchiveInfo *CDataArchiveFile::CreateArchiveInfoClone()
{
	CDataArchiveFile *pNew = new CDataArchiveFile();
	
	// Nyni musime pNew vyplnit
	
	// Nejdrive mu zrusime jeho common
	pNew->m_pCommon->Release(pNew);
	pNew->m_pCommon=NULL;

	// Nastavime na nas
	pNew->m_pCommon=m_pCommon;
	pNew->m_pCommon->AddRef(pNew);
	
	// Volame si vlastni create
	pNew->CDataArchiveInfo::Create(m_strArchiveName, m_dwFlags, CDataArchiveInfo::archiveFile);

	// Jeste nas nikdo necreatnul
	ASSERT(!pNew->m_pMappedFile);

	// Nastavime ID threadu pro novy archiv
	pNew->m_dwThreadID=GetCurrentThreadId();

	// Zkusime vsechno udelat, kdyz ne - velka dealokace	
	try {
		// Vytvorime soubor podle uz fungujiciho
		pNew->m_pMappedFile=new CMappedFile(m_pMappedFile);

		// Je treba vytvorit vlastni instanci root directory a vlastni cache
		pNew->m_pRootDirectory=new CDataArchiveFileDirectory();
		pNew->m_pRootDirectory->Open(*pNew->m_pMappedFile, m_pCommon->m_pMainHeader->RootOffset());

		// Vytvorime cache na adresare
		pNew->m_pCache=new CDataArchiveFileDirectoryCache(pNew->m_pRootDirectory, pNew->m_pMappedFile);

		// A vratime vytvoreny archiv
		return pNew;

	} catch(...) { // Neco se nepovedlo			
		// Delame velkou dealokaci toho, co jsme provedli
		if(pNew) {
			pNew->CleanupDataMembers();
			delete pNew;
			pNew=NULL;
		}
		
		// A hazime vyjimku vys
		throw;
	}

	return NULL;
}

void CDataArchiveFile::AdoptFile(CArchiveFile &file)
{	
	CArchiveFileInfo *pInfo=file.m_pFileInfo;

	CDataArchiveFile *pDA=(CDataArchiveFile*)pInfo->m_pArchiveInfo;
	
	// Soubor musi byt ze stejne rodiny archivu
	// ASSERT(pDA->GetMappedFile()==GetMappedFile());

	// Prehakujeme si info
	pInfo->ChangeDataArchive(this, pInfo->m_strFileName);
}	