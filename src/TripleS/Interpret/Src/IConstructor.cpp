/************************************************************
*
* Projekt: Strategicka hra
*    Cast: Interpret
*   Autor: Helena Kupková
*  
*   Popis: Implementace tridy CIConstructor, ktera reprezentuje
*          konstruktor (i destruktor) nejake jednotky.
*          
***********************************************************/

#include "stdafx.h"
#include "..\Interpret.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CIConstructor::CIConstructor()
{
    m_nArgAndLocalVarCount = 0;
    m_apArgAndLocalVars = NULL;
    m_bLoaded = false;
    
    m_nLocalProfileCounter = 0;
    m_nLastLocalProfileCounter = 0;
    m_nGlobalProfileCounter = 0;
}

CIConstructor::~CIConstructor()
{
    Clean();    
}

#ifdef _DEBUG
void CIConstructor::AssertValid() const
{
    ASSERT(m_bLoaded);
}
#endif

void CIConstructor::Clean()
{
    delete [] m_apArgAndLocalVars;
    m_nArgAndLocalVarCount = 0;
    m_apArgAndLocalVars = NULL;
    
    m_nLocalProfileCounter = 0;
    m_nLastLocalProfileCounter = 0;
    m_nGlobalProfileCounter = 0;
}

void CIConstructor::Load(CFile &file, CIUnitType *pIUnitType)
{
    short i;
    
    ASSERT(!m_bLoaded);
    Clean();
    
    // pocet lokalnich promennych
    file.Read(&m_nArgAndLocalVarCount, sizeof(short));
    ASSERT(m_nArgAndLocalVarCount>=0);
    
    // nacteni typu lokalnich promennych
    if (m_nArgAndLocalVarCount !=0) m_apArgAndLocalVars = new CIType[m_nArgAndLocalVarCount]; 
    else m_apArgAndLocalVars = NULL;
    
    for (i=0; i<m_nArgAndLocalVarCount; i++) 
    {
        m_apArgAndLocalVars[i].Load(file);
    }
    
    // nacteni kodu
    m_Code.Load(file);
    
    // KONVERZE JMEN typu se provede az po naloadovani cele unity,
    // teprve potom bude zavolana metoda Translate a nastaven
    // priznak m_bLoaded.
    
    m_pIUnitType = pIUnitType;
}

void CIConstructor::Translate(CICodeStringTable *pCodeStringTable)
{
    int i;
    
    for (i=0; i<m_nArgAndLocalVarCount; i++) 
    {
        m_apArgAndLocalVars[i].Translate(pCodeStringTable);
    }
    
    m_bLoaded = true;
}

bool CIConstructor::operator==(CIConstructor &anotherOne)
{
    int i;
    
    // pocet lokalnich promennych
    if (m_nArgAndLocalVarCount != anotherOne.m_nArgAndLocalVarCount) return false;
    
    // typy lokalnich promennych
    for (i=0; i<m_nArgAndLocalVarCount; i++)
    {
        if (!(m_apArgAndLocalVars[i] == anotherOne.m_apArgAndLocalVars[i])) return false;
    }
    
    // kod
    if (!(m_Code == anotherOne.m_Code)) return false;
    
    return true;
}