/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Interpret
 *   Autor: Helena Kupková
 *  
 *   Popis: Implementace tridy CIStructureType uchovavajici
 *          informace o nejake strukture (o typu jejich polozek..).
 *          
 ***********************************************************/

#if !defined(AFX_ISTRUCTURETYPE_H__574E728B_986E_11D3_AF75_004F49068BD6__INCLUDED_)
#define AFX_ISTRUCTURETYPE_H__574E728B_986E_11D3_AF75_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CIStructureType  
{
public:
	CIStructureType();
	virtual ~CIStructureType();

    // natazeni struktury z disku
    // throws CFileException, CStringException
    void Load(CFile &file);

    // Cleaning
    void Clean();

    EImplementation GetMember(CStringTableItem *stiName, CIMember* &pMember);

    // update struktury, vycucne pFrom typ, v pFrom zbyde prazdna struktura.
    void UpdateStructFrom(CIStructureType *pFrom);

public:
    // jmeno struktury
    CStringTableItem *m_stiName;
    
    // polozky
    CTypedPtrMap<CMapPtrToPtr, CStringTableItem*, CIMember*> m_tpMembers;
    // pocet polozek
    short m_nMemberCount;
    // heap size  - velikost pameti potrebna pro ulozeni polozek (offsety jsou
    // u kazde polozky
    long m_nHeapSize;

    // code stringtable
    CICodeStringTable *m_pCodeStringTable;

public:
    // priznak, zda byl objekt spravne natazen
    bool m_bLoaded;

#ifdef _DEBUG
public:
	virtual void AssertValid() const;
//	virtual void Dump(CDumpContext& dc) const;
#endif

};

#endif // !defined(AFX_ISTRUCTURETYPE_H__574E728B_986E_11D3_AF75_004F49068BD6__INCLUDED_)
