/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Interpret
 *   Autor: Helena Kupková
 *  
 *   Popis: Implementace tridy CIType uchovavajici informace o datovem typu
 *          nejake polozky, argumentu atp.          
 *          
 ***********************************************************/

#if !defined(AFX_ITYPE_H__574E728E_986E_11D3_AF75_004F49068BD6__INCLUDED_)
#define AFX_ITYPE_H__574E728E_986E_11D3_AF75_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#include "ICodeStringTable.h"

class CFile;

class CIType  :  public CType
{
public:
	CIType();
//	CIType(CIType& anotherType);
	virtual ~CIType();

    // Nastavi vsechny polozky az na m_stiName, kterou nastavi na NULL, a ma-li
    // v ni neco byt, dostane se to tam v metode Translate, ktera musi
    // byt zavolana po loadu stringtable daneho skriptu, aby se prelozily 
    // ID stringu na pointery do globalni stringtable.
    // throws CFileException
    void Load(CFile &file);
    // preklada z ID stringu dle stringtable kodu na pointer do globalni stringtable
    // - musi byt zavolana prave jednou
    void Translate(CICodeStringTable *CST);

    // Cleaning
    void SetNoneType();

    virtual void Set(CString &strType);

protected:
    // ID nazvu typu ve stringtable, ktera prisla s kodem
    // (po nacteni cele binarky se vola fce Traslate pro preklad do glob. stringtable).
    // !!! Vztahuje se k poslednimu typu v retezci (retezec je napojen skrz m_pOfType).
    long m_nCodeStringTableID;

public:
    // priznak, zda byl objekt spravne natazen
    bool m_bLoaded;

#ifdef _DEBUG
public:
	virtual void AssertValid() const;
//	virtual void Dump(CDumpContext& dc) const;
#endif
};

#endif // !defined(AFX_ITYPE_H__574E728E_986E_11D3_AF75_004F49068BD6__INCLUDED_)
