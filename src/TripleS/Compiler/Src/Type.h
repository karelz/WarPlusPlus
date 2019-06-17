/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Prekladac
 *   Autor: Helena Kupková
 *  
 *   Popis: CType - trida reprezentujici typ jazyka
 * 
 ***********************************************************/

#if !defined(AFX_TYPE_H__247E9186_32C6_11D3_AF0C_004F49068BD6__INCLUDED_)
#define AFX_TYPE_H__247E9186_32C6_11D3_AF0C_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CPLexVal;
class CCompilerOutput;
class CPNamespace;
class CPPreview;

#include "Common\PersistentStorage\PersistentObject.h"

// Trida CType
// ===========
// Tato trida se pouziva pro ulozeni datoveho typu skriptovaciho jazyka.
// Vsechny stringy se udrzuji v globalni stringtable.
// Dedi z CPersistentObject, protoze se obcas i uklada do SaveGame (napr. jako datovy typ polozek v poli nebo mnozine).
class CType : public CPersistentObject
{
public:
	CType();
	CType(CType& anotherType);
	virtual ~CType();

    // "typ" typu dane instance
    EDataType m_nType;                

    // objekt, jednotka nebo struktura - nazev typu
    CStringTableItem *m_stiName;  // valid for STRUCT and UNIT

    // objekt
    // typ systemoveho objektu
    ESystemObjectType m_nSOType;
    // podtyp - pro pole a mnozinu
    CType *m_pOfType;     // valid for ARRAY and SET
    // velikost pole (platne jen kdyz >0, napr. v parametrech funkce se velikost 
    // pole neudava
	int m_nArraySize;			  // valid for ARRAY if >0  

    // operatory
    operator==(CType& anotherType);
    operator=(CType& anotherType);

    // Nastaveni typu - dle stringu, nove identifikatory se pridavaji do globalni stringtable
    virtual void Set(CString &strType, int nArrayLevel, CPNamespace *pNamespace, CPPreview *pPreview);
    // Nastaveni typu - dle konstantny (lexikalni hodnoty).
	void Set(CPLexVal &pLexVal);
    // Nastaveni primitivniho typu primo.
	void Set(EDataType nType);

    // Vraci retezec popisujici typ (napr "int[]" pro pole typu int atd.).
    CString GetString();

    // Od aktualni instance se ulozi typ, ktery byl az dosud podtypem
    // (napr. pro pole 'int p[]' -> 'int').
	void SetOfType();

    // Vraci ukazatel na zakladni typ (tedy pro pole int[][] vraci int).
    // Pro typy bez podtypu vraci ukazatel na sama sebe.
	virtual CType* GetBasicType();

    // Nastaveni typu.
	void SetError();	   
    void SetNone();     
    void SetVoid();        
    inline void SetChar()   { Set(T_CHAR); }
    inline void SetInt()    { Set(T_INT); }
    inline void SetFloat()  { Set(T_FLOAT); }
    inline void SetBool()   { Set(T_BOOL); }
    
    void SetObject(ESystemObjectType nSOType);
    void SetUnit(CStringTableItem *stiName);
    void SetStruct(CStringTableItem *stiName);

	// zvlastni pripady SetObject pro string a mnozinu
    inline void SetString()         { SetObject(SO_STRING); }

    // Dotazani na typ.
	inline bool IsError()		{ return m_nType==T_ERROR; }
    inline bool IsVoid()		{ return m_nType==T_VOID; }
    inline bool IsSimpleType()	{ return m_nType==T_CHAR || m_nType==T_INT || m_nType==T_FLOAT || 
                                 m_nType==T_BOOL; }
	inline bool IsInt()			{ return m_nType==T_INT;	}
	inline bool IsFloat()		{ return m_nType==T_FLOAT;	}
	inline bool IsChar()		{ return m_nType==T_CHAR;	}
	inline bool IsBool()		{ return m_nType==T_BOOL;	}
    inline bool IsObject()		{ return m_nType==T_OBJECT; }  
    inline bool IsUnit()		{ return m_nType==T_UNIT; }    
    inline bool IsStruct()		{ return m_nType==T_STRUCT; }

    inline bool IsArray()		{ return m_nType==T_OBJECT && m_nSOType==SO_ARRAY;  }   
    inline bool IsString()		{ return m_nType==T_OBJECT && m_nSOType==SO_STRING; }
    inline bool IsSet()			{ return m_nType==T_OBJECT && m_nSOType==SO_SET; } 

    inline bool IsObjectUnitStruct()   { return m_nType==T_OBJECT || m_nType==T_UNIT || m_nType==T_STRUCT; }

    // Z aktualniho typu vytvori pole (tj. int -> int[], int[] -> int[][] atd.)
	virtual void MakeArray(int nArraySize=0);

    // Z aktualniho typu vytvori mnozinu (tj. int -> int{}, jen do hloubky jedna)
	virtual void MakeSet();

	// Ulozeni typu na vystup. Pro ukladani typu argumentu se neukladaji
    // delky poli, pro lokalni promenne a membery ano. Nazvy typu se (krome jednoduchych
    // typu) ukladaji do zadane stringtable (puvodne jsou v globalni)).
	void Save(CCompilerOutput *pOut, CPCodeStringTable &StrTbl, bool bSaveArrayLengths);
    
    // Nacteni typu ze souboru. 
//	void Load(CFile &file);

protected:
    void SetStiName(CStringTableItem* pSTI);

public:
//
// SAVE & LOAD
//
	void PersistentSave( CPersistentStorage &storage);
	void PersistentLoad( CPersistentStorage &storage);
	void PersistentTranslatePointers( CPersistentStorage &storage);
	void PersistentInit();
};

#endif // !defined(AFX_TYPE_H__247E9186_32C6_11D3_AF0C_004F49068BD6__INCLUDED_)
