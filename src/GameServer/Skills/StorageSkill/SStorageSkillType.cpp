/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Server hry - skilly
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

// implementace interfacu vol�n� ze skript�
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
// Inicializace a zni�en� dat objektu
/////////////////////////////////////////////////////////////////////

// inicializuje data typu skilly z dat "pData" velikosti "dwDataSize"
//		v�jimky: CPersistentLoadException
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

// zni�� data typu skilly
void CSStorageSkillType::DeleteSkillType () 
{
}

/////////////////////////////////////////////////////////////////////
// Ukl�d�n� dat potomk� (analogie CPersistentObject metod)
/////////////////////////////////////////////////////////////////////

/* CSStorageSkillType stored
	DWORD m_dwAppearanceWhileUnloading;
	m_AcceptedPerTimeslice stored

*/

// ukl�d�n� dat potomka
void CSStorageSkillType::PersistentSaveSkillType ( CPersistentStorage &storage ) 
{
    BRACE_BLOCK(storage);

	storage << m_dwAppearanceWhileUnloading;
	m_AcceptedPerTimeslice.Save(storage);
}

// nahr�v�n� pouze ulo�en�ch dat potomka
void CSStorageSkillType::PersistentLoadSkillType ( CPersistentStorage &storage ) 
{
    BRACE_BLOCK(storage);

	storage >> m_dwAppearanceWhileUnloading;
	m_AcceptedPerTimeslice.Load(storage);
}

// p�eklad ukazatel� potomka
void CSStorageSkillType::PersistentTranslatePointersSkillType ( CPersistentStorage &storage ) 
{
}

// inicializace nahran�ho objektu potomka
void CSStorageSkillType::PersistentInitSkillType () 
{
}

/////////////////////////////////////////////////////////////////////
// Get/Set metody serverov�ch informac� o typu skilly jednotky
/////////////////////////////////////////////////////////////////////

// vr�t� velikost dat skilly pro jednotku
DWORD CSStorageSkillType::GetSkillDataSize () 
{
    return sizeof(SSStorageSkillUnit);
}

// vr�t� masku skupin skill
DWORD CSStorageSkillType::GetGroupMask () 
{
	return Group_Storage;
}

/////////////////////////////////////////////////////////////////////
// Metody skilly
/////////////////////////////////////////////////////////////////////

// provede TimeSlice na skille jednotky "pUnit"
// jednotka "pUnit" je ji� zam�en� pro z�pis, metoda m��e zamykat libovoln� po�et jin�ch
//		jednotek pro z�pis/�ten� (v�etn� jednotek ze sv�ho bloku)
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

// zni�� skillu jednotky "pUnit"
void CSStorageSkillType::DestructSkill ( CSUnit *pUnit ) 
{
	SSStorageSkillUnit *pData = SKILLDATA ( pUnit );
}

// aktivuje skillu jednotky "pUnit" (jednotka mus� b�t zam�ena pro z�pis)
// vraci false, kdyz bylo malo resourcu na vybudovani jednotky
BOOL CSStorageSkillType::ActivateSkill(CSUnit *pUnit, CZUnit *pStorageUnit) 
{
	SSStorageSkillUnit *pData = SKILLDATA ( pUnit );    

	// zjist�, je-li skilla ji� aktivn�
	if ( IsSkillActive ( pUnit ) )
	{	// skilla je aktivn�
		// nejprve se deaktivuje
		DeactivateSkill ( pUnit );
	}
    
	// deaktivuje neslu�iteln� skilly
	GetUnitType()->DeactivateSkillGroup ( pUnit, Group_Move | Group_Attack );

	// Zkontrolujeme data na jednotce na prazdnost
	ASSERT ( CheckEmptySkillData ( pUnit ) );

	// aktivuje skillu na jednotce
	CSSkillType::ActivateSkill ( pUnit );

    return TRUE;
}

// deaktivuje skillu jednotky "pUnit" (jednotka mus� b�t zam�ena pro z�pis)
void CSStorageSkillType::DeactivateSkill ( CSUnit *pUnit ) 
{
	SSStorageSkillUnit *pData = SKILLDATA ( pUnit );

	// Zmena appearance na default
	pUnit->SetDefaultAppearanceID();

	// deaktivuje skillu na jednotce
	CSSkillType::DeactivateSkill ( pUnit );
}

/////////////////////////////////////////////////////////////////////
// Ukl�d�n� dat od jednotek
/////////////////////////////////////////////////////////////////////

/* StorageSkill stored
	int m_nUnloaders
*/

// ukl�d�n� dat skilly
void CSStorageSkillType::PersistentSaveSkill ( CPersistentStorage &storage, CSUnit *pUnit ) 
{
	BRACE_BLOCK(storage);

	SSStorageSkillUnit *pData = SKILLDATA ( pUnit );
	
	storage << pData->m_nUnloaders;
}

// nahr�v�n� pouze ulo�en�ch dat skilly
void CSStorageSkillType::PersistentLoadSkill ( CPersistentStorage &storage, CSUnit *pUnit ) 
{
	BRACE_BLOCK(storage);

	SSStorageSkillUnit *pData = SKILLDATA ( pUnit );
	
	storage >> pData->m_nUnloaders;
}

// p�eklad ukazatel� dat skilly
void CSStorageSkillType::PersistentTranslatePointersSkill ( CPersistentStorage &storage, CSUnit *pUnit ) 
{
	SSStorageSkillUnit *pData = SKILLDATA ( pUnit );
}

// inicializace nahran�ch dat skilly
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
// Pomocn� metody skilly
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
// Debuggovac� informace
//////////////////////////////////////////////////////////////////////

#ifdef _DEBUG

// zkontroluje ne�kodn� data skilly
BOOL CSStorageSkillType::CheckEmptySkillData ( CSUnit *pUnit ) 
{
	SSStorageSkillUnit *pData = SKILLDATA ( pUnit );

	ASSERT(pData->m_nUnloaders==0);

	return TRUE;
}

#endif //_DEBUG
