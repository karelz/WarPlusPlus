// IDCachedObject.cpp: implementation of the CIDCachedObject class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IDCachedObject.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CIDCachedObject, CCachedObject)

CIDCachedObject::CIDCachedObject()
{

}

CIDCachedObject::~CIDCachedObject()
{

}

#ifdef _DEBUG

void CIDCachedObject::AssertValid() const
{
  CCachedObject::AssertValid();
}

void CIDCachedObject::Dump(CDumpContext &dc) const
{
  CCachedObject::Dump(dc);
  dc << "CIDCachedObject. ID = " << m_dwID << "\n";
}

#endif