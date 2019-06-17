// IAbstrObjectType.h: interface for the CIObjUnitStruct   class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IABSTROBJECTTYPE_H__27329694_AD4E_11D3_AF9A_004F49068BD6__INCLUDED_)
#define AFX_IABSTROBJECTTYPE_H__27329694_AD4E_11D3_AF9A_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CIObjUnitStruct : public CPersistentObject
{
    DECLARE_DYNAMIC(CIObjUnitStruct);

public:
	CIObjUnitStruct  ();
	virtual ~CIObjUnitStruct  ();

    virtual CIObjUnitStruct* AddRef() = 0;
    virtual CIObjUnitStruct* Release() = 0;
    virtual void Delete() = 0;

protected:
    // pocitadlo odkazu
    long m_nRefCount;

#ifdef _DEBUG
    void AssertValid() const;
#endif

//
//	SAVE & LOAD
//
	void PersistentSave( CPersistentStorage &storage);
	void PersistentLoad( CPersistentStorage &storage);
	void PersistentTranslatePointers( CPersistentStorage &storage);
	void PersistentInit();
};

#endif // !defined(AFX_IABSTROBJECTTYPE_H__27329694_AD4E_11D3_AF9A_004F49068BD6__INCLUDED_)
