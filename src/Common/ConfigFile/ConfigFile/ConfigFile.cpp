/************************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Konfiguraèní soubory
 *   Autor: Karel Zikmund
 * 
 *   Popis: Objekt CConfigFile implementující rozhraní s konfiguraèními 
 *          soubory
 * 
 ***********************************************************/

#include "StdAfx.h"
#include "ConfigFile.h"

#include "..\ConfigFileLex\ConfigFileLex.h"

#include "ConfigFileException.h"

// ************************************************************
// tøída reprezentující konfiguraèní soubor
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
// Incializace a znièení dat objektu
//////////////////////////////////////////////////////////////////////

// otevøe konfiguraèní soubor "lpszName" v archívu "cArchive"
//		výjimky: CMemoryException, CDataArchiveException, CConfigFileException
void CConfigFile::Create ( LPCSTR lpszName, CDataArchive cArchive ) 
{
	ASSERT ( !m_fileInput.IsOpened () );

// pomocné promìnné
	// jméno konfiguraèního souboru
	CString strName;

	{
	// pomocné promìnné
		// soubor hlavního konfiguraèního souboru
		CArchiveFile fileMainConfigFile ( cArchive.CreateFile ( 
			_T("MainConfigFileIndex.cfg"), CFile::modeRead | CFile::shareDenyWrite ) );
		// hlavní konfiguraèní soubor
		CConfigFile cMainConfigFile;

		// otevøe hlavní konfiguraèní soubor
		cMainConfigFile.Create ( fileMainConfigFile, _T("") );

		// otevøe root sekci hlavního konfiguraèního souboru
		CConfigFileSection cRootSection = cMainConfigFile.GetSection ( _T("") );

		// zjistí jméno požadovaného konfiguraèního souboru
		strName = cRootSection.GetString ( lpszName );
	}

	// otevøe požadovaný konfiguraèní soubor a naète z nìj všechny sekce
	Create ( cArchive.CreateFile ( strName, CFile::modeRead | CFile::shareDenyWrite ), 
		_T("") );
}

// z konfiguraèního souboru "fileInput", jehož hlavní sekce se jmenuje "lpszRootName", 
//		naète stukturu sekcí (lexikální analyzátor si sám inicializuje)
//		výjimky: CMemoryException, CDataArchiveException, CConfigFileException
void CConfigFile::Create ( CArchiveFile fileInput, LPCSTR lpszRootName ) 
{
	ASSERT ( !m_fileInput.IsOpened () );
	ASSERT ( fileInput.IsOpened () );

// pomocné promìnné
	// poslední naètený token z konfiguraèního souboru
	CConfigFileLexToken cLastToken;
	// lexikální analyzátor
	CConfigFileLex cConfigFileLex;

	// inicializuje vstupní konfiguraèní soubor
	m_fileInput = fileInput;

	// inicializuje lexikální analyzátor
	cConfigFileLex.Create ( fileInput );

	// naète informace o hlavní sekci konfiguraèního souboru
	m_cRootSectionInfo.Read ( this, lpszRootName, cLastToken, cConfigFileLex );
	// sekce byla øádnì ukonèena (zaèátkem nové sekce nebo koncem souboru)

	// pøeète zbylé sekce až do konce souboru
	while ( cLastToken.m_eToken != CConfigFileLexToken::CLTEndOfFile )
	{
		// byl pøeèten zaèátek nové sekce

		// zkontroluje naètení zaèátku nové sekce
		ASSERT ( cLastToken.m_eToken == CConfigFileLexToken::CLTSectionBegin );

		// naète z konfiguraèního souboru další token
		cConfigFileLex.GetToken ( cLastToken );

		// nechá naèíst informace o podsekci hlavní sekce konfiguraèního souboru
		m_cRootSectionInfo.ReadSubsection ( cLastToken, cConfigFileLex );
		// podsekce byla øádnì ukonèena (zaèátkem nové sekce nebo koncem souboru)
	}
	// byl naèten celý soubor

	// znièí lexikální analyzátor
	cConfigFileLex.Delete ();
}

// zavøe otevøený konfiguraèní soubor
void CConfigFile::Delete () 
{
	ASSERT ( m_fileInput.IsOpened () );

	// znièí informace o hlavní sekci
	m_cRootSectionInfo.Delete ( FALSE );

	// zavøe otevøený konfiguraèní soubor
	m_fileInput.Close ();
}
