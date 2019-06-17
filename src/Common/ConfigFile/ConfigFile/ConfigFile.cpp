/************************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Konfigura�n� soubory
 *   Autor: Karel Zikmund
 * 
 *   Popis: Objekt CConfigFile implementuj�c� rozhran� s konfigura�n�mi 
 *          soubory
 * 
 ***********************************************************/

#include "StdAfx.h"
#include "ConfigFile.h"

#include "..\ConfigFileLex\ConfigFileLex.h"

#include "ConfigFileException.h"

// ************************************************************
// t��da reprezentuj�c� konfigura�n� soubor
// ************************************************************

//////////////////////////////////////////////////////////////////////
// Konstrukce a destrukce
//////////////////////////////////////////////////////////////////////

// konstruktor
CConfigFile::CConfigFile () 
{
}

// destruktor
CConfigFile::~CConfigFile () 
{
}

//////////////////////////////////////////////////////////////////////
// Incializace a zni�en� dat objektu
//////////////////////////////////////////////////////////////////////

// otev�e konfigura�n� soubor "lpszName" v arch�vu "cArchive"
//		v�jimky: CMemoryException, CDataArchiveException, CConfigFileException
void CConfigFile::Create ( LPCSTR lpszName, CDataArchive cArchive ) 
{
	ASSERT ( !m_fileInput.IsOpened () );

// pomocn� prom�nn�
	// jm�no konfigura�n�ho souboru
	CString strName;

	{
	// pomocn� prom�nn�
		// soubor hlavn�ho konfigura�n�ho souboru
		CArchiveFile fileMainConfigFile ( cArchive.CreateFile ( 
			_T("MainConfigFileIndex.cfg"), CFile::modeRead | CFile::shareDenyWrite ) );
		// hlavn� konfigura�n� soubor
		CConfigFile cMainConfigFile;

		// otev�e hlavn� konfigura�n� soubor
		cMainConfigFile.Create ( fileMainConfigFile, _T("") );

		// otev�e root sekci hlavn�ho konfigura�n�ho souboru
		CConfigFileSection cRootSection = cMainConfigFile.GetSection ( _T("") );

		// zjist� jm�no po�adovan�ho konfigura�n�ho souboru
		strName = cRootSection.GetString ( lpszName );
	}

	// otev�e po�adovan� konfigura�n� soubor a na�te z n�j v�echny sekce
	Create ( cArchive.CreateFile ( strName, CFile::modeRead | CFile::shareDenyWrite ), 
		_T("") );
}

// z konfigura�n�ho souboru "fileInput", jeho� hlavn� sekce se jmenuje "lpszRootName", 
//		na�te stukturu sekc� (lexik�ln� analyz�tor si s�m inicializuje)
//		v�jimky: CMemoryException, CDataArchiveException, CConfigFileException
void CConfigFile::Create ( CArchiveFile fileInput, LPCSTR lpszRootName ) 
{
	ASSERT ( !m_fileInput.IsOpened () );
	ASSERT ( fileInput.IsOpened () );

// pomocn� prom�nn�
	// posledn� na�ten� token z konfigura�n�ho souboru
	CConfigFileLexToken cLastToken;
	// lexik�ln� analyz�tor
	CConfigFileLex cConfigFileLex;

	// inicializuje vstupn� konfigura�n� soubor
	m_fileInput = fileInput;

	// inicializuje lexik�ln� analyz�tor
	cConfigFileLex.Create ( fileInput );

	// na�te informace o hlavn� sekci konfigura�n�ho souboru
	m_cRootSectionInfo.Read ( this, lpszRootName, cLastToken, cConfigFileLex );
	// sekce byla ��dn� ukon�ena (za��tkem nov� sekce nebo koncem souboru)

	// p�e�te zbyl� sekce a� do konce souboru
	while ( cLastToken.m_eToken != CConfigFileLexToken::CLTEndOfFile )
	{
		// byl p�e�ten za��tek nov� sekce

		// zkontroluje na�ten� za��tku nov� sekce
		ASSERT ( cLastToken.m_eToken == CConfigFileLexToken::CLTSectionBegin );

		// na�te z konfigura�n�ho souboru dal�� token
		cConfigFileLex.GetToken ( cLastToken );

		// nech� na��st informace o podsekci hlavn� sekce konfigura�n�ho souboru
		m_cRootSectionInfo.ReadSubsection ( cLastToken, cConfigFileLex );
		// podsekce byla ��dn� ukon�ena (za��tkem nov� sekce nebo koncem souboru)
	}
	// byl na�ten cel� soubor

	// zni�� lexik�ln� analyz�tor
	cConfigFileLex.Delete ();
}

// zav�e otev�en� konfigura�n� soubor
void CConfigFile::Delete () 
{
	ASSERT ( m_fileInput.IsOpened () );

	// zni�� informace o hlavn� sekci
	m_cRootSectionInfo.Delete ( FALSE );

	// zav�e otev�en� konfigura�n� soubor
	m_fileInput.Close ();
}
