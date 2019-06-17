/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Interpret
 *   Autor: Helena Kupková
 *  
 *   Popis: Implementace tridy CIProcess reprezentujici jeden process 
 *          interpretovany interpretem.
 *          
 ***********************************************************/

#include "stdafx.h"
#include "..\Interpret.h"

#include "TripleS\CodeManager\CodeManager.h"

// civilizace a CZUnit
#include "GameServer\Civilization\ZCivilization.h"
#include "GameServer\Civilization\Src\ZUnit.h"

BEGIN_OBSERVER_MAP(CIProcess, CObserver)
    BEGIN_NOTIFIER( WAITING_INTERFACE_NOTIFIER_ID)
        EVENT( WAITING_FINISHED)
            m_pInterpret->Wakeup(this, (EInterpretStatus) dwParam);
            return true;
    END_NOTIFIER()
END_OBSERVER_MAP(CMyObserver, CObserver)


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

DEFINE_MULTITHREAD_POOL( CIProcess, 10);

CIProcess::CIProcess()
{
    m_pCodeStack = NULL;
    m_pNext = NULL;
    m_pPrevious = NULL;
    m_bCreated = false;
    m_nProcessType = PROCESS_NONE;

#ifdef _DEBUG
    m_stiName = NULL;
#endif
}

CIProcess::~CIProcess()
{
}

EInterpretStatus CIProcess::CreateConstructorDestructorCall(CInterpret *pInterpret, CZUnit *pUnit, 
															bool bConstructor)
{
    ASSERT(pInterpret != NULL);
    ASSERT(pUnit != NULL);

    CIConstructor *pConstructor;
    CIUnitType *pIUnitType = pUnit->m_pIUnitType;
    EInterpretStatus st;

    ASSERT(pIUnitType->m_stiName != NULL);

    
    // inicializace objektu pro cekani, az dobehne konstruktor (pro destruktor se nepouziva)
    if (bConstructor)
    {
        m_nProcessType = PROCESS_CONSTRUCTOR;
        m_pUnit = pUnit;

        // inicializace jmena a pomocne promenne (ptr. na kod)
        pConstructor = pIUnitType->m_pConstructor;
        m_stiName = pIUnitType->m_stiName->AddRef();

    }
    else 
    {
        m_nProcessType = PROCESS_DESTRUCTOR;

        // inicializace jmena a pomocne promenne (ptr. na kod)
        pConstructor = pIUnitType->m_pDestructor;
        m_stiName = g_StringTable.AddItem( "~" + (CString)*pIUnitType->m_stiName);
    }

    // interpret
    m_pInterpret = pInterpret;

    // code stack
    m_pCodeStack = new CICodeStackItem;
    
    if (bConstructor)
	{
		m_pCodeStack->Create(pInterpret, NULL, pConstructor, 0, pIUnitType->m_pCodeStringTable, 
			pIUnitType->m_pFloatTable, pIUnitType->m_stiName, pIUnitType->m_stiName);
	}
	else
	{
		m_pCodeStack->Create(pInterpret, NULL, pConstructor, 0, pIUnitType->m_pCodeStringTable, 
			pIUnitType->m_pFloatTable, pIUnitType->m_stiName, g_StringTable.AddItem( "~" + (CString)*pIUnitType->m_stiName, false));
	}

    // data stack
    m_DataStack.Create(pInterpret);

    // neni nikde zapojen
    m_pNext = m_pPrevious = NULL;

    // inicializace lokalnich promennych
    int i;
    for (i=0; i < pConstructor->m_nArgAndLocalVarCount; i++)
    {
        st = m_DataStack[ m_pCodeStack->m_nBorder - i-1 ].SetTypeAndCreate(
                                                pConstructor->m_apArgAndLocalVars[i],
                                                pInterpret);
        if (st != STATUS_SUCCESS) return st;
    }

    // this
    m_DataStack[ m_pCodeStack->m_nBorder ].Set( pUnit);

    // stav
    m_nState = CREATED;

    // process ID
    m_nProcessID = m_pInterpret->GetProcessID();

    m_pEventFinished = NULL;
    m_pWaitingInterface = NULL;

    m_bCreated = true;

    pInterpret->m_nProcessCount++;

    return STATUS_SUCCESS;
}

EInterpretStatus CIProcess::CreateEventCall(CInterpret *pInterpret, CZUnit *pUnit,
                                CIEvent *pEvent, CIBag *pBag, CIWaitingInterface *pEventFinished,
                                CIProcess::EProcessType eEventType)
{
    ASSERT(pInterpret != NULL);
    ASSERT(pUnit != NULL);
    ASSERT(pEvent != NULL);
    ASSERT(pBag != NULL);

    CIUnitType *pIUnitType = pEvent->m_pIUnitType;
    EInterpretStatus status;

	// kontrola poctu argumentu
	if (pBag->GetArgCount() != pEvent->m_nArgCount) return STATUS_INVALID_ARGUMENT_COUNT;

    // cekani na eventu
    m_pEventFinished = pEventFinished;
    if (m_pEventFinished != NULL) m_pEventFinished->AddRef();
    m_pUnit = pUnit;

    // jmeno procesu
    m_stiName = g_StringTable.AddItem((CString)*pUnit->m_pIUnitType->m_stiName + "::" +
            (CString)*pEvent->m_stiName);

    m_pInterpret = pInterpret;

    // code stack
    m_pCodeStack = new CICodeStackItem;
    m_pCodeStack->Create(pInterpret, NULL, pEvent, 0, pIUnitType->m_pCodeStringTable, 
		pIUnitType->m_pFloatTable, pIUnitType->m_stiName, pEvent->m_stiName);

    // datastack
    m_DataStack.Create(pInterpret);

    // neni nikde zapojen
    m_pNext = m_pPrevious = NULL;

    // inicializace argumentu
    int i;
    for (i=0; i<pEvent->m_nArgCount; i++)
    {
        // kontrola typu
        if (pBag->m_aArgs[i].SameTypeAsAndConvertUnits(pEvent->m_apArgAndLocalVars[i]))
        {
            m_DataStack[ m_pCodeStack->m_nBorder - i-1 ] = pBag->m_aArgs[i];
        }
        else
        {
            pInterpret->ReportError( "%s::%s: argument %d: type mismatch", 
                (CString)*pUnit->m_pIUnitType->m_stiName,
                (CString)*pEvent->m_stiName, i+1);

            status = STATUS_ARGUMENT_TYPE_MISMATCH;
            goto Error;
        }
    }

    // inicializace lokalnich promennych
    for (; i<pEvent->m_nArgAndLocalVarCount; i++)
    {
        status = m_DataStack[ m_pCodeStack->m_nBorder - i-1 ].SetTypeAndCreate(
                                                pEvent->m_apArgAndLocalVars[i],
                                                pInterpret);
        if (status != STATUS_SUCCESS) goto Error;
    }

    // this
    m_DataStack[ m_pCodeStack->m_nBorder ].Set( pUnit, pIUnitType);

    // stav
    m_nState = CREATED;

    // process ID
    m_nProcessID = m_pInterpret->GetProcessID();

    m_nProcessType = eEventType;

    m_pWaitingInterface = NULL;

    m_bCreated = true;

    pInterpret->m_nProcessCount++;

    return STATUS_SUCCESS;

Error:
    // zruseni vseho
    m_DataStack.Delete();
    m_pCodeStack->Delete();
    delete m_pCodeStack;
    m_pCodeStack = NULL;

#ifdef _DEBUG            
    m_stiName->Release();
#endif

    return status;
}

EInterpretStatus  CIProcess::CreateGlobalFunctionCall( CInterpret *pInterpret, 
                                                       CIGlobalFunction *pGlobalFunction, CIBag *pBag)
{
    ASSERT(pInterpret != NULL);
    ASSERT(pGlobalFunction != NULL);
    ASSERT(pBag != NULL);

    EInterpretStatus status;

	// kontrola poctu argumentu
	if ( pBag->GetArgCount() != pGlobalFunction->m_nArgCount) return STATUS_INVALID_ARGUMENT_COUNT;

    // interpret
    m_pInterpret = pInterpret;

    // code stack
    m_pCodeStack = new CICodeStackItem;
    
    m_pCodeStack->Create(pInterpret, NULL, pGlobalFunction, 0, pGlobalFunction->m_pCodeStringTable, 
        pGlobalFunction->m_pFloatTable, NULL, pGlobalFunction->m_stiName);

    // data stack
    m_DataStack.Create(pInterpret);

    // neni nikde zapojen
    m_pNext = m_pPrevious = NULL;

    // inicializace argumentu
    int i;
    for (i=0; i<pGlobalFunction->m_nArgCount; i++)
    {
        // kontrola typu
        if (pBag->m_aArgs[i].SameTypeAsAndConvertUnits(pGlobalFunction->m_apArgAndLocalVars[i]))
        {
            m_DataStack[ m_pCodeStack->m_nBorder - i-1 ] = pBag->m_aArgs[i];
        }
        else
        {
            pInterpret->ReportError( "%s: argument %d: type mismatch",  
                (CString)*pGlobalFunction->m_stiName, i+1);

            status = STATUS_ARGUMENT_TYPE_MISMATCH;
            goto Error;
        }
    }

    // inicializace lokalnich promennych
    for ( ; i < pGlobalFunction->m_nArgAndLocalVarCount; i++)
    {
        status = m_DataStack[ m_pCodeStack->m_nBorder - i-1 ].SetTypeAndCreate(
                                                pGlobalFunction->m_apArgAndLocalVars[i],
                                                pInterpret);
        if (status != STATUS_SUCCESS) goto Error;
    }

    // this
    m_DataStack[ m_pCodeStack->m_nBorder ].Set( (CZUnit*)NULL ); // pro jistotu, 'this' by se nemel v glob.fci vyskytovat

    // stav
    m_nState = CREATED;

    // jmeno
    m_stiName = pGlobalFunction->m_stiName->AddRef();

    // process ID
    m_nProcessID = m_pInterpret->GetProcessID();

	m_nProcessType = PROCESS_GLOBALFUNCTION;

    m_pEventFinished = NULL;
    m_pWaitingInterface = NULL;

    m_bCreated = true;

    pInterpret->m_nProcessCount++;

    return STATUS_SUCCESS;

Error:

    // zruseni vseho
    m_DataStack.Delete();
    m_pCodeStack->Delete();
    delete m_pCodeStack;
    m_pCodeStack = NULL;

    return status;
}


void CIProcess::Delete( EInterpretStatus eStatus)
{
    // uklid po padu pri loadovani hry
    if ( g_bGameLoading)
    {
        CICodeStackItem *pPom;

        // Uvolneni code stacku
        while (m_pCodeStack != NULL)
        {
            // prepojeni a uvolneni
            pPom = m_pCodeStack->m_pPrevious;
            m_pCodeStack->Delete();
            delete m_pCodeStack;
        
            m_pCodeStack = pPom;
        }
            
        return;
    }

    CICodeStackItem *pPom;

	// spal jsem, na neco jsem cekal, a oni me killnuli -> zrusit cekani
	if (m_nState == SLEEPING)
	{
		ASSERT( m_pWaitingInterface != NULL);
		if (eStatus != STATUS_GAME_CLOSING)  // konci se hra -> nevolat Cancel
		{
			m_pWaitingInterface->Cancel( this);
		}

		m_pWaitingInterface->Release();
		m_pWaitingInterface = NULL;
	}

    // vzbuzeni tech, co cekaji
    switch (m_nProcessType)
    {
    case PROCESS_CONSTRUCTOR:
        // cekani, az dojede konstruktor
        ASSERT( m_pUnit != NULL);
        ASSERT( m_pUnit->m_pConstructorFinished != NULL);
  
        // at uz konstruktor skoncil jakkoli, cekajici se vzbudi
		  if (eStatus != STATUS_GAME_CLOSING)
		  {
				m_pUnit->m_pConstructorFinished->FinishWaiting( STATUS_SUCCESS);
		  }
        m_pUnit->m_pConstructorFinished->Release();  // to zavola Delete a uvolni ho, kdyz 
                                                     // m_nRefCount == 0, coz by mel
        m_pUnit->m_pConstructorFinished = NULL;
 
        break;

    case PROCESS_DESTRUCTOR:
        // na destruktor muze nekdo cekat, aby mohl jednotku uvolnit 
        // (nekdo z venku)
        // --> tady se to zajisti, aby se to dozvedel
        break;

    case PROCESS_EVENT_NOTIFICATION:
        // cekani az dojede eventa
        if (m_pEventFinished != NULL)
        {
            m_pEventFinished->FinishWaiting( eStatus);
            m_pEventFinished->Release();
        }

        ASSERT(m_pUnit != NULL);
		  if (eStatus != STATUS_GAME_CLOSING) 
		  {
				m_pUnit->OnEventFinished(this, EVENT_NOTIFICATION, eStatus, m_pInterpret);
		  }
        break;

    case PROCESS_EVENT_CURRENT_COMMAND:
        // cekani az dojede eventa
		  if (m_pEventFinished != NULL)
		  {
			  if (eStatus != STATUS_GAME_CLOSING)
			  {
					m_pEventFinished->FinishWaiting( eStatus);
			  }
			  m_pEventFinished->Release();
		  }

        ASSERT(m_pUnit != NULL);
		  if (eStatus != STATUS_GAME_CLOSING) 
		  {
		      m_pUnit->OnEventFinished(this, EVENT_COMMAND, eStatus, m_pInterpret);
		  }
        break;

    case PROCESS_EVENT_MANDATORY_COMMAND:
        // cekani az dojede eventa
        if (m_pEventFinished != NULL)
        {
			  if (eStatus != STATUS_GAME_CLOSING)
			  {
					m_pEventFinished->FinishWaiting( eStatus);
			  }
           m_pEventFinished->Release();
        }

        ASSERT(m_pUnit != NULL);
		  if (eStatus != STATUS_GAME_CLOSING) 
		  {
				m_pUnit->OnEventFinished(this, EVENT_COMMAND, eStatus, m_pInterpret);
		  }
        break;
    }

    // Uvolneni code stacku
    while (m_pCodeStack != NULL)
    {
        // prepojeni a uvolneni
        pPom = m_pCodeStack->m_pPrevious;
        m_pCodeStack->Delete();
        delete m_pCodeStack;
        
        m_pCodeStack = pPom;
    }

    // data stack - uvolni vsechny lokalni promenne, argumenty, ....
    m_DataStack.Delete();

    // jmeno
    ASSERT(m_stiName!=NULL);
    m_stiName = m_stiName->Release();

    // neni nikde zapojen
    m_pNext = m_pPrevious = NULL;

    // stav
    m_nState = CIProcess::DESTROYED;

    // typ procesu
    m_nProcessType = PROCESS_NONE;

    m_pInterpret->m_nProcessCount--;
    
    m_bCreated = false;
}

EInterpretStatus CIProcess::CallParentConstructorOrDestructor(bool bConstructor)
{
    // this
    CIDataStackItem *pThisReg = &m_DataStack[ m_pCodeStack->m_nBorder];
    ASSERT( pThisReg->IsUnit() );
    ASSERT( pThisReg->GetUnit() != NULL);

    // pomocne promenne pro casto pouzivane veci
    CIUnitType *pParent = pThisReg->GetUnitType()->m_pParent;
    EInterpretStatus st;

    if (pParent->m_stiName != m_pInterpret->m_stiDefaultUnit)
    // skriptovy predek - vnorene volani
    {
        CIConstructor *pConstr;
        CICodeStackItem *pNewCodeStack = new CICodeStackItem;

        // vyber kodu
        if (bConstructor) 
        {
            pConstr = pParent->m_pConstructor;
            
            // code stack
            pNewCodeStack->Create(m_pInterpret, m_pCodeStack, pConstr, 
				m_DataStack.GetFirstSurelyUnusedRegister(), pParent->m_pCodeStringTable, 
				pParent->m_pFloatTable, pParent->m_stiName, pParent->m_stiName);
        }
        else 
        {
            pConstr = pParent->m_pDestructor;

            // code stack
            pNewCodeStack->Create(m_pInterpret, m_pCodeStack, pConstr,  
				m_DataStack.GetFirstSurelyUnusedRegister(),
                pParent->m_pCodeStringTable, pParent->m_pFloatTable, pParent->m_stiName, 
                g_StringTable.AddItem( "~" + (CString)*pParent->m_stiName)  );
        }

        m_pCodeStack = pNewCodeStack;

        // inicializace lokalnich promennych
        int i;
        int border = m_pCodeStack->m_nBorder;

        for (i=0; i<pConstr->m_nArgAndLocalVarCount; i++)
        {
            st = m_DataStack[ border - i-1 ].SetTypeAndCreate(
                                                pConstr->m_apArgAndLocalVars[i],
                                                m_pInterpret);
            if (st != STATUS_SUCCESS) return st;
        }

        // this do Reg(0)
        m_DataStack[ border ].Set( pThisReg->GetUnit(), pParent);
    }

    return STATUS_SUCCESS;
}

EInterpretStatus CIProcess::CallMethod(CZUnit *pUnit, CIMethod *pMethod)
{
    EInterpretStatus st;
    CIUnitType *pIUnitType = pMethod->m_pIUnitType;

    // code stack
    CICodeStackItem *pNewCodeStack = new CICodeStackItem;
    pNewCodeStack->Create(m_pInterpret, m_pCodeStack, pMethod, 
		m_DataStack.GetFirstSurelyUnusedRegister(),
        pIUnitType->m_pCodeStringTable, pIUnitType->m_pFloatTable,
        pIUnitType->m_stiName, pMethod->m_stiName);

    // inicializace argumentu
    int i;
    for (i=0; i<pMethod->m_nArgCount; i++)
    {
        // kontrola typu
        if (m_pCodeStack->m_pBag->m_aArgs[i].SameTypeAsAndConvertUnits(pMethod->m_apArgAndLocalVars[i]))
        {
            m_DataStack[ pNewCodeStack->m_nBorder - i-1 ] = *m_pCodeStack->m_pBag->GetArg(i);
        }
        else
        {
            m_pInterpret->ReportError( "%s::%s: argument %d: type mismatch", 
                (CString)*pIUnitType->m_stiName,
                (CString)*pMethod->m_stiName, i+1);

            pNewCodeStack->Delete();
            delete pNewCodeStack;
            return STATUS_ARGUMENT_TYPE_MISMATCH;
        }
    }

    // inicializace lokalnich promennych
    for (; i<pMethod->m_nArgAndLocalVarCount; i++)
    {
        st = m_DataStack[ pNewCodeStack->m_nBorder - i-1 ].SetTypeAndCreate(
                                                pMethod->m_apArgAndLocalVars[i],
                                                m_pInterpret);
        if (st != STATUS_SUCCESS) return st;
    }

    // this
    m_DataStack[ pNewCodeStack->m_nBorder ].Set( pUnit, pIUnitType);

    // navratova hodnota
    pNewCodeStack->m_pReturnRegister = m_pCodeStack->m_pBag->GetReturnRegister();

    // nastaveni code stacku
    m_pCodeStack = pNewCodeStack;

    return STATUS_SUCCESS;
}

EInterpretStatus CIProcess::CallFunction( CIGlobalFunction *pGlobalFunc)
{
    EInterpretStatus st;

    // code stack
    CICodeStackItem *pNewCodeStack = new CICodeStackItem;
    pNewCodeStack->Create(m_pInterpret, m_pCodeStack, pGlobalFunc, 
		m_DataStack.GetFirstSurelyUnusedRegister(),
        pGlobalFunc->m_pCodeStringTable, pGlobalFunc->m_pFloatTable, 
        NULL, pGlobalFunc->m_stiName);

    // inicializace argumentu
    int i;
    for (i=0; i<pGlobalFunc->m_nArgCount; i++)
    {
        // kontrola typu
        if (m_pCodeStack->m_pBag->m_aArgs[i].SameTypeAsAndConvertUnits(pGlobalFunc->m_apArgAndLocalVars[i]))
        {
            m_DataStack[ pNewCodeStack->m_nBorder - i-1 ] = *m_pCodeStack->m_pBag->GetArg(i);
        }
        else
        {
            m_pInterpret->ReportError( "%s: argument %d: type mismatch", 
                (CString)*pGlobalFunc->m_stiName, i+1);

            pNewCodeStack->Delete();
            delete pNewCodeStack;
            return STATUS_ARGUMENT_TYPE_MISMATCH;
        }
    }

    // inicializace lokalnich promennych
    for (; i<pGlobalFunc->m_nArgAndLocalVarCount; i++)
    {
        st = m_DataStack[ pNewCodeStack->m_nBorder - i-1 ].SetTypeAndCreate(
                                                pGlobalFunc->m_apArgAndLocalVars[i],
                                                m_pInterpret);
        if (st != STATUS_SUCCESS) return st;
    }

    // this zadne neni
    m_DataStack[ pNewCodeStack->m_nBorder ].Clean();

    // navratova hodnota
    pNewCodeStack->m_pReturnRegister = m_pCodeStack->m_pBag->GetReturnRegister();

    // nastaveni code stacku
    m_pCodeStack = pNewCodeStack;

    return STATUS_SUCCESS;
}
 
EInterpretStatus CIProcess::Return()
{
    // navrat z volani, neni to ukonceni procesu
    ASSERT(m_pCodeStack->m_pPrevious != NULL);

    // uvolneni promennych
    int i;
    int border = m_pCodeStack->m_nBorder;

    for (i=0; i < m_pCodeStack->m_nArgAndLocalVarCount; i++)
    {
        m_DataStack[ border -i-1].Clean();
    }

    // data stack - uvolneni pridanych setu
    m_DataStack.FreeSetsFromRegister(m_pCodeStack->m_nCallReg);

    // code stack
    CICodeStackItem *pOldCodeStack = m_pCodeStack;

    m_pCodeStack = m_pCodeStack->m_pPrevious;

    pOldCodeStack->Delete();
    delete pOldCodeStack;

    return STATUS_SUCCESS;
}

// nalezeni odesilatele zpravy = jednotky, ktere je to metoda/ktera volala globalni funkci ...
CZUnit* CIProcess::FindSenderUnit()
{
    CICodeStackItem *pCS = m_pCodeStack;
    
    while ( pCS != NULL)
    {
        if (m_DataStack[ pCS->m_nBorder].IsUnit()) return m_DataStack[ pCS->m_nBorder].GetUnit();
        else pCS = pCS->m_pPrevious;
    }

    return NULL;
}

// Dotaz na to, zda proces obsahuje codestack, ktery runi zadanou
// metodu nebo eventu zadane jednotky.
bool CIProcess::ContainsMethodOrEventOfType( CStringTableItem *stiUnitName, CStringTableItem *stiMethodEventName)
{
    CICodeStackItem *pCodeStack = m_pCodeStack;

    while (pCodeStack != NULL)
    {
        if (pCodeStack->m_stiUnitName == stiUnitName && 
            pCodeStack->m_stiFunctionName == stiMethodEventName) return true;
        else pCodeStack = pCodeStack->m_pPrevious;
    }

    return false;
}

// Dotaz na to, zda proces obsahuje codestack, ktery runi jakoukoli
// metodu nebo eventu od zadane jednotky nebo jejiho potomka.
// !! Tohle bezi kdyz je mutex CCodeManager::m_tpScripts zamcenej, takze se CCodeManager nesmi volat, jinak bude deadlock
bool CIProcess::ContainsUnitOfType( CIUnitType *pIUnitType)
{
    CICodeStackItem *pCodeStack = m_pCodeStack;

    while (pCodeStack != NULL)
    {
		// ziskani typu jednotky, nad kterym runi aktualni codestack
		// pruchod stromem dedicnosti az k predkovi Unit
		while (pIUnitType != CCodeManager::m_pDefaultUnitType)
		{
			ASSERT( pIUnitType != NULL);

			if (pCodeStack->m_stiUnitName == pIUnitType->m_stiName) return true;
			pIUnitType = pIUnitType->m_pParent;
		}
        pCodeStack = pCodeStack->m_pPrevious;
    }

    return false;
}

// dotaz, zda obsahuje instanci dané jednotky (zda vola nekterou jeji metodu) (bezi ve threadu interpreta)
bool CIProcess::ContainsInstanceOfUnit( CZUnit *pUnit)
{
    CICodeStackItem *pCodeStack = m_pCodeStack;
	CIDataStackItem *pDSI;

    while (pCodeStack != NULL)
    {
		pDSI = &m_DataStack[ pCodeStack->m_nBorder];
		// this
		if (pDSI->IsUnit() && pDSI->GetUnit() == pUnit) return true;

		pCodeStack = pCodeStack->m_pPrevious;
	}

	return false;
}			

//
// SAVE & LOAD
//
void CIProcess::PersistentSave( CPersistentStorage &storage)
{
    BRACE_BLOCK(storage);

    // ukazatel na sebe
	storage << (DWORD)this;

	// observer
	CObserver::PersistentSave( storage);

    // interpret
    storage << (DWORD)m_pInterpret;

    // kodovy zasobnik
	m_pCodeStack->PersistentSave( storage);
    
    // datovy zasobnik
    m_DataStack.PersistentSave( storage);
    
    // jmeno procesu (napr. MyUnit::MOVE)
    storage << (DWORD)m_stiName;

	// stav procesu
    storage << (DWORD)m_nState;

	// retezeni ve fronte cekajicich nebo spicich procesu 
    storage << (DWORD)m_pNext << (DWORD)m_pPrevious;

	// process ID
    storage << m_nProcessID;

	// typ procesu
    storage << (DWORD) m_nProcessType;
	
	// jednotka, na ktere se to vola
    // platne pro PROCESS_CONSTRUCTOR, PROCESS_DESTRUCTOR, PROCESS_EVENT_XXX
    storage << (DWORD)m_pUnit;

	// nekdo ceka na dokonceni eventy
    // - platne pro PROCESS_EVENT_XXX_COMMAND
	// - po dokonceni se na tomto vola WaitingFinished => ukladame
    storage << (DWORD)m_pEventFinished;
	if (m_pEventFinished != NULL) PersistentSaveWaiting( m_pEventFinished, storage);

	// Na co process ceka (platne kdyz m_nState == SLEEPING);
	// - pri zabiti ve spanku se na tomto vola Cancel (ukonci cinnost, na kterou jsem cekal)
	storage << (DWORD)m_pWaitingInterface;
	
}

void CIProcess::PersistentLoad( CPersistentStorage &storage)
{
    BRACE_BLOCK(storage);

    // ukazatel na sebe
	CIProcess *pThis;
	storage >> (DWORD&)pThis;
	storage.RegisterPointer( pThis, this);

	// observer
	CObserver::PersistentLoad( storage);

    // interpret
    storage >> (DWORD&)m_pInterpret;

    // kodovy zasobnik
    m_pCodeStack = new CICodeStackItem;
	m_pCodeStack->PersistentLoad( storage);
    
    // datovy zasobnik
    m_DataStack.PersistentLoad( storage);
    
    // jmeno procesu (napr. MyUnit::MOVE)
    storage >> (DWORD&) m_stiName;

	// stav procesu
    storage >> (DWORD&)m_nState;

	// retezeni ve fronte cekajicich nebo spicich procesu 
    storage >> (DWORD&)m_pNext >> (DWORD&)m_pPrevious;

	// process ID
    storage >> m_nProcessID;

	// typ procesu
    storage >> (DWORD&) m_nProcessType;
	
	// jednotka, na ktere se to vola
    // platne pro PROCESS_CONSTRUCTOR, PROCESS_DESTRUCTOR, PROCESS_EVENT_XXX
    storage >> (DWORD&)m_pUnit;

	// nekdo ceka na dokonceni eventy
    // - platne pro PROCESS_EVENT_XXX_COMMAND
	// - po dokonceni se na tomto vola WaitingFinished => ukladame
    storage >> (DWORD&)m_pEventFinished;
	if (m_pEventFinished != NULL) 
	{
		m_pEventFinished = PersistentLoadWaiting( storage);
	}

	// Na co process ceka (platne kdyz m_nState == SLEEPING);
	// - pri zabiti ve spanku se na tomto vola Cancel (ukonci cinnost, na kterou jsem cekal)
	storage >> (DWORD&)m_pWaitingInterface;
}

void CIProcess::PersistentTranslatePointers( CPersistentStorage &storage)
{
	// observer
	CObserver::PersistentTranslatePointers( storage);

	// interpret
    m_pInterpret = (CInterpret*) storage.TranslatePointer( m_pInterpret);

    // kodovy zasobnik
	m_pCodeStack->PersistentTranslatePointers( storage);
    
    // datovy zasobnik
    m_DataStack.PersistentTranslatePointers( storage);
    
    // jmeno procesu (napr. MyUnit::MOVE)
    m_stiName = (CStringTableItem*) storage.TranslatePointer( m_stiName);

	// retezeni ve fronte cekajicich nebo spicich procesu 
    m_pNext = (CIProcess*) storage.TranslatePointer( m_pNext);
	m_pPrevious = (CIProcess*) storage.TranslatePointer( m_pPrevious);

	// jednotka, na ktere se to vola
    // platne pro PROCESS_CONSTRUCTOR, PROCESS_DESTRUCTOR, PROCESS_EVENT_XXX
    if(m_nProcessType==PROCESS_CONSTRUCTOR || m_nProcessType==PROCESS_DESTRUCTOR || 
        m_nProcessType==PROCESS_EVENT_NOTIFICATION || m_nProcessType==PROCESS_EVENT_MANDATORY_COMMAND
        || m_nProcessType==PROCESS_EVENT_CURRENT_COMMAND) 
    // Mimochodem, Heleno, tenhle ifak jsem psal ja, Roman, a fakt jsem zuril. ;-)
    {
        
        m_pUnit = (CZUnit*)storage.TranslatePointer ( m_pUnit);
    }

	// nekdo ceka na dokonceni eventy
    // - platne pro PROCESS_EVENT_XXX_COMMAND
	// - po dokonceni se na tomto vola WaitingFinished => ukladame
	if (m_pEventFinished != NULL) 
	{
		PersistentTranslatePointersWaiting( m_pEventFinished, storage);
	}

	// Na co process ceka (platne kdyz m_nState == SLEEPING);
	// - pri zabiti ve spanku se na tomto vola Cancel (ukonci cinnost, na kterou jsem cekal)
	m_pWaitingInterface = (CIWaitingInterface*) storage.TranslatePointer( m_pWaitingInterface);
}

void CIProcess::PersistentInit()
{
	// observer
	CObserver::PersistentInit();

    m_pCodeStack->PersistentInit();

    m_DataStack.PersistentInit();

	m_bCreated = true;
}

