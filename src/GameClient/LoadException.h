#ifndef GAMECLIENT_LOADEXCEPTION_H_
#define GAMECLIENT_LOADEXCEPTION_H_

#include "resource.h"

class CLoadException : public CException
{
public:
	CLoadException () {}
	CLoadException ( const CLoadException & src ) {}
	virtual ~CLoadException () {}

	virtual BOOL GetErrorMessage( LPTSTR lpszError, UINT nMaxError, PUINT pnHelpContext = NULL )
	{
		CString strLoadError;
		strLoadError.LoadString ( IDS_LOADERROR );
		strncpy ( lpszError, (LPCSTR)strLoadError, nMaxError );
		lpszError [ nMaxError - 1 ] = 0;
		return TRUE;
	}
};

#define LOAD_ASSERT(expression) \
	if ( !(expression) ){ throw new CLoadException (); }

#endif // GAMECLIENT_LOADEXCEPTION_H_