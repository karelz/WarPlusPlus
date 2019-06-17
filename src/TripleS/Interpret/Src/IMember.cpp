/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Interpret
 *   Autor: Helena Kupková
 *  
 *   Popis: Trida CIMember uchovavajici informace o jedne polozce
 *          jednotky nebo struktury.
 *          
 ***********************************************************/

#include "stdafx.h"
#include "..\Interpret.h"

//////////////////////////////////////////////////////////////////////

CIMember::CIMember()
{
    m_stiName = NULL;
    m_nHeapOffset = -1;
}

//////////////////////////////////////////////////////////////////////

bool CIMember::operator==(CIMember &anotherOne)
{
    // m_nCodeStringTableID nemusi byt stejna, pouziva se jen pri loadu, dulezite je jmeno
    return (m_stiName == anotherOne.m_stiName) && (m_DataType == anotherOne.m_DataType) && 
           (m_nHeapOffset == anotherOne.m_nHeapOffset);

}

//////////////////////////////////////////////////////////////////////

void CIMember::Clean()
{
    if (m_stiName != NULL) m_stiName = m_stiName->Release();
    m_DataType.SetNoneType();
    m_nHeapOffset = -1;
}

//////////////////////////////////////////////////////////////////////

void CIMember::Load(CFile &file, long nHeapSize)
{
    // nacteni jmena memberu (ID kodove stringtable)
    file.Read( &m_nCodeStringTableID, sizeof(long));

    // nacteni typu polozky
    m_DataType.Load(file);               

    // nastaveni offsetu
    m_nHeapOffset=nHeapSize;
}

//////////////////////////////////////////////////////////////////////

void CIMember::Translate( CICodeStringTable *pCodeStringTable)
{
    ASSERT(m_stiName == NULL);
    m_stiName = (*pCodeStringTable)[m_nCodeStringTableID]->AddRef();

    m_DataType.Translate( pCodeStringTable);
}