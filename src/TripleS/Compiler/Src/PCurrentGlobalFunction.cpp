/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Prekladac
 *   Autor: Helena Kupková
 *  
 *   Popis: Implementace tridy CPCurrentGlobalFunction reprezentujici 
 *          globalni funkci, ktera je prave prekladana (nebo je uz prelozena).
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

CPCurrentGlobalFunction::CPCurrentGlobalFunction(CString strName, CType &ReturnDataType)
	: CPCurrentUnit::CMethod(NULL, ReturnDataType)
{
	m_strName=strName;
	m_bOK=true;
    m_pNext = NULL;
}

CPCurrentGlobalFunction::~CPCurrentGlobalFunction()
{
	m_StringTable.RemoveAll();
}

void CPCurrentGlobalFunction::Save(CCompilerOutput *pOut)
{
	int i,len;

	TRACE_COMPILED_CODE("\nGlobalni funkce '%s'\n", m_strName);

	// sign
	pOut->PutString(FUNCTIONFILE_FIRTSBYTES);
	
	// jmeno funkce
	pOut->PutShort(m_strName.GetLength());
	pOut->PutString(m_strName);

	// navratovy typ
	m_ReturnDataType.Save(pOut, m_StringTable, false);

	// pocet argumentu
	pOut->PutShort(m_nArgCount);

	// pocet argumentu + lokalnich promennych
	pOut->PutShort(len=m_aArgsAndLocalVars.GetSize());

	// typy argumentu & lokalnich promennych
	for (i=0; i<len; i++) 
	{
		TRACE_COMPILED_CODE2("%2d: %s\n", -(i+1), (CString)*m_aArgsAndLocalVars[i].m_stiName);
		m_aArgsAndLocalVars[i].m_DataType.Save(pOut, m_StringTable, (i>=m_nArgCount));
	}

	// kod
	m_Code.Save(pOut);

	// tabulky
	m_StringTable.Save(pOut);
	m_FloatTable.Save(pOut);
}

/*#ifdef _DEBUG_LOAD
bool CPCurrentGlobalFunction::Load(CFile &fileIn)
{
	char pom[512];
	short i,len;

	// sign
	fileIn.Read(pom, strlen(FUNCTIONFILE_FIRTSBYTES));
	pom[strlen(FUNCTIONFILE_FIRTSBYTES)]=0;
	if (strcmp(pom, FUNCTIONFILE_FIRTSBYTES)!=0) return false;

	// jmeno funkce
	fileIn.Read(&i, sizeof(short));
	fileIn.Read(pom, i);
	pom[i]=0;
	m_strName=pom;

	// tabulky
	m_StringTable.Load(fileIn);
	m_FloatTable.Load(fileIn);

	// navratovy typ
	m_ReturnDataType.Load(fileIn, m_StringTable, m_StringTable);

	// pocet argumentu
	fileIn.Read(&i, sizeof(short));
	m_nArgCount=i;

	// pocet argumentu + lokalnich promennych
	fileIn.Read(&len, sizeof(short));

	// typy argumentu a lokalnich promennych
	m_aArgsAndLocalVars.SetSize(len);
	for (i=0; i<len; i++)
	{
		m_aArgsAndLocalVars[i].m_stiName=m_StringTable.Add("");
		m_aArgsAndLocalVars[i].m_DataType.Load(fileIn, m_StringTable, m_StringTable);
	}

	// kod
	m_Code.Load(fileIn);

	m_bOK=true;

	return true;
}
#endif
*/

void CPCurrentGlobalFunction::DebugWrite(CCompilerErrorOutput *pOut)
{
	int i, len;

	// hlavicka funkce
	pOut->Format("\r\n===================================\r\n");
	pOut->Format("%s", m_strName);
	pOut->Format("\r\n===================================\r\n");
	pOut->Format(">> returns %s\r\n",m_ReturnDataType.GetString());
	pOut->Format(">> %d arguments\r\n", m_nArgCount);

	len=m_aArgsAndLocalVars.GetSize();
	for (i=0; i<len; i++)
	{
		pOut->Format("%d:\t%s %s;\r\n", -(i+1), (CString)*m_aArgsAndLocalVars[i].m_stiName,
			m_aArgsAndLocalVars[i].m_DataType.GetString());
	}
	pOut->PutString("---------------\r\n");
	m_Code.DebugWrite(pOut);

	// stringtable
	pOut->Format("\r\nStringTable:\r\n");
	m_StringTable.DebugWrite(pOut);

	// float table
	pOut->Format("\r\nFloatTable:\r\n");
	m_FloatTable.DebugWrite(pOut);
}

