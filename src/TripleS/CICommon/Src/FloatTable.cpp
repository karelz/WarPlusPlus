/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Prekladac, Interpret
 *   Autor: Helena Kupková
 *  
 *   Popis: Implementace tridy CFloatTable reprezentujici 
 *          tabulku floatu pouzitych prekladanou jednotkou/glob.fci
 * 
 ***********************************************************/

#include "stdafx.h"
#include "..\CICommon.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFloatTable::CFloatTable()
{
	m_aFloats.SetSize(3,3);
	m_nCount=0;
}

CFloatTable::~CFloatTable()
{
	m_aFloats.RemoveAll();
}

int CFloatTable::Add(double fFloat)
{
	int i;

	for (i=0; i<m_nCount; i++) if (m_aFloats[i]==fFloat) return i;
	m_aFloats.SetAtGrow(m_nCount, fFloat);
	return m_nCount++;
}

double CFloatTable::operator[] (int nIndex)
{
    ASSERT(nIndex >= 0 && nIndex < m_nCount);
    return m_aFloats[nIndex];
}

bool CFloatTable::operator== (CFloatTable &anotherOne)
{
    if (m_nCount != anotherOne.m_nCount) return false;

    int i;
    for (i=0; i<m_nCount; i++) if (m_aFloats[i] != anotherOne.m_aFloats[i]) return false;

    return true;
}

void CFloatTable::Save(CCompilerOutput* pOut)
{
	int i;

	pOut->PutLong(m_nCount);

	for (i=0; i<m_nCount; i++) 
	{
		TRACE_COMPILED_CODE2("%2d: %3.2\n", i, m_aFloats[i]);
		pOut->PutDouble(m_aFloats[i]);
	}
}

void CFloatTable::Load(CFile &file)
{
	long i,len;

	file.Read(&len, sizeof(long));

	m_aFloats.SetSize(len);
	for (i=0; i<len; i++) file.Read(&(m_aFloats[i]), sizeof(double));

	m_nCount = len;
}

void CFloatTable::Clean()
{
	m_aFloats.RemoveAll();
	m_nCount=0;
}

void CFloatTable::DebugWrite(CCompilerErrorOutput* pOut)
{
	int i;

	for (i=0; i<m_nCount; i++) pOut->Format("%d: %f\r\n", i, m_aFloats[i]);
	
	if (m_nCount==0) pOut->Format("empty\r\n");
}

void CFloatTable::UpdateFrom( CFloatTable *pAnotherOne)
{
    int i = 0;

    while ( i < m_nCount && i < pAnotherOne->m_nCount && m_aFloats[i] == pAnotherOne->m_aFloats[i]) i++;

    m_nCount = pAnotherOne->m_nCount;

    m_aFloats.SetSize(m_nCount);

    while ( i < m_nCount) 
    {
        m_aFloats[i] = pAnotherOne->m_aFloats[i];
        i++;
    }
}
