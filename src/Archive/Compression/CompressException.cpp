// CompressException.cpp: implementation of the CCompressException class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CompressException.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCompressException::CCompressException()
{
	m_eCode=ECompressExceptionCode::compressOK;
	m_eLocation=ECompressExceptionLocation::unknownLocation;
	m_pEngine=NULL;
	m_strDescription="";
}

CCompressException::CCompressException(ECompressExceptionLocation eLocation, CString strDescription)
{
	m_eCode=ECompressExceptionCode::compressOK;
	m_eLocation=eLocation;
	m_pEngine=NULL;
	m_strDescription=strDescription;	
}

CCompressException::CCompressException(CCompressException *pException, CCompressEngine *pEngine) {
	m_pEngine=pEngine;
	m_eCode=pException->m_eCode;
	m_eLocation=pException->m_eLocation;
	m_strDescription=pException->m_strDescription;
	pException->Delete();
}

CCompressException::CCompressException(ECompressExceptionCode eCode, CCompressEngine *pEngine) {
	m_eCode=eCode;
	m_eLocation=ECompressExceptionLocation::engineItself;
	m_pEngine=pEngine;
	m_strDescription="";
}

CCompressException::~CCompressException()
{

}

BOOL CCompressException::GetErrorMessage(LPTSTR lpszError, UINT nMaxError, PUINT pnHelpContext) {
	strcpy(lpszError, (LPCTSTR)(GetErrorMessage().Left(nMaxError-1)));
	return TRUE;
}

CString CCompressException::GetErrorMessage() {
	CString strMsg;
	if(m_strDescription.IsEmpty()) {
		switch(m_eCode) {
		case compressOK:
			m_strDescription="no error";
			break;
		case errnoError:
			m_strDescription="unknown error";
			break;
		case streamError:
			m_strDescription="stream error";
			break;
		case dataError:
			m_strDescription="data error";
			break;
		case memError:
			m_strDescription="mem error";
			break;
		case bufError:
			m_strDescription="buffer error";
			break;
		case versionError:
			m_strDescription="wrong version";
			break;
		default:
			m_strDescription="unknown error";
		}
	}
	switch(m_eLocation) {
	case unknownLocation:
		strMsg="?: ";
		break;
	case engineItself:
		if(m_pEngine) {
			strMsg=m_pEngine->GetID() + ": ";
		}
		break;
	case dataSource:
		if(m_pEngine) {
			strMsg=m_pEngine->GetID()+": data source: ";
		}
	case dataSink:
		if(m_pEngine) {
			strMsg=m_pEngine->GetID()+": data sink: ";
		}
	default:
		strMsg="";
	}
	return strMsg+m_strDescription;
}
