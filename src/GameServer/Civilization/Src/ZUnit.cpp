/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Interpret
 *   Autor: Helena Kupková
 *  
 *   Popis: Implementace tridy CZUnit -> bude CZUnit!!!
 *          
 ***********************************************************/

#include "stdafx.h"
#include "..\ZCivilization.h"
#include "ZUnit.h"

#include "Triples\Interpret\Src\SysObjs\SystemObjects.h"

#include "GameServer\GameServer\SUnit.inl"
#include "GameServer\GameServer\SSkillType.h"

class CZUnitListItem;

IMPLEMENT_DYNAMIC(CZUnit, CIObjUnitStruct);

//////////////////////////////////////////////////////////////////////
// Definice poolu malych objektu, co nemaji cpp

DEFINE_MULTITHREAD_POOL( CZUnitListItem, 10);

//////////////////////////////////////////////////////////////////////
// makra pro praci se spojakovyma frontama

static inline void RemoveNotificationFromQueueBegin( CZUnitNotifyQueueItem **ppFirst, 
					 								 CZUnitNotifyQueueItem **ppLast, 
													 CZUnitNotifyQueueItem **ppEvent)
{
	ASSERT( ppFirst != NULL  &&  ppLast != NULL  && ppEvent != NULL);

	if ( (*ppFirst) != NULL)
	{
		(*ppEvent) = (*ppFirst);
		(*ppFirst) = (*ppFirst)->m_pNext; 
		if ( (*ppFirst) == NULL) 
		{
			ASSERT( (*ppEvent) == (*ppLast));
			(*ppLast) = NULL; 
		}
	}	
	else *ppEvent = NULL;
}

static inline void RemoveCommandFromQueueBegin( CZUnitCommandQueueItem **ppFirst, 
					 						    CZUnitCommandQueueItem **ppLast, 
											    CZUnitCommandQueueItem **ppEvent)
{
	RemoveNotificationFromQueueBegin(  (CZUnitNotifyQueueItem**) ppFirst,
									   (CZUnitNotifyQueueItem**) ppLast,
									   (CZUnitNotifyQueueItem**) ppEvent);
}

//////////////////////////////////////////////////////////////////////

static inline void AddNotificationToQueueEnd( CZUnitNotifyQueueItem **ppFirst, 
					 						  CZUnitNotifyQueueItem **ppLast, 
											  CZUnitNotifyQueueItem *pEvent)
{
	ASSERT( ppFirst != NULL  &&  ppLast != NULL);
	ASSERT( pEvent != NULL);  

    pEvent->m_pNext = NULL;  
    
	if ( (*ppFirst) == NULL)   
    {   
		ASSERT( (*ppLast) == NULL); 
		(*ppFirst) = (*ppLast) = pEvent;   
    }   
    else    
    {   
		ASSERT( (*ppLast) != NULL); 
        (*ppLast)->m_pNext = pEvent;  
        (*ppLast) = pEvent;   
    }
}


static inline void AddCommandToQueueEnd( CZUnitCommandQueueItem **ppFirst, 
					 		  		     CZUnitCommandQueueItem **ppLast, 
										 CZUnitCommandQueueItem *pEvent)
{
	AddNotificationToQueueEnd(  (CZUnitNotifyQueueItem**) ppFirst,
							    (CZUnitNotifyQueueItem**) ppLast,
								(CZUnitNotifyQueueItem*)  pEvent);
}

//////////////////////////////////////////////////////////////////////
// Staticke polozky

CMap<CStringTableItem*, CStringTableItem*, int, int> CZUnit::m_tpMethodIDs;
CStringTableItem* CZUnit::m_stiMemberName = NULL;
CStringTableItem* CZUnit::m_stiMemberStatus = NULL;
CMap<CStringTableItem*, CStringTableItem*, int, int> * CZUnit::m_ptpLoadMethodIDs = 0;

bool CZUnit::m_bStaticInitialized = false;
long CZUnit::m_nUnitInstanceCount = 0;

EInterpretStatus (CZUnit::* CZUnit::m_apMethods[ CZUnit::UNIT_METHOD_COUNT])( CIProcess*, CIBag*, bool) = {
    M_GetLives,
    M_GetLivesMax,
    M_GetPosition,
    M_GetScriptName,
    M_GetViewRadius,
    M_GetCommander,
    M_GetInferiorUnits,
    M_IsCommander,
    M_EmptyCommandQueue,
    M_ChangeScript,
	M_IsDead,
	M_CreateCommander,
	M_KillCommander,
    M_IsMyUnit,
	M_ChangeCommander,
	M_GetUnitTypeName,
	M_SetTimer,
	M_IsEnemyUnit,
    M_IsCommandQueueEmpty,
	/*UNITMETHOD*/ 
};

//////////////////////////////////////////////////////////////////////

CZUnit::CZUnit()
{
	 m_bPreDeleted = false;

// -> COMMON
    m_pCivilization = NULL;

    m_bIsDead = false;

    m_pCommander = NULL;
    m_pPreviousSibbling = m_pNextSibbling = NULL;                                    
    m_pFirstInferior = NULL;

// -> PRO INTERPRET
    m_pData = NULL;
    m_pIUnitType = NULL;
    m_pConstructorFinished = NULL;

//
//  -> FRONTY ZPRAV JEDNOTKY
//
    m_pMandatoryCommandQueueFirst = m_pMandatoryCommandQueueLast = NULL;

    m_pCurrentCommandQueueFirst = m_pCurrentCommandQueueLast = NULL;
    m_nCommandProcessID = 0;
    m_bProcesingMandatoryQueueCommand = false;

    m_pNotifyQueueFirst = m_pNotifyQueueLast = NULL;
    m_nNotifyProcessID = 0;

    m_nNotificationsCount = 0;
    m_nMandatoryCommandsCount = 0;
    m_nCurrentCommandsCount = 0;

    m_bSendIdle = true;
}

//////////////////////////////////////////////////////////////////////
CZUnit::~CZUnit()
{
}

//////////////////////////////////////////////////////////////////////

void CZUnit::CreateStatic()
{
// INICIALIZACE STATICKYCH POLOZEK
	m_tpMethodIDs.InitHashTable( (int)(UNIT_METHOD_COUNT * 1.5));

	/*UNITMETHOD*/ 
	m_tpMethodIDs.SetAt( g_StringTable.AddItem("GetLives"),     0 );
	m_tpMethodIDs.SetAt( g_StringTable.AddItem("GetLivesMax"),  1);
	m_tpMethodIDs.SetAt( g_StringTable.AddItem("GetPosition"),  2);
	m_tpMethodIDs.SetAt( g_StringTable.AddItem("GetScriptName"),3);
	m_tpMethodIDs.SetAt( g_StringTable.AddItem("GetViewRadius"),4);
	m_tpMethodIDs.SetAt( g_StringTable.AddItem("GetCommander"), 5);
	m_tpMethodIDs.SetAt( g_StringTable.AddItem("GetInferiorUnits"),  6);
	m_tpMethodIDs.SetAt( g_StringTable.AddItem("IsCommander"),  7);
	m_tpMethodIDs.SetAt( g_StringTable.AddItem("EmptyCommandQueue"),  8);
	m_tpMethodIDs.SetAt( g_StringTable.AddItem("ChangeScript"),  9);
	m_tpMethodIDs.SetAt( g_StringTable.AddItem("IsDead"),  10);
	m_tpMethodIDs.SetAt( g_StringTable.AddItem("CreateCommander"),  11);
	m_tpMethodIDs.SetAt( g_StringTable.AddItem("KillCommander"),  12);
	m_tpMethodIDs.SetAt( g_StringTable.AddItem("IsMyUnit"),  13);
	m_tpMethodIDs.SetAt( g_StringTable.AddItem("ChangeCommander"), 14);
	m_tpMethodIDs.SetAt( g_StringTable.AddItem("GetUnitTypeName"), 15);
    m_tpMethodIDs.SetAt( g_StringTable.AddItem("SetTimer"), 16);
	m_tpMethodIDs.SetAt( g_StringTable.AddItem("IsEnemyUnit"),  17);
    m_tpMethodIDs.SetAt( g_StringTable.AddItem("IsCommandQueueEmpty"),  18);

	/*UNITMEMBER*/
	m_stiMemberName = g_StringTable.AddItem("Name");
	m_stiMemberStatus = g_StringTable.AddItem("Status");
}

//////////////////////////////////////////////////////////////////////

void CZUnit::DeleteStatic()
{
	// UVOLNENI STATICKYCH POLOZEK
	
	ASSERT(m_nUnitInstanceCount == 0);

	POSITION pos;
	int nMethodID;
	CStringTableItem *pSTI;

	// metody
	pos = m_tpMethodIDs.GetStartPosition();
	while (pos != NULL)
	{
		m_tpMethodIDs.GetNextAssoc( pos, pSTI, nMethodID);
		pSTI->Release();
	}
	m_tpMethodIDs.RemoveAll();

	// polozky
	m_stiMemberName = m_stiMemberName->Release();
	m_stiMemberStatus = m_stiMemberStatus->Release();
	/*UNITMEMBER*/
}

//////////////////////////////////////////////////////////////////////

void CZUnit::Create( CZCivilization *pCivilization, CSUnit *pSUnit, CZUnit *pCommander, 
                     CIUnitType *pIUnitType, bool bPhysicalUnit, DWORD dwID)
{
	::InterlockedIncrement( &m_nUnitInstanceCount);
    
	ASSERT(pCivilization != NULL);
    ASSERT(pIUnitType != NULL);

#ifdef _DEBUG
    if (pCivilization->m_pGeneralCommander == NULL) ASSERT( pCommander == NULL);
    else ASSERT(pCommander != NULL);
#endif

// -> COMMON
    m_pCivilization = pCivilization;

    m_dwID = dwID;

    m_pSUnit = pSUnit;

    m_bIsDead = false;
    
	m_bPhysicalUnit = bPhysicalUnit;

// vojenska hierarchie
	if (pCommander != NULL) 
	{
		// zamknout vojenskou hierarchii
		m_pCivilization->LockHierarchyAndEvents();

		// velitel je mrtvy
		while ( pCommander != NULL && pCommander->m_bIsDead)
		{
			// predat jeho veliteli.
			pCommander = pCommander->m_pCommander;	
		}
		// kdyz to dojde az nahoru, tak hlavni velitel nesmi byt mrtvy
		ASSERT( pCommander != NULL);

		m_pCommander = pCommander;
		m_pCommander->AddInferior( this);

	    // odemknout vojenskou hierarchii
	    m_pCivilization->UnlockHierarchyAndEvents();

	}
	else m_pCommander = NULL;


// -> PRO INTERPRET
    
    // typ jednotky
    m_pIUnitType = pIUnitType;

    // datove polozky
    if (m_pIUnitType->m_nHeapSize != 0) 
    {
        m_pData = new int[m_pIUnitType->m_nHeapSize];
        ::ZeroMemory( m_pData, sizeof(int)*m_pIUnitType->m_nHeapSize);

		CIUnitType *pIUnitType;
		POSITION pos;
		CIMember *pMember;
		CStringTableItem *stiName;

		pIUnitType = m_pIUnitType;
		while (pIUnitType->m_stiName != m_pCivilization->m_pInterpret->m_stiDefaultUnit)
		{
			pos = pIUnitType->m_tpMembers.GetStartPosition();
			while (pos != NULL)
			{
				pIUnitType->m_tpMembers.GetNextAssoc( pos, stiName, pMember);
				
				if (pMember->m_DataType.IsObject())
				{
					m_pData[ pMember->m_nHeapOffset] = (int)m_pCivilization->m_pInterpret->CreateSystemObject(pMember->m_DataType);
					if (m_pData[ pMember->m_nHeapOffset] == NULL) 
					{
						m_pCivilization->ReportError("Error while creating unit '%s': cannot create member '%s', no such system object.",
							(CString)*m_pIUnitType->m_stiName, (CString)*pMember->m_stiName);
					}
				}
				else if (pMember->m_DataType.IsStruct())
				{
					ASSERT(pMember->m_DataType.m_stiName != NULL);

					CIStructureType *pStructType;
					if (( pStructType = m_pCivilization->m_pCodeManager->GetStructureType( pMember->m_DataType.m_stiName)) == NULL)
					{
						m_pCivilization->ReportError("Error while creating unit '%s': cannot create member '%s %s', no such structure.",
							(CString)*m_pIUnitType->m_stiName, (CString)*pMember->m_DataType.m_stiName, (CString)*pMember->m_stiName);
					}
					else
					{
						m_pData[ pMember->m_nHeapOffset] = (int)m_pCivilization->m_pInterpret->CreateStructure( pStructType, true);
					}
				}
			}

			pIUnitType = pIUnitType->m_pParent;
		}
    }

    // konstruktor
    m_pConstructorFinished = new CIConstructorFinishedWaiting();
    m_pConstructorFinished->Create();

    // spusteni konstruktoru
    m_pCivilization->m_pInterpret->InlayEvent( RUN_CONSTRUCTOR, (DWORD)this);
}

//////////////////////////////////////////////////////////////////////

void CZUnit::PreDelete()
{
	// zrusit se z civilizace
	 ::InterlockedDecrement( (LONG*)&m_pCivilization->m_nUnitCount);
// -> PRO INTERPRET

    // Datove polozky
    if (m_pData != NULL)
    {
		CIUnitType *pIUnitType;
		POSITION pos;
		CIMember *pMember;
		CStringTableItem *stiName;

		pIUnitType = m_pIUnitType;
		while (pIUnitType != CCodeManager::m_pDefaultUnitType)
		{
			pos = pIUnitType->m_tpMembers.GetStartPosition();
			while (pos != NULL)
			{
				pIUnitType->m_tpMembers.GetNextAssoc( pos, stiName, pMember);
				
				if ( pMember->m_DataType.IsObjectUnitStruct() && m_pData[ pMember->m_nHeapOffset] != 0) 
				{
					((CIObjUnitStruct*)m_pData[ pMember->m_nHeapOffset])->Release();
				}
			}

			pIUnitType = pIUnitType->m_pParent;
		}

        delete [] m_pData;
        m_pData = NULL;
    }

    m_pIUnitType = NULL;

    // Cekani na construktor -> zrusit
    if (m_pConstructorFinished != NULL)
    {
        m_pConstructorFinished->FinishWaiting( STATUS_UNIT_IS_DEAD);
        m_pConstructorFinished->Release();
        m_pConstructorFinished = NULL;
    }

// -> FRONTY ZPRAV - vyprazdnit
	// zde se zamyka CZCivilization::m_lockEvents
    EmptyMandatoryCommandQueue( false );
    EmptyCurrentCommandQueue( false );
    EmptyNotificationQueue();

// VOJENSKA HIERARCHIE - vypojit z hierarchie
	
	// zamknout vojenskou hierarchii
	m_pCivilization->LockHierarchyAndEvents();

    // nesmi mit nic pod sebou
    ASSERT( m_pFirstInferior == NULL);

    // vyhaknout od sousedu
    if (m_pPreviousSibbling != NULL) m_pPreviousSibbling->m_pNextSibbling = m_pNextSibbling;
    if (m_pNextSibbling != NULL) m_pNextSibbling->m_pPreviousSibbling = m_pPreviousSibbling;
    if (m_pCommander != NULL) 
    {
        if (m_pCommander->m_pFirstInferior == this) 
		  {
			  ASSERT( m_pPreviousSibbling == NULL);
			  m_pCommander->m_pFirstInferior = m_pNextSibbling;
		  }
    }

    // vynulovat ( & odpojit se od velitele)
    m_pPreviousSibbling = NULL;
    m_pNextSibbling = NULL;
    m_pCommander = NULL;

	// odemknout vojenskou hierarchii
	m_pCivilization->UnlockHierarchyAndEvents();

	m_bIsDead = true;
	m_bPreDeleted = true;
}

void CZUnit::Delete()
{
	m_pCivilization = NULL;
	::InterlockedDecrement( &m_nUnitInstanceCount);
}

//////////////////////////////////////////////////////////////////////
/// -> COMMON
//////////////////////////////////////////////////////////////////////

bool CZUnit::AddInferior( CZUnit *pInferior)
{
    ASSERT( pInferior != NULL);

    // zamknout
    m_pCivilization->LockHierarchyAndEvents();

	// fyzicke nebo  mrtve jednotky na mape nemuzou mit podrizene
	if (m_bPhysicalUnit || m_bIsDead) 
	{
		// odemknout
	    m_pCivilization->UnlockHierarchyAndEvents();
		
		return false;
	}

    // pripojeni na zacatek zpojaku podrizenych
    pInferior->m_pNextSibbling = m_pFirstInferior;
    if (m_pFirstInferior != NULL) m_pFirstInferior->m_pPreviousSibbling = pInferior;
    m_pFirstInferior = pInferior;

    pInferior->m_pCommander = this;

    // odemknout
    m_pCivilization->UnlockHierarchyAndEvents();

	return true;
}

bool CZUnit::ChangeCommander( CZUnit *pNewCommander)
{
    ASSERT(pNewCommander != NULL);

    // fyzicke jednotky nemuzou mit podrizene
    if (pNewCommander->m_bPhysicalUnit) return false;

    // zamknout
    m_pCivilization->LockHierarchyAndEvents();

	// na mrtvem veliteli taky nemuzou byt podrizeni
	if (pNewCommander->m_bIsDead) 
	{
		m_pCivilization->UnlockHierarchyAndEvents();
		return false;
	}

    // jsem nejvyssi velitel?
    if ( m_pCommander == NULL)
    {
        // odemknout
        m_pCivilization->UnlockHierarchyAndEvents();

        return false;
    }

	CZUnit *pOldCommander = m_pCommander;

    // vyhaknout ze spojaku
    if (m_pPreviousSibbling != NULL) m_pPreviousSibbling->m_pNextSibbling = m_pNextSibbling;
    if (m_pNextSibbling != NULL) m_pNextSibbling->m_pPreviousSibbling = m_pPreviousSibbling;
    if (m_pCommander->m_pFirstInferior == this) m_pCommander->m_pFirstInferior = m_pNextSibbling;
    m_pPreviousSibbling = NULL;
    m_pNextSibbling = NULL;

    // prihaknout novemu veliteli
    m_pCommander = pNewCommander;

    m_pNextSibbling = pNewCommander->m_pFirstInferior;
    if (pNewCommander->m_pFirstInferior != NULL) pNewCommander->m_pFirstInferior->m_pPreviousSibbling = this;
    pNewCommander->m_pFirstInferior = this;

    // odemknout
    m_pCivilization->UnlockHierarchyAndEvents();

	// zaslat notifikace:
	CIBag Bag;

	// puvodnimu veliteli
	Bag.Create( Bag);
	Bag.AddUnit( this, CCodeManager::m_pDefaultUnitType);
	pOldCommander->SendNotification( CZCivilization::m_stiINFERIOR_REMOVED, &Bag);
	Bag.Delete();

	// novemu veliteli
	Bag.Create( Bag);
	Bag.AddUnit( this, CCodeManager::m_pDefaultUnitType);
	pNewCommander->SendNotification( CZCivilization::m_stiINFERIOR_ADDED, &Bag);
	Bag.Delete();

	// sobe
	Bag.Create( Bag);
	this->SendNotification( CZCivilization::m_stiCOMMANDER_CHANGED, &Bag);
	Bag.Delete();

    return true;
}

//////////////////////////////////////////////////////////////////////
/// -> PRO INTERPRET
//////////////////////////////////////////////////////////////////////

CIObjUnitStruct* CZUnit::AddRef()
{
    ::InterlockedIncrement( &m_nRefCount);
    return this; 
}

CIObjUnitStruct* CZUnit::Release()
{
    // konecne zruseni mrtve jednotky
    if (::InterlockedDecrement( &m_nRefCount) == 0 && m_bIsDead)
    {
        // fyzicka jednotka
        if (m_bPhysicalUnit)
        {
            ASSERT( m_pSUnit != NULL);
            m_pSUnit->ZUnitRefCountIsZero();
        }
        // velitel
        else
        {
            if (!m_bPreDeleted) PreDelete();
            Delete();
            delete this;
            
            return NULL;
        }
    }
    
    return NULL;
}

//////////////////////////////////////////////////////////////////////
// Tohle se vola jen ze serveru, nevolat primo!!!   
bool CZUnit::TryDelete()
{
	if ( !m_bIsDead)
	{
		return false;
	}
	
	if ( m_nRefCount == 0)
	{
		if (!m_bPreDeleted) PreDelete();
		Delete();
		delete this;

		return true;
	}
	else
	{
		return false;
	}
	return false;
}
   
//////////////////////////////////////////////////////////////////////

bool CZUnit::LoadMember( CIDataStackItem &Reg, CStringTableItem *stiName, bool bCalledByEnemy)
{
    ASSERT(stiName != NULL);

    CIMember *pMember = NULL;

    switch (m_pIUnitType->GetMember( stiName, pMember))
    {
    case IMPL_NONE:
        return false;

    case IMPL_SCRIPT:
        ASSERT( m_pData != NULL);
        ASSERT( !bCalledByEnemy);
        Reg.Load(pMember->m_DataType, m_pData + pMember->m_nHeapOffset);
        return true;

    case IMPL_CPLUSPLUS:
        return LoadSystemMember( Reg, stiName, bCalledByEnemy);
    }

    return false;
}

//////////////////////////////////////////////////////////////////////

bool CZUnit::StoreMember( CIDataStackItem *pReg, CStringTableItem *stiName, bool bCalledFromEnemy)
{
    ASSERT(stiName != NULL);

    CIMember *pMember = NULL;

    switch (m_pIUnitType->GetMember( stiName, pMember))
    {
    case IMPL_NONE:
        return false;

    case IMPL_SCRIPT:
        ASSERT( !bCalledFromEnemy);
        
		if ( pMember->m_DataType.IsObjectUnitStruct() && pReg == NULL)
		{
			if (((CIObjUnitStruct*)*(m_pData + pMember->m_nHeapOffset)) != NULL)
			{
				((CIObjUnitStruct*)*(m_pData + pMember->m_nHeapOffset))->Release();
			}

			*(m_pData + pMember->m_nHeapOffset) = NULL;
			return true;
		}
		else
		{
			ASSERT( pReg != NULL);

			if (pReg->SameTypeAsAndConvertUnits(pMember->m_DataType))
			{
				pReg->Store( m_pData + pMember->m_nHeapOffset);
				return true;
			}
			else
			{
				TRACE2("%s::%s: STOREUM: type mismatch\n", (CString)*m_pIUnitType->m_stiName, (CString)*stiName);
				ASSERT(false);
				return false;
			}
		}
        break;

    case IMPL_CPLUSPLUS:
        return StoreSystemMember( pReg, stiName, bCalledFromEnemy);
    }

    return false;
}

//////////////////////////////////////////////////////////////////////

bool CZUnit::LoadSystemMember(  CIDataStackItem &Reg, CStringTableItem *stiName, bool bCalledByEnemy)
{
    if ( stiName == NULL) return false;
    
    // Name
    if (stiName == m_stiMemberName)
    {
        CISOString *pString = (CISOString*)m_pCivilization->m_pInterpret->CreateSystemObject(SO_STRING);
        if (pString == NULL) return false;

        if (bCalledByEnemy)
        {
            pString->Set("");
        }
        else
        {
            m_pCivilization->LockUnitData();
            pString->Set(GetName());
            m_pCivilization->UnlockUnitData();
        }

        Reg.Set( pString);
		pString->Release();
        return true;
    }
	// Status
	else if (stiName == m_stiMemberStatus)
	{
        CISOString *pString = (CISOString*)m_pCivilization->m_pInterpret->CreateSystemObject(SO_STRING);
        if (pString == NULL) return false;

        if (bCalledByEnemy)
        {
            pString->Set("");
        }
        else
        {
            m_pCivilization->LockUnitData();
            pString->Set( GetStatus());
            m_pCivilization->UnlockUnitData();
        }

        Reg.Set( pString);
		pString->Release();
        return true;
	}
	/*UNITMEMBER*/
	/* else if ( stiName == m_stiJmenoPolozky)
	{
	    ...
	}*/


#ifdef PROJEKTAPP
    ASSERT(false);
#else
 	 // SKILLS
	 CISkillInterface *pSkillInterface;

	 if (!m_bPhysicalUnit) 
	 {
		 pSkillInterface = NULL;
	 }
	 else
	 {
		 ASSERT( m_pSUnit != NULL);

		 // Skills -> create skill interface
		 CSSkillType* pSkillType = m_pSUnit->GetSkillTypeByNameID( stiName );
		 if (pSkillType == NULL || ( bCalledByEnemy && !pSkillType->CanBeCalledByEnemy())) 
		 {
			   pSkillInterface = NULL;
		 }
		 else
		 {
			 pSkillInterface = m_pCivilization->m_pInterpret->CreateSkillInterface( this, pSkillType);
		 }
     }

	 Reg.Set( pSkillInterface);

	 if (pSkillInterface != NULL) pSkillInterface->Release();

#endif
    return true;
}

//////////////////////////////////////////////////////////////////////

bool CZUnit::StoreSystemMember( CIDataStackItem *pReg, CStringTableItem *stiName, bool bCalledByEnemy)
{
    if ( stiName == NULL) return false;

    if (bCalledByEnemy) return false;
    
    // Name
    if (stiName == m_stiMemberName)
    {
		if (pReg == NULL)
		{
			m_pCivilization->LockUnitData();
			SetName(CString(""));
			m_pCivilization->UnlockUnitData();
        }
		else
		{
			m_pCivilization->LockUnitData();
			SetName( ((CISOString*)pReg->GetSystemObject())->m_strString);
			m_pCivilization->UnlockUnitData();
		}

        return true;
    }
    // Status
    else if (stiName == m_stiMemberStatus)
    {
		if (pReg == NULL)
		{
			m_pCivilization->LockUnitData();
			SetStatus( CString(""));
			m_pCivilization->UnlockUnitData();
        }
		else
		{
			m_pCivilization->LockUnitData();
			SetStatus( ((CISOString*)pReg->GetSystemObject())->m_strString);
			m_pCivilization->UnlockUnitData();
		}

        return true;
    }
	/*UNITMEMBER*/
	/* else if ( stiName == m_stiJmenoPolozky)
	{
	    ...
	}*/

	
    // Skills -> error
    //ASSERT(false);
    return false;
}

//////////////////////////////////////////////////////////////////////
// -> FRONTY ZPRAV
//////////////////////////////////////////////////////////////////////

CZUnit::ETargetEventQueue CZUnit::FindTargetQueue( CZUnit *pSender, EEventType eEventType)
{
    CZUnit::ETargetEventQueue eResult=UNKNOWN;
    CZUnit *pUnit;
    
    // prikaz od uzivatele
    if (pSender == NULL) 
    {
        if (eEventType == EVENT_NOTIFICATION) eResult = NOTIFICATION;
        else eResult = MANDATORY;
    }
    else if (pSender == this) eResult = CURRENT;
    else
    {
        // zkus najit odesilatele na ceste od this do korene ( velitel->vojak )

        // zamknout vojenskou hierarchii
        m_pCivilization->LockHierarchyAndEvents();

        pUnit = m_pCommander;
        while (pUnit != NULL)
        {
            if (pUnit == pSender)
            {
                // je to od velitele
                if (eEventType == EVENT_NOTIFICATION) eResult = NOTIFICATION;
                else eResult = MANDATORY;
                break;
            }
            else pUnit = pUnit->m_pCommander;
        }

        // odemknout vojenskou hierarchii
        m_pCivilization->UnlockHierarchyAndEvents();

        // neni to od velitele ani od uzivatele => je to vojak->velitel nebo vojak->vojak
        if ( eResult == UNKNOWN) eResult = NOTIFICATION;
    }

    return eResult;
}

//////////////////////////////////////////////////////////////////////

EInterpretStatus CZUnit::AppendEventArrow( CInterpret *pInterpret, CStringTableItem *stiEventName, CIBag *pBag)
{
	ASSERT( pInterpret != NULL);
	ASSERT( stiEventName != NULL);
	ASSERT( pBag != NULL);

	CIProcess *pProcess = pInterpret->m_pCurProcess;
	CZUnit *pSender = pProcess->FindSenderUnit();

	if ( pSender == this)
	{
		ETargetEventQueue eTargetQueue = FindTargetQueue( this, EVENT_UNKNOWN);
		if ( eTargetQueue == CURRENT)
		{
			if ( !CanSendCommandToMyselfFromProcess( pProcess))
			{
				return STATUS_FORBIDDEN_SEND_COMMAND;
			}
		}
	}

	AppendEvent( pSender, pInterpret, EVENT_UNKNOWN, stiEventName, pBag, NULL, true);

	return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////

EInterpretStatus CZUnit::AppendEvent( CZUnit *pSender, CInterpret *pInterpret, EEventType eEventType, 
              CStringTableItem *stiEventName, CIBag *pBag, CIWaitingInterface *pWaitingInterface, bool bFromScript)
{
    ASSERT( pInterpret != NULL);
    ASSERT( stiEventName != NULL);
    ASSERT( pBag != NULL);

    // mrtve jednotce uz nesmi nic chodit
    if (m_bIsDead && stiEventName != CZCivilization::m_stiUNIT_DIED) return STATUS_UNIT_IS_DEAD;

    // zjisteni, zda existuje implementace pro systemovou notifikaci
    if ( !bFromScript)
    {
        CIEvent *pEvent;
        m_pCivilization->LockUnitData();
        if ( m_pIUnitType->GetEvent( stiEventName, pEvent) == IMPL_NONE) 
        {
            m_pCivilization->UnlockUnitData();
            if ( pWaitingInterface != NULL)
            {
                ASSERT( eEventType == EVENT_COMMAND);
                pWaitingInterface->FinishWaiting( STATUS_SUCCESS);
            }
            return STATUS_SUCCESS;
        }
        m_pCivilization->UnlockUnitData();
    }

    // ok, jedeme dal
    CZUnitCommandQueueItem *pCommand;
    ETargetEventQueue eTargetQueue;

	if ( eEventType == EVENT_NOTIFICATION) eTargetQueue = NOTIFICATION;
    else eTargetQueue = FindTargetQueue( pSender, eEventType);

    // zamceni prace se zpravama
    m_pCivilization->LockEvents();

	switch ( eTargetQueue)
	{
    case MANDATORY:
		if ( eEventType == EVENT_UNKNOWN || eEventType == EVENT_COMMAND)
		{
	        // vytvorit novou polozku fronty
		    pCommand = new CZUnitCommandQueueItem;
            pCommand->Create( stiEventName, pBag, pWaitingInterface);

			// pridat do fronty
			AppendCommand( pSender, pInterpret, pCommand, MANDATORY, bFromScript);

			break;
		}
		else return STATUS_COMMAND_NOTIFICATION_MISMATCH;

    case CURRENT:
        if ( eEventType == EVENT_UNKNOWN || eEventType == EVENT_COMMAND)
		{
	        // vytvorit novou polozky fronty
		    pCommand = new CZUnitCommandQueueItem;
            pCommand->Create( stiEventName, pBag, pWaitingInterface);

			// pridat do fronty
			AppendCommand( pSender, pInterpret, pCommand, CURRENT, bFromScript);

			break;
		}
		else return STATUS_COMMAND_NOTIFICATION_MISMATCH;

    case NOTIFICATION:

        if( eEventType == EVENT_UNKNOWN || eEventType == EVENT_NOTIFICATION)
		{       
			// vytvorit novou polozku fronty
			CZUnitNotifyQueueItem *pNotify;
			pNotify = new CZUnitNotifyQueueItem;
            pNotify->Create( stiEventName, pBag);

			// pridat do fronty
			AppendNotification( pSender, pInterpret, pNotify, bFromScript);

			break;
		}
		else return STATUS_COMMAND_NOTIFICATION_MISMATCH;

	default:
		ASSERT(false);
    }

    // Odemceni prace se zpravama
    m_pCivilization->UnlockEvents();

    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////

EInterpretStatus CZUnit::AppendCommand( CZUnit *pSender, CInterpret *pInterpret, CZUnitCommandQueueItem *pCommand, 
                  ETargetEventQueue eQueue, bool bFromScript)
// zamek m_lockEvents je zamcen (z AppendEvent);
{
    // kontrola, jestli neni prikazu moc (->zacykleni prikazu)
    if ( m_nMandatoryCommandsCount + m_nCurrentCommandsCount > MAX_COMMANDS_COUNT && bFromScript) 
    {
        pCommand->Release();
        return STATUS_INFINITE_COMMAND_CYCLE;
    }

    if (eQueue == UNKNOWN) eQueue = FindTargetQueue( pSender, EVENT_COMMAND);

    switch (eQueue)
    {
    case CURRENT:
        // zabit co, co bezi z horni fronty
        if (m_bProcesingMandatoryQueueCommand)
        {
			// v dolni nic neni
            ASSERT( m_pCurrentCommandQueueFirst == NULL);

			if ( m_nCommandProcessID != 0)
            {
			    TRACE_INTERPRET("STATUS_COMMAND_DEFFERED: ThreadID = %d  pCommand = 0x%X  pCommand->m_stiEventName = 0x%X\n",
				    ::GetCurrentThreadId(), pCommand, pCommand->m_stiEventName);
                SIKillProcessData *pKillProcessData = new SIKillProcessData;
                pKillProcessData->nProcessID = m_nCommandProcessID;
                pKillProcessData->Status = STATUS_COMMAND_DEFFERED;
			    pKillProcessData->m_bDelete = true;

                m_pCivilization->InlayEvent( KILL_PROCESS, (DWORD)pKillProcessData);
            }
        }

        // pridat ji do fronty
        AddCommandToQueueEnd( &m_pCurrentCommandQueueFirst, 
							  &m_pCurrentCommandQueueLast,
							  pCommand);
        m_nCurrentCommandsCount++;

        // spustit ji, kdyz je tam jedina
        if (m_pCurrentCommandQueueFirst == pCommand)
        {
            m_bProcesingMandatoryQueueCommand = false;
            CIRunEventData *pRunEventData = new CIRunEventData( this, EVENT_COMMAND, pCommand);
            pInterpret->InlayEvent( RUN_EVENT, (DWORD)pRunEventData);
        }

        break;

    case MANDATORY:
        
		// pridat ji do fronty
        AddCommandToQueueEnd( &m_pMandatoryCommandQueueFirst,
							  &m_pMandatoryCommandQueueLast,
							  pCommand);
        m_nMandatoryCommandsCount++;

        // spustit ji, pokud je ve fronte sama, a pokud neni nic v current fronte
        if ( m_pMandatoryCommandQueueFirst == pCommand  && 
			 m_pCurrentCommandQueueFirst == NULL)
        {
            ASSERT( m_nCommandProcessID == 0);

            m_bProcesingMandatoryQueueCommand = true;
            CIRunEventData *pRunEventData = new CIRunEventData( this, EVENT_COMMAND, pCommand);
            pInterpret->InlayEvent( RUN_EVENT, (DWORD)pRunEventData);
        }

        break;

    default:
        ASSERT(false);
    }
    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////

EInterpretStatus CZUnit::AppendNotification( CZUnit *pSender, CInterpret *pInterpret, 
                                             CZUnitNotifyQueueItem *pNotification, bool bFromScript)
// zamek m_lockEvents je zamcen (z AppendEvent);
{
    // kontrola, jestli neni notifikaci moc (->zacykleni notifikaci)
    if ( m_nNotificationsCount > MAX_NOTIFICATIONS_COUNT && bFromScript) 
    {
        pNotification->Release();
        return STATUS_INFINITE_NOTIFICATION_CYCLE;
    }

    // pridat ji do fronty
    AddNotificationToQueueEnd( &m_pNotifyQueueFirst, 
							   &m_pNotifyQueueLast, 
							   pNotification);
    
    m_nNotificationsCount++;
    
    // kdyz je tam jedina, tak spustit
    if (m_pNotifyQueueFirst == pNotification)
    {
        ASSERT( m_nNotifyProcessID == 0);
    
        CIRunEventData *pRunEventData = new CIRunEventData( this, EVENT_NOTIFICATION, pNotification);
        pInterpret->InlayEvent( RUN_EVENT, (DWORD)pRunEventData);
    }

    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////

bool CZUnit::CanSendCommandToMyselfFromProcess( CIProcess *pProcess)
{
    // zamceni prace se zpravama
    m_pCivilization->LockEvents();

	bool bResult = !(m_bProcesingMandatoryQueueCommand && m_nCommandProcessID == pProcess->m_nProcessID);
	
    // Odemceni prace se zpravama
    m_pCivilization->UnlockEvents();

	return bResult;
}

//////////////////////////////////////////////////////////////////////
// TOTO BEZI NA THREADU INTERPRETA !!!
void CZUnit::OnEventFinished( CIProcess *pProcess, EEventType eEventType, EInterpretStatus eStatus, 
							  CInterpret *pInterpret)
{
    ASSERT( pInterpret != NULL);

    /*
	ASSERT( (pProcess != NULL && ((eEventType == EVENT_COMMAND && m_nCommandProcessID != 0) ||
								         (eEventType == EVENT_NOTIFICATION && m_nNotifyProcessID != 0))) ||
			   pProcess == NULL);
*/

	// konec hry -> uvolneni se provede v CZUnit::Delete();
	if ( g_bGameClosing) return;

    // Zamceni prace se zpravama
    m_pCivilization->LockEvents();

	// Vyjimka - zabiti mandatory procesu vyrizujici prikaz z horni fronty 
	// (STATUS_COMMAND_DEFERRED)
	if ( eEventType == EVENT_COMMAND && pProcess != NULL && eStatus == STATUS_COMMAND_DEFFERED)
	{
		// v dolni musi byt neco, co to prerusilo, pokud ne, tak to spustit znova
        if ( m_pCurrentCommandQueueFirst == NULL)
		{
			// nastane to nekdy?
			ASSERT(false);
			goto RunMandatory;
		}

		// nic se nedela - spusteni prikazu, co to zpusobil, je uz provedeno (uz se poslala
		// interpretu zprava RUN_EVENT

		// Odemceni prace se zpravama
		m_pCivilization->UnlockEvents();
		return;
	}

	// Obycejne dobehnuti - z fronty prikazu
    if ( eEventType == EVENT_COMMAND)
    {
        ASSERT( pProcess == NULL || (pProcess != NULL && (pProcess->m_nProcessID == m_nCommandProcessID ||
                                                          m_nCommandProcessID == 0)));
        CZUnitCommandQueueItem *pPom;

        m_nCommandProcessID = 0;

        // dojel prikaz z dolni fronty
        if (!m_bProcesingMandatoryQueueCommand)
        {
          // vyndat z fronty a zrusit
          RemoveCommandFromQueueBegin( &m_pCurrentCommandQueueFirst, 
            &m_pCurrentCommandQueueLast,
            &pPom);
          m_nCurrentCommandsCount--;
          
          if ( pPom != NULL) pPom->Release();
          
          // kdyz je tam dalsi, tak spustit
          if ( m_pCurrentCommandQueueFirst != NULL)
          {
RunCurrent: 
          m_bProcesingMandatoryQueueCommand = false;
          CIRunEventData *pRunEventData = new CIRunEventData( this, EVENT_COMMAND, m_pCurrentCommandQueueFirst);
          pInterpret->InlayEvent( RUN_EVENT, (DWORD)pRunEventData);
          }
          // jinak spust neco z horni
          else goto RunMandatory;
        }
        // dojel prikaz z horni fronty
        else
        {
          m_nCommandProcessID = 0;
          m_bProcesingMandatoryQueueCommand = false;
          
          // kdyz dojel normalne, nebyl zabit kvuli pridani neceho do spodni fronty
          if ( eStatus != STATUS_COMMAND_DEFFERED)
          {
            // vyndat z fronty a zrusit
            RemoveCommandFromQueueBegin( &m_pMandatoryCommandQueueFirst, 
              &m_pMandatoryCommandQueueLast, 
              &pPom);
            m_nMandatoryCommandsCount--;
            
            if ( pPom != NULL) pPom->Release();
            
            // kdyz je tam dalsi, tak spustit
RunMandatory: 
				if (m_pMandatoryCommandQueueFirst != NULL)
				{
					m_bProcesingMandatoryQueueCommand = true;
					CIRunEventData *pRunEventData = new CIRunEventData( this, EVENT_COMMAND, m_pMandatoryCommandQueueFirst);
					pInterpret->InlayEvent( RUN_EVENT, (DWORD)pRunEventData);
				}
                else
                {
                    // nic na praci -> send IDLE notification
                    if ( m_bSendIdle )
                    {
                        CIBag Bag;
                        Bag.Create();
                        SendNotification( CZCivilization::m_stiIDLE, &Bag);
                        Bag.Delete();
                    }
                    else m_bSendIdle = true;
                }
            }
            // zabity kvuli tomu, ze neco prislo do spodni fronty
            else
            {
                // v dolni musi byt neco, co to prerusilo
                ASSERT( m_pCurrentCommandQueueFirst != NULL);
                goto RunCurrent;
            }
        }
    }
    // je to z fronty notifikaci
    else if (eEventType == EVENT_NOTIFICATION)
    {
        ASSERT( pProcess == NULL || (pProcess != NULL && (pProcess->m_nProcessID == m_nNotifyProcessID || 
                                                          m_nNotifyProcessID == 0)));
        m_nNotifyProcessID = 0;

        // dojela notifikace -> odebrat ji z fronty a znicit
        CZUnitNotifyQueueItem *pPom;

        RemoveNotificationFromQueueBegin( &m_pNotifyQueueFirst, 
										  &m_pNotifyQueueLast, 
										  &pPom);
        m_nNotificationsCount--;

		ASSERT( pPom != NULL); // muze to nastat?
        if ( pPom != NULL) pPom->Release();

		// spustit dalsi notifikaci, pokud tam nejaka je
		if (m_pNotifyQueueFirst != NULL)
		{
			CIRunEventData *pRunEventData = new CIRunEventData( this, EVENT_NOTIFICATION, m_pNotifyQueueFirst);
			m_nNotifyProcessID = NULL;
			pInterpret->InlayEvent( RUN_EVENT, (DWORD)pRunEventData);
		}
    }
    else ASSERT(false);

    // Odemceni prace se zpravama
    m_pCivilization->UnlockEvents();

}

//////////////////////////////////////////////////////////////////////
//	SAVE & LOAD
//////////////////////////////////////////////////////////////////////

void CZUnit::PersistentSaveStatic( CPersistentStorage &storage)
{
    POSITION pos;
    CStringTableItem *stiName;
    int m;

    /*UNITMETHOD*/ 
    pos = m_tpMethodIDs.GetStartPosition();
    while ( pos != NULL)
    {
        m_tpMethodIDs.GetNextAssoc( pos, stiName, m);
        storage << (DWORD)stiName;
        storage << (DWORD)m;
    }
    storage << (DWORD) NULL;

	/*UNITMEMBER*/
	storage << (DWORD)m_stiMemberName;
	storage << (DWORD)m_stiMemberStatus;
}

void CZUnit::PersistentLoadStatic( CPersistentStorage &storage)
{
    CStringTableItem *stiMethodName;
    int m;

    m_ptpLoadMethodIDs = new CMap<CStringTableItem*, CStringTableItem*, int, int>;

    while (true) 
    {
        storage >> (DWORD&)stiMethodName;
        if ( stiMethodName == NULL) break;

        storage >> (DWORD&)m;
        m_ptpLoadMethodIDs->SetAt( stiMethodName, m);
    } 

    /*UNITMEMBER*/
	storage >> (DWORD&)m_stiMemberName;
	storage >> (DWORD&)m_stiMemberStatus;
}

void CZUnit::PersistentTranslatePointersStatic( CPersistentStorage &storage)
{
    POSITION pos;
    CStringTableItem *stiOldName, *stiNewName;
    int m;

    pos = m_ptpLoadMethodIDs->GetStartPosition();
    while ( pos != NULL)
    {
        m_ptpLoadMethodIDs->GetNextAssoc( pos, stiOldName, m);
        stiNewName = (CStringTableItem*) storage.TranslatePointer( stiOldName);
        m_tpMethodIDs.SetAt( stiNewName, m);
    }

    delete m_ptpLoadMethodIDs;
    m_ptpLoadMethodIDs = NULL;

    /*UNITMEMBER*/
    m_stiMemberName = (CStringTableItem*)storage.TranslatePointer( m_stiMemberName);
    m_stiMemberStatus = (CStringTableItem*)storage.TranslatePointer( m_stiMemberStatus);
}

void CZUnit::PersistentInitStatic()
{
}

void CZUnit::PersistentSave( CPersistentStorage &storage)
{
    BRACE_BLOCK(storage);
    
    // predek (v nem se uklada i this)
	CIObjUnitStruct::PersistentSave( storage);

//
// -> COMMON
//
	// Civilizace, do ktere jednotka patri.
    storage << (DWORD)m_pCivilization;

  // ID jednotky
    storage << m_dwID;

  // jmeno jednotky
    storage << (DWORD)(m_strName.GetLength() + 1);
    storage.Write((LPCSTR)m_strName, m_strName.GetLength() + 1); 

  // status
    storage << (DWORD)(m_strStatus.GetLength() + 1);
    storage.Write((LPCSTR)m_strStatus, m_strStatus.GetLength() + 1);

//
// -> PRO SERVER
//
	storage << (DWORD)m_pSUnit;

//
// VOJENSKA HIERARCHIE
//
    // Velitel jednotky
     storage << (DWORD)m_pCommander;
	 storage << (DWORD)m_pPreviousSibbling;
	 storage << (DWORD)m_pNextSibbling;
	 storage << (DWORD)m_pFirstInferior;

	// Priznak, zda se jedna o realnou (fyzickou) jednotku. 
     ASSERT( sizeof( bool) == sizeof( BYTE));
	storage << (BYTE)m_bPhysicalUnit;

    // Priznak PreDeleted
    storage << (BYTE)m_bPreDeleted;

    // send idle flag
    storage << (BYTE)m_bSendIdle;
//
// -> PRO INTERPRET
//

    // typ jednotky
    storage << (DWORD)m_pIUnitType;

    // Cekaci objekt, zda jiz bylo dokonceno volani konstruktoru.
    // Jestli ne, je nenulovy, a je treba ho ulozit.
    storage << (DWORD)m_pConstructorFinished;
    if (m_pConstructorFinished != NULL)
    {
        m_pConstructorFinished->PersistentSave(storage);
    }

    // datove polozky    
	storage << m_pIUnitType->m_nHeapSize;
    if (m_pIUnitType->m_nHeapSize != 0) storage.Write( m_pData, m_pIUnitType->m_nHeapSize * sizeof(int));

//
//  -> FRONTY ZPRAV JEDNOTKY
//
	int nCount;
	CZUnitCommandQueueItem *pCommand;

// nadrizena (horni) fronta prikazu
	// spocitat, kolik ma polozek a ulozit to
	for ( nCount = 0, pCommand = m_pMandatoryCommandQueueFirst; pCommand != NULL; nCount++, pCommand = (CZUnitCommandQueueItem*)pCommand->m_pNext);
	storage << nCount;

	// ulozit frontu
    int nTempCount=0;
	pCommand = m_pMandatoryCommandQueueFirst;
	while (pCommand != NULL)
	{
		pCommand->PersistentSave( storage);
		pCommand = (CZUnitCommandQueueItem*)pCommand->m_pNext;
        nTempCount++;
	}

    ASSERT(nTempCount==nCount);

// aktualni (dolni) fronta prikazu
	// spocitat, kolik ma polozek a ulozit to

	for ( nCount = 0, pCommand = m_pCurrentCommandQueueFirst; pCommand != NULL; nCount++, pCommand = (CZUnitCommandQueueItem*)pCommand->m_pNext);
	storage << nCount;

	// ulozit frontu
	pCommand = m_pCurrentCommandQueueFirst;
    nTempCount=0;
	while (pCommand != NULL)
	{
		pCommand->PersistentSave( storage);
		pCommand = (CZUnitCommandQueueItem*)pCommand->m_pNext;
        nTempCount++;
	}

    ASSERT(nTempCount==nCount);

    // priznak, zda se vyrizuje prikaz z mandatory queue, nebo z current queue
    storage << (BYTE)m_bProcesingMandatoryQueueCommand;

    // process vyrizujici prvni prikaz 
    storage << m_nCommandProcessID;

// fronta notifikaci
	CZUnitNotifyQueueItem *pNotify;

	// spocitat, kolik ma polozek a ulozit to
	for ( nCount = 0, pNotify = m_pNotifyQueueFirst; pNotify != NULL; nCount++, pNotify = pNotify->m_pNext);
	storage << nCount;
	
	// ulozit frontu
	pNotify = m_pNotifyQueueFirst;
	while (pNotify != NULL)
	{
		pNotify->PersistentSave( storage);
		pNotify = pNotify->m_pNext;
	}

    // process, ve kterem se vyrizuje 
    storage << m_nNotifyProcessID;

}

void CZUnit::PersistentLoad( CPersistentStorage &storage)
{
    BRACE_BLOCK(storage);

	// predek (v nem se i registruje this pointer)
	CIObjUnitStruct::PersistentLoad( storage);

//
// -> COMMON
//
	// Civilizace, do ktere jednotka patri.
    storage >> (DWORD&)m_pCivilization;

  // ID jednotky
    storage >> m_dwID;

  // jmeno jednotky
    DWORD dwLength;
    storage >> dwLength;
    storage.Read(m_strName.GetBuffer(dwLength), dwLength);

  // status
    storage >> dwLength;
    storage.Read(m_strStatus.GetBuffer(dwLength), dwLength);

//
// -> PRO SERVER
//
	storage >> (DWORD&)m_pSUnit;

//
// VOJENSKA HIERARCHIE
//
    // Velitel jednotky
    storage >> (DWORD&)m_pCommander;
	 storage >> (DWORD&)m_pPreviousSibbling;
	 storage >> (DWORD&)m_pNextSibbling;
	 storage >> (DWORD&)m_pFirstInferior;

	// Priznak, zda se jedna o realnou (fyzickou) jednotku. 
	storage >> (BYTE&)m_bPhysicalUnit;

    storage >> (BYTE&)m_bPreDeleted;

    // send idle flag
    storage >> (BYTE&)m_bSendIdle;

//
// -> PRO INTERPRET
//

    // typ jednotky
    storage >> (DWORD&)m_pIUnitType;

    // Cekaci objekt, zda jiz bylo dokonceno volani konstruktoru.
    // Jestli ne, je nenulovy, a je treba ho ulozit.
    storage >> (DWORD&)m_pConstructorFinished;
    if (m_pConstructorFinished != NULL)
    {
        m_pConstructorFinished = new CIConstructorFinishedWaiting;
        m_pConstructorFinished->PersistentLoad(storage);
    }

    // datove polozky    
	long nHeapSize;
	storage >> nHeapSize;
	if (nHeapSize != 0)
	{
		m_pData = new int[nHeapSize];
		storage.Read( m_pData, nHeapSize * sizeof(int));
	}
	else m_pData = NULL;

//
//  -> FRONTY ZPRAV JEDNOTKY
//
	int nCount, i;
	CZUnitCommandQueueItem *pPom;

// nadrizena (horni) fronta prikazu
	storage >> nCount;

	m_pMandatoryCommandQueueFirst = NULL;

	for (i=0; i<nCount; i++)
	{
		pPom = new CZUnitCommandQueueItem;
		pPom->PersistentLoad( storage);

		if (m_pMandatoryCommandQueueFirst == NULL) m_pMandatoryCommandQueueFirst = m_pMandatoryCommandQueueLast = pPom;
		else
		{
			m_pMandatoryCommandQueueLast->m_pNext = pPom;
			m_pMandatoryCommandQueueLast = pPom;
		}
	}

// aktualni (dolni) fronta prikazu

	storage >> nCount;

	m_pCurrentCommandQueueFirst = NULL;

	for (i=0; i<nCount; i++)
	{
		pPom = new CZUnitCommandQueueItem;
		pPom->PersistentLoad( storage);

		if (m_pCurrentCommandQueueFirst == NULL) m_pCurrentCommandQueueFirst = m_pCurrentCommandQueueLast = pPom;
		else
		{
			m_pCurrentCommandQueueLast->m_pNext = pPom;
			m_pCurrentCommandQueueLast = pPom;
		}
	}

    // priznak, zda se vyrizuje prikaz z mandatory queue, nebo z current queue
    storage >> (BYTE&)m_bProcesingMandatoryQueueCommand;

    // process vyrizujici prvni prikaz 
    storage >> (DWORD&)m_nCommandProcessID;

// fronta notifikaci
	CZUnitNotifyQueueItem *pPom2;
	storage >> nCount;

	m_pNotifyQueueFirst = NULL;

	for (i=0; i<nCount; i++)
	{
		pPom2 = new CZUnitNotifyQueueItem;
		pPom2->PersistentLoad( storage);

		if (m_pNotifyQueueFirst == NULL) m_pNotifyQueueFirst = m_pNotifyQueueLast = pPom2;
		else
		{
			m_pNotifyQueueLast->m_pNext = pPom2;
			m_pNotifyQueueLast = pPom2;
		}
	}

    // process, ve kterem se vyrizuje 
    storage >> (DWORD&)m_nNotifyProcessID;

//
// -> PRO SERVER
//

	//storage >> m_pSUnit;
}

void CZUnit::PersistentTranslatePointers( CPersistentStorage &storage)
{
	// predek
	CIObjUnitStruct::PersistentTranslatePointers( storage);

//
// -> COMMON
//
	// Civilizace, do ktere jednotka patri.
    m_pCivilization = (CZCivilization*)storage.TranslatePointer( m_pCivilization);

//
// -> PRO SERVER
//
	m_pSUnit = (CSUnit*)storage.TranslatePointer( m_pSUnit);


// VOJENSKA HIERARCHIE
    m_pCommander  = (CZUnit*) storage.TranslatePointer( m_pCommander);
	m_pPreviousSibbling = (CZUnit*) storage.TranslatePointer( m_pPreviousSibbling);
	m_pNextSibbling = (CZUnit*) storage.TranslatePointer( m_pNextSibbling);
	m_pFirstInferior = (CZUnit*) storage.TranslatePointer( m_pFirstInferior);

//
// -> PRO INTERPRET
//
    // typ jednotky
    m_pIUnitType = (CIUnitType*) storage.TranslatePointer( m_pIUnitType);

    // cekani na konstruktor
    if (m_pConstructorFinished != NULL)
    {
        m_pConstructorFinished->PersistentTranslatePointers(storage);
    }

    // datove polozky 
	CIUnitType *pIUnitType;
	POSITION pos;
	CIMember *pMember;
	CStringTableItem *stiName;

	pIUnitType = m_pIUnitType;
	while (pIUnitType->m_stiName != m_pCivilization->m_pInterpret->m_stiDefaultUnit)
	{
		pos = pIUnitType->m_tpMembers.GetStartPosition();
		while (pos != NULL)
		{
			pIUnitType->m_tpMembers.GetNextAssoc( pos, stiName, pMember);
			
			if (pMember->m_DataType.IsObjectUnitStruct() && m_pData[ pMember->m_nHeapOffset] != 0) 
			{
			    *(m_pData + pMember->m_nHeapOffset) = (int) storage.TranslatePointer((void*)*(m_pData + pMember->m_nHeapOffset));
			}
		}

		pIUnitType = pIUnitType->m_pParent;
	}

//
//  -> FRONTY ZPRAV JEDNOTKY
//
	CZUnitCommandQueueItem *pPom;
    CZUnitNotifyQueueItem *pPom2;

// nadrizena (horni) fronta prikazu
    for ( pPom = m_pMandatoryCommandQueueFirst;  pPom != NULL; pPom = (CZUnitCommandQueueItem*)pPom->m_pNext)
    {
		pPom->PersistentTranslatePointers( storage);
	}

// aktualni (dolni) fronta prikazu
    for ( pPom = m_pCurrentCommandQueueFirst;  pPom != NULL; pPom = (CZUnitCommandQueueItem*)pPom->m_pNext)
    {
		pPom->PersistentTranslatePointers( storage);
	}


// fronta notifikaci
    for ( pPom2 = m_pNotifyQueueFirst;  pPom2 != NULL; pPom2 = pPom2->m_pNext)
    {
		pPom2->PersistentTranslatePointers( storage);
	}

//
// -> PRO SERVER
//

	//m_pSUnit = storage.TranslatePointer( m_pSUnit);
}

void CZUnit::PersistentInit()
{
	// predek
	CIObjUnitStruct::PersistentInit();

//
//  -> FRONTY ZPRAV JEDNOTKY
//
	CZUnitCommandQueueItem *pPom;
    CZUnitNotifyQueueItem *pPom2;

// nadrizena (horni) fronta prikazu
    for ( pPom = m_pMandatoryCommandQueueFirst;  pPom != NULL; pPom = (CZUnitCommandQueueItem*)pPom->m_pNext)
    {
		pPom->PersistentInit();
	}

// aktualni (dolni) fronta prikazu
    for ( pPom = m_pCurrentCommandQueueFirst;  pPom != NULL; pPom = (CZUnitCommandQueueItem*)pPom->m_pNext)
    {
		pPom->PersistentInit();
	}


// fronta notifikaci
    for ( pPom2 = m_pNotifyQueueFirst;  pPom2 != NULL; pPom2 = pPom2->m_pNext)
    {
		pPom2->PersistentInit();
	}

    // cekani na konstruktor
    if (m_pConstructorFinished != NULL)
    {
        m_pConstructorFinished->PersistentInit();
    }
	
	// increments unit instances count
	::InterlockedIncrement( &m_nUnitInstanceCount);
}

//////////////////////////////////////////////////////////////////////
//	PRO INTERPRET - VOLANI METOD
//////////////////////////////////////////////////////////////////////

EInterpretStatus CZUnit::InvokeMethod(CIProcess *pProcess, CStringTableItem *stiName, CIBag *pBag)
{
    ASSERT( pProcess != NULL);
    ASSERT( stiName != NULL);
    ASSERT( pBag != NULL);

    int nMethodID;

    if (!m_tpMethodIDs.Lookup( stiName, nMethodID)) return STATUS_METHOD_NOT_FOUND;
    
    if (nMethodID >= UNIT_METHOD_COUNT) 
    {
        // nekonzistence mapovani m_tpMethodIDs a m_apMethods
        ASSERT(false);
    }

    return (this->*m_apMethods[nMethodID])( pProcess, pBag, 
        pProcess->m_pInterpret->m_pCivilization != m_pCivilization);
}

EInterpretStatus CZUnit::M_GetLives( CIProcess*, CIBag *pBag, bool)
{
    ASSERT( pBag->GetArgCount() == 0);
    ASSERT( pBag->GetReturnRegister() != NULL);

    if (m_pSUnit != NULL) pBag->ReturnInt( m_pSUnit->GetLives());
    else pBag->ReturnInt( 1);  // velitel

    return STATUS_SUCCESS;
}

EInterpretStatus CZUnit::M_GetLivesMax( CIProcess*, CIBag *pBag, bool)
{
    ASSERT( pBag->GetArgCount() == 0);
    ASSERT( pBag->GetReturnRegister() != NULL);

    ASSERT( m_pSUnit != NULL);

    if (m_pSUnit != NULL) pBag->ReturnInt( m_pSUnit->GetLivesMax());
    else pBag->ReturnInt( 1); // velitel

    return STATUS_SUCCESS;
}

EInterpretStatus CZUnit::M_GetPosition( CIProcess *pProcess, CIBag *pBag, bool bCalledFromEnemy)
{
    ASSERT( pBag->GetArgCount() == 0);
    ASSERT( pBag->GetReturnRegister() != NULL);

    CISOPosition *pPosition = (CISOPosition*)m_pCivilization->m_pInterpret->CreateSystemObject(SO_POSITION);
    if (pPosition == NULL) return STATUS_CANNOT_CREATE_SYSTEM_OBJECT;

    ASSERT( m_pSUnit != NULL);

    CSPosition cSPos;

    m_pSUnit->ReaderLock();
    m_pSUnit->GetLastSeenPosition( pProcess->m_pInterpret->m_pCivilization->GetSCivilization()->GetCivilizationIndex(), cSPos );
    m_pSUnit->ReaderUnlock();

    pPosition->Set( &cSPos );

    pBag->ReturnSystemObject( pPosition);

    pPosition->Release();
    
    return STATUS_SUCCESS;
}

EInterpretStatus CZUnit::M_GetScriptName( CIProcess*, CIBag *pBag, bool bCalledFromEnemy)
{
    ASSERT( pBag->GetArgCount() == 0);
    ASSERT( pBag->GetReturnRegister() != NULL);
    
    CISOString *pString = (CISOString*)m_pCivilization->m_pInterpret->CreateSystemObject(SO_STRING);
    if (pString == NULL) return STATUS_CANNOT_CREATE_SYSTEM_OBJECT;

    if (bCalledFromEnemy)
    {
        // TODO: domyslet (pProcess->m_pInterpret->m_pCivilization je nepratelska civilizace)
        pString->Set("");
    }
    else
    {
        pString->Set( (CString)*m_pIUnitType->m_stiName);
    }

    pBag->ReturnSystemObject( pString);

	pString->Release();
        
    return STATUS_SUCCESS;
}

EInterpretStatus CZUnit::M_GetViewRadius( CIProcess*, CIBag *pBag, bool bCalledFromEnemy)
{
    ASSERT( pBag->GetArgCount() == 0);
    ASSERT( pBag->GetReturnRegister() != NULL);

    ASSERT( m_pSUnit != NULL);

    if (bCalledFromEnemy)
    {
        // TODO: domyslet (pProcess->m_pInterpret->m_pCivilization je nepratelska civilizace)
        return STATUS_PERMISSION_DENIED;
    }
    else
    {
        pBag->ReturnInt( m_pSUnit->GetViewRadius());
    }

    return STATUS_SUCCESS;
}

EInterpretStatus CZUnit::M_GetCommander( CIProcess*, CIBag *pBag, bool bCalledFromEnemy)
{
    ASSERT( pBag->GetArgCount() == 0);
    ASSERT( pBag->GetReturnRegister() != NULL);

    if (bCalledFromEnemy)
    {
        // TODO: domyslet (pProcess->m_pInterpret->m_pCivilization je nepratelska civilizace)
        return STATUS_PERMISSION_DENIED;
    }
    else
    {
        // zamknout vojenskou hierarchii
        m_pCivilization->LockHierarchyAndEvents();

        pBag->ReturnUnit( m_pCommander, CCodeManager::m_pDefaultUnitType);

        // odemnknout vojenskou hierarchii
        m_pCivilization->UnlockHierarchyAndEvents();
    }

    return STATUS_SUCCESS;
}

EInterpretStatus CZUnit::M_GetInferiorUnits(CIProcess*, CIBag *pBag, bool bCalledFromEnemy)
{
    ASSERT( pBag->GetArgCount() == 0);
    ASSERT( pBag->GetReturnRegister() != NULL);
//    ASSERT( pBag->GetReturnRegister != NULL);

    if (bCalledFromEnemy)
    {
        // TODO: domyslet (pProcess->m_pInterpret->m_pCivilization je nepratelska civilizace)
        return STATUS_PERMISSION_DENIED;
    }
    else
    {
        CType Type;
        Type.SetUnit( g_StringTable.AddItem(DEFAULT_UNIT_PARENT, false));

        CISOSet<CZUnit*> *pSet = (CISOSet<CZUnit*>*)m_pCivilization->m_pInterpret->CreateSet( &Type, true);
        if (pSet == NULL) return STATUS_CANNOT_CREATE_SYSTEM_OBJECT;

        // zamknout vojenskou hierarchii
        m_pCivilization->LockHierarchyAndEvents();

        CZUnit *pPom = m_pFirstInferior;
        while (pPom != NULL)
        {
            if (!pPom->m_bIsDead) pSet->AddItemToSet( pPom);
            pPom = pPom->m_pNextSibbling;
        }

        // odemnknout vojenskou hierarchii
        m_pCivilization->UnlockHierarchyAndEvents();

        pBag->ReturnSystemObject( pSet);

        pSet->Release();
    }

    return STATUS_SUCCESS;
}

EInterpretStatus CZUnit::M_IsCommander( CIProcess*, CIBag *pBag, bool bCalledFromEnemy)
{
    ASSERT( pBag->GetArgCount() == 0);
    ASSERT( pBag->GetReturnRegister() != NULL);

    if (bCalledFromEnemy)
    {
        // TODO: domyslet (pProcess->m_pInterpret->m_pCivilization je nepratelska civilizace)
        return STATUS_PERMISSION_DENIED;
    }
    else
    {
        pBag->ReturnBool( !m_bPhysicalUnit);
    }

    return STATUS_SUCCESS;
}

EInterpretStatus CZUnit::M_EmptyCommandQueue( CIProcess *pProcess, CIBag *pBag, bool bCalledFromEnemy)
{
    ASSERT( pBag->GetArgCount() == 2);
    ASSERT( pBag->GetReturnRegister() != NULL);

    if (bCalledFromEnemy)
    {
        // TODO: domyslet (pProcess->m_pInterpret->m_pCivilization je nepratelska civilizace)
        return STATUS_PERMISSION_DENIED;
    }
    else
    {
		// kontrola prav - zda je to volano velitelem, userem nebo jednotkou samotnou
		CZUnit *pInvocUnit = pProcess->FindSenderUnit();
		if (pInvocUnit != NULL && pInvocUnit != this)
		{
			if (!pInvocUnit->IsCommanderOfUnit( this)) return STATUS_PERMISSION_DENIED;
		}

        ETargetEventQueue eTargetQueue;

        eTargetQueue = FindTargetQueue( pInvocUnit, EVENT_UNKNOWN);

        switch (eTargetQueue)
        {
        // velitel -> vojak nebo user->vojak
        case MANDATORY:
          {
              bool bSendIdle = true;
              // patch nez se prekompiluji vsechny skriptsety

              int *pBoth = pBag->GetBool( 0);
              int *pSendIdle = pBag->GetBool ( 1 );
              ASSERT( pBoth != NULL);
              ASSERT ( pSendIdle != NULL );
              bSendIdle = *pSendIdle != 0;
                  
              if ( *pBoth != 0) EmptyCurrentCommandQueue ( bSendIdle );
              
              //  horni fronta
              EmptyMandatoryCommandQueue ( bSendIdle );
              pBag->ReturnBool( true);
          }
          break;

        // vojak sam sobe
        case CURRENT:
          {
            bool bSendIdle = true;
			      // patch nez se prekompiluji vsechny skriptsety
			      if ( pBag->GetArgCount() != 0) // novy skriptset
			      {
				      ASSERT( pBag->GetArgCount() == 2);
				      int *pBoth = pBag->GetBool( 0);
              int *pSendIdle = pBag->GetBool ( 1 );
				      ASSERT( pBoth != NULL);
              ASSERT ( pSendIdle != NULL );
              bSendIdle = *pSendIdle ? true : false;
			      }

            EmptyCurrentCommandQueue ( bSendIdle );
            pBag->ReturnBool( true);

          }
          break;

        // nekdo jiny -> vojak => chyba
        case NOTIFICATION:
            pBag->ReturnBool( false);
            break;
        }
    }

    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////

EInterpretStatus CZUnit::M_ChangeScript( CIProcess *pProcess, CIBag *pBag, bool bCalledByEnemy)
{
    ASSERT( pBag->GetArgCount() == 1);

    CISOString *pString = pBag->GetString(0);
//    CIUnitType *pNewUnitType;
//    CStringTableItem *stiUnitName;

    if (bCalledByEnemy)
    {
        return STATUS_PERMISSION_DENIED;
    }

    if ( m_bIsDead) 
    {
        return STATUS_UNIT_IS_DEAD;
    }

	// kontrola prav - zda je to volano velitelem, userem nebo jednotkou samotnou
	CZUnit *pInvocUnit = pProcess->FindSenderUnit();
	if (pInvocUnit != NULL && pInvocUnit != this)
	{
		if (!pInvocUnit->IsCommanderOfUnit( this)) return STATUS_PERMISSION_DENIED;
	}

	ASSERT( pString != NULL);

	// zmena skriptu
    SIChangeUnitScript Data;
    
    Data.m_pUnit = this;
    Data.m_strScriptName = pString->m_strString;
    
    CInterpret *pInterpret = pProcess->m_pInterpret;
    
    pInterpret->OnChangeUnitScript( &Data);
    
    if (!Data.m_bSuccess) pBag->ReturnBool( false);
    // jinak je process uz zabitej, takze bag je neplatnej

    return STATUS_SUCCESS;
}

EInterpretStatus CZUnit::M_IsDead           (CIProcess*, CIBag *pBag, bool bCalledByEnemy)
{
    ASSERT( pBag->GetArgCount() == 0);
    ASSERT( pBag->GetReturnRegister() != NULL);

    pBag->ReturnBool( m_bIsDead);

    return STATUS_SUCCESS;
}

EInterpretStatus CZUnit::M_CreateCommander  (CIProcess*, CIBag *pBag, bool bCalledByEnemy)
{
    ASSERT( pBag->GetArgCount() == 1);
    ASSERT( pBag->GetReturnRegister() != NULL);

    CISOString *pScriptName = pBag->GetString(0);

    ASSERT( pScriptName != NULL);

    CZUnit *pUnit;

	if (m_bPhysicalUnit) pBag->ReturnUnit( NULL, CCodeManager::m_pDefaultUnitType);
	else
	{
	    pUnit = m_pCivilization->CreateUnit( NULL, this, pScriptName->m_strString, false);
	    pBag->ReturnUnit( pUnit, CCodeManager::m_pDefaultUnitType);
	}
    
    return STATUS_SUCCESS;
}

EInterpretStatus CZUnit::M_KillCommander    (CIProcess* pProcess, CIBag *pBag, bool bCalledByEnemy)
{
    ASSERT( pBag->GetArgCount() == 1);
    ASSERT( pBag->GetReturnRegister() != NULL);

    CZUnit *pUnit = pBag->GetUnit(0);

    ASSERT( pUnit != NULL);

	if (bCalledByEnemy) return STATUS_PERMISSION_DENIED;

    if (m_bPhysicalUnit) 
	{
		pBag->ReturnInt(1);
		return STATUS_SUCCESS;
	}

	CZCivilization *pCiv = m_pCivilization;
	CZUnit *pKiller = pProcess->FindSenderUnit();
	CZUnit *pPom;

	// velitel se nemuze zasebevrazdit
	if (pKiller == this) 
	{
		pBag->ReturnInt( 2);
		return STATUS_SUCCESS;
	}

	// kontrola, zda ho zabiji nadrizeny nebo user
	if (pKiller != NULL)
	{
		if (!pKiller->IsCommanderOfUnit( this))
		{
			return STATUS_PERMISSION_DENIED;
		}
	}

	// kontrola, zda nema zive podrizene
	CZUnit *pInfer = m_pFirstInferior;
	while (pInfer != NULL)
	{
		if (!pInfer->IsDead()) 
		{
			// odemceni vojenske hierarchie
			pCiv->UnlockHierarchyAndEvents();

			pBag->ReturnInt( 4);
			return STATUS_SUCCESS;
		}
		
		pInfer = pInfer->m_pNextSibbling;
	}

	// OK, muzeme ho zabit.
	// Musime predat mrtve jednotky tomu, kdo tuto metodu zavolal. 
	// Pokud to je fyzicka jednotka, tak jejimu veliteli.
	// Pokud to neni volani z zadne jednotky (je to napr. z globalni funkce (?)), tak predat nejvyssimu veliteli.

	if (pKiller == NULL) pKiller = m_pCivilization->m_pGeneralCommander;
	else ASSERT( !pKiller->m_bPhysicalUnit );
	
	ASSERT( pKiller != NULL);

	pInfer = m_pFirstInferior;
	while (pInfer != NULL)
	{
		// nejdrive se posunout ve spojaku, protoze ChangeCommander tu jednotku vyhakne
		pPom = pInfer;
		pInfer = pInfer->m_pNextSibbling;
		// zmenit velitele
		pInfer->ChangeCommander( pKiller);
	}

	// uz nic nema -> muzu ji zabit
	ASSERT( m_pFirstInferior == NULL);

	// ZABITI VELITELE
	// vyhaknuti od nadrizeneho
	ASSERT( m_pCommander != NULL);

	pBag->ReturnInt( 0);

	// nastavit flag, ze se ma znicit, kdyz refcount klesne na 0
    m_bIsDead = true;
    m_pCivilization->m_mapUnitsByIDs.RemoveKey(pUnit->GetID());

	// zajistit uvolneni pro pripad kdy uz je refcount==0
	pUnit->AddRef();
	pUnit->Release();

	// odemceni vojenske hierarchie
	pCiv->UnlockHierarchyAndEvents();

    return STATUS_SUCCESS;
}

EInterpretStatus CZUnit::M_IsMyUnit (CIProcess* pProcess, CIBag *pBag, bool bCalledByEnemy)
{
    ASSERT( pBag->GetArgCount() == 0);
    ASSERT( pBag->GetReturnRegister() != NULL);

    pBag->ReturnBool(!bCalledByEnemy);

    return STATUS_SUCCESS;
}

EInterpretStatus CZUnit::M_IsEnemyUnit (CIProcess* pProcess, CIBag *pBag, bool bCalledByEnemy)
{
    ASSERT( pBag->GetArgCount() == 0);
    ASSERT( pBag->GetReturnRegister() != NULL);

    // Jestlize se mne pta nepritel, jsem mu nepritelem, jenom pokud nepatrim
    // systemove civilizaci, pak nejsem nepritel ani pritel.
    pBag->ReturnBool(bCalledByEnemy && GetCivilization()->GetCivilizationIndex()!=0);

    return STATUS_SUCCESS;
}

EInterpretStatus CZUnit::M_ChangeCommander (CIProcess* pProcess, CIBag *pBag, bool bCalledByEnemy)
{
    ASSERT( pBag->GetArgCount() == 1);
    ASSERT( pBag->GetReturnRegister() != NULL);

    if(bCalledByEnemy) return STATUS_PERMISSION_DENIED;

	// kontrola prav - zda je to volano velitelem nebo userem
	CZUnit *pInvocUnit = pProcess->FindSenderUnit();
	if (pInvocUnit != NULL)
	{
		if (!pInvocUnit->IsCommanderOfUnit( this)) return STATUS_PERMISSION_DENIED;
	}

	CZUnit *pNewCommander = pBag->GetUnit(0);

	if ( pNewCommander == NULL)
	{
		return STATUS_INVALID_REFERENCE;
	}
	
	pBag->ReturnBool( ChangeCommander( pNewCommander));

    return STATUS_SUCCESS;
}

EInterpretStatus CZUnit::M_GetUnitTypeName (CIProcess *pProcess, CIBag *pBag, bool bCalledByEnemy)
{
	ASSERT( pBag->GetArgCount() == 0 );
	ASSERT( pBag->GetReturnRegister() != NULL );

	if(m_pSUnit == NULL)
	{
		pBag->ReturnSystemObject((CISystemObject *)NULL);
	}
	else
	{
		CISOString *pString = (CISOString *)pProcess->m_pInterpret->CreateSystemObject(SO_STRING);
		if(pString == NULL) return STATUS_CANNOT_CREATE_SYSTEM_OBJECT;

		pString->Set(m_pSUnit->GetUnitType()->GetName());
		
		pBag->ReturnSystemObject(pString);

		pString->Release();
	}
	return STATUS_SUCCESS;
}

EInterpretStatus CZUnit::M_SetTimer (CIProcess *pProcess, CIBag *pBag, bool bCalledByEnemy)
{
  ASSERT( pBag->GetArgCount() == 2 );
  if ( bCalledByEnemy ) return STATUS_PERMISSION_DENIED;

  // Get parameters
  int * pUserID, * pDelay;

  // First is the User ID for this timer
  pUserID = pBag->GetInt ( 0 );
  if ( pUserID == NULL ) return STATUS_INVALID_REFERENCE;

  // Second is delay to which to set the timer in timeslices
  pDelay = pBag->GetInt ( 1 );
  if ( pDelay == NULL ) return STATUS_INVALID_REFERENCE;

  // Now set the timer
  GetCivilization ()->SetTimer ( this, *pUserID, g_cMap.GetTimeSlice () + *pDelay );

  // And that's all
  return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////

EInterpretStatus CZUnit::M_IsCommandQueueEmpty(CIProcess*, CIBag *pBag, bool bCalledByEnemy)
{
    ASSERT( pBag->GetArgCount() == 0 );
    if ( bCalledByEnemy ) return STATUS_PERMISSION_DENIED;

    pBag->ReturnBool( m_pMandatoryCommandQueueFirst == NULL &&
                      m_pCurrentCommandQueueFirst == NULL);

    return STATUS_SUCCESS;
}

// Pridat implementaci metody
/*UNITMETHOD*/

//////////////////////////////////////////////////////////////////////

EInterpretStatus CZUnit::SendNotification( CStringTableItem *stiNotificationName, CIBag *pBag)
{
    ASSERT( m_pCivilization != NULL);
	return AppendEvent( NULL, m_pCivilization->m_pInterpret, EVENT_NOTIFICATION, 
                        stiNotificationName, pBag, NULL, false);
}

//////////////////////////////////////////////////////////////////////

void CZUnit::EmptyMandatoryCommandQueue ( bool bSendIdle )
{
    CZUnitNotifyQueueItem *pPom;

    // zamceni prace se zpravama
    m_pCivilization->LockEvents();

    if (m_pMandatoryCommandQueueFirst != NULL)
    {
        // vyprazdnit frontu az na prvni prvek, ktery muze byt executed
        while (m_pMandatoryCommandQueueFirst->m_pNext != NULL)
        {
            pPom = m_pMandatoryCommandQueueFirst->m_pNext;
            m_pMandatoryCommandQueueFirst->m_pNext = m_pMandatoryCommandQueueFirst->m_pNext->m_pNext;
            pPom->Release();
        }
		m_pMandatoryCommandQueueLast = m_pMandatoryCommandQueueFirst;

		// kdyz se prave neco provadi, zastrelit
		if (m_bProcesingMandatoryQueueCommand)
		{
			ASSERT( m_pCurrentCommandQueueFirst == NULL);
        
			// ukoncit process
			if ( !g_bGameClosing && m_nCommandProcessID != 0)
			{
				SIKillProcessData *pKillProcessData = new SIKillProcessData;
				pKillProcessData->nProcessID = m_nCommandProcessID;
				pKillProcessData->Status = STATUS_EMPTYING_QUEUE;
				pKillProcessData->m_bDelete = true;

                m_bSendIdle = bSendIdle;

				m_pCivilization->InlayEvent( KILL_PROCESS, (DWORD)pKillProcessData);
			}
			else
			{
				m_pMandatoryCommandQueueFirst->Release();
				m_pMandatoryCommandQueueFirst = NULL;
				m_pMandatoryCommandQueueLast = NULL;
				m_nCommandProcessID = 0;
			}
		}
		else
		{
			// smazat prvni polozku
			m_pMandatoryCommandQueueFirst->Release();
			m_pMandatoryCommandQueueFirst = m_pMandatoryCommandQueueLast = NULL;
			m_nCommandProcessID = 0;
		}
    }

	ASSERT( m_pMandatoryCommandQueueFirst == m_pMandatoryCommandQueueLast);

    if ( m_pMandatoryCommandQueueFirst != NULL) m_nMandatoryCommandsCount = 1;
    else m_nMandatoryCommandsCount = 0;

    // odemceni prace se zpravama
    m_pCivilization->UnlockEvents();
}

//////////////////////////////////////////////////////////////////////

void CZUnit::EmptyCurrentCommandQueue ( bool bSendIdle )
{
    CZUnitNotifyQueueItem *pPom;

    // zamceni prace se zpravama
    m_pCivilization->LockEvents();

    if ( m_pCurrentCommandQueueFirst != NULL)
    {
        // vyprazdnit frontu az na prvni prvek, ktery muze byt executed
        while (m_pCurrentCommandQueueFirst->m_pNext != NULL)
        {
            pPom = m_pCurrentCommandQueueFirst->m_pNext;
            m_pCurrentCommandQueueFirst->m_pNext = m_pCurrentCommandQueueFirst->m_pNext->m_pNext;
            pPom->Release();
        }
		m_pCurrentCommandQueueLast = m_pCurrentCommandQueueFirst;

		// prvni polozka se musi provadet
		ASSERT( !m_bProcesingMandatoryQueueCommand);

		// process bezi
		if ( !g_bGameClosing && m_nCommandProcessID != 0)
		{
			// -> zabit proces
			SIKillProcessData *pKillProcessData = new SIKillProcessData;
			pKillProcessData->nProcessID = m_nCommandProcessID;
			pKillProcessData->Status = STATUS_EMPTYING_QUEUE;
			pKillProcessData->m_bDelete = true;

            m_bSendIdle = bSendIdle;
			m_pCivilization->InlayEvent( KILL_PROCESS, (DWORD)pKillProcessData);
		}
		else
		{
			m_pCurrentCommandQueueFirst->Release();
			m_pCurrentCommandQueueFirst = m_pCurrentCommandQueueLast = NULL;
			m_nCommandProcessID = 0;
		}
    }

	ASSERT( m_pCurrentCommandQueueFirst == m_pCurrentCommandQueueLast);

    if ( m_pCurrentCommandQueueFirst != NULL) m_nCurrentCommandsCount = 1;
    else m_nCurrentCommandsCount = 0;

    // odemceni prace se zpravama
    m_pCivilization->UnlockEvents();
}

//////////////////////////////////////////////////////////////////////

void CZUnit::EmptyNotificationQueue()
{
    CZUnitNotifyQueueItem *pPom;

    // zamceni prace se zpravama
    m_pCivilization->LockEvents();

	if (m_pNotifyQueueFirst != NULL)
	{
		// vyprazdnit frontu az na prvni prvek, ktery muze byt executed
		while (m_pNotifyQueueFirst->m_pNext != NULL)
		{
			pPom = m_pNotifyQueueFirst->m_pNext;
			m_pNotifyQueueFirst->m_pNext = m_pNotifyQueueFirst->m_pNext->m_pNext;
			pPom->Release();
		}
		m_pNotifyQueueLast = m_pNotifyQueueFirst;

		// process bezi
		if ( !g_bGameClosing)
		{
			// -> zabit proces
			SIKillProcessData *pKillProcessData = new SIKillProcessData;
			pKillProcessData->nProcessID = m_nNotifyProcessID;
			pKillProcessData->Status = STATUS_EMPTYING_QUEUE;
			pKillProcessData->m_bDelete = true;

			m_pCivilization->InlayEvent( KILL_PROCESS, (DWORD)pKillProcessData);
		}
		else
		{
			m_pNotifyQueueFirst->Release();
			m_pNotifyQueueFirst = m_pNotifyQueueLast = NULL;
			m_nNotifyProcessID = NULL;
		}
	}	

	ASSERT( m_pNotifyQueueFirst == m_pNotifyQueueLast);

    if ( m_pNotifyQueueFirst != NULL) m_nNotificationsCount = 1;
    else m_nNotificationsCount = 0;
    
	// odemceni prace se zpravama
    m_pCivilization->UnlockEvents();
}

//////////////////////////////////////////////////////////////////////

bool CZUnit::IsCommanderOfUnit( CZUnit *pUnit)
{
	m_pCivilization->LockHierarchyAndEvents();

	ASSERT( pUnit != NULL);
	pUnit = pUnit->m_pCommander;

	while (pUnit != NULL)
	{
		if (pUnit == this) 
		{
			m_pCivilization->UnlockHierarchyAndEvents();
			return true;
		}
		pUnit = pUnit->m_pCommander;
	}

	m_pCivilization->UnlockHierarchyAndEvents();
	return false;
}

//////////////////////////////////////////////////////////////////////
