// ISOArrayParent.h: interface for the CISOArrayParent class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ISOARRAYPARENT_H__380C8AF6_DFDE_11D3_AFE0_004F49068BD6__INCLUDED_)
#define AFX_ISOARRAYPARENT_H__380C8AF6_DFDE_11D3_AFE0_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IDuplicSystemObject.h"

// Spolecny abstraktni predek pro vsechny typovane tridy CISOArray.
class CISOArrayParent  : public CIDuplicSystemObject  
{
public:
    CISOArrayParent() { m_nSOType = SO_ARRAY; }
    virtual ~CISOArrayParent() {}

    // pro pole se musi volat Create s parametrem velikost pole a typem polozek
    virtual bool Create() { ASSERT(false); return false; }    
    virtual bool Create(CInterpret *pInterpret, CType *pType, int nArraySize) = 0;

    // load & store polozek objektu
    virtual bool LoadMember( CIDataStackItem&, long) { return false; }
    virtual bool StoreMember( CIDataStackItem*, long) { return false; }

    // indexace [] - preda se ukazatel na data daneho typu, v typovanych
    // naslednicich bude spravne prirazeni. Tohle je kvuli tomu, aby v LOADIX a STOREIX
    // nemusel byt switch.
    virtual bool SetAt(int i, CIDataStackItem *pDSI) = 0;
    virtual bool GetAt(int i, CIDataStackItem &DSI) = 0;

// polozky
public:
    // velikost pole
    int m_nArraySize;
    // typ, ktery je v poli
    CType m_DataType;

public:
// 
// SAVE & LOAD
//
	void PersistentSave( CPersistentStorage &storage)
	{
        BRACE_BLOCK(storage);

		CIDuplicSystemObject::PersistentSave(storage);
		m_DataType.PersistentSave( storage);
		storage << m_nArraySize;
	}

	void PersistentLoad( CPersistentStorage &storage)
	{
        BRACE_BLOCK(storage);

		CIDuplicSystemObject::PersistentLoad(storage);
		m_DataType.PersistentLoad( storage);
		storage >> m_nArraySize;
	}

	void PersistentTranslatePointers( CPersistentStorage &storage)
	{
		CIDuplicSystemObject::PersistentTranslatePointers(storage);
		m_DataType.PersistentTranslatePointers( storage);
	}

	void PersistentInit()
	{
		CIDuplicSystemObject::PersistentInit();
		m_DataType.PersistentInit();
	}
};

#endif // !defined(AFX_ISOARRAYPARENT_H__380C8AF6_DFDE_11D3_AFE0_004F49068BD6__INCLUDED_)
