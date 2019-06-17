/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Interpret
 *   Autor: Helena Kupková
 *  
 *   Popis: Implementace tridy CIEvent reprezentujici reakci na udalost
 *          nejake jednotky.
 *          
 ***********************************************************/

#if !defined(AFX_IEVENT_H__BB5D6166_9DA4_11D3_AF7F_004F49068BD6__INCLUDED_)
#define AFX_IEVENT_H__BB5D6166_9DA4_11D3_AF7F_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IConstructor.h"

class CIEvent : public CIConstructor  
{
public:
	CIEvent();
	virtual ~CIEvent();

    // natazeni eventy z disku
    // throws CFileException, CStringException
    virtual void Load(CFile &file, CIUnitType *pIUnitType);

    // Cleaning
    virtual void Clean();

    // Translation of types (code string table -> global string table)
    virtual void Translate(CICodeStringTable *pCodeStringTable);

    // porovnavani eventu
    bool operator==(CIEvent &anotherOne);

public:
    // jmeno eventy (uppercase)
    CStringTableItem *m_stiName;

	// priznak, zda je to command nebo notification
	bool m_bIsCommand;

    // pocet argumentu
    short m_nArgCount;

protected:
    // ID jmena ve stringtable kodu
    long m_nCodeStringTableID;
};

#endif // !defined(AFX_IEVENT_H__BB5D6166_9DA4_11D3_AF7F_004F49068BD6__INCLUDED_)
