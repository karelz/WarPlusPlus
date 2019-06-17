/************************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: Tøída výjimky chyby bìhem nahrávání
 * 
 ***********************************************************/

#ifndef __PERSISTENT_LOAD_EXCEPTION__HEADER_INCLUDED__
#define __PERSISTENT_LOAD_EXCEPTION__HEADER_INCLUDED__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// exception class for loading persistent storage exceptions
class CPersistentLoadException : public CException 
{
public:

  // constructor
  CPersistentLoadException () {};
	// destructor
	virtual ~CPersistentLoadException () {};

public:

  // returns info about the exception
  virtual BOOL GetErrorMessage( LPTSTR lpszError, UINT nMaxError, 
    PUINT pnHelpContext = NULL ){
  	ASSERT ( ( lpszError != NULL ) && AfxIsValidString ( lpszError, nMaxError ) );

  	if ( pnHelpContext != NULL )
  		*pnHelpContext = 0;

	  CString strMessage ( "Loading Error" );
	  lstrcpyn ( lpszError, strMessage, nMaxError );

  	return TRUE;
  };
};

// macros to use the exception with
#define LOAD_ASSERT(condition) do { if ( !(condition) ) throw new CPersistentLoadException(); \
	} while ( 0 )

#endif //__PERSISTENT_LOAD_EXCEPTION__HEADER_INCLUDED__
