// ISyscallSome.h: interface for the CISyscallSome class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ISYSCALLSOME_H__32218D25_ED20_11D3_AFF3_004F49068BD6__INCLUDED_)
#define AFX_ISYSCALLSOME_H__32218D25_ED20_11D3_AFF3_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\ISyscall.h"

class CISyscallSome : public CISyscall  
{
public:
    CISyscallSome() {}
    virtual ~CISyscallSome() {}

    virtual bool Create( CIProcess*, CIBag *pBag);

	void PersistentSave( CPersistentStorage &storage);
	void PersistentLoad( CPersistentStorage &storage);				
	void PersistentTranslatePointers( CPersistentStorage &storage);	
	void PersistentInit();											

};

#endif // !defined(AFX_ISYSCALLSOME_H__32218D25_ED20_11D3_AFF3_004F49068BD6__INCLUDED_)
