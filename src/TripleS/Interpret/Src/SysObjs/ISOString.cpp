// ISOString.cpp: implementation of the CISOString class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "../../Interpret.h"
#include "ISOString.h"

//////////////////////
// Metody
//////////////////////


BEGIN_SYSCALL_INTERFACE_MAP(CISOString)
	SYSCALL_METHOD(M_GetLength),
	SYSCALL_METHOD(M_Mid),
	SYSCALL_METHOD(M_Left),
	SYSCALL_METHOD(M_Right),
	SYSCALL_METHOD(M_MakeLower),
	SYSCALL_METHOD(M_MakeUpper),
	SYSCALL_METHOD(M_Find),
	SYSCALL_METHOD(M_FindFrom)
END_SYSCALL_INTERFACE_MAP()


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CISOString::CISOString()
    : CIDuplicSystemObject()
{
    m_nSOType = SO_STRING;
}

//////////////////////////////////////////////////////////////////////

CISOString::~CISOString()
{
}

//////////////////////////////////////////////////////////////////////

bool CISOString::Create( CInterpret *pInterpret)
{
	if (!CISystemObject::Create( pInterpret)) return false;
    
    m_strString.Empty();
    
    return true;
}

//////////////////////////////////////////////////////////////////////

void CISOString::Delete()
{
    m_strString.Empty();
}

//////////////////////////////////////////////////////////////////////

bool CISOString::CreateFrom(CInterpret *pInterpret, CIDuplicSystemObject *pSrc)
{
	if (!CISystemObject::Create( pInterpret)) return false;
    
	ASSERT( pSrc->GetObjectType() == SO_STRING);

	m_strString = ((CISOString*)pSrc)->m_strString;

	return true;
}

//////////////////////////////////////////////////////////////////////
// 0: int GetLength();
ESyscallResult CISOString::M_GetLength( CIProcess*, CISyscall**, CIDataStackItem *pRet)
{
    pRet->Set((int)m_strString.GetLength());
    return SYSCALL_SUCCESS;
}

//////////////////////////////////////////////////////////////////////
// 1: String Mid(int first, int count);
ESyscallResult CISOString::M_Mid( CIProcess* pProcess, CISyscall**, CIDataStackItem *pRet, int *pPos, int *pCount)
{
    // vytvoreni navratoveho stringu
    CISOString *pString;
    
    if ((pString = (CISOString*)pProcess->m_pInterpret->CreateSystemObject(SO_STRING)) == NULL) return SYSCALL_ERROR;

    // naplneni navratove hodnoty
    if ( *pPos >= m_strString.GetLength()) pString->Set("");
    else pString->Set( m_strString.Mid( *pPos, *pCount) );

    pRet->Set( pString);

    pString->Release();
    return SYSCALL_SUCCESS;
}

//////////////////////////////////////////////////////////////////////
// 2: String Left(int count);		
ESyscallResult CISOString::M_Left( CIProcess* pProcess, CISyscall**, CIDataStackItem *pRet, int *pCount)
{
    // vytvoreni navratoveho stringu
    CISOString *pString;
    
    if ((pString = (CISOString*)pProcess->m_pInterpret->CreateSystemObject(SO_STRING)) == NULL) return SYSCALL_ERROR;

    // naplneni navratove hodnoty
    pString->Set( m_strString.Left( *pCount) );

    pRet->Set( pString);

    pString->Release();
    return SYSCALL_SUCCESS;
}

//////////////////////////////////////////////////////////////////////
// 3: String Right(int count);
ESyscallResult CISOString::M_Right( CIProcess* pProcess, CISyscall**, CIDataStackItem *pRet, int *pCount)
{
    // vytvoreni navratoveho stringu
    CISOString *pString;
    
    if ((pString = (CISOString*)pProcess->m_pInterpret->CreateSystemObject(SO_STRING)) == NULL) return SYSCALL_ERROR;

    // naplneni navratove hodnoty
    pString->Set( m_strString.Right( *pCount) );

    pRet->Set( pString);

    pString->Release();
    return SYSCALL_SUCCESS;
}

//////////////////////////////////////////////////////////////////////
// 4: void MakeLower();
ESyscallResult CISOString::M_MakeLower( CIProcess*, CISyscall**)
{
    m_strString.MakeLower();
    return SYSCALL_SUCCESS;
}

//////////////////////////////////////////////////////////////////////
// 5: void MakeUpper();
ESyscallResult CISOString::M_MakeUpper( CIProcess*, CISyscall**)
{
    m_strString.MakeUpper();
    return SYSCALL_SUCCESS;
}

//////////////////////////////////////////////////////////////////////
// 6: int Find(String substring);
ESyscallResult CISOString::M_Find( CIProcess* pProcess, CISyscall**, CIDataStackItem *pRet, CISOString *pSubstr)
{
    // parametry
    pRet->Set( (int)m_strString.Find( pSubstr->m_strString) );

    return SYSCALL_SUCCESS;
}

//////////////////////////////////////////////////////////////////////
// 7: int FindFrom(String substring, int start);
ESyscallResult CISOString::M_FindFrom( CIProcess*, CISyscall**, CIDataStackItem *pRet, CISOString *pSubstr, int *pStart)
{
    pRet->Set( (int)m_strString.Find( pSubstr->m_strString, *pStart) );
    return SYSCALL_SUCCESS;
}

//////////////////////////////////////////////////////////////////////
// SAVE & LOAD
//

void CISOString::PersistentSave( CPersistentStorage &storage)
{
    BRACE_BLOCK(storage);

    CIDuplicSystemObject::PersistentSave( storage);

	storage << (int)m_strString.GetLength();
	storage.Write( (LPCTSTR)m_strString, m_strString.GetLength());
}

//////////////////////////////////////////////////////////////////////

void CISOString::PersistentLoad( CPersistentStorage &storage)
{
    BRACE_BLOCK(storage);

    CIDuplicSystemObject::PersistentLoad( storage);

	int len;

	storage >> len;

    char *pStr=m_strString.GetBufferSetLength(len+1);
    storage.Read(pStr, len);
    m_strString.ReleaseBuffer(len);
}

//////////////////////////////////////////////////////////////////////

void CISOString::PersistentTranslatePointers( CPersistentStorage &storage)
{
	CIDuplicSystemObject::PersistentTranslatePointers( storage);
}

//////////////////////////////////////////////////////////////////////

void CISOString::PersistentInit()
{
	CIDuplicSystemObject::PersistentInit( );
}
