/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Interpret
 *   Autor: Helena Kupková
 *  
 *   Popis: Implementace objektu CIBag pro predavani argumentu
 *          metoda, funkcim a reakcim na udalosti.
 * 
 ***********************************************************/

#include "stdafx.h"
#include "..\Interpret.h"
#include "TripleS\CodeManager\CodeManager.h"

extern bool g_bGameLoading;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

DEFINE_MULTITHREAD_POOL( CIBag, 20);

CIBag::CIBag()
{
    m_aArgs.SetSize(GROW_BY,GROW_BY);
    m_nArgCount = 0;
    m_pPreviousBag = NULL;
    m_pReturnRegister = NULL;
}

CIBag::~CIBag()
{
    m_aArgs.RemoveAll();
}

void CIBag::Create()
{
    m_nArgCount = 0;
    m_pReturnRegister = NULL;

    m_pPreviousBag = NULL;
}

void CIBag::Delete()
{
    if (g_bGameLoading)
    {
        if (m_pPreviousBag != NULL)
        {
            m_pPreviousBag->Delete();
            delete m_pPreviousBag;
        }
        m_pPreviousBag = NULL;
        return;
    }
    
    int i;
    for (i=0; i<m_nArgCount; i++) m_aArgs[i].Clean();
    m_nArgCount = 0;

    m_pPreviousBag = NULL;
}

CIBag::operator= (CIBag &anotherOne)
{
    Delete();

    m_nArgCount = anotherOne.m_nArgCount;
    m_aArgs.SetSize( m_nArgCount, GROW_BY);

    for (int i=0; i<m_nArgCount; i++)
    {
        m_aArgs[i] = anotherOne.m_aArgs[i];
    }

    m_pReturnRegister = anotherOne.m_pReturnRegister;

    m_pPreviousBag = NULL;
}

#define _helper_GrowArgArrayIfNeeded    \
        if (m_aArgs.GetSize() == m_nArgCount)                 \
        {                                                   \
            m_aArgs.SetSize(m_aArgs.GetSize() + GROW_BY, GROW_BY);  \
        }

void CIBag::AddConstantChar(char cChar)
{
    _helper_GrowArgArrayIfNeeded;
    m_aArgs[m_nArgCount++].Set(cChar);
}

void CIBag::AddConstantInt(int nInt)
{
    _helper_GrowArgArrayIfNeeded;
    m_aArgs[m_nArgCount++].Set(nInt);
}

void CIBag::AddConstantFloat(double fFloat)
{
    _helper_GrowArgArrayIfNeeded;
    m_aArgs[m_nArgCount++].Set(fFloat);
}

void CIBag::AddConstantBool(bool bBool)
{
    _helper_GrowArgArrayIfNeeded;
    m_aArgs[m_nArgCount++].Set(bBool);
}

void CIBag::AddUnit( CZUnit *pUnit, CIUnitType *pIUnitType)
{
    _helper_GrowArgArrayIfNeeded;
    
    if (pIUnitType == NULL)
    {
        m_aArgs[m_nArgCount++].Set( pUnit, CCodeManager::m_pDefaultUnitType);
    }
    else
    {
        m_aArgs[m_nArgCount++].Set( pUnit, pIUnitType);
    }
}

void CIBag::AddSystemObject( CISystemObject *pSystemObject)
{
    _helper_GrowArgArrayIfNeeded;
    m_aArgs[m_nArgCount++].Set( pSystemObject);
}

void CIBag::AddStructure( CIStructure *pStructure)
{
    _helper_GrowArgArrayIfNeeded;
    m_aArgs[m_nArgCount++].Set( pStructure);
}

void CIBag::AddRegister(CIDataStackItem &DSI)
{
    _helper_GrowArgArrayIfNeeded;
    m_aArgs[m_nArgCount++] = DSI;
}

void CIBag::AddNull()
{
	_helper_GrowArgArrayIfNeeded;
	m_aArgs[m_nArgCount++].Set( (int)0);
}

#undef _helper_GrowArgArrayIfNeeded

CIDataStackItem* CIBag::GetArg(int nIndex)
{
    ASSERT(nIndex >= 0);
    
    if (nIndex >= m_nArgCount) return NULL;
    else return &m_aArgs[nIndex];
}
CIDataStackItem* CIBag::GetReturnRegister()
{
    return m_pReturnRegister;
}

int* CIBag::GetChar(int nIndex)
{
    ASSERT(nIndex >= 0);

    if (nIndex >= m_nArgCount || !m_aArgs[nIndex].IsChar()) return NULL;
    else return &m_aArgs[nIndex].m_nInt;
}

int* CIBag::GetInt(int nIndex)
{
    ASSERT(nIndex >= 0);

    if (nIndex >= m_nArgCount || !m_aArgs[nIndex].IsInt()) return NULL;
    else return &m_aArgs[nIndex].m_nInt;
}

int* CIBag::GetBool(int nIndex)
{
    ASSERT(nIndex >= 0);

    if (nIndex >= m_nArgCount || !m_aArgs[nIndex].IsBool()) return NULL;
    else return &m_aArgs[nIndex].m_nInt;
}

double* CIBag::GetFloat(int nIndex)
{
    ASSERT(nIndex >= 0);

    if (nIndex >= m_nArgCount || !m_aArgs[nIndex].IsFloat()) return NULL;
    else return &m_aArgs[nIndex].m_fFloat;
}

CISystemObject* CIBag::GetSystemObject(int nIndex)
{
    ASSERT(nIndex >= 0);

    if (nIndex >= m_nArgCount || !m_aArgs[nIndex].IsSystemObject()) return NULL;
    else return (CISystemObject*)m_aArgs[nIndex].m_pSystemObject;
}

CIStructure* CIBag::GetStructure(int nIndex)
{
    ASSERT(nIndex >= 0);

    if (nIndex >= m_nArgCount || !m_aArgs[nIndex].IsStruct()) return NULL;
    else return (CIStructure*)m_aArgs[nIndex].m_pStruct;
}

CZUnit* CIBag::GetUnit(int nIndex)
{
    ASSERT(nIndex >= 0);

    if (nIndex >= m_nArgCount || !m_aArgs[nIndex].IsUnit()) return NULL;
    else return (CZUnit*)m_aArgs[nIndex].m_pUnit;
}

CISOString* CIBag::GetString(int nIndex)
{
    ASSERT(nIndex >= 0);

    if (nIndex >= m_nArgCount || !m_aArgs[nIndex].IsString()) return NULL;
    else return (CISOString*)m_aArgs[nIndex].m_pSystemObject;
}

CISOArrayParent* CIBag::GetArray(int nIndex)
{
    ASSERT(nIndex >= 0);

    if (nIndex >= m_nArgCount || !m_aArgs[nIndex].IsArray()) return NULL;
    else return (CISOArrayParent*)m_aArgs[nIndex].m_pSystemObject;
}

CISOSetParent* CIBag::GetSet(int nIndex)
{
    ASSERT(nIndex >= 0);

    if (nIndex >= m_nArgCount || !m_aArgs[nIndex].IsSet()) return NULL;
    else return (CISOSetParent*)m_aArgs[nIndex].m_pSystemObject;
}


//
//	SAVE & LOAD
//

void CIBag::PersistentSave( CPersistentStorage &storage)
{
    BRACE_BLOCK(storage);
    
    int i;

	// pointer na sebe
	storage << (DWORD)this;

    // pocet argumentu
    storage << m_nArgCount;
    
	// argumenty
    for (i=0; i<m_nArgCount; i++)
	{
		m_aArgs[i].PersistentSave( storage);
	}

    // navratovy registr
    storage << (DWORD)m_pReturnRegister;

    // navazani rozdelanych bagu
    storage << (DWORD)m_pPreviousBag;
	if (m_pPreviousBag != NULL) m_pPreviousBag->PersistentSave( storage);
}

void CIBag::PersistentLoad( CPersistentStorage &storage)
{
    BRACE_BLOCK(storage);
    
    int i;

	// pointer na sebe
	CIBag *pThis;
	storage >> (DWORD&)pThis;
	storage.RegisterPointer( pThis, this);

    // pocet argumentu
    storage >> m_nArgCount;
    
	// argumenty
	m_aArgs.SetSize( m_nArgCount, GROW_BY);
    for (i=0; i<m_nArgCount; i++)
	{
		m_aArgs[i].PersistentLoad( storage);
	}

    // navratovy registr
    storage >> (DWORD&)m_pReturnRegister;

    // navazani rozdelanych bagu
    storage >> (DWORD&)m_pPreviousBag;
	if (m_pPreviousBag != NULL) 
	{
		m_pPreviousBag = new CIBag;
		m_pPreviousBag->PersistentLoad( storage);
	}
}

void CIBag::PersistentTranslatePointers( CPersistentStorage &storage)
{
	int i;

	// argumenty
    for (i=0; i<m_nArgCount; i++)
	{
		m_aArgs[i].PersistentTranslatePointers( storage);
	}

    // navratovy registr
    m_pReturnRegister = (CIDataStackItem*) storage.TranslatePointer( m_pReturnRegister);

    // navazani rozdelanych bagu
    if (m_pPreviousBag != NULL) 
	{
		m_pPreviousBag->PersistentTranslatePointers( storage);
	}
}

void CIBag::PersistentInit()
{
	// empty
}

void CIBag::ReturnChar(char cChar)     
{ 
    ASSERT( m_pReturnRegister != NULL); 
    m_pReturnRegister->Set(cChar); 
}

void CIBag::ReturnInt(int nInt)        
{ 
    ASSERT( m_pReturnRegister != NULL); 
    m_pReturnRegister->Set(nInt);  
}

void CIBag::ReturnFloat(double fFloat) 
{
    ASSERT( m_pReturnRegister != NULL); 
    m_pReturnRegister->Set(fFloat);
}

void CIBag::ReturnBool(bool bBool)     
{ 
    ASSERT( m_pReturnRegister != NULL); 
    m_pReturnRegister->Set(bBool); 
}

void CIBag::ReturnSystemObject( CISystemObject *pSystemObject)
{
    ASSERT( m_pReturnRegister != NULL);
    m_pReturnRegister->Set( pSystemObject);
}

void CIBag::ReturnUnit( CZUnit *pUnit, CIUnitType *pIUnitType)
{
    ASSERT( m_pReturnRegister != NULL);
    m_pReturnRegister->Set( pUnit, pIUnitType);
}
