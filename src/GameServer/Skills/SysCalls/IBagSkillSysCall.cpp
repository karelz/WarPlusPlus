/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Interpret
 *   Autor: Karel Zikmund
 * 
 *   Popis: Tøída systémového volání na skille s návratovou hodnotou
 * 
 ***********************************************************/

#include "StdAfx.h"

#include "IBagSkillSysCall.h"

IMPLEMENT_DYNAMIC ( CIBagSkillSysCall, CISkillSysCall )

//////////////////////////////////////////////////////////////////////
// Operace se systémovým voláním
//////////////////////////////////////////////////////////////////////

// vytvoøení systémového volání na skille typu "pSkillType" jednotky "pUnit" s parametry 
//		"pBag" a s volajícím "pProcess" (zvyšuje poèet odkazù na objekt)
bool CIBagSkillSysCall::Create ( CIProcess *pProcess, CSUnit *pUnit, 
	CSSkillType *pSkillType, CIBag *pBag ) 
{
	ASSERT ( m_pBag == NULL );
	ASSERT ( pBag != NULL );

	// nechá vytvoøit pøedka
	VERIFY ( CISkillSysCall::Create ( pProcess, pUnit, pSkillType ) );

	// uschová si parametry systémového volání
	m_pBag = pBag;

	return true;
}

// zrušení systémového volání volajícím (zámek skilly=zámek jednotky musí být zamèen pro 
//		zápis) (ukazatele na jednotku a na bag parametrù dále nejsou platné)
void CIBagSkillSysCall::Cancel ( CIProcess *pProcess ) 
{
	ASSERT ( m_pBag != NULL );

	// znehodnotí parametry systémového volání
	m_pBag = NULL;

	// zrušení systémového volání volajícím (zámek skilly=zámek jednotky musí být zamèen 
	//		pro zápis) (ukazatel na jednotku dále není platný)
	CISkillSysCall::Cancel ( pProcess );
}

//////////////////////////////////////////////////////////////////////
// Ukládání dat (CPersistentObject metody)
//////////////////////////////////////////////////////////////////////

/*
CIBagSkillSysCall stored

	CISkillSysCall stored
	CIBag *m_pBag
*/

// ukládání dat
void CIBagSkillSysCall::PersistentSave ( CPersistentStorage &storage ) 
{
	BRACE_BLOCK ( storage );

	CISkillSysCall::PersistentSave ( storage );

	storage << m_pBag;
}

// nahrávání pouze uložených dat
void CIBagSkillSysCall::PersistentLoad ( CPersistentStorage &storage ) 
{
	BRACE_BLOCK ( storage );

	ASSERT ( m_pBag == NULL );

	CISkillSysCall::PersistentLoad ( storage );

	storage >> (void *&)m_pBag;
}

// pøeklad ukazatelù
void CIBagSkillSysCall::PersistentTranslatePointers ( CPersistentStorage &storage ) 
{
	CISkillSysCall::PersistentTranslatePointers ( storage );

	m_pBag = (CIBag *)storage.TranslatePointer ( m_pBag );
}

// inicializace nahraného objektu
void CIBagSkillSysCall::PersistentInit () 
{
	CISkillSysCall::PersistentInit ();
}
