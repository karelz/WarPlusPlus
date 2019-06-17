// CUnitSurfaceManager.h: interface for the CCUnitSurfaceManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CUNITSURFACEMANAGER_H__BBF76684_78D8_11D4_80CF_0000B4A08F9A__INCLUDED_)
#define AFX_CUNITSURFACEMANAGER_H__BBF76684_78D8_11D4_80CF_0000B4A08F9A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "CUnitSurface.h"

class CCUnitSurfaceManager  
{
// Konstrukce a destrukce
public:
    // Konstruktor
	CCUnitSurfaceManager();
    
    // Destruktor
    virtual ~CCUnitSurfaceManager();

// Vytvareni a ruseni
public:
    // Vytvoreni
    void Create();

    // Zruseni 
    void Delete();

// Operace s CCUnitSurfacy
public:
    // Vrati surface ze zadaneho souboru o dane barve
    CCUnitSurface *GetSurface(CArchiveFile file, CCUnitSurface::tagSColoringParams *pColoringParams,
      int nXOffset, int nYOffset, DWORD dwFrameDelay );

    // Odstrani jiz nepouzivany surface
    void Release(CCUnitSurface *pSurface);

// Pomocne struktury
private:
    // Drzak na surface
    struct SUnitSurfaceHandle {
    // Metody
    public:
        // Konstruktor
        SUnitSurfaceHandle() { 
            m_dwDestColor=0; m_strFileName=""; m_pUnitSurface=NULL; m_pNextHandle=NULL; 
        }

        // Konstruktor
        SUnitSurfaceHandle(DWORD dwDestColor, CString strFileName, CCUnitSurface *pUnitSurface=NULL) {
            m_dwDestColor=dwDestColor; 
            m_strFileName=strFileName; 
            m_pUnitSurface=pUnitSurface;
            m_pNextHandle=NULL; 
        }

    // Klic
    public:
        // Barva pro obarveni surface
        DWORD m_dwDestColor;

        // Jmeno souboru se surfacem
        CString m_strFileName;
    
    // Data
    public:
        // Ukazatel na surface
        CCUnitSurface *m_pUnitSurface;

    // Vazby do spojaku
    public:
        SUnitSurfaceHandle *m_pNextHandle;
    };

// Data
private:
    // Hlava spojaku drziciho seznam surfacu
    SUnitSurfaceHandle *m_pHead;

    // Zamek na spojak
    CMutex m_lockData;
};

#endif // !defined(AFX_CUNITSURFACEMANAGER_H__BBF76684_78D8_11D4_80CF_0000B4A08F9A__INCLUDED_)
