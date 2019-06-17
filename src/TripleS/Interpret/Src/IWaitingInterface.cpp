// IWaitingInterface.cpp: implementation of the CIWaitingInterface class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\Interpret.h"

IMPLEMENT_DYNAMIC ( CIWaitingInterface, CNotifier )

//////////////////////////////////////////////////////////////////////

bool CIWaitingInterface::Create()
{
	VERIFY ( CNotifier::Create () );
	AddRef();
	return true;
}

//////////////////////////////////////////////////////////////////////

void CIWaitingInterface::Delete()
{
	ASSERT( m_nRefCount == 0);
    CNotifier::Delete();
}

//////////////////////////////////////////////////////////////////////

CIWaitingInterface* CIWaitingInterface::AddRef()
{
	::InterlockedIncrement( &m_nRefCount);
	return this;
}

//////////////////////////////////////////////////////////////////////

CIWaitingInterface* CIWaitingInterface::Release()
{
#ifdef _DEBUG
	if ( m_nRefCount <= 0) __asm{ int 3 };
#endif

	if ( ::InterlockedDecrement( &m_nRefCount) == 0)
    {
        Delete();
        delete this;
    } 
    
	return NULL;
}

//
// SAVE & LOAD
//
void CIWaitingInterface::PersistentSave( CPersistentStorage &storage)
{
    BRACE_BLOCK(storage);

    // pointer na sebe
	storage << (DWORD)this;

	// notifier
	CNotifier::PersistentSave(storage);
	
	// data
	storage << (DWORD)m_eWaitingType;
	storage << m_nRefCount;
}

void CIWaitingInterface::PersistentLoad( CPersistentStorage &storage)
{
    BRACE_BLOCK(storage);

    // pointer na sebe
	CIWaitingInterface *pThis;
	storage >> (DWORD&)pThis;
	storage.RegisterPointer(pThis, this);

	// notifier
	CNotifier::PersistentLoad( storage);
	
	// data
	storage >> (DWORD&)m_eWaitingType;
	storage >> m_nRefCount;
}
void CIWaitingInterface::PersistentTranslatePointers( CPersistentStorage &storage)
{
	// notifier
	CNotifier::PersistentTranslatePointers( storage);
}

void CIWaitingInterface::PersistentInit()
{
	// notifier
	CNotifier::PersistentInit();
}

