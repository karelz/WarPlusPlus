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

#if !defined(AFX_ICODESTACKITEM_H__12901454_A24A_11D3_AF86_004F49068BD6__INCLUDED_)
#define AFX_ICODESTACKITEM_H__12901454_A24A_11D3_AF86_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CIDataStackItem;
class CISendtoInformation;

class CICodeStackItem
{
public:
	CICodeStackItem();
	virtual ~CICodeStackItem();

    DECLARE_MULTITHREAD_POOL( CICodeStackItem);

public:
    // inicializace
    void Create(CInterpret *pInterpret, CICodeStackItem *pPrevious, CICodeBlock *pCodeBlock,
        long nDataStackFirstFreeItem, CICodeStringTable *pCodeStringTable, CFloatTable *pFloatTable,
        CStringTableItem *stiUnitName, CStringTableItem *stiFunctionName);

    // uvolneni
    void Delete();

public:
    // pointer na predchozi zaznam
    CICodeStackItem *m_pPrevious;

    // pointer na interpret
    CInterpret *m_pInterpret;

public:
// KOD
    // ukazatel na aktualni pozici v kodu
    INSTRUCTION *m_pCode;   
    
	// kod (konstruktor, eventa, metoda, globalni funkce..)
	CICodeBlock *m_pCodeBlock;

#ifdef _DEBUG
    // kolik zbyva bytu do konce kodu
//    long m_nRemainLength;  
#endif

// DATA STACK
    // "tlusta cara"
    long m_nBorder; 
    // registr, od ktereho probiha volani metody/funkce (pokud nejake probiha),
    // po navratu se musi zavolat CIDataStack::FreeSetsFromRegister(m_nCallReg);
    // Vice viz komentar u teto metody.
    REG m_nCallReg;
    // pocet lokalnich promennych a argumentu (datovych polozek pod tlustou carou)
    long m_nArgAndLocalVarCount;
    // navratovy registr
    CIDataStackItem *m_pReturnRegister;

// tabulky
    CICodeStringTable *m_pCodeStringTable;
    CFloatTable *m_pFloatTable;

// spojak (zasobnik) rozdelanych bagu, ukazujou na sebe pomoci CIBag::m_pPrevious
    CIBag *m_pBag;

// SENDTO information - o rozdelanem sendto statementu
    CISendtoInformation *m_pSendtoInfo;

// JMENA
    // jmeno unity, jejiz metoda se provadi (u globalnich funkci je NULL)
    CStringTableItem *m_stiUnitName;
    // jmeno metody/eventy/funkce, ktera se provadi
    CStringTableItem *m_stiFunctionName;

protected:
    // nastaveni 'zadnych' hodnot (inicializace promennych napr. v konstruktoru)
    void SetNone();

public:
	// profiling refcounts
	inline void IncrementProfileCounter() { m_pCodeBlock->IncrementProfileCounter(); }

public:
//
// SAVE & LOAD
//
	void PersistentSave( CPersistentStorage &storage);
	void PersistentLoad( CPersistentStorage &storage);
	void PersistentTranslatePointers( CPersistentStorage &storage);
	void PersistentInit();

	long m_nPersistentLoadCodePosition;
};

#endif // !defined(AFX_ICODESTACKITEM_H__12901454_A24A_11D3_AF86_004F49068BD6__INCLUDED_)
