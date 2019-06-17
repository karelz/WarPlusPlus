// ISyscallSome.cpp: implementation of the CISyscallSome class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\..\Interpret.h"

#include "ISyscallSome.h"

//////////////////////////////////////////////////////////////////////

UINT DoStWaiting( LPVOID wParam)
{
    Sleep(1000);

    ((CISyscallSome*)wParam)->FinishWaiting( STATUS_SUCCESS);

    return 0;
}

bool CISyscallSome::Create( CIProcess* pProcess, CIBag *pBag)
{
    if (!CISyscall::Create( pProcess)) return false;

    pBag->GetReturnRegister()->Set(12345);
    ::AfxBeginThread( DoStWaiting,  this);

    return true;
}

void CISyscallSome::PersistentSave( CPersistentStorage &storage)
{
    BRACE_BLOCK(storage);

    CIWaitingInterface::PersistentSave( storage);
}

void CISyscallSome::PersistentLoad( CPersistentStorage &storage)
{
    BRACE_BLOCK(storage);

    CIWaitingInterface::PersistentLoad( storage);
}

void CISyscallSome::PersistentTranslatePointers( CPersistentStorage &storage)
{
	CIWaitingInterface::PersistentTranslatePointers( storage);
}

void CISyscallSome::PersistentInit()
{
	CIWaitingInterface::PersistentInit();
}
