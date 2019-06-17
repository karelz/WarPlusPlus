// DataArchiveException.h: interface for the CDataArchiveException class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DATAARCHIVEEXCEPTION_H__74ECB8D6_2D50_11D3_8909_00C04F514356__INCLUDED_)
#define AFX_DATAARCHIVEEXCEPTION_H__74ECB8D6_2D50_11D3_8909_00C04F514356__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afx.h>

class CDataArchiveInfo;

class CDataArchiveException : public CException
{
public:
	enum EDataArchiveExceptionTypes;
	enum EDataArchiveExceptionLocations;

	// Konstruktor
	CDataArchiveException();

	// Vytvori vyjimku ze zadane CFileException, kterou smaze
	CDataArchiveException(CFileException *pFileException, DWORD eWhereHappened=EDataArchiveExceptionLocations::fileInArchive, CDataArchiveInfo *pArchiveInfo=NULL);

	// Vytvori vyjimku ze zadane CFileException, kterou smaze
	CDataArchiveException(CFileException *pFileException, CDataArchiveInfo *pArchiveInfo, DWORD eWhereHappened=EDataArchiveExceptionLocations::fileInArchive);

	// Vytvori vyjimku ze zadaneho kodu a jmena souboru
	CDataArchiveException(CString strFileName, DWORD eErrorCode, DWORD eWhereHappened=EDataArchiveExceptionLocations::fileInArchive, CDataArchiveInfo *pArchiveInfo=NULL);

	// Vytvori vyjimku ze zadane vyjimky, zmeni jmeno souboru a umisteni
	CDataArchiveException(CDataArchiveException *pArchiveException, CString strFileName, DWORD eWhereHappened=EDataArchiveExceptionLocations::fileInArchive, CDataArchiveInfo *pArchiveInfo=NULL);

	// Destruktor
	virtual ~CDataArchiveException();

	// Vrati chybovou hlasku, tato funkce je tu kvuli kompatibilite
	virtual BOOL GetErrorMessage(LPTSTR lpszError, UINT nMaxError, PUINT pnHelpContext=NULL);

    // Vrati chybovou hlasku nejak lidsky
	virtual CString GetErrorMessage();

	// Jmeno provinileho souboru, na kterem doslo k vyjimce
	virtual CString GetFileName() { return m_strDataArchiveExceptionFileName; }

	// Provinileho CDataArchiveInfo, na kterem doslo k vyjimce
	virtual CString GetArchiveName() { return m_strDataArchiveExceptionArchiveName; }

	// Misto vzniku chyby
	virtual EDataArchiveExceptionLocations GetLocation() { return m_eDataArchiveExceptionLocation; }

	// Pricina chyby - cislo
	virtual EDataArchiveExceptionTypes GetErrorCode() { return m_eDataArchiveExceptionCause; }

public:
	// Pricina chyby - cislo
	EDataArchiveExceptionTypes m_eDataArchiveExceptionCause;

    // Kde doslo k chybe - cislo
	EDataArchiveExceptionLocations m_eDataArchiveExceptionLocation;

	// Jmeno souboru, na kterem k chybe doslo
	CString m_strDataArchiveExceptionFileName;

	// Archive, na kterem doslo k chybe
	CString m_strDataArchiveExceptionArchiveName;

public:
	// Vlastni typy vyjimek
	enum EDataArchiveExceptionTypes {
		
		// Stejne jako u CFileException
		none				=CFileException::none,
		generic				=CFileException::generic,
		fileNotFound		=CFileException::fileNotFound,
		badPath				=CFileException::badPath,
		tooManyOpenFiles	=CFileException::tooManyOpenFiles,
		accessDenied		=CFileException::accessDenied,
		invalidFile			=CFileException::invalidFile,
		removeCurrentDir	=CFileException::removeCurrentDir,
		directoryFull		=CFileException::directoryFull,
		badSeek				=CFileException::badSeek,
		hardIO				=CFileException::hardIO,
		sharingViolation	=CFileException::sharingViolation,
		lockViolation		=CFileException::lockViolation,
		diskFull			=CFileException::diskFull,
		endOfFile			=CFileException::endOfFile,

		// Plus nejake vlastni
		alreadyExists,	// Objekt jiz existuje
		notADirectory,	// Neni to adresar
		makeDirFailed,	// Nelze vytvorit adresar
		rmDirFailed,	// Nelze zrusit adresar
		dirNotEmpty,	// Adresar je neprazdny
		fileNotAssigned, // Soubor jeste neni nicemu prirazen
		unknownArchiveFormat, // Neznamy format archivu
		archiveCorrupt, // Porusena struktura archivu
		decompressionFailed, // Dekomprese selhala
		compressionFailed // Komprese selhala
	};

	enum EDataArchiveExceptionLocations {
		nowhere,			// Nic se nedeje, proc mne budite?
		fileInArchive,		// Je to v souboru v archivu
		archiveItself,		// Je to v archivnim souboru samotnem
		temporaryDirectory,	// Je to v pomocnem adresari
		unknownLocation		// Neznama pozice
	};

};

#endif // !defined(AFX_DATAARCHIVEEXCEPTION_H__74ECB8D6_2D50_11D3_8909_00C04F514356__INCLUDED_)
