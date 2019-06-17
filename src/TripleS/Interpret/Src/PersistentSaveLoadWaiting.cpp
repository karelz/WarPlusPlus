// IPersistentSaveLoadWaiting.cpp: implementation of the CIPersistentSaveLoadWaiting class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\Interpret.h"

// Zde jsou funkce pro ukladani trid zdedenych z CIWaitingInterface

void PersistentSaveWaiting( CIWaitingInterface *pWaitingInterface, CPersistentStorage &storage)
{
    BRACE_BLOCK(storage);

    storage << (DWORD)pWaitingInterface->m_eWaitingType;

	switch ( pWaitingInterface->m_eWaitingType)
	{
	case CIWaitingInterface::EVENT_WAITING_1ST:		
		((CISendtoWaiting1st*)pWaitingInterface)->PersistentSave( storage); 
		break;

	case CIWaitingInterface::EVENT_WAITING_ALL:		
		((CISendtoWaitingAll*)pWaitingInterface)->PersistentSave( storage); 
		break;
	
	case CIWaitingInterface::CONSTRUCTOR_WAITING:	
		((CIConstructorFinishedWaiting*)pWaitingInterface)->PersistentSave( storage); 
		break;
	
	case CIWaitingInterface::SYSCALL_WAITING:		
		ASSERT ( FALSE );
		//PersistentSaveSyscall( (CISyscall*)pWaitingInterface, storage); 
		break;
	
	case CIWaitingInterface::ERROR_WAITING:
		ASSERT(false);
	}
}

CIWaitingInterface* PersistentLoadWaiting( CPersistentStorage &storage)
{
    BRACE_BLOCK(storage);

    CIWaitingInterface::EWaitingType eWaitingType;
	CIWaitingInterface *pWaitingInterface = NULL;

	storage >> (DWORD&)eWaitingType;

	switch ( eWaitingType)
	{
	case CIWaitingInterface::EVENT_WAITING_1ST:		
		pWaitingInterface = new CISendtoWaiting1st;
		((CISendtoWaiting1st*)pWaitingInterface)->PersistentLoad( storage); 
		break;

	case CIWaitingInterface::EVENT_WAITING_ALL:		
		pWaitingInterface = new CISendtoWaitingAll;
		((CISendtoWaitingAll*)pWaitingInterface)->PersistentLoad( storage); 
		break;

	case CIWaitingInterface::CONSTRUCTOR_WAITING:	
		pWaitingInterface = new CIConstructorFinishedWaiting;
		((CIConstructorFinishedWaiting*)pWaitingInterface)->PersistentLoad( storage); 
		break;

	case CIWaitingInterface::SYSCALL_WAITING:		
		ASSERT ( FALSE );
		//pWaitingInterface = PersistentLoadSyscall( storage); 
		break;

	case CIWaitingInterface::ERROR_WAITING:
		ASSERT(false);
	}

	return pWaitingInterface;
}

void PersistentTranslatePointersWaiting( CIWaitingInterface *pWaitingInterface, CPersistentStorage &storage)
{
	switch ( pWaitingInterface->m_eWaitingType)
	{
	case CIWaitingInterface::EVENT_WAITING_1ST:		
		((CISendtoWaiting1st*)pWaitingInterface)->PersistentTranslatePointers( storage); 
		break;

	case CIWaitingInterface::EVENT_WAITING_ALL:		
		((CISendtoWaitingAll*)pWaitingInterface)->PersistentTranslatePointers( storage); 
		break;
	
	case CIWaitingInterface::CONSTRUCTOR_WAITING:	
		((CIConstructorFinishedWaiting*)pWaitingInterface)->PersistentTranslatePointers( storage); 
		break;
	
	case CIWaitingInterface::SYSCALL_WAITING:		
		ASSERT ( FALSE );
	//	PersistentTranslatePointersSyscall( (CISyscall*)pWaitingInterface, storage); 
		break;
	
	case CIWaitingInterface::ERROR_WAITING:
		ASSERT(false);
	}
}

void PersistentInitWaiting( CIWaitingInterface *pWaitingInterface)
{
	switch ( pWaitingInterface->m_eWaitingType)
	{
	case CIWaitingInterface::EVENT_WAITING_1ST:		
		((CISendtoWaiting1st*)pWaitingInterface)->PersistentInit( ); 
		break;

	case CIWaitingInterface::EVENT_WAITING_ALL:		
		((CISendtoWaitingAll*)pWaitingInterface)->PersistentInit( ); 
		break;
	
	case CIWaitingInterface::CONSTRUCTOR_WAITING:	
		((CIConstructorFinishedWaiting*)pWaitingInterface)->PersistentInit( ); 
		break;
	
	case CIWaitingInterface::SYSCALL_WAITING:		
		ASSERT ( FALSE );
		//PersistentInitSyscall( (CISyscall*)pWaitingInterface); 
		break;
	
	case CIWaitingInterface::ERROR_WAITING:
		ASSERT(false);
	}
}
