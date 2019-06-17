/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Interpret
 *   Autor: Helena Kupková
 *  
 *   Popis: Implementace objektu CIObjUnitStruct, predka objektu
 *          CIObject, CZUnit a CIStructure, ktere reprezentujici instance
 *          objektu, jednotek a struktur.
 * 
 ***********************************************************/

#include "stdafx.h"
#include "..\Interpret.h"

IMPLEMENT_DYNAMIC(CIObjUnitStruct, CPersistentObject);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CIObjUnitStruct ::CIObjUnitStruct ()
{
    m_nRefCount = 0;
}

CIObjUnitStruct ::~CIObjUnitStruct ()
{
    ASSERT(m_nRefCount == 0);
}

#ifdef _DEBUG
void CIObjUnitStruct ::AssertValid() const
{
    CPersistentObject::AssertValid();
}
#endif

//
//	SAVE & LOAD
//

void CIObjUnitStruct::PersistentSave( CPersistentStorage &storage)
{
    BRACE_BLOCK(storage);

    // pointer na sebe
	storage << (DWORD)this;

	// pocitadlo odkazu
    storage << m_nRefCount;
}

void CIObjUnitStruct::PersistentLoad( CPersistentStorage &storage)
{
    BRACE_BLOCK(storage);
	
    // pointer na sebe
    CIObjUnitStruct *pThis;
	storage >> (DWORD&)pThis;
	storage.RegisterPointer( pThis, this);

	// pocitadlo odkazu
    storage >> m_nRefCount;
}

void CIObjUnitStruct::PersistentTranslatePointers( CPersistentStorage &storage)
{
}

void CIObjUnitStruct::PersistentInit()
{
}


