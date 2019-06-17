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

#ifndef __CONFIG_FILE_SECTION_INFO__INLINE_HEADER__
#define __CONFIG_FILE_SECTION_INFO__INLINE_HEADER__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ConfigFileSectionInfo.h"

#include "Common\AfxDebugPlus\AfxDebugPlus.h"

// ************************************************************
// inline metody tøídy reprezentující informace o sekci konfiguraèního souboru

//////////////////////////////////////////////////////////////////////
// Konstrukce a destrukce
//////////////////////////////////////////////////////////////////////

// konstruktor
inline CConfigFileSectionInfo::CConfigFileSectionInfo ( CConfigFile *pConfigFile /*= NULL*/ ) 
{
	m_pConfigFile = pConfigFile;
	m_dwVariablesReferenceCount = 0;
}

//////////////////////////////////////////////////////////////////////
// Operace se sekcí
//////////////////////////////////////////////////////////////////////

// sníží poèet odkazù na pole promìnných sekce
inline void CConfigFileSectionInfo::ReleaseVariablesReference () 
{
	// musí obsahovat platné informace o sekci konfiguraèního souboru
	ASSERT ( m_pConfigFile != NULL );
	// musí existovat odkaz na promìnné sekce
	ASSERT ( m_dwVariablesReferenceCount > 0 );

	// sníží poèet odkazù na promìnné sekce
	m_dwVariablesReferenceCount--;

	// zjistí, byl-li to poslední odkaz na pole promìnných sekce
	if ( m_dwVariablesReferenceCount == 0 )
	{	// byl to poslední odkaz na pole promìnných
		// zjistí, jedná-li se o prázdnou sekci
		if ( m_nVariablesCount > 0 )
		{	// nejedná se o prázdnou sekci
			ASSERT ( m_pVariableValues != NULL );
			ASSERT ( m_nStringTableSize > 0 );
			ASSERT ( m_pStringTable != NULL );

			// znièí data promìnných sekce
			delete m_pVariableValues;
			delete m_pStringTable;
		}
	}
}

//////////////////////////////////////////////////////////////////////
// Informace o sekci
//////////////////////////////////////////////////////////////////////

// vrátí poèet podsekcí sekce
inline int CConfigFileSectionInfo::GetSubsectionsCount () const 
{
	// musí obsahovat platné informace o sekci konfiguraèního souboru
	ASSERT ( m_pConfigFile != NULL );

	// vrátí poèet podsekcí sekce
	return m_aSubsections.GetSize ();
}

// vrátí jméno podsekce (nSubsectionIndex je index do "pole", tj. od 0)
inline LPCSTR CConfigFileSectionInfo::GetSubsectionName ( int nSubsectionIndex ) const 
{
	// musí obsahovat platné informace o sekci konfiguraèního souboru
	ASSERT ( m_pConfigFile != NULL );

	// index podsekce musí být platný
	ASSERT ( ( nSubsectionIndex >= 0 ) && ( nSubsectionIndex < m_aSubsections.GetSize () ) );

	// vrátí jméno podsekce
	return ((CConfigFileSectionInfo *)m_aSubsections[nSubsectionIndex])->m_strName;
}

// vrátí poèet promìnných sekce
inline int CConfigFileSectionInfo::GetVariablesCount () const 
{
	// musí obsahovat platné informace o sekci konfiguraèního souboru
	ASSERT ( m_pConfigFile != NULL );
	// musí existovat odkaz na promìnné sekce
	ASSERT ( m_dwVariablesReferenceCount > 0 );

	// vrátí poèet promìnných sekce
	return m_nVariablesCount;
}

// vrátí informace o promìnné sekce (nVariableIndex je index do "pole", tj. od 0)
inline struct CConfigFileSection::SVariableInfo CConfigFileSectionInfo::GetVariableInfo ( 
	int nVariableIndex ) const 
{
	// musí obsahovat platné informace o sekci konfiguraèního souboru
	ASSERT ( m_pConfigFile != NULL );
	// musí existovat odkaz na promìnné sekce
	ASSERT ( m_dwVariablesReferenceCount > 0 );

	// index promìnné musí být platný
	ASSERT ( ( nVariableIndex >= 0 ) && ( nVariableIndex < m_nVariablesCount ) );

// pomocné promìnné
	// ukazatel na hodnotu promìnné sekce
	struct SVariableValue *pVariableValue = m_pVariableValues + nVariableIndex;
	// vracené informace o promìnné sekce
	struct CConfigFileSection::SVariableInfo sVariableInfo;

	// vyplní vracené informace o promìnné
	sVariableInfo.lpszName = pVariableValue->m_lpszName;
	sVariableInfo.eType = (CConfigFileSection::EVariableType)pVariableValue->m_eType;

	// vrátí informace o promìnné sekce
	return sVariableInfo;
}

#endif //__CONFIG_FILE_SECTION_INFO__INLINE_HEADER__
