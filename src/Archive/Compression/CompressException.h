// CompressException.h: interface for the CCompressException class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_COMPRESSEXCEPTION_H__0B32E32F_9569_11D3_BF6E_CD1501B48D07__INCLUDED_)
#define AFX_COMPRESSEXCEPTION_H__0B32E32F_9569_11D3_BF6E_CD1501B48D07__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "CompressEngine.h"

class CCompressException : public CException  
{
public:
	enum ECompressExceptionCode;
	enum ECompressExceptionLocation;

public:
	// Konstruktor
	CCompressException();

	// Konstruktor
	CCompressException(ECompressExceptionLocation eLocation, CString strDescription="");

	// Konstruktor
	CCompressException(ECompressExceptionCode eCode, CCompressEngine *pEngine);

	// Konstruktor pridavajici odkaz na engine a rusici starou vyjimku
	CCompressException(CCompressException *pException, CCompressEngine *pEngine);

	// Destruktor
	virtual ~CCompressException();

	// Vraci popis chyby
	virtual BOOL GetErrorMessage(LPTSTR lpszError, UINT nMaxError, PUINT pnHelpContext = NULL);

	// Vraci popis chyby
	virtual CString GetErrorMessage();

public:
	enum ECompressExceptionCode {
		compressOK	=0,
		errnoError	= Z_ERRNO,
		streamError	= Z_STREAM_ERROR,
		dataError	= Z_DATA_ERROR,
		memError	= Z_MEM_ERROR,
		bufError	= Z_BUF_ERROR,
		versionError=Z_VERSION_ERROR
	};

	enum ECompressExceptionLocation {
		unknownLocation	=0,
		engineItself	=1,
		dataSource		=2,
		dataSink		=3
	};

private:
	// Kod chyby
	ECompressExceptionCode m_eCode;

	// Popis chyby
	CString m_strDescription;

	// Engine, kde chyba nastala
	CCompressEngine *m_pEngine;

	// Kde presne v ramci enginu nastala
	ECompressExceptionLocation m_eLocation;
};

#endif // !defined(AFX_COMPRESSEXCEPTION_H__0B32E32F_9569_11D3_BF6E_CD1501B48D07__INCLUDED_)
