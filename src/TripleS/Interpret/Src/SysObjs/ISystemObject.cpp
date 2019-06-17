// ISystemObject.cpp: implementation of the CISystemObject class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\..\Interpret.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CIObjUnitStruct * CISystemObject::AddRef()    
{ 
    ::InterlockedIncrement(&m_nRefCount); 
    return this; 
}

CIObjUnitStruct * CISystemObject::Release()   
{ 
    if (::InterlockedDecrement(&m_nRefCount) == 0) 
    {
		m_pInterpret->UnregisterSystemObject( this);
        Delete();
        delete this; 
    }
    return NULL;
}

//////////////////////////////////////////////////////////////////////
// SAVE & LOAD
//

void CISystemObject::PersistentSave( CPersistentStorage &storage)
{
    BRACE_BLOCK(storage);

    CIObjUnitStruct::PersistentSave( storage);

	storage << (DWORD)m_pInterpret; 
    storage << (DWORD)m_nSOType;
	storage << (BYTE)m_bCanBeDuplicated;	
}

//////////////////////////////////////////////////////////////////////

void CISystemObject::PersistentLoad( CPersistentStorage &storage)
{
    BRACE_BLOCK(storage);

    CIObjUnitStruct::PersistentLoad( storage);

	storage >> (DWORD&)m_pInterpret; 
    storage >> (DWORD&)m_nSOType;
	storage >> (BYTE&)m_bCanBeDuplicated;	
}

//////////////////////////////////////////////////////////////////////

void CISystemObject::PersistentTranslatePointers( CPersistentStorage &storage)
{
	CIObjUnitStruct::PersistentTranslatePointers( storage);

	m_pInterpret = (CInterpret*) storage.TranslatePointer( m_pInterpret); 
}

//////////////////////////////////////////////////////////////////////

void CISystemObject::PersistentInit()
{
	CIObjUnitStruct::PersistentInit();
}

//////////////////////////////////////////////////////////////////////