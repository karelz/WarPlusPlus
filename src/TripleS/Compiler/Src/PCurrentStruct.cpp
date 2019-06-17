/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Prekladac
 *   Autor: Helena Kupková
 *  
 *   Popis: Implementace tridy CPCurrentStruct reprezentujici 
 *          strukturu (struct), ktera je prave prekladana (nebo je uz prelozena).
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
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPCurrentStruct::CPCurrentStruct(CString strName)
{
    m_strName=strName;
    m_nMemberCount=0;
    m_bOK=true;
    m_pNext = NULL;
}

CPCurrentStruct::~CPCurrentStruct()
{
    m_aMemberNames.RemoveAll();
    m_aMemberTypes.RemoveAll();
}

void CPCurrentStruct::AddMember(CString &strName, CType &DataType)
{
    m_aMemberNames.SetAtGrow(m_nMemberCount, m_StringTable.Add(strName));
    m_aMemberTypes.SetAtGrow(m_nMemberCount, DataType);
    m_nMemberCount++;
}

bool CPCurrentStruct::HasMember(CString strName)
{
    int i;
    for (i=0; i<m_nMemberCount; i++) if ((CString)*m_aMemberNames.GetAt(i)==strName) return true;
    return false;
}

// ulozeni na disk
void CPCurrentStruct::Save(CCompilerOutput *pOut)
{
	int i;
    CString str;

	// sign
	pOut->PutString(STRUCTFILE_FIRTSBYTES);
	
	// jmeno struktury
	pOut->PutShort(m_strName.GetLength());
	pOut->PutString(m_strName);

	// pocet polozek
	pOut->PutShort(m_nMemberCount);

	// polozky
	for (i=0; i<m_nMemberCount; i++) 
    {
        pOut->PutLong(m_aMemberNames.GetAt(i)->m_nID);
        m_aMemberTypes.GetAt(i).Save(pOut, m_StringTable, true);
    }

    // tabulka stringu
	m_StringTable.Save(pOut);
}

void CPCurrentStruct::DebugWrite(CCompilerErrorOutput *pOut)
{
	int i;

	// hlavicka funkce
	pOut->Format("\r\n===================================\r\n");
	pOut->Format("struct %s", m_strName);
	pOut->Format("\r\n===================================\r\n");

	for (i=0; i<m_nMemberCount; i++)
	{
		pOut->Format("%s %s;\r\n", m_aMemberTypes[i].GetString(), (CString)*m_aMemberNames[i]);
	}
	pOut->PutString("---------------\r\n");

	// stringtable
	pOut->Format("\r\nStringTable:\r\n");
	m_StringTable.DebugWrite(pOut);
}