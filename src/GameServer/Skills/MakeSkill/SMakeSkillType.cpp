/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Server hry - skilly
 *   Autor: Roman Zenka
 * 
 *   Popis: Skila pro vytvareni jednotek
 * 
 ***********************************************************/

#include "StdAfx.h"

#include "SMakeSkillType.h"
#include "SMakeSkill.h"

#include "ISysCallMake.h"

#include "GameServer\GameServer\SMap.h"
#include "GameServer\GameServer\SCivilization.h"
#include "GameServer\Civilization\Src\ZUnit.h"
#include "TripleS\Interpret\Src\SysObjs\ISOString.h"
#include "TripleS\Interpret\Src\SysObjs\ISOResources.h"

// Vytahne z jednotky jeji skill data
#define SKILLDATA(pUnit) (SSMakeSkillUnit *)GetSkillData(pUnit)

// implementace interfacu volání ze skriptù
BEGIN_SKILLCALL_INTERFACE_MAP ( CSMakeSkillType, CSSkillType )
	SKILLCALL_BASECLASS,
	SKILLCALL_BASECLASS,
    SKILLCALL_METHOD(SCI_Make),
    SKILLCALL_METHOD(SCI_GetCost)
END_SKILLCALL_INTERFACE_MAP ()

// implementace typu skilly
IMPLEMENT_SKILL_TYPE ( CSMakeSkillType, 3, "Make" );

/////////////////////////////////////////////////////////////////////
// Konstrukce a destrukce
/////////////////////////////////////////////////////////////////////

// konstruktor
CSMakeSkillType::CSMakeSkillType () 
{
    m_nRecords=0;
    m_pRecords=NULL;
}

// destruktor
CSMakeSkillType::~CSMakeSkillType () 
{
    ASSERT(m_pRecords==NULL);
}

/////////////////////////////////////////////////////////////////////
// Inicializace a znièení dat objektu
/////////////////////////////////////////////////////////////////////

// inicializuje data typu skilly z dat "pData" velikosti "dwDataSize"
//		výjimky: CPersistentLoadException
void CSMakeSkillType::CreateSkillType ( void *pData, DWORD dwDataSize ) 
{
    SMMakeSkillType *pMData=(SMMakeSkillType*)pData;
    
    // Kopie dat
    m_nRecords=pMData->m_nRecords;

    // Alokujeme pole zaznamu
    ASSERT(m_pRecords==NULL);
    m_pRecords=new SSMakeSkillRecord[m_nRecords];

    SSMakeSkillRecord *pRecord=m_pRecords;
    SMMakeSkillTypeRecord *pMRecord=(SMMakeSkillTypeRecord*)(((char *)pMData)+sizeof(pMData));
    for(int i=0; i<pMData->m_nRecords; i++) {
        // Vlastni kopie dat
        pRecord->m_dwAppearance=pMRecord->m_dwAppearance;
        pRecord->m_nTimeToMake=pMRecord->m_nTimeToMake;
        
        // Protoze nemuzeme jen tak prelozit ukazatel na unit type
        // (to udela PostCreateSkillType), prekopirujeme si ID do ukazatele        
        pRecord->m_pUnitType=(CSUnitType*)pMRecord->m_dwUnitType;

        memcpy(pRecord->m_ResourcesNeeded, pMRecord->m_ResourcesNeeded, sizeof(TSResources));
            
        pMRecord=(SMMakeSkillTypeRecord*)(((char *)pMRecord)+sizeof(SMMakeSkillTypeRecord));
        pRecord++;
    }
}

void CSMakeSkillType::PostCreateSkillType()
{
    SSMakeSkillRecord *pRecord=m_pRecords;
    for(int i=0; i<m_nRecords; i++) {
        pRecord->m_pUnitType=GetUnitType()->GetCivilization()->GetUnitTypeByID((DWORD)pRecord->m_pUnitType);
        pRecord++;
    }
}

// znièí data typu skilly
void CSMakeSkillType::DeleteSkillType () 
{
    if(m_pRecords!=NULL) {
        delete [] m_pRecords;
        m_pRecords=NULL;
    }
    m_nRecords=0;
}

/////////////////////////////////////////////////////////////////////
// Ukládání dat potomkù (analogie CPersistentObject metod)
/////////////////////////////////////////////////////////////////////

/* CSMakeSkillType stored

int nRecords;
if(nRecords!=0) 
    all data of m_pRecords stored

*/

// ukládání dat potomka
void CSMakeSkillType::PersistentSaveSkillType ( CPersistentStorage &storage ) 
{
    BRACE_BLOCK(storage);

    storage << m_nRecords;

    if(m_nRecords!=0) {
        storage.Write(m_pRecords, sizeof(SSMakeSkillRecord)*m_nRecords);
    }
}

// nahrávání pouze uložených dat potomka
void CSMakeSkillType::PersistentLoadSkillType ( CPersistentStorage &storage ) 
{
    BRACE_BLOCK(storage);
    
    ASSERT(m_pRecords==NULL);

    storage >> m_nRecords;
    ASSERT(m_nRecords>=0);

    if(m_nRecords!=0) {
        m_pRecords=new SSMakeSkillRecord[m_nRecords];
        storage.Read(m_pRecords, sizeof(SSMakeSkillRecord)*m_nRecords);
    } else {
        m_pRecords=NULL;
    }
}

// pøeklad ukazatelù potomka
void CSMakeSkillType::PersistentTranslatePointersSkillType ( CPersistentStorage &storage ) 
{
    SSMakeSkillRecord *pRecord=m_pRecords;
    for(int i=0; i<m_nRecords; i++) {
        pRecord->m_pUnitType=(CSUnitType*)storage.TranslatePointer(pRecord->m_pUnitType);
        pRecord++;
    }
}

// inicializace nahraného objektu potomka
void CSMakeSkillType::PersistentInitSkillType () 
{
}

/////////////////////////////////////////////////////////////////////
// Get/Set metody serverových informací o typu skilly jednotky
/////////////////////////////////////////////////////////////////////

// vrátí velikost dat skilly pro jednotku
DWORD CSMakeSkillType::GetSkillDataSize () 
{
    return sizeof(SSMakeSkillUnit);
}

// vrátí masku skupin skill
DWORD CSMakeSkillType::GetGroupMask () 
{
	return Group_None;
}

/////////////////////////////////////////////////////////////////////
// Metody skilly
/////////////////////////////////////////////////////////////////////

// provede TimeSlice na skille jednotky "pUnit"
// jednotka "pUnit" je již zamèená pro zápis, metoda mùže zamykat libovolný poèet jiných
//		jednotek pro zápis/ètení (vèetnì jednotek ze svého bloku)
void CSMakeSkillType::DoTimeSlice ( CSUnit *pUnit ) 
{
	SSMakeSkillUnit *pData = SKILLDATA ( pUnit );
    
    pData->m_nTimeRemaining--;

    // Zrovna se nam jednotka dodelala
    if(pData->m_nTimeRemaining<=0) {
        CSUnit *pNewUnit;
        CSUnitType *pNewUnitType;
        
        pNewUnitType=pData->m_pUnitType;
        ASSERT(pNewUnitType!=NULL);

        // TODO: zmenit smer pri vytvareni (posledni 0)
        pNewUnit=pNewUnitType->CreateUnit(pUnit->GetPosition(), pData->m_pCommander, pData->m_pScriptName->Get(), 0);

        Finish(pUnit, pNewUnit);
    }
}

// inicializuje skillu jednotky "pUnit"
void CSMakeSkillType::InitializeSkill ( CSUnit *pUnit ) 
{
	SSMakeSkillUnit *pData = SKILLDATA ( pUnit );

    // Prazdna data pro jednotku
    pData->m_nTimeRemaining=0;
	pData->m_nTimeTotal=0;
    pData->m_pUnitType=NULL;
    pData->m_pScriptName=NULL;
    pData->m_pCommander=NULL;
    pData->m_pSysCall=NULL;
}

// znièí skillu jednotky "pUnit"
void CSMakeSkillType::DestructSkill ( CSUnit *pUnit ) 
{
	SSMakeSkillUnit *pData = SKILLDATA ( pUnit );

    // TODO: Vracet resourcy ????

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
// vraci false, kdyz bylo malo resourcu na vybudovani jednotky
BOOL CSMakeSkillType::ActivateSkill(CSUnit *pUnit, SSMakeSkillRecord *pMakeSkillRecord, 
						CISOString *pScriptName, CZUnit *pCommander, CISysCallMake *pSysCall) 
{
	ASSERT(pMakeSkillRecord!=NULL);
    SSMakeSkillUnit *pData = SKILLDATA ( pUnit );    

	// zjistí, je-li skilla již aktivní
	if ( IsSkillActive ( pUnit ) )
	{	// skilla je aktivní
		// nejprve se deaktivuje
		DeactivateSkill ( pUnit );
	}

    // Pokusime se sezrat resourcy
    VERIFY(pUnit->GetCivilization()->GetCivilizationLock()->WriterLock());
    // Jestlize neni dost resourcu
    if(!pUnit->GetCivilization()->CanRemoveResources(pMakeSkillRecord->m_ResourcesNeeded))
    {
        pUnit->GetCivilization()->GetCivilizationLock()->WriterUnlock();
        return FALSE;
    } else {
        // Je dost resourcu
        // Tak je odebereme
        pUnit->GetCivilization()->RemoveResources(pMakeSkillRecord->m_ResourcesNeeded);
        pUnit->GetCivilization()->GetCivilizationLock()->WriterUnlock();
    }
    
	// deaktivuje nesluèitelné skilly
	// GetUnitType()->DeactivateSkillGroup ( pUnit, Group_Move | Group_Attack );

	// Zkontrolujeme data na jednotce na prazdnost
    ASSERT ( CheckEmptySkillData ( pUnit ) );

    // Vyplneni dat
    pData->m_nTimeRemaining=pMakeSkillRecord->m_nTimeToMake;
	pData->m_nTimeTotal=pMakeSkillRecord->m_nTimeToMake;
    pData->m_pUnitType=pMakeSkillRecord->m_pUnitType;
    pScriptName->AddRef();
    pData->m_pScriptName = pScriptName;  
    pData->m_pCommander=pCommander; pCommander->AddRef(); // Aby nam commander nevysumel 
    pSysCall->AddRef();
    pData->m_pSysCall=pSysCall;

    // Zmena appearance
    pUnit->SetAppearanceID(pMakeSkillRecord->m_dwAppearance);

	// aktivuje skillu na jednotce
	CSSkillType::ActivateSkill ( pUnit );

    return TRUE;
}

// deaktivuje skillu jednotky "pUnit" (jednotka musí být zamèena pro zápis)
void CSMakeSkillType::DeactivateSkill ( CSUnit *pUnit ) 
{
	SSMakeSkillUnit *pData = SKILLDATA ( pUnit );
    int i;

    // Najdeme zaznam o unittypu, ktery jsme vytvareli
    SSMakeSkillRecord *pRecord=m_pRecords;
    for(i=0; i<m_nRecords; i++) {
        if(pRecord->m_pUnitType==pData->m_pUnitType) {
            // Vyskocime z cyklu, nasli jsme spravny zaznam
            break;
        }
        pRecord++;
    }
    
    ASSERT(i<m_nRecords);

    // Vratime resourcy
    VERIFY(pUnit->GetCivilization()->GetCivilizationLock()->WriterLock());       
    pUnit->GetCivilization()->AddResources(pRecord->m_ResourcesNeeded);
    pUnit->GetCivilization()->GetCivilizationLock()->WriterUnlock();

    pData->m_nTimeRemaining=0;
	pData->m_nTimeTotal=0;
    pData->m_pUnitType=NULL;
    if(pData->m_pScriptName != NULL){
      pData->m_pScriptName->Release();
      pData->m_pScriptName = NULL;
    }
    if(pData->m_pCommander!=NULL) {
        pData->m_pCommander->Release();
        pData->m_pCommander=NULL;
    }

	// zjistí, je-li asociováno systémové volání
	if ( pData->m_pSysCall != NULL )
	{	// je asociování systémové volání
		// ukonèí èekající systémové volání
		pData->m_pSysCall->FinishWaiting ( STATUS_SKILL_COLLISION );
		pData->m_pSysCall->Release();
		pData->m_pSysCall = NULL;
	}

    // Zmena appearance na default
    pUnit->SetDefaultAppearanceID();

	// deaktivuje skillu na jednotce
	CSSkillType::DeactivateSkill ( pUnit );
}

/////////////////////////////////////////////////////////////////////
// Ukládání dat od jednotek
/////////////////////////////////////////////////////////////////////

/* MakeSkill stored
    DWORD m_dwTimeRemaining;
	DWORD m_dwTimeTotal;
    CSUnitType m_pUnitType;
    CZUnit *m_pCommander
    BOOL bSysCall=m_pSysCall!=NULL
    if(bSysCall)
      m_pSysCall stored;
*/

// ukládání dat skilly
void CSMakeSkillType::PersistentSaveSkill ( CPersistentStorage &storage, CSUnit *pUnit ) 
{
    BRACE_BLOCK(storage);
    
    SSMakeSkillUnit *pData = SKILLDATA ( pUnit );

    storage << pData->m_nTimeRemaining;
	storage << pData->m_nTimeTotal;
    storage << pData->m_pUnitType;
    storage << (void *)(pData->m_pScriptName);
    storage << (void*)pData->m_pCommander;
	storage << (BOOL)( pData->m_pSysCall != NULL );

	// zjistí, je-li systémové volání platné
	if ( pData->m_pSysCall != NULL )
	{	// systémové volání je platné
		// uloží systémové volání
		pData->m_pSysCall->PersistentSave ( storage );
	}

}

// nahrávání pouze uložených dat skilly
void CSMakeSkillType::PersistentLoadSkill ( CPersistentStorage &storage, CSUnit *pUnit ) 
{
    BRACE_BLOCK(storage);

    SSMakeSkillUnit *pData = SKILLDATA ( pUnit );

    storage >> pData->m_nTimeRemaining;
	storage >> pData->m_nTimeTotal;
    storage >> (void*&)pData->m_pUnitType;
    storage >> (void *&)pData->m_pScriptName;
    storage >> (void*&)pData->m_pCommander;
    BOOL bSysCall;
    storage >> bSysCall;

	// zjistí, je-li systémové volání platné
	if ( bSysCall )
	{	// systémové volání je platné
		// vytvoøí systémové volání
		CISysCallMake *pSysCall = new CISysCallMake;
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
}

// pøeklad ukazatelù dat skilly
void CSMakeSkillType::PersistentTranslatePointersSkill ( CPersistentStorage &storage, CSUnit *pUnit ) 
{
	SSMakeSkillUnit *pData = SKILLDATA ( pUnit );

    pData->m_pCommander=(CZUnit*)storage.TranslatePointer(pData->m_pCommander);

    pData->m_pUnitType=(CSUnitType*)storage.TranslatePointer(pData->m_pUnitType);

    pData->m_pScriptName = (CISOString *)storage.TranslatePointer(pData->m_pScriptName);

	// zjistí, je-li systémové volání platné
	if ( pData->m_pSysCall != NULL )
	{	// systémové volání je platné
		// pøeloží ukazatele systémového volání
		pData->m_pSysCall->PersistentTranslatePointers ( storage );
	}
}

// inicializace nahraných dat skilly
void CSMakeSkillType::PersistentInitSkill ( CSUnit *pUnit ) 
{
	SSMakeSkillUnit *pData = SKILLDATA ( pUnit );

	// zjistí, je-li systémové volání platné
	if ( pData->m_pSysCall != NULL )
	{	// systémové volání je platné
		// inicializuje systémové volání
		pData->m_pSysCall->PersistentInit ();
	}

    if(pData->m_pCommander!=NULL) {
        pData->m_pCommander->AddRef();
    }
}

/////////////////////////////////////////////////////////////////////
// Interface volání ze skriptù
/////////////////////////////////////////////////////////////////////

ESyscallResult CSMakeSkillType::SCI_Make ( CSUnit *pUnit, CIProcess *pProcess, CISyscall **ppSysCall,
                                          CIBag *pBag, CIDataStackItem *pReturn, CISystemObject *pUnitTypeName,
                                          CISystemObject *pScriptName, CZUnit *pCommander)
{
    SSMakeSkillRecord *pMakeSkillRecord;
    CISysCallMake *pMakeSysCall;

    ASSERT(pUnitTypeName->GetObjectType()==SO_STRING);
    ASSERT(pScriptName->GetObjectType()==SO_STRING);

    CISOString *pScriptStr=(CISOString*)pScriptName;
    CString strUnitType=((CISOString*)pUnitTypeName)->Get();

    pMakeSkillRecord=FindRecord(pUnit, strUnitType);

    if(pMakeSkillRecord==NULL) {
        pProcess->m_pInterpret->ReportError("Nonexisting unit type '%s'", (LPCTSTR)strUnitType);
        return SYSCALL_ERROR;
    }

    pMakeSysCall=new CISysCallMake();
	VERIFY( pMakeSysCall->Create(pProcess, pUnit, this, pBag) );

    if(!ActivateSkill(pUnit, pMakeSkillRecord, pScriptStr, pCommander, pMakeSysCall))
    {
        // Nepovedlo se zaktivovat
        // -> neni dost resourcu
        pProcess->m_pInterpret->ReportError("Not enough resources to build unit of type '%s'", (LPCTSTR)strUnitType);
        pMakeSysCall->Release();

        pReturn->Set(NULL, CCodeManager::m_pDefaultUnitType);
        return SYSCALL_SUCCESS;
    }

    *ppSysCall=pMakeSysCall;
    return SYSCALL_SUCCESS_SLEEP;
}

ESyscallResult CSMakeSkillType::SCI_GetCost (CSUnit *pUnit, CIProcess *pProcess, CISyscall **ppSysCall, CIBag *pBag, CIDataStackItem *pReturn, CISystemObject *pUnitTypeName)
{
    SSMakeSkillRecord *pMakeSkillRecord;
    ASSERT(pUnitTypeName!=NULL);
    ASSERT(pUnitTypeName->GetObjectType()==SO_STRING);
    CString strUnitType=((CISOString*)pUnitTypeName)->Get();

    pMakeSkillRecord=FindRecord(pUnit, strUnitType);

    if(pMakeSkillRecord==NULL) {
        pProcess->m_pInterpret->ReportError("Nonexisting unit type '%s'", (LPCTSTR)strUnitType);
        return SYSCALL_ERROR;
    }

    CISOResources *pResources;

    pResources = (CISOResources *)pProcess->m_pInterpret->CreateSystemObject(SO_RESOURCES);

    if (pResources == NULL) {
        return SYSCALL_ERROR;
    }

    pResources->Set(pMakeSkillRecord->m_ResourcesNeeded);
    pReturn->Set(pResources);
    pResources->Release(); 

    return SYSCALL_SUCCESS;
}

/////////////////////////////////////////////////////////////////////
// Pomocné metody skilly
/////////////////////////////////////////////////////////////////////

// Najde zaznam pro zadany unit type
SSMakeSkillRecord *CSMakeSkillType::FindRecord ( CSUnit *pUnit, CString strUnitType ) 
{
    SSMakeSkillRecord *pMakeSkillRecord;
    int i;

    // Najdeme odpovidajici unit type
    pMakeSkillRecord=m_pRecords;
    for(i=0; i<m_nRecords; i++) {
        CSUnitType *pUnitType=pMakeSkillRecord->m_pUnitType;
        // Jestli tenhle assert selze, mapeditor ulozil spatne ID unit typu
        ASSERT(pUnitType!=NULL);

        if(strUnitType.CompareNoCase(pUnitType->GetName())==0) {
            // Nasli jsme spravny zaznam
            break;
        }
        pMakeSkillRecord++;
    }

    if(i>=m_nRecords) {
        return NULL;
    }

    return pMakeSkillRecord;
}

// ukonci skilu a vrati vyrobenou jednotku
void CSMakeSkillType::Finish ( CSUnit *pUnit, CSUnit *pRet ) 
{
	SSMakeSkillUnit *pData = (SSMakeSkillUnit *)GetSkillData ( pUnit );

    pData->m_nTimeRemaining=0;
	pData->m_nTimeTotal=0;
    pData->m_pUnitType=NULL;
    if(pData->m_pScriptName != NULL){
      pData->m_pScriptName->Release();
      pData->m_pScriptName = NULL;
    }
    if(pData->m_pCommander!=NULL) {
        pData->m_pCommander->Release();
        pData->m_pCommander=NULL;
    }

    // Zmena appearance na default
    pUnit->SetDefaultAppearanceID();

	// zjistí, je-li platné systémové volání
	if ( pData->m_pSysCall != NULL )
	{	// systémové volání je platné

        if(pRet!=NULL) {
            // Povedlo se vytvorit jednotku
            TRACE_SKILL2 ("@ MakeSkill on unit %d returned newly created unit %d\n", pUnit->GetID (), pRet->GetID());

		    // nastaví návratovou hodnotu
            pData->m_pSysCall->GetBag()->ReturnUnit(pRet->GetZUnit(), pRet->GetZUnit()->m_pIUnitType);

        } else {
            // Nepovedlo se vytvorit jednotku            
            TRACE_SKILL1 ("@ MakeSkill on unit %d could not create any unit.\n", pUnit->GetID ());

		    // nastaví návratovou hodnotu
            pData->m_pSysCall->GetBag()->ReturnUnit(NULL, CCodeManager::m_pDefaultUnitType);
        }

		// ukonèí èekající systémové volání
		pData->m_pSysCall->FinishWaiting ( STATUS_SUCCESS );        
		// zruší odkaz na èekající systémové volání
		pData->m_pSysCall->Release();
		pData->m_pSysCall = NULL;
	}

	// deaktivuje skillu na jednotce
	CSSkillType::DeactivateSkill ( pUnit );
}

void CSMakeSkillType::FillFullInfo ( CSUnit *pUnit, CString &strInfo )
{
	SSMakeSkillUnit *pData = (SSMakeSkillUnit *)GetSkillData ( pUnit );
	CString tmp;

	if(pData->m_nTimeRemaining<=0) {
		strInfo="";
		return;
	}

	if(pData->m_pUnitType!=NULL) {
		strInfo=pData->m_pUnitType->GetName();
		if(pData->m_nTimeTotal<=0) return;

		tmp.Format(" %d%% (%d/%d)", 100-(100*pData->m_nTimeRemaining)/pData->m_nTimeTotal, pData->m_nTimeTotal-pData->m_nTimeRemaining, pData->m_nTimeTotal);
		strInfo+=tmp;
	} else {
		strInfo+="(unknown unit)";
	}
}

//////////////////////////////////////////////////////////////////////
// Debuggovací informace
//////////////////////////////////////////////////////////////////////

#ifdef _DEBUG

// zkontroluje neškodná data skilly
BOOL CSMakeSkillType::CheckEmptySkillData ( CSUnit *pUnit ) 
{
	SSMakeSkillUnit *pData = SKILLDATA ( pUnit );

    ASSERT(pData->m_nTimeRemaining==0);
	ASSERT(pData->m_nTimeTotal==0);
    ASSERT(pData->m_pUnitType==NULL);
    ASSERT(pData->m_pScriptName==NULL);
    ASSERT(pData->m_pCommander==NULL);
    ASSERT(pData->m_pSysCall==NULL);

    return TRUE;
}

#endif //_DEBUG
