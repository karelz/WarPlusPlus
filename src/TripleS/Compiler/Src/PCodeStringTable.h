/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Prekladac
 *   Autor: Helena Kupková
 *  
 *   Popis: Implementace trid CPCodeStringTable a CPCodeStringTableItem,
 *          tabulky stringu kodu, ktere se budou ukladat na disk.
 * 
 ***********************************************************/

#if !defined(AFX_CODESTRINGTABLE_H__247E9183_32C6_11D3_AF0C_004F49068BD6__INCLUDED_)
#define AFX_CODESTRINGTABLE_H__247E9183_32C6_11D3_AF0C_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// class CPCodeStringTableItem 
// ======================
// Reprezentuje jednu polozku v tabulce stringu (CPCodeStringTable)
class CPCodeStringTableItem 
{
public:
    // retezec
    CString m_strString;
    // identifikace
    long m_nID;

    // operatory
    operator LPCTSTR () const;
    operator CString () const;
};

// class CPCodeStringTable
// ==================
// Trida pro reprezentaci tabulky stringu. Retezce se do tabulky hasuji 
// pomoci MFC tridy CTypedPtrMap. Dale se take v poli udrzuji ukazatele na 
// vsechny polozky (setridene dle ID), to kvuli ukladani na disk, aby se
// tabulka ukladala setridena (a pak se jednoduse nacitala).
// Identifikatory stringu se prirazuji vzestupne souvisle od 0 dle poradi, v jakem 
// se do tabulky pridava.
class CPCodeStringTable : public CObject
{
public:
	CPCodeStringTable();
	virtual ~CPCodeStringTable();

    // Pridavani do tabulky - CString, LPCTSTR.
	CPCodeStringTableItem* Add(CString &strString);
	CPCodeStringTableItem* Add(LPCTSTR string);

    // Ziskani polozky dle ID.
	CPCodeStringTableItem* GetItemByID(long nID) { if (nID<m_nFirstFreeID) return m_apCSTIArray[nID]; 
											      else return NULL; }

    // Ulozeni na vystup.
	void Save(CCompilerOutput *pOut);
    // Nacteni ze vstupu.
	void Load(CFile &fileIn);

    // Odstraneni vsech polozek z tabulky.
	void RemoveAll();

    // Debugovaci vypis obsahu tabulky stringu.
	void DebugWrite(CCompilerErrorOutput* pOut);

protected:
    // velikost hashovaci tabulky
    enum { HASHTABLE_SIZE = 2047 };
    
    // prvni volne ID
    long m_nFirstFreeID;

    // hasovaci tabulka
	CTypedPtrMap<CMapStringToPtr, CString, CPCodeStringTableItem*> m_apCSTITable;

    // Pole udrzujici odkazy na polozky v poradi, ve kterem byly 
    // pridavany => setridene dle ID.
	CTypedPtrArray<CPtrArray, CPCodeStringTableItem*> m_apCSTIArray;
};

#endif // !defined(AFX_CODESTRINGTABLE_H__247E9183_32C6_11D3_AF0C_004F49068BD6__INCLUDED_)
