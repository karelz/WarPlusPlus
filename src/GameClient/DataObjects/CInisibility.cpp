// CInvisibility.cpp: implementation of the CCInvisibility class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CInvisibility.h"

#include "Common\Map\Map.h"
#include "..\LoadException.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CCInvisibility, CObject)

// constructor
CCInvisibility::CCInvisibility()
{
}

// destructor
CCInvisibility::~CCInvisibility()
{
  ASSERT(m_strName.IsEmpty());
}

// Debug functions
#ifdef _DEBUG

void CCInvisibility::AssertValid() const
{
  CObject::AssertValid();
}

void CCInvisibility::Dump(CDumpContext &dc) const
{
  CObject::Dump(dc);
}

#endif

// Creates the object from the MapFile (loads it)
void CCInvisibility::Create(CArchiveFile MapFile)
{
  SInvisibility h;

  // Load our header from the file
  LOAD_ASSERT ( MapFile.Read ( &h, sizeof ( h ) ) == sizeof ( h ) );

  // copy values
  m_strName = h.m_szName;
}

// Deletes the object
void CCInvisibility::Delete()
{
  m_strName.Empty();
}
