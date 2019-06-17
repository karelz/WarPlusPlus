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

#include "SStorageSkillType.h"
#include "SStorageSkill.h"

#include "ISysCallStorage.h"

#include "GameServer\GameServer\SMap.h"
#include "GameServer\GameServer\SCivilization.h"
#include "GameServer\Civilization\Src\ZUnit.h"
#include "TripleS\Interpret\Src\SysObjs\ISOResources.h"

// Vytahne z jednotky jeji skill data
#define SKILLDATA(pUnit) (SSStorageSkillUnit *)GetSkillData(pUnit)

// implementace interfacu volání ze skriptù
BEGIN_SKILLCALL_INTERFACE_MAP ( CSStorageSkillType, CSSkillType )
	SKILLCALL_BASECLASS,
	SKILLCALL_BASECLASS,
END_SKILLCALL_INTERFACE_MAP ()

IMPLEMENT_DYNAMIC(CSStorageSkillType, CSSkillType);
// implementace typu skilly
IMPLEMENT_SKILL_TYPE ( CSStorageSkillType, 6, "Storage" );

/////////////////////////////////////////////////////////////////////
// Konstrukce a destrukce
/////////////////////////////////////////////////////////////////////

// konstruktor
CSStorageSkillType::CSStorageSkillType () 
{
}

// destruktor
CSStorageSkillType::~CSStorageSkillType () 
{
}

/////////////////////////////////////////////////////////////////////
// Inicializace a znièení dat objektu
/////////////////////////////////////////////////////////////////////

// inicializuje data typu skilly z dat "pData" velikosti "dwDataSize"
//		výjimky: CPersistentLoadException
void CSStorageSkillType::CreateSkillType ( void *pData, DWORD dwDataSize ) 
{
    SMStorageSkillType *pMData=(SMStorageSkillType*)pData;
    
    // Kopie dat
	m_AcceptedPerTimeslice.Create(pMData->m_AcceptedPerTimeslice);
	m_dwAppearanceWhileUnloading=pMData->m_dwAppearanceWhileUnloading;
}

// Dodatecne zavolane po projeti CreateSkillType na vsech skillach
void CSStorageSkillType::PostCreateSkillType()
{
}

// znièí data typu skilly
void CSStorageSkillType::DeleteSkillType () 
{
}

/////////////////////////////////////////////////////////////////////
// Ukládání dat potomkù (analogie CPersistentObject metod)
/////////////////////////////////////////////////////////////////////

/* CSStorageSkillType stored
	DWORD m_dwAppearanceWhileUnloading;
	m_AcceptedPerTimeslice stored

*/

// ukládání dat potomka
void CSStorageSkillType::PersistentSaveSkillType ( CPersistentStorage &storage ) 
{
    BRACE_BLOCK(storage);

	storage << m_dwAppearanceWhileUnloading;
	m_AcceptedPerTimeslice.Save(storage);
}

// nahrávání pouze uložených dat potomka
void CSStorageSkillType::PersistentLoadSkillType ( CPersistentStorage &storage ) 
{
    BRACE_BLOCK(storage);

	storage >> m_dwAppearanceWhileUnloading;
	m_AcceptedPerTimeslice.Load(storage);
}

// pøeklad ukazatelù potomka
void CSStorageSkillType::PersistentTranslatePointersSkillType ( CPersistentStorage &storage ) 
{
}

// inicializace nahraného objektu potomka
void CSStorageSkillType::PersistentInitSkillType () 
{
}

/////////////////////////////////////////////////////////////////////
// Get/Set metody serverových informací o typu skilly jednotky
/////////////////////////////////////////////////////////////////////

// vrátí velikost dat skilly pro jednotku
DWORD CSStorageSkillType::GetSkillDataSize () 
{
    return sizeof(SSStorageSkillUnit);
}

// vrátí masku skupin skill
DWORD CSStorageSkillType::GetGroupMask () 
{
	return Group_Storage;
}

/////////////////////////////////////////////////////////////////////
// Metody skilly
/////////////////////////////////////////////////////////////////////

// provede TimeSlice na skille jednotky "pUnit"
// jednotka "pUnit" je již zamèená pro zápis, metoda mùže zamykat libovolný poèet jiných
//		jednotek pro zápis/ètení (vèetnì jednotek ze svého bloku)
void CSStorageSkillType::DoTimeSlice ( CSUnit *pUnit ) 
{
	SSStorageSkillUnit *pData = SKILLDATA ( pUnit );
	
	// Nikdo nas nikdy nemuze zaktivovat
	ASSERT(FALSE);
}

// inicializuje skillu jednotky "pUnit"
void CSStorageSkillType::InitializeSkill ( CSUnit *pUnit ) 
{
	SSStorageSkillUnit *pData = SKILLDATA ( pUnit );

	pData->m_nUnloaders=0;
}

// znièí skillu jednotky "pUnit"
void CSStorageSkillType::DestructSkill ( CSUnit *pUnit ) 
{
	SSStorageSkillUnit *pData = SKILLDATA ( pUnit );
}

// aktivuje skillu jednotky "pUnit" (jednotka musí být zamèena pro zápis)
// vraci false, kdyz bylo malo resourcu na vybudovani jednotky
BOOL CSStorageSkillType::ActivateSkill(CSUnit *pUnit, CZUnit *pStorageUnit) 
{
	SSStorageSkillUnit *pData = SKILLDATA ( pUnit );    

	// zjistí, je-li skilla již aktivní
	if ( IsSkillActive ( pUnit ) )
	{	// skilla je aktivní
		// nejprve se deaktivuje
		DeactivateSkill ( pUnit );
	}
    
	// deaktivuje nesluèitelné skilly
	GetUnitType()->DeactivateSkillGroup ( pUnit, Group_Move | Group_Attack );

	// Zkontrolujeme data na jednotce na prazdnost
	ASSERT ( CheckEmptySkillData ( pUnit ) );

	// aktivuje skillu na jednotce
	CSSkillType::ActivateSkill ( pUnit );

    return TRUE;
}

// deaktivuje skillu jednotky "pUnit" (jednotka musí být zamèena pro zápis)
void CSStorageSkillType::DeactivateSkill ( CSUnit *pUnit ) 
{
	SSStorageSkillUnit *pData = SKILLDATA ( pUnit );

	// Zmena appearance na default
	pUnit->SetDefaultAppearanceID();

	// deaktivuje skillu na jednotce
	CSSkillType::DeactivateSkill ( pUnit );
}

/////////////////////////////////////////////////////////////////////
// Ukládání dat od jednotek
/////////////////////////////////////////////////////////////////////

/* StorageSkill stored
	int m_nUnloaders
*/

// ukládání dat skilly
void CSStorageSkillType::PersistentSaveSkill ( CPersistentStorage &storage, CSUnit *pUnit ) 
{
	BRACE_BLOCK(storage);

	SSStorageSkillUnit *pData = SKILLDATA ( pUnit );
	
	storage << pData->m_nUnloaders;
}

// nahrávání pouze uložených dat skilly
void CSStorageSkillType::PersistentLoadSkill ( CPersistentStorage &storage, CSUnit *pUnit ) 
{
	BRACE_BLOCK(storage);

	SSStorageSkillUnit *pData = SKILLDATA ( pUnit );
	
	storage >> pData->m_nUnloaders;
}

// pøeklad ukazatelù dat skilly
void CSStorageSkillType::PersistentTranslatePointersSkill ( CPersistentStorage &storage, CSUnit *pUnit ) 
{
	SSStorageSkillUnit *pData = SKILLDATA ( pUnit );
}

// inicializace nahraných dat skilly
void CSStorageSkillType::PersistentInitSkill ( CSUnit *pUnit ) 
{
	SSStorageSkillUnit *pData = SKILLDATA ( pUnit );	
}

// Volano mine skillou, kdyz se do nas zacalo unloadit
// Predpoklada ze, ze nase jednotka je zamcena na zapis
void CSStorageSkillType::UnloadStarted(CSUnit *pUnit) {
	SSStorageSkillUnit *pData = SKILLDATA ( pUnit );
	pData->m_nUnloaders++;

	if(pData->m_nUnloaders>0) {
		pUnit->SetAppearanceID(m_dwAppearanceWhileUnloading);
	}
}

// Volano mine skillou, kdyz se z nas prestalo unloadit
void CSStorageSkillType::UnloadEnded(CSUnit *pUnit) {
	SSStorageSkillUnit *pData = SKILLDATA ( pUnit );
	pData->m_nUnloaders--;

	if(pData->m_nUnloaders<=0) {
		pUnit->SetDefaultAppearanceID();
	}
}

CSStorageSkillType::EStoreResult CSStorageSkillType::StoreResources(CSUnit *pStorageUnit, CSResources &Storage, CSResources &UnloadPerTimeslice)
{
	SSStorageSkillUnit *pData = SKILLDATA ( pStorageUnit );

	if(pStorageUnit->GetLives()<=0) {
		// Storage umrela
		return didMyBest;
	}

	// Tolik by jich jednotka jeste chtela unloadnout
	CSResources unloadAmount=Storage;

	if(unloadAmount.IsEmpty()) {
		// Spokojena jednotka
		return allNeedsSatisfied;
	}

	// Tolik jich ona muze unloadnout najednou
	unloadAmount.Min(UnloadPerTimeslice);

	if(unloadAmount.IsEmpty()) {
		// Jednotka uz vic neunloadne
		// To je chyba, nemelo by se stat, ze jednotka neco chce,
		// to neco ma, ale neni schopna to vylozit
		ASSERT(FALSE);
		return error;
	}

	// A tolik muzeme vzit MY najednou
	unloadAmount.Min(m_AcceptedPerTimeslice);
	
	// Takze jsme nyni urcili, kolik muzeme vzit
	if(unloadAmount.IsEmpty()) {
		// Nemuzeme vzit nic, a jednotka jeste chce
		// Neni to nase chyba
		return didMyBest;
	}

	// Vylozime danou davku
	Storage-=unloadAmount;

	// Pridame ji civilizaci
    VERIFY(pStorageUnit->GetCivilization()->GetCivilizationLock()->WriterLock());
	pStorageUnit->GetCivilization()->AddResources(unloadAmount);
	pStorageUnit->GetCivilization()->GetCivilizationLock()->WriterUnlock();

	// A nyni, jestlize jsme jednotku vylozili uplne
	if(Storage.IsEmpty()) {
		return allNeedsSatisfied;
	} else {
		// Nevylozili jsme ji uplne
		// tvrdime, ze jeste jsme schopni vykladat, bud je to pravda,
		// nebo se na to v pristim kole prijde
		return canAcceptMore;
	}

	return error;
}

void CSStorageSkillType::FillFullInfo ( CSUnit *pUnit, CString &strInfo )
{
	int i, j;
	strInfo="Storage: ";
	CString tmp;
	j=0;
	for(i=0; i<RESOURCE_COUNT; i++) {
		if(m_AcceptedPerTimeslice.m_Resources[i]>0) {
			tmp.Format("#%d", i+1);
			j++;
			if(j==1) strInfo+="accepts ";
			if(j>1) strInfo+=",";
			strInfo+=tmp;
		}
	}
	if(j==0) strInfo="doesn't accept any resources";
}

void CSStorageSkillType::FillEnemyFullInfo ( CSUnit *pUnit, CString &strInfo )
{
	strInfo="";
}


/////////////////////////////////////////////////////////////////////
// Pomocné metody skilly
/////////////////////////////////////////////////////////////////////

// ukonci skilu
void CSStorageSkillType::Finish ( CSUnit *pUnit ) 
{
	SSStorageSkillUnit *pData = (SSStorageSkillUnit *)GetSkillData ( pUnit );

	// Zmena appearance na default
	pUnit->SetDefaultAppearanceID();

	// deaktivuje skillu na jednotce
	CSSkillType::DeactivateSkill ( pUnit );
}

//////////////////////////////////////////////////////////////////////
// Debuggovací informace
//////////////////////////////////////////////////////////////////////

#ifdef _DEBUG

// zkontroluje neškodná data skilly
BOOL CSStorageSkillType::CheckEmptySkillData ( CSUnit *pUnit ) 
{
	SSStorageSkillUnit *pData = SKILLDATA ( pUnit );

	ASSERT(pData->m_nUnloaders==0);

	return TRUE;
}

#endif //_DEBUG
