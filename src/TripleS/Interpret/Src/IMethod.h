/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Interpret
 *   Autor: Helena Kupková
 *  
 *   Popis: Implementace tridy CIMethod reprezentujici metodu
 *          nejake jednotky.
 *          
 ***********************************************************/

#if !defined(AFX_IMETHOD_H__BB5D6164_9DA4_11D3_AF7F_004F49068BD6__INCLUDED_)
#define AFX_IMETHOD_H__BB5D6164_9DA4_11D3_AF7F_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IEvent.h"

class CIMethod  : public CIEvent
{
public:
	CIMethod();
	virtual ~CIMethod();

    // natazeni metody z disku
    // throws CFileException, CStringException
    virtual void Load(CFile &file, CIUnitType *pIUnitType);

    // Cleaning
    virtual void Clean();

    // Translation of types (code string table -> global string table)
    virtual void Translate(CICodeStringTable *pCodeStringTable);

    // porovnavani metod
    bool operator==(CIMethod &anotherOne);

public:
    // navratovy typ
    CIType m_ReturnType;
};

#endif // !defined(AFX_IMETHOD_H__BB5D6164_9DA4_11D3_AF7F_004F49068BD6__INCLUDED_)
