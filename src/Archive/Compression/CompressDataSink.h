// CompressDataSink.h: interface for the CCompressDataSink class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_COMPRESSDATASINK_H__0B32E32E_9569_11D3_BF6E_CD1501B48D07__INCLUDED_)
#define AFX_COMPRESSDATASINK_H__0B32E32E_9569_11D3_BF6E_CD1501B48D07__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "includes.h"

class CCompressDataSink : public CObject  
{
public:
	// Konstruktor
	CCompressDataSink();

	// Destruktor
	virtual ~CCompressDataSink();

	// Ulozi blok dat, ktery se mu podstrci
	virtual ECompressData StoreData(void *lpBuf, DWORD dwSize)=0;

	// Vrati buffer, pomoci ktereho se bude ukladat
	virtual void GetStoreBuffer(void *&pBuffer, DWORD &dwSize);

	// Vrati nejaky retezec umoznujici identifikaci (napr. cestu k souboru, kam sink pise)
	virtual CString GetID()=0;

protected:
	// Buffer pro ukladani
	void *m_pStoreBuffer;

	// Velikost bufferu pro ukladani
	DWORD m_dwBufferSize;
};

#endif // !defined(AFX_COMPRESSDATASINK_H__0B32E32E_9569_11D3_BF6E_CD1501B48D07__INCLUDED_)
