/************************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Konfiguraèní soubory
 *   Autor: Karel Zikmund
 * 
 *   Popis: Objekt CConfigFileSectionInfo reprezentující informace
 *          o sekci konfiguraèního souboru
 * 
 ***********************************************************/

#ifndef __CONFIG_FILE_SECTION_INFO__HEADER__
#define __CONFIG_FILE_SECTION_INFO__HEADER__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxwin.h>

#include "..\ConfigFileLex\ConfigFileLex.h"
#include "ConfigFileSection.h"

// ************************************************************
// deklarace jmen tøíd

// tøída reprezentující konfiguraèní soubor
class CConfigFile;

// ************************************************************
// tøída reprezentující informace o sekci konfiguraèního souboru
class CConfigFileSectionInfo
{
// Datové typy
private:
	// typ hodnoty promìnné
	enum EVariableValueType
	{
		VVTString	= 0,
		VVTInteger	= 1,
		VVTReal		= 2,
		VVTBoolean	= 3
	};

	// struktura reprezentující hodnotu promìnné
	struct SVariableValue
	{
		// jméno promìnné
		LPCSTR m_lpszName;
		// typ hodnoty promìnné
		enum EVariableValueType m_eType;
		// hodnota promìnné - závisí na typu hodnoty promìnné
		union
		{
			LPCSTR m_lpszValue;
			long m_nValue;
			double m_dbValue;
			BOOL m_bValue;
		};
	};

// Metody
protected:
// Konstrukce a destrukce

	// konstruktor
	inline CConfigFileSectionInfo ( CConfigFile *pConfigFile = NULL );
	// destruktor
	~CConfigFileSectionInfo ();

// Incializace a znièení dat objektu

	// naète z konfiguraèního souboru "pConfigFile" informace o sekci jménem "lpszName" 
	//		z aktuální pozice v souboru a v "cLastToken" vrátí poslední naètený token, 
	//		požívá lexikální analyzátor "cConfigFileLex"
	//		vyjímky: CConfigFileException, CDataArchiveException, CMemoryException
	void Read ( CConfigFile *pConfigFile, LPCSTR lpszName, 
		CConfigFileLexToken &cLastToken, CConfigFileLex &cConfigFileLex );
private:
	// pøeète z aktuální pozice v konfiguraèním souboru obsah sekce a v "cLastToken" vrátí 
	//		poslední naètený token, požívá lexikální analyzátor "cConfigFileLex"
	//		vyjímky: CConfigFileException, CDataArchiveException, CMemoryException
	void Read ( CConfigFileLexToken &cLastToken, CConfigFileLex &cConfigFileLex );
	// znièí informace o sekci (vèetnì informací o podsekcích a vèetnì konfiguraèních 
	//		souborù) (bSelfDelete - pøíznak volání destruktoru na sebe)
	void Delete ( BOOL bSelfDelete = FALSE );
	// naète informace o podsekci této sekce z aktuální pozice v souboru a v "cLastToken" 
	//		vrátí poslední naètený token, požívá lexikální analyzátor "cConfigFileLex"
	//		vyjímky: CConfigFileException, CDataArchiveException, CMemoryException
	void ReadSubsection ( CConfigFileLexToken &cLastToken, CConfigFileLex &cConfigFileLex );

private:
// Operace se sekcí

	// vrátí jméno první podsekce "strName", které o tuto podsekci zkrátí
	static CString CConfigFileSectionInfo::GetSubsectionName ( CString &strName );
protected:
	// vrátí sekci "lpszName" konfiguraèního souboru
	CConfigFileSection GetSection ( LPCSTR lpszName );
	// zvýší poèet odkazù na pole promìnných sekce
	void AddVariablesReference ();
	// sníží poèet odkazù na pole promìnných sekce
	inline void ReleaseVariablesReference ();

// Informace o sekci

	// vrátí poèet podsekcí sekce
	inline int GetSubsectionsCount () const;
	// vrátí jméno podsekce (nSubsectionIndex je index do "pole", tj. od 0)
	inline LPCSTR GetSubsectionName ( int nSubsectionIndex ) const;
	// vrátí poèet promìnných sekce
	inline int GetVariablesCount () const;
	// vrátí informace o promìnné sekce (nVariableIndex je index do "pole", tj. od 0)
	inline struct CConfigFileSection::SVariableInfo GetVariableInfo ( int nVariableIndex ) const;
	// vrátí pøíznak existence podsekce v sekci
	BOOL IsSubsection ( LPCSTR lpszSubsectionName ) const;
	// vrátí pøíznak existence promìnné v sekci
	BOOL IsVariable ( LPCSTR lpszVariableName ) const;

protected:
// Ètení hodnot promìnných sekce

	// vrátí øetìzcovou hodnotu promìnné sekce
	//    výjimky: CConfigFileException
	LPCSTR GetVariableStringValue ( LPCSTR lpszVariableName ) const;
	// vrátí èíselnou hodnotu promìnné sekce
	//    výjimky: CConfigFileException
	long GetVariableIntegerValue ( LPCSTR lpszVariableName ) const;
	// vrátí reálnou hodnotu promìnné sekce
	//    výjimky: CConfigFileException
	double GetVariableRealValue ( LPCSTR lpszVariableName ) const;
	// vrátí logickou hodnotu promìnné sekce
	//    výjimky: CConfigFileException
	BOOL GetVariableBooleanValue ( LPCSTR lpszVariableName ) const;
private:
	// vrátí index hodnoty promìnné v poli hodnot promìnných
	//    výjimky: CConfigFileException
	int GetVariableIndex ( LPCSTR lpszVariableName ) const;

	// Data
private:
// Informace o sekci konfiguraèního souboru

	// konfiguraèní soubor
	CConfigFile *m_pConfigFile;
	// jméno sekce konfiguraèního souboru
	CString m_strName;
	// offset zaèátku dat sekce v konfiguraèním souboru
	DWORD m_dwOffset;
	// pole informací o podsekcích
	CPtrArray m_aSubsections;

// Informace o promìnných sekce

	// poèet odkazù na pole promìnných sekce
	DWORD m_dwVariablesReferenceCount;
	// poèet promìnných sekce
	int m_nVariablesCount;
	// ukazatel na pole hodnot promìnných sekce
	struct SVariableValue *m_pVariableValues;
	// velikost tabulky øetìzcù jmen a hodnot promìnných
	DWORD m_nStringTableSize;
	// ukazatel na tabulku øetìzcù jmen a hodnot promìnných
	char *m_pStringTable;

	// pole promìnných sekce
	//CArray m_aVariables;

	friend class CConfigFile;
	friend class CConfigFileSection;
};

#endif //__CONFIG_FILE_SECTION_INFO__HEADER__
