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

#include "stdafx.h"
#include "..\Interpret.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CIEvent::CIEvent()
    :CIConstructor()
{
    m_stiName = NULL;
    m_nCodeStringTableID = -1;
    m_nArgCount = 0;
	m_bIsCommand = false;
}

CIEvent::~CIEvent()
{
    Clean();    
}

void CIEvent::Clean()
{
    CIConstructor::Clean();
    if (m_stiName != NULL) m_stiName->Release();
}

void CIEvent::Load(CFile &file, CIUnitType *pIUnitType)
{
    short i;

    ASSERT(!m_bLoaded);
    Clean();

	// jmeno eventy
    file.Read(&m_nCodeStringTableID, sizeof(long));

	// priznak, zda je to command nebo notification
	file.Read( &i, sizeof(short));
	m_bIsCommand = (i==1) ? true : false;

    // pocet argumentu
	file.Read(&m_nArgCount, sizeof(short));
    ASSERT(m_nArgCount>=0);

    // pocet argumentu + lokalnich promennych
	file.Read(&m_nArgAndLocalVarCount, sizeof(short));
    ASSERT(m_nArgAndLocalVarCount>=0);

    // nacteni typu argumentu & lokalnich promennych
	 if (m_nArgAndLocalVarCount != 0)
	 {
		m_apArgAndLocalVars = new CIType[m_nArgAndLocalVarCount]; 
		for (i=0; i<m_nArgAndLocalVarCount; i++) 
		{
			m_apArgAndLocalVars[i].Load(file);
		}
	 }
	 else m_apArgAndLocalVars = NULL;

    // nacteni kodu
    m_Code.Load(file);

    m_pIUnitType = pIUnitType;
}

void CIEvent::Translate(CICodeStringTable *pCodeStringTable)
{
    ASSERT(m_stiName == NULL);
    m_stiName = (*pCodeStringTable)[m_nCodeStringTableID]->AddRef();

    CIConstructor::Translate(pCodeStringTable);
}

bool CIEvent::operator==(CIEvent &anotherOne)
{
    return (m_stiName == anotherOne.m_stiName) && (m_nArgCount == anotherOne.m_nArgCount) &&
		(m_bIsCommand == anotherOne.m_bIsCommand) && (CIConstructor::operator ==(anotherOne) );
}