/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Interpret
 *   Autor: Helena Kupková
 *  
 *   Popis: Implementace tridy CICodeStackItem, ktera reprezentuje
 *          jednu bunku na kodovem zasobniku, obsahuje kontext procesu.
 * 
 ***********************************************************/

#include "stdafx.h"
#include "..\Interpret.h"
#include "ISendtoInformation.h"

#include "TripleS\CodeManager\CodeManager.h"

extern bool g_bGameLoading;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

DEFINE_MULTITHREAD_POOL( CICodeStackItem, 20);

CICodeStackItem::CICodeStackItem()
{
    SetNone(); 
}

CICodeStackItem::~CICodeStackItem()
{
}

void CICodeStackItem::SetNone()
{
    m_pInterpret = NULL;
    m_pPrevious = NULL;

    m_pCode = NULL;
//    m_nRemainLength = 0;
    m_nBorder = -1;
    m_nCallReg = NO_REG;
    m_nArgAndLocalVarCount = 0;
    m_pBag = NULL;

    m_pSendtoInfo = NULL;
}

void CICodeStackItem::Create(CInterpret *pInterpret, CICodeStackItem *pPrevious, CICodeBlock *pCodeBlock,
        long nCallReg, CICodeStringTable *pCodeStringTable,
        CFloatTable *pFloatTable, CStringTableItem *stiUnitName, CStringTableItem *stiFunctionName)
{
    // navazani
    m_pInterpret = pInterpret;
    m_pPrevious = pPrevious;

    // kod
	m_pCodeBlock = pCodeBlock;
    m_pCode = pCodeBlock->m_Code.m_pCode;

    // data
    m_nCallReg = nCallReg;
    m_nArgAndLocalVarCount = m_pCodeBlock->m_nArgAndLocalVarCount;
    m_nBorder = nCallReg + m_nArgAndLocalVarCount;

    m_pCodeStringTable = pCodeStringTable;
    m_pFloatTable = pFloatTable;

    // ostatni
    m_pBag = NULL;

    m_pSendtoInfo = NULL;

    // jmena
    if (stiUnitName != NULL) m_stiUnitName = stiUnitName->AddRef();
    else m_stiUnitName = g_StringTable.AddItem(GLOBAL_FUNCTIONS_CLASS_NAME);
    m_stiFunctionName = stiFunctionName->AddRef();

    // zadny navratovy registr
    m_pReturnRegister = NULL;
}

void CICodeStackItem::Delete()
{
    if (g_bGameLoading)
    {
        if (m_pBag != NULL)
        {
            m_pBag->Delete();
            delete m_pBag;
            m_pBag = NULL;
        }

        if ( m_pSendtoInfo != NULL)
        {
            delete m_pSendtoInfo;
            m_pSendtoInfo = NULL;
        }

        return;
    }

    CIBag *pBag;

    // uvolneni bagu
    while (m_pBag != NULL)
    {
        pBag = m_pBag;
        m_pBag = m_pBag->m_pPreviousBag;
        delete pBag;
    }

    // odvazani
    m_pPrevious = NULL;
    m_pInterpret = NULL;

    // uvolneni jmen
    m_stiUnitName = m_stiUnitName->Release();
    m_stiFunctionName = m_stiFunctionName->Release();

    // uvolneni sendto informace
    if (m_pSendtoInfo != NULL) 
    {
        delete m_pSendtoInfo;
        m_pSendtoInfo = NULL;
    }
}

void CICodeStackItem::PersistentSave( CPersistentStorage &storage)
{
    BRACE_BLOCK(storage);

    // pointer na sebe
	storage << (DWORD)this;

    // pointer na predchozi zaznam
	storage << (DWORD)m_pPrevious;
    if (m_pPrevious != NULL) m_pPrevious->PersistentSave( storage);

// KOD, ktery se interpretuje
    // jmeno unity, jejiz metoda se provadi (u globalnich funkci je NULL)
    storage << (DWORD)m_stiUnitName;
    // jmeno metody/eventy/funkce, ktera se provadi
    storage << (DWORD)m_stiFunctionName;

    // aktualni pozice v kodu
    storage << (LONG)(m_pCode - m_pCodeBlock->m_Code.m_pCode);

// Dalsi veci
    // pointer na interpret
    storage << (DWORD)m_pInterpret;

    // "tlusta cara"  na data stacku
    storage << m_nBorder; 

    // registr, od ktereho probiha volani metody/funkce (pokud nejake probiha),
    storage << m_nCallReg;

    // pocet lokalnich promennych a argumentu (datovych polozek pod tlustou carou)
    storage << m_nArgAndLocalVarCount;
    
	// navratovy registr
    storage << (DWORD)m_pReturnRegister;

	// spojak (zasobnik) rozdelanych bagu
	storage << (DWORD)m_pBag;
	if (m_pBag != NULL) m_pBag->PersistentSave( storage);

	// SENDTO information - o rozdelanem sendto statementu
    storage << (DWORD)m_pSendtoInfo;
	if (m_pSendtoInfo != NULL) m_pSendtoInfo->PersistentSave( storage);
}

void CICodeStackItem::PersistentLoad( CPersistentStorage &storage)
{
    BRACE_BLOCK(storage);	
    
    // pointer na sebe
	CICodeStackItem *pThis;
	storage >> (DWORD&)pThis;
	storage.RegisterPointer( pThis, this);

    // pointer na predchozi zaznam
	storage >> (DWORD&)m_pPrevious;
    if (m_pPrevious != NULL) 
	{
		m_pPrevious = new CICodeStackItem;
		m_pPrevious->PersistentLoad( storage);
	}

// KOD, ktery se interpretuje
    // jmeno unity, jejiz metoda se provadi (u globalnich funkci je NULL)
    storage >> (DWORD&)m_stiUnitName;
    // jmeno metody/eventy/funkce, ktera se provadi
    storage >> (DWORD&)m_stiFunctionName;

    // aktualni pozice v kodu
	storage >> m_nPersistentLoadCodePosition;

// Dalsi veci
    // pointer na interpret
    storage >> (DWORD&)m_pInterpret;

    // "tlusta cara"  na data stacku
    storage >> m_nBorder; 

    // registr, od ktereho probiha volani metody/funkce (pokud nejake probiha),
    storage >> m_nCallReg;

    // pocet lokalnich promennych a argumentu (datovych polozek pod tlustou carou)
    storage >> m_nArgAndLocalVarCount;
    
	// navratovy registr
    storage >> (DWORD&)m_pReturnRegister;

	// spojak (zasobnik) rozdelanych bagu, ukazujou na sebe pomoci CIBag::m_pPrevious
	storage >> (DWORD&)m_pBag;

	if (m_pBag != NULL)
	{
		m_pBag = new CIBag;
		m_pBag->PersistentLoad( storage);
	}

	// SENDTO information - o rozdelanem sendto statementu
    storage >> (DWORD&)m_pSendtoInfo;
	if (m_pSendtoInfo != NULL) 
	{
		m_pSendtoInfo = new CISendtoInformation;
		m_pSendtoInfo->PersistentLoad( storage);
	}
}

void CICodeStackItem::PersistentTranslatePointers( CPersistentStorage &storage)
{
    // predchozi zaznam
    if (m_pPrevious != NULL) m_pPrevious->PersistentTranslatePointers( storage);

// KOD, ktery se interpretuje
    
	// jmeno unity, jejiz metoda se provadi (u globalnich funkci je NULL)
    m_stiUnitName = (CStringTableItem *) storage.TranslatePointer( m_stiUnitName);
    
	// jmeno metody/eventy/funkce, ktera se provadi
    m_stiFunctionName = (CStringTableItem *) storage.TranslatePointer( m_stiFunctionName);

// Dalsi veci
    // pointer na interpret
    m_pInterpret = (CInterpret*)storage.TranslatePointer( m_pInterpret);

	// navratovy registr
    m_pReturnRegister = (CIDataStackItem*) storage.TranslatePointer( m_pReturnRegister);

   if (m_pBag != NULL)
   {
   	// spojak (zasobnik) rozdelanych bagu, ukazujou na sebe pomoci CIBag::m_pPrevious
   	m_pBag->PersistentTranslatePointers( storage);
   }
}

void CICodeStackItem::PersistentInit()
{
    // predchozi zaznam
    if (m_pPrevious != NULL) m_pPrevious->PersistentInit();

    // NASTAVENI KODU pro implementaci

	CStringTableItem *stiGlobals = g_StringTable.AddItem( GLOBAL_FUNCTIONS_CLASS_NAME);

	if (m_stiUnitName == stiGlobals)
	// globalni funkce
	{
		stiGlobals->Release();

		CIGlobalFunction *pGlobalFunction;
        
        // globalni funkce, ktera byla interpretovana, najednou neni naloadovana?
		LOAD_ASSERT( (pGlobalFunction = m_pInterpret->m_pCodeManager->GetGlobalFunction( m_stiFunctionName)) != NULL);

		// kod & pozice
		m_pCodeBlock = pGlobalFunction;
		m_pCode = pGlobalFunction->m_Code.m_pCode + m_nPersistentLoadCodePosition;

		// tabulky
		m_pCodeStringTable = pGlobalFunction->m_pCodeStringTable;
		m_pFloatTable = pGlobalFunction->m_pFloatTable;
	}
	else
	{
		stiGlobals->Release();

		CIUnitType *pIUnitType;
		LOAD_ASSERT( (pIUnitType = m_pInterpret->m_pCodeManager->GetUnitType( m_stiUnitName)) != NULL);

        // konstruktor
        if ( m_stiFunctionName == m_stiUnitName)
        {
            CIConstructor *pConstructor;

            LOAD_ASSERT( (pConstructor = pIUnitType->m_pConstructor) != NULL);

		    m_pCodeBlock = pConstructor;
        }
        // destruktor
        else if ( ((CString)*m_stiFunctionName)[0] == '~')
        {
            CIDestructor *pDestructor;

            if ( ( pDestructor = pIUnitType->m_pDestructor) == NULL)
            {
				// destruktor, ktery byl interpretovan, najednou neni
				LOAD_ASSERT(false); 
				return;
            }

		    m_pCodeBlock = pDestructor;
        }
        // eventa nebo metoda
        else
        {
		    CIEvent *pEvent;
		    if (pIUnitType->GetEvent( m_stiFunctionName, pEvent) != IMPL_SCRIPT)
		    {
			    CIMethod *pMethod;
			    if (pIUnitType->GetMethod( m_stiFunctionName, pMethod) != IMPL_SCRIPT)
			    {
				    // metoda/eventa, ktera byla interpretovana, najednou neni
				    LOAD_ASSERT(false); 
				    return;
			    }
			    pEvent = pMethod;
		    }

		    m_pCodeBlock = pEvent;
        }

        // aktualni pozice v kodu
		m_pCode = m_pCodeBlock->m_Code.m_pCode + m_nPersistentLoadCodePosition;

		// tabulky
		m_pCodeStringTable = pIUnitType->m_pCodeStringTable;
		m_pFloatTable = pIUnitType->m_pFloatTable;

	}

   if (m_pBag != NULL)
   {
      // bagy
      m_pBag->PersistentInit();
   }
}

