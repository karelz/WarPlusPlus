// IStructure.h: interface for the CIStructure class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ISTRUCTURE_H__27329693_AD4E_11D3_AF9A_004F49068BD6__INCLUDED_)
#define AFX_ISTRUCTURE_H__27329693_AD4E_11D3_AF9A_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CIDataStackItem;

class CIStructure : public CIObjUnitStruct 
{
public:
	CIStructure();
	virtual ~CIStructure();

    virtual void Create( CInterpret *pInterpret, CIStructureType *pStructType, bool bCreateMembers);
    virtual void Delete();

    virtual CIObjUnitStruct* AddRef();
    virtual CIObjUnitStruct* Release();

    // returns false kdyz member nenalezen
    bool LoadMember( CIDataStackItem &Reg, CStringTableItem *stiName);
    bool StoreMember( CIDataStackItem *pReg, CStringTableItem *stiName);

	void CIStructure::CopyDataFrom( CIStructure *pFrom);

protected:
	CInterpret *m_pInterpret;

public:
    // typ struktury
    CIStructureType *m_pStructType;

    // datove polozky    
    int *m_pData;

public:
//
//	SAVE & LOAD
//
	void PersistentSave( CPersistentStorage &storage);
	void PersistentLoad( CPersistentStorage &storage);
	void PersistentTranslatePointers( CPersistentStorage &storage);
	void PersistentInit();

};

#endif // !defined(AFX_ISTRUCTURE_H__27329693_AD4E_11D3_AF9A_004F49068BD6__INCLUDED_)
