/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Prekladac
 *   Autor: Helena Kupková
 *  
 *   Popis: Implementace trid CPPreviewMethod, CPPreviewUnit a CPPreview,
 *          uchovavaji informace ziskane behem prvniho pruchodu prekladace
 *          (nazvy jednotek a jejich metod, hlavicky metod)
 * 
 ***********************************************************/

#if !defined(AFX_PREVIEW_H__247E9187_32C6_11D3_AF0C_004F49068BD6__INCLUDED_)
#define AFX_PREVIEW_H__247E9187_32C6_11D3_AF0C_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// class CPPreview
// ==============
// Do tridy CPPreview se ukladaji informace ziskane pri prvnim pruchodu zdrojovym
// kodem, tedy jake obsahuje globalni funkce a jednotky.
class CPPreview : public CObject
{
public:
	CPPreview(CPNamespace *pNamespace);
	virtual ~CPPreview();

    // prislusny prekladac
    CPNamespace *m_pNamespace;

    // Smazani vsech informaci, ktere trida obsahuje.
    void RemoveAll();

    // Pridani jednotky.
    CPNamespace::CUnit* AddUnit(CString &strName);
    // Ziskani preview jednotky dle jmena.
	CPNamespace::CUnit* GetUnit(CString &strName);

    // Pridani globalni funkce.
	CPNamespace::CGlobalFunction* AddGlobalFunction(CString &strName, CString& strReturnType, int nArrayLevel);
    // Ziskani preview globalni funkce dle jmena.
	CPNamespace::CGlobalFunction* GetGlobalFunction(CString &strName);

    // Pridani struktury.
    CPNamespace::CStruct* AddStruct(CString &strName);
    // Ziskani preview struktury dle jmena
    CPNamespace::CStruct* GetStruct(CString &strName);

    // Nalezene jednotky - ulozene v hasovaci tabulce (CTypedPtrMap).
    CTypedPtrMap<CMapStringToPtr, CString, CPNamespace::CUnit*> m_apUnits;
    // Nalezene globalni funkce - ulozene v hasovaci tabulce (CTypedPtrMap).
    CTypedPtrMap<CMapStringToPtr, CString, CPNamespace::CGlobalFunction*> m_apGlobalFunctions;
    // Nalezene struktury
    CTypedPtrMap<CMapStringToPtr, CString, CPNamespace::CStruct*> m_apStructs;

    // velikosti hash.tabulek
	enum { UNITS_HASHTALE_SIZE=19, GLOBALFUNCT_HASHTABLE_SIZE=17 };

// DEBUG
    // Debugovaci vypis 
	void DebugWrite(CCompilerErrorOutput *pOut);
};

#endif // !defined(AFX_PREVIEW_H__247E9187_32C6_11D3_AF0C_004F49068BD6__INCLUDED_)
