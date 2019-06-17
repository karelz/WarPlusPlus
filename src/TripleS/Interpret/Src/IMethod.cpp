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

#include "stdafx.h"
#include "..\Interpret.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CIMethod::CIMethod()
    : CIEvent()
{
}

CIMethod::~CIMethod()
{
}

void CIMethod::Clean()
{
    CIEvent::Clean();
    m_ReturnType.SetNoneType();
}

void CIMethod::Load(CFile &file, CIUnitType *pIUnitType)
{
    short i;

    ASSERT(!m_bLoaded);
    Clean();

	// jmeno metody
    file.Read(&m_nCodeStringTableID, sizeof(long));

    // navratovy typ
    m_ReturnType.Load(file);

    // pocet argumentu
	file.Read(&m_nArgCount, sizeof(short));
    ASSERT(m_nArgCount>=0);

    // pocet argumentu + lokalnich promennych
	file.Read(&m_nArgAndLocalVarCount, sizeof(short));
    ASSERT(m_nArgAndLocalVarCount>=0);

    // nacteni typu argumentu & lokalnich promennych
    if ( m_nArgAndLocalVarCount != 0) m_apArgAndLocalVars = new CIType[m_nArgAndLocalVarCount]; 
    else m_apArgAndLocalVars = NULL;
    for (i=0; i<m_nArgAndLocalVarCount; i++) 
    {
        m_apArgAndLocalVars[i].Load(file);
    }

    // nacteni kodu
    m_Code.Load(file);

    m_pIUnitType = pIUnitType;
}

void CIMethod::Translate(CICodeStringTable *pCodeStringTable)
{
    m_ReturnType.Translate(pCodeStringTable);
    
    CIEvent::Translate(pCodeStringTable);
}

bool CIMethod::operator==(CIMethod &anotherOne)
{
    return ( m_ReturnType == anotherOne.m_ReturnType) && (CIEvent::operator ==(anotherOne));
}