/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Prekladac
 *   Autor: Helena Kupková
 *  
 *   Popis: Implementace tridy CPNamespace, ktera obsahuje znama
 *          jmena objektu/promennych/glob.fci, hlavicky eventu atp.
 * 
 ***********************************************************/

#include "stdafx.h"
#include "CompilerKernel.h"
#include "PLexNsp.h"

#include "TripleS\CompilerIO\CompilerIO.h"

#include "GameServer\GameServer\SFileManager.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// Bison - parsovani interface objektu
int nspparse(void *pNamespace);

//////////////////////////////////////////////////////////////////////
// CPNamespace::CSysObjectMember
//////////////////////////////////////////////////////////////////////

CPNamespace::CSysObjectMember::CSysObjectMember(long nID, CStringTableItem *stiName, 
											   CType&DataType)
{
    ASSERT(stiName!=NULL);

    m_nID=nID;
	m_stiName=stiName->AddRef();
	m_DataType=DataType;
}

CPNamespace::CSysObjectMember::~CSysObjectMember()
{
    if (m_stiName!=NULL) m_stiName->Release();
}

//////////////////////////////////////////////////////////////////////
// CPNamespace::CSysObjectMethod
//////////////////////////////////////////////////////////////////////

CPNamespace::CSysObjectMethod::CSysObjectMethod(long nID, CStringTableItem *stiName,
											   CType&ReturnDataType)
{
    ASSERT(stiName!=NULL);

    m_nID=nID;
	m_stiName=stiName->AddRef();
	m_ReturnDataType=ReturnDataType;
	m_aArgs.SetSize(0);
}

CPNamespace::CSysObjectMethod::~CSysObjectMethod()
{
    if (m_stiName!=NULL) m_stiName->Release();
	m_aArgs.RemoveAll();
}

void CPNamespace::CSysObjectMethod::AddArgument(CPNamespace *pNamespace, CString &strType, int nArrayLevel, CPPreview *pPreview)
{
	CType Type;

	Type.Set(strType, nArrayLevel, pNamespace, pPreview);

	m_aArgs.Add(Type);
}


//////////////////////////////////////////////////////////////////////
// CPNamespace::CSysObject
//////////////////////////////////////////////////////////////////////

CPNamespace::CSysObject::CSysObject(CPNamespace *pNamespace, CStringTableItem *stiName, bool bCanCreateInstance)
{
    ASSERT(stiName!=NULL);
    ASSERT(pNamespace!=NULL);

    m_pNamespace = pNamespace;
	
    m_stiName=stiName->AddRef();
	m_bCanCreateInstance=bCanCreateInstance;
	m_apMembers.InitHashTable(MEMBERS_HASHTABLE_SIZE);
	m_apMethods.InitHashTable(METHODS_HASHTABLE_SIZE);
}

CPNamespace::CSysObject::~CSysObject()
{
	POSITION pos;
	CString str;
	CSysObjectMember *pMember;
	CSysObjectMethod *pMethod;

	pos=m_apMembers.GetStartPosition();
	while (pos!=NULL) 
	{
		m_apMembers.GetNextAssoc(pos, str, pMember);
		delete pMember;
	}
	m_apMembers.RemoveAll();

	pos=m_apMethods.GetStartPosition();
	while (pos!=NULL) 
	{
		m_apMethods.GetNextAssoc(pos, str, pMethod);
		delete pMethod;
	}
	m_apMethods.RemoveAll();

    if (m_stiName!=NULL) m_stiName->Release();
}

CPNamespace::CSysObjectMethod* CPNamespace::CSysObject::GetMethodByID(long nID)
{
	CString str;
	CSysObjectMethod *pMethod;
	POSITION pos=m_apMethods.GetStartPosition();

	while (pos!=NULL) 
	{
		m_apMethods.GetNextAssoc(pos, str, pMethod);
		if (pMethod->m_nID==nID) return pMethod;
	}
	return NULL;
}

CPNamespace::CSysObjectMember* CPNamespace::CSysObject::GetMemberByID(long nID)
{
	CString str;
	CSysObjectMember *pMember;
	POSITION pos=m_apMembers.GetStartPosition();

	while (pos!=NULL) 
	{
		m_apMembers.GetNextAssoc(pos, str, pMember);
		if (pMember->m_nID==nID) return pMember;
	}
	return NULL;
}

CPNamespace::CSysObjectMethod* CPNamespace::CSysObject::GetMethodByName(CString &strName)
{
	CSysObjectMethod *pMethod;
	if (m_apMethods.Lookup(strName, pMethod)) return pMethod;
	else return NULL;
}

CPNamespace::CSysObjectMember* CPNamespace::CSysObject::GetMemberByName(CString &strName)
{
	CSysObjectMember *pMember;
	if (m_apMembers.Lookup(strName, pMember)) return pMember;
	else return NULL;
}

CPNamespace::CSysObjectMember* CPNamespace::CSysObject::AddMember(
                        long nID, CString &strName, CString &strType, int nArrayLevel, CPPreview *pPreview)
{
	CType Type;
	CSysObjectMember *pOMember;

	Type.Set(strType, nArrayLevel, m_pNamespace, pPreview);

	pOMember=new CSysObjectMember(nID, g_StringTable.AddItem(strName, false), Type);
	m_apMembers.SetAt(strName, pOMember);
	return pOMember;
}

CPNamespace::CSysObjectMethod* CPNamespace::CSysObject::AddMethod(
                long nID, CString &strName, CString &strReturnType, int nArrayLevel, CPPreview *pPreview)
{
	CType RetType;
	CSysObjectMethod *pOMethod;

	RetType.Set(strReturnType, nArrayLevel, m_pNamespace, pPreview);

	pOMethod=new CSysObjectMethod(nID, g_StringTable.AddItem(strName, false), RetType);
	m_apMethods.SetAt(strName, pOMethod);
	return pOMethod;
}

//////////////////////////////////////////////////////////////////////
// CPNamespace::CUnitMember
//////////////////////////////////////////////////////////////////////

CPNamespace::CUnitMember::CUnitMember(CStringTableItem *stiName, CType&DataType)
{
    ASSERT(stiName!=NULL);

    m_stiName=stiName->AddRef();
	m_DataType=DataType;
}

CPNamespace::CUnitMember::~CUnitMember()
{
    if (m_stiName!=NULL) m_stiName->Release();
}

//////////////////////////////////////////////////////////////////////
// CPNamespace::CUnitMethod
//////////////////////////////////////////////////////////////////////

CPNamespace::CUnitMethod::CUnitMethod(CStringTableItem *stiName, CType&ReturnDataType)
{
    ASSERT(stiName!=NULL);

    m_stiName=stiName->AddRef();
	m_ReturnDataType=ReturnDataType;
	m_aArgs.SetSize(0);
}

CPNamespace::CUnitMethod::CUnitMethod()
{
    m_stiName=NULL;
}

CPNamespace::CUnitMethod::~CUnitMethod()
{
    if (m_stiName!=NULL) m_stiName->Release();
	m_aArgs.RemoveAll();
}

void CPNamespace::CUnitMethod::AddArgument(CPNamespace *pNamespace, CString &strType, int nArrayLevel, CPPreview *pPreview)
{
	CType Type;

	Type.Set(strType, nArrayLevel, pNamespace, pPreview);

	m_aArgs.Add(Type);
}

void CPNamespace::CUnitMethod::AddArgument(CType&DataType)
{
	m_aArgs.Add(DataType);
}

//////////////////////////////////////////////////////////////////////
// CPNamespace::CUnit
//////////////////////////////////////////////////////////////////////

CPNamespace::CUnit::CUnit(CPNamespace *pNamespace, CStringTableItem *stiName, CStringTableItem *stiParent)
{
    ASSERT(stiName!=NULL);
    ASSERT(pNamespace!=NULL);

    m_pNamespace = pNamespace;

	m_stiName=stiName->AddRef();
	m_stiParent=stiParent;
    if (stiParent!=NULL) stiParent->AddRef();

	m_apMembers.InitHashTable(MEMBERS_HASHTABLE_SIZE);
	m_apMethods.InitHashTable(METHODS_HASHTABLE_SIZE);
}

CPNamespace::CUnit::CUnit(CPNamespace *pNamespace)
{
    m_stiName = NULL;
    m_stiParent = NULL;

    ASSERT(pNamespace!=NULL);
    m_pNamespace = pNamespace;
}

CPNamespace::CUnit::~CUnit()
{
	POSITION pos;
	CString str;
	CUnitMember *pMember;
	CUnitMethod *pMethod;

	pos=m_apMembers.GetStartPosition();
	while (pos!=NULL) 
	{
		m_apMembers.GetNextAssoc(pos, str, pMember);
		delete pMember;
	}
	m_apMembers.RemoveAll();

	pos=m_apMethods.GetStartPosition();
	while (pos!=NULL) 
	{
		m_apMethods.GetNextAssoc(pos, str, pMethod);
		delete pMethod;
	}
	m_apMethods.RemoveAll();

    if (m_stiName!=NULL) m_stiName->Release();
    if (m_stiParent!=NULL) m_stiParent->Release();
}

CPNamespace::CUnitMethod* CPNamespace::CUnit::GetMethodOnSelf(CString &strName)
{
    CUnitMethod *pMethod;
	if (m_apMethods.Lookup(strName, pMethod)) return pMethod;
    else return NULL;
}

CPNamespace::CUnitMethod* CPNamespace::CUnit::GetMethod(CString &strName, CPPreview *pPreview)
{
	CUnitMethod *pMethod;
	if (m_apMethods.Lookup(strName, pMethod)) return pMethod;
	else 
	{
		if (m_stiParent==NULL) return NULL;
		else
		{
			CUnit *pUnit=m_pNamespace->GetUnit((CString)*m_stiParent);
			if (pUnit == NULL && pPreview != NULL) pUnit = pPreview->GetUnit((CString)*m_stiParent);
            if (pUnit == NULL)
			{

				ASSERT(false); // to je divne
				return NULL;
			}
			else return pUnit->GetMethod(strName, pPreview);
		}
	}
}

CPNamespace::CUnitMember* CPNamespace::CUnit::GetMember(CString &strName, CPPreview *pPreview)
{
	CUnitMember *pMember;
	if (m_apMembers.Lookup(strName, pMember)) return pMember;
	else 
	{
		if (m_stiParent==NULL) return NULL;
		else
		{
			CUnit *pUnit=m_pNamespace->GetUnit((CString)*m_stiParent);
            if (pUnit == NULL && pPreview != NULL) pUnit = pPreview->GetUnit((CString)*m_stiParent);
			if (pUnit==NULL)
			{
				ASSERT(false); // to je divne
				return NULL;
			}
			else return pUnit->GetMember(strName, pPreview);
		}
	}
}

CPNamespace::CUnitMember* CPNamespace::CUnit::AddMember(CString &strName, CString &strType, 
                                                        int nArrayLevel, CPPreview *pPreview)
{
	CType Type;
	CUnitMember *pUMember;

	Type.Set(strType, nArrayLevel, m_pNamespace, pPreview);

	pUMember=new CUnitMember(g_StringTable.AddItem(strName, false), Type);
	m_apMembers.SetAt(strName, pUMember);
	return pUMember;
}

CPNamespace::CUnitMember* CPNamespace::CUnit::AddMember(CString &strName, CType &DataType)
{
    return AddMember(g_StringTable.AddItem(strName, false), DataType);
}

CPNamespace::CUnitMember* CPNamespace::CUnit::AddMember(CStringTableItem *stiName, CType &DataType)
{
	CUnitMember *pUMember;
	pUMember=new CUnitMember(stiName, DataType);
	m_apMembers.SetAt((CString)*stiName, pUMember);
	return pUMember;
}


CPNamespace::CUnitMethod* CPNamespace::CUnit::AddMethod(CString &strName, CString &strReturnType, int nArrayLevel, CPPreview *pPreview)
{
	CType RetType;
	CUnitMethod *pUMethod;

	if ( !m_apMethods.Lookup( strName, pUMethod))
	{
		RetType.Set(strReturnType, nArrayLevel, m_pNamespace, pPreview);

		pUMethod=new CUnitMethod(g_StringTable.AddItem(strName, false), RetType);
		m_apMethods.SetAt(strName, pUMethod);
	}
	return pUMethod;
}

CPNamespace::CUnitMethod* CPNamespace::CUnit::AddMethod(CString &strName, CType&ReturnDataType)
{
    return AddMethod(g_StringTable.AddItem(strName, false), ReturnDataType);
}

CPNamespace::CUnitMethod* CPNamespace::CUnit::AddMethod(CStringTableItem *stiName, CType& ReturnDataType)
{
	CUnitMethod *pUMethod;

	pUMethod=new CUnitMethod(stiName, ReturnDataType);
	m_apMethods.SetAt((CString)*stiName, pUMethod);
	return pUMethod;
}

bool CPNamespace::CUnit::IsChildOf(CPNamespace::CUnit *pAnotherUnit, CPPreview *pPreview)
{
	if (m_stiParent!=NULL)
	{
        ASSERT(m_pNamespace!=NULL);
		CUnit *pUnit=m_pNamespace->GetUnit((CString)*m_stiParent);
        if ( pUnit == NULL)
        {
            if (pPreview != NULL) pUnit = pPreview->GetUnit( (CString)*m_stiParent);
		    if (pUnit==NULL)
		    {
			    ASSERT(false); // to je divne
			    return NULL;
		    }
        }
		if (pUnit==pAnotherUnit) return true;
		else return pUnit->IsChildOf(pAnotherUnit, pPreview);
	}
	else return false;
}

bool CPNamespace::CUnit::HasMember(CString &strName, CPPreview *pPreview)
{
	CPNamespace::CUnitMember* pMember;
	if (m_apMembers.Lookup(strName, pMember)) return true;
    else 
    {
        if (m_stiParent==NULL) return false;
		else
		{
			CUnit *pUnit=m_pNamespace->GetUnit((CString)*m_stiParent);
			if ( pUnit==NULL) 
            {
                if (pPreview != NULL) pUnit = pPreview->GetUnit( (CString)*m_stiParent);
			    if ( pUnit == NULL)
                {
    				ASSERT(false); // to je divne
	    			return false;
                }
			}
            return pUnit->HasMember(strName, pPreview);
		}
    }
}

bool CPNamespace::CUnit::GetMemberType(CString &strName, CType&MemberType, CPPreview *pPreview)
{
	CPNamespace::CUnitMember *pMember;

	if (m_apMembers.Lookup(strName, pMember))
	{
		MemberType=pMember->m_DataType;
		return true;
	}
	else 
    {
        if (m_stiParent == NULL) return false;
        else 
        {
            CUnit *pUnit=m_pNamespace->GetUnit((CString)*m_stiParent);
            if ( pUnit == NULL)
            {
                if (pPreview != NULL) pUnit = pPreview->GetUnit( (CString)*m_stiParent);
			    if (pUnit==NULL)
			    {
				    ASSERT(false); // to je divne
				    return false;
			    }
            }
            return pUnit->GetMemberType(strName, MemberType, pPreview);
        }
	}
}

//////////////////////////////////////////////////////////////////////
// CPNamespace::CStruct
//////////////////////////////////////////////////////////////////////

CPNamespace::CStruct::CStruct(CPNamespace *pNamespace, CStringTableItem *stiName)
{
    ASSERT(stiName!=NULL);
    ASSERT(pNamespace!=NULL);

    m_pNamespace = pNamespace;
    
    m_stiName=stiName->AddRef();

    m_nMemberCount=0;
}

CPNamespace::CStruct::CStruct()
{
    m_stiName = NULL;
    m_nMemberCount = 0;
}

CPNamespace::CStruct::~CStruct()
{
    int i;
    
    for (i=0; i<m_nMemberCount; i++) m_aMemberNames[i]->Release();

    m_aMemberNames.RemoveAll();
    m_aMemberTypes.RemoveAll();

    if (m_stiName!=NULL) m_stiName->Release();
}

void CPNamespace::CStruct::AddMember(CString &strName, CString &strType, int nArrayLevel, CPPreview *pPreview)
{
	CType Type;

	Type.Set(strType, nArrayLevel, m_pNamespace, pPreview);

	m_aMemberNames.SetAtGrow(m_nMemberCount, g_StringTable.AddItem(strName, true));
    m_aMemberTypes.SetAtGrow(m_nMemberCount, Type);
    m_nMemberCount++;
}

void CPNamespace::CStruct::AddMember(CString &strName, CType&DataType)
{
	m_aMemberNames.SetAtGrow(m_nMemberCount, g_StringTable.AddItem(strName, true));
    m_aMemberTypes.SetAtGrow(m_nMemberCount, DataType);
    m_nMemberCount++;
}

bool CPNamespace::CStruct::HasMember(CString &strName)
{
    CStringTableItem *stiName=g_StringTable.AddItem(strName);
    int i;
    for (i=0; i<m_nMemberCount; i++) 
    {
        if (m_aMemberNames.GetAt(i)==stiName) 
        {
            stiName->Release();
            return true;
        }
    }
    stiName->Release();
    return false;
}

bool CPNamespace::CStruct::GetMemberType(CString &strName, CType&MemberType)
{
    CStringTableItem *stiName=g_StringTable.AddItem(strName);
    int i;
    for (i=0; i<m_nMemberCount; i++) 
    {
        if (m_aMemberNames.GetAt(i)==stiName) 
        {
            MemberType=m_aMemberTypes[i];
            stiName->Release();
            return true;
        }
    }
    stiName->Release();
    return false;
}

//////////////////////////////////////////////////////////////////////
// CPNamespace::CEventHeader
//////////////////////////////////////////////////////////////////////

CPNamespace::CEventHeader::CEventHeader(CStringTableItem *stiName)
{
    ASSERT(stiName!=NULL);
	m_stiName=stiName->AddRef();
    m_aArgs.SetSize(0,1);
}

CPNamespace::CEventHeader::~CEventHeader()
{
    if (m_stiName!=NULL) m_stiName->Release();
}

void CPNamespace::CEventHeader::AddArgument(CPNamespace *pNamespace, CString &strType, int nArrayLevel, CPPreview *pPreview)
{
	CType Type;

	Type.Set(strType, nArrayLevel, pNamespace, pPreview);

	m_aArgs.Add(Type);
}

CString CPNamespace::CEventHeader::GetHeaderString()
{
	CString str=(CString)*m_stiName+" (";
	int i,len=m_aArgs.GetSize();
	
	for (i=0; i<len; i++)
	{
		str+=m_aArgs[i].GetString();
		if (i+1!=len) str+=", ";
	}
	str+=")";

	return str;
}

//////////////////////////////////////////////////////////////////////
// CPNamespace::CVariable
//////////////////////////////////////////////////////////////////////

CPNamespace::CVariable::CVariable(CStringTableItem *stiName, CType&DataType, REG nReg, int nLevel, bool bArgument/*=false*/)
{
    ASSERT(stiName!=NULL);

    m_stiName=stiName->AddRef();
	m_DataType=DataType;
	m_nLevel=nLevel;
	m_bArgument=bArgument;
	m_nReg=nReg;

	m_nInitialized=false;
}

CPNamespace::CVariable::~CVariable()
{
    if (m_stiName!=NULL) m_stiName->Release();                            
}

//////////////////////////////////////////////////////////////////////
// CPNamespace::CUnitObjsType
//////////////////////////////////////////////////////////////////////

CPNamespace::CUnitObjsType::CUnitObjsType(EScriptType nType, void *pData)
{
	m_nType=nType;
	switch (nType)
    {
    case SCRIPT_UNIT:
        m_pUnit=(CPNamespace::CUnit*)pData;
        break;
    case SCRIPT_OBJECT:
        m_pSysObj=(CPNamespace::CSysObject*)pData;
        break;
    case SCRIPT_STRUCTURE:
        m_pStruct=(CPNamespace::CStruct*)pData;
        break;
    }
}

CPNamespace::CUnitObjsType::~CUnitObjsType()
{
	switch (m_nType)
    {
    case SCRIPT_UNIT: delete m_pUnit; break;
    case SCRIPT_OBJECT: delete m_pSysObj; break;
    case SCRIPT_STRUCTURE: delete m_pStruct; break;
    default: ASSERT(false);
    }
}

//////////////////////////////////////////////////////////////////////
// CPNamespace
//////////////////////////////////////////////////////////////////////

CPNamespace::CPNamespace()
{
    m_pDefaultUnit=NULL;

	m_pCurrentObject=NULL;
	m_pCurrentObjectMethod=NULL;
	m_pCurrentEventHeader=NULL;
	m_pCurrentFunction=NULL;

	m_apVariables.InitHashTable(VAR_HASHTABLE_SIZE);
	m_apUnitObjsTypes.InitHashTable(TYPES_HASHTABLE_SIZE);
	m_apEventHeaders.InitHashTable(EVENTS_HASHTABLE_SIZE);
	m_apGlobalFunctions.InitHashTable(GLOBALFUNCTIONS_HASHTABLE_SIZE);
	m_apSysGlobalFunctions.InitHashTable(SYSTEMGLOBALFUNCTIONS_HASHTABLE_SIZE);

    m_pLexNsp = NULL;
    m_pCompilerKrnl = NULL;
}

CPNamespace::~CPNamespace()
{
	RemoveAll();
}

bool CPNamespace::Create(CCompilerKernel *pCompilerKrnl)
{
    m_pCompilerKrnl = pCompilerKrnl;

	if (!ReadNamespace()) 
	{
		RemoveAll();
		return false;
	}

    CString strDefaultUnit = DEFAULT_UNIT_PARENT;

    if (!LoadUnitType(strDefaultUnit))
    {
        throw new CStringException("Cannot load default unit '"DEFAULT_UNIT_PARENT"' from the Code Manager.");
    }
    else 
    {
        m_pDefaultUnit = GetUnit(strDefaultUnit);
        ASSERT(m_pDefaultUnit != NULL);
    }

	return true;
}

void CPNamespace::Delete()
{
	RemoveAll();
}

void CPNamespace::RemoveAll()
{
	POSITION pos;
	CString str;
	CVariable *pVar;
	CUnitObjsType *pUOType;
	CEventHeader *pEH;
	CGlobalFunction *pGF;
	CSysGlobalFunction *pSGF;

	pos=m_apVariables.GetStartPosition();
	while (pos!=NULL)
	{
		m_apVariables.GetNextAssoc(pos, str, pVar);
		delete pVar;
	}
	m_apVariables.RemoveAll();

	pos=m_apUnitObjsTypes.GetStartPosition();
	while (pos!=NULL)
	{
		m_apUnitObjsTypes.GetNextAssoc(pos, str, pUOType);
		delete pUOType;
	}
	m_apUnitObjsTypes.RemoveAll();

	pos=m_apEventHeaders.GetStartPosition();
	while (pos!=NULL)
	{
		m_apEventHeaders.GetNextAssoc(pos, str, pEH);
		delete pEH;
	}
	m_apEventHeaders.RemoveAll();

	pos=m_apGlobalFunctions.GetStartPosition();
	while (pos!=NULL)
	{
		m_apGlobalFunctions.GetNextAssoc(pos, str, pGF);
		delete pGF;
	}
	m_apGlobalFunctions.RemoveAll();

	pos=m_apSysGlobalFunctions.GetStartPosition();
	while (pos!=NULL)
	{
		m_apSysGlobalFunctions.GetNextAssoc(pos, str, pSGF);
		delete pSGF;
	}
	m_apSysGlobalFunctions.RemoveAll();
}

void CPNamespace::RemoveVariablesToLevel(int nLevel)
{
	POSITION pos;
	CString str;
	CVariable *pVar;

	pos=m_apVariables.GetStartPosition();
	while (pos!=NULL)
	{
		m_apVariables.GetNextAssoc(pos, str, pVar);
		if (pVar->m_nLevel>nLevel)
		{
			m_apVariables.RemoveKey(str);
			delete pVar;
		}
	}
}

bool CPNamespace::ReadNamespace()
{
    CArchiveFile fileNsp;
    int ret;

    // tohle muze hodit vyjimku
    try
    {
        fileNsp = g_cSFileManager.GetFile(NAMESPACE_FILE);
    }
    catch (CException *e)
    {
		m_pCompilerKrnl->m_pStdErr->Format(NAMESPACE_FILE": fatal error: can't open file\n");
        e->Delete();
		return false;
	}

    m_pLexNsp = new CPLexNsp(&fileNsp);

	ret=nspparse(this);

    delete m_pLexNsp;
    m_pLexNsp = NULL;
	
	fileNsp.Close();

	return !ret;
}

#define _helper_Replace(str, strnew)    if (strstr(errstr,str)==errstr) \
                                {  \
                                    strErrOut+=strnew;      \
                                    errstr+=strlen(str); \
                                }   

void CPNamespace::ParserError(char *errstr)
{
    CString strErrOut;

/* Error messages produced by bison: 

  (1)	"parse error"
  (2)	"parse error, expecting `'st'' or `'st''"  (nekdy jen jednoduche uvozovky)
  (3)	"syntax error: cannot back up"
  (4)	"parser stack overflow"
  (5)	"parse error; also virtual memory exceeded"
*/

    if (strstr(errstr,"parse error")==errstr) // messages (1),(2) and (5)
	{
		if (strlen(errstr)==11)  // length of "parse error" => message (1)
		{
			if (m_pCompilerKrnl->m_pStdIn->GetLastChar()==EOF)
			{
				strErrOut="unexpected end of file";
			}
			else 
            {
                strErrOut.Format("syntax error: '%s'",m_pLexNsp->YYText());
            }
		}
		else
		{
			if (errstr[11]==',') // message (2)
			{
				if (strstr(errstr,"`$'")) strErrOut.Format("syntax error: '%s'",m_pLexNsp->YYText());
				else
				{
					errstr+=23;  // "expecting "
	
					strErrOut="syntax error: missing ";
					
					while (errstr[0])
					{
						switch (errstr[0])
						{
						case '`': 
						case '\'': 
							if (errstr[1]!='\'') strErrOut+="'"; 
							errstr++; 
							break;
						default: 
							switch (errstr[0])
							{
                            case 'N': _helper_Replace("NSP_IDENTIFIER","identifier"); 
									  _helper_Replace("NSP_OBJECT","object");
                                      _helper_Replace("NSP_NUMBER","number");
                                      break;
                            default:
                                // neco jineho
								char *s;
    							if ((s=strchr(errstr,'\''))!=NULL)
								{
									strErrOut+=CString(errstr).Left(s-errstr);
									errstr=s;
								}
								else
								{
									strErrOut+=errstr;
									errstr[0]=0;
								}
							}
							break;
						}
					}

					strErrOut+=" before '";
					if (m_pLexNsp->m_cLastChar!=EOF) strErrOut+=m_pLexNsp->YYText();
                    else strErrOut+="end of file";
					strErrOut+="'";
				}
			}
			else {
				m_pCompilerKrnl->m_pStdErr->Format(NAMESPACE_FILE"(%d) : %s\r\n",
					m_pLexNsp->GetLineNo(), errstr); // message (5);
				return;
			}
		}

		m_pCompilerKrnl->m_pStdErr->Format(NAMESPACE_FILE"(%d) : %s\r\n",
			m_pLexNsp->GetLineNo(), strErrOut); // message (5);
	}
	else m_pCompilerKrnl->m_pStdErr->Format(NAMESPACE_FILE"(%d) : %s\r\n",
			m_pLexNsp->GetLineNo(), errstr); // message (3), (4) or some other one;
}

#undef _helper_Replace

CPNamespace::CVariable* CPNamespace::GetVariable(CString &strName)
{
	CVariable *pVar;
	if (m_apVariables.Lookup(strName, pVar)) return pVar;
	else return NULL;
}

CPNamespace::CVariable* CPNamespace::AddVariable(CStringTableItem *stiName, CType&DataType, 
											   REG nReg, int nLevel, bool bArgument/*=false*/)
{
	CVariable *pVar=new CVariable(stiName, DataType, nReg, nLevel, bArgument);
	m_apVariables.SetAt(*stiName, pVar);
	return pVar;
}

CPNamespace::CUnitObjsType* CPNamespace::GetType(CString &strName)
{
	CUnitObjsType* pUOType;
	if (m_apUnitObjsTypes.Lookup(strName, pUOType)) return pUOType;
	else return NULL;
}

CPNamespace::CUnitObjsType* CPNamespace::AddType(EScriptType nType, void *pData)
{
	CUnitObjsType* pUOType=new CUnitObjsType(nType, pData);
	if (nType==SCRIPT_UNIT) 	m_apUnitObjsTypes.SetAt(*((CUnit*)pData)->m_stiName, pUOType);
	else if (nType==SCRIPT_OBJECT) m_apUnitObjsTypes.SetAt(*((CSysObject*)pData)->m_stiName, pUOType);
    else if (nType==SCRIPT_STRUCTURE) m_apUnitObjsTypes.SetAt(*((CStruct*)pData)->m_stiName, pUOType);
    else ASSERT(false);
	return pUOType;
}

CPNamespace::CEventHeader* CPNamespace::GetEventHeader(CString &strName)
{
	CEventHeader *pEH;
	if (m_apEventHeaders.Lookup(strName, pEH)) return pEH;
	else return NULL;
}

CPNamespace::CUnit* CPNamespace::AddUnit(CString &strName, CString &strParent)
{
	CUnit *pUnit;

    ASSERT(m_pDefaultUnit!=NULL);
	
	if (strParent.IsEmpty()) pUnit=new CUnit(this, g_StringTable.AddItem(strName, false), 
        m_pDefaultUnit->m_stiName);
	else pUnit=new CUnit(this, g_StringTable.AddItem(strName, false), 
        g_StringTable.AddItem(strParent, false));

	AddType(SCRIPT_UNIT, pUnit);
	return pUnit;
}

CPNamespace::CUnit* CPNamespace::GetUnit(CString &strName)
{
	CUnitObjsType* pUOType=GetType(strName);
	if (pUOType==NULL) return NULL;
	if (pUOType->m_nType==SCRIPT_UNIT) return pUOType->m_pUnit;
	else return NULL;
}

CPNamespace::CSysObject* CPNamespace::AddObject(CString &strName, bool bCanCreateInstance)
{
    ASSERT( GetSOTypeFromString(strName) != SO_UNKNOWN);

    CSysObject *pObject=new CSysObject(this, g_StringTable.AddItem(strName, false), bCanCreateInstance);
    return AddType(SCRIPT_OBJECT, pObject)->m_pSysObj;
}

CPNamespace::CSysObject* CPNamespace::GetObject(ESystemObjectType nSOType)
{
    ASSERT( nSOType != SO_NONE && nSOType != SO_UNKNOWN); 

    CUnitObjsType* pUOType=GetType( CString(GetStringFromSOType(nSOType)) );
	if (pUOType!=NULL && pUOType->m_nType==SCRIPT_OBJECT) return pUOType->m_pSysObj;
	else return NULL;
}

CPNamespace::CSysObject* CPNamespace::GetObject(CString &strName)
{
    CUnitObjsType* pUOType=GetType( strName );
	if (pUOType!=NULL && pUOType->m_nType==SCRIPT_OBJECT) return pUOType->m_pSysObj;
	else return NULL;
}

void CPNamespace::RemoveType(CString &strName)
{
	CUnitObjsType *pUOType;
	
	if (m_apUnitObjsTypes.Lookup(strName, pUOType)) 
	{
		m_apUnitObjsTypes.RemoveKey(strName);
		delete pUOType;
	}
}

void CPNamespace::RemoveGlobalFunction(CString &strName)
{
	CGlobalFunction *pFunc;

	if (m_apGlobalFunctions.Lookup(strName, pFunc))
	{
		m_apGlobalFunctions.RemoveKey(strName);
		delete pFunc;
	}
}

CPNamespace::CGlobalFunction* CPNamespace::AddGlobalFunction(CString &strName, CType&ReturnDataType)
{
	CGlobalFunction *pFunc=new CGlobalFunction(g_StringTable.AddItem(strName, false), 
        ReturnDataType);
	m_apGlobalFunctions.SetAt(strName, pFunc);
	return pFunc;
}

CPNamespace::CGlobalFunction* CPNamespace::GetGlobalFunction(CString &strName)
{
	CGlobalFunction *pFunc;
	if (m_apGlobalFunctions.Lookup(strName, pFunc)) return pFunc;
	else return NULL;
}

CPNamespace::CSysGlobalFunction* CPNamespace::AddSysGlobalFunction(long nID, CString &strName, CString &strReturnDataType, int nArrayLevel, CPPreview *pPreview)
{
	CType RetType;
	CSysGlobalFunction *pFunc;

	RetType.Set(strReturnDataType, nArrayLevel, this, pPreview);

	pFunc=new CSysGlobalFunction(nID, g_StringTable.AddItem(strName, false), RetType);
	m_apSysGlobalFunctions.SetAt(strName, pFunc);
	return pFunc;
}

CPNamespace::CSysGlobalFunction* CPNamespace::GetSysGlobalFunctionByName(CString &strName)
{
	CSysGlobalFunction *pFunc;
	if (m_apSysGlobalFunctions.Lookup(strName, pFunc)) return pFunc;
	else return NULL;
}

CPNamespace::CSysGlobalFunction* CPNamespace::GetSysGlobalFunctionByID(long nID)
{
	CString str;
	CSysGlobalFunction *pFunc;
	POSITION pos=m_apSysGlobalFunctions.GetStartPosition();

	while (pos!=NULL) 
	{
		m_apSysGlobalFunctions.GetNextAssoc(pos, str, pFunc);
		if (pFunc->m_nID==nID) return pFunc;
	}
	return NULL;
}

CPNamespace::CStruct* CPNamespace::AddStruct(CString &strName)
{
    CStruct *pStruct;

	pStruct=new CStruct(this, g_StringTable.AddItem(strName, false));

	AddType(SCRIPT_STRUCTURE, pStruct);
	return pStruct;
}

CPNamespace::CStruct* CPNamespace::GetStruct(CString &strName)
{
	CUnitObjsType* pUOType=GetType(strName);
	if (pUOType==NULL) return NULL;
	if (pUOType->m_nType==SCRIPT_STRUCTURE) return pUOType->m_pStruct;
	else return NULL;
    
}

// DEBUGOVACI VYPIS
void CPNamespace::DebugWrite(CCompilerErrorOutput *pOut)
{
	long j;
    int i;
	POSITION pos, pos2;
	CString str;
	CVariable *pVar;
	CUnitObjsType *pUOType;

	CUnitMember *pUMember;
	CUnitMethod *pUMethod;
	CSysObjectMember *pOMember;
	CSysObjectMethod *pOMethod;
	CEventHeader *pEH;
	CSysGlobalFunction *pSGF;

	pOut->Format("\r\nNAMESPACE - TYPES:\r\n==================\r\n\r\n");

	pos=m_apUnitObjsTypes.GetStartPosition();
	while (pos!=NULL)
	{
		m_apUnitObjsTypes.GetNextAssoc(pos, str, pUOType);
		
		if (pUOType->m_nType==SCRIPT_UNIT) 
		{
			pOut->Format("unit %s", (CString)*pUOType->m_pUnit->m_stiName);

			if (pUOType->m_pUnit->m_stiParent!=NULL)
				 pOut->Format("  extends %s",(CString)*pUOType->m_pUnit->m_stiParent);
			pOut->Format("\r\n{\r\n");

			pos2=pUOType->m_pUnit->m_apMembers.GetStartPosition();
			while (pos2!=NULL)
			{
				pUOType->m_pUnit->m_apMembers.GetNextAssoc(pos2, str, pUMember);
				pOut->Format("\t%s %s;\r\n", pUMember->m_DataType.GetString(), (CString)*pUMember->m_stiName);
			}

			pos2=pUOType->m_pUnit->m_apMethods.GetStartPosition();
			while (pos2!=NULL)
			{
				pUOType->m_pUnit->m_apMethods.GetNextAssoc(pos2, str, pUMethod);
				pOut->Format("\t%s %s (", pUMethod->m_ReturnDataType.GetString(), 
					(CString)*pUMethod->m_stiName);

				for (j=0; j<pUMethod->m_aArgs.GetSize(); j++)
				{
					pOut->Format("%s", pUMethod->m_aArgs[j].GetString());
					if (j!=pUMethod->m_aArgs.GetSize()-1) pOut->Format(", ");
				}

				pOut->Format(");\r\n");
			}
			pOut->Format("}\r\n\r\n");
		}
		else if (pUOType->m_nType==SCRIPT_OBJECT)
		{
			pOut->Format("object %s\r\n{\r\n", (CString)*pUOType->m_pSysObj->m_stiName);

			pos2=pUOType->m_pSysObj->m_apMembers.GetStartPosition();
			while (pos2!=NULL)
			{
				pUOType->m_pSysObj->m_apMembers.GetNextAssoc(pos2, str, pOMember);
				pOut->Format("\t%2d: %s %s;\r\n", pOMember->m_nID, pOMember->m_DataType.GetString(), (CString)*pOMember->m_stiName);
			}

			pos2=pUOType->m_pSysObj->m_apMethods.GetStartPosition();
			while (pos2!=NULL)
			{
				pUOType->m_pSysObj->m_apMethods.GetNextAssoc(pos2, str, pOMethod);
				pOut->Format("\t%2d: %s %s (", pOMethod->m_nID, pOMethod->m_ReturnDataType.GetString(), 
					(CString)*pOMethod->m_stiName);

				for (j=0; j<pOMethod->m_aArgs.GetSize(); j++)
				{
					pOut->Format("%s", pOMethod->m_aArgs[j].GetString());
					if (j!=pOMethod->m_aArgs.GetSize()-1) pOut->Format(", ");
				}

				pOut->Format(");\r\n");
			}

			pOut->Format("}\r\n\r\n");
		}
        else if (pUOType->m_nType==SCRIPT_STRUCTURE)
        {
			pOut->Format("struct %s\r\n{\r\n", (CString)*pUOType->m_pStruct->m_stiName);

			for (i=0; i<pUOType->m_pStruct->m_nMemberCount; i++)
            {
                pOut->Format("%s %s;\r\n", pUOType->m_pStruct->m_aMemberTypes[i].GetString(), 
                    (CString)*pUOType->m_pStruct->m_aMemberNames[i]);
            }

			pOut->Format("}\r\n\r\n");
        }
        else ASSERT(false);
	}

	pOut->Format("\r\nNAMESPACE - EVENT HEADERS:\r\n==========================\r\n\r\n");

	pos=m_apEventHeaders.GetStartPosition();
	while (pos!=NULL)
	{
		m_apEventHeaders.GetNextAssoc(pos, str, pEH);

		pOut->Format("event %s (", (CString)*pEH->m_stiName);

		for (j=0; j<pEH->m_aArgs.GetSize(); j++)
		{
			pOut->Format("%s", pEH->m_aArgs[j].GetString());
			if (j!=pEH->m_aArgs.GetSize()-1) pOut->Format(", ");
		}

		pOut->Format(");\r\n");
	}

	pOut->Format("\r\nNAMESPACE - SYSTEM GLOBAL FUNCTIONS:\r\n==========================\r\n\r\n");

	pos=m_apSysGlobalFunctions.GetStartPosition();
	while (pos!=NULL)
	{
		m_apSysGlobalFunctions.GetNextAssoc(pos, str, pSGF);

		pOut->Format("%2d: %s %s (", pSGF->m_nID, pSGF->m_ReturnDataType.GetString(), (CString)*pSGF->m_stiName);

		for (j=0; j<pSGF->m_aArgs.GetSize(); j++)
		{
			pOut->Format("%s", pSGF->m_aArgs[j].GetString());
			if (j!=pSGF->m_aArgs.GetSize()-1) pOut->Format(", ");
		}

		pOut->Format(");\r\n");
	}


	pOut->Format("\r\n\r\nNAMESPACE - VARIABLES:\r\n======================\r\n\r\n");

	pos=m_apVariables.GetStartPosition();
	while (pos!=NULL)
	{
		m_apVariables.GetNextAssoc(pos, str, pVar);
		
		pOut->Format("%d%s: %s %s; \r\n", pVar->m_nLevel, pVar->m_bArgument ? "(arg)" : "",
			pVar->m_DataType.GetString(), (CString)*pVar->m_stiName);
	}
}

bool CPNamespace::LoadUnitOrStructType(CType &Type)
{
    CStruct *pStruct = NULL;
    CUnitObjsType *pUOType = NULL;
    CUnit *pUnit = NULL;

    try
    {
        switch (m_pCompilerKrnl->m_pCodeManagerForCompiler->GetScriptType(Type.m_stiName))
        {
        case SCRIPT_UNIT:
			if (Type.m_nType != T_UNIT) return false;

            pUnit = new CUnit(this);
            pUOType = new CUnitObjsType(SCRIPT_UNIT, pUnit);

            if (m_pCompilerKrnl->m_pCodeManagerForCompiler->GetUnitTypeForCompiler(Type.m_stiName, pUnit))
            {
                m_apUnitObjsTypes.SetAt((CString)*Type.m_stiName, pUOType);
                return true;
            }
            else 
            {
                delete pUOType;
                return false;
            }

        case SCRIPT_STRUCTURE:
			if (Type.m_nType != T_STRUCT) return false;

            pStruct = new CStruct();
            pUOType = new CUnitObjsType(SCRIPT_STRUCTURE, pStruct);

            if (m_pCompilerKrnl->m_pCodeManagerForCompiler->GetStructureTypeForCompiler(Type.m_stiName, pStruct))
            {
                m_apUnitObjsTypes.SetAt((CString)*Type.m_stiName, pUOType);
                Type.m_nType=T_STRUCT;
                return true;
            }
            else
            {
                delete pUOType;
                return false;
            }

        default:
            return false;
        }
    }
    catch (CException *e)
    {
        char str[256];
        e->GetErrorMessage(str, 256);
        m_pCompilerKrnl->ErrorMessage("following error occured while trying to load "
            "unit or structure '%s':", (CString)*Type.m_stiName);
        m_pCompilerKrnl->ErrorMessage(str);

        delete pUOType;

        return false;
    }
}

bool CPNamespace::LoadUnitType(CString &strName)
{
    CStruct *pStruct = NULL;
    CUnitObjsType *pUOType = NULL;
    CUnit *pUnit = NULL;
    CStringTableItem *pSTI = g_StringTable.AddItem(strName);

    if (m_pCompilerKrnl->m_pCodeManagerForCompiler->GetScriptType(pSTI)==SCRIPT_UNIT)
    {
        try
        {
            pUnit = new CUnit(this);
            pUOType = new CUnitObjsType(SCRIPT_UNIT, pUnit);

            if (m_pCompilerKrnl->m_pCodeManagerForCompiler->GetUnitTypeForCompiler(pSTI, pUnit))
            {
                m_apUnitObjsTypes.SetAt((CString)*pSTI, pUOType);
                pSTI->Release();

                if (pUnit->m_stiParent != NULL && 
                    GetUnit((CString)*pUnit->m_stiParent) == NULL)
                {
                    if (!LoadUnitType((CString)*pUnit->m_stiParent))
                    {
                        m_pCompilerKrnl->ErrorMessage("can't load unit '%s', parent of '%s'",
                            pUnit->m_stiParent, pUnit->m_stiName);
                    }
                    else
                    {
                        // neco tu hnije
                        ASSERT(pUnit!=NULL);
                    }
                    
                }


                return true;
            }
            else
            {
                pSTI->Release();
                delete pUOType;
                return false;
            }
        }
        catch (CException *e)
        {
            char str[256];
            e->GetErrorMessage(str, 256);
            m_pCompilerKrnl->ErrorMessage("following error occured while trying to load "
                "unit '%s':", (CString)*pSTI);
            m_pCompilerKrnl->ErrorMessage(str);

            delete pUOType;
        }
    }

    pSTI->Release();
    return false;
}

bool CPNamespace::LoadGlobalFunctionType(CString &strName)
{
    CStringTableItem *pSTI = g_StringTable.AddItem(strName);
    CString strFilename;

    if (m_pCompilerKrnl->m_pCodeManagerForCompiler->GetScriptType(pSTI) == SCRIPT_UNKNOWN)
    {
        pSTI->Release();
        return false;
    }
    else
    {
        CPNamespace::CGlobalFunction *pFunc = new CGlobalFunction();

        try
        {
            if (m_pCompilerKrnl->m_pCodeManagerForCompiler->GetGlobalFunctionTypeForCompiler(pSTI, pFunc))
            {
                m_apGlobalFunctions.SetAt(strName, pFunc);
                pSTI->Release();
                return true;
            }
            else 
            {
                delete pFunc;
                pSTI->Release();
                return false;
            }
        }
        catch (CException *e)
        {
            char str[256];
            e->GetErrorMessage(str, 256);
            m_pCompilerKrnl->ErrorMessage("following error occured while trying to load header "
                "of global function '%s':", strName);
            m_pCompilerKrnl->ErrorMessage(str);
        }
    }

    return false;
}

EScriptType CPNamespace::GetScriptType(CString &strName)
{
    CUnitObjsType *pUOType;
    EScriptType ScriptType;

    // znamy typ objektu/unity/struktury
	if ((pUOType=GetType(strName))!=NULL)
	{
		return pUOType->m_nType;
	}
    // glob. funkce
    else if (GetGlobalFunction(strName)!=NULL)
    {
        return SCRIPT_FUNCTION;
    }
    // systemove glob.funkce
    else if (GetSysGlobalFunctionByName(strName)!=NULL)
    {
        return SCRIPT_SYSFUNCTION;        
    }
    // kompiler nezna -> dotaz na codemanagera
    else 
    {
        CStringTableItem *pSTI = g_StringTable.AddItem(strName);
        ScriptType = m_pCompilerKrnl->m_pCodeManagerForCompiler->GetScriptType(pSTI);
        pSTI->Release();
        return ScriptType;
    }
}

////////////////////////////////////////////////////////////////
// ACTIONS

// ObjectAndEventInterface:	  NSP_OBJECT NSP_IDENTIFIER		 { if (!m_pCompilerKrnl->m_Namespace.OnObjectBegin($2)) YYERROR;	}
//							   '{' MembersAndMethodS '}'	 { m_pCompilerKrnl->m_Namespace.OnObjectEnd();		}
//							;
bool CPNamespace::OnObjectBegin(CString &strName)
{
	if (GetType(strName)!=NULL)
	{
		ParserError("error: entity of that name already exists");
		return false;
	}

	m_pCurrentObject=AddObject(strName, true);

	return true;
}

void CPNamespace::OnObjectEnd()
{
	m_pCurrentObject=NULL;
}

//ObjectAndEventInterface:	  NSP_EVENT NSP_IDENTIFIER	 { if (!m_pCompilerKrnl->m_Namespace.OnEventBegin($2)) YYERROR:	}
//								'(' ArgList_opt ')' ';'	 { m_pCompilerKrnl->m_Namespace.OnEventEnd();		}
//							;

bool CPNamespace::OnEventBegin(CString &strName)
{
	if (GetEventHeader(strName)!=NULL)
	{
		ParserError("error: header for this event already exists");
		return false;
	}

	m_pCurrentEventHeader=new CEventHeader(g_StringTable.AddItem(strName, false));
	m_apEventHeaders.SetAt(strName, m_pCurrentEventHeader);

	return true;
}

void CPNamespace::OnEventEnd()
{
	m_pCurrentEventHeader=NULL;
}

//ObjectMemberOrMethod:		  NSP_NUMBER ':' NSP_IDENTIFIER ArrayOrSetDecl_opt NSP_IDENTIFIER ArrayOrSetDecl_opt ';'
//	    					    {	if (!((CPNamespace*)pNamespace)->OnObjectMember($1.m_nNumber, $3.m_strString, $4.m_nNumber, $5.m_strString, $6.m_nNumber)) YYERROR;	}
						
// nTypeArrayLevel je jen kvuli spravnemu parsovani, musi byt = 0, jinak syntax error
bool CPNamespace::OnObjectMember(long nID, CString &strType, int nTypeArrayLevel, CString &strName, 
						  int nArrayLevel)
{
	CType Type;

    ASSERT(m_pCurrentObject!=NULL);

    if (nTypeArrayLevel != 0)
    {
        ParserError("syntax error");
        return false;
    }

	if (m_pCurrentObject!=NULL && (m_pCurrentObject->GetMemberByID(nID)!=NULL || 
        m_pCurrentObject->GetMethodByID(nID)!=NULL))
	{
		ParserError("error: duplicate ID");
		return false;
	}

	if (m_pCurrentObject!=NULL && (m_pCurrentObject->GetMemberByName(strName)!=NULL || 
            m_pCurrentObject->GetMethodByName(strName)!=NULL))
	{
		ParserError("error: member or method of that name already exists");
		return false;
	}

    m_pCurrentObject->AddMember(nID, strName, strType, nArrayLevel, NULL);

	return true;
}

//ObjectMemberOrMethod:		| NSP_NUMBER ':' NSP_IDENTIFIER ArrayOrSetDecl_opt NSP_IDENTIFIER '('	{	if (!((CPNamespace*)pNamespace)->OnObjectMethodBegin($1.m_nNumber, $3.m_strString, $4.m_nNumber, $5.m_strString)) YYERROR;	}
//					        	ArgList_opt ')' ';'								{	((CPNamespace*)pNamespace)->OnObjectMethodEnd();					}
bool CPNamespace::OnObjectMethodBegin(long nID, CString &strRetType, int nArrayLevel, CString &strName)
{
	CType Type;

    ASSERT(m_pCurrentObject!=NULL);

	if (m_pCurrentObject!=NULL && (m_pCurrentObject->GetMemberByID(nID)!=NULL || 
        m_pCurrentObject->GetMethodByID(nID)!=NULL))
	{
		ParserError("error: duplicate ID");
		return false;
	}

	if (m_pCurrentObject!=NULL && (m_pCurrentObject->GetMemberByName(strName)!=NULL || 
		m_pCurrentObject->GetMethodByName(strName)!=NULL))
	{
		ParserError("error: member or method of that name already exists");
		return false;
	}

    m_pCurrentObjectMethod=m_pCurrentObject->AddMethod(nID, strName, strRetType, nArrayLevel, NULL);

	return true;
}

void CPNamespace::OnObjectMethodEnd()
{
	m_pCurrentObjectMethod=NULL;
}


//Function:	NSP_NUMBER ':' NSP_IDENTIFIER ArrayOrSetDecl_opt NSP_IDENTIFIER '('	{	if (!((CPNamespace*)pNamespace)->OnFunctionBegin($1.m_nNumber, $3.m_strString, $4.m_nNumber, $5.m_strString)) YYERROR;	}
//						ArgList_opt ')' ';'						{	((CPNamespace*)pNamespace)->OnFunctionEnd();					}
//					;
bool CPNamespace::OnFunctionBegin(long nID, CString &strRetType, int nArrayLevel, CString &strName)
{
    if (GetSysGlobalFunctionByID(nID)!=NULL)
	{
		ParserError("error: duplicate ID");
		return false;
	}

	if (GetSysGlobalFunctionByName(strName)!=NULL)
	{
		ParserError("error: function of that name already exists");
		return false;
	}

	m_pCurrentFunction=AddSysGlobalFunction(nID, strName, strRetType, nArrayLevel, NULL);

	return true;
}

void CPNamespace::OnFunctionEnd()
{
	m_pCurrentFunction=NULL;
}

//Arg:	  NSP_IDENTIFIER  NSP_IDENTIFIER ArrayDecl_opt { m_pCompilerKrnl->m_Namespace.OnArgument($1, $3);  }
//		;
void CPNamespace::OnArgument(CString &strType, long nArrayLevel)
{
	if (m_pCurrentObjectMethod!=NULL) m_pCurrentObjectMethod->AddArgument(this, strType, nArrayLevel, NULL);
	else if (m_pCurrentFunction!=NULL) m_pCurrentFunction->AddArgument(this, strType, nArrayLevel, NULL);
	else if (m_pCurrentEventHeader!=NULL) m_pCurrentEventHeader->AddArgument(this, strType, nArrayLevel, NULL);
    else ASSERT(false);
}

bool CPNamespace::OnNoinstancesObjectBegin(CString &strName)
{
    if (GetObject(strName)!=NULL)
	{
		ParserError("error: object of that name already exists");
		return false;
	}

	m_pCurrentObject=AddObject(strName, false);

	return true;
}

void CPNamespace::OnNoinstancesObjectEnd()
{
    m_pCurrentObject=NULL;
}

