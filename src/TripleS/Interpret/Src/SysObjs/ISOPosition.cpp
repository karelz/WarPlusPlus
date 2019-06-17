// ISOPosition.cpp: implementation of the CISOPosition class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\..\Interpret.h"
#include "ISOPosition.h"

#include "GameServer\GameServer\SMap.h"

#include <math.h>

//////////////////////////////////////////////////////////////////////

BEGIN_SYSCALL_INTERFACE_MAP(CISOPosition)
    SYSCALL_METHOD(M_GetDistancePos),
    SYSCALL_METHOD(M_GetDistance),
    SYSCALL_METHOD(M_IsOnMap)
END_SYSCALL_INTERFACE_MAP()

//////////////////////////////////////////////////////////////////////

CISOPosition::CISOPosition()
{
    m_nSOType = SO_POSITION;
}

CISOPosition::~CISOPosition()
{
}

bool CISOPosition::Create( CInterpret *pInterpret)
{
	if (!CIDuplicSystemObject::Create( pInterpret)) return false;
    
    ZeroMemory( m_nXYZ, sizeof(int)*3);
    return true;
}

void CISOPosition::Delete()
{
    ZeroMemory( m_nXYZ, sizeof(int)*3);
}

void CISOPosition::Set(CPointDW *pPoint, int Z)
{
    m_nXYZ[0] = pPoint->x;
    m_nXYZ[1] = pPoint->y;
    m_nXYZ[2] = Z;
}

void CISOPosition::Set(int X, int Y, int Z)
{
    m_nXYZ[0] = X;
    m_nXYZ[1] = Y;
    m_nXYZ[2] = Z;
}

void CISOPosition::Set(CSPosition *pPos)
{
    m_nXYZ[0] = pPos->x;
    m_nXYZ[1] = pPos->y;
    m_nXYZ[2] = pPos->z;
}

bool CISOPosition::LoadMember( CIDataStackItem& DSI, long nID)
{
    if ( nID < 103 && nID >= 100) DSI.Set( m_nXYZ[nID-100]);
    else 
    {
        // bad ID
        ASSERT(false);
        return false;
    }
    return true;
}

bool CISOPosition::StoreMember( CIDataStackItem *pDSI, long nID)
{
	ASSERT( pDSI != NULL);

    if ( nID < 103 && nID >= 100) m_nXYZ[nID - 100] = pDSI->GetInt();
    else 
    {
        ASSERT(false);
        return false;
    }
    return true;
}


bool CISOPosition::CreateFrom(CInterpret *pInterpret, CIDuplicSystemObject *pSrc)
{
    if (!CIDuplicSystemObject::Create( pInterpret)) return false;
    
	ASSERT( pSrc->GetObjectType() == SO_POSITION);

    ::memcpy( m_nXYZ, ((CISOPosition*)pSrc)->m_nXYZ, 3*sizeof(int));

	return true;
}


ESyscallResult CISOPosition::M_GetDistancePos( CIProcess*, CISyscall **pSyscall, CIDataStackItem *pRet,
                                    CISystemObject *pSystemObject)
{
    CISOPosition *pPos = (CISOPosition*)pSystemObject;

    if ( g_cMap.IsMapPosition( CPointDW(  m_nXYZ[0], m_nXYZ[1])) &&
         g_cMap.IsMapPosition( CPointDW( pPos->m_nXYZ[0], pPos->m_nXYZ[1])))
    {
        pRet->Set( (int)ceil(sqrt( (m_nXYZ[0] - pPos->m_nXYZ[0]) * (m_nXYZ[0] - pPos->m_nXYZ[0]) +
                                 (m_nXYZ[1] - pPos->m_nXYZ[1]) * (m_nXYZ[1] - pPos->m_nXYZ[1]) ) ));
    }
    else
    {
        pRet->Set( -1);
    }

    return SYSCALL_SUCCESS;
}

ESyscallResult CISOPosition::M_GetDistance( CIProcess*, CISyscall **pSyscall, CIDataStackItem *pRet,
                                int *pX, int *pY)
{
    pRet->Set( (int)ceil( sqrt( (m_nXYZ[0] - *pX) * (m_nXYZ[0] - *pX) +
                              (m_nXYZ[1] - *pY) * (m_nXYZ[1] - *pY) ) ) );

    return SYSCALL_SUCCESS;
}

ESyscallResult CISOPosition::M_IsOnMap(CIProcess*, CISyscall **pSyscall, CIDataStackItem *pRet)
{
    pRet->Set( g_cMap.IsMapPosition( CPointDW( m_nXYZ[0], m_nXYZ[1])) != 0 );
    return SYSCALL_SUCCESS;
}

//////////////////////////////////////////////////////////////////////
// SAVE & LOAD
//

void CISOPosition::PersistentSave( CPersistentStorage &storage)
{
    BRACE_BLOCK(storage);

    CIDuplicSystemObject::PersistentSave( storage);

	storage.Write( m_nXYZ, sizeof(int)*3);
}

//////////////////////////////////////////////////////////////////////

void CISOPosition::PersistentLoad( CPersistentStorage &storage)
{
    BRACE_BLOCK(storage);

    CIDuplicSystemObject::PersistentLoad( storage);

    storage.Read( m_nXYZ, sizeof(int)*3);
}

//////////////////////////////////////////////////////////////////////

void CISOPosition::PersistentTranslatePointers( CPersistentStorage &storage)
{
	CIDuplicSystemObject::PersistentTranslatePointers( storage);
}

//////////////////////////////////////////////////////////////////////

void CISOPosition::PersistentInit()
{
	CIDuplicSystemObject::PersistentInit( );
}

