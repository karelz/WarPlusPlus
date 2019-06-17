/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Prekladac
 *   Autor: Helena Kupková
 *  
 *   Popis: Implementace trid CPCodeStringTable a CPCodeStringTableItem,
 *          tabulky stringu kompilatoru pro aktualne prekladanou jednotku,
 *          strunturu, nebo globalni funkci.
 * 
 ***********************************************************/

#include "stdafx.h"
#include "CompilerKernel.h"

#include "TripleS\CompilerIO\CompilerIO.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// CPCodeStringTableItem
//////////////////////////////////////////////////////////////////////

CPCodeStringTableItem::operator LPCTSTR() const
{
    return (LPCTSTR)m_strString;
}

CPCodeStringTableItem::operator CString() const
{
    return m_strString;
}

//////////////////////////////////////////////////////////////////////
// CPCodeStringTable
//////////////////////////////////////////////////////////////////////

CPCodeStringTable::CPCodeStringTable()
{
    m_apCSTIArray.SetSize(10,10);
	m_apCSTITable.InitHashTable(HASHTABLE_SIZE);
    m_nFirstFreeID=0;
}

CPCodeStringTable::~CPCodeStringTable()
{
    RemoveAll();
}

void CPCodeStringTable::RemoveAll()
{
    CPCodeStringTableItem *pCSTI;
	CString str;
	POSITION pos;

	pos=m_apCSTITable.GetStartPosition();
	while (pos!=NULL)
	{
		m_apCSTITable.GetNextAssoc(pos, str, pCSTI);
		delete pCSTI;
    }
	m_apCSTITable.RemoveAll();

	m_apCSTIArray.RemoveAll();
}

void CPCodeStringTable::Save(CCompilerOutput *pOut)
// writes the string table on the output, return # bytes written
{
    int i;

	pOut->PutLong(m_nFirstFreeID);  // pocet polozek

    for (i=0; i<m_nFirstFreeID; i++)  
	{
		TRACE_COMPILED_CODE2("%2d: \"%s\"\n", i, m_apCSTIArray[i]->m_strString);

		pOut->PutShort(m_apCSTIArray[i]->m_strString.GetLength());
		pOut->PutString(m_apCSTIArray[i]->m_strString);
	}
}

void CPCodeStringTable::Load(CFile &fileIn)
{
	long i,len;
	short slen;
	char pom[1024];

	RemoveAll();

	fileIn.Read(&len, sizeof(long));

	for (i=0; i<len; i++)
	{
		fileIn.Read(&slen, sizeof(short));
		fileIn.Read(pom, slen);
		pom[slen]=0;
		Add(pom);
	}
}

CPCodeStringTableItem* CPCodeStringTable::Add(CString &strString)
{
    CPCodeStringTableItem *pCSTI;

    if (m_apCSTITable.Lookup(strString, pCSTI)) return pCSTI;
	else
	{
        pCSTI=new CPCodeStringTableItem();
        pCSTI->m_nID=m_nFirstFreeID++;
        pCSTI->m_strString=strString;
		
		m_apCSTIArray.SetAtGrow(pCSTI->m_nID, pCSTI);
		m_apCSTITable.SetAt(strString, pCSTI);
    }

    return pCSTI;
}   

CPCodeStringTableItem* CPCodeStringTable::Add(LPCTSTR string)
{
	return Add(CString(string));
}

void CPCodeStringTable::DebugWrite(CCompilerErrorOutput* pOut)
{
	int i;

    for (i=0; i<m_nFirstFreeID; i++)
        pOut->Format("%d: %s\r\n", m_apCSTIArray[i]->m_nID, m_apCSTIArray[i]->m_strString);

	if (m_nFirstFreeID==0) pOut->Format("empty\r\n");
}
