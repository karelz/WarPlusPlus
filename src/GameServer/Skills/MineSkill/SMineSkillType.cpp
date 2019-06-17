/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Server hry - skilly
 *   Autor: Roman Zenka
 * 
 *   Popis: Skila pro tezeni resourcu a jejich vykladani
 * 
 ***********************************************************/

#include "StdAfx.h"

#include "SMineSkillType.h"
#include "SMineSkill.h"

#include "GameServer\Skills\ResourceSkill\SResourceSkillType.h"
#include "GameServer\Skills\StorageSkill\SStorageSkillType.h"

#include "ISysCallMine.h"

#include "GameServer\GameServer\SMap.h"
#include "GameServer\GameServer\SCivilization.h"
#include "GameServer\Civilization\Src\ZUnit.h"
#include "TripleS\Interpret\Src\SysObjs\ISOResources.h"

#include "SMineSetModeDPC.h" // For Vitek
#include "GameServer\Civilization\ZCivilization.h" // Also for Vitek
#include "GameServer\Skills\SysCalls\IDPCSysCall.h" // One more for Vitek


// Vytahne z jednotky jeji skill data
#define SKILLDATA(pUnit) (SSMineSkillUnit *)GetSkillData(pUnit)

// implementace interfacu volání ze skriptù
BEGIN_SKILLCALL_INTERFACE_MAP ( CSMineSkillType, CSSkillType )
	SKILLCALL_BASECLASS,
	SKILLCALL_BASECLASS,
	SKILLCALL_METHOD(SCI_Mine),
	SKILLCALL_METHOD(SCI_Unload),
	SKILLCALL_METHOD(SCI_GetResources),
	SKILLCALL_METHOD(SCI_SetMode),
	SKILLCALL_METHOD(SCI_GetCapacity),
	SKILLCALL_METHOD(SCI_IsEmpty),
	SKILLCALL_METHOD(SCI_IsFull)
END_SKILLCALL_INTERFACE_MAP ()

IMPLEMENT_DYNAMIC(CSMineSkillType, CSSkillType);
// implementace typu skilly
IMPLEMENT_SKILL_TYPE ( CSMineSkillType, 4, "Mine" );

/////////////////////////////////////////////////////////////////////
// Konstrukce a destrukce
/////////////////////////////////////////////////////////////////////

// konstruktor
CSMineSkillType::CSMineSkillType () 
{
}

// destruktor
CSMineSkillType::~CSMineSkillType () 
{
}

/////////////////////////////////////////////////////////////////////
// Inicializace a znièení dat objektu
/////////////////////////////////////////////////////////////////////

// inicializuje data typu skilly z dat "pData" velikosti "dwDataSize"
//		výjimky: CPersistentLoadException
void CSMineSkillType::CreateSkillType ( void *pData, DWORD dwDataSize ) 
{
    SMMineSkillType *pMData=(SMMineSkillType*)pData;
    
    // Kopie dat
	m_dwAppearanceBeforeMining = pMData->m_dwAppearanceBeforeMining;
	m_dwAppearanceWhileMining = pMData->m_dwAppearanceWhileMining;
	m_dwAppearanceAfterMining = pMData->m_dwAppearanceAfterMining;

	m_dwAppearanceBeforeUnload = pMData->m_dwAppearanceBeforeUnload;
	m_dwAppearanceWhileUnload = pMData->m_dwAppearanceWhileUnload;
	m_dwAppearanceAfterUnload = pMData->m_dwAppearanceAfterUnload;

	m_dwAppearanceBeforeMiningFull = pMData->m_dwAppearanceBeforeMiningFull;
	m_dwAppearanceWhileMiningFull = pMData->m_dwAppearanceWhileMiningFull;
	m_dwAppearanceAfterUnloadFull = pMData->m_dwAppearanceAfterUnloadFull;

	m_dwMaxMineDistance = pMData->m_dwMaxMineDistance;
	m_dwMaxUnloadDistance = pMData->m_dwMaxUnloadDistance;

	m_dwTimeslicesBeforeMining = pMData->m_dwTimeslicesBeforeMining;
	m_dwTimeslicesAfterMining = pMData->m_dwTimeslicesAfterMining;

	m_dwTimeslicesBeforeUnload = pMData->m_dwTimeslicesBeforeUnload;
	m_dwTimeslicesAfterUnload = pMData->m_dwTimeslicesAfterUnload;

	m_dwTimeslicesBeforeMiningFull = pMData->m_dwTimeslicesBeforeMiningFull;
	m_dwTimeslicesAfterUnloadFull = pMData->m_dwTimeslicesAfterUnloadFull;

	m_StorageSize.Create(pMData->m_StorageSize);
	m_LoadPerTimeslice.Create(pMData->m_LoadPerTimeslice);
	m_UnloadPerTimeslice.Create(pMData->m_UnloadPerTimeslice);

	m_nModeFull=pMData->m_nModeFull;
}

// Dodatecne zavolane po projeti CreateSkillType na vsech skillach
void CSMineSkillType::PostCreateSkillType()
{
}

// znièí data typu skilly
void CSMineSkillType::DeleteSkillType () 
{
}

/////////////////////////////////////////////////////////////////////
// Ukládání dat potomkù (analogie CPersistentObject metod)
/////////////////////////////////////////////////////////////////////

/* CSMineSkillType stored
	
	DWORD m_dwAppearanceBeforeMining
	DWORD m_dwAppearanceWhileMining
	DWORD m_dwAppearanceAfterMining

	DWORD m_dwAppearanceBeforeUnload
	DWORD m_dwAppearanceWhileUnload
	DWORD m_dwAppearanceAfterUnload

	DWORD m_dwAppearanceBeforeMiningFull
	DWORD m_dwAppearanceWhileMiningFull
	DWORD m_dwAppearanceAfterUnloadFull

	DWORD m_dwMaxMineDistance
	DWORD m_dwMaxUnloadDistance

	DWORD m_dwTimeslicesBeforeMining
	DWORD m_dwTimeslicesAfterMining

	DWORD m_dwTimeslicesBeforeUnload
	DWORD m_dwTimeslicesAfterUnload

	DWORD m_dwTimeslicesBeforeMiningFull
	DWORD m_dwTimeslicesAfterUnloadFull

	m_StoregeSize STORED
	m_LoadPerTimeslice STORED
	m_UnloadPerTimeslice STORED

	int m_nModeFull 
*/

// ukládání dat potomka
void CSMineSkillType::PersistentSaveSkillType ( CPersistentStorage &storage ) 
{
	BRACE_BLOCK(storage);
	
	storage << m_dwAppearanceBeforeMining;
	storage << m_dwAppearanceWhileMining;
	storage << m_dwAppearanceAfterMining;

	storage << m_dwAppearanceBeforeUnload;
	storage << m_dwAppearanceWhileUnload;
	storage << m_dwAppearanceAfterUnload;

	storage << m_dwAppearanceBeforeMiningFull;
	storage << m_dwAppearanceWhileMiningFull;
	storage << m_dwAppearanceAfterUnloadFull;

	storage << m_dwMaxMineDistance;
	storage << m_dwMaxUnloadDistance;

	storage << m_dwTimeslicesBeforeMining;
	storage << m_dwTimeslicesAfterMining;

	storage << m_dwTimeslicesBeforeUnload;
	storage << m_dwTimeslicesAfterUnload;

	storage << m_dwTimeslicesBeforeMiningFull;
	storage << m_dwTimeslicesAfterUnloadFull;

	m_StorageSize.Save(storage);
	m_LoadPerTimeslice.Save(storage);
	m_UnloadPerTimeslice.Save(storage);

	storage << m_nModeFull;
}

// nahrávání pouze uložených dat potomka
void CSMineSkillType::PersistentLoadSkillType ( CPersistentStorage &storage ) 
{
    BRACE_BLOCK(storage);

	storage >> m_dwAppearanceBeforeMining;
	storage >> m_dwAppearanceWhileMining;
	storage >> m_dwAppearanceAfterMining;

	storage >> m_dwAppearanceBeforeUnload;
	storage >> m_dwAppearanceWhileUnload;
	storage >> m_dwAppearanceAfterUnload;

	storage >> m_dwAppearanceBeforeMiningFull;
	storage >> m_dwAppearanceWhileMiningFull;
	storage >> m_dwAppearanceAfterUnloadFull;

	storage >> m_dwMaxMineDistance;
	storage >> m_dwMaxUnloadDistance;

	storage >> m_dwTimeslicesBeforeMining;
	storage >> m_dwTimeslicesAfterMining;

	storage >> m_dwTimeslicesBeforeUnload;
	storage >> m_dwTimeslicesAfterUnload;

	storage >> m_dwTimeslicesBeforeMiningFull;
	storage >> m_dwTimeslicesAfterUnloadFull;

	m_StorageSize.Load(storage);
	m_LoadPerTimeslice.Load(storage);
	m_UnloadPerTimeslice.Load(storage);

	storage >> m_nModeFull;
}

// pøeklad ukazatelù potomka
void CSMineSkillType::PersistentTranslatePointersSkillType ( CPersistentStorage &storage ) 
{
}

// inicializace nahraného objektu potomka
void CSMineSkillType::PersistentInitSkillType () 
{
}

/////////////////////////////////////////////////////////////////////
// Get/Set metody serverových informací o typu skilly jednotky
/////////////////////////////////////////////////////////////////////

// vrátí velikost dat skilly pro jednotku
DWORD CSMineSkillType::GetSkillDataSize () 
{
    return sizeof(SSMineSkillUnit);
}

// vrátí masku skupin skill
DWORD CSMineSkillType::GetGroupMask () 
{
	return Group_Mine;
}

/////////////////////////////////////////////////////////////////////
// Metody skilly
/////////////////////////////////////////////////////////////////////

// provede TimeSlice na skille jednotky "pUnit"
// jednotka "pUnit" je již zamèená pro zápis, metoda mùže zamykat libovolný poèet jiných
//		jednotek pro zápis/ètení (vèetnì jednotek ze svého bloku)
void CSMineSkillType::DoTimeSlice ( CSUnit *pUnit ) 
{
	SSMineSkillUnit *pData = SKILLDATA ( pUnit );

    DWORD dwSkillIndex = 0;
    CSSkillType *pSkillType;
	int nResult=0;

	// Sjednoceni promennych (aby se kod nepsal zbytecne dvakrat)
	BOOL bMining=(pData->m_nDoing==SSMineSkillUnit::mining);
	
	DWORD dwAppearanceBefore=bMining?(pData->m_bWasFull?m_dwAppearanceBeforeMiningFull:m_dwAppearanceBeforeMining):m_dwAppearanceBeforeUnload;
	DWORD dwAppearanceWhile=bMining?(pData->m_bWasFull?m_dwAppearanceWhileMiningFull:m_dwAppearanceWhileMining):m_dwAppearanceWhileUnload;
	DWORD dwAppearanceAfter=bMining?m_dwAppearanceAfterMining:m_dwAppearanceAfterUnload;
	DWORD dwAppearanceAfterFull=bMining?m_dwAppearanceAfterMining:m_dwAppearanceAfterUnloadFull;
	DWORD dwTimeslicesBefore=bMining?(pData->m_bWasFull?m_dwTimeslicesBeforeMiningFull:m_dwTimeslicesBeforeMining):m_dwTimeslicesBeforeUnload;
	DWORD dwTimeslicesAfter=bMining?m_dwTimeslicesAfterMining:m_dwTimeslicesAfterUnload;
	DWORD dwTimeslicesAfterFull=bMining?m_dwTimeslicesAfterMining:m_dwTimeslicesAfterUnloadFull;
	DWORD dwMaxDistance=bMining?m_dwMaxMineDistance:m_dwMaxUnloadDistance;
	CSResources &PerTimeslice=bMining?m_LoadPerTimeslice:m_UnloadPerTimeslice;
	int nSkillGroup=bMining?Group_Resource:Group_Storage;

	CPointDW pointPosition;
	DWORD dwDistance;

	// pro jistotu zamkneme tezenou jednotku na zapis
	VERIFY(pData->m_pUnit->PrivilegedThreadWriterLock());
	
	ASSERT(pData->m_nState!=SSMineSkillUnit::undefined);

	// Jestlize jednotka je v pocatecnim stavu
	if(pData->m_nState==SSMineSkillUnit::initial) {
		dwSkillIndex=0;
		nResult=0;
		// Nejprve ze vseho zjistime, jestli ma dana jednotka protejsek pro tezeni/unload
		while ((pSkillType = pData->m_pUnit->GetUnitType()->GetNextSkillType(dwSkillIndex, nSkillGroup)) != NULL) nResult=1;

		// Spocitame vzdalenost jednotek
		pointPosition = pData->m_pUnit->GetPosition();		
		dwDistance = pointPosition.GetDistanceSquare(pUnit->GetPosition());

		if(dwDistance>dwMaxDistance*dwMaxDistance) {
			pData->m_nState=SSMineSkillUnit::done;
		} else if(nResult==0) {
			// Jednotka nema protejsek pro mine skillu, problem		
			// Nastavujeme, ze jsme skoncili
			pData->m_nState=SSMineSkillUnit::done;

			// Vypsat nejake varovne hlaseni?
		} else if(!bMining && pData->m_Resources.IsEmpty()) {
			// Vykladame a jsme prazdni
			pData->m_nState=SSMineSkillUnit::done;
		} else if(bMining && pData->m_Resources==m_StorageSize) {
			// Tezime a jsme plni
			pData->m_nState=SSMineSkillUnit::done;
		} else {
			// Ma smysl tezit/unloadit
			pData->m_nState=SSMineSkillUnit::before;
			pData->m_dwRemainingTime=dwTimeslicesBefore;

			// natocime se k tezene jednotce
			if (g_cMap.IsMapPosition(pointPosition))
				pUnit->SetDirection(pointPosition);

			if(dwTimeslicesBefore>0) {
				pUnit->SetAppearanceID(dwAppearanceBefore);
			}
		}
	}

	// Jestlize jednotka provadi pocatecni animaci
	if(pData->m_nState==SSMineSkillUnit::before) {
		if(pData->m_dwRemainingTime==0) {
			// Dosel cas na uvodni animaci
			pData->m_nState=SSMineSkillUnit::working;
			pUnit->SetAppearanceID(dwAppearanceWhile);
			dwSkillIndex=0;

			if((pSkillType = pData->m_pUnit->GetUnitType()->GetNextSkillType(dwSkillIndex, nSkillGroup)) != NULL)
			{
				// Rekneme tezene/unloadovaci jednotce, ze s ni pracujeme
				if(bMining) {
					((CSResourceSkillType*)pSkillType)->LoadStarted(pData->m_pUnit);
				} else {
					((CSStorageSkillType*)pSkillType)->UnloadStarted(pData->m_pUnit);
				}
			}			
		} else {
			// Odecteme z casu na pocatecni animaci
			pData->m_dwRemainingTime--;
		}
	}
	
	// Jestlize pracujeme
	if(pData->m_nState==SSMineSkillUnit::working) {		
		nResult=0;
		dwSkillIndex=0;

		// Spocitame vzdalenost jednotek
		pointPosition = pData->m_pUnit->GetPosition();		
		dwDistance = pointPosition.GetDistanceSquare(pUnit->GetPosition());

		// Jestlize ma smysl tezit/unloadit
		if(dwDistance<=dwMaxDistance*dwMaxDistance) {
			while ((pSkillType = pData->m_pUnit->GetUnitType()->GetNextSkillType(dwSkillIndex, nSkillGroup)) != NULL)
			{
				int nTmpResult;
				
				// Zavolame na skille metodu, ktera k nam prevede tolik resourcu, kolik muze
				if(bMining) {
					nTmpResult=((CSResourceSkillType*)pSkillType)->LoadResources(pData->m_pUnit, pData->m_Resources, m_StorageSize, PerTimeslice);
				} else {
					nTmpResult=((CSStorageSkillType*)pSkillType)->StoreResources(pData->m_pUnit, pData->m_Resources, PerTimeslice);
				}

				pData->m_bChange=true;

				// Pamatujeme si nejoptimistictejsi vysledek, podle neho se dal zaridime
				if(nResult<nTmpResult) nResult=nTmpResult;
			}
		}

		if(nResult<CSResourceSkillType::continueWork || nResult==CSResourceSkillType::allNeedsSatisfied) {
			// Nema smysl pokracovat v praci

			// Tezili jsme a natezili?
			if(bMining && !pData->m_Resources.IsEmpty()) {
				pUnit->SetMode(m_nModeFull);
			}
			// Vykladali jsme a jsme prazdni?
			if(!bMining && pData->m_Resources.IsEmpty()) {
				pUnit->SetMode(0);
			}

			pData->m_nState=SSMineSkillUnit::after;
			if(pData->m_Resources.IsEmpty()) {
				// Jsme prazdni
				pData->m_dwRemainingTime=dwTimeslicesAfter;
				if(dwTimeslicesAfter>0) {
					pUnit->SetAppearanceID(dwAppearanceAfter);
				}
			} else {
				pData->m_dwRemainingTime=dwTimeslicesAfterFull;
				if(dwTimeslicesAfterFull>0) {
					pUnit->SetAppearanceID(dwAppearanceAfterFull);
				}
			}
			dwSkillIndex=0;
			if((pSkillType = pData->m_pUnit->GetUnitType()->GetNextSkillType(dwSkillIndex, nSkillGroup)) != NULL)
			{
				// Rekneme tezene/unloadovaci jednotce, ze s ni pracujeme
				if(bMining) {
					((CSResourceSkillType*)pSkillType)->LoadEnded(pData->m_pUnit);
				} else {
					((CSStorageSkillType*)pSkillType)->UnloadEnded(pData->m_pUnit);
				}
			}
		}
	}

	// Jestlize jednotka provadi konecnou animaci
	if(pData->m_nState==SSMineSkillUnit::after) {
		if(pData->m_dwRemainingTime==0) {
			// Dosel cas na zaverecnou animaci
			pData->m_nState=SSMineSkillUnit::done;
			pUnit->SetDefaultAppearanceID();
		} else {
			// Odecteme z casu na konecnou animaci
			pData->m_dwRemainingTime--;
		}
	}

	// odemkneme jednotku, koncime
	pData->m_pUnit->PrivilegedThreadWriterUnlock();

	// Jestlize jsme skoncili
	if(pData->m_nState==SSMineSkillUnit::done) {
		Finish(pUnit);        
        return;
	}
}

// inicializuje skillu jednotky "pUnit"
void CSMineSkillType::InitializeSkill ( CSUnit *pUnit ) 
{
	SSMineSkillUnit *pData = SKILLDATA ( pUnit );

	// Prazdna data pro jednotku
	pData->m_pSysCall=NULL;
	pData->m_nDoing=SSMineSkillUnit::nothing;
	pData->m_nState=SSMineSkillUnit::initial;
	pData->m_dwRemainingTime=0;
	pData->m_pUnit=NULL;
	pData->m_Resources.Empty();
	pData->m_bChange=false;	
}

// znièí skillu jednotky "pUnit"
void CSMineSkillType::DestructSkill ( CSUnit *pUnit ) 
{
	SSMineSkillUnit *pData = SKILLDATA ( pUnit );

	// zjistí, je-li asociováno systémové volání
	if ( pData->m_pSysCall != NULL )
	{	// je asociování systémové volání
		// ukonèí èekající systémové volání
		pData->m_pSysCall->FinishWaiting ( STATUS_SKILL_COLLISION );
        pData->m_pSysCall->Release();
		pData->m_pSysCall = NULL;
	}	
}

// aktivuje skillu jednotky "pUnit" (jednotka musí být zamèena pro zápis)
// vraci false, pokud pozadovana cinnost neni proveditelna (jednotku nelze tezit/unloadit)
// bUnload==TRUE -> Unload, bUnload==FALSE -> Mine
BOOL CSMineSkillType::ActivateSkill(CSUnit *pUnit, CSUnit *pMineUnit, BOOL bUnload, CISysCallMine *pMineSysCall) 
{
	SSMineSkillUnit *pData = SKILLDATA ( pUnit );    

	if(pUnit==pMineUnit) {
		return FALSE;
	}

	// zjistí, je-li skilla již aktivní
	if ( IsSkillActive ( pUnit ) )
	{	// skilla je aktivní
		// nejprve se deaktivuje
		DeactivateSkill ( pUnit );
	}
    
	// deaktivuje nesluèitelné skilly
	GetUnitType()->DeactivateSkillGroup ( pUnit, Group_Move | Group_Attack );

	// Inicializujeme data
	pData->m_nState=SSMineSkillUnit::initial;
	pData->m_dwRemainingTime=0;
	pData->m_bChange=false;

	// Naplnime priznak, ze jiz bylo natezeno predem
	pData->m_bWasFull=!pData->m_Resources.IsEmpty();

    // zvysime pocet referenci na jednotce, kterou budeme zpracovavat
    pMineUnit->AddRef();
    // ulozime si pointer na ni
    pData->m_pUnit = pMineUnit;

	// nastavime spravne cinnost
	pData->m_nDoing = bUnload?SSMineSkillUnit::unloading:SSMineSkillUnit::mining;

	// nastavime syscall
    pMineSysCall->AddRef();
	pData->m_pSysCall = pMineSysCall;

	// aktivuje skillu na jednotce
	CSSkillType::ActivateSkill ( pUnit );

    return TRUE;
}

// deaktivuje skillu jednotky "pUnit" (jednotka musí být zamèena pro zápis)
void CSMineSkillType::DeactivateSkill ( CSUnit *pUnit ) 
{
	SSMineSkillUnit *pData = SKILLDATA ( pUnit );

	// zjistí, je-li asociováno systémové volání
	if ( pData->m_pSysCall != NULL )
	{	// je asociování systémové volání
		// ukonèí èekající systémové volání
		pData->m_pSysCall->FinishWaiting ( STATUS_SKILL_COLLISION );
        pData->m_pSysCall->Release();
		pData->m_pSysCall = NULL;
	}

	if(pData->m_nState==SSMineSkillUnit::working) {
		// Byli jsme deaktivovani uprostred prace
		// Je treba rict jednotce, ze uz z ni netezime/neunloadime
		DWORD dwSkillIndex=0;
		CSSkillType *pSkillType;
		if(pData->m_nDoing==SSMineSkillUnit::mining) {
			if((pSkillType = pData->m_pUnit->GetUnitType()->GetNextSkillType(dwSkillIndex, Group_Resource)) != NULL)
			{
				((CSResourceSkillType*)pSkillType)->LoadEnded(pData->m_pUnit);
			}
		} else {
			if((pSkillType = pData->m_pUnit->GetUnitType()->GetNextSkillType(dwSkillIndex, Group_Storage)) != NULL)
			{			
				((CSStorageSkillType*)pSkillType)->UnloadEnded(pData->m_pUnit);
			}
		}
		
		// A pokud jsme naprosto vytezeni, musime zmenit mod na vytezeno
		// Pokud jsme natezeni, menime mod na natezeno
		if(pData->m_Resources.IsEmpty()) {
			pUnit->SetMode(0);
		} else {
			pUnit->SetMode(m_nModeFull);
		}
	}

    // snizime pocet referenci zpracovavane jednotky
    pData->m_pUnit->Release();
    // vynulujeme pointer na zpracovanou jednotku
    pData->m_pUnit = NULL;

	// deaktivuje skillu na jednotce
	CSSkillType::DeactivateSkill ( pUnit );
}

/////////////////////////////////////////////////////////////////////
// Ukládání dat od jednotek
/////////////////////////////////////////////////////////////////////

/* MineSkill stored

	BOOL m_pSysCall != NULL
	if(m_pSysCall)
		m_pSysCall stored

	int m_nDoing
	int m_nState
	DWORD m_dwRemainingTime
	bool m_bWasFull
	void *m_pUnit
	m_Resources stored
	bool m_bChange
*/

// ukládání dat skilly
void CSMineSkillType::PersistentSaveSkill ( CPersistentStorage &storage, CSUnit *pUnit ) 
{
	BRACE_BLOCK(storage);

	SSMineSkillUnit *pData = SKILLDATA ( pUnit );
	
	storage << (BOOL)( pData->m_pSysCall != NULL );

	// zjistí, je-li systémové volání platné
	if ( pData->m_pSysCall != NULL )
	{	// systémové volání je platné
		// uloží systémové volání
		pData->m_pSysCall->PersistentSave ( storage );
	}

	storage << pData->m_nDoing;
	storage << pData->m_nState;
	storage << pData->m_dwRemainingTime;
	storage << pData->m_bWasFull;
	storage << (void*)pData->m_pUnit;
	pData->m_Resources.Save(storage);
	storage << pData->m_bChange;
}

// nahrávání pouze uložených dat skilly
void CSMineSkillType::PersistentLoadSkill ( CPersistentStorage &storage, CSUnit *pUnit ) 
{
	BRACE_BLOCK(storage);

	SSMineSkillUnit *pData = SKILLDATA ( pUnit );

	BOOL bSysCall;
	storage >> bSysCall;

	// zjistí, je-li systémové volání platné
	if ( bSysCall )
	{	// systémové volání je platné
		// vytvoøí systémové volání
		CISysCallMine *pSysCall = new CISysCallMine;
		// naète systémové volání
		pSysCall->PersistentLoad ( storage );
		// aktualizuje ukazatel na systémové volání
		pData->m_pSysCall = pSysCall;
	}
	else
	{	// systémové volání je neplatné
		// aktualizuje ukazatel na systémové volání
		pData->m_pSysCall = NULL;
	}

	storage >> (int &)pData->m_nDoing;
	storage >> (int &)pData->m_nState;
	storage >> pData->m_dwRemainingTime;
	storage >> pData->m_bWasFull;
	storage >> (void*&)pData->m_pUnit;
	pData->m_Resources.Load(storage);
	storage >> pData->m_bChange;
}

// pøeklad ukazatelù dat skilly
void CSMineSkillType::PersistentTranslatePointersSkill ( CPersistentStorage &storage, CSUnit *pUnit ) 
{
	SSMineSkillUnit *pData = SKILLDATA ( pUnit );

	// zjistí, je-li systémové volání platné
	if ( pData->m_pSysCall != NULL )
	{	// systémové volání je platné
		// pøeloží ukazatele systémového volání
		pData->m_pSysCall->PersistentTranslatePointers ( storage );
	}

	pData->m_pUnit=(CSUnit *)storage.TranslatePointer(pData->m_pUnit);

    // otestujeme, ze se to rozumne prelozilo
    if (pData->m_pUnit != NULL)
    {
        LOAD_ASSERT(pData->m_pUnit->IsKindOf(RUNTIME_CLASS(CSUnit)));
    }
}

// inicializace nahraných dat skilly
void CSMineSkillType::PersistentInitSkill ( CSUnit *pUnit ) 
{
	SSMineSkillUnit *pData = SKILLDATA ( pUnit );	

	// zjistí, je-li systémové volání platné
	if ( pData->m_pSysCall != NULL )
	{	// systémové volání je platné
		// inicializuje systémové volání
		pData->m_pSysCall->PersistentInit ();
	}

    // otestujeme parametr
    ASSERT(pUnit != NULL);
    ASSERT_KINDOF(CSUnit, pUnit);
}

/////////////////////////////////////////////////////////////////////
// Interface volání ze skriptù
/////////////////////////////////////////////////////////////////////

ESyscallResult CSMineSkillType::SCI_Mine ( CSUnit *pUnit, CIProcess *pProcess, CISyscall **ppSysCall,
                                          CIBag *pBag, CIDataStackItem *pReturn, 
                                          CZUnit *pMineUnit)
{
    CISysCallMine *pMineSysCall=new CISysCallMine();
    VERIFY( pMineSysCall->Create(pProcess, pUnit, this, pBag) );

    if(pMineUnit==NULL || !ActivateSkill(pUnit, pMineUnit->GetSUnit(), FALSE, pMineSysCall))
    {
        // Nepovedlo se zaktivovat
        pProcess->m_pInterpret->ReportError("Cannot mine.");
        pMineSysCall->Release();

		// vrati chybu
		pBag->ReturnBool ( false );

        return SYSCALL_SUCCESS;
    }

    *ppSysCall=pMineSysCall;
    return SYSCALL_SUCCESS_SLEEP;
}

ESyscallResult CSMineSkillType::SCI_Unload ( CSUnit *pUnit, CIProcess *pProcess, CISyscall **ppSysCall,
                                          CIBag *pBag, CIDataStackItem *pReturn, 
                                          CZUnit *pUnitForUnload)
{
    CISysCallMine *pMineSysCall=new CISysCallMine();
    VERIFY( pMineSysCall->Create(pProcess, pUnit, this, pBag) );

    if(pUnitForUnload==NULL || !ActivateSkill(pUnit, pUnitForUnload->GetSUnit(), TRUE, pMineSysCall))
    {
        // Nepovedlo se zaktivovat
        pProcess->m_pInterpret->ReportError("Cannot unload.");
        pMineSysCall->Release();

		// vrati chybu
		pBag->ReturnBool ( false );

        return SYSCALL_SUCCESS;
    }
    
    *ppSysCall=pMineSysCall;
    return SYSCALL_SUCCESS_SLEEP;
}

ESyscallResult CSMineSkillType::SCI_GetResources (CSUnit *pUnit, CIProcess *pProcess, CISyscall **ppSysCall, CIBag *pBag, CIDataStackItem *pReturn)
{
	CISOResources *pResources;
	SSMineSkillUnit *pData = SKILLDATA ( pUnit );

	pResources = (CISOResources *)pProcess->m_pInterpret->CreateSystemObject(SO_RESOURCES);

	if (pResources == NULL) {
		return SYSCALL_ERROR;
	}

	pResources->Set(pData->m_Resources);
	pReturn->Set(pResources);
	pResources->Release(); 

	return SYSCALL_SUCCESS;
}

ESyscallResult CSMineSkillType::SCI_SetMode (CSUnit *pUnit, CIProcess *pProcess, CISyscall **ppSysCall, CIBag *pBag, CIDataStackItem *pReturn, int* pMode)
{
    CSMineSetModeDPC *pDPC = new CSMineSetModeDPC(pProcess, pReturn, pUnit, this, *pMode);
    g_cMap.RegisterDPC(pDPC, pProcess->m_pInterpret->m_pCivilization->GetCivilizationIndex());

    *ppSysCall = pDPC->GetDPCSysCall();

    return SYSCALL_SUCCESS_SLEEP;
}

ESyscallResult CSMineSkillType::SCI_GetCapacity (CSUnit *pUnit, CIProcess *pProcess, CISyscall **ppSysCall, CIBag *pBag, CIDataStackItem *pReturn)
{
	CISOResources *pResources;

	pResources = (CISOResources *)pProcess->m_pInterpret->CreateSystemObject(SO_RESOURCES);

	if (pResources == NULL) {
		return SYSCALL_ERROR;
	}

	pResources->Set(m_StorageSize);
	pReturn->Set(pResources);
	pResources->Release(); 

	return SYSCALL_SUCCESS;
}

ESyscallResult CSMineSkillType::SCI_IsEmpty (CSUnit *pUnit, CIProcess *pProcess, CISyscall **ppSysCall, CIBag *pBag, CIDataStackItem *pReturn)
{
	SSMineSkillUnit *pData = SKILLDATA ( pUnit );
	bool bRet=pData->m_Resources.IsEmpty()?true:false;
	
	pReturn->Set(bRet);

	return SYSCALL_SUCCESS;
}

ESyscallResult CSMineSkillType::SCI_IsFull (CSUnit *pUnit, CIProcess *pProcess, CISyscall **ppSysCall, CIBag *pBag, CIDataStackItem *pReturn)
{
	SSMineSkillUnit *pData = SKILLDATA ( pUnit );
	bool bRet=(pData->m_Resources==m_StorageSize)?true:false;
	
	pReturn->Set(bRet);

	return SYSCALL_SUCCESS;
}

/////////////////////////////////////////////////////////////////////
// Pomocné metody skilly
/////////////////////////////////////////////////////////////////////

// ukonci skilu
void CSMineSkillType::Finish ( CSUnit *pUnit ) 
{
	SSMineSkillUnit *pData = (SSMineSkillUnit *)GetSkillData ( pUnit );

	// Zmena appearance na default
	pUnit->SetDefaultAppearanceID();

	// zjistí, je-li platné systémové volání
	if ( pData->m_pSysCall != NULL )
	{	
		// nastaví návratovou hodnotu
		pData->m_pSysCall->GetBag()->ReturnBool ( pData->m_bChange );

		// systémové volání je platné   
		// ukonèí èekající systémové volání
		pData->m_pSysCall->FinishWaiting ( STATUS_SUCCESS );        
	
		// zruší odkaz na èekající systémové volání
        pData->m_pSysCall->Release();
		pData->m_pSysCall = NULL;
	}

	// deaktivuje skillu na jednotce
	DeactivateSkill ( pUnit );
}

void CSMineSkillType::FillFullInfo ( CSUnit *pUnit, CString &strInfo )
{
	int i, j;
	SSMineSkillUnit *pData = (SSMineSkillUnit *)GetSkillData ( pUnit );

	strInfo="Mine: ";
	CString s1, s2, s3, tmp;
	j=0;
	for(i=0; i<RESOURCE_COUNT; i++) {
		if(m_StorageSize.m_Resources[i]>0) {
			j++;
			if(j>1) {
				s1+=","; s2+=","; s3+=",";
			}
			tmp.Format("%d", m_StorageSize.m_Resources[i]);
			s2+=tmp;
			tmp.Format("%d", pData->m_Resources[i]);
			s1+=tmp;
			tmp.Format("%d", (int)+((100*pData->m_Resources[i])/m_StorageSize.m_Resources[i]));
			s3+=tmp+"%";
		}
	}
	strInfo+=s3+" ("+s1+"/"+s2+")";
}

void CSMineSkillType::ModeChanged ( CSUnit *pUnit, BYTE nOldMode )
{
	// Je potreba najit starou skillu ze stareho modu
	// A prekopirovat jeji data k nam
}

//////////////////////////////////////////////////////////////////////
// Debuggovací informace
//////////////////////////////////////////////////////////////////////

#ifdef _DEBUG

// zkontroluje neškodná data skilly
BOOL CSMineSkillType::CheckEmptySkillData ( CSUnit *pUnit ) 
{
	SSMineSkillUnit *pData = SKILLDATA ( pUnit );

	ASSERT(pData->m_pSysCall==NULL);
	ASSERT(pData->m_dwRemainingTime==0);
	ASSERT(pData->m_nDoing==SSMineSkillUnit::nothing);
	ASSERT(pData->m_nState==SSMineSkillUnit::initial);
	ASSERT(pData->m_pUnit==NULL);
	ASSERT(pData->m_bChange==false);

	return TRUE;
}

#endif //_DEBUG
