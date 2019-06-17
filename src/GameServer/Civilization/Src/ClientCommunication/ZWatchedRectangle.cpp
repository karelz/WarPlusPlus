// ZWatchedRectangle.cpp: implementation of the CZWatchedRectangle class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ZWatchedRectangle.h"
#include "ZClientUnitInfoSender.h"
#include "GameServer/GameServer/GameServerCommon.h"
#include "GameServer/GameServer/SMap.h"

#ifndef PROJEKTAPP
extern CSMap g_cMap;
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

void CZWatchedRectangle::Create(CZClientUnitInfoSender *pInfoSender, DWORD dwID)
{
	m_dwID=dwID;
	m_dwLeft=m_dwTop=m_dwWidth=m_dwHeight=0;
	m_dwLastLeft=m_dwLastTop=m_dwLastWidth=m_dwLastHeight=0;
	m_pNext=m_pPrev=this;
	m_pInfoSender=pInfoSender;
}

CZWatchedRectangle::CZWatchedRectangle()
{
	Create(NULL, 0);
}

CZWatchedRectangle::CZWatchedRectangle(CZClientUnitInfoSender *pInfoSender, DWORD dwID)
{
	Create(pInfoSender, dwID);
}

CZWatchedRectangle::~CZWatchedRectangle()
{

}

void CZWatchedRectangle::Append(CZWatchedRectangle *pList)
{
	ASSERT(pList);
	ASSERT(pList->m_pPrev);
	ASSERT(pList->m_pPrev->m_pNext);
	ASSERT(pList->m_pPrev->m_pNext==pList);
	
	m_pPrev=pList->m_pPrev;
	m_pNext=pList;
	pList->m_pPrev->m_pNext=this;
	pList->m_pPrev=this;
}

void CZWatchedRectangle::Insert(CZWatchedRectangle *pList)
{
	ASSERT(pList);
	ASSERT(pList->m_pNext);
	ASSERT(pList->m_pNext->m_pPrev);
	ASSERT(pList->m_pNext->m_pPrev==pList);
	
	m_pNext=pList->m_pNext;
	m_pPrev=pList;
	pList->m_pNext->m_pPrev=this;
	pList->m_pNext=this;
}

void CZWatchedRectangle::Disconnect()
{
	ASSERT(m_pPrev);
	ASSERT(m_pNext);
	ASSERT(m_pPrev->m_pNext);
	ASSERT(m_pNext->m_pPrev);

	m_pPrev->m_pNext=m_pNext;
	m_pNext->m_pPrev=m_pPrev;
	m_pPrev=m_pNext=this;
}

CZWatchedRectangle *CZWatchedRectangle::FindRectangle(DWORD dwID)
{
	ASSERT(m_pPrev);
	ASSERT(m_pNext);
	if(m_dwID==dwID) return this;
	// Cyklus pres cely seznam, dokud se nedostaneme zase zpatky
	for(CZWatchedRectangle *r=this->Next(); r!=this; r=r->Next()) {
		if(r->m_dwID==dwID) return r;
	}
	return NULL;
}

void CZWatchedRectangle::NotifyMap()
{
	// Spolecna oblast
	DWORD cl, ct, cr, cb;
	CPointDW pnt;

	// Musime mit touhle dobou nastaveneho sendera
	ASSERT(m_pInfoSender);

	// Nejprve napocitame spolecnou oblast
	cl=m_dwLastLeft;
	ct=m_dwLastTop;
	cr=m_dwLastLeft+m_dwLastWidth;
	cb=m_dwLastTop+m_dwLastHeight;

	if(m_dwLeft>cl) cl=m_dwLeft;
	if(m_dwTop>ct) ct=m_dwTop;
	if(m_dwLeft+m_dwWidth<cr) cr=m_dwLeft+m_dwWidth;
	if(m_dwTop+m_dwHeight<cb) cb=m_dwTop+m_dwHeight;
	
	// Nyni mame napocitanou prekryvajici se oblast
	if(cl>=cr || ct>=cb) {
		// Oblasti se naprosto neprekryvaji
		// Rychle projeti a registrace/odregistrace

		// Nejdrive odregistrujeme starou oblast
		for(pnt.y=m_dwLastTop; pnt.y<m_dwLastTop+m_dwLastHeight; pnt.y++) {
			for(pnt.x=m_dwLastLeft; pnt.x<m_dwLastLeft+m_dwLastWidth; pnt.x++) {				
#ifndef PROJEKTAPP
                g_cMap.StopWatchingMapSquare(m_pInfoSender, pnt);
#endif
            }
		}
		// A ted zaregistrujeme novou
		for(pnt.y=m_dwTop; pnt.y<m_dwTop+m_dwHeight; pnt.y++) {
			for(pnt.x=m_dwLeft; pnt.x<m_dwLeft+m_dwWidth; pnt.x++) {				
#ifndef PROJEKTAPP
                g_cMap.StartWatchingMapSquare(m_pInfoSender, pnt);
#endif
            }
		}
	} else {
		// Oblasti se prekryvaji
		
		// Nejdrive odregistrujeme starou oblast
		for(pnt.y=m_dwLastTop; pnt.y<m_dwLastTop+m_dwLastHeight; pnt.y++) {
			for(pnt.x=m_dwLastLeft; pnt.x<m_dwLastLeft+m_dwLastWidth; pnt.x++) {
				if(!(pnt.x>=cl && pnt.x<cr && pnt.y>=ct && pnt.y<cb)) {
					// Nejsme v prekryvu
#ifndef PROJEKTAPP
                    g_cMap.StopWatchingMapSquare(m_pInfoSender, pnt);
#endif
                }
			}
		}

		// A ted zaregistrujeme novou
		for(pnt.y=m_dwTop; pnt.y<m_dwTop+m_dwHeight; pnt.y++) {
			for(pnt.x=m_dwLeft; pnt.x<m_dwLeft+m_dwWidth; pnt.x++) {
				if(!(pnt.x>=cl && pnt.x<cr && pnt.y>=ct && pnt.y<cb)) {
					// Nejsme v prekryvu, tam se nedeje nic
#ifndef PROJEKTAPP
                    g_cMap.StartWatchingMapSquare(m_pInfoSender, pnt);
#endif
                }
			}
		}
	}

	// A zapamatujeme si naposledy sdelenou pozici pro mapu
	m_dwLastLeft=m_dwLeft;
	m_dwLastTop=m_dwTop;
	m_dwLastWidth=m_dwWidth;
	m_dwLastHeight=m_dwHeight;
}
