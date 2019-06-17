// ISOPath.cpp: implementation of the CISOPath class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\..\Interpret.h"
#include "ISOPath.h"
#include "ISOPosition.h"
#include "GameServer\GameServer\SPath.h"
#include "GameServer\Civilization\ZCivilization.h"

//////////////////////////////////////////////////////////////////////

BEGIN_SYSCALL_INTERFACE_MAP(CISOPath)
    SYSCALL_METHOD(M_GetStartPoint),
    SYSCALL_METHOD(M_GetEndPoint),
    SYSCALL_METHOD(M_SwapDirection),
    SYSCALL_METHOD(M_GetDistance),
    SYSCALL_METHOD(M_GetBeeLineDistance),
    SYSCALL_METHOD(M_GetMinWidth),
    SYSCALL_METHOD(M_GetMinDistance),
    SYSCALL_METHOD(M_GetMaxDistance),
    SYSCALL_METHOD(M_Truncate),
    SYSCALL_METHOD(M_TruncateBeeLine),
    SYSCALL_METHOD(M_AppendPath),
END_SYSCALL_INTERFACE_MAP()

//////////////////////////////////////////////////////////////////////

CISOPath::CISOPath()
{
    m_nSOType = SO_PATH;

    m_pPath=NULL;
}

CISOPath::~CISOPath()
{
    ASSERT(m_pPath==NULL);
}

bool CISOPath::Create( CInterpret *pInterpret)
{
	if (!CIDuplicSystemObject::Create( pInterpret)) return false;
    
    m_pPath=NULL;
    return true;
}

void CISOPath::Delete()
{
    if(m_pPath!=NULL) {
        m_pPath->Destroy();
        m_pPath=NULL;
    }
}

void CISOPath::Set(CSPath *pPath)
{
    ASSERT(pPath!=NULL);
    
    m_pPath=pPath;
}

bool CISOPath::LoadMember( CIDataStackItem& DSI, long nID)
{
    // Zatim zadne member polozky

    return false;
}

bool CISOPath::StoreMember( CIDataStackItem *pDSI, long nID)
{
    // Zadne member polozky

    return false;
}


bool CISOPath::CreateFrom(CInterpret *pInterpret, CIDuplicSystemObject *pSrc)
{
	ASSERT( pSrc->GetObjectType() == SO_PATH);
    if (!CIDuplicSystemObject::Create( pInterpret)) return false;

    // TODO: Kopie dat
	return true;
}

ESyscallResult CISOPath::M_GetStartPoint( CIProcess *pProcess, CISyscall **pSyscall, CIDataStackItem *pRet)
{
    CISOPosition *pPosition;

    if ((pPosition = (CISOPosition*)pProcess->m_pInterpret->CreateSystemObject(SO_POSITION)) == NULL)
        return SYSCALL_ERROR;
    
    pRet->Set(pPosition);
    pPosition->Release();

    return SYSCALL_SUCCESS;
}

ESyscallResult CISOPath::M_GetEndPoint( CIProcess *pProcess, CISyscall **pSyscall, CIDataStackItem *pRet)
{
    CISOPosition *pPosition;

    if ((pPosition = (CISOPosition*)pProcess->m_pInterpret->CreateSystemObject(SO_POSITION)) == NULL)
        return SYSCALL_ERROR;
    
    pRet->Set(pPosition);
    pPosition->Release();

    return SYSCALL_SUCCESS;
}

ESyscallResult CISOPath::M_SwapDirection( CIProcess*, CISyscall **pSyscall)
{
    return SYSCALL_SUCCESS;
}

ESyscallResult CISOPath::M_GetDistance( CIProcess*, CISyscall **pSyscall, CIDataStackItem *pRet)
{
    pRet->Set(0);
    return SYSCALL_SUCCESS;
}

ESyscallResult CISOPath::M_GetBeeLineDistance( CIProcess*, CISyscall **pSyscall, CIDataStackItem *pRet)
{
    pRet->Set(0);
    return SYSCALL_SUCCESS;
}

ESyscallResult CISOPath::M_GetMinWidth( CIProcess*, CISyscall **pSyscall, CIDataStackItem *pRet)
{
    pRet->Set(0);
    return SYSCALL_SUCCESS;
}

ESyscallResult CISOPath::M_GetMinDistance( CIProcess*, CISyscall **pSyscall, CIDataStackItem *pRet, CISystemObject *pPos)
{
    pRet->Set(0);
    return SYSCALL_SUCCESS;
}

ESyscallResult CISOPath::M_GetMaxDistance( CIProcess*, CISyscall **pSyscall, CIDataStackItem *pRet, CISystemObject *pPos)
{
    pRet->Set(0);
    return SYSCALL_SUCCESS;
}

ESyscallResult CISOPath::M_Truncate( CIProcess*, CISyscall **pSyscall, int *pDistance)
{
    return SYSCALL_SUCCESS;
}

ESyscallResult CISOPath::M_TruncateBeeLine( CIProcess*, CISyscall **pSyscall, int *pDistance)
{
    return SYSCALL_SUCCESS;
}

ESyscallResult CISOPath::M_AppendPath( CIProcess*, CISyscall **pSyscall, CISystemObject *pPos)
{
    return SYSCALL_SUCCESS;
}

//////////////////////////////////////////////////////////////////////
// SAVE & LOAD
//

/*
CISOPath stored

	CIDuplicSystemObject stored
	m_pPath
	if ( m_pPath != NULL )
		CSPath stored
*/

void CISOPath::PersistentSave( CPersistentStorage &storage)
{
    BRACE_BLOCK(storage);

    // uloží pøedka
	CIDuplicSystemObject::PersistentSave ( storage );

	// uloží ukazatel na cestu
	storage << m_pPath;

	// zjistí, je-li cesta platná
	if ( m_pPath != NULL )
	{	// cesta je platná
		// uloží cestu
		m_pPath->PersistentSave ( storage );
	}
}

//////////////////////////////////////////////////////////////////////

void CISOPath::PersistentLoad( CPersistentStorage &storage)
{
    BRACE_BLOCK(storage);

    // naète pøedka
	CIDuplicSystemObject::PersistentLoad ( storage );

	// naète starý ukazatel na cestu
	void *pOldPath;
	storage >> pOldPath;

	// zjistí, je-li cesta platná
	if ( pOldPath != NULL )
	{	// cesta je platná
		// naète cestu
		m_pPath = CSPath::PersistentLoadPath ( storage );
		// zaregistruje ukazatel na cestu
		storage.RegisterPointer ( pOldPath, m_pPath );
	}
	else
	{	// cesta je neplatná
		// aktualizuje ukazatel na cestu
		m_pPath = NULL;
	}
}

//////////////////////////////////////////////////////////////////////

void CISOPath::PersistentTranslatePointers( CPersistentStorage &storage)
{
	// pøeloží ukazatele pøedka
	CIDuplicSystemObject::PersistentTranslatePointers ( storage );

	// zjistí, je-li cesta platná
	if ( m_pPath != NULL )
	{	// cesta je platná
		// pøeloží ukazatele cesty
		m_pPath->PersistentTranslatePointers ( storage );
	}

	// Momentalne neni potreba prekladat zadne pointery,
	// protoze jediny ukazatel, ktery mame, si vytvarime sami
}

//////////////////////////////////////////////////////////////////////

void CISOPath::PersistentInit()
{
	// inicializuje pøedka
	CIDuplicSystemObject::PersistentInit ();

	// zjistí, je-li cesta platná
	if ( m_pPath != NULL )
	{	// cesta je platná
		// inicializuje cestu
		m_pPath->PersistentInit ();
	}
}
