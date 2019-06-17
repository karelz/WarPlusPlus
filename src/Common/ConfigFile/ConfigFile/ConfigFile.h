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

#ifndef __CONFIG_FILE__HEADER__
#define __CONFIG_FILE__HEADER__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxwin.h>

#include "Archive\Archive\Archive.h"

#include "ConfigFileSectionInfo.h"
#include "ConfigFileSection.h"

// aby se nemusela speci�ln� includovat t��da v�jimky
#include "ConfigFileException.h"
// aby se nemusely speci�ln� includovat inline metody t��dy sekce
#include "ConfigFileSection.inl"
// aby se nemusely speci�ln� includovat inline metody t��dy informac� o sekci
#include "ConfigFileSectionInfo.inl"

// ************************************************************
// t��da reprezentuj�c� konfigura�n� soubor
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
// Incializace a zni�en� dat objektu

	// otev�e konfigura�n� soubor "lpszName" v arch�vu "cArchive"
	//		v�jimky: CMemoryException, CDataArchiveException, CConfigFileException
	void Create ( LPCSTR lpszName, CDataArchive cArchive );
	// otev�e konfigura�n� soubor "fileInput"
	//		v�jimky: CMemoryException, CDataArchiveException, CConfigFileException
	void Create ( CArchiveFile fileInput ) 
		{ Create ( fileInput, _T("") ); };
	// zav�e otev�en� konfigura�n� soubor
	//		v�jimky: CDataArchiveException
	void Delete ();
private:
	// otev�e konfigura�n� soubor "fileInput" jeho� hlavn� sekce se jmenuje "lpszRootName"
	//		v�jimky: CMemoryException, CDataArchiveException, CConfigFileException
	void Create ( CArchiveFile fileInput, LPCSTR lpszRootName );

protected:
// Informace o souboru

	// vr�t� ukazatel na informace o hlavn� sekci konfigura�n�ho souboru
	const CConfigFileSectionInfo *GetRootSectionInfo () const 
		{ return &m_cRootSectionInfo; };
public:
	// vr�t� objekt vstupn�ho konfigura�n�ho souboru
	CArchiveFile GetFile () const 
		{ return m_fileInput; };
	// vr�t� sekci "lpszName" konfigura�n�ho souboru
	//		v�jimky: CConfigFileException
	CConfigFileSection GetSection ( LPCSTR lpszName ) 
		{ return m_cRootSectionInfo.GetSection ( lpszName ); };
	// vr�t� hlavn� sekci konfigura�n�ho souboru
  CConfigFileSection GetRootSection ()
    { return CConfigFileSection ( &m_cRootSectionInfo ); };

// Data
private:
// Informace o konfigura�n�m souboru

	// vstupn� konfigura�n� soubor
	CArchiveFile m_fileInput;

	// informace o hlavn� sekci konfigura�n�ho souboru
	CConfigFileSectionInfo m_cRootSectionInfo;

	friend class CConfigFileSectionInfo;
};

#endif //__CONFIG_FILE__HEADER__
