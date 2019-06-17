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

#ifndef __CONFIG_FILE_SECTION__INLINE_HEADER__
#define __CONFIG_FILE_SECTION__INLINE_HEADER__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ConfigFileSection.h"

#include "ConfigFileSectionInfo.h"

// ************************************************************
// inline metody tøídy reprezentující sekci konfiguraèního souboru

//////////////////////////////////////////////////////////////////////
// Konstrukce a destrukce
//////////////////////////////////////////////////////////////////////

// konstruktor
inline CConfigFileSection::CConfigFileSection ( 
	CConfigFileSectionInfo *pConfigFileSectionInfo ) 
{
	// schová si ukazatel na objekt informací o sekci
	m_pConfigFileSectionInfo = pConfigFileSectionInfo;
	// zvýší poèet odkazù na promìnné sekce
	m_pConfigFileSectionInfo->AddVariablesReference ();
}

// copy konstruktor
inline CConfigFileSection::CConfigFileSection ( CConfigFileSection &cConfigFileSection ) 
{
	// schová si ukazatel na objekt informací o sekci
	m_pConfigFileSectionInfo = cConfigFileSection.m_pConfigFileSectionInfo;
	// zvýší poèet odkazù na promìnné sekce
	m_pConfigFileSectionInfo->AddVariablesReference ();
}

// destruktor
inline CConfigFileSection::~CConfigFileSection () 
{
	// sníží poèet odkazù na promìnné sekce
	m_pConfigFileSectionInfo->ReleaseVariablesReference ();
}

//////////////////////////////////////////////////////////////////////
// Inicializace a znièení dat objektu
//////////////////////////////////////////////////////////////////////

// zkopíruje sekci do tohoto objektu
inline CConfigFileSection &CConfigFileSection::operator = ( 
	const CConfigFileSection &cConfigFileSection ) 
{
	// sníží poèet odkazù na promìnné sekce
	m_pConfigFileSectionInfo->ReleaseVariablesReference ();

	// schová si ukazatel na nový objekt informací o sekci
	m_pConfigFileSectionInfo = cConfigFileSection.m_pConfigFileSectionInfo;
	// zvýší poèet odkazù na promìnné sekce
	m_pConfigFileSectionInfo->AddVariablesReference ();

	return *this;
}

//////////////////////////////////////////////////////////////////////
// Navigace stromem sekcí
//////////////////////////////////////////////////////////////////////

// vrátí podsekci "lpszName" konfiguraèního souboru
inline CConfigFileSection CConfigFileSection::GetSubsection ( LPCSTR lpszName ) 
{
	return m_pConfigFileSectionInfo->GetSection ( lpszName );
}

//////////////////////////////////////////////////////////////////////
// Informace o sekci
//////////////////////////////////////////////////////////////////////

// vrátí poèet podsekcí sekce
inline int CConfigFileSection::GetSubsectionsCount () const 
{
	return m_pConfigFileSectionInfo->GetSubsectionsCount ();
}

// vrátí jméno podsekce (nSubsectionIndex je index do "pole", tj. od 0)
inline LPCSTR CConfigFileSection::GetSubsectionName ( int nSubsectionIndex ) const 
{
	return m_pConfigFileSectionInfo->GetSubsectionName ( nSubsectionIndex );
}

// vrátí poèet promìnných sekce
inline int CConfigFileSection::GetVariablesCount () const 
{
	return m_pConfigFileSectionInfo->GetVariablesCount ();
}

// vrátí informace o promìnné sekce (nVariableIndex je index do "pole", tj. od 0)
inline struct CConfigFileSection::SVariableInfo CConfigFileSection::GetVariableInfo ( int nVariableIndex ) const 
{
	return m_pConfigFileSectionInfo->GetVariableInfo ( nVariableIndex );
}

// vrátí pøíznak existence podsekce v sekci
inline BOOL CConfigFileSection::IsSubsection ( LPCSTR lpszSubsectionName ) const 
{
	return m_pConfigFileSectionInfo->IsSubsection ( lpszSubsectionName );
}

// vrátí pøíznak existence promìnné v sekci
inline BOOL CConfigFileSection::IsVariable ( LPCSTR lpszVariableName ) const 
{
	return m_pConfigFileSectionInfo->IsVariable ( lpszVariableName );
}

//////////////////////////////////////////////////////////////////////
// Ètení hodnot promìnných sekce
//////////////////////////////////////////////////////////////////////

// vrátí øetìzcovou hodnotu promìnné sekce
//    výjimky: CConfigFileException
inline LPCSTR CConfigFileSection::GetString ( LPCSTR lpszVariableName ) 
{
	return m_pConfigFileSectionInfo->GetVariableStringValue ( lpszVariableName );
}

// vrátí èíselnou hodnotu promìnné sekce
//    výjimky: CConfigFileException
inline long CConfigFileSection::GetInteger ( LPCSTR lpszVariableName ) 
{
	return m_pConfigFileSectionInfo->GetVariableIntegerValue ( lpszVariableName );
};

// vrátí reálnou hodnotu promìnné sekce
//    výjimky: CConfigFileException
inline double CConfigFileSection::GetReal ( LPCSTR lpszVariableName ) 
{
	return m_pConfigFileSectionInfo->GetVariableRealValue ( lpszVariableName );
};

// vrátí logickou hodnotu promìnné sekce
//    výjimky: CConfigFileException
inline BOOL CConfigFileSection::GetBoolean ( LPCSTR lpszVariableName ) 
{
	return m_pConfigFileSectionInfo->GetVariableBooleanValue ( lpszVariableName );
};

#endif //__CONFIG_FILE_SECTION__INLINE_HEADER__
