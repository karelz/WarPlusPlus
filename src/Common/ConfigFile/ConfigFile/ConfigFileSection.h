/************************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Konfiguraèní soubory
 *   Autor: Karel Zikmund
 * 
 *   Popis: Objekt CConfigFileSection reprezentující sekci 
 *          konfiguraèního souboru
 * 
 ***********************************************************/

#ifndef __CONFIG_FILE_SECTION__HEADER__
#define __CONFIG_FILE_SECTION__HEADER__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// ************************************************************
// deklarace jmen tøíd

// tøída reprezentující informace o sekci konfiguraèního souboru
class CConfigFileSectionInfo;

// ************************************************************
// tøída reprezentující sekci konfiguraèního souboru
class CConfigFileSection
{
// Data
public:
	// typ promìnné sekce
	enum EVariableType
	{
		VTString		= 0,
		VTInteger	= 1,
		VTReal		= 2,
		VTBoolean	= 3
	};

	// informace o promìnné sekce
	struct SVariableInfo
	{
		// jméno promìnné sekce
		LPCSTR lpszName;
		// typ promìnné sekce
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
// Inicializace a znièení dat objektu

	// zkopíruje sekci do tohoto objektu
	inline CConfigFileSection &operator = ( const CConfigFileSection &cConfigFileSection );

// Navigace stromem sekcí

	// vrátí podsekci "lpszName" konfiguraèního souboru
	inline CConfigFileSection GetSubsection ( LPCSTR lpszName );

// Informace o sekci

	// vrátí poèet podsekcí sekce
	inline int GetSubsectionsCount () const;
	// vrátí jméno podsekce (nSubsectionIndex je index do "pole", tj. od 0)
	inline LPCSTR GetSubsectionName ( int nSubsectionIndex ) const;
	// vrátí poèet promìnných sekce
	inline int GetVariablesCount () const;
	// vrátí informace o promìnné sekce (nVariableIndex je index do "pole", tj. od 0)
	inline struct SVariableInfo GetVariableInfo ( int nVariableIndex ) const;
	// vrátí pøíznak existence podsekce v sekci
	inline BOOL IsSubsection ( LPCSTR lpszSubsectionName ) const;
	// vrátí pøíznak existence promìnné v sekci
	inline BOOL IsVariable ( LPCSTR lpszVariableName ) const;

// Ètení hodnot promìnných sekce

	// vrátí øetìzcovou hodnotu promìnné sekce
	//    výjimky: CConfigFileException
	inline LPCSTR GetString ( LPCSTR lpszVariableName );
	// vrátí èíselnou hodnotu promìnné sekce
	//    výjimky: CConfigFileException
	inline long GetInteger ( LPCSTR lpszVariableName );
	// vrátí reálnou hodnotu promìnné sekce
	//    výjimky: CConfigFileException
	inline double GetReal ( LPCSTR lpszVariableName );
	// vrátí logickou hodnotu promìnné sekce
	//    výjimky: CConfigFileException
	inline BOOL GetBoolean ( LPCSTR lpszVariableName );

// Data
private:
	// informace o sekci konfiguraèního souboru
	CConfigFileSectionInfo *m_pConfigFileSectionInfo;

  friend class CConfigFile;
	friend class CConfigFileSectionInfo;
};

#endif //__CONFIG_FILE_SECTION__HEADER__
