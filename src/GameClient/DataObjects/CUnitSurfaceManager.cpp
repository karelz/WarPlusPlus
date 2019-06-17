// CUnitSurfaceManager.cpp: implementation of the CCUnitSurfaceManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CUnitSurfaceManager.h"

CCUnitSurfaceManager::CCUnitSurfaceManager()
{
    m_pHead=NULL;
}

CCUnitSurfaceManager::~CCUnitSurfaceManager()
{
    ASSERT(m_pHead==NULL);
}

void CCUnitSurfaceManager::Create()
{
    m_pHead=NULL;
}

void CCUnitSurfaceManager::Delete()
{
    // Je treba, aby pri deletu uz ve spojaku nebyly zadne surfacy
    ASSERT(m_pHead==NULL);
}

CCUnitSurface *CCUnitSurfaceManager::GetSurface(CArchiveFile file, CCUnitSurface::tagSColoringParams *pColoringParams,
                                                int nXOffset, int nYOffset, DWORD dwFrameDelay)
{    
    VERIFY(m_lockData.Lock());

    CCUnitSurface *pSurface=NULL;
    SUnitSurfaceHandle *pHandle=m_pHead;
    CString strFileName=file.GetFileName();
    DWORD dwDestColor = (pColoringParams!=NULL)?pColoringParams->m_dwDestColor:0;

    // Dve vetve, aby se zbytecne netestovala v cyklu nesmyslna podminka
    if(pColoringParams!=NULL) {
        // Mame informace o vybarveni
        while(pHandle!=NULL) {
            if(pHandle->m_dwDestColor==dwDestColor && pHandle->m_strFileName==strFileName) {
                // Nalezli jsme
               pSurface=pHandle->m_pUnitSurface;
               ASSERT(pSurface!=NULL);
               break;
            }        
            pHandle=pHandle->m_pNextHandle;
        }
    } else {
        // Informace o vybarveni nemame
        while(pHandle!=NULL) {
            if(pHandle->m_strFileName==strFileName) {
                // Nalezli jsme
               pSurface=pHandle->m_pUnitSurface;
               ASSERT(pSurface!=NULL);
               break;
            }        
            pHandle=pHandle->m_pNextHandle;
        }
    }

    if(pSurface!=NULL) {
        // Nasli jsme surface
        // Zvysime na nem pocet referenci
        pSurface->AddRef();
    } else {
        // Vytvarime novy surface
        pSurface=new CCUnitSurface();
        pSurface->Create(file, pColoringParams, nXOffset, nYOffset, dwFrameDelay);
        pSurface->AddRef();

        // Vytvorime handle
        SUnitSurfaceHandle *pSurfaceHandle;
        pSurfaceHandle=new SUnitSurfaceHandle(dwDestColor, strFileName, pSurface);

        // A zaclenime ho do spojaku
        pSurfaceHandle->m_pNextHandle=m_pHead;
        m_pHead=pSurfaceHandle;
    }

    VERIFY(m_lockData.Unlock());

    // A vratime nalezeny surface
    return pSurface;
}

void CCUnitSurfaceManager::Release(CCUnitSurface *pSurface)
{
    ASSERT(pSurface);
    
    VERIFY(m_lockData.Lock());    
    
    ASSERT(m_pHead);

    SUnitSurfaceHandle *pHandle, *pPrevHandle;

    pPrevHandle=NULL;
    pHandle=m_pHead;

    while(pHandle) {
        if(pHandle->m_pUnitSurface==pSurface) break;
        pPrevHandle=pHandle;
        pHandle=pHandle->m_pNextHandle;
    }    

    // Surface MUSI byt nalezen
    ASSERT(pHandle!=NULL);
    ASSERT(pHandle->m_pUnitSurface==pSurface);

    if(pHandle==m_pHead) {
        // Jestlize rusime hlavu, je treba ji posunout na dalsi pozici
        m_pHead=pHandle->m_pNextHandle;
    } else {
        // Jinak je potreba premostit ruseny prvek
        pPrevHandle->m_pNextHandle=pHandle->m_pNextHandle;
    }

    ASSERT(pHandle->m_pUnitSurface);
    pHandle->m_pUnitSurface->Release();
    // Jestlize refcount klesl na nulu, rusime surface
    if(pHandle->m_pUnitSurface->GetRefCount()==0) {
        pHandle->m_pUnitSurface->Delete();
        delete pHandle->m_pUnitSurface;
    }

    // A rusime handle
    delete pHandle;

    VERIFY(m_lockData.Unlock());
}
