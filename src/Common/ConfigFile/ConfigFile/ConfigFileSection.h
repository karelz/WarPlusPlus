/************************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Konfigura�n� soubory
 *   Autor: Karel Zikmund
 * 
 *   Popis: Objekt CConfigFileSection reprezentuj�c� sekci 
 *          konfigura�n�ho souboru
 * 
 ***********************************************************/

#ifndef __CONFIG_FILE_SECTION__HEADER__
#define __CONFIG_FILE_SECTION__HEADER__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// ************************************************************
// deklarace jmen t��d

// t��da reprezentuj�c� informace o sekci konfigura�n�ho souboru
class CConfigFileSectionInfo;

// ************************************************************
// t��da reprezentuj�c� sekci konfigura�n�ho souboru
class CConfigFileSection
{
// Data
public:
	// typ prom�nn� sekce
	enum EVariableType
	{
		VTString		= 0,
		VTInteger	= 1,
		VTReal		= 2,
		VTBoolean	= 3
	};

	// informace o prom�nn� sekce
	struct SVariableInfo
	{
		// jm�no prom�nn� sekce
		LPCSTR lpszName;
		// typ prom�nn� sekce
		enum EVariableType eType;
	};

// Metody
protected:
// Konstrukce a destrukce

	// konstruktor
	inline CConfigFileSection ( CConfigFileSectionInfo *pConfigFileSectionInfo );
public:
	// copy konstruktor
	inline CConfigFileSection ( CConfigFileSection &cConfigFileSection );
	// destruktor
	inline ~CConfigFileSection ();

public:
// Inicializace a zni�en� dat objektu

	// zkop�ruje sekci do tohoto objektu
	inline CConfigFileSection &operator = ( const CConfigFileSection &cConfigFileSection );

// Navigace stromem sekc�

	// vr�t� podsekci "lpszName" konfigura�n�ho souboru
	inline CConfigFileSection GetSubsection ( LPCSTR lpszName );

// Informace o sekci

	// vr�t� po�et podsekc� sekce
	inline int GetSubsectionsCount () const;
	// vr�t� jm�no podsekce (nSubsectionIndex je index do "pole", tj. od 0)
	inline LPCSTR GetSubsectionName ( int nSubsectionIndex ) const;
	// vr�t� po�et prom�nn�ch sekce
	inline int GetVariablesCount () const;
	// vr�t� informace o prom�nn� sekce (nVariableIndex je index do "pole", tj. od 0)
	inline struct SVariableInfo GetVariableInfo ( int nVariableIndex ) const;
	// vr�t� p��znak existence podsekce v sekci
	inline BOOL IsSubsection ( LPCSTR lpszSubsectionName ) const;
	// vr�t� p��znak existence prom�nn� v sekci
	inline BOOL IsVariable ( LPCSTR lpszVariableName ) const;

// �ten� hodnot prom�nn�ch sekce

	// vr�t� �et�zcovou hodnotu prom�nn� sekce
	//    v�jimky: CConfigFileException
	inline LPCSTR GetString ( LPCSTR lpszVariableName );
	// vr�t� ��selnou hodnotu prom�nn� sekce
	//    v�jimky: CConfigFileException
	inline long GetInteger ( LPCSTR lpszVariableName );
	// vr�t� re�lnou hodnotu prom�nn� sekce
	//    v�jimky: CConfigFileException
	inline double GetReal ( LPCSTR lpszVariableName );
	// vr�t� logickou hodnotu prom�nn� sekce
	//    v�jimky: CConfigFileException
	inline BOOL GetBoolean ( LPCSTR lpszVariableName );

// Data
private:
	// informace o sekci konfigura�n�ho souboru
	CConfigFileSectionInfo *m_pConfigFileSectionInfo;

  friend class CConfigFile;
	friend class CConfigFileSectionInfo;
};

#endif //__CONFIG_FILE_SECTION__HEADER__
