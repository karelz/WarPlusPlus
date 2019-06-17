/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Prekladac
 *   Autor: Helena Kupková
 *  
 *   Popis: Implementace tridy CPCurrentUnit reprezentujici 
 *          jednotku (unit), ktera je prave prekladana (nebo je uz prelozena)
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
// CPCurrentUnit::CMethodEvent
//////////////////////////////////////////////////////////////////////

CPCurrentUnit::CMethodEvent::CMethodEvent(CPCodeStringTableItem *cstiName)
{
    m_cstiName=cstiName;
	m_aArgsAndLocalVars.SetSize(0,1);
	m_nArgCount=0;
}

CPCurrentUnit::CMethodEvent::~CMethodEvent()
{
    int i, count=m_aArgsAndLocalVars.GetSize();
    
    for (i=0; i<count; i++) m_aArgsAndLocalVars[i].m_stiName->Release();
}

REG CPCurrentUnit::CMethodEvent::AddArgument(CStringTableItem *stiName, CType&DataType)
{
	m_aArgsAndLocalVars.SetSize(m_nArgCount+1);
	m_aArgsAndLocalVars[m_nArgCount].m_stiName=stiName->AddRef();
	m_aArgsAndLocalVars[m_nArgCount].m_DataType=DataType;
	m_nArgCount++;
	
	return -m_nArgCount;
}

REG CPCurrentUnit::CMethodEvent::AddLocalVariable(CStringTableItem *stiName, CType&DataType)
{
	int i=m_aArgsAndLocalVars.GetSize()+1;
	m_aArgsAndLocalVars.SetSize(i);
	m_aArgsAndLocalVars[i-1].m_stiName=stiName->AddRef();
	m_aArgsAndLocalVars[i-1].m_DataType=DataType;

	return -i;
}

bool CPCurrentUnit::CMethodEvent::IsArgOrLocalVar(CString& strName)
{
	int i,len=m_aArgsAndLocalVars.GetSize();
	for (i=0; i<len; i++) 
		if ((CString)*m_aArgsAndLocalVars[i].m_stiName==strName) return true;
	return false;
}


/*#ifdef _DEBUG_LOAD
void CPCurrentUnit::CEvent::Load(CFile &fileIn, CStringTable &OldStringTable, CStringTable &NewStringTable)
{
	long l=0;
	short i,len=0;

	// jmeno
	fileIn.Read(&l, sizeof(long));
	m_stiName=OldStringTable.GetItemByID(l);

	// pocet argumentu
	fileIn.Read(&i, sizeof(short));
	m_nArgCount=i;

	// pocet argumentu + lokalnich promennych
	fileIn.Read(&len, sizeof(short));

	// typy argumentu a lokalnich promennych
	m_aArgsAndLocalVars.SetSize(len);
	for (i=0; i<len; i++)
	{
		m_aArgsAndLocalVars[i].m_stiName=NULL;
		m_aArgsAndLocalVars[i].m_DataType.Load(fileIn, OldStringTable, NewStringTable);
	}

	// kod
	m_Code.Load(fileIn);
}
#endif
*/


//////////////////////////////////////////////////////////////////////
// CPCurrentUnit::CMethod
//////////////////////////////////////////////////////////////////////

CPCurrentUnit::CMethod::CMethod(CPCodeStringTableItem *cstiName, CType&ReturnDataType)
	: CMethodEvent(cstiName)
{
	m_ReturnDataType=ReturnDataType;
	m_nArgCount=0;
}

void CPCurrentUnit::CMethod::Save(CCompilerOutput *pOut, CPCodeStringTable &StrTbl)
{
	int i,len;

	// jmeno
	pOut->PutLong(m_cstiName->m_nID);

	// navratovy typ
	m_ReturnDataType.Save(pOut, StrTbl, false);

	// pocet argumentu
	pOut->PutShort(m_nArgCount);

	// pocet argumentu + lokalnich promennych
	pOut->PutShort(len=m_aArgsAndLocalVars.GetSize());

	// typy argumentu & lokalnich promennych
	for (i=0; i<len; i++) 
	{
		TRACE_COMPILED_CODE2("%2d: %s\n", -(i+1), (CString)*m_aArgsAndLocalVars[i].m_stiName);
		m_aArgsAndLocalVars[i].m_DataType.Save(pOut, StrTbl, (i>=m_nArgCount));
	}

	// kod
	m_Code.Save(pOut);
}

/*#ifdef _DEBUG_LOAD
void CPCurrentUnit::CMethod::Load(CFile &fileIn, CStringTable &OldStringTable, CStringTable &NewStringTable)
{
	long l=0;
	int i,len=0;

	// jmeno
	fileIn.Read(&l, sizeof(long));
	m_stiName=OldStringTable.GetItemByID(l);
	m_stiName=NewStringTable.Add((CString)*m_stiName);

	// navratovy typ
	m_ReturnDataType.Load(fileIn, OldStringTable, NewStringTable);

	// pocet argumentu
	fileIn.Read(&len, sizeof(short));
	m_nArgCount=len;

	// pocet argumentu + lokalnich promennych
	fileIn.Read(&len, sizeof(short));

	// typy argumentu a lokalnich promennych
	m_aArgsAndLocalVars.SetSize(len);
	for (i=0; i<len; i++)
	{
		m_aArgsAndLocalVars[i].m_stiName=NewStringTable.Add("");
		m_aArgsAndLocalVars[i].m_DataType.Load(fileIn, OldStringTable, NewStringTable);
	}

	// kod
	m_Code.Load(fileIn);
}
#endif
*/
//////////////////////////////////////////////////////////////////////
// CPCurrentUnit::CEvent
//////////////////////////////////////////////////////////////////////

CPCurrentUnit::CEvent::CEvent(CPCodeStringTableItem *cstiName, bool bIsCommand)
	: CMethodEvent(cstiName)
{
	m_bIsCommand = bIsCommand;
}

void CPCurrentUnit::CEvent::Save(CCompilerOutput *pOut, CPCodeStringTable &StrTbl)
{
	int i,len;

	// jmeno
	pOut->PutLong(m_cstiName->m_nID);

	// command / notification
	pOut->PutShort( m_bIsCommand ? 1 : 0);

	// pocet argumentu
	pOut->PutShort(m_nArgCount);

	// pocet argumentu + lokalnich promennych
	pOut->PutShort(len=m_aArgsAndLocalVars.GetSize());

	// typy argumentu & lokalnich promennych
	for (i=0; i<len; i++) 
	{
		TRACE_COMPILED_CODE2("%2d: %s\n", -(i+1), (CString)*m_aArgsAndLocalVars[i].m_stiName);
		m_aArgsAndLocalVars[i].m_DataType.Save(pOut, StrTbl, (i>=m_nArgCount));
	}

	// kod
	m_Code.Save(pOut);
}

//////////////////////////////////////////////////////////////////////
// CPCurrentUnit::CMember
//////////////////////////////////////////////////////////////////////

CPCurrentUnit::CMember::CMember(CPCodeStringTableItem *cstiName, CType&DataType)
{
    m_cstiName=cstiName;
	m_DataType=DataType;
}

void CPCurrentUnit::CMember::Save(CCompilerOutput *pOut, CPCodeStringTable &StrTbl)
{
	// jmeno
	pOut->PutLong(m_cstiName->m_nID);

	// typ
	m_DataType.Save(pOut, StrTbl, true);
}

/*#ifdef _DEBUG_LOAD
void CPCurrentUnit::CMember::Load(CFile &fileIn, CStringTable &OldStringTable, CStringTable &NewStringTable)
{
	long l=0;

	// jmeno
	fileIn.Read(&l, sizeof(long));
	m_stiName=OldStringTable.GetItemByID(l);
	ASSERT(m_stiName!=NULL);
	m_stiName=NewStringTable.Add((CString)*m_stiName);

	// typ
	m_DataType.Load(fileIn, OldStringTable, NewStringTable);
}
#endif
  */
//////////////////////////////////////////////////////////////////////
// CPCurrentUnit::CConstructor
//////////////////////////////////////////////////////////////////////

void CPCurrentUnit::CConstructor::Save(CCompilerOutput *pOut, CPCodeStringTable &StrTbl)
{
	int i,len;

	// pocet lokalnich promennych
	pOut->PutShort(len=m_aArgsAndLocalVars.GetSize());

	// typy lokalnich promennych
	for (i=0; i<len; i++) 
	{
		TRACE_COMPILED_CODE2("%2d: %s\n", -(i+1), (CString)*m_aArgsAndLocalVars[i].m_stiName);
		m_aArgsAndLocalVars[i].m_DataType.Save(pOut, StrTbl, true);
	}
	
	// kod
	m_Code.Save(pOut);
}

/*#ifdef _DEBUG_LOAD
void CPCurrentUnit::CConstructor::Load(CFile &fileIn, CStringTable &OldStringTable, CStringTable &NewStringTable)
{
	int i,len=0;

	// pocet lokalnich promennych
	fileIn.Read(&len, sizeof(short));

	// lokalni promenne
	m_aArgsAndLocalVars.SetSize(len);
	for (i=0; i<len; i++)
	{
		m_aArgsAndLocalVars[i].m_stiName=NULL;
		m_aArgsAndLocalVars[i].m_DataType.Load(fileIn, OldStringTable, NewStringTable);
	}

	// kod
	m_Code.Load(fileIn);
}
#endif
  */
//////////////////////////////////////////////////////////////////////
// CPCurrentUnit
//////////////////////////////////////////////////////////////////////

CPCurrentUnit::CPCurrentUnit(CString &strName)
{
	m_strName=strName;
	m_strParent.Empty();
	m_apMembers.InitHashTable(MEMBERS_HASHTABLE_SIZE);
	m_apMethods.InitHashTable(METHODS_HASHTABLE_SIZE);
	m_apEvents.InitHashTable(EVENTS_HASHTABLE_SIZE);

	m_bConstructorDefined=false;
	m_bDestructorDefined=false;

    m_pNext = NULL;
}

CPCurrentUnit::~CPCurrentUnit()
{
	POSITION pos;
	CMethod *pMethod;
	CEvent *pEvent;
	CMember *pMember;
	CString str;

	pos=m_apMethods.GetStartPosition();
	while (pos!=NULL)
	{
		m_apMethods.GetNextAssoc(pos, str, pMethod);
		delete (pMethod);
	}
	m_apMethods.RemoveAll();

	pos=m_apEvents.GetStartPosition();
	while (pos!=NULL)
	{
		m_apEvents.GetNextAssoc(pos, str, pEvent);
		delete (pEvent);
	}
	m_apEvents.RemoveAll();

	pos=m_apMembers.GetStartPosition();
	while (pos!=NULL)
	{
		m_apMembers.GetNextAssoc(pos, str, pMember);
		delete (pMember);
	}
	m_apMembers.RemoveAll();
}


CPCurrentUnit::CMethod* CPCurrentUnit::AddMethod(CString &strName, CType&ReturnDataType)
{
	CMethod *pMethod;

	pMethod=new CMethod(m_StringTable.Add(strName), ReturnDataType);
	m_apMethods.SetAt(strName, pMethod);
	return pMethod;
}

CPCurrentUnit::CEvent* CPCurrentUnit::AddEvent(CString &strName, bool bIsCommand)
{
	CEvent *pEvent;

	pEvent=new CEvent(m_StringTable.Add(strName), bIsCommand);
	m_apEvents.SetAt(strName, pEvent);
	return pEvent;
}

bool CPCurrentUnit::HasMethodOnSelf(CString &strName)
{
	CPCurrentUnit::CMethod* pMethod;
    return m_apMethods.Lookup(strName, pMethod) != NULL ? true : false;
}

bool CPCurrentUnit::GetMethodFromParent(CString &strName, CPNamespace::CUnitMethod **ppParentMethod, 
                                        CStringTableItem **pstiParentName, CPNamespace *pNamespace)
{
    ASSERT( ppParentMethod != NULL);
    ASSERT( pstiParentName != NULL);
    ASSERT( pNamespace != NULL);

    CPNamespace::CUnit *pUnit = m_pParent;
    CPNamespace::CUnitMethod *pParentMethod;

    while ( pUnit != NULL)
    {
        if ( (pParentMethod = pUnit->GetMethodOnSelf( strName)) != NULL)
        {
            *ppParentMethod = pParentMethod;
            *pstiParentName = pUnit->m_stiName->AddRef();
            return true;
        }
        if ( pUnit->m_stiParent == NULL) return false;
        pUnit = pNamespace->GetUnit( (CString)*pUnit->m_stiParent);
    }

    return false;
}

bool CPCurrentUnit::HasEvent(CString &strName)
{
	strName.MakeUpper();

	CPCurrentUnit::CEvent* pEvent;
	if (m_apEvents.Lookup(strName, pEvent)) return true;
	return false;
}

CPCurrentUnit::CMember* CPCurrentUnit::AddMember(CString &strName, CType &DataType)
{
	CMember *pMember;

	pMember=new CMember(m_StringTable.Add(strName), DataType);
	m_apMembers.SetAt(strName, pMember);
	return pMember;
}

bool CPCurrentUnit::HasMember(CString &strName, CPPreview *pPreview)
{
	CPCurrentUnit::CMember* pMember;
	if (m_apMembers.Lookup(strName, pMember)) return true;
    else if (m_pParent!=NULL) return (m_pParent->GetMember(strName, pPreview)!=NULL);
	else return false;
}

bool CPCurrentUnit::GetMemberType(CString &strName, CType&MemberType, CPPreview *pPreview)
{
	CPCurrentUnit::CMember *pMember;

	if (m_apMembers.Lookup(strName, pMember))
	{
		MemberType=pMember->m_DataType;
		return true;
	}
	else if (m_pParent!=NULL)
	{
        return m_pParent->GetMemberType( strName, MemberType, pPreview);
	}
    else return false;
}

void CPCurrentUnit::Save(CCompilerOutput *pOut)
{
	POSITION pos;
	CMember *pMember;
	CMethod *pMethod;
	CEvent *pEvent;
	CString str;

	TRACE_COMPILED_CODE("\nJednotka '%s' (predek '%s'):\n", m_strName, m_strParent);
	// sign
	pOut->PutString(UNITFILE_FIRTSBYTES);

	// jmeno unity
	pOut->PutShort(m_strName.GetLength());
	pOut->PutString(m_strName);

	// jmeno predka
	pOut->PutShort(m_strParent.GetLength());
	pOut->Format(m_strParent);

	// pocet polozek
	pOut->PutShort(m_apMembers.GetCount());
	
	pos=m_apMembers.GetStartPosition();
	while (pos!=NULL)
	{
		m_apMembers.GetNextAssoc(pos, str, pMember);
		pMember->Save(pOut, m_StringTable);
	}

	// constructor
	TRACE_COMPILED_CODE1("\n%s:\n", m_strName);
	m_Constructor.Save(pOut, m_StringTable);

	// destructor
	m_Destructor.Save(pOut, m_StringTable);

	// metody
	pOut->PutShort(m_apMethods.GetCount());
	pos=m_apMethods.GetStartPosition();
	while (pos!=NULL)
	{
		m_apMethods.GetNextAssoc(pos,str,pMethod);
		TRACE_COMPILED_CODE2("\n%s::%s\n", m_strName, str);	
		pMethod->Save(pOut, m_StringTable);
	}

	// eventy
	pOut->PutShort(m_apEvents.GetCount());
	pos=m_apEvents.GetStartPosition();
	while (pos!=NULL)
	{
		m_apEvents.GetNextAssoc(pos, str, pEvent);
		TRACE_COMPILED_CODE2("\n%s::%s\n", m_strName, str);	
		pEvent->Save(pOut, m_StringTable);
	}

    // stringtable
	m_StringTable.Save(pOut);

	// float table
	m_FloatTable.Save(pOut);
}

/*#ifdef _DEBUG_LOAD
bool CPCurrentUnit::Load(CFile &fileIn)
{
	char pom[512];
	short i,j,len,k;

	CMember Member;
	CMethod Method, *pMethod;
	CEvent Event, *pEvent;

	// sign
	fileIn.Read(pom, strlen(UNITFILE_FIRTSBYTES));
	pom[strlen(UNITFILE_FIRTSBYTES)]=0;
	if (strcmp(pom, UNITFILE_FIRTSBYTES)!=0) return false;

	// jmeno unity
	fileIn.Read(&i, sizeof(short));
	fileIn.Read(pom, i);
	pom[i]=0;
	m_strName=pom;

	// jmeno predka
	fileIn.Read(&i, sizeof(short));
	fileIn.Read(pom, i);
	pom[i]=0;
	m_strParent=pom;

	// tabulky
	m_StringTable.Load(fileIn);
	m_FloatTable.Load(fileIn);

	// pocet polozek
	fileIn.Read(&len, sizeof(short));

	for (i=0; i<len; i++)
	{
		Member.Load(fileIn, m_StringTable, m_StringTable);
		AddMember((CString)*Member.m_stiName, Member.m_DataType);
	}

	// konstruktor
	m_Constructor.Load(fileIn, m_StringTable, m_StringTable);

	// destruktor
	m_Destructor.Load(fileIn, m_StringTable, m_StringTable);

	// pocet metod
	fileIn.Read(&len, sizeof(short));

	for (i=0; i<len; i++)
	{
		Method.Load(fileIn, m_StringTable, m_StringTable);
		pMethod=AddMethod((CString)*Method.m_stiName, Method.m_ReturnDataType, m_StringTable);

		pMethod->m_nArgCount=Method.m_nArgCount;
		k=Method.m_aArgsAndLocalVars.GetSize();
		pMethod->m_aArgsAndLocalVars.SetSize(k);
		for (j=0; j<k; j++) pMethod->m_aArgsAndLocalVars[j]=Method.m_aArgsAndLocalVars[j];
		pMethod->m_Code=Method.m_Code;
	}

	// pocet eventu
	fileIn.Read(&len, sizeof(short));

	for (i=0; i<len; i++)
	{
		Event.Load(fileIn, m_StringTable, m_StringTable);
		pEvent=AddEvent((CString)*Event.m_stiName, m_StringTable);

		pEvent->m_nArgCount=Event.m_nArgCount;
		k=Event.m_aArgsAndLocalVars.GetSize();
		pEvent->m_aArgsAndLocalVars.SetSize(k);
		for (j=0; j<k; j++) pEvent->m_aArgsAndLocalVars[j]=Event.m_aArgsAndLocalVars[j];
		pEvent->m_Code=Event.m_Code;
	}

	return true;
}
#endif
*/

void CPCurrentUnit::DebugWrite(CCompilerErrorOutput *pOut)
{
	POSITION pos;
	CMember *pMember;
	CMethod *pMethod;
	CEvent *pEvent;
	CString str;
	int len,i;

	// hlavicka unity
	pOut->Format("\r\n===================================\r\n");
	pOut->Format("unit %s ", m_strName);
	if (!m_strParent.IsEmpty()) pOut->Format(" extends  %s", m_strParent);
	pOut->Format("\r\n===================================\r\n");
	
	// members
	pos=m_apMembers.GetStartPosition();
	if (pos!=NULL) 	pOut->Format("\r\nMembers:\r\n\r\n");
	while (pos!=NULL)
	{
		m_apMembers.GetNextAssoc(pos, str, pMember);
		pOut->Format("%s %s;\r\n", pMember->m_DataType.GetString(), (CString)*pMember->m_cstiName);
	}

	// constructor
	pOut->Format("\r\nConstructor:\r\n");
	len=m_Constructor.m_aArgsAndLocalVars.GetSize();
	for (i=0; i<len; i++)
	{
		pOut->Format("%d:\t%s %s;\r\n", -(i+1), (CString)*m_Constructor.m_aArgsAndLocalVars[i].m_stiName,
			m_Constructor.m_aArgsAndLocalVars[i].m_DataType.GetString());
	}
	pOut->PutString("-----------------\r\n");
	m_Constructor.m_Code.DebugWrite(pOut);

	// destructor
	pOut->Format("\r\nDestructor:\r\n");
	len=m_Destructor.m_aArgsAndLocalVars.GetSize();
	for (i=0; i<len; i++)
	{
		pOut->Format("%d:\t%s %s;\r\n", -(i+1), (CString)*m_Destructor.m_aArgsAndLocalVars[i].m_stiName,
			m_Destructor.m_aArgsAndLocalVars[i].m_DataType.GetString());
	}
	pOut->PutString("-----------------\r\n");
	m_Destructor.m_Code.DebugWrite(pOut);
		
	// methods
	pos=m_apMethods.GetStartPosition();
	if (pos!=NULL) 	pOut->Format("\r\nMethods:\r\n");
	while (pos!=NULL)
	{
		m_apMethods.GetNextAssoc(pos, str, pMethod);
		
		pOut->Format("\r\n>> %s\r\n>> returns %s\r\n", (CString)*pMethod->m_cstiName,
			pMethod->m_ReturnDataType.GetString());
		
		pOut->Format(">> %d arguments\r\n",pMethod->m_nArgCount);

		len=pMethod->m_aArgsAndLocalVars.GetSize();
		for (i=0; i<len; i++)
		{
			if (pMethod->m_aArgsAndLocalVars[i].m_stiName!=NULL)
				pOut->Format("%d:\t%s %s;\r\n", -(i+1), (CString)*pMethod->m_aArgsAndLocalVars[i].m_stiName,
					pMethod->m_aArgsAndLocalVars[i].m_DataType.GetString());
			else pOut->Format("%d:\t%s;\r\n", -(i+1), pMethod->m_aArgsAndLocalVars[i].m_DataType.GetString());
		}
		pOut->PutString("-----------------\r\n");
		pMethod->m_Code.DebugWrite(pOut);
	}
	
	// events
	pos=m_apEvents.GetStartPosition();
	if (pos!=NULL) pOut->Format("\r\nEvents:\r\n");
	while (pos!=NULL)
	{
		m_apEvents.GetNextAssoc(pos, str, pEvent);
		
		pOut->Format("\r\n>> %s\r\n", (CString)*pEvent->m_cstiName);
		pOut->Format(">> %d arguments\r\n",pEvent->m_nArgCount);

		len=pEvent->m_aArgsAndLocalVars.GetSize();
		for (i=0; i<len; i++)
		{
			if (pEvent->m_aArgsAndLocalVars[i].m_stiName!=NULL)
				pOut->Format("%d:\t%s %s\r\n", -(i+1), (CString)*pEvent->m_aArgsAndLocalVars[i].m_stiName,
					pEvent->m_aArgsAndLocalVars[i].m_DataType.GetString());
			else pOut->Format("%d:\t%s\r\n", -(i+1), pEvent->m_aArgsAndLocalVars[i].m_DataType.GetString());
		}
		pOut->PutString("-----------------\r\n");		
		pEvent->m_Code.DebugWrite(pOut);
	}

	// stringtable
	pOut->Format("\r\nStringTable:\r\n");
	m_StringTable.DebugWrite(pOut);

	// float table
	pOut->Format("\r\nFloatTable:\r\n");
	m_FloatTable.DebugWrite(pOut);
}

