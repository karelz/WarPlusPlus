/************************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Konfigura�n� soubory
 *   Autor: Karel Zikmund
 * 
 *   Popis: Objekt CConfigFileSectionInfo reprezentuj�c� informace
 *          o sekci konfigura�n�ho souboru
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
// deklarace jmen t��d

// t��da reprezentuj�c� konfigura�n� soubor
class CConfigFile;

// ************************************************************
// t��da reprezentuj�c� informace o sekci konfigura�n�ho souboru
class CConfigFileSectionInfo
{
// Datov� typy
private:
	// typ hodnoty prom�nn�
	enum EVariableValueType
	{
		VVTString	= 0,
		VVTInteger	= 1,
		VVTReal		= 2,
		VVTBoolean	= 3
	};

	// struktura reprezentuj�c� hodnotu prom�nn�
	struct SVariableValue
	{
		// jm�no prom�nn�
		LPCSTR m_lpszName;
		// typ hodnoty prom�nn�
		enum EVariableValueType m_eType;
		// hodnota prom�nn� - z�vis� na typu hodnoty prom�nn�
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

// Incializace a zni�en� dat objektu

	// na�te z konfigura�n�ho souboru "pConfigFile" informace o sekci jm�nem "lpszName" 
	//		z aktu�ln� pozice v souboru a v "cLastToken" vr�t� posledn� na�ten� token, 
	//		po��v� lexik�ln� analyz�tor "cConfigFileLex"
	//		vyj�mky: CConfigFileException, CDataArchiveException, CMemoryException
	void Read ( CConfigFile *pConfigFile, LPCSTR lpszName, 
		CConfigFileLexToken &cLastToken, CConfigFileLex &cConfigFileLex );
private:
	// p�e�te z aktu�ln� pozice v konfigura�n�m souboru obsah sekce a v "cLastToken" vr�t� 
	//		posledn� na�ten� token, po��v� lexik�ln� analyz�tor "cConfigFileLex"
	//		vyj�mky: CConfigFileException, CDataArchiveException, CMemoryException
	void Read ( CConfigFileLexToken &cLastToken, CConfigFileLex &cConfigFileLex );
	// zni�� informace o sekci (v�etn� informac� o podsekc�ch a v�etn� konfigura�n�ch 
	//		soubor�) (bSelfDelete - p��znak vol�n� destruktoru na sebe)
	void Delete ( BOOL bSelfDelete = FALSE );
	// na�te informace o podsekci t�to sekce z aktu�ln� pozice v souboru a v "cLastToken" 
	//		vr�t� posledn� na�ten� token, po��v� lexik�ln� analyz�tor "cConfigFileLex"
	//		vyj�mky: CConfigFileException, CDataArchiveException, CMemoryException
	void ReadSubsection ( CConfigFileLexToken &cLastToken, CConfigFileLex &cConfigFileLex );

private:
// Operace se sekc�

	// vr�t� jm�no prvn� podsekce "strName", kter� o tuto podsekci zkr�t�
	static CString CConfigFileSectionInfo::GetSubsectionName ( CString &strName );
protected:
	// vr�t� sekci "lpszName" konfigura�n�ho souboru
	CConfigFileSection GetSection ( LPCSTR lpszName );
	// zv��� po�et odkaz� na pole prom�nn�ch sekce
	void AddVariablesReference ();
	// sn�� po�et odkaz� na pole prom�nn�ch sekce
	inline void ReleaseVariablesReference ();

// Informace o sekci

	// vr�t� po�et podsekc� sekce
	inline int GetSubsectionsCount () const;
	// vr�t� jm�no podsekce (nSubsectionIndex je index do "pole", tj. od 0)
	inline LPCSTR GetSubsectionName ( int nSubsectionIndex ) const;
	// vr�t� po�et prom�nn�ch sekce
	inline int GetVariablesCount () const;
	// vr�t� informace o prom�nn� sekce (nVariableIndex je index do "pole", tj. od 0)
	inline struct CConfigFileSection::SVariableInfo GetVariableInfo ( int nVariableIndex ) const;
	// vr�t� p��znak existence podsekce v sekci
	BOOL IsSubsection ( LPCSTR lpszSubsectionName ) const;
	// vr�t� p��znak existence prom�nn� v sekci
	BOOL IsVariable ( LPCSTR lpszVariableName ) const;

protected:
// �ten� hodnot prom�nn�ch sekce

	// vr�t� �et�zcovou hodnotu prom�nn� sekce
	//    v�jimky: CConfigFileException
	LPCSTR GetVariableStringValue ( LPCSTR lpszVariableName ) const;
	// vr�t� ��selnou hodnotu prom�nn� sekce
	//    v�jimky: CConfigFileException
	long GetVariableIntegerValue ( LPCSTR lpszVariableName ) const;
	// vr�t� re�lnou hodnotu prom�nn� sekce
	//    v�jimky: CConfigFileException
	double GetVariableRealValue ( LPCSTR lpszVariableName ) const;
	// vr�t� logickou hodnotu prom�nn� sekce
	//    v�jimky: CConfigFileException
	BOOL GetVariableBooleanValue ( LPCSTR lpszVariableName ) const;
private:
	// vr�t� index hodnoty prom�nn� v poli hodnot prom�nn�ch
	//    v�jimky: CConfigFileException
	int GetVariableIndex ( LPCSTR lpszVariableName ) const;

	// Data
private:
// Informace o sekci konfigura�n�ho souboru

	// konfigura�n� soubor
	CConfigFile *m_pConfigFile;
	// jm�no sekce konfigura�n�ho souboru
	CString m_strName;
	// offset za��tku dat sekce v konfigura�n�m souboru
	DWORD m_dwOffset;
	// pole informac� o podsekc�ch
	CPtrArray m_aSubsections;

// Informace o prom�nn�ch sekce

	// po�et odkaz� na pole prom�nn�ch sekce
	DWORD m_dwVariablesReferenceCount;
	// po�et prom�nn�ch sekce
	int m_nVariablesCount;
	// ukazatel na pole hodnot prom�nn�ch sekce
	struct SVariableValue *m_pVariableValues;
	// velikost tabulky �et�zc� jmen a hodnot prom�nn�ch
	DWORD m_nStringTableSize;
	// ukazatel na tabulku �et�zc� jmen a hodnot prom�nn�ch
	char *m_pStringTable;

	// pole prom�nn�ch sekce
	//CArray m_aVariables;

	friend class CConfigFile;
	friend class CConfigFileSection;
};

#endif //__CONFIG_FILE_SECTION_INFO__HEADER__
