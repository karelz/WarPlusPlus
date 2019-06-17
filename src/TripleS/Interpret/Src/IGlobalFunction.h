// IGlobalFunction.h: interface for the CIGlobalFunction class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IGLOBALFUNCTION_H__574E7288_986E_11D3_AF75_004F49068BD6__INCLUDED_)
#define AFX_IGLOBALFUNCTION_H__574E7288_986E_11D3_AF75_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IMethod.h"

class CIGlobalFunction : public CIMethod 
{
public:
	CIGlobalFunction();
	virtual ~CIGlobalFunction();

    // natazeni globalni funkce z disku
    // throws CFileException, CStringException
    virtual void Load(CFile &file);

    // Cleaning
    virtual void Clean();

    // Translation of types (code string table -> global string table)
    // !! tato funkce by nemela byt u glob.fce volana 
    virtual void Translate(CICodeStringTable*) { ASSERT(false); }

public:
    // tabulky
    CICodeStringTable *m_pCodeStringTable;
    CFloatTable *m_pFloatTable;

public:
    // priznak, zda byl objekt spravne natazen
    bool m_bLoaded;

#ifdef _DEBUG
public:
	virtual void AssertValid() const;
//	virtual void Dump(CDumpContext& dc) const;
#endif

public:
};

#endif // !defined(AFX_IGLOBALFUNCTION_H__574E7288_986E_11D3_AF75_004F49068BD6__INCLUDED_)
