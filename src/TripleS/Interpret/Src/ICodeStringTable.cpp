/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Interpret
 *   Autor: Helena Kupková
 *  
 *   Popis: Implementace tridy CICodeStringTable, ktera reprezentuje
 *          tabulku stringu urciteho kodu (inicializuje se z tabulky
 *          stringu, ktera se uklada s kodem). Je to vlastne jen prevodni
 *          tabulka, prevadi ID stringu pouziteho v kodu na polozce globalni 
 *          tabulky stringu.
 * 
 ***********************************************************/

#include "stdafx.h"
#include "..\Interpret.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CICodeStringTable::CICodeStringTable()
{
    m_nStringTableCount=0;
    m_apCodeStringTable.SetSize(0,0);
}

CICodeStringTable::~CICodeStringTable()
{
    Clean();
    m_apCodeStringTable.RemoveAll();
}

void CICodeStringTable::Clean()
{
    int i;
    for (i=0; i < m_nStringTableCount; i++) m_apCodeStringTable.GetAt(i)->Release();
    m_nStringTableCount = 0;
}

void CICodeStringTable::Load(CFile &file)
{
    long i;
    short length;
    char string[MAX_STRING_LENGTH];
    
    ASSERT(m_nStringTableCount == 0);

    file.Read(&m_nStringTableCount, sizeof(long));

    m_apCodeStringTable.SetSize(m_nStringTableCount, 0);

    for (i=0; i<m_nStringTableCount; i++)
    {
        file.Read(&length, sizeof(short));
        if (length>=MAX_STRING_LENGTH) 
            throw new CStringException("%s: string too long (Code Stringtable ID=%d)", file.GetFileName(), i);
        file.Read(&string, length);
        string[length]=0;

        m_apCodeStringTable.SetAt(i, g_StringTable.AddItem(string));
    }
}

CStringTableItem* CICodeStringTable::operator[] (long nIndex)
{
    if (nIndex < 0 || nIndex >= m_nStringTableCount) return NULL;
    return m_apCodeStringTable.GetAt(nIndex);
}

bool CICodeStringTable::operator==(CICodeStringTable &anotherOne)
{
    long L;

    if (m_nStringTableCount != anotherOne.m_nStringTableCount) return false;

    for ( L = 0; L < m_nStringTableCount; L++) 
    {
        if (m_apCodeStringTable[L] != anotherOne.m_apCodeStringTable[L]) return false;
    }
    return true;
}

void CICodeStringTable::UpdateFrom( CICodeStringTable *pAnotherOne)
{
    int j, i = 0;

    while ( i < m_nStringTableCount  &&  i < pAnotherOne->m_nStringTableCount &&
            m_apCodeStringTable[i] == pAnotherOne->m_apCodeStringTable[i])  i++;

    j = i;

    while ( i < m_nStringTableCount) 
    {
        m_apCodeStringTable[i]->Release();
        i++;
    }

    m_nStringTableCount =  pAnotherOne->m_nStringTableCount;
    
    m_apCodeStringTable.SetSize( m_nStringTableCount, 0);

    while ( j < m_nStringTableCount)
    {
        m_apCodeStringTable[j] = pAnotherOne->m_apCodeStringTable[j]->AddRef();
        j++;
    }
}
