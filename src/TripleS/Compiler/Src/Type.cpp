/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Prekladac
 *   Autor: Helena Kupková
 *  
 *   Popis: CType - trida reprezentujici typ jazyka
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

CType::CType()
{
    m_nType=T_VOID;
    m_stiName=NULL;
    m_nSOType=SO_NONE;
    m_pOfType=NULL;
	m_nArraySize=0;
}


CType::CType(CType& anotherType)
{
    m_nType=T_VOID;
    m_stiName=NULL;
    m_nSOType=SO_NONE;
    m_pOfType=NULL;
	m_nArraySize=0;

    *this=anotherType;
}


CType::~CType()
{
    if (m_pOfType!=NULL) 
	{
		delete m_pOfType;
		m_pOfType=NULL;
	}

    if (m_stiName!=NULL) m_stiName->Release();
}

CType::operator==(CType& anotherType)
{
    if (m_nType==anotherType.m_nType)
    {
        switch (m_nType)
        {
        case T_OBJECT: 
            if (m_nSOType == anotherType.m_nSOType)
            {
                if (m_nSOType == SO_ARRAY) return *m_pOfType == *anotherType.m_pOfType;
                else return true;
            }
            else return false;            
        case T_UNIT:
        case T_STRUCT:
            return m_stiName==anotherType.m_stiName;
        case T_ERROR:
            return false;
        default:
            return true;
        }
    }

    return false;
}

CType::operator=(CType& anotherType)
{
    SetStiName(anotherType.m_stiName);
    m_nType=anotherType.m_nType;

    m_nSOType = anotherType.m_nSOType;
    m_nArraySize=anotherType.m_nArraySize;

	if (m_pOfType!=NULL) 
	{
		delete m_pOfType;
		m_pOfType=NULL;
	}

    if (anotherType.m_pOfType!=NULL)
    {
        m_pOfType=new CType();
        (*m_pOfType)=(*anotherType.m_pOfType);
    }
}

void CType::Set(CString &strType, int nArrayLevel, CPNamespace *pNamespace, CPPreview *pPreview)
{
    if (strType=="char") Set(T_CHAR);
    else if (strType=="void") SetVoid();
    else if (strType=="int") Set(T_INT);
    else if (strType=="float") Set(T_FLOAT);
    else if (strType=="bool") Set(T_BOOL);
    else 
	{
        ASSERT(pNamespace!=NULL);
		CPNamespace::CUnitObjsType *pUOType=pNamespace->GetType(strType);
        EScriptType eScriptType = SCRIPT_UNKNOWN;
        if ( pUOType == NULL)
        {
            if (pPreview != NULL) 
            {
                if (pPreview->GetUnit(strType) != NULL) eScriptType = SCRIPT_UNIT;
                else if (pPreview->GetStruct(strType) != NULL) eScriptType = SCRIPT_STRUCTURE;
            }
        }
        if ( eScriptType == SCRIPT_UNKNOWN)
        {
            if (pUOType != NULL) eScriptType = pUOType->m_nType;
            else eScriptType = pNamespace->GetScriptType(strType);
        }
		
		switch ( eScriptType)
        {
        case SCRIPT_OBJECT:
            m_nType=T_OBJECT;
            m_nSOType = GetSOTypeFromString(strType);
            break;

        case SCRIPT_UNIT:
            m_nType=T_UNIT;
            break;

        case SCRIPT_STRUCTURE:
            m_nType=T_STRUCT;
            break;

        default:
			      m_nType = T_ERROR;
			      break;
        }

        SetStiName(g_StringTable.AddItem(strType, false));
	}

    if (nArrayLevel > 0)
    {
        do
        {
            MakeArray();
            nArrayLevel--;
        } while (nArrayLevel > 0);
    }
    else if (nArrayLevel == -1) MakeSet();
}

void CType::Set(CPLexVal &LexVal)
{
	switch (LexVal.m_nType)
	{
	case CPLexVal::LEXVAL_ERROR:
		SetError();
		break;
	case CPLexVal::LEXVAL_NONE:
		SetNone();
		break;
	case CPLexVal::LEXVAL_INT:
		Set(T_INT);
		break;
	case CPLexVal::LEXVAL_FLOAT:
		Set(T_FLOAT);
		break;
	case CPLexVal::LEXVAL_BOOL:
		Set(T_BOOL);
		break;
	case CPLexVal::LEXVAL_CHAR:
		Set(T_CHAR);
		break;
	case CPLexVal::LEXVAL_STRING:
		SetString();
	}
}

void CType::Set(EDataType nType)
{
	ASSERT(nType!=T_OBJECT && nType!=T_UNIT && nType!=T_STRUCT);

	if (m_pOfType!=NULL) 
	{
		delete m_pOfType;
		m_pOfType=NULL;
	}
	
    SetStiName(NULL);
    
    m_nSOType = SO_NONE;
    m_nArraySize = 0;

	m_nType=nType;
}

CString CType::GetString()
{
    switch (m_nType)
    {
	case T_ERROR: return CString("type_error");
    case T_VOID:  return CString("void");
    case T_CHAR:  return CString("char");
    case T_INT:   return CString("int");
    case T_FLOAT: return CString("float");
    case T_BOOL: return CString("bool");
    case T_OBJECT:	if (IsArray()) return m_pOfType->GetString()+"[]";
					else if (IsSet()) return m_pOfType->GetString()+"{}";
                    else return CString( GetStringFromSOType(m_nSOType));
    case T_UNIT:
    case T_STRUCT:   return CString(*m_stiName);
    }

    return CString("none");
}

CType* CType::GetBasicType()
{
	if (IsArray())
	{
		CType *pBasicType=m_pOfType;
		ASSERT(pBasicType!=NULL);
		
		while (pBasicType->IsArray()) 
		{
			pBasicType=pBasicType->m_pOfType;
			ASSERT(pBasicType!=NULL);
		}

		return pBasicType;
	}
    else if (IsSet())
    {
        return m_pOfType;
    }
    else return this;
}

void CType::MakeArray(int nArraySize)
{
	CType *pBasicType=this;

	while (pBasicType->IsArray()) 
	{
		pBasicType=pBasicType->m_pOfType;
		ASSERT(pBasicType!=NULL);
	}

	CType *pOfType=new CType();
	(*pOfType)=(*pBasicType);
	
	pBasicType->m_nType=T_OBJECT;
    pBasicType->SetStiName( g_StringTable.AddItem(GetStringFromSOType(SO_ARRAY), false ) );
    pBasicType->m_nSOType = SO_ARRAY;

	pBasicType->m_nArraySize=nArraySize;

	if (pBasicType->m_pOfType!=NULL) delete pBasicType->m_pOfType;
	pBasicType->m_pOfType=pOfType;
}

void CType::MakeSet()
{
    // nesmi byt mnozina poli nebo mnozina mnozin
    ASSERT(!IsArray());
    ASSERT(!IsSet());

	CType *pOfType=new CType();
	(*pOfType)=*this;

    SetNone();

    m_nType = T_OBJECT;
    
    SetStiName( g_StringTable.AddItem(GetStringFromSOType(SO_SET), false ) );
    m_nSOType = SO_SET;

    m_pOfType = pOfType;
}

void CType::SetError()	   
{ 
	if (m_pOfType!=NULL) 
	{
		delete m_pOfType;
		m_pOfType=NULL;
	}
	
    SetStiName(NULL);

	m_nType=T_ERROR;
}	

void CType::SetNone()     
{ 
	if (m_pOfType!=NULL) 
	{
		delete m_pOfType;
		m_pOfType=NULL;
	}

	if (!IsObject()) SetStiName(NULL);
    else
    {
        m_nSOType = SO_NONE;
        m_nArraySize = 0;
    }

	m_nType=T_VOID; 
}

void CType::SetVoid()         
{ 
	if (m_pOfType!=NULL) 
	{
		delete m_pOfType;
		m_pOfType=NULL;
	}
	
    if (!IsObject()) SetStiName(NULL);
    else
    {
        m_nSOType = SO_NONE;
        m_nArraySize = 0;
    }

	m_nType=T_VOID; 
}

void CType::SetObject(ESystemObjectType nSOType) 
{ 
    ASSERT(nSOType != SO_ARRAY && nSOType != SO_SET);

	SetNone();
	
	m_nType=T_OBJECT; 

    SetStiName( g_StringTable.AddItem( GetStringFromSOType(nSOType), false) );
    m_nSOType = nSOType;

    m_nArraySize = 0;
}

void CType::SetStruct(CStringTableItem *stiName) 
{ 
	SetNone();

    m_nType=T_STRUCT; 

    SetStiName(stiName);
}

void CType::SetUnit(CStringTableItem *stiName)   
{ 
	SetNone();

    m_nType=T_UNIT;  
	
    SetStiName(stiName);  
}

void CType::SetOfType()
{
	ASSERT(m_pOfType!=NULL);
	
    m_nType = m_pOfType->m_nType;
    SetStiName( m_pOfType->m_stiName);
    m_nSOType = m_pOfType->m_nSOType;
    m_nArraySize = m_pOfType->m_nArraySize;

    CType *pPom = m_pOfType;

    m_pOfType = m_pOfType->m_pOfType;

    pPom->m_pOfType = NULL;
    delete pPom;
}

void CType::Save(CCompilerOutput *pOut, CPCodeStringTable &StrTbl, bool bSaveArrayLengths)
{
	static char pBuffer[50];

	int arrayLevel=0;
	CType *pBasicType=this;
	int nPos=0;

    if (IsArray())
    {
	    while (pBasicType->IsArray()) 
	    {
		    pBasicType=pBasicType->m_pOfType;
		    arrayLevel++;
		    ASSERT(pBasicType!=NULL); 
	    }
        ASSERT(!pBasicType->IsSet());
    }
    else if (IsSet())
    {
        pBasicType = pBasicType->m_pOfType;
        arrayLevel = -1;
    }

	// zakladni typ
	*(short*)(pBuffer+nPos)=pBasicType->m_nType;
	nPos+=sizeof(short);

	// kdyz unita, objekt nebo struktura, tak uloz strID nazvu
	if (pBasicType->IsObjectUnitStruct())
	{
        *(long*)(pBuffer+nPos)=StrTbl.Add((CString)*pBasicType->m_stiName)->m_nID;

        nPos+=sizeof(long);
	}

	// array level
	*(short*)(pBuffer+nPos)=arrayLevel;
	nPos+=sizeof(short);

	// delka poli
	if (arrayLevel>0)
	{
		if (bSaveArrayLengths)
		{
			pBuffer[nPos++]=1;

			pBasicType=this;
			while (pBasicType->IsArray())
			{
				*(short*)(pBuffer+nPos)=pBasicType->m_nArraySize;
				nPos+=sizeof(short);
				pBasicType=pBasicType->m_pOfType;
			}
		}
		else pBuffer[nPos++]=0;
	}

	pOut->PutShort(nPos);
	pOut->PutChars(pBuffer, nPos);
}

void CType::SetStiName(CStringTableItem* pSTI)
{
    if (m_stiName!=NULL) m_stiName=m_stiName->Release(); // da tam NULL
    if (pSTI!=NULL) m_stiName=pSTI->AddRef();
}

//
// SAVE & LOAD
//

void CType::PersistentSave( CPersistentStorage &storage)
{
    BRACE_BLOCK(storage);
    
    // "typ" typu dane instance
    storage << (DWORD)m_nType;                

    // objekt, jednotka nebo struktura - nazev typu
    storage << (DWORD)m_stiName;  

    // objekt
    // typ systemoveho objektu
    storage << (DWORD)m_nSOType;

    // podtyp - pro pole a mnozinu
    storage << (DWORD)m_pOfType;
	if (m_pOfType != NULL) m_pOfType->PersistentSave( storage);

    // velikost pole (platne jen kdyz >0, napr. v parametrech funkce se velikost 
    // pole neudava
	storage << m_nArraySize;
}

void CType::PersistentLoad( CPersistentStorage &storage)
{
    BRACE_BLOCK(storage);
    
    // "typ" typu dane instance
    storage >> (DWORD&)m_nType;                

    // objekt, jednotka nebo struktura - nazev typu
    storage >> (DWORD&)m_stiName;  

    // objekt
    // typ systemoveho objektu
    storage >> (DWORD&)m_nSOType;

    // podtyp - pro pole a mnozinu
    storage >> (DWORD&)m_pOfType;
	if (m_pOfType != NULL) 
	{
		m_pOfType = new CType;
		m_pOfType->PersistentLoad( storage);
	}

    // velikost pole (platne jen kdyz >0, napr. v parametrech funkce se velikost 
    // pole neudava
	storage >> m_nArraySize;
}

void CType::PersistentTranslatePointers( CPersistentStorage &storage)
{
	m_stiName = (CStringTableItem*) storage.TranslatePointer( m_stiName);
	if (m_pOfType != NULL) m_pOfType->PersistentTranslatePointers( storage);
}

void CType::PersistentInit()
{
}
