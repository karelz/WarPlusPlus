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

#ifndef __CONFIG_FILE__HEADER__
#define __CONFIG_FILE__HEADER__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxwin.h>

#include "Archive\Archive\Archive.h"

#include "ConfigFileSectionInfo.h"
#include "ConfigFileSection.h"

// aby se nemusela speciálnì includovat tøída výjimky
#include "ConfigFileException.h"
// aby se nemusely speciálnì includovat inline metody tøídy sekce
#include "ConfigFileSection.inl"
// aby se nemusely speciálnì includovat inline metody tøídy informací o sekci
#include "ConfigFileSectionInfo.inl"

// ************************************************************
// tøída reprezentující konfiguraèní soubor
class CConfigFile
{
// Metody
public:
// Konstrukce a destrukce

	// konstruktor
	CConfigFile ();
	// destruktor
	~CConfigFile ();

public:
// Incializace a znièení dat objektu

	// otevøe konfiguraèní soubor "lpszName" v archívu "cArchive"
	//		výjimky: CMemoryException, CDataArchiveException, CConfigFileException
	void Create ( LPCSTR lpszName, CDataArchive cArchive );
	// otevøe konfiguraèní soubor "fileInput"
	//		výjimky: CMemoryException, CDataArchiveException, CConfigFileException
	void Create ( CArchiveFile fileInput ) 
		{ Create ( fileInput, _T("") ); };
	// zavøe otevøený konfiguraèní soubor
	//		výjimky: CDataArchiveException
	void Delete ();
private:
	// otevøe konfiguraèní soubor "fileInput" jehož hlavní sekce se jmenuje "lpszRootName"
	//		výjimky: CMemoryException, CDataArchiveException, CConfigFileException
	void Create ( CArchiveFile fileInput, LPCSTR lpszRootName );

protected:
// Informace o souboru

	// vrátí ukazatel na informace o hlavní sekci konfiguraèního souboru
	const CConfigFileSectionInfo *GetRootSectionInfo () const 
		{ return &m_cRootSectionInfo; };
public:
	// vrátí objekt vstupního konfiguraèního souboru
	CArchiveFile GetFile () const 
		{ return m_fileInput; };
	// vrátí sekci "lpszName" konfiguraèního souboru
	//		výjimky: CConfigFileException
	CConfigFileSection GetSection ( LPCSTR lpszName ) 
		{ return m_cRootSectionInfo.GetSection ( lpszName ); };
	// vrátí hlavní sekci konfiguraèního souboru
  CConfigFileSection GetRootSection ()
    { return CConfigFileSection ( &m_cRootSectionInfo ); };

// Data
private:
// Informace o konfiguraèním souboru

	// vstupní konfiguraèní soubor
	CArchiveFile m_fileInput;

	// informace o hlavní sekci konfiguraèního souboru
	CConfigFileSectionInfo m_cRootSectionInfo;

	friend class CConfigFileSectionInfo;
};

#endif //__CONFIG_FILE__HEADER__
