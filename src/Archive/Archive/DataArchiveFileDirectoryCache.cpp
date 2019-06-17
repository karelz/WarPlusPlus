// DataArchiveFileDirectoryCache.cpp: implementation of the CDataArchiveFileDirectoryCache class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DataArchiveFileDirectoryCache.h"
#include "DataArchiveFile.h"
#include "DataArchiveFileDirectory.h"		
#include "DataArchiveException.h"
#include "MappedFile.h"

// Po kolika pribyvaji zaznamy v cachi?
#define DATA_ARCHIVE_FILE_DIRECTORY_CACHE_GRANULARITY	10

// Kolik bajtu priblizne muze cache maximalne zabirat v pameti?
// (pokud 0, zadne omezeni neni kladeno)
#define DATA_ARCHIVE_FILE_DIRECTORY_CACHE_MAX_SIZE 32768

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDataArchiveFileDirectoryCache::CDataArchiveFileDirectoryCache(CDataArchiveFileDirectory *pRootDir, CMappedFile *pMappedFile)
{
	// Zapamatujeme si, kdo nas pouziva
	m_pRootDir=pRootDir;
	m_pMappedFile=pMappedFile;

	m_pDirTable=new CMapStringToPtr(DATA_ARCHIVE_FILE_DIRECTORY_CACHE_GRANULARITY);

	// Zaciname v case nula
	m_dwActualTime=0;

#ifdef _DEBUG
	m_dwMaxCacheSize=0;
#endif // _DEBUG
}

CDataArchiveFileDirectoryCache::~CDataArchiveFileDirectoryCache()
{
	ASSERT(m_pDirTable);

	// Nejdriv flushneme obsah cache na disk
	Flush();

#ifdef _DEBUG
	// Jestlize velikost cache byla neomezena, vypiseme max. velikost, ktere dosahla
	if(DATA_ARCHIVE_FILE_DIRECTORY_CACHE_MAX_SIZE==0) {
		TRACE("CDataArchiveFileDirectoryCache: Max. cache size was: %d\n", m_dwMaxCacheSize);
	}
#endif

	// Ted je mozne ji zdeletit bez memory leaku
	delete m_pDirTable;
}

CDataArchiveFileDirectory CDataArchiveFileDirectoryCache::GetDirectory(CString strDirName) {
	ASSERT(m_pDirTable);
	
	void *pData;

	// Nejprve si orezeme konec stringu od lomitek
	strDirName.TrimRight('\\');
	strDirName.TrimRight();

	// .. a zacatek taky
	strDirName.TrimLeft();
	strDirName.TrimLeft('\\');

	// Jestlize to hodi prazdny retezec, vracime root
	if(strDirName.IsEmpty()) {
		return *m_pRootDir;
	}

	// Podivame se, jestli nahodou adresar v cachi neni
	if(!m_pDirTable->Lookup(strDirName, (void *&)pData)) pData=NULL;
	if(pData!=NULL) { // Adresar v cachi opravdu je
		// TRACE("V cachi nalezen %s\n", strDirName);
		CDataArchiveFileDirectory *pDirData;
		pDirData=(CDataArchiveFileDirectory *)pData;
		pDirData->SetLastAccessTime(m_dwActualTime++);
		return *pDirData;
	}

	// Adresar v cachi bohuzel neni

	CString dir="";
	CString newDir;
	CString restDir;
	CDataArchiveFileDirectory Dir=*m_pRootDir;
	restDir=strDirName;
	
	// Projdeme adresare od zacatku, pridavame si je do cache
	while(restDir!="") {
		int lpos=restDir.FindOneOf("\\/"); // Pozice prvniho lomitka
		if(lpos<0) { // Zadne lomitko neni
			newDir=restDir;
			restDir="";
		} else {
			newDir=restDir.Left(lpos); // To je novy adresar
			restDir=restDir.Mid(lpos+1); // .. urizne se od retezce
		}
		
		// Nyni udelame test na jmeno, ktere jsme urizli

		// Jmeno NESMI byt prazdne
		if(newDir.GetLength()==0) {
			TRACE("Spatna cesta v archivu (prazdne jmeno adresare): %s\n", (LPCTSTR)strDirName);
			throw new CDataArchiveException(m_pMappedFile->Path(), CDataArchiveException::badPath, CDataArchiveException::archiveItself);
		}

		// Pokud jsme na rootu, jmeno nesmi byt '..'
		if(newDir==".." && Dir==*m_pRootDir) {
			TRACE("Spatna cesta v archivu (nelze delat 'cd ..' v rootu): %s\n", (LPCTSTR)strDirName);
			throw new CDataArchiveException(m_pMappedFile->Path(), CDataArchiveException::badPath, CDataArchiveException::archiveItself);
		}

		// Adresar musi existovat (a musi to byt adresar a ne smazany)
		CDataArchiveFileDirectoryItem *pItem;
		pItem=Dir.FileInfo(newDir);
		if(pItem==NULL || !pItem->IsDirectory() || pItem->IsDeleted()) {
			TRACE("Spatna cesta v archivu: %s (neexistujici adresar %s)\n", (LPCTSTR)newDir, (LPCTSTR)strDirName);
			throw new CDataArchiveException(m_pMappedFile->Path(), CDataArchiveException::badPath, CDataArchiveException::archiveItself);
		}

		// Nyni je snad vse v poradku
		// Osetrime zvlast '.' a '..', abychom zbytecne nezaplacali cache
		CDataArchiveFileDirectory *pNewDir;

		if(newDir==".") {
			// Nedelame s tim nic
		} else if(newDir=="..") {
			// Z dir-u je treba uriznout posledni adresar
			int last=dir.ReverseFind('\\');
			ASSERT(last>=0);
			dir=dir.Left(last);

			if(dir.IsEmpty()) { // Bud jsme v rootu
				Dir=*m_pRootDir;
			} else { // nebo pDir nutne musi byt v cachi			
				void *pVoidDir;
				if(!m_pDirTable->Lookup(dir, (void *&)pVoidDir)) pVoidDir=NULL;
				ASSERT(pVoidDir);
				Dir=*(CDataArchiveFileDirectory *)pVoidDir;
				Dir.SetLastAccessTime(m_dwActualTime++);
			}
		} else {
			// Pridame novy adresar na konec dir-u
			if(!dir.IsEmpty()) dir+="\\";
			dir+=newDir;
			// Pridame vznikly adresar do cache (pokud tam neni)
			// Je tam?
			void *pFoundDir;
			if(!m_pDirTable->Lookup(dir, (void *&)pFoundDir)) pFoundDir=NULL;
			if(pFoundDir!=NULL) { // Je tam.
				pNewDir=(CDataArchiveFileDirectory*)pFoundDir;
			} else { // Neni tam
				pNewDir=new CDataArchiveFileDirectory();
				try {
					pNewDir->Open(*m_pMappedFile, pItem->GetOffset());
					// Na novy adresar jsme pristoupili prave ted
					pNewDir->SetLastAccessTime(m_dwActualTime++);
					// Pridame novy adresar do cache					
					m_pDirTable->SetAt(dir, pNewDir);
				} catch(...) {
					delete pNewDir;
					throw;
				}
			}
			// Povedlo se
			Dir=*pNewDir;
		}
	}

	// Podivame se, jestli cache neni moc velka
	CutCache(DATA_ARCHIVE_FILE_DIRECTORY_CACHE_MAX_SIZE, Dir);

	return Dir;
}

void CDataArchiveFileDirectoryCache::Flush() {
	void *pValue;
	CDataArchiveFileDirectory *pDir;
	CString key;

	// Projedeme tabulku, flushujeme a deletujeme vsechny zaznamy
	for(POSITION pos=m_pDirTable->GetStartPosition(); pos!=NULL;) {
		m_pDirTable->GetNextAssoc(pos, key, pValue);
		
		pDir=(CDataArchiveFileDirectory *)pValue;

		// Nejdrive flushnout adresar
		pDir->Flush();

		// Pak je mozne smazat
		delete pDir;
	}

	// Vyprazdnime tabulku
	m_pDirTable->RemoveAll();
}

void CDataArchiveFileDirectoryCache::CutCache(DWORD dwSizeLimit, CDataArchiveFileDirectory DirNoDelete) {
	void *pValue;
	CDataArchiveFileDirectory *pDir, *pOldestDir;
	CString key, oldestKey;
	DWORD dwSize;
	DWORD dwOldestTime;
	POSITION pos;

// Pokud nejsme v debugovacim rezimu a velikost cache neni omezena
// muzeme orezavani ukoncit okamzite
#ifndef _DEBUG
	// Pokud nam nekdo dal 0, zadne orezavani se neprovadi
	if(!dwSizeLimit) return;	
#endif // _DEBUG

	// Spocitame velikost cache

	dwSize=0;
	for(pos=m_pDirTable->GetStartPosition(); pos!=NULL;) {
		m_pDirTable->GetNextAssoc(pos, key, pValue);
		
		pDir=(CDataArchiveFileDirectory *)pValue;

		if((*pDir)!=DirNoDelete) { // Nepocita se ten, ktery se nesmi smazat
			dwSize+=pDir->GetStorageSize();
		}
	}

// Ulozime novou velikost
#ifdef _DEBUG
	if(dwSize>m_dwMaxCacheSize) {
		m_dwMaxCacheSize=dwSize;
	}
#endif

	while(dwSize>dwSizeLimit) { // Nasleduje kraceni metodou LRU
		ASSERT(dwSize>0);
		dwOldestTime=0;
		for(pos=m_pDirTable->GetStartPosition(); pos!=NULL;) {
			m_pDirTable->GetNextAssoc(pos, key, pValue);
			
			pDir=(CDataArchiveFileDirectory *)pValue;

			// Opet se ignoruje ten, ktery se nesmi smazat
			if((*pDir)!=DirNoDelete && pDir->GetLastAccessTime()>=dwOldestTime) {
				dwOldestTime=pDir->GetLastAccessTime();
				pOldestDir=pDir;
				oldestKey=key;
			}
		}
		
		ASSERT((*pOldestDir)!=DirNoDelete);

		// Nyni mame nejstarsi zaznam v pOldestDir, zrusime
		dwSize-=pOldestDir->GetStorageSize(); // Zmensime si velikost o zruseny adresar
		pOldestDir->Flush();
		m_pDirTable->RemoveKey(oldestKey); // Odstranime adresar z tabulky a z heapu
		delete pOldestDir;
	}
}