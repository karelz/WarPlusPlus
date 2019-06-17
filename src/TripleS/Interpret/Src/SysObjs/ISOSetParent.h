// ISOSetParent.h: interface for the CISOSetParent class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ISOSETPARENT_H__380C8AF6_DFDE_11D3_AFE0_004F49068BD6__INCLUDED_)
#define AFX_ISOSETPARENT_H__380C8AF6_DFDE_11D3_AFE0_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IDuplicSystemObject.h"

// Spolecny abstraktni predek pro vsechny typovane tridy CISOSet.
class CISOSetParent  : public CIDuplicSystemObject  
{
public:
    CISOSetParent() { m_nSOType = SO_SET; }
    virtual ~CISOSetParent() {}

    // pro pole se musi volat Create s parametrem typ polozek
    virtual bool Create() { ASSERT(false); return false; }    
    virtual bool CreateEmpty( CInterpret *pInterpret, CType *pType) = 0;
    
    virtual bool CreateFromUnion( CInterpret *pInterpret, CISOSetParent *pSet1, CISOSetParent *pSet2) = 0;
    virtual bool CreateFromIntersection( CInterpret *pInterpret, CISOSetParent *pSet1, CISOSetParent *pSet2) = 0;
    virtual bool CreateFromDifference( CInterpret *pInterpret, CISOSetParent *pSet1, CISOSetParent *pSet2) = 0;

    // load & store polozek objektu
    virtual bool LoadMember( CIDataStackItem&, long) { return false; }
    virtual bool StoreMember( CIDataStackItem*, long) { return false; }

    // dotaz zda je prvek v mnozine
    virtual EInterpretStatus Contains( CIDataStackItem &DSI, bool &bResult) = 0;

    virtual EInterpretStatus AddItemToSet( CIDataStackItem &DSI) = 0;
    virtual EInterpretStatus RemoveItemFromSet( CIDataStackItem &DSI) = 0;

    virtual void UnionWith( CISOSetParent *pSet) = 0;
    virtual void IntersectWith( CISOSetParent *pSet) = 0;
    virtual void DifferWith( CISOSetParent *pSet) = 0;

    virtual bool SetForFirst( CIDataStackItem &Var, CIDataStackItem &BoolRes) = 0;
    virtual void SetForNext( CIDataStackItem &Var, CIDataStackItem &BoolRes) = 0;
    virtual void SetForEnd() = 0;

// polozky
public:
    // typ, ktery je v mnozine
    CType m_DataType;
    // pocet polozek v mnozine
    int m_nItemCount;

public:
// 
// SAVE & LOAD
//
	void PersistentSave( CPersistentStorage &storage)
	{
        BRACE_BLOCK(storage);

        CIDuplicSystemObject::PersistentSave(storage);
		m_DataType.PersistentSave( storage);
		storage << m_nItemCount;
	}

	void PersistentLoad( CPersistentStorage &storage)
	{
        BRACE_BLOCK(storage);

		CIDuplicSystemObject::PersistentLoad(storage);
		m_DataType.PersistentLoad( storage);
		storage >> m_nItemCount;
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

#endif // !defined(AFX_ISOSETPARENT_H__380C8AF6_DFDE_11D3_AFE0_004F49068BD6__INCLUDED_)
