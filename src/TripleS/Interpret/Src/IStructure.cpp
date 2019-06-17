// IStructure.cpp: implementation of the CIStructure class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\Interpret.h"

#include "TripleS\CodeManager\CodeManager.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CIStructure::CIStructure()
{
    m_pData = NULL;
    m_pStructType = NULL;
}

CIStructure::~CIStructure()
{
}

void CIStructure::Create(CInterpret *pInterpret, CIStructureType *pStructType, bool bCreateMembers)
{
    ASSERT(pStructType != NULL);

	m_pInterpret = pInterpret;
    m_pStructType = pStructType;

    if (m_pStructType->m_nHeapSize != 0) 
    {
        m_pData = new int[m_pStructType->m_nHeapSize];
        ::ZeroMemory(m_pData, m_pStructType->m_nHeapSize*sizeof(int));

        if (bCreateMembers)
        {
            POSITION pos;
            CStringTableItem *stiName;
            CIMember *pMember;

            pos = m_pStructType->m_tpMembers.GetStartPosition();
            while (pos != NULL)
            {
                m_pStructType->m_tpMembers.GetNextAssoc(pos, stiName, pMember);
            
                // systemovy objekt
                if (pMember->m_DataType.IsObject())
                {
                    m_pData[ pMember->m_nHeapOffset] = (int) pInterpret->CreateSystemObject(pMember->m_DataType);
                }
                // struktura
                else if (pMember->m_DataType.IsStruct())
                {
                    ASSERT(pMember->m_DataType.m_stiName != NULL);

                    // vytvor instanci struktury, pokud ovsem neni stejneho typu (aby se to nezacyklilo)
                    if (pMember->m_DataType.m_stiName != m_pStructType->m_stiName)
                    {
                        CIStructureType *pStructType;
                        if (( pStructType = pInterpret->m_pCodeManager->GetStructureType( pMember->m_DataType.m_stiName)) != NULL)
                        {
			                m_pData[ pMember->m_nHeapOffset] = (int) pInterpret->CreateStructure( pStructType, false);
                        }
                    }
                }
                // ostatni typy jsou inicializovany na 0 nebo NULL
            }
        }
    }
}

void CIStructure::CopyDataFrom( CIStructure *pFrom)
{
    ASSERT(pFrom != NULL);

    POSITION pos;
    CStringTableItem *stiName;
    CIMember *pMember;

	// zvysit refcount systemovym objektum, jednotkam nebo strukturam 
    pos = m_pStructType->m_tpMembers.GetStartPosition();
    while (pos != NULL)
    {
        m_pStructType->m_tpMembers.GetNextAssoc(pos, stiName, pMember);

		if (((CIObjUnitStruct*)m_pData[ pMember->m_nHeapOffset]) != NULL)
		{
      if ( (CIObjUnitStruct*)m_pData[ pMember->m_nHeapOffset] != NULL)
      {
			  ((CIObjUnitStruct*)m_pData[ pMember->m_nHeapOffset])->Release();
      }
		}

		m_pData[ pMember->m_nHeapOffset] = pFrom->m_pData[ pMember->m_nHeapOffset];
		if (pMember->m_DataType.IsFloat()) m_pData[ pMember->m_nHeapOffset + 1] = pFrom->m_pData[ pMember->m_nHeapOffset + 1];
    
        if (pMember->m_DataType.IsObjectUnitStruct())
        {
          if ((CIObjUnitStruct*)m_pData[ pMember->m_nHeapOffset] != NULL)
          {
            ((CIObjUnitStruct*)m_pData[ pMember->m_nHeapOffset])->AddRef();
          }
        }
	}
}

void CIStructure::Delete()
{
    // vzpamatovani po padu pri loadovani hry
    if ( g_bGameLoading)
    {
        delete m_pData;
        return;
    }

    // uvolneni polozek
    POSITION pos;
    CStringTableItem *stiName;
    CIMember *pMember;

    pos = m_pStructType->m_tpMembers.GetStartPosition();
    while (pos != NULL)
    {
        m_pStructType->m_tpMembers.GetNextAssoc( pos, stiName, pMember);
        if (pMember->m_DataType.IsObjectUnitStruct())
        {
            if (m_pData[ pMember->m_nHeapOffset] != NULL)
            {
                ((CIObjUnitStruct*)m_pData[ pMember->m_nHeapOffset])->Release();
            }
        }
    }
    
    // uvolneni pameti na polozky
    delete [] m_pData;
    m_pData = NULL;

    m_pStructType = NULL;
}

CIObjUnitStruct * CIStructure::AddRef()    
{ 
    ::InterlockedIncrement(&m_nRefCount); 
    return this; 
}

CIObjUnitStruct * CIStructure::Release()   
{ 
    if (::InterlockedDecrement(&m_nRefCount) == 0) 
    {
		m_pInterpret->UnregisterStructure( this);
        Delete();
        delete this; 
    }
    return NULL;
}

bool CIStructure::LoadMember( CIDataStackItem &Reg, CStringTableItem *stiName)
{
    ASSERT(stiName != NULL);

    CIMember *pMember = NULL;

    switch (m_pStructType->GetMember( stiName, pMember))
    {
    case IMPL_NONE:
        return false;

    case IMPL_SCRIPT:
        Reg.Load(pMember->m_DataType, m_pData + pMember->m_nHeapOffset);
        return true;

    case IMPL_CPLUSPLUS:
        ASSERT(false);
    }

    return false;
}

bool CIStructure::StoreMember( CIDataStackItem *pReg, CStringTableItem *stiName)
{
    ASSERT(stiName != NULL);

    CIMember *pMember = NULL;

    switch (m_pStructType->GetMember( stiName, pMember))
    {
    case IMPL_NONE:
        return false;

    case IMPL_SCRIPT:
		if (pMember->m_DataType.IsObjectUnitStruct() && pReg == NULL)
		{
			if (((CIObjUnitStruct*)*(m_pData + pMember->m_nHeapOffset)) != NULL)
			{
				((CIObjUnitStruct*)*(m_pData + pMember->m_nHeapOffset))->Release();
			}
			*(m_pData + pMember->m_nHeapOffset) = NULL;
			return true;
		}
		else
		{
			ASSERT( pReg != NULL);

			if (pReg->SameTypeAsAndConvertUnits(pMember->m_DataType))
			{
				pReg->Store( m_pData + pMember->m_nHeapOffset);
				return true;
			}
			else
			{
				TRACE_INTERPRET2("- %s::%s: STORESM: type mismatch\n", 
					(CString)*m_pStructType->m_stiName,
					(CString)*stiName);
				ASSERT(false);
				return false;
			}
		}
        break;

    case IMPL_CPLUSPLUS:
        ASSERT(false);
    }

    return false;
}

//////////////////////////////////////////////////////////////////////////////////////

void CIStructure::PersistentSave( CPersistentStorage &storage)
{
    BRACE_BLOCK(storage);

    // predek
	CIObjUnitStruct::PersistentSave( storage);

    // interpret
    storage << (DWORD)m_pInterpret;

    // typ struktury
    storage << (DWORD)m_pStructType;

    // datove polozky    
	storage << m_pStructType->m_nHeapSize;
    if (m_pStructType->m_nHeapSize != 0) storage.Write( m_pData, m_pStructType->m_nHeapSize * sizeof(int));
}

//////////////////////////////////////////////////////////////////////////////////////
	
void CIStructure::PersistentLoad( CPersistentStorage &storage)
{
    BRACE_BLOCK(storage);

    // predek
	CIObjUnitStruct::PersistentLoad( storage);

    // interpret
    storage >> (DWORD&)m_pInterpret;
    
    // typ struktury
    storage >> (DWORD&)m_pStructType;

    // datove polozky    
	long nHeapSize;
	storage >> nHeapSize;
	if (nHeapSize != 0)
	{
		m_pData = new int[nHeapSize];
		storage.Read( m_pData, nHeapSize * sizeof(int));
	}
	else m_pData = NULL;
}

//////////////////////////////////////////////////////////////////////////////////////

void CIStructure::PersistentTranslatePointers( CPersistentStorage &storage)
{
	// predek
	CIObjUnitStruct::PersistentTranslatePointers( storage);

    // interpret
    m_pInterpret = (CInterpret*) storage.TranslatePointer( m_pInterpret);

	// typ sturktury
	m_pStructType = (CIStructureType*) storage.TranslatePointer( m_pStructType);

	// datove polozky
	POSITION pos = m_pStructType->m_tpMembers.GetStartPosition();
	CStringTableItem *stiName;
	CIMember *pMember;
	while (pos != NULL)
	{
		m_pStructType->m_tpMembers.GetNextAssoc( pos, stiName, pMember);
		if (pMember->m_DataType.IsObjectUnitStruct())
		{
			m_pData[ pMember->m_nHeapOffset] = (int) storage.TranslatePointer((void*)*(m_pData + pMember->m_nHeapOffset));
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////

void CIStructure::PersistentInit()
{
	// predek
	CIObjUnitStruct::PersistentInit();
}

//////////////////////////////////////////////////////////////////////////////////////
