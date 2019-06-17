// CWatchedRectangle.cpp: implementation of the CCWatchedRectangle class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CWatchedRectangle.h"
#include "CServerUnitInfoReceiver.h"
#include "../DataObjects/CMap.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

DWORD CCWatchedRectangle::m_dwNewFreeID = 1;
CMutex CCWatchedRectangle::m_lockNewFreeID;

// Rozmery s okraji
#define B_LEFT		(m_lLeft-m_lWidth)
#define B_TOP		(m_lTop-m_lHeight)
#define B_WIDTH		(3*m_lWidth)
#define B_HEIGHT	(3*m_lHeight)

#define L_LEFT		(lLeft-lWidth)
#define L_TOP		(lTop-lHeight)
#define L_WIDTH		(3*lWidth)
#define L_HEIGHT	(3*lHeight)

void CCWatchedRectangle::Create(CCServerUnitInfoReceiver *pInfoReceiver)
{
	VERIFY(m_lockNewFreeID.Lock());
	m_dwID = m_dwNewFreeID++;
	VERIFY(m_lockNewFreeID.Unlock());

	m_bWatched=FALSE;

	m_lLeft=m_lTop=m_lWidth=m_lHeight=0;
	m_lLeftMapSquare=m_lTopMapSquare=m_lWidthMapSquare=m_lHeightMapSquare=0;
	m_lLastLeftMapSquare=m_lLastTopMapSquare=m_lLastWidthMapSquare=m_lLastHeightMapSquare=0;
	m_lLastLeft=m_lLastTop=m_lLastWidth=m_lLastHeight=0;
	m_nXMoveVertex=m_nYMoveVertex=0;
	m_pNext=m_pPrev=this;
	m_pInfoReceiver=pInfoReceiver;
}

void CCWatchedRectangle::Delete()
{
	if(m_pInfoReceiver) {
		m_pInfoReceiver->RemoveWatchedRectangle(this);
		m_pInfoReceiver = NULL;
	}
}

CCWatchedRectangle::CCWatchedRectangle()
{
	Create(NULL);
}

CCWatchedRectangle::CCWatchedRectangle(CCServerUnitInfoReceiver *pInfoReceiver)
{
	Create(pInfoReceiver);
}

CCWatchedRectangle::~CCWatchedRectangle()
{
	Delete();
}

void CCWatchedRectangle::Unregister() 
{ 
	ASSERT(m_pInfoReceiver); 
	m_pInfoReceiver->RemoveWatchedRectangle(this); 
}

void CCWatchedRectangle::Append(CCWatchedRectangle *pList)
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

void CCWatchedRectangle::Insert(CCWatchedRectangle *pList)
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

void CCWatchedRectangle::Disconnect()
{
	ASSERT(m_pPrev);
	ASSERT(m_pNext);
	ASSERT(m_pPrev->m_pNext);
	ASSERT(m_pNext->m_pPrev);

	m_pPrev->m_pNext=m_pNext;
	m_pNext->m_pPrev=m_pPrev;
	m_pPrev=m_pNext=this;
}

CCWatchedRectangle *CCWatchedRectangle::FindRectangle(DWORD dwID)
{
	ASSERT(m_pPrev);
	ASSERT(m_pNext);
	if(m_dwID==dwID) return this;
	// Cyklus pres cely seznam, dokud se nedostaneme zase zpatky
	for(CCWatchedRectangle *r=this->Next(); r!=this; r=r->Next()) {
		if(r->m_dwID==dwID) return r;
	}
	return NULL;
}

void CCWatchedRectangle::ClipDimensions() 
{
	CCMap *pMap=m_pInfoReceiver->GetMap();
	ASSERT(pMap);	

// Pomocne define
#define MS_CLIP(a, min, max) (((long)(a)<(long)(min))?(min):(((long)(a)>(long)(max))?(max):(a)))

  // Orizneme souradnice
	m_lLeftMapSquare=MS_CLIP(m_lLeftMapSquare, 0,  pMap->GetWidth()-1);
	m_lTopMapSquare=MS_CLIP(m_lTopMapSquare, 0, pMap->GetHeight()-1);
	
	// Orizneme velikost
	if(m_lLeftMapSquare+m_lWidthMapSquare-1>=(long)pMap->GetWidth())
		m_lWidthMapSquare=pMap->GetWidth()-m_lLeftMapSquare;

	if(m_lTopMapSquare+m_lHeightMapSquare-1>=(long)pMap->GetHeight())
		m_lHeightMapSquare=pMap->GetHeight()-m_lTopMapSquare;	

// Konec pomocneho define
#undef MS_CLIP

}

void CCWatchedRectangle::SetDimensions(long lLeft, long lTop, long lWidth, long lHeight) 
{
	ASSERT(m_pInfoReceiver);

	CCMap *pMap=m_pInfoReceiver->GetMap();
	ASSERT(pMap);	

	// Prepocitame pozici na map squary
	m_lLeftMapSquare=(L_LEFT / MAPSQUARE_WIDTH);
	m_lTopMapSquare=(L_TOP / MAPSQUARE_HEIGHT);

	// Prepocitame velikost na map squary
	m_lWidthMapSquare=(L_LEFT+L_WIDTH) / MAPSQUARE_WIDTH-m_lLeftMapSquare+1;
	m_lHeightMapSquare=(L_TOP+L_HEIGHT) / MAPSQUARE_HEIGHT-m_lTopMapSquare+1;
		
	ClipDimensions();

	if(lLeft==m_lLeft && lTop==m_lTop && lWidth==m_lWidth && lHeight==m_lHeight) {
		// Vlastne se nic nestalo, v tom pripade zachovavame predchozi
		// vektor pohybu
	} else {
		// Spocitame novy vektor pohybu
		m_nXMoveVertex=L_LEFT-B_LEFT;
		m_nYMoveVertex=L_TOP-B_TOP;
	}

	// Zapamatujeme si novou pozici
	m_lLeft=lLeft; m_lTop=lTop; m_lWidth=lWidth; m_lHeight=lHeight;

	// A vyslepicime to serveru
	UpdateServerData();
};

void CCWatchedRectangle::SetPos(long lLeft, long lTop) 
{
	SetDimensions(lLeft, lTop, m_lWidth, m_lHeight);
};

void CCWatchedRectangle::SetSize(long lWidth, long lHeight) 
{
	SetDimensions(m_lLeft, m_lTop, lWidth, lHeight);
};

void CCWatchedRectangle::UpdateServerData()
{
	long lTmpLeft, lTmpTop, lTmpWidth, lTmpHeight;

	if(!m_bWatched) return;

	// Jestlize je vyska/sirka nulova, znamena to, ze jeste nebyly inicializovane rozmery
	// V tom pripade jsme ticho a nic nedelame
	if(m_lWidth==0 || m_lHeight==0) return;

	// Pokud se viewport po mapsquarech NEPOHNUL... predpovidame akci uzivatele
	
 	lTmpLeft=m_lLeftMapSquare; lTmpTop=m_lTopMapSquare;
	lTmpWidth=m_lWidthMapSquare; lTmpHeight=m_lHeightMapSquare;

	if(m_lLeftMapSquare==m_lLastLeftMapSquare &&
		m_lTopMapSquare==m_lLastTopMapSquare &&
		m_lWidthMapSquare==m_lLastWidthMapSquare &&
		m_lHeightMapSquare==m_lLastHeightMapSquare) {		

		// K pohybu nedoslo
		// Pokud se ale viewport pohybuje nejakym smerem
		// a je momentalne naoffsetovany o vice nez pul mapsquaru
		// - potom ho ve smeru pohybu protahneme o jednicku a posleme ho vetsi, nez je ve
		// skutecnosti potreba

		// ZAPRACOVAT TADY.. ? Je to tak dobre/optimalne?

		// Posun doprava (offset v ramci mapsquaru je pres jeho pulku)
		if(m_nXMoveVertex>0 && ((B_LEFT+B_WIDTH) % MAPSQUARE_WIDTH >= MAPSQUARE_WIDTH/2)) {
			// Protahneme viewport doprava  
			m_lWidthMapSquare++;
		}

		// Posun doleva (offset v ramci mapsquaru je pod jeho pulku)
		if(m_nXMoveVertex<0 && (B_LEFT % MAPSQUARE_WIDTH <= MAPSQUARE_WIDTH/2)) {
			// Protahneme viewport doleva
			m_lWidthMapSquare++;
			m_lLeftMapSquare--;
		}

		// Posun nahoru (offset v ramci mapsquaru je nad jeho pulku)
		if(m_nYMoveVertex>0 && ((B_TOP+B_HEIGHT) % MAPSQUARE_HEIGHT >= MAPSQUARE_HEIGHT/2)) {
			// Protahneme viewport dolu
			m_lHeightMapSquare++;
		}

		// Posun dolu (offset v ramci mapsquaru je pod jeho pulku)
		if(m_nYMoveVertex<0 && (B_TOP % MAPSQUARE_HEIGHT <= MAPSQUARE_HEIGHT/2)) {
			// Protahneme viewport nahoru
			m_lHeightMapSquare++;
			m_lTopMapSquare--;
		}
	}

	// Orezeme rozmery, aby to nedelalo problemy
	ClipDimensions();

	// Pokud se viewport po mapsquarech POHNUL, updatujeme automaticky data na serveru
	if(m_lLeftMapSquare!=m_lLastLeft ||
		m_lTopMapSquare!=m_lLastTop ||
		m_lWidthMapSquare!=m_lLastWidth ||
		m_lHeightMapSquare!=m_lLastHeight) {		

		// Doslo k pohyby, posilame data
		m_pInfoReceiver->GetRequestConnection()->SendRectangleMoved(this);

		// Zapamatujeme si posledni poslanou pozici
		m_lLastLeft=m_lLeftMapSquare;
		m_lLastTop=m_lTopMapSquare;
		m_lLastWidth=m_lWidthMapSquare;
		m_lLastHeight=m_lHeightMapSquare;	
	}

	m_lLeftMapSquare=lTmpLeft; m_lTopMapSquare=lTmpTop;
	m_lWidthMapSquare=lTmpWidth; m_lHeightMapSquare=lTmpHeight;

	m_lLastLeftMapSquare=m_lLeftMapSquare;
	m_lLastTopMapSquare=m_lTopMapSquare;
	m_lLastWidthMapSquare=m_lWidthMapSquare;
	m_lLastHeightMapSquare=m_lHeightMapSquare;
}