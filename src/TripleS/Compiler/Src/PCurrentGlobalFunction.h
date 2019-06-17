/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Prekladac
 *   Autor: Helena Kupková
 *  
 *   Popis: Implementace tridy CCurentGlobalFunction reprezentujici 
 *          globalni funkci, ktera je prave prekladana (nebo je uz prelozena).
 * 
 ***********************************************************/

#if !defined(AFX_CURRENTGLOBALFUNCTION_H__A5EE9B43_4B48_11D3_A4E0_00A0C970CB8E__INCLUDED_)
#define AFX_CURRENTGLOBALFUNCTION_H__A5EE9B43_4B48_11D3_A4E0_00A0C970CB8E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// class CPCurrentGlobalFunction
// ============================
// Trida uchovavajici informace o prave prekladane globalni funkci.
class CPCurrentGlobalFunction : public CPCurrentUnit::CMethod
{
public:
	CPCurrentGlobalFunction(CString strName, CType &ReturnDataType);
	virtual ~CPCurrentGlobalFunction();

public:
    // jmeno funkce (mame sice m_stiName od predka, ale to nechceme)
	CString m_strName;

    // kod funkce
	CPCode m_Code;

    // tabulka stringu, ktera se uklada s kodem
	CPCodeStringTable m_StringTable;
    // tabulka realnych cisel
	CFloatTable m_FloatTable;

    // priznak, zda byla funkce zkompilovana bez chyb
	bool m_bOK;

    // ukazatel na dalsi zkompilovanou globalni funkci ve spojaku kompileru
    CPCurrentGlobalFunction *m_pNext;

public:
    // ulozeni funkce na vystup
	void Save(CCompilerOutput *pOut);

	// debugovaci vypis obsahu
	void DebugWrite(CCompilerErrorOutput *pOut);
};

#endif // !defined(AFX_CURRENTGLOBALFUNCTION_H__A5EE9B43_4B48_11D3_A4E0_00A0C970CB8E__INCLUDED_)
