/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Interpret
 *   Autor: Helena Kupková
 *  
 *   Popis: Implementace trid CIDataStack, CIDataStackItem a CIDataStackItemSet.
 *          CIDataStack - typovany datovy zasobnik procesu
 *          CIDataStackItem - jedna bunka datoveho zasobniku
 *          CIDataStackItemSet - DATA_STACK_ITEM_SET_SIZE bunek CIDataStackItem
 *                          zabalenych do jedne tridy (tzv. setu). Datovy zasobnik 
 *                          roste po setech.
 *          
 ***********************************************************/

#include "stdafx.h"
#include "..\Interpret.h"
#include "SysObjs\SystemObjects.h"
// code manager
#include "TripleS\CodeManager\CodeManager.h"
// CZUnit
#include "GameServer\Civilization\Src\ZUnit.h"

//////////////////////////////////////////////////////////////////////
// CIDataStackItem
//////////////////////////////////////////////////////////////////////

CIDataStackItem::CIDataStackItem()
{
	// typ
	m_nDataType = T_VOID;

	// data
	// tohle by melo vse pokryt
    m_pUnit = NULL;
	m_pIUnitType = NULL;
}

//////////////////////////////////////////////////////////////////////

CIDataStackItem::~CIDataStackItem()
{
	Clean();
}

//////////////////////////////////////////////////////////////////////

void CIDataStackItem::Clean()
{
	// data
    switch (m_nDataType)
    {
    case T_UNIT:
        if (m_pUnit != NULL) m_pUnit->Release();
        break;
    case T_OBJECT:
        if (m_pSystemObject != NULL) m_pSystemObject->Release();
        break;
    case T_STRUCT:
        if (m_pStruct != NULL) m_pStruct->Release();
    }

	// tohle by melo vse pokryt
    m_pUnit = NULL;
	m_pIUnitType = NULL;

    // typ
	m_nDataType = T_VOID;
}

//////////////////////////////////////////////////////////////////////

#define _help_IsObject( SOType)       { return m_nDataType == T_OBJECT && (m_pSystemObject == NULL || m_pSystemObject->GetObjectType() == SOType); }

bool CIDataStackItem::IsString(){      _help_IsObject(SO_STRING)}
bool CIDataStackItem::IsArray(){       _help_IsObject(SO_ARRAY)}
bool CIDataStackItem::IsSet(){         _help_IsObject(SO_SET)}

#undef _help_IsObject


EInterpretStatus CIDataStackItem::SetTypeAndCreate(CIType &IType, CInterpret *pInterpret)
{
	Clean();

    // nastaveni typu
    m_nDataType = IType.m_nType;
    
    // nastaveni hodnoty, pripadne vytvoreni noveho objektu/struktury
    m_nInt = 0;

    if (IType.IsUnit())
    {
        m_pUnit = NULL;
        m_pIUnitType = NULL;
    }

    if (IType.IsObject())
    {
        m_pSystemObject = pInterpret->CreateSystemObject(IType);

        if (m_pSystemObject == NULL) return STATUS_NO_SUCH_SYSTEM_OBJECT;
    }

    if (IType.IsStruct())
    {
        ASSERT(IType.m_stiName != NULL);

        CIStructureType *pStructType;
        if (( pStructType = pInterpret->m_pCodeManager->GetStructureType( IType.m_stiName)) == NULL)
        {
            return STATUS_STRUCTURE_NOT_FOUND;
        }
        else
        {
			m_pStruct = pInterpret->CreateStructure( pStructType, true);
        }
    }

    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////

void CIDataStackItem::Load(CIType &IType, int *pData)
{
    Clean();

    ASSERT(pData!=NULL);

    // nastaveni typu
    m_nDataType = IType.m_nType;

    switch (m_nDataType)
    {
    case T_ERROR:
        ASSERT(false);
        break;

    case T_VOID:
        ASSERT(false);
        break;

    case T_CHAR:
    case T_INT:
    case T_BOOL:
        m_nInt = *pData;
        break;

    case T_FLOAT:
        m_fFloat = *(double*)pData;
        break;

    case T_OBJECT:
        m_pSystemObject = (CISystemObject*)*pData;
		if (m_pSystemObject != NULL)
		{
			ASSERT_VALID(m_pSystemObject);
			m_pSystemObject->AddRef();
		}
        break;

    case T_UNIT:
        m_pUnit = (CZUnit*)*pData;
        if (m_pUnit != NULL)
		{
			ASSERT_VALID(m_pUnit);
			m_pUnit->AddRef();
			m_pIUnitType = m_pUnit->m_pIUnitType;
		}
		else 
		{
			m_pIUnitType = NULL;
		}
        break;

    case T_STRUCT:
        m_pStruct = (CIStructure*)*pData;
		if (m_pStruct != NULL)
		{
			ASSERT_VALID( m_pStruct);
			m_pStruct->AddRef();
		}
        break;
    }
}

//////////////////////////////////////////////////////////////////////

void CIDataStackItem::Store(int *pData)
{
    ASSERT(pData!=NULL);

    switch (m_nDataType)
    {
    case T_ERROR:
        ASSERT(false);
        break;

    case T_VOID:
        ASSERT(false);
        break;

    case T_CHAR:
    case T_INT:
    case T_BOOL:
        *pData = m_nInt;
        break;

    case T_FLOAT:
        *(double*)pData = m_fFloat;
        break;

    case T_OBJECT:
        if (((CISystemObject*)*pData)!=NULL) ((CISystemObject*)*pData)->Release();
        *pData = (int)m_pSystemObject;
		if ( ((CISystemObject*)*pData) != NULL) 
		{
			((CISystemObject*)*pData)->AddRef();
			ASSERT_VALID((CISystemObject*)*pData);
		}
        break;

    case T_UNIT:
        if (((CZUnit*)*pData)!=NULL) ((CZUnit*)*pData)->Release();
        *pData = (int)m_pUnit;
        if (((CZUnit*)*pData) != NULL) 
		{
			((CZUnit*)*pData)->AddRef();
			ASSERT_VALID( ((CZUnit*)*pData) );
		}

        break;

    case T_STRUCT:
        if (((CIStructure*)*pData)!=NULL) ((CIStructure*)*pData)->Release();
        *pData = (int)m_pStruct;
		if (((CIStructure*)*pData) != NULL) 
		{
			((CIStructure*)*pData)->AddRef();
			ASSERT_VALID( (CIStructure*)*pData);
		}
        break;
    }
}

//////////////////////////////////////////////////////////////////////

void CIDataStackItem::operator=(CIDataStackItem &anotherOne)
{
	Clean();

	// type
	m_nDataType = anotherOne.m_nDataType;

	// data
	switch (m_nDataType)
    {
    case T_UNIT:
        if(anotherOne.m_pUnit != NULL)
		{
          m_pUnit = (CZUnit*)anotherOne.m_pUnit->AddRef();
        }
        else
		{
          m_pUnit = NULL;
        }
        m_pIUnitType = anotherOne.m_pIUnitType;
        break;
    case T_OBJECT:
        if (anotherOne.m_pSystemObject != NULL)
		{
			m_pSystemObject = (CISystemObject*)anotherOne.m_pSystemObject->AddRef();
		}
		else 
		{
			m_pSystemObject = NULL;
		}
        break;
    case T_STRUCT:
        if (anotherOne.m_pStruct != NULL)
		{
			m_pStruct = (CIStructure*)anotherOne.m_pStruct->AddRef();
		}
		else 
		{
			m_pStruct = NULL;
		}
        break;
    case T_FLOAT:
        m_fFloat = anotherOne.m_fFloat;
        break;
    default:
        m_nInt = anotherOne.m_nInt;
        break;
    }
}

//////////////////////////////////////////////////////////////////////
/*
bool CIDataStackItem::SameTypeAs( CIDataStackItem &anotherOne)
{
    if (m_nDataType == anotherOne.m_nDataType)
    {
        if (IsSimpleType()) return true;
        else 
        {
            switch (m_nDataType)
            {
            case T_OBJECT: 
				if (m_pSystemObject == NULL || anotherOne.m_pSystemObject == NULL) return true;
				else return m_pSystemObject->GetObjectType() == anotherOne.m_pSystemObject->GetObjectType();
            case T_UNIT:
                return m_pIUnitType == anotherOne.m_pIUnitType;
            case T_STRUCT:
                return m_pStruct->m_pStructType == anotherOne.m_pStruct->m_pStructType;
            }
        }
    }
    
    return false;
}
*/
//////////////////////////////////////////////////////////////////////

bool CIDataStackItem::SameTypeAs( CIType &Type)
{
    if (m_nDataType == Type.m_nType)
    {
        if (IsSimpleType()) return true;
        else 
        {
            switch (m_nDataType)
            {
            case T_OBJECT: 
				if (m_pSystemObject == NULL) return true;
                else return m_pSystemObject->GetObjectType() == Type.m_nSOType;
            case T_UNIT:
                if (m_pIUnitType == NULL) return true;
                return m_pIUnitType->m_stiName == Type.m_stiName;
            case T_STRUCT:
				if (m_pStruct == NULL) return true;
                else return m_pStruct->m_pStructType->m_stiName == Type.m_stiName;
            }
        }
    }
	// null
	else if ( Type.IsObjectUnitStruct() && m_nDataType == T_INT && m_nInt == 0)
	{
		if (Type.IsUnit()) 
		{
			m_nDataType = T_UNIT;
			m_pUnit = NULL;
			m_pIUnitType = NULL;
		}
		else if (Type.IsObject())
		{
			m_nDataType = T_OBJECT;
			m_pSystemObject = NULL;
		}
		else if (Type.IsStruct())
		{
			m_nDataType = T_STRUCT; 
			m_pStruct = NULL;
		}
		else ASSERT(false);

		return true;
    }
    
    return false;
}



bool CIDataStackItem::SameTypeAsAndConvertUnits( CIType &Type)
{
	if (SameTypeAs( Type))
    {
        if (m_nDataType != T_UNIT) return true;
	    else
	    {
		    // konverze typu jednotky - pokud to jde
		    CIUnitType *pUnitType;

		    if (m_pUnit == NULL) 
		    {
			    // hodnota null
			    if (m_pIUnitType == NULL)
			    {
				    return true;
			    }
			    pUnitType = m_pIUnitType;
		    }
		    else pUnitType = m_pUnit->m_pIUnitType;

		    // pruchod dedicnosti
		    while (pUnitType != NULL && pUnitType->m_stiName != Type.m_stiName)
		    {
			    pUnitType = pUnitType->m_pParent;
		    }

		    if ( pUnitType != NULL) 
		    {
			    m_pIUnitType = pUnitType;
			    return true;
		    }
		    else return false;
        }
    }
    else 
    {
        if ( !Type.IsUnit() && !IsUnit()) return false;
        else
        {
            CIUnitType *pSrcUnitType = m_pIUnitType;
            CStringTableItem *pDestUnitTypeName = Type.m_stiName;

            while ( pSrcUnitType != NULL && pSrcUnitType->m_stiName != pDestUnitTypeName)
            {
                pSrcUnitType = pSrcUnitType->m_pParent;
            }

            if ( pSrcUnitType == NULL) return false;
            else
            {
                m_pIUnitType = pSrcUnitType;
                return true;
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////

void CIDataStackItem::Set(CISystemObject *pSystemObject)  
{ 
    Clean(); 
    m_nDataType = T_OBJECT; 
    m_pSystemObject = pSystemObject; 
	if (pSystemObject != NULL) m_pSystemObject->AddRef(); 
}

//////////////////////////////////////////////////////////////////////

void CIDataStackItem::Set(CZUnit *pUnit) 
{ 
    Clean(); 
    m_nDataType = T_UNIT; 
    m_pUnit = pUnit;
    if (m_pUnit != NULL) 
    {
        m_pUnit->AddRef(); 
        m_pIUnitType = pUnit->m_pIUnitType; 
    }
    else m_pIUnitType = NULL;
}

//////////////////////////////////////////////////////////////////////

void CIDataStackItem::Set(CIStructure *pStruct)   
{ 
    Clean(); 
    m_nDataType = T_STRUCT; 
    m_pStruct = pStruct;
    if ( pStruct != NULL) m_pStruct->AddRef(); 
}

//////////////////////////////////////////////////////////////////////

void CIDataStackItem::Set( CZUnit *pUnit, CIUnitType *pIUnitType)
{
    Clean();

    // typ
    m_nDataType = T_UNIT;

    // data
    m_pIUnitType = pIUnitType;

    if(pUnit != NULL)
	{
		ASSERT( pIUnitType != NULL);
		m_pUnit = (CZUnit*)pUnit->AddRef();
    }
    else
	{
      m_pUnit = NULL;
    }
}


//
//	SAVE & LOAD
//

void CIDataStackItem::PersistentSave( CPersistentStorage &storage)
{
    BRACE_BLOCK(storage);
    
    // pointer na sebe
	storage << (DWORD)this;

	// datovy typ - enum 
	storage << (DWORD)m_nDataType;

	// hodnota
	// double je nejvetsi - 8B => pokryje vsechno
	storage << m_fFloat;
}

//////////////////////////////////////////////////////////////////////

void CIDataStackItem::PersistentLoad( CPersistentStorage &storage)
{
    BRACE_BLOCK(storage);
    
    // pointer na sebe
	CIDataStackItem *pThis;
	storage >> (DWORD&)pThis;
	storage.RegisterPointer( pThis, this);

	// datovy typ - enum
	storage >> (DWORD&)m_nDataType;

	// hodnota
	// double je nejvetsi - 8B => pokryje vsechno
	storage >> m_fFloat;

}

//////////////////////////////////////////////////////////////////////

void CIDataStackItem::PersistentTranslatePointers( CPersistentStorage &storage)
{
	switch ( m_nDataType)
	{
	case T_OBJECT:
		if (m_pSystemObject != NULL)
		{
			m_pSystemObject = (CISystemObject *)storage.TranslatePointer( m_pSystemObject);
		}
		break;

	case T_UNIT:
		if (m_pUnit != NULL)
		{
			m_pUnit = (CZUnit *) storage.TranslatePointer( m_pUnit);
		}
        m_pIUnitType = (CIUnitType *) storage.TranslatePointer( m_pIUnitType);
		break;

	case T_STRUCT:
		if (m_pStruct != NULL)
		{
			m_pStruct = (CIStructure*) storage.TranslatePointer( m_pStruct);
		}
		break;
	}
}

//////////////////////////////////////////////////////////////////////

void CIDataStackItem::PersistentInit()
{
	// empty
}

//////////////////////////////////////////////////////////////////////
/*
void CIDataStackItem::GetSystemObjectsAndStructures( CTypedPtrMap<CMapPtrToPtr, CISystemObject*, CISystemObject*> &tpSystemObjects,
														CTypedPtrMap<CMapPtrToPtr, CIStructure*, CIStructure*> &tpStructures)
{
	switch (m_nDataType)
	{
	case T_OBJECT:
		tpSystemObjects.SetAt( m_pSystemObject, m_pSystemObject);
		switch (m_pSystemObject->GetObjectType())
		{
		case SO_ARRAY:
			switch ( ((CISOArrayParent*)m_pSystemObject)->m_DataType.m_nType)
			{
			case T_OBJECT:
				((CISOArray<CISystemObject*>*)m_pSystemObject)->GetSystemObjectsAndStructures( tpSystemObjects, tpStructures);
				break;
			case T_STRUCT:
				((CISOArray<CIStructure*>*)m_pSystemObject)->GetSystemObjectsAndStructures( tpSystemObjects, tpStructures);
				break;
			}
			break;

		case SO_SET:
			switch ( ((CISOSetParent*)m_pSystemObject)->m_DataType.m_nType)
			{
			case T_OBJECT:
				((CISOSet<CISystemObject*>*)m_pSystemObject)->GetSystemObjectsAndStructures( tpSystemObjects, tpStructures);
				break;
			case T_STRUCT:
				((CISOSet<CIStructure*>*)m_pSystemObject)->GetSystemObjectsAndStructures( tpSystemObjects, tpStructures);
				break;
			}
			break;
		}

		break;
	case T_STRUCT:
		tpStructures.SetAt( m_pStruct, m_pStruct);
		break;
	}
}
*/
//////////////////////////////////////////////////////////////////////
// CIDataStackItemSet
//////////////////////////////////////////////////////////////////////

DEFINE_MULTITHREAD_POOL( CIDataStackItemSet, 20);

void CIDataStackItemSet::Clean()
{
    int i;

    for (i=0; i<DATA_STACK_ITEM_SET_SIZE; i++) m_pItems[i].Clean();
}

// SAVE & LOAD

void CIDataStackItemSet::PersistentSave( CPersistentStorage &storage)
{
    BRACE_BLOCK(storage);

    int i;
	for (i=0; i<DATA_STACK_ITEM_SET_SIZE; i++) m_pItems[i].PersistentSave(storage);
}

void CIDataStackItemSet::PersistentLoad( CPersistentStorage &storage)
{
    BRACE_BLOCK(storage);

    int i;
	for (i=0; i<DATA_STACK_ITEM_SET_SIZE; i++) m_pItems[i].PersistentLoad(storage);
}

void CIDataStackItemSet::PersistentTranslatePointers( CPersistentStorage &storage)
{
	int i;
	for (i=0; i<DATA_STACK_ITEM_SET_SIZE; i++) m_pItems[i].PersistentTranslatePointers(storage);
}

void CIDataStackItemSet::PersistentInit()
{
	int i;
	for (i=0; i<DATA_STACK_ITEM_SET_SIZE; i++) m_pItems[i].PersistentInit();
}

//////////////////////////////////////////////////////////////////////
// CIDataStack
//////////////////////////////////////////////////////////////////////

CIDataStack::CIDataStack()
{
    int i;

    m_apDSItemSets.SetSize(5, 5);
    for (i=0; i<5; i++) m_apDSItemSets[i]=NULL;

    m_nDSItemSetCount = 0;
    m_bCreated = false;
}

//////////////////////////////////////////////////////////////////////

CIDataStack::~CIDataStack()
{
    if (m_bCreated) Delete();
    m_apDSItemSets.RemoveAll();
}

//////////////////////////////////////////////////////////////////////

void CIDataStack::Create(CInterpret *pInterpret)
{
    ASSERT(!m_bCreated);
    ASSERT(pInterpret != NULL);
    
    m_pInterpret = pInterpret;

    // nastavi si prvni set
    m_apDSItemSets[0] = new CIDataStackItemSet;
    ASSERT(m_apDSItemSets[0] != NULL);

    m_nDSItemSetCount = 1;

    m_bCreated = true;
}

//////////////////////////////////////////////////////////////////////

void CIDataStack::Delete()
{
    if(m_bCreated) {

        while (m_nDSItemSetCount > 0)
        {
            // uvolneni drzenych objektu
            m_apDSItemSets[m_nDSItemSetCount-1]->Clean();
            // pridani do poolu
            delete m_apDSItemSets[m_nDSItemSetCount-1];

            m_nDSItemSetCount--;
        }

        m_bCreated = false;
    
    }
}

//////////////////////////////////////////////////////////////////////

CIDataStackItem& CIDataStack::operator[] (int nIndex)
{
    ASSERT(nIndex>=0);

    int nSetIndex = nIndex >> DATA_STACK_ITEM_SET_BITS;

    while (nSetIndex >= m_nDSItemSetCount)
    {
        // musim pridat novy set
        CIDataStackItemSet *pDSI = new CIDataStackItemSet;
        ASSERT( pDSI != NULL);
        m_apDSItemSets.SetAtGrow( m_nDSItemSetCount, pDSI);

        m_nDSItemSetCount++;
    }

	CIDataStackItemSet *pSet = m_apDSItemSets[nSetIndex];
    return pSet->m_pItems[nIndex % DATA_STACK_ITEM_SET_SIZE];
}

//////////////////////////////////////////////////////////////////////

void CIDataStack::FreeSetsFromRegister(REG nReg)
{
    ASSERT( nReg != NO_REG);

    int i, nSetIndex = nReg >> DATA_STACK_ITEM_SET_BITS;

    // musi to byt prvni registr z setu - vraceny metodou GetFirstSurelyUnusedRegister();
    ASSERT( nReg % DATA_STACK_ITEM_SET_SIZE == 0);
    
    for (i=nSetIndex; i<m_nDSItemSetCount; i++)
    {
        m_apDSItemSets[i]->Clean();
        // pridani do poolu
        delete m_apDSItemSets[i];
    }

    m_nDSItemSetCount = nSetIndex;
}

//////////////////////////////////////////////////////////////////////
// SAVE & LOAD
//////////////////////////////////////////////////////////////////////
    
void CIDataStack::PersistentSave( CPersistentStorage &storage)
{
    BRACE_BLOCK(storage);

    // pointer na sebe
	storage << (DWORD)this;

    // prislusny interpret
    storage << (DWORD)m_pInterpret;

    // pocet setu
    storage << m_nDSItemSetCount;

    // pole CIDataStackItem
	int i;
	for (i=0; i<m_nDSItemSetCount; i++)
	{
		m_apDSItemSets[i]->PersistentSave( storage);
	}
}

void CIDataStack::PersistentLoad( CPersistentStorage &storage)
{
    BRACE_BLOCK(storage);

    // pointer na sebe
	CIDataStack *pThis;
	storage >> (DWORD&)pThis;
	storage.RegisterPointer( pThis, this);

    // prislusny interpret
    storage >> (DWORD&)m_pInterpret;

    // pocet setu
    storage >> m_nDSItemSetCount;

    // pole CIDataStackItem
	m_apDSItemSets.SetSize( m_nDSItemSetCount);
	
	int i;
	for (i=0; i<m_nDSItemSetCount; i++)
	{
		m_apDSItemSets[i] = new CIDataStackItemSet;
		m_apDSItemSets[i]->PersistentLoad( storage);
	}
}

void CIDataStack::PersistentTranslatePointers( CPersistentStorage &storage)
{
    // prislusny interpret
    m_pInterpret = (CInterpret*)storage.TranslatePointer( m_pInterpret);

	int i;
	for (i=0; i<m_nDSItemSetCount; i++)
	{
		m_apDSItemSets[i]->PersistentTranslatePointers( storage);
	}
}

void CIDataStack::PersistentInit()
{
	int i;
	for (i=0; i<m_nDSItemSetCount; i++)
	{
		m_apDSItemSets[i]->PersistentInit();
	}

	m_bCreated = true;
}


//////////////////////////////////////////////////////////////////////
