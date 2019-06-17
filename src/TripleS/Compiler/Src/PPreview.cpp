/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Prekladac
 *   Autor: Helena Kupková
 *  
 *   Popis: Implementace trid CMethod, CUnit a CPPreview,
 *          uchovavaji informace ziskane behem prvniho pruchodu prekladace
 *          (nazvy jednotek a jejich metod, hlavicky metod)
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

CPPreview::CPPreview(CPNamespace *pNamespace)
{
    ASSERT(pNamespace!=NULL);
    m_pNamespace = pNamespace;

    m_apUnits.InitHashTable(UNITS_HASHTALE_SIZE);
	m_apGlobalFunctions.InitHashTable(GLOBALFUNCT_HASHTABLE_SIZE);
}

CPPreview::~CPPreview()
{
    RemoveAll();
}

void CPPreview::RemoveAll()
{
	CPNamespace::CUnit *pUnit;
	CPNamespace::CGlobalFunction *pFunc;
    CPNamespace::CStruct *pStruct;
	POSITION pos;
	CString str;

	pos=m_apUnits.GetStartPosition();
	while (pos!=NULL)
	{
		m_apUnits.GetNextAssoc(pos, str, pUnit);
		delete pUnit;
	}
	m_apUnits.RemoveAll();

	pos=m_apGlobalFunctions.GetStartPosition();
	while (pos!=NULL)
	{
		m_apGlobalFunctions.GetNextAssoc(pos, str, pFunc);
		delete pFunc;
	}
	m_apGlobalFunctions.RemoveAll();

    pos=m_apStructs.GetStartPosition();
    while (pos!=NULL)
    {
        m_apStructs.GetNextAssoc(pos,str,pStruct);
        delete pStruct;
    }
    m_apStructs.RemoveAll();
}

CPNamespace::CUnit* CPPreview::AddUnit(CString &strName)
{
	CPNamespace::CUnit *pUnit;
	if (!m_apUnits.Lookup( strName, pUnit))
	{
		pUnit = new CPNamespace::CUnit(m_pNamespace, g_StringTable.AddItem(strName, false), NULL);
		m_apUnits.SetAt(strName, pUnit);
	}
    return pUnit;
}

CPNamespace::CUnit* CPPreview::GetUnit(CString &strName)
{
	CPNamespace::CUnit *pUnit;
	if (m_apUnits.Lookup(strName, pUnit)) return pUnit;
	else return NULL;
}		

CPNamespace::CGlobalFunction* CPPreview::AddGlobalFunction(CString &strName, CString& strReturnType,
                                                           int nArrayLevel)
{
	CType RetType;
	RetType.Set(strReturnType, nArrayLevel, m_pNamespace, this);

	CPNamespace::CGlobalFunction *pFunc=
        new CPNamespace::CGlobalFunction(g_StringTable.AddItem(strName, false), RetType);
	m_apGlobalFunctions.SetAt(strName, pFunc);
	return pFunc;
}

CPNamespace::CGlobalFunction* CPPreview::GetGlobalFunction(CString &strName)
{
	CPNamespace::CGlobalFunction *pFunc;
	if (m_apGlobalFunctions.Lookup(strName, pFunc)) return pFunc;
	else return NULL;
}

CPNamespace::CStruct* CPPreview::AddStruct(CString &strName)
{
    CPNamespace::CStruct *pStruct;
    pStruct=new CPNamespace::CStruct(m_pNamespace, g_StringTable.AddItem(strName, false));
    m_apStructs.SetAt(strName,pStruct);
    return pStruct;
}

CPNamespace::CStruct* CPPreview::GetStruct(CString &strName)
{
    CPNamespace::CStruct *pStruct;
    if (m_apStructs.Lookup(strName, pStruct)) return pStruct;
    else return NULL;
}

void CPPreview::DebugWrite(CCompilerErrorOutput *pOut)
{
	long j;
	POSITION pos, pos1;
	CString str;
	CPNamespace::CUnit *pUnit;
	CPNamespace::CUnitMember *pUMember;
	CPNamespace::CUnitMethod *pUMethod;
	CPNamespace::CGlobalFunction *pFunc;

	pOut->Format("\r\nPREVIEWED UNITS:\r\n==================\r\n\r\n");

	pos1=m_apUnits.GetStartPosition();
	while (pos1!=NULL)
	{
		m_apUnits.GetNextAssoc(pos1, str, pUnit);
		pOut->Format("unit %s\r\n{\r\n", (CString)*pUnit->m_stiName);

		pos=pUnit->m_apMembers.GetStartPosition();
		while (pos!=NULL)
		{
			pUnit->m_apMembers.GetNextAssoc(pos, str, pUMember);
			pOut->Format("\t%s %s;\r\n", pUMember->m_DataType.GetString(), (CString)*pUMember->m_stiName);
		}

		pos=pUnit->m_apMethods.GetStartPosition();
		while (pos!=NULL)
		{
			pUnit->m_apMethods.GetNextAssoc(pos, str, pUMethod);
			pOut->Format("\t%s %s (", pUMethod->m_ReturnDataType.GetString(), 
				(CString)*pUMethod->m_stiName);

			for (j=0; j<pUMethod->m_aArgs.GetSize(); j++)
			{
				pOut->Format("%s", pUMethod->m_aArgs[j].GetString());
				if (j!=pUMethod->m_aArgs.GetSize()-1) pOut->Format(", ");
			}

			pOut->Format(");\r\n");
		}
		pOut->Format("\r\n}\r\n\r\n");
	}

	pOut->Format("\r\nPREVIEWED GLOBAL FUNCTIONS:\r\n===========================\r\n\r\n");

	pos1=m_apGlobalFunctions.GetStartPosition();
	while (pos1!=NULL)
	{
		m_apGlobalFunctions.GetNextAssoc(pos1, str, pFunc);
		pOut->Format("%s %s (", pFunc->m_ReturnDataType.GetString(), (CString)*pFunc->m_stiName);
		for (j=0; j<pFunc->m_aArgs.GetSize(); j++)
		{
			pOut->Format("%s", pFunc->m_aArgs[j].GetString());
			if (j!=pFunc->m_aArgs.GetSize()-1) pOut->Format(", ");
		}

		pOut->Format(");\r\n");
	}
}