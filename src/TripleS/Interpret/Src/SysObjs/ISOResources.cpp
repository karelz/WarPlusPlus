// ISOResources.cpp: implementation of the CISOResources class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\..\Interpret.h"
#include "ISOResources.h"

//////////////////////////////////////////////////////////////////////

BEGIN_SYSCALL_INTERFACE_MAP(CISOResources)
SYSCALL_METHOD(M_Empty),
SYSCALL_METHOD(M_LessThan),
SYSCALL_METHOD(M_LessEqual),
SYSCALL_METHOD(M_MoreThan),
SYSCALL_METHOD(M_MoreEqual),
SYSCALL_METHOD(M_Equal),
SYSCALL_METHOD(M_GetResource),
SYSCALL_METHOD(M_SetResource),
SYSCALL_METHOD(M_GetNumResources),
SYSCALL_METHOD(M_IsEmpty),
END_SYSCALL_INTERFACE_MAP()

//////////////////////////////////////////////////////////////////////

CISOResources::CISOResources()
{
    m_nSOType = SO_RESOURCES;
}

CISOResources::~CISOResources()
{
}

bool CISOResources::Create( CInterpret *pInterpret)
{
    if (!CIDuplicSystemObject::Create( pInterpret)) return false;
    
    for(int i=0; i<RESOURCE_COUNT; i++) m_Resources[i]=0;
    
    return true;
}

void CISOResources::Delete()
{
    for(int i=0; i<RESOURCE_COUNT; i++) m_Resources[i]=0;
}

void CISOResources::Set(TSResources &Resources)
{
    for(int i=0; i<RESOURCE_COUNT; i++) m_Resources[i]=Resources[i];
}

bool CISOResources::LoadMember( CIDataStackItem& DSI, long nID)
{
    return false;
}

bool CISOResources::StoreMember( CIDataStackItem *pDSI, long nID)
{
    return false;
}


bool CISOResources::CreateFrom(CInterpret *pInterpret, CIDuplicSystemObject *pSrc)
{
    if (!CIDuplicSystemObject::Create( pInterpret)) return false;
    
    ASSERT( pSrc->GetObjectType() == SO_RESOURCES);
    
    ::memcpy( &m_Resources, ((CISOResources*)pSrc)->m_Resources, sizeof(m_Resources));
    
    return true;
}

//////////////////////////////////////////////////////////////////////
// Metody

ESyscallResult CISOResources::M_Empty(CIProcess *pProcess, CISyscall **pSyscall) 
{
    for(int i=0; i<RESOURCE_COUNT; i++) m_Resources[i]=0;

    return SYSCALL_SUCCESS;
}

ESyscallResult CISOResources::M_LessThan(CIProcess *pProcess, CISyscall **pSyscall, CIDataStackItem *pRet, CISystemObject *pSystemObject) 
{  
    CISOResources *pRes = (CISOResources*)pSystemObject;
    bool bRet = true;
    
    for(int i=0; i<RESOURCE_COUNT; i++) bRet = (bRet && (m_Resources[i] < pRes->m_Resources[i]));
    
    pRet->Set(bRet);
    
    return SYSCALL_SUCCESS;
}

ESyscallResult CISOResources::M_LessEqual(CIProcess *pProcess, CISyscall **pSyscall, CIDataStackItem *pRet, CISystemObject *pSystemObject) 
{  
    CISOResources *pRes = (CISOResources*)pSystemObject;
    bool bRet = true;
    
    for(int i=0; i<RESOURCE_COUNT; i++) bRet = (bRet && (m_Resources[i] <= pRes->m_Resources[i]));
    
    pRet->Set(bRet);
    
    return SYSCALL_SUCCESS;
}

ESyscallResult CISOResources::M_MoreThan(CIProcess *pProcess, CISyscall **pSyscall, CIDataStackItem *pRet, CISystemObject *pSystemObject) 
{  
    CISOResources *pRes = (CISOResources*)pSystemObject;
    bool bRet = true;
    
    for(int i=0; i<RESOURCE_COUNT; i++) bRet = bRet && (m_Resources[i] > pRes->m_Resources[i]);
    
    pRet->Set(bRet);
    
    return SYSCALL_SUCCESS;
}

ESyscallResult CISOResources::M_MoreEqual(CIProcess *pProcess, CISyscall **pSyscall, CIDataStackItem *pRet, CISystemObject *pSystemObject) 
{  
    CISOResources *pRes = (CISOResources*)pSystemObject;
    bool bRet = true;
    
    for(int i=0; i<RESOURCE_COUNT; i++) bRet = (bRet && (m_Resources[i] >= pRes->m_Resources[i]));
    
    pRet->Set(bRet);
    
    return SYSCALL_SUCCESS;
}

ESyscallResult CISOResources::M_Equal(CIProcess *pProcess, CISyscall **pSyscall, CIDataStackItem *pRet, CISystemObject *pSystemObject) 
{  
    CISOResources *pRes = (CISOResources*)pSystemObject;
    bool bRet = true;
    
    for(int i=0; i<RESOURCE_COUNT; i++) bRet = (bRet && (m_Resources[i] == pRes->m_Resources[i]));
    
    pRet->Set(bRet);
    
    return SYSCALL_SUCCESS;
}

ESyscallResult CISOResources::M_GetResource(CIProcess *pProcess, CISyscall **pSyscall, CIDataStackItem *pRet, int *pID) 
{  
    if(*pID>0 && *pID < RESOURCE_USED_COUNT) 
    {
        pRet->Set(m_Resources[*pID]);
        return SYSCALL_SUCCESS;
    } 
    else 
    {
        CString str;
        str.Format("Wrong resource number: %d", *pID);
        pProcess->m_pInterpret->ReportError(str);
        return SYSCALL_ERROR;
    }
}

ESyscallResult CISOResources::M_SetResource(CIProcess *pProcess, CISyscall **pSyscall, int *pID, int *pValue) 
{  
    CString str;
    if(*pID>0 && *pID < RESOURCE_USED_COUNT) 
    {
        if(*pValue>=0) 
        {
            m_Resources[*pID]=*pValue;
            return SYSCALL_SUCCESS;
        } 
        else 
        {
            str.Format("Resource vector can not contain negative value: %d", *pValue);
            pProcess->m_pInterpret->ReportError(str);
            return SYSCALL_ERROR;
        }
    } 
    else 
    {
        str.Format("Wrong resource number: %d", *pID);
        pProcess->m_pInterpret->ReportError(str);
        return SYSCALL_ERROR;
    }
}

ESyscallResult CISOResources::M_GetNumResources(CIProcess *pProcess, CISyscall **pSyscall, CIDataStackItem *pRet) 
{  
    pRet->Set(RESOURCE_USED_COUNT);
    return SYSCALL_SUCCESS;
}

ESyscallResult CISOResources::M_IsEmpty(CIProcess *pProcess, CISyscall **pSyscall, CIDataStackItem *pRet) 
{  
    bool bRet = true;
    
    for(int i=0; i<RESOURCE_COUNT; i++) bRet = (bRet && (m_Resources[i] == 0));
    
    pRet->Set(bRet);
    
    return SYSCALL_SUCCESS;
}

//////////////////////////////////////////////////////////////////////
// SAVE & LOAD
//

void CISOResources::PersistentSave( CPersistentStorage &storage)
{
    BRACE_BLOCK(storage);
    
    CIDuplicSystemObject::PersistentSave( storage);
    
    storage.Write(&m_Resources, sizeof(m_Resources));
}

//////////////////////////////////////////////////////////////////////

void CISOResources::PersistentLoad( CPersistentStorage &storage)
{
    BRACE_BLOCK(storage);
    
    CIDuplicSystemObject::PersistentLoad( storage);
    
    storage.Read(&m_Resources, sizeof(m_Resources));
}

//////////////////////////////////////////////////////////////////////

void CISOResources::PersistentTranslatePointers( CPersistentStorage &storage)
{
    CIDuplicSystemObject::PersistentTranslatePointers( storage);
}

//////////////////////////////////////////////////////////////////////

void CISOResources::PersistentInit()
{
    CIDuplicSystemObject::PersistentInit( );
}

