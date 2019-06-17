// IDCachedObject.h: interface for the CIDCachedObject class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IDCACHEDOBJECT_H__14F8F163_1047_11D4_8040_0000B4A08F9A__INCLUDED_)
#define AFX_IDCACHEDOBJECT_H__14F8F163_1047_11D4_8040_0000B4A08F9A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "CachedObject.h"

class CIDCachedObject : public CCachedObject 
{
  DECLARE_DYNAMIC(CIDCachedObject);

// Konstrukce a destrukce
public:
	// Konstruktor
	CIDCachedObject();

	// Destruktor
	virtual ~CIDCachedObject();

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

// Metody pro praci s ID
public:
	// Vraci ID
	DWORD GetID() { return m_dwID; }

	// Nastavuje ID
	DWORD SetID(DWORD dwID) { m_dwID=dwID; return dwID; }

private:
	// Identifikace
	DWORD m_dwID;
};

#endif // !defined(AFX_IDCACHEDOBJECT_H__14F8F163_1047_11D4_8040_0000B4A08F9A__INCLUDED_)
