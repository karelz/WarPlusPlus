// IDuplicableSystemObject.h: interface for the CIDuplicSystemObject class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IDUPLICABLESYSTEMOBJECT_H__2F55EE22_DC88_11D3_AF88_DCCBF4444946__INCLUDED_)
#define AFX_IDUPLICABLESYSTEMOBJECT_H__2F55EE22_DC88_11D3_AF88_DCCBF4444946__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ISystemObject.h"

class CIDuplicSystemObject : public CISystemObject  
{
public:
	CIDuplicSystemObject() : CISystemObject()  {  m_bCanBeDuplicated = true; }
	virtual ~CIDuplicSystemObject() {}

	virtual bool CreateFrom( CInterpret *pInterpret, CIDuplicSystemObject *pSrc) = 0;
};

#endif // !defined(AFX_IDUPLICABLESYSTEMOBJECT_H__2F55EE22_DC88_11D3_AF88_DCCBF4444946__INCLUDED_)
