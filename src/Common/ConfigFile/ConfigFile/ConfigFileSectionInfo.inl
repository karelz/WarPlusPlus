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

#ifndef __CONFIG_FILE_SECTION_INFO__INLINE_HEADER__
#define __CONFIG_FILE_SECTION_INFO__INLINE_HEADER__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ConfigFileSectionInfo.h"

#include "Common\AfxDebugPlus\AfxDebugPlus.h"

// ************************************************************
// inline metody t��dy reprezentuj�c� informace o sekci konfigura�n�ho souboru

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
// Operace se sekc�
//////////////////////////////////////////////////////////////////////

// sn�� po�et odkaz� na pole prom�nn�ch sekce
inline void CConfigFileSectionInfo::ReleaseVariablesReference () 
{
	// mus� obsahovat platn� informace o sekci konfigura�n�ho souboru
	ASSERT ( m_pConfigFile != NULL );
	// mus� existovat odkaz na prom�nn� sekce
	ASSERT ( m_dwVariablesReferenceCount > 0 );

	// sn�� po�et odkaz� na prom�nn� sekce
	m_dwVariablesReferenceCount--;

	// zjist�, byl-li to posledn� odkaz na pole prom�nn�ch sekce
	if ( m_dwVariablesReferenceCount == 0 )
	{	// byl to posledn� odkaz na pole prom�nn�ch
		// zjist�, jedn�-li se o pr�zdnou sekci
		if ( m_nVariablesCount > 0 )
		{	// nejedn� se o pr�zdnou sekci
			ASSERT ( m_pVariableValues != NULL );
			ASSERT ( m_nStringTableSize > 0 );
			ASSERT ( m_pStringTable != NULL );

			// zni�� data prom�nn�ch sekce
			delete m_pVariableValues;
			delete m_pStringTable;
		}
	}
}

//////////////////////////////////////////////////////////////////////
// Informace o sekci
//////////////////////////////////////////////////////////////////////

// vr�t� po�et podsekc� sekce
inline int CConfigFileSectionInfo::GetSubsectionsCount () const 
{
	// mus� obsahovat platn� informace o sekci konfigura�n�ho souboru
	ASSERT ( m_pConfigFile != NULL );

	// vr�t� po�et podsekc� sekce
	return m_aSubsections.GetSize ();
}

// vr�t� jm�no podsekce (nSubsectionIndex je index do "pole", tj. od 0)
inline LPCSTR CConfigFileSectionInfo::GetSubsectionName ( int nSubsectionIndex ) const 
{
	// mus� obsahovat platn� informace o sekci konfigura�n�ho souboru
	ASSERT ( m_pConfigFile != NULL );

	// index podsekce mus� b�t platn�
	ASSERT ( ( nSubsectionIndex >= 0 ) && ( nSubsectionIndex < m_aSubsections.GetSize () ) );

	// vr�t� jm�no podsekce
	return ((CConfigFileSectionInfo *)m_aSubsections[nSubsectionIndex])->m_strName;
}

// vr�t� po�et prom�nn�ch sekce
inline int CConfigFileSectionInfo::GetVariablesCount () const 
{
	// mus� obsahovat platn� informace o sekci konfigura�n�ho souboru
	ASSERT ( m_pConfigFile != NULL );
	// mus� existovat odkaz na prom�nn� sekce
	ASSERT ( m_dwVariablesReferenceCount > 0 );

	// vr�t� po�et prom�nn�ch sekce
	return m_nVariablesCount;
}

// vr�t� informace o prom�nn� sekce (nVariableIndex je index do "pole", tj. od 0)
inline struct CConfigFileSection::SVariableInfo CConfigFileSectionInfo::GetVariableInfo ( 
	int nVariableIndex ) const 
{
	// mus� obsahovat platn� informace o sekci konfigura�n�ho souboru
	ASSERT ( m_pConfigFile != NULL );
	// mus� existovat odkaz na prom�nn� sekce
	ASSERT ( m_dwVariablesReferenceCount > 0 );

	// index prom�nn� mus� b�t platn�
	ASSERT ( ( nVariableIndex >= 0 ) && ( nVariableIndex < m_nVariablesCount ) );

// pomocn� prom�nn�
	// ukazatel na hodnotu prom�nn� sekce
	struct SVariableValue *pVariableValue = m_pVariableValues + nVariableIndex;
	// vracen� informace o prom�nn� sekce
	struct CConfigFileSection::SVariableInfo sVariableInfo;

	// vypln� vracen� informace o prom�nn�
	sVariableInfo.lpszName = pVariableValue->m_lpszName;
	sVariableInfo.eType = (CConfigFileSection::EVariableType)pVariableValue->m_eType;

	// vr�t� informace o prom�nn� sekce
	return sVariableInfo;
}

#endif //__CONFIG_FILE_SECTION_INFO__INLINE_HEADER__
