/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Interpret
 *   Autor: Helena Kupková
 *  
 *   Popis: Implementace tridy CICodeStringTable, ktera reprezentuje
 *          tabulku stringu urciteho kodu (inicializuje se z tabulky
 *          stringu, ktera se uklada s kodem). Je to vlastne jen prevodni
 *          tabulka, prevadi ID stringu pouziteho v kodu na polozce globalni 
 *          tabulky stringu.
 * 
 ***********************************************************/

#if !defined(AFX_ICODESTRINGTABLE_H__574E728D_986E_11D3_AF75_004F49068BD6__INCLUDED_)
#define AFX_ICODESTRINGTABLE_H__574E728D_986E_11D3_AF75_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CFile;

class CICodeStringTable  
{
public:
	CICodeStringTable();
	virtual ~CICodeStringTable();

    // Naloadeni strigtable ze souboru, prelozeni do globalni stringtable
    // throws CFileException, CStringException
    void Load(CFile &file);  

    // porovnani dvou stringtable
    bool operator==(CICodeStringTable &anotherOne);

    // Vyprazdneni tabulky
    void Clean();

    // vytvoreni tabulky dle jine (kopie)
    void UpdateFrom( CICodeStringTable *pAnotherOne);

    // pole odkazu do globalni stringtable, index v poli je ID stringu v kodu
    CTypedPtrArray<CPtrArray, CStringTableItem*> m_apCodeStringTable;
    // pocet platnych polozek v tabulce
    long m_nStringTableCount;

    CStringTableItem* operator[] (long nIndex);
};

#endif // !defined(AFX_ICODESTRINGTABLE_H__574E728D_986E_11D3_AF75_004F49068BD6__INCLUDED_)
