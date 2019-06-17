/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Interpret
 *   Autor: Helena Kupková
 *  
 *   Popis: Implementace tridy CICode uchovavajici kod,
 *          ktery je implementovan interpretem.
 * 
 ***********************************************************/

#include "stdafx.h"
#include "..\Interpret.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CICode::CICode()
{
    m_nLength = NULL;
    m_pCode = NULL;
}

CICode::~CICode()
{
    if (m_nLength != 0 && m_pCode != NULL) delete m_pCode;
}

void CICode::Load(CFile &file)
{
    ASSERT(m_nLength == 0);
    ASSERT(m_pCode == NULL);

    file.Read(&m_nLength, sizeof(long));

    if (m_nLength != 0)
    {
        m_pCode = new INSTRUCTION [ m_nLength / sizeof(INSTRUCTION) ];
        file.Read(m_pCode, m_nLength);
    }
}

bool CICode::operator ==(CICode &anotherOne)
{
    long L=0, len = m_nLength/sizeof(INSTRUCTION);

    if (m_nLength != anotherOne.m_nLength) return false;

    while ( L < len  &&  m_pCode[L] == anotherOne.m_pCode[L]) 
    {
        L++;
    }
    
    if (L < len) return false;
    
    return true;
}