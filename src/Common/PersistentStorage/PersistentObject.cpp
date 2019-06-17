// PersistentObject.cpp: implementation of the CPersistentObject class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PersistentObject.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CPersistentObject, CObject);

CPersistentObject::CPersistentObject()
{

}

CPersistentObject::~CPersistentObject()
{

}

#ifdef _DEBUG

void CPersistentObject::AssertValid() const
{
  CObject::AssertValid();
}

void CPersistentObject::Dump(CDumpContext &dc) const
{
  CObject::Dump(dc);
}

#endif