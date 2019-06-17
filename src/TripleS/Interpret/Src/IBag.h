/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Interpret
 *   Autor: Helena Kupková
 *  
 *   Popis: Implementace objektu CIBag pro predavani argumentu
 *          metoda, funkcim a reakcim na udalosti.
 * 
 ***********************************************************/

#if !defined(AFX_IBAG_H__12901455_A24A_11D3_AF86_004F49068BD6__INCLUDED_)
#define AFX_IBAG_H__12901455_A24A_11D3_AF86_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CISOString;
class CISOArrayParent;
class CISOSetParent;
class CISystemObject;
class CIStructure;
class CIUnitType;
class CIDataStackItem;
class CZUnit;

class CIBag  
{
    DECLARE_MULTITHREAD_POOL( CIBag);

public:
	CIBag();
	virtual ~CIBag();

public:
    void Create();
    inline void Create( CIBag &anotherOne) { operator=(anotherOne); }
    void Delete();

    operator=(CIBag &anotherOne);

    // pridani argumentu
    void AddConstantChar(char cChar);
    void AddConstantInt(int nInt);
    void AddConstantFloat(double fFloat);
    void AddConstantBool(bool bBool);
    void AddUnit( CZUnit *pUnit, CIUnitType *pIUnitType);
    void AddSystemObject( CISystemObject *pSystemObject);
    void AddStructure( CIStructure *pStructure);
    void AddRegister(CIDataStackItem &DSI);
	void AddNull();

    // ziskani poctu argumentu
    inline int GetArgCount() { return m_nArgCount; }

    // ziskani argumentu  ( char a bool se uchovavaji v int)
    int* GetChar(int nIndex);
    int* GetInt(int nIndex);
    int* GetBool(int nIndex);
    double* GetFloat(int nIndex);
    CISystemObject* GetSystemObject(int nIndex);
    CIStructure* GetStructure(int nIndex);
    CZUnit* GetUnit(int nIndex);

    CISOString* GetString(int nIndex);
    CISOArrayParent* GetArray(int nIndex);
	CISOSetParent* GetSet(int nIndex);

    CIDataStackItem* GetArg(int nIndex);


    // navratovy registr
    inline void SetReturnRegister(CIDataStackItem *pReturnRegister)
        {
            ASSERT(m_pReturnRegister == NULL);
            m_pReturnRegister = pReturnRegister;
        }

    CIDataStackItem* GetReturnRegister();

    // navratova hodnota - zjednodusene pridavani
    void ReturnChar(char cChar);    
    void ReturnInt(int nInt);       
    void ReturnFloat(double fFloat);
    void ReturnBool(bool bBool);  
    void ReturnSystemObject( CISystemObject *pSystemObject);
    void ReturnUnit( CZUnit *pUnit, CIUnitType *pIUnitType);

public:
    // pocet argumentu
    int m_nArgCount;
    // argumenty
    CArray<CIDataStackItem, CIDataStackItem&> m_aArgs;

    // navratovy registr
    CIDataStackItem *m_pReturnRegister;

    enum { GROW_BY = 5 };

    // navazani rozdelanych bagu
    CIBag *m_pPreviousBag;

public:
//
//	SAVE & LOAD
//
	void PersistentSave( CPersistentStorage &storage);
	void PersistentLoad( CPersistentStorage &storage);
	void PersistentTranslatePointers( CPersistentStorage &storage);
	void PersistentInit();
	
};

#endif // !defined(AFX_IBAG_H__12901455_A24A_11D3_AF86_004F49068BD6__INCLUDED_)
