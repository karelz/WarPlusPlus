/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Interpret
 *   Autor: Helena Kupková
 *  
 *   Popis: Implementace trid CIDataStack, CIDataStackItem a CIDataStackItemSet.
 *          CIDataStack - typovany datovy zasobnik procesu
 *          CIDataStackItem - jedna bunka datoveho zasobniku
 *          CIDataStackItemSet - DATA_STACK_ITEM_SET_SIZE bunek CIDataStackItem
 *                          zabalenych do jedne tridy (tzv. setu). Datovy zasobnik 
 *                          roste po setech.
 *          
 ***********************************************************/

#if !defined(AFX_IDATASTACK_H__12901453_A24A_11D3_AF86_004F49068BD6__INCLUDED_)
#define AFX_IDATASTACK_H__12901453_A24A_11D3_AF86_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define DATA_STACK_ITEM_SET_BITS  4
#define DATA_STACK_ITEM_SET_SIZE  (1<<DATA_STACK_ITEM_SET_BITS)

class CZUnit;
class CIStructure;
class CIObjUnitStruct;
class CISystemObject;

class CIDataStack;
class CIDataStackItemSet;

// class CIDataStackItem
// =====================
// Trida reprezentujici jednu bunku datoveho zasobniku, 
// obsahuje datovy typ bunky a jeji obsah (dle datoveho typu).
class CIDataStackItem
{
	friend CIBag;
	friend CIDataStackItemSet;

public:
    CIDataStackItem();
    ~CIDataStackItem();

    // vycisteni
    void Clean();

	// kopirovani
	void operator=(CIDataStackItem &anotherType);

protected:
// DATOVY TYP
	// datovy typ - enum 
	EDataType m_nDataType;

// HODNOTA
    union
    {
        // char, int, bool
        int m_nInt;
        // float
        double m_fFloat;

        // unit
        struct
        {
            CZUnit *m_pUnit;
            CIUnitType *m_pIUnitType;   // kvuli dedicnosti, tohle je aktualni typ jednotky
        };

        // object
        CISystemObject *m_pSystemObject;    // typ je v polozce m_pSOType

        // structure
        CIStructure *m_pStruct;             // typ je v polozce m_pStructType
    };

public:	
//  SETTING
    EInterpretStatus SetTypeAndCreate(CIType &IType, CInterpret *pInterpret);

// LOAD
    void Load(CIType &IType, int *pData);

// STORE
    void Store(int *pData);

// CONSTANT SETTING
	inline void Set(int nInt)	    { Clean(); m_nDataType = T_INT; m_nInt = nInt; }
	inline void Set(double fFloat) { Clean(); m_nDataType = T_FLOAT; m_fFloat = fFloat; }
	inline void Set(char cChar)	{ Clean(); m_nDataType = T_CHAR;  m_nInt = cChar; }
	inline void Set(bool bBool)	{ Clean(); m_nDataType = T_BOOL;  m_nInt = (int)bBool; }
    void Set(CISystemObject *pSystemObject);
    void Set(CZUnit *pUnit);
    void Set(CIStructure *pStruct);

// UNIT SETTING - kdyz je typ jednotky jiny nez pUnit->m_pIUnitType (napr. je pretypovana na predka)
    void Set( CZUnit *pUnit, CIUnitType *pIUnitType);

// OBJECT SETTING

// TYPE CHECKING
//    bool SameTypeAs( CIDataStackItem &anotherOne);
    bool SameTypeAs( CIType &Type);
    bool SameTypeAsAndConvertUnits( CIType &Type);

// QUERYING
	inline bool IsErrorType()	{ return m_nDataType==T_ERROR; }
    inline bool IsVoidType()	{ return m_nDataType==T_VOID; }
	inline bool IsInt()			{ return m_nDataType==T_INT;	}
	inline bool IsFloat()		{ return m_nDataType==T_FLOAT;	}
	inline bool IsChar()		{ return m_nDataType==T_CHAR;	}
	inline bool IsBool()		{ return m_nDataType==T_BOOL;	}
    inline bool IsSystemObject(){ return m_nDataType==T_OBJECT; }  
    inline bool IsUnit()	    { return m_nDataType==T_UNIT; }                      
    inline bool IsStruct()	    { return m_nDataType==T_STRUCT; }
    
    bool IsString();
    bool IsArray(); 
    bool IsSet();   

    inline bool IsObjectUnitStruct()   { return IsSystemObject() || IsUnit() || IsStruct(); }

    inline bool IsSimpleType() { return m_nDataType==T_CHAR || m_nDataType==T_INT || 
                                        m_nDataType==T_FLOAT || m_nDataType==T_BOOL; }

// GETTING VALUES
    inline void            Get( char *pC)            { ASSERT(m_nDataType == T_CHAR);	ASSERT( pC != NULL); *pC = (char)m_nInt;	}
	inline void			   Get( int  *pI)		     { ASSERT(m_nDataType == T_INT);	ASSERT( pI != NULL); *pI = (int)m_nInt;	}
	inline void 		   Get( bool *pB)	         { ASSERT(m_nDataType == T_BOOL);	ASSERT( pB != NULL); *pB = m_nInt != 0;	}
	inline void     	   Get( double *pF)	         { ASSERT(m_nDataType == T_FLOAT);	ASSERT( pF != NULL); *pF = m_fFloat;		}
	inline void            Get( CISystemObject **pO) { ASSERT(m_nDataType == T_OBJECT);	ASSERT( pO != NULL); *pO = m_pSystemObject;}
	inline void		       Get( CZUnit **pU)		 { ASSERT(m_nDataType == T_UNIT);	ASSERT( pU != NULL); *pU = m_pUnit;		}
    inline void	           Get( CIStructure **pS)    { ASSERT(m_nDataType == T_STRUCT); ASSERT( pS != NULL); *pS = m_pStruct;	}

	inline char			   GetChar()		{ ASSERT(m_nDataType == T_CHAR);	return (char)m_nInt;	}
	inline int			   GetInt()			{ ASSERT(m_nDataType == T_INT);		return (int)m_nInt;		}
	inline bool			   GetBool()		{ ASSERT(m_nDataType == T_BOOL);	return m_nInt != 0;		}
	inline double		   GetFloat()		{ ASSERT(m_nDataType == T_FLOAT);	return m_fFloat;		}
	inline CISystemObject* GetSystemObject(){ ASSERT(m_nDataType == T_OBJECT);	return m_pSystemObject;	}
	inline CZUnit*		   GetUnit()		{ ASSERT(m_nDataType == T_UNIT);	return m_pUnit;			}
	inline CIStructure*	   GetStruct()		{ ASSERT(m_nDataType == T_STRUCT);  return m_pStruct;		}

	inline CIUnitType*	   GetUnitType()	{ ASSERT(m_nDataType == T_UNIT);	return m_pIUnitType;		}

// GETTING UNIT TYPE
	inline EDataType GetDataType()	{ return m_nDataType; }

// ONE-REGISTER ARITHMETIC OPERATIONS
	inline void IncreaseChar()	{	ASSERT(m_nDataType == T_CHAR);  m_nInt = (char)m_nInt +1; }
	inline void IncreaseInt()	{	ASSERT(m_nDataType == T_INT);   m_nInt++;				 }
	inline void IncreaseFloat()	{	ASSERT(m_nDataType == T_FLOAT); m_fFloat++;				 }
	inline void DecreaseChar()	{	ASSERT(m_nDataType == T_CHAR);  m_nInt = (char)m_nInt -1; }
	inline void DecreaseInt()	{	ASSERT(m_nDataType == T_INT);   m_nInt--;				 }
	inline void DecreaseFloat()	{	ASSERT(m_nDataType == T_FLOAT); m_fFloat--;				 }
	inline void InvertInt()		{	ASSERT(m_nDataType == T_INT);   m_nInt = -m_nInt;		 }
	inline void InvertFloat()	{	ASSERT(m_nDataType == T_FLOAT); m_fFloat = -m_fFloat;	 }
	inline void NegateBool()	{	ASSERT(m_nDataType == T_BOOL);  m_nInt = !m_nInt;		 }

public:
//
//	SAVE & LOAD
//
	void PersistentSave( CPersistentStorage &storage);
	void PersistentLoad( CPersistentStorage &storage);
	void PersistentTranslatePointers( CPersistentStorage &storage);
	void PersistentInit();

//	void GetSystemObjectsAndStructures( CTypedPtrMap<CMapPtrToPtr, CISystemObject*, CISystemObject*> &tpSystemObjects,
//										CTypedPtrMap<CMapPtrToPtr, CIStructure*, CIStructure*> &tpStructures);
};

// class CIDataStackItemSet
// ========================
// Trida reprezentujici DATA_STACK_ITEM_SET_SIZE bunek datoveho zasobniku,
// datovy zasobnik se sklada z techto "setu" (tj. kdyz se zvetsuje, zvetsuje
// se po DATA_STACK_ITEM_SET_SIZE bunkach.
class CIDataStackItemSet
{
public:
    CIDataStackItemSet() {}
    ~CIDataStackItemSet() { Clean(); }

    DECLARE_MULTITHREAD_POOL( CIDataStackItemSet);

public:
    // vycisteni (uvolneni objektu drzenych v bunkach m_pItems)
    void Clean();

public:
    // pole bunek
    CIDataStackItem m_pItems[DATA_STACK_ITEM_SET_SIZE];


public:
//
// SAVE & LOAD
//
	void PersistentSave( CPersistentStorage &storage);
	void PersistentLoad( CPersistentStorage &storage);
	void PersistentTranslatePointers( CPersistentStorage &storage);
	void PersistentInit();
};

// class CIDataStack  
// =================
// Trida reprezentujici datovy zasobnik procesu. Roste
// DATA_STACK_ITEM_SET_SIZE polozkach, tyto kusy jsou obalene
// tridou CIDataStackItemSet.
class CIDataStack  
{
public:
	CIDataStack();
	virtual ~CIDataStack();

    // inicializace - pripoji si jeden set
    void Create(CInterpret *pInterpret);

    // uvolneni - odpoji vsechny sety
    void Delete();

public:
    CIDataStackItem& operator[] (int nIndex);

// PODPORA PRO VOLANI FUNKCI
    // nevim, od ktereho registru mam loadovat dalsi funkci,
    // nastavovani nejvyssiho pouziteho pri kazdem pristupu by bylo
    // pomale => naloaduju to do noveho DataStack Setu.
    // Tato metoda vraci prvni registr, pro ktery neni DataStack Set,
    // tj urcite jeste nebyl pouzit.
    REG GetFirstSurelyUnusedRegister() { return m_nDSItemSetCount<<DATA_STACK_ITEM_SET_BITS; }
    // Tato metoda uvolnuje DataStack Sety od zadaneho registru, pouziva se
    // po navratu z volani funkce a parametrem je registr
    // vraceni predchozim volanim GetFirstSurelyUnusedRegister()
    void FreeSetsFromRegister(REG nReg);

protected:
    // prislusny interpret
    CInterpret *m_pInterpret;

    // pole CIDataStackItem
    CTypedPtrArray<CPtrArray, CIDataStackItemSet*> m_apDSItemSets;
    // pocet setu
    int m_nDSItemSetCount;

    // priznak, zda byl vytvoren
    bool m_bCreated;

public:
//
// SAVE & LOAD
//
	void PersistentSave( CPersistentStorage &storage);
	void PersistentLoad( CPersistentStorage &storage);
	void PersistentTranslatePointers( CPersistentStorage &storage);
	void PersistentInit();
};

#endif // !defined(AFX_IDATASTACK_H__12901453_A24A_11D3_AF86_004F49068BD6__INCLUDED_)
