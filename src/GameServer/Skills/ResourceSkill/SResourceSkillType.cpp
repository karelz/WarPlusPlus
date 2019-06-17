/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Server hry - skilly
 *   Autor: Roman Zenka
 * 
 *   Popis: Skila, pro drzeni resourcu u jednotky
 * 
 ***********************************************************/

#include "StdAfx.h"

#include "SResourceSkillType.h"
#include "SResourceSkill.h"

#include "GameServer\GameServer\SMap.h"
#include "GameServer\GameServer\SCivilization.h"
#include "TripleS\Interpret\Src\SysObjs\ISOResources.h"

// Vytahne z jednotky jeji skill data
#define SKILLDATA(pUnit) (SSResourceSkillUnit *)GetSkillData(pUnit)

// implementace interfacu volání ze skriptù
BEGIN_SKILLCALL_INTERFACE_MAP ( CSResourceSkillType, CSSkillType )
	SKILLCALL_BASECLASS,
	SKILLCALL_BASECLASS,
	SKILLCALL_METHOD(SCI_GetResources)
END_SKILLCALL_INTERFACE_MAP ()

IMPLEMENT_DYNAMIC(CSResourceSkillType, CSSkillType);
// implementace typu skilly
IMPLEMENT_SKILL_TYPE ( CSResourceSkillType, 5, "Resource" );

/////////////////////////////////////////////////////////////////////
// Konstrukce a destrukce
/////////////////////////////////////////////////////////////////////

// konstruktor
CSResourceSkillType::CSResourceSkillType () 
{
}

// destruktor
CSResourceSkillType::~CSResourceSkillType () 
{
}

/////////////////////////////////////////////////////////////////////
// Inicializace a znièení dat objektu
/////////////////////////////////////////////////////////////////////

// inicializuje data typu skilly z dat "pData" velikosti "dwDataSize"
//		výjimky: CPersistentLoadException
void CSResourceSkillType::CreateSkillType ( void *pData, DWORD dwDataSize ) 
{
    SMResourceSkillType *pMData=(SMResourceSkillType*)pData;
    
	int i;

    // Kopie dat
	m_AvailablePerTimeslice.Create(pMData->m_AvailablePerTimeslice);
	for(i=0; i<3; i++) {
		m_dwAppearance[i]=pMData->m_dwAppearance[i];
		m_dwAppearanceMine[i]=pMData->m_dwAppearanceMine[i];
		m_ResourcesLimit[i].Create(pMData->m_ResourcesLimit[i]);
	}

	m_dwAppearanceAfterMining=pMData->m_dwAppearanceAfterMining;
	m_dwTimeslicesAfterMining=pMData->m_dwTimeslicesAfterMining;	

	m_StorageSize.Create(pMData->m_StorageSize);
}

// Dodatecne zavolane po projeti CreateSkillType na vsech skillach
void CSResourceSkillType::PostCreateSkillType()
{
}

// znièí data typu skilly
void CSResourceSkillType::DeleteSkillType () 
{
}

/////////////////////////////////////////////////////////////////////
// Ukládání dat potomkù (analogie CPersistentObject metod)
/////////////////////////////////////////////////////////////////////

/* CSResourceSkillType stored
	
	m_AvailablePerTimeslice stored
	m_dwAppearance[0..2] stored
	m_dwAppearanceMine[0..2] stored
	m_dwApperanceAfterMining
	m_dwTimeslicesAfterMining
	m_ResourcesLimit[0..2] stored
	m_StorageSize stored
*/

// ukládání dat potomka
void CSResourceSkillType::PersistentSaveSkillType ( CPersistentStorage &storage ) 
{
    BRACE_BLOCK(storage);

	int i;

	m_AvailablePerTimeslice.Save(storage);
	for(i=0; i<3; i++) {
		storage << m_dwAppearance[i];
	}
	for(i=0; i<3; i++) {
		storage << m_dwAppearanceMine[i];
	}
	storage << m_dwAppearanceAfterMining;
	storage << m_dwTimeslicesAfterMining;
	for(i=0; i<3; i++) {
		m_ResourcesLimit[i].Save(storage);
	}
	m_StorageSize.Save(storage);
}

// nahrávání pouze uložených dat potomka
void CSResourceSkillType::PersistentLoadSkillType ( CPersistentStorage &storage ) 
{
    BRACE_BLOCK(storage);

	int i;

	m_AvailablePerTimeslice.Load(storage);
	for(i=0; i<3; i++) {
		storage >> (DWORD&)m_dwAppearance[i];
	}
	for(i=0; i<3; i++) {
		storage >> (DWORD&)m_dwAppearanceMine[i];
	}
	storage >> (DWORD&)m_dwAppearanceAfterMining;
	storage >> (DWORD&)m_dwTimeslicesAfterMining;
	for(i=0; i<3; i++) {
		m_ResourcesLimit[i].Load(storage);
	}
	m_StorageSize.Load(storage);
}

// pøeklad ukazatelù potomka
void CSResourceSkillType::PersistentTranslatePointersSkillType ( CPersistentStorage &storage ) 
{
}

// inicializace nahraného objektu potomka
void CSResourceSkillType::PersistentInitSkillType () 
{
}

/////////////////////////////////////////////////////////////////////
// Get/Set metody serverových informací o typu skilly jednotky
/////////////////////////////////////////////////////////////////////

// vrátí velikost dat skilly pro jednotku
DWORD CSResourceSkillType::GetSkillDataSize () 
{
    return sizeof(SSResourceSkillUnit);
}

// vrátí masku skupin skill
DWORD CSResourceSkillType::GetGroupMask () 
{
	return Group_Resource;
}

/////////////////////////////////////////////////////////////////////
// Metody skilly
/////////////////////////////////////////////////////////////////////

// provede TimeSlice na skille jednotky "pUnit"
// jednotka "pUnit" je již zamèená pro zápis, metoda mùže zamykat libovolný poèet jiných
//		jednotek pro zápis/ètení (vèetnì jednotek ze svého bloku)
void CSResourceSkillType::DoTimeSlice ( CSUnit *pUnit ) 
{
	SSResourceSkillUnit *pData = SKILLDATA ( pUnit );

	if(pData->m_dwTimeslicesAfterMining>0) {
		pData->m_dwTimeslicesAfterMining--;
		pUnit->SetAppearanceID(m_dwAppearanceAfterMining);
	} else {
		pUnit->DecreaseLives(pUnit->GetLives());
	}
}

// inicializuje skillu jednotky "pUnit"
void CSResourceSkillType::InitializeSkill ( CSUnit *pUnit ) 
{
	SSResourceSkillUnit *pData = SKILLDATA ( pUnit );

	// Prazdna data pro jednotku
	pData->m_nMiners=0;
	pData->m_Resources=m_StorageSize;
	pData->m_dwTimeslicesAfterMining=0;
	pData->m_nAppearance=0;
}

// znièí skillu jednotky "pUnit"
void CSResourceSkillType::DestructSkill ( CSUnit *pUnit ) 
{
	SSResourceSkillUnit *pData = SKILLDATA ( pUnit );

}

// aktivuje skillu jednotky "pUnit" (jednotka musí být zamèena pro zápis)
// vraci false, kdyz bylo malo resourcu na vybudovani jednotky
BOOL CSResourceSkillType::ActivateSkill(CSUnit *pUnit) 
{
	SSResourceSkillUnit *pData = SKILLDATA ( pUnit );    

	if(m_dwTimeslicesAfterMining==0) {
		pUnit->DecreaseLives(pUnit->GetLives());
		return TRUE;
	}

	// zjistí, je-li skilla již aktivní
	if ( IsSkillActive ( pUnit ) )
	{	// skilla je aktivní
		// nejprve se deaktivuje
		DeactivateSkill ( pUnit );
	}
    
	// deaktivuje nesluèitelné skilly
	GetUnitType()->DeactivateSkillGroup ( pUnit, Group_Move | Group_Attack );

	pData->m_dwTimeslicesAfterMining=m_dwTimeslicesAfterMining;

	pUnit->SetAppearanceID(m_dwAppearanceAfterMining);
	
	// aktivuje skillu na jednotce
	CSSkillType::ActivateSkill ( pUnit );

    return TRUE;
}

// deaktivuje skillu jednotky "pUnit" (jednotka musí být zamèena pro zápis)
void CSResourceSkillType::DeactivateSkill ( CSUnit *pUnit ) 
{
	SSResourceSkillUnit *pData = SKILLDATA ( pUnit );

	// deaktivuje skillu na jednotce
	CSSkillType::DeactivateSkill ( pUnit );
}

/////////////////////////////////////////////////////////////////////
// Ukládání dat od jednotek
/////////////////////////////////////////////////////////////////////

/* ResourceSkill stored
	int m_nMiners;
	m_Resources stored
	DWORD m_dwTimeslicesAfterMining;
	int m_nAppearance;
*/

// ukládání dat skilly
void CSResourceSkillType::PersistentSaveSkill ( CPersistentStorage &storage, CSUnit *pUnit ) 
{
	BRACE_BLOCK(storage);

	SSResourceSkillUnit *pData = SKILLDATA ( pUnit );

	storage << pData->m_nMiners;
	pData->m_Resources.Save(storage);
	storage << pData->m_dwTimeslicesAfterMining;
	storage << pData->m_nAppearance;
}

// nahrávání pouze uložených dat skilly
void CSResourceSkillType::PersistentLoadSkill ( CPersistentStorage &storage, CSUnit *pUnit ) 
{
	BRACE_BLOCK(storage);

	SSResourceSkillUnit *pData = SKILLDATA ( pUnit );

	storage >> pData->m_nMiners;
	pData->m_Resources.Load(storage);
	storage >> (DWORD&)pData->m_dwTimeslicesAfterMining;
	storage >> (int&)pData->m_nAppearance;
}

// pøeklad ukazatelù dat skilly
void CSResourceSkillType::PersistentTranslatePointersSkill ( CPersistentStorage &storage, CSUnit *pUnit ) 
{
	SSResourceSkillUnit *pData = SKILLDATA ( pUnit );
}

// inicializace nahraných dat skilly
void CSResourceSkillType::PersistentInitSkill ( CSUnit *pUnit ) 
{
	SSResourceSkillUnit *pData = SKILLDATA ( pUnit );
}

// Volano mine skillou, kdyz se z nas zacalo tezit
// Predpoklada ze, ze nase jednotka je zamcena na zapis
void CSResourceSkillType::LoadStarted(CSUnit *pUnit) {
	SSResourceSkillUnit *pData = SKILLDATA ( pUnit );
	pData->m_nMiners++;
	
	if(pData->m_nMiners>0) {
		ASSERT(pData->m_nAppearance>=0 && pData->m_nAppearance<3);
		if(!pData->m_Resources.IsEmpty()) {
			pUnit->SetAppearanceID(m_dwAppearanceMine[pData->m_nAppearance]);
		}
	}
}

// Volano mine skillou, kdyz se z nas prestalo tezit
void CSResourceSkillType::LoadEnded(CSUnit *pUnit) {
	SSResourceSkillUnit *pData = SKILLDATA ( pUnit );
	pData->m_nMiners--;

	if(pData->m_nMiners<=0) {
		ASSERT(pData->m_nAppearance>=0 && pData->m_nAppearance<3);	
		if(!pData->m_Resources.IsEmpty()) {
			pUnit->SetAppearanceID(m_dwAppearance[pData->m_nAppearance]);
		}
	}
}

// Volano mine skillou, kdyz se z nas tezi
CSResourceSkillType::ELoadResult CSResourceSkillType::LoadResources(CSUnit *pMinedUnit, CSResources &Storage, CSResources &StorageSize, CSResources &LoadPerTimeslice)
{
	SSResourceSkillUnit *pData = SKILLDATA ( pMinedUnit );

	if(pMinedUnit->GetLives()<=0) {
		// Tezena jednotka umrela
		return didMyBest;
	}

	// Do Storage, ktera muze byt max StorageSize velka, je treba nahrat nase resourcy,
	// pricemz jednotka nezvlada za timeslice vice nez LoadPerTimeslice

	// Tolik by jich jednotka jeste chtela
	CSResources loadAmount=StorageSize-Storage;

	if(loadAmount.IsEmpty()) {
		// Spokojena jednotka
		return allNeedsSatisfied;
	}

	// Tolik jich pojme ona sama najednou
	loadAmount.Min(LoadPerTimeslice);

	if(loadAmount.IsEmpty()) {
		// Jednotka uz vic nepojme
		// To je chyba, nemelo by se stat, ze jednotka neco chce,
		// to neco je k dispozici, ale neni schopna to nalozit
		ASSERT(FALSE);
		return error;
	}

	// A tolik ji muzeme dat my najednou
	loadAmount.Min(m_AvailablePerTimeslice);
	
	// A jeste omezime tim, co mame
	loadAmount.Min(pData->m_Resources);

	// Takze jsme nyni urcili, kolik muzeme dat
	if(loadAmount.IsEmpty()) {
		// Nemuzeme dat nic, a jednotka jeste chce
		// Neni to nase chyba
		return didMyBest;
	}

	// Nalozime danou davku
	Storage+=loadAmount;

	// Odstranime ji sobe
	pData->m_Resources-=loadAmount;

	// Otestujeme, jestli jsme neklesli pod limit
	if(pData->m_nAppearance<2) {
		if(pData->m_Resources<=m_ResourcesLimit[pData->m_nAppearance]) {
			pData->m_nAppearance++;
			pMinedUnit->SetAppearanceID(m_dwAppearanceMine[pData->m_nAppearance]);
		}
	}

	// Pokud dosly resourcy nam, umirame
	if(pData->m_Resources.IsEmpty()) {		
		ActivateSkill(pMinedUnit);
	}

	// A nyni, jestlize jsme jednotku nalozili naplno
	if(Storage==StorageSize) {
		return allNeedsSatisfied;
	} else {
		// Nenalozili jsme ji naplno
		// tvrdime, ze jeste jsme schopni nakladat, bud je to pravda,
		// nebo se na to v pristim kole prijde
		return canOfferMore;
	}

	return error;
}


/////////////////////////////////////////////////////////////////////
// Interface volání ze skriptù
/////////////////////////////////////////////////////////////////////

ESyscallResult CSResourceSkillType::SCI_GetResources (CSUnit *pUnit, CIProcess *pProcess, CISyscall **ppSysCall, CIBag *pBag, bool bCalledByEnemy, CIDataStackItem *pReturn)
{
	CISOResources *pResources;
	SSResourceSkillUnit *pData = SKILLDATA ( pUnit );

	pResources = (CISOResources *)pProcess->m_pInterpret->CreateSystemObject(SO_RESOURCES);

	if (pResources == NULL) {
		return SYSCALL_ERROR;
	}

	pResources->Set(pData->m_Resources);
	pReturn->Set(pResources);
	pResources->Release(); 

	return SYSCALL_SUCCESS;
}

void CSResourceSkillType::FillFullInfo ( CSUnit *pUnit, CString &strInfo )
{
	int i, j;
	SSResourceSkillUnit *pData = (SSResourceSkillUnit *)GetSkillData ( pUnit );

	strInfo="Resource: ";
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

void CSResourceSkillType::FillEnemyFullInfo ( CSUnit *pUnit, CString &strInfo )
{
	FillFullInfo(pUnit, strInfo);
}

/////////////////////////////////////////////////////////////////////
// Pomocné metody skilly
/////////////////////////////////////////////////////////////////////

// ukonci skilu
void CSResourceSkillType::Finish ( CSUnit *pUnit ) 
{
	SSResourceSkillUnit *pData = (SSResourceSkillUnit *)GetSkillData ( pUnit );

	// deaktivuje skillu na jednotce
	CSSkillType::DeactivateSkill ( pUnit );
}

//////////////////////////////////////////////////////////////////////
// Debuggovací informace
//////////////////////////////////////////////////////////////////////

#ifdef _DEBUG

// zkontroluje neškodná data skilly
BOOL CSResourceSkillType::CheckEmptySkillData ( CSUnit *pUnit ) 
{
	SSResourceSkillUnit *pData = SKILLDATA ( pUnit );

	ASSERT(pData->m_nMiners==0);
	ASSERT(pData->m_dwTimeslicesAfterMining==0);

	return TRUE;
}

#endif //_DEBUG
