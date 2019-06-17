// AbstractCache.cpp: implementation of the CAbstractCache class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AbstractCache.h"
#include <iostream.h>

IMPLEMENT_DYNAMIC(CAbstractCache, CObject)

void CAbstractCache::Create(DWORD dwMaxSize, DWORD dwPreferredSize) 
{
	// Aby na nas nekdo nevolal Create dvakrat
	ASSERT(m_pObjects==NULL);
	ASSERT(m_dwNumObjects==0);
	ASSERT(m_pQueue==NULL);
	
	m_dwMaxSize=dwMaxSize;
	m_dwPreferredSize=dwPreferredSize;
	m_dwActualSize=0;
	m_pObjects=NULL;
	m_dwNumObjects=0;

	m_pQueue=NULL;

	m_dwMaxSoFar=0;
}

void CAbstractCache::Create(CCacheLoadQueue *pQueue, DWORD dwMaxSize, DWORD dwPreferredSize) 
{
	Create(dwMaxSize, dwPreferredSize);
	m_pQueue=pQueue;
}

CAbstractCache::CAbstractCache(DWORD dwMaxSize /* =10240 */, DWORD dwPreferredSize /* =5120 */)
{
	m_pObjects=NULL; m_dwNumObjects=0; m_pQueue=NULL;
	Create(dwMaxSize, dwPreferredSize);
}

CAbstractCache::CAbstractCache(CCacheLoadQueue *pQueue, DWORD dwMaxSize /* =10240 */, DWORD dwPreferredSize /* =5120 */)
{
	m_pObjects=NULL; m_dwNumObjects=0; m_pQueue=NULL;
	Create(pQueue, dwMaxSize, dwPreferredSize);
}

CAbstractCache::~CAbstractCache()
{

}

#ifdef _DEBUG // Diagnostics

void CAbstractCache::AssertValid() const
{
	ASSERT(m_dwMaxSize>0);	// Max. velikost musi byt kladna
	ASSERT(m_dwPreferredSize>=0);	// Preferovana velikost musi byt nezaporna
	ASSERT(m_dwMaxSize>=m_dwPreferredSize);	// Max. velikost musi byt vetsi nez preferovana
	ASSERT(m_dwNumObjects>=0); // Objektu v cachi musi byt kladne
	ASSERT(m_dwMaxSoFar>=0); // Max. velikost cache je vzdy nezaporna
}

void CAbstractCache::Dump(CDumpContext &dc) const
{
	CObject::Dump(dc);
	dc << "CAbstractCache (max: " << m_dwMaxSize;
	dc << ", preferred: " << m_dwPreferredSize;
	dc << ", objects: " << m_dwNumObjects;
	dc << ", memory used: " << MemoryUsed();
	dc << ", actual size: " << m_dwActualSize << ")\n";
}

#endif // _DEBUG

void CAbstractCache::InsertObject(CCachedObject *pCachedObject) {

    // Objekt nesmi byt v zadne cachi
	ASSERT(pCachedObject->m_pAbstractCache==(CAbstractCache*)NULL);

	// Nastavime objektu odkaz na nasi cache
	pCachedObject->m_pAbstractCache=this;

	// Vytvorime objektu obalku
	CCachedObjectHandle *pHandle=new CCachedObjectHandle(pCachedObject);

	// Nastavime objektu vazbu na tuto obalku
	pCachedObject->m_pInternalData=pHandle;

	// A soupneme ji na konec
	m_RWLock.WriterLock();

	if(!m_pObjects) { // Objekty nejsou veskere zadne
		m_pObjects=pHandle;
		pHandle->PrevHandle(pHandle); // Zamotana hlavicka
		pHandle->NextHandle(pHandle);		
	} else {
		// Vlozeni objektu na posledni misto
		pHandle->PrevHandle(m_pObjects->PrevHandle());
		pHandle->NextHandle(m_pObjects);
		
		m_pObjects->PrevHandle()->NextHandle(pHandle);
		m_pObjects->PrevHandle(pHandle);
	}

	// Aktualizujeme si udaje o pameti
	m_dwNumObjects++;

	pCachedObject->Lock();
	if(pCachedObject->IsLoaded()) {
		m_dwActualSize+=pCachedObject->GetSize();
	}
	pCachedObject->Unlock();

	m_RWLock.WriterUnlock();
}

CCachedObject *CAbstractCache::RemoveObject(CCachedObject *pCachedObject) {

  // Objekt je v nasi cachi
	ASSERT(pCachedObject->m_pAbstractCache==this);
	
	// Zjistime si handle na objekt
	CCachedObjectHandle *pHandle=(CCachedObjectHandle *)pCachedObject->m_pInternalData;

	// Odstranime handle z fronty na load
	if(m_pQueue) {
		m_pQueue->Dequeue(pHandle);
	}

	// Odstranime handle z datovych struktur cache		
	m_RWLock.WriterLock();

	// A snizila se celkova velikost objektu v cachi
	pCachedObject->Lock();

    // Odstranime odkaz na nasi cache
	pCachedObject->m_pAbstractCache=(CAbstractCache *)NULL;

	// Zapamatujeme si nasledovnika handle
	CCachedObjectHandle *pHandleNext=pHandle->NextLoad();

	pHandle->NextHandle()->PrevHandle(pHandle->PrevHandle());
	pHandle->PrevHandle()->NextHandle(pHandle->NextHandle());
	pHandle->NextHandle(NULL);
	pHandle->PrevHandle(NULL);

	if(m_pObjects==pHandle) { // Cirou nahodou rusime hlavicku
		// Hlavicka se posouva na nasledovnika pHandle
		// (pokud pHandle bylo samo zacyklene, je to NULL)
		m_pObjects=pHandleNext;
	}

	// No a protoze pHandle uz nikdo nikde nedrzi, muzeme ho smazat
	delete pHandle;

	// Snizil se pocet cachovanych objektu
	m_dwNumObjects--;

	if(pCachedObject->IsLoaded()) {
		// Jestlize byl nahrany, prispival svou velikosti do m_dwActualSize
		m_dwActualSize-=pCachedObject->GetSize();
	}
	pCachedObject->Unlock();

	m_RWLock.WriterUnlock();

	return (CCachedObject*)pCachedObject;
}

void CAbstractCache::HintDataNeeded(CCachedObject *pCachedObject)
{
	if(m_pQueue) {
		m_pQueue->QueueLoad((CCachedObjectHandle *)pCachedObject->m_pInternalData);
	}
}

void CAbstractCache::HintDataNotNeeded(CCachedObject *pCachedObject)
{
	// V pripade, ze data nejsou potreba, staci prehodit cachovany objekt
	// na zacatek cache, takze LRU algoritmus ho vyrusi jako prvni, jakmile
	// nastane potreba

	ASSERT(pCachedObject->m_pAbstractCache==this);

	// Zamkneme struktury cache na zapis
	m_RWLock.WriterLock();

	CCachedObjectHandle *pHandle=(CCachedObjectHandle *)pCachedObject->m_pInternalData;
	if(pHandle) {
		// Jestli se hlava rovnala nasemu handle, nedelame nic
		if(m_pObjects==pHandle) {
			m_RWLock.WriterUnlock();
			return;
		}
		
		// Vypareme handle ze spojaku
		pHandle->PrevHandle()->NextHandle(pHandle->NextHandle());
		pHandle->NextHandle()->PrevHandle(pHandle->PrevHandle());

		// Pak ho vlozime na ocas (pred hlavu)
		pHandle->PrevHandle(m_pObjects->PrevHandle());
		pHandle->NextHandle(m_pObjects);
		m_pObjects->PrevHandle()->NextHandle(pHandle);
		m_pObjects->PrevHandle(pHandle);
		// A posuneme hlavu na nej
		m_pObjects=pHandle;

		// A vyhodime objekt pripadne z fronty
		if(m_pQueue) {
			m_pQueue->Dequeue(pHandle);
		}
	}

	// Zpristupneny objekt je nyni na konci
	m_RWLock.WriterUnlock();
}

void CAbstractCache::SetMaxSize(DWORD dwMaxSize) {
	m_dwMaxSize=dwMaxSize;
}

	
void CAbstractCache::SetPreferredSize(DWORD dwPreferredSize) {
	m_dwPreferredSize=dwPreferredSize;
}

void CAbstractCache::NotifyObjectAccessed(CCachedObject *pCachedObject)
{
    ASSERT(pCachedObject->m_pAbstractCache==this);

	CCachedObjectHandle *pHandle=(CCachedObjectHandle *)pCachedObject->m_pInternalData;
	if(pHandle) {	
		pHandle->Hit();
	}
}

void CAbstractCache::Cleanup(DWORD dwLimit, CCachedObject *pNoClean /* =NULL */) {
	// Je potreba promazat cache abychom se v poradku vesli
	CCachedObjectHandle *pHandle=m_pObjects;
	DWORD dwMinHits, dwHitsToUnload, dwHits;	
	BOOL bObjectsToUnload=TRUE;

	if(pHandle==NULL) return;
	
	// Dokud vychazi, ze potrebna velikost pameti je vice nez preferovana
	// zkousime unloadit objekty v poradi danem jejich hitcountem
	dwHitsToUnload=0;

	// Dokud cache moc zabira a je co unloadit
	while(MemoryUsed()>dwLimit && bObjectsToUnload) {

		dwMinHits=0xFFFFFFFF;
		pHandle=m_pObjects;
		bObjectsToUnload=FALSE;
	
		while(pHandle) {
			dwHits=pHandle->NumHits();
			// Hledame minimum vetsi nez dwHitsToUnload
			if(dwHits<dwMinHits && dwHits>dwHitsToUnload) {
				dwMinHits=dwHits;
			}

			if(dwHits<=dwHitsToUnload) {
				CCachedObject *pObj=pHandle->GetCachedObject();

				if(pObj && pObj!=pNoClean) {
					// Je tam cachovany objekt
			
					// Zkusime ho vykousat z pameti
					if(pObj->TestWriteLock()) {
						// A mame tento objekt zamceny na zapis
						
						// Jestlize je nahrany
						if(pObj->IsLoaded()) {							
							// Odecteme si velikost objektu
							m_dwActualSize-=pObj->GetSize();

							// A muzeme ho vyruchat z pameti
							pObj->Unload();
							pObj->m_bLoaded=FALSE;
						}
						
						pObj->WriteUnlock();
					}
				}
			} else {
				// Mame objekt, ktery je potencialne unloaditelny
				bObjectsToUnload=TRUE;
			}

			// Zkusime dalsi objekt
			pHandle=pHandle->NextHandle();
			if(pHandle==m_pObjects) {
				// Uz jsme projeli cely seznam
				break;
			}
		}

		dwHitsToUnload=dwMinHits;
	}

	pHandle=m_pObjects;
	
	// A vynulujeme pocitadla hitu
	while(pHandle) {
		pHandle->ResetHits();
		pHandle=pHandle->NextHandle();
		if(pHandle==m_pObjects) break;
	}
}

void CAbstractCache::NotifyObjectLoad(CCachedObject *pCachedObject)
{
    ASSERT(pCachedObject->m_pAbstractCache==this);
	// Objekt se chce nahrat. Je treba provest unload na dostatek objektu
	// aby se tento vesel do pameti

	// Pote co se objekt nahraje, velikost pouzite pameti se zvetsi o dane cislo
	DWORD dwMemoryWanted=pCachedObject->GetSize();
	DWORD dwMemoryAfterLoad=MemoryUsed()+dwMemoryWanted;

	if(dwMemoryAfterLoad<=m_dwPreferredSize) {
		// Velikost pameti je stale mensi, nez preferovana
		// Neni treba mazat zadne objekty

		// Zmenime velikost cache na predpokladanou hodnotu
		m_dwActualSize+=pCachedObject->GetSize();
	} else {
		// Pokusime se vycistit cache, aby mela velikost max. zadane cislo
		// (pak po vlozeni dwMemoryWanted bytu se dostaneme presne pod preferred size)
		m_RWLock.ReaderLock();
		Cleanup(m_dwPreferredSize-dwMemoryWanted);
		
		// Nyni jsme bud uspeli s vykousavanim objektu, nebo ne
		if(MemoryUsed()+dwMemoryWanted>m_dwMaxSize) {
			// Cache vychazi vetsi, nez kolik smi maximalne byt!
			// Dokud to je tak, jsme zablokovani a cekame na unload
			while(MemoryUsed()+dwMemoryWanted>m_dwMaxSize) {				
				// Odemkneme cache..
				m_RWLock.ReaderUnlock();
				// .. a usneme na chvili
				Sleep(0);
				// Zase je zablokujeme kvuli cisteni
				m_RWLock.ReaderLock();
				// A cistime
				Cleanup(m_dwMaxSize-dwMemoryWanted);
			}
		} else {
			// Cache je momentalne pod maximalni velikost			
		}

		// Zmenime velikost cache na predpokladanou hodnotu
		m_dwActualSize+=pCachedObject->GetSize();

		// Odemkneme struktury cache
		m_RWLock.ReaderUnlock();		
	}	
}

void CAbstractCache::NotifyObjectLoaded(CCachedObject *pCachedObject, DWORD dwSizeEstimated)
{
	ASSERT(pCachedObject->m_pAbstractCache==this);

	// Objekt se uspesne nahral. Mame jeho skutecnou velikost
	// takze muzeme presne spocitat, kolik pameti cache spotrebovava
	m_dwActualSize+=pCachedObject->GetSize()-dwSizeEstimated;

	if(MemoryUsed()>m_dwPreferredSize) {
		// A diky tomu muzeme dodatecne vyruchat z pameti nepotrebne objekty
		if(m_RWLock.ReaderLock(1)) {
			// Ale jestli se to nepovede rychle, tak se nic nedeje a vykasleme se na to
			Cleanup(m_dwPreferredSize, pCachedObject);
			m_RWLock.ReaderUnlock();
		}
	}
}

DWORD CAbstractCache::MemoryUsed() const
{
	// Celkova spotreba pameti je pamet spotrebovana loadnutymi objekty
	// plus pamet spotrebovana na handles na objekty a objekty samotnymi
	DWORD dwSize=m_dwActualSize+m_dwNumObjects*(sizeof(CCachedObjectHandle)+sizeof(CCachedObject));
	
	// Tohle sice meni objekt, ale jde jen o nevyznamny udaj - statistiku
	if(m_dwMaxSoFar<dwSize) const_cast <CAbstractCache*>(this)->m_dwMaxSoFar=dwSize;	
	
	return dwSize;
}

DWORD CAbstractCache::MaxSoFar() const
{
	return m_dwMaxSoFar;
}