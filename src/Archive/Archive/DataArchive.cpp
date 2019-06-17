// DataArchive.cpp: implementation of the CDataArchive class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DataArchive.h"

// Potrebne includy pro Create(LPCTSTR lpcszFileName);
#include "DataArchiveInfo.h"
#include "DataArchiveDirectory.h"
#include "DataArchiveFile.h"
#include <direct.h>
#include <errno.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDataArchive::CDataArchive()
{
	// Uplne na zacatku vynullujeme ukazatel na 'letter'
	m_pArchiveInfo=NULL;

	// Pripocteme se k poctu CDataArchive objektu
	m_dwNumDataArchives++;

  // Smazem archive info
  m_pArchiveInfo = NULL;  
}

// Vytvori archiv primo v zadanem souboru
CDataArchive::CDataArchive(LPCTSTR lpcszFileName) {
	// Uplne na zacatku se vynulluje ukazatel na 'letter'
	m_pArchiveInfo=NULL;

	// Pripocteme se k poctu CDataArchive objektu
	m_dwNumDataArchives++;

/***** vvv ZMENIT !!! vvv */

	// Nechame ho stupidne vytvorit prazdny
	Create();

	// Pak to prepalime spravnym souborem
	Create(lpcszFileName);

/****** ^^^ ZMENIT!!! ^^^ */
}

// Vytvori archiv primo v zadanem souboru
CDataArchive::CDataArchive(LPCTSTR lpcszFileName, DWORD dwFlags, DWORD dwArchiveType) {
	// Uplne na zacatku se vynulluje ukazatel na 'letter'
	m_pArchiveInfo=NULL;

	// Pripocteme se k poctu CDataArchive objektu
	m_dwNumDataArchives++;

/***** vvv ZMENIT !!! vvv */

	// Nechame ho stupidne vytvorit prazdny
	Create();

	// Pak to prepalime spravnym souborem
	Create(lpcszFileName, dwFlags, dwArchiveType);

/****** ^^^ ZMENIT!!! ^^^ */
}

// Obalovaci konstruktor
CDataArchive::CDataArchive(CDataArchiveInfo *pDataArchiveInfo) {
	// Zatim zadne info nemame
	m_pArchiveInfo=(CDataArchiveInfo*)NULL;

	// Pripocteme se k poctu CDataArchive objektu
	m_dwNumDataArchives++;

	// Pouzijeme operator = na DataArchiveInfo
	(*this)=pDataArchiveInfo;
}

// Copy konstruktor
CDataArchive::CDataArchive(const CDataArchive& DataArchive)
{
	// Zadne informace o archivu
	m_pArchiveInfo=(CDataArchiveInfo*)NULL;

	// Pripocteme se k poctu CDataArchive objektu
	m_dwNumDataArchives++;

	// Pouzijeme operator =
	(*this)=DataArchive; 
}

// Operator =
CDataArchive& CDataArchive::operator =(const CDataArchive & DataArchive)
{
	// Prirazuji sam do sebe
	if(((CDataArchive*)&DataArchive)==this) return *this;

	(*this)=DataArchive.m_pArchiveInfo; // Priradime archive info

	return *this;
}

// Operator = pro nastaveni m_pArchiveInfo
CDataArchive& CDataArchive::operator =(CDataArchiveInfo *pDataArchiveInfo)
{
	// Prirazuji sam do sebe
	if(pDataArchiveInfo==m_pArchiveInfo) return *this;

	// Byli jsme pripojeni k objektu CDataArchiveInfo.
	if(m_pArchiveInfo) {
		m_pArchiveInfo->DetachFromDataArchive(this); // Odpojime se
		m_pArchiveInfo=(CDataArchiveInfo*)NULL; // A vynulujeme odkaz
	}

	ASSERT(!m_pArchiveInfo);
	
	// Zkopiruji se informace o souboru
	m_pArchiveInfo=pDataArchiveInfo;

	// ArchiveInfo musi byt vyplnene, aby se s nim neco delalo
	// Jestli neni - patologicky pripad, kopiruji se dva nevyplnene archivy 
	if(m_pArchiveInfo) {
		m_pArchiveInfo->AttachToDataArchive(this);
	}
	return *this;
}


CDataArchive::~CDataArchive()
{
	// Neco tam je, je treba pustit delete
	if(m_pArchiveInfo!=NULL) 
		Delete();

	// Odecteme se od poctu CDataArchive objektu
	DecreaseNumberOfArchives();

	// Ted je vse v poradku
	ASSERT(m_pArchiveInfo==NULL);
}

#ifdef _DEBUG

void CDataArchive::AssertValid(void) const
{
	// Assert pro otce
	CObject::AssertValid();

	// Vlastni asserty
	ASSERT(m_pEmptyArchiveInfo);
}

void CDataArchive::Dump(CDumpContext& dc) const
{
	// Dumpnout otce
	CObject::Dump(dc);	

	// Pokud je neco vevnitr
	if(m_pArchiveInfo) { 
		dc << "DataArchive - obalka pro objekt:\n";
		m_pArchiveInfo->Dump(dc);	
	} else {
		dc << "DataArchive - prazdna obalka\n";
	}
}

#endif

BOOL CDataArchive::Create()
{
  if(m_pArchiveInfo != NULL)
    m_pArchiveInfo->DetachFromDataArchive(this);
  // Vytvorime prazdne ArchiveInfo

	// Jeste neni m_pEmptyArchiveInfo inicializovano (nebylo volano CDataArchive::Initialize
	ASSERT(m_pEmptyArchiveInfo!=(CDataArchiveInfo*)NULL);

	m_pArchiveInfo=m_pEmptyArchiveInfo;
	m_pArchiveInfo->AttachToDataArchive(this);
	return SUCCESS;
}

BOOL CDataArchive::Create(LPCTSTR lpcszFileName, DWORD dwFlags, DWORD dwArchiveType)
{
	CString strFileName=lpcszFileName;
	CString strArchiveName;
	// Upravime jmeno, aby nekoncilo backslashem ani whitespacama apod.

	strFileName.TrimRight();
	strFileName.TrimRight('\\');

    if(strFileName.IsEmpty()) {
        throw new CDataArchiveException("<empty filename>", CDataArchiveException::badPath, CDataArchiveException::archiveItself);
    } 

	if(dwFlags & CArchiveFile::modeCreate) {  // Vytvareni noveho
		
		// Bude se vytvaret novy archiv
		strArchiveName=strFileName;

	} else { // Bude se pouzivat jiz existujici
	
		if(strFileName=="\xff\xff\xff") { // Jmeno je prazdne, tvorime globalni archiv
			strArchiveName="";
			dwArchiveType=CDataArchiveInfo::archiveDirectory;
		} else { // Jmeno neni prazdne
			// Budeme hledat zadany soubor
			CFileFind finder;
			BOOL bWorking=finder.FindFile(strFileName);

            DWORD dwError = GetLastError();
			
			if(!bWorking) { // Nezafachcilo to
				TRACE("Nelze otevrit archiv '%s'.\n", strFileName);
				throw new CDataArchiveException(strFileName, CDataArchiveException::fileNotFound, CDataArchiveException::archiveItself);
				return FAILURE; 
			} else { // Cesta funguje

				finder.FindNextFile(); // Tohle je treba zavolat alespon jednou, aby sly cist udaje

				// Archiv bude to, co rekne finder o ceste
				strArchiveName=finder.GetFilePath();

				dwArchiveType=finder.IsDirectory()?CDataArchiveInfo::archiveDirectory:CDataArchiveInfo::archiveFile;
			} // Cesta funguje
		}	
	} // Bude se pouzivat jiz existujici

  // Odpojime stare ArchiveInfo
  if(m_pArchiveInfo != NULL)
	  m_pArchiveInfo->DetachFromDataArchive(this);
	
	switch(dwArchiveType) {
	case CDataArchiveInfo::archiveDirectory: // Jedna se o adresar
		// Vytvorime CDataArchiveDirectory s danym jmenem
		m_pArchiveInfo=new CDataArchiveDirectory();
		break;
	case CDataArchiveInfo::archiveFile: // Je to soubor
		m_pArchiveInfo=new CDataArchiveFile();
		break;
	default:
		TRACE("Neznamy typ archivu");
		ASSERT(FALSE);
	}

	// Pripojime nove ArchiveInfo
	m_pArchiveInfo->AttachToDataArchive(this);

	// A nechame je vytvorit se na danem souboru
	try {
		return m_pArchiveInfo->Create(strArchiveName, dwFlags, dwArchiveType);
	} catch(...) {
		Delete();
		Create();
		throw;
	}

	return SUCCESS;
}

BOOL CDataArchive::Delete()
{	
    ASSERT(m_pArchiveInfo!=NULL);
	m_pArchiveInfo->DetachFromDataArchive(this);
	m_pArchiveInfo=NULL;

	ASSERT(m_pArchiveInfo==NULL);

	return SUCCESS;
}

void CDataArchive::Initialize(LPCTSTR lpcszTemporaryDirectory)
{	
	if(m_bInitializeWasCalled) return;
	m_bInitializeWasCalled=TRUE;
	if(m_pEmptyArchiveInfo==(CDataArchiveInfo*)NULL) {
		// Zvysime si pocet archivu o jednicku, to se da odcinit jen pomoci Done
		m_dwNumDataArchives++;

		// Schvalne predefinujeme z const - je treba inicializovat
		(CDataArchiveInfo*)m_pEmptyArchiveInfo=new CDataArchiveInfo();
		// Pripojime nove info k virtualnimu prazdnemu archivu
		m_pEmptyArchiveInfo->AttachToDataArchive((CDataArchive*)NULL);

		// Upravime si jmeno adresare
		CString strFileName=lpcszTemporaryDirectory;
		strFileName.TrimRight();
		strFileName.Replace('/', '\\');
		strFileName.TrimRight('\\');
		if(strFileName=="")  strFileName=".";

		// Podivame se, jestli temporary adresar existuje
		CFileFind finder;
		if(!finder.FindFile(strFileName)) { // Nezafungovalo to
			// Musime ho vytvorit
			m_bTempCreated=FALSE;
			if(m_bTempCreated) { // Ale promenna tvrdi, ze temp jiz byl vytvoren.. sem se to nema dostat
				TRACE("Nelze otevrit temporary adresar '%s'. Nevolejte proboha inicializaci dvakrat!\n", strFileName);
				throw new CDataArchiveException(strFileName, CDataArchiveException::makeDirFailed, CDataArchiveException::temporaryDirectory);
			} else { // Temp jeste nebyl vytvoren
				// Vytvorime ho
				if(_mkdir(strFileName)==0) {
					// Povedlo se vytvorit
					if(!finder.FindFile(strFileName)) { // Vsechno je uplne spatne
						TRACE("Nelze otevrit temporary adresar '%s' ktery jsem prave vytvoril!\n", strFileName);
						throw new CDataArchiveException(strFileName, CDataArchiveException::makeDirFailed, CDataArchiveException::temporaryDirectory);
					}
					m_bTempCreated=TRUE;
				} else {
					// Nejde vytvorit adresar
					TRACE("Nelze otevrit temporary adresar '%s'.\n", strFileName);
					m_bTempCreated=FALSE;
					throw new CDataArchiveException(strFileName, CDataArchiveException::makeDirFailed, CDataArchiveException::temporaryDirectory);
				}
			}
		}
		finder.FindNextFile();

		if(finder.IsDirectory()) {
			m_strArchiveTemp=finder.GetFilePath();
			m_strArchiveTemp.TrimRight();
			m_strArchiveTemp.Replace('/', '\\');			
			m_strArchiveTemp.TrimRight('\\');
		} else {
			// Nepovedlo se, neni to adresar
			throw new CDataArchiveException(strFileName, CDataArchiveException::notADirectory, CDataArchiveException::temporaryDirectory);
		}
	}
	
	if(!CDataArchive::m_pTemporaryArchive) {
		CDataArchive::m_pTemporaryArchive=new CDataArchive(m_strArchiveTemp);
	}
	if(!CDataArchive::m_pGlobalArchive) {
		CDataArchive::m_pGlobalArchive=new CDataArchive("\xff\xff\xff");
	}
}

void CDataArchive::Done()
{
	if(!m_bInitializeWasCalled || m_bDoneWasCalled) return;
	m_bDoneWasCalled=TRUE;
	// Snizime pocet data archivu o jedna
	DecreaseNumberOfArchives();
}

void CDataArchive::RemoveDirectoryWithContents(CString strDirName) {
	CFileFind finder;

	strDirName.TrimRight();
	strDirName.TrimRight('\\');
	strDirName.TrimRight('/');
	if(strDirName=="")  strDirName=".";
	
	if(!finder.FindFile(strDirName)) { // Soubor neni
		throw new CDataArchiveException(strDirName, CDataArchiveException::fileNotFound, CDataArchiveException::unknownLocation);		
	} else { // Soubor je pritomen
		finder.FindNextFile();
		if(finder.IsDirectory()) { // Je to adresar
			finder.Close(); // Uz nepotrebujeme stare info
			// Vyprazdni obsah adresare			
			BOOL bWorking=finder.FindFile(strDirName+"\\*.*");
			while(bWorking) {
				bWorking=finder.FindNextFile();
				if(finder.IsDirectory()) { // Mame podadresar ...
					if(finder.GetFileName()!="." && finder.GetFileName()!="..") { // ... a nejsme to my ani rodic
						TRACE("Adresar %s\n", finder.GetFilePath());
						RemoveDirectoryWithContents(finder.GetFilePath());
					}
				} else { // Mame soubor
					try { // Zkusime ho smazat
						CFile::Remove(finder.GetFilePath());
					} catch(CFileException *pError) { throw new CDataArchiveException(pError, CDataArchiveException::unknownLocation); }
				}
			}
			finder.Close();
			// Ted je adresar prazdny, muzeme ho zkusit smazat
			int nErrCode=_rmdir(strDirName);
			if(nErrCode!=0) { // Nastala chyba pri mazani, hazeme vyjimku
				switch(errno) {
				case ENOENT:
					throw new CDataArchiveException(strDirName, CDataArchiveException::rmDirFailed, CDataArchiveException::unknownLocation);
					break;
				case ENOTEMPTY:
					throw new CDataArchiveException(strDirName, CDataArchiveException::dirNotEmpty, CDataArchiveException::unknownLocation);
					break;
				default:
					throw new CDataArchiveException(strDirName, CDataArchiveException::generic, CDataArchiveException::unknownLocation);
					break;
				}
			}
		} else { // Neni to adresar
			throw new CDataArchiveException(strDirName, CDataArchiveException::notADirectory, CDataArchiveException::unknownLocation);
		}		
	}
}

CDataArchive *CDataArchive::GetTempArchive() {
	ASSERT(m_pTemporaryArchive);
	return m_pTemporaryArchive;
}

CDataArchive *CDataArchive::GetRootArchive() {
	ASSERT(m_pGlobalArchive);
	return m_pGlobalArchive;
}

CString CDataArchive::GetNewTemporaryName() {
	CString strName;
	CTime time;
	BOOL bExists;
	bExists=TRUE;
	time = CTime::GetCurrentTime();
	while(bExists) {
		DWORD dwSessionNr=InterlockedIncrement((LPLONG)&m_dwSessionNr);
		strName.Format("tmp_%d_%d_%d_%d_%d_%d_%d_%d", 
			time.GetDay(), time.GetMonth(), time.GetYear(),
			time.GetHour(), time.GetMinute(), time.GetSecond(),
			dwSessionNr, rand());
		CFileFind finder;
		if(!finder.FindFile(m_strArchiveTemp+'\\'+strName)) 
			bExists=FALSE; 
		else
			bExists=finder.FindNextFile();
	}
	return strName;
}

// Snizi pocet zijicich archivu, kdyz uz je mrtvo, dela praci za done
void CDataArchive::DecreaseNumberOfArchives() {
    CDataArchive *pArchive;
    ASSERT(m_dwNumDataArchives>0);
	m_dwNumDataArchives--;

	// Nejvys globalni a temporary adresar existuji - a nic vic
	if((m_dwNumDataArchives==2 && CDataArchive::m_pTemporaryArchive!=NULL && CDataArchive::m_pGlobalArchive!=NULL) || 
        (m_dwNumDataArchives==1 && CDataArchive::m_pTemporaryArchive!=NULL) || 
        (m_dwNumDataArchives==1 && CDataArchive::m_pGlobalArchive!=NULL) || 
        (m_dwNumDataArchives==0)) {
		if(CDataArchive::m_pTemporaryArchive) {
			// Jestlize byl temp archive vytvoren
			if(CDataArchive::m_bTempCreated) {
				// Je treba po sobe temp uklidit
				try {
					CDataArchive::RemoveDirectoryWithContents(m_pTemporaryArchive->GetArchivePath());
					m_bTempCreated=FALSE;
				} catch(CException *e) {
					// Exceptiony ignorujeme
					e->Delete();
				}
			}
			// A smazeme vlastni objekt archivu
			pArchive=CDataArchive::m_pTemporaryArchive;
			CDataArchive::m_pTemporaryArchive=(CDataArchive *)NULL;
            delete pArchive;
		}
		if(CDataArchive::m_pGlobalArchive) {
			pArchive=CDataArchive::m_pGlobalArchive;
			CDataArchive::m_pGlobalArchive=(CDataArchive *)NULL;
            delete pArchive;
		}
		ASSERT(m_dwNumDataArchives==0);
		if(m_pEmptyArchiveInfo!=(CDataArchiveInfo*)NULL) {
			// Jakoby ho odpojime od virtualniho prazdneho archivu
			m_pEmptyArchiveInfo->DetachFromDataArchive((CDataArchive*)NULL);
			// A opet prepalime const, je treba vynullovat
			(CDataArchiveInfo*)m_pEmptyArchiveInfo=NULL;
		}
	}
}

// Inicializace statickych promennych

// Jedno globalni info o prazdnem archivu
CDataArchiveInfo *CDataArchive::m_pEmptyArchiveInfo=(CDataArchiveInfo*)NULL;

// Cesta k tempu
CString CDataArchive::m_strArchiveTemp="";

// Cislo v ramci 'session'
DWORD CDataArchive::m_dwSessionNr=0;

// Byl vytvoren temporary adresar na zacatku?
BOOL CDataArchive::m_bTempCreated=FALSE;

// Temporary archiv
CDataArchive *CDataArchive::m_pTemporaryArchive=(CDataArchive *)NULL;

// Globalni archiv - vsechny disky vsude
CDataArchive *CDataArchive::m_pGlobalArchive=(CDataArchive *)NULL;

// Pocet zijicich CDataArchivu
DWORD CDataArchive::m_dwNumDataArchives=0;

// Bylo zavolano initialize?
BOOL CDataArchive::m_bInitializeWasCalled=FALSE;

// Bylo zavolano done?
BOOL CDataArchive::m_bDoneWasCalled=FALSE;