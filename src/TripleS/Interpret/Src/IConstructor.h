/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Interpret
 *   Autor: Helena Kupková
 *  
 *   Popis: Implementace tridy CIConstructor, ktera reprezentuje
 *          konstruktor (i destruktor) nejake jednotky.
 *          
 ***********************************************************/

#if !defined(AFX_ICONSTRUCTOR_H__BB5D6165_9DA4_11D3_AF7F_004F49068BD6__INCLUDED_)
#define AFX_ICONSTRUCTOR_H__BB5D6165_9DA4_11D3_AF7F_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CIUnitType;
class CIConstructor;

typedef CIConstructor CICodeBlock;

class CIConstructor  
{
public:
	CIConstructor();
	virtual ~CIConstructor();

    // natazeni konstruktoru z disku
    // throws CFileException, CStringException
    virtual void Load(CFile &file, CIUnitType *pIUnitType);

    // Cleaning
    virtual void Clean();

    // Translation of types (code string table -> global string table)
    virtual void Translate(CICodeStringTable *pCodeStringTable);

    // porovnavani dvou konstruktoru
    bool operator==(CIConstructor &anotherOne);


public:
    // pocet argumentu + lokalnich promennych
    // (konstruktor sice argumenty nema, ale potomci CIEvent, CIMethod a 
    // CIGlobalFunction ano)
    short m_nArgAndLocalVarCount;
    // typy argumentu a lokalnich promennych (naalokovane pole delky m_nArgAndLocalVarCount);
    CIType *m_apArgAndLocalVars;
    // typ jednotky, ktere je to konstruktor/eventa/metoda
    CIUnitType *m_pIUnitType;

    // kod
    CICode m_Code;

public:
    // priznak, zda byl konstruktor/eventa/metoda/funkce spravne natazen
    bool m_bLoaded;

protected:
	// profiling counters
	ULONG m_nGlobalProfileCounter;
	ULONG m_nLocalProfileCounter;
	ULONG m_nLastLocalProfileCounter;

public:
	inline void IncrementProfileCounter()
	{
		::InterlockedIncrement( (LONG*)&m_nLocalProfileCounter);
	}

	inline ULONG GetLocalProfileCounter()	{ return m_nLastLocalProfileCounter;  }
    inline ULONG GetGlobalProfileCounter()  { return m_nGlobalProfileCounter; }

	inline void ClearLocalProfileCounter()
	{
        m_nGlobalProfileCounter += m_nLocalProfileCounter;
		m_nLastLocalProfileCounter = m_nLocalProfileCounter;
		m_nLocalProfileCounter = 0;
	}

	inline void ClearGlobalAndLocalProfileCounters()	{ m_nGlobalProfileCounter = 0; 
                                                          m_nLastLocalProfileCounter = 0;
		                                                  m_nLocalProfileCounter = 0; }

#ifdef _DEBUG
public:
	virtual void AssertValid() const;
//	virtual void Dump(CDumpContext& dc) const;
#endif

};

typedef CIConstructor CIDestructor;

#endif // !defined(AFX_ICONSTRUCTOR_H__BB5D6165_9DA4_11D3_AF7F_004F49068BD6__INCLUDED_)
