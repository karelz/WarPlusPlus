// CompressDataSource.h: interface for the CCompressDataSource class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_COMPRESSDATASOURCE_H__0B32E32D_9569_11D3_BF6E_CD1501B48D07__INCLUDED_)
#define AFX_COMPRESSDATASOURCE_H__0B32E32D_9569_11D3_BF6E_CD1501B48D07__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "includes.h"

class CCompressDataSource : public CObject
{
public:
	// Konstruktor
	CCompressDataSource();

	// Destruktor
	virtual ~CCompressDataSource();

	// Vyprodukuje blok dat a vrati jeho velikost
	virtual ECompressData GiveData(void *&lpBuf, DWORD &dwSize)=0;

	// Vrati nejaky retezec umoznujici identifikaci (napr. cestu k souboru, odkud source cte)
	virtual CString GetID()=0;

	// Vrati velikost nekomprimovaneho souboru
	virtual DWORD GetUncompressedSize()=0;

};

#endif // !defined(AFX_COMPRESSDATASOURCE_H__0B32E32D_9569_11D3_BF6E_CD1501B48D07__INCLUDED_)
