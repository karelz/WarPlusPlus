// DataArchiveInfo.cpp: implementation of the CDataArchiveInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DataArchiveInfo.h"
#include "DataArchiveDirContents.h"
#include "DataArchive.h"

#define SUCCESS TRUE
#define FAILURE FALSE

#pragma warning ( disable : 4100 )

BOOL CDataArchiveInfo::Create()
{
	// Archiv zatim neni nastaven
	m_eArchiveType=archiveNone;
	m_bOpened=FALSE;
	m_strArchiveName="";
	m_nCreatedFiles=0;
	m_dwFlags=0;

	return SUCCESS;
}

BOOL CDataArchiveInfo::Create(CString strFileName, DWORD dwFlags, DWORD dwArchiveType)
{
	// Priradime jmeno
	m_strArchiveName=strFileName;
	m_dwFlags=dwFlags;

	return SUCCESS;
}

void CDataArchiveInfo::Open()
{
	TRACE("Neni mozne otevirat prazdny archiv!");
	ASSERT(FALSE);
}

void CDataArchiveInfo::Close()
{
	TRACE("Neni mozne uzavirat prazdny archiv!");
	ASSERT(FALSE);
}	

void CDataArchiveInfo::Rebuild()
{
	TRACE("Neni mozne rebuildnout prazdny archiv!");
	ASSERT(FALSE);
}

BOOL CDataArchiveInfo::Delete()
{
	return SUCCESS;
}


#pragma warning ( disable : 4100 ) // Nevarovat na nepouzite promenne
CArchiveFile CDataArchiveInfo::CreateFile(LPCTSTR lpcszFileName, DWORD dwFlags)
{	
	CArchiveFile f;

	TRACE("Prazdny archiv nemuze vracet soubory!\n");
	ASSERT(FALSE);

	return f;
}

CDataArchive CDataArchiveInfo::CreateArchive(LPCTSTR lpcszPath, BOOL bCreate)
{
	CDataArchive a;

	TRACE("Prazdny archiv nemuze vracet podarchivy!\n");
	ASSERT(FALSE);

	return a;
}

void CDataArchiveInfo::AppendFile(LPCTSTR lpcszFileName, LPCTSTR lpcszFilePath, DWORD dwFlags)
{
	TRACE("Neni mozne pridavat soubory do prazdneho archivu!");
	ASSERT(FALSE);
}

void CDataArchiveInfo::AppendFile(CArchiveFile file, LPCTSTR lpcszFilePath, DWORD dwFlags)
{
	TRACE("Neni mozne pridavat soubory do prazdneho archivu!");
	ASSERT(FALSE);
}

void CDataArchiveInfo::AppendDir(LPCTSTR lpcszDirPathTo, CDataArchive ArchiveFrom, LPCTSTR lpcszDirPathFrom, EAppendModes nFlags /* = appendRecursive */)
{
	TRACE("Neni mozne pridavat adresare do prazdneho archivu!");
	ASSERT(FALSE);
}

void CDataArchiveInfo::AppendDirNoRef(LPCTSTR lpcszDirPathTo, CDataArchiveInfo *pArchiveFrom, LPCTSTR lpcszDirPathFrom, EAppendModes nFlags /* = appendRecursive */)
{
	TRACE("Neni mozne pridavat adresare do prazdneho archivu!");
	ASSERT(FALSE);
}

void CDataArchiveInfo::ExtractFile(LPCTSTR lpcszFileName, LPCTSTR lpcszFilePath)
{
	TRACE("Neni mozne extrahovat soubory z prazdneho archivu!");
	ASSERT(FALSE);
}

void CDataArchiveInfo::RemoveFile(LPCTSTR lpcszFileName)
{
	TRACE("Neni mozne odstranovat soubory z prazdneho archivu!");
	ASSERT(FALSE);
}

void CDataArchiveInfo::MakeDirectory(LPCTSTR lpcszDirectoryName)
{
	TRACE("Neni mozne vytvorit adresar v prazdnem archivu!");
	ASSERT(FALSE);
}

int CDataArchiveInfo::MakeDirNoExceptions(LPCTSTR lpcszDirectoryName) 
{
	TRACE("Neni mozne vytvorit adresar v prazdnem archivu!");
	ASSERT(FALSE);
    return 0;
}

void CDataArchiveInfo::RemoveDirectory(LPCTSTR lpcszDirectoryName)
{
	TRACE("Neni mozne zrusit adresar v prazdnem archivu!");
	ASSERT(FALSE);
}

CDataArchiveDirContents *CDataArchiveInfo::GetDirContents(CString strPath)
{
	TRACE("Neni mozne ziskat obsah adresare prazdneho archivu!");
	ASSERT(FALSE);
	return NULL;
}

#pragma warning ( default : 4100 )

//////////////////////////////////////////////////////////////////////
// Debugovaci veci
//////////////////////////////////////////////////////////////////////

#ifdef _DEBUG

void CDataArchiveInfo::AssertValid(void) const
{
	// Assert pro otce
	CObject::AssertValid();

	// Vlastni asserty
}

void CDataArchiveInfo::Dump(CDumpContext& dc) const
{
	// Dumpnout otce
	CObject::Dump(dc);

	dc << "-> ";
	dc << (m_bOpened?"Otevreny":"Neotevreny");
	dc << " archiv '" << m_strArchiveName << "' ";
	switch(m_eArchiveType) {
	case archiveNone:
		dc << "(prazdny)";
		break;
	case archiveDirectory:
		dc << "(adresar)";
		break;
	case archiveFile:
		dc << "(soubor)";
		break;
	default:
		dc << "(neznamy typ)";
	}
	dc << "\n";
	dc << "m_nPointerCounter=" << m_nPointerCounter << "\n";
	dc << "m_nCreatedFiles=" << m_nCreatedFiles << "\n";
}

#endif // DEBUG

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDataArchiveInfo::CDataArchiveInfo()
{
	// Na zacatku na mne nikdo neodkazuje
	m_nPointerCounter=0;

	m_bDestructorRunning=FALSE;

	Create();
}

CDataArchiveInfo::~CDataArchiveInfo()
{
	Delete();
	
	// Niceny objekt nesmi mit zadne ostatni odkazujici na nej	
	ASSERT(m_nPointerCounter==0);

	// Nesmi byt otevrene zadne soubory z tohoto archivu
	ASSERT(m_nCreatedFiles==0);
}

#pragma warning ( disable : 4100 )
void CDataArchiveInfo::AttachToDataArchive(CDataArchive *DataArchive)
{
	// Zvysime pocet odkazu
	InterlockedIncrement((LONG*)&m_nPointerCounter);
}

void CDataArchiveInfo::DetachFromDataArchive(CDataArchive *DataArchive)
{
	// Pokud se odpojujeme, museli jsme jiz byt pripojeni
	ASSERT(m_nPointerCounter>0);
	
	// Snizim pocet odkazu
	long nPC=InterlockedDecrement((LONG*)&m_nPointerCounter);

	// Koncim, pokud jiz na mne nikdo neodkazuje
	if(nPC==0 && m_nCreatedFiles==0) {
		delete this;
		return;
	}
}

void CDataArchiveInfo::FileConstructed(CArchiveFileInfo *ArchiveFileInfo)
{
	// Vytvorenych souboru musi byt vice nez nula
	ASSERT(m_nCreatedFiles>=0);

	// Zvysim pocet pripojenych souboru
	m_nCreatedFiles++;
}

void CDataArchiveInfo::FileDestructed(CArchiveFileInfo *ArchiveFileInfo)
{
	// Vytvorenych souboru musi byt vice nez jeden
	ASSERT(m_nCreatedFiles>0);

	// Snizim pocet pripojenych souboru
	m_nCreatedFiles--;

	// Koncim, pokud uz neni nikdo, kdo by na mne odkazoval
	if(m_nPointerCounter==0 && m_nCreatedFiles==0) {
		if(!m_bDestructorRunning) {
			delete this;
		}
		return;
	}
}
#pragma warning ( default : 4100 )
