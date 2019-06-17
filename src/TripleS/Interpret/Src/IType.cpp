/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Interpret
 *   Autor: Helena Kupková
 *  
 *   Popis: Implementace tridy CIType uchovavajici informace o datovem typu
 *          nejake polozky, argumentu atp.          
 *          
 ***********************************************************/

#include "stdafx.h"
#include "..\Interpret.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CIType::CIType()
    : CType()
{
    m_nCodeStringTableID = -1;
    m_pOfType = NULL;
    m_bLoaded = false;
}

CIType::~CIType()
{
    SetNoneType();
}

#ifdef _DEBUG
void CIType::AssertValid() const
{
    CType::AssertValid();
    ASSERT(m_bLoaded);
}
#endif

void CIType::SetNoneType()
{
    CType::SetNone();
    m_nCodeStringTableID = -1;
}

void CIType::Load(CFile &file)
{
	short i,size;
	char c;

    ASSERT(!m_bLoaded);
	SetNoneType();
    
	//delka zaznamu
	file.Read(&i,sizeof(short));

	//zakladni typ
	file.Read(&m_nType, sizeof(short));

	// objektovy typ
	if (IsObjectUnitStruct())
	{
        // Po nacteni celeho skriptu se ID typu prelozi funkci Translate
        // dle stringtable u kodu a m_nCodeStringTableID ztrati vyznam.
		file.Read(&m_nCodeStringTableID, sizeof(long));
	}
    m_stiName=NULL;

	// array level (>0) / set flag (==-1)
	file.Read(&i, sizeof(short));

    if (i==-1) // mnozina
    {
        MakeSet();
    }
    else if (i>0)
	{
        // priznak zda jsou ulozeny delky poli
		file.Read(&c, sizeof(char));
        // nacteni delek poli
		while (i>0)
		{
			if (c!=0) file.Read(&size, sizeof(short));
			else size=0;
			MakeArray(size);
			i--;
		}
	}
}

void CIType::Translate(CICodeStringTable *pCST) 
    // tato metoda smi byt zavolana prave jednou
{
    CType *pBasicType = this;

	if ( pBasicType->IsArray())
	{
		while (pBasicType->IsArray())
		{
			pBasicType = pBasicType->m_pOfType;
			ASSERT(pBasicType!=NULL);
		}
	}
	else if (pBasicType->IsSet()) pBasicType = pBasicType->m_pOfType;

    if (pBasicType->IsObjectUnitStruct())
    {
        ASSERT(m_nCodeStringTableID!=-1);
        ASSERT(pBasicType->m_stiName==NULL);
    
        if ( pBasicType->IsObject()) 
        {
            pBasicType->m_nSOType = GetSOTypeFromString((CString)*(*pCST)[m_nCodeStringTableID]);
            ASSERT( pBasicType->m_nSOType != SO_UNKNOWN);
        }
        else pBasicType->m_stiName=(*pCST)[m_nCodeStringTableID]->AddRef();
    }

    m_bLoaded = true;
}

void CIType::Set(CString &strType)
{
    if (strType=="char") CType::Set(T_CHAR);
    else if (strType=="void") CType::SetVoid();
    else if (strType=="int") CType::Set(T_INT);
    else if (strType=="float") CType::Set(T_FLOAT);
    else if (strType=="bool") CType::Set(T_BOOL);
    else 
	{
        // volani CIType::Set() na typ, ktery neni simple!!
        ASSERT(false);
	}
}

