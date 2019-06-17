/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Prekladac
 *   Autor: Helena Kupková
 *  
 *   Popis: Implementace tridy CPCurrentStruct reprezentujici 
 *          strukturu (struct), ktera je prave prekladana (nebo je uz prelozena).
 * 
 ***********************************************************/

#if !defined(AFX_CURRENTSTRUCT_H__473ECAC3_912A_11D3_AF65_004F49068BD6__INCLUDED_)
#define AFX_CURRENTSTRUCT_H__473ECAC3_912A_11D3_AF65_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CPCurrentStruct  
{
public:
	CPCurrentStruct(CString strName);
	virtual ~CPCurrentStruct();

public:
    // jmeno
    CString m_strName;

    // polozky
	CArray<CType, CType&> m_aMemberTypes;
    CTypedPtrArray<CPtrArray, CPCodeStringTableItem*> m_aMemberNames;
    int m_nMemberCount;

    // priznak, zda je definice v poradku
    bool m_bOK;

    // stringtable
    CPCodeStringTable m_StringTable;

    // ukazatel na dalsi zkompilovanou strukturu ve spojaku kompileru
    CPCurrentStruct *m_pNext;

public:
    // pridani polozky 
    void AddMember(CString &strName, CType&DataType);

    // dotaz na polozku
    bool HasMember(CString strName);

    // ulozeni na disk
    void Save(CCompilerOutput *pOut);

// debugovaci vypis
    void DebugWrite(CCompilerErrorOutput *pOut);
};

#endif // !defined(AFX_CURRENTSTRUCT_H__473ECAC3_912A_11D3_AF65_004F49068BD6__INCLUDED_)
