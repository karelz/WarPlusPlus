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

#ifndef __CONFIG_FILE_SECTION__INLINE_HEADER__
#define __CONFIG_FILE_SECTION__INLINE_HEADER__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ConfigFileSection.h"

#include "ConfigFileSectionInfo.h"

// ************************************************************
// inline metody t��dy reprezentuj�c� sekci konfigura�n�ho souboru

//////////////////////////////////////////////////////////////////////
// Konstrukce a destrukce
//////////////////////////////////////////////////////////////////////

// konstruktor
inline CConfigFileSection::CConfigFileSection ( 
	CConfigFileSectionInfo *pConfigFileSectionInfo ) 
{
	// schov� si ukazatel na objekt informac� o sekci
	m_pConfigFileSectionInfo = pConfigFileSectionInfo;
	// zv��� po�et odkaz� na prom�nn� sekce
	m_pConfigFileSectionInfo->AddVariablesReference ();
}

// copy konstruktor
inline CConfigFileSection::CConfigFileSection ( CConfigFileSection &cConfigFileSection ) 
{
	// schov� si ukazatel na objekt informac� o sekci
	m_pConfigFileSectionInfo = cConfigFileSection.m_pConfigFileSectionInfo;
	// zv��� po�et odkaz� na prom�nn� sekce
	m_pConfigFileSectionInfo->AddVariablesReference ();
}

// destruktor
inline CConfigFileSection::~CConfigFileSection () 
{
	// sn�� po�et odkaz� na prom�nn� sekce
	m_pConfigFileSectionInfo->ReleaseVariablesReference ();
}

//////////////////////////////////////////////////////////////////////
// Inicializace a zni�en� dat objektu
//////////////////////////////////////////////////////////////////////

// zkop�ruje sekci do tohoto objektu
inline CConfigFileSection &CConfigFileSection::operator = ( 
	const CConfigFileSection &cConfigFileSection ) 
{
	// sn�� po�et odkaz� na prom�nn� sekce
	m_pConfigFileSectionInfo->ReleaseVariablesReference ();

	// schov� si ukazatel na nov� objekt informac� o sekci
	m_pConfigFileSectionInfo = cConfigFileSection.m_pConfigFileSectionInfo;
	// zv��� po�et odkaz� na prom�nn� sekce
	m_pConfigFileSectionInfo->AddVariablesReference ();

	return *this;
}

//////////////////////////////////////////////////////////////////////
// Navigace stromem sekc�
//////////////////////////////////////////////////////////////////////

// vr�t� podsekci "lpszName" konfigura�n�ho souboru
inline CConfigFileSection CConfigFileSection::GetSubsection ( LPCSTR lpszName ) 
{
	return m_pConfigFileSectionInfo->GetSection ( lpszName );
}

//////////////////////////////////////////////////////////////////////
// Informace o sekci
//////////////////////////////////////////////////////////////////////

// vr�t� po�et podsekc� sekce
inline int CConfigFileSection::GetSubsectionsCount () const 
{
	return m_pConfigFileSectionInfo->GetSubsectionsCount ();
}

// vr�t� jm�no podsekce (nSubsectionIndex je index do "pole", tj. od 0)
inline LPCSTR CConfigFileSection::GetSubsectionName ( int nSubsectionIndex ) const 
{
	return m_pConfigFileSectionInfo->GetSubsectionName ( nSubsectionIndex );
}

// vr�t� po�et prom�nn�ch sekce
inline int CConfigFileSection::GetVariablesCount () const 
{
	return m_pConfigFileSectionInfo->GetVariablesCount ();
}

// vr�t� informace o prom�nn� sekce (nVariableIndex je index do "pole", tj. od 0)
inline struct CConfigFileSection::SVariableInfo CConfigFileSection::GetVariableInfo ( int nVariableIndex ) const 
{
	return m_pConfigFileSectionInfo->GetVariableInfo ( nVariableIndex );
}

// vr�t� p��znak existence podsekce v sekci
inline BOOL CConfigFileSection::IsSubsection ( LPCSTR lpszSubsectionName ) const 
{
	return m_pConfigFileSectionInfo->IsSubsection ( lpszSubsectionName );
}

// vr�t� p��znak existence prom�nn� v sekci
inline BOOL CConfigFileSection::IsVariable ( LPCSTR lpszVariableName ) const 
{
	return m_pConfigFileSectionInfo->IsVariable ( lpszVariableName );
}

//////////////////////////////////////////////////////////////////////
// �ten� hodnot prom�nn�ch sekce
//////////////////////////////////////////////////////////////////////

// vr�t� �et�zcovou hodnotu prom�nn� sekce
//    v�jimky: CConfigFileException
inline LPCSTR CConfigFileSection::GetString ( LPCSTR lpszVariableName ) 
{
	return m_pConfigFileSectionInfo->GetVariableStringValue ( lpszVariableName );
}

// vr�t� ��selnou hodnotu prom�nn� sekce
//    v�jimky: CConfigFileException
inline long CConfigFileSection::GetInteger ( LPCSTR lpszVariableName ) 
{
	return m_pConfigFileSectionInfo->GetVariableIntegerValue ( lpszVariableName );
};

// vr�t� re�lnou hodnotu prom�nn� sekce
//    v�jimky: CConfigFileException
inline double CConfigFileSection::GetReal ( LPCSTR lpszVariableName ) 
{
	return m_pConfigFileSectionInfo->GetVariableRealValue ( lpszVariableName );
};

// vr�t� logickou hodnotu prom�nn� sekce
//    v�jimky: CConfigFileException
inline BOOL CConfigFileSection::GetBoolean ( LPCSTR lpszVariableName ) 
{
	return m_pConfigFileSectionInfo->GetVariableBooleanValue ( lpszVariableName );
};

#endif //__CONFIG_FILE_SECTION__INLINE_HEADER__
