// AfxDebugPlus.cpp : prostøedky diagnostiky
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include <afxwin.h>
#include <stdarg.h>

#include "AfxDebugPlus.h"

/////////////////////////////////////////////////////////////////////////////

// promìnná øídící automatické ukonèování øádkù diagnostických zpráv
BOOL afxTraceAutoEOLEnabled = FALSE;

// vypíše diagnostickou zprávu vèetnì jména souboru a èísla øádky
void AFX_CDECL AfxTraceLine ( LPCSTR lpszFileName, int nLine, LPCTSTR lpszFormat, ... )
{
	// veškerý diagnostický výstup je øízen afxTraceEnabled
	if ( !afxTraceEnabled )
		return;

	// vypíše hlavièku diagnostické zprávy
	afxDump << lpszFileName << _T("(") << nLine << _T("): ");

	// pøipraví promìnnou èást parametrù funkce
	va_list args;
	va_start ( args, lpszFormat );

	int nBuf;
	TCHAR szBuffer[512];

	nBuf = _vsntprintf ( szBuffer, sizeof ( szBuffer ) / sizeof ( TCHAR ), lpszFormat, args );

	ASSERT ( nBuf >= 0 );

	if ( ( afxTraceFlags & traceMultiApp ) && ( AfxGetApp () != NULL) )
		afxDump << AfxGetApp ()->m_pszExeName << _T(": ");
	afxDump << szBuffer;

	// automatické ukonèení øádkù diagnostického výstupu je øízeno afxTraceAutoEOLEnabled
	if ( afxTraceAutoEOLEnabled && ( ( nBuf == 0 ) || ( szBuffer[nBuf-1] != _T('\n') ) ) )
		afxDump << _T("\n");

	// ukonèí práci s promìnnou èástí parametrù funkce
	va_end ( args );
}

// vypíše diagnostickou zprávu bez jména souboru a èísla øádky
void AFX_CDECL AfxTraceNext ( LPCTSTR lpszFormat, ... )
{
	// veškerý diagnostický výstup je øízen afxTraceEnabled
	if ( !afxTraceEnabled )
		return;

	// pøipraví promìnnou èást parametrù funkce
	va_list args;
	va_start ( args, lpszFormat );

	int nBuf;
	TCHAR szBuffer[512];

	nBuf = _vsntprintf ( szBuffer, sizeof ( szBuffer ) / sizeof ( TCHAR ), lpszFormat, args );

	ASSERT ( nBuf >= 0 );

	if ( ( afxTraceFlags & traceMultiApp ) && ( AfxGetApp () != NULL) )
		afxDump << AfxGetApp ()->m_pszExeName << _T(": ");
	afxDump << szBuffer;

	// automatické ukonèení øádkù diagnostického výstupu je øízeno afxTraceAutoEOLEnabled
	if ( afxTraceAutoEOLEnabled && ( ( nBuf == 0 ) || ( szBuffer[nBuf-1] != _T('\n') ) ) )
		afxDump << _T("\n");

	// ukonèí práci s promìnnou èástí parametrù funkce
	va_end ( args );
}

// vypíše diagnostickou zprávu selhání assertace
void AFXAPI AfxTraceAssertFailedLine ( LPCSTR lpszFileName, int nLine, LPCSTR lpszAssertation )
{
	// vypíše diagnostickou zprávu selhání assertace
	AfxTraceLine ( lpszFileName, nLine, _T("Assertation failed: %s\n"), lpszAssertation );
}

// vypíše diagnostickou zprávu bez jména souboru a èísla øádky
void SAfxTraceOutput::Output ( LPCTSTR lpszFormat, ... )
{
	// veškerý diagnostický výstup je øízen afxTraceEnabled
	if ( !afxTraceEnabled )
		return;

	// pøipraví promìnnou èást parametrù funkce
	va_list args;
	va_start ( args, lpszFormat );

	int nBuf;
	TCHAR szBuffer[512];

	nBuf = _vsntprintf ( szBuffer, sizeof ( szBuffer ) / sizeof ( TCHAR ), lpszFormat, args );

	ASSERT ( nBuf >= 0 );

	if ( ( afxTraceFlags & traceMultiApp ) && ( AfxGetApp () != NULL) )
		afxDump << AfxGetApp ()->m_pszExeName << ": ";
	afxDump << szBuffer;

	// automatické ukonèení øádkù diagnostického výstupu je øízeno afxTraceAutoEOLEnabled
	if ( afxTraceAutoEOLEnabled && ( ( nBuf == 0 ) || ( szBuffer[nBuf-1] != _T('\n') ) ) )
		afxDump << _T("\n");

	// ukonèí práci s promìnnou èástí parametrù funkce
	va_end ( args );
}

// vypíše diagnostickou zprávu vèetnì jména souboru a èísla øádky
SAfxTraceOutput AFXAPI AfxTraceLine ( LPCSTR lpszFileName, int nLine )
{
	// vypíše hlavièku diagnostické zprávy
	afxDump << lpszFileName << _T("(") << nLine << _T("): ");

	// vrátí strukturu výpisu diagnostické zprávu bez jména souboru a èísla øádky
	return SAfxTraceOutput ();
}

// zkontroluje objekt
void AFXAPI AfxAssertValidObjectLine ( const CObject *pOb, LPCSTR lpszFileName, int nLine )
{
	if ( pOb == NULL )
	{
		AfxTraceLine ( lpszFileName, nLine, _T("ASSERT_VALID fails with NULL pointer.\n") );
		if ( AfxAssertFailedLine ( lpszFileName, nLine ) )
			AfxDebugBreak ();
		return;
	}
	if ( !AfxIsValidAddress ( pOb, sizeof ( CObject ) ) )
	{
		AfxTraceLine ( lpszFileName, nLine, _T("ASSERT_VALID fails with illegal pointer.\n") );
		if ( AfxAssertFailedLine ( lpszFileName, nLine ) )
			AfxDebugBreak ();
		return;
	}

	// ujistí se, že ukazatel na virtuální tabulku funkcí je správný
	ASSERT ( sizeof ( CObject ) == sizeof ( void * ) );
	if ( !AfxIsValidAddress ( *(void **)pOb, sizeof ( void * ), FALSE ) )
	{
		AfxTraceLine ( lpszFileName, nLine, _T("ASSERT_VALID fails with illegal vtable pointer.\n") );
		if ( AfxAssertFailedLine ( lpszFileName, nLine ) )
			AfxDebugBreak ();
		return;
	}

	if ( !AfxIsValidAddress ( pOb, pOb->GetRuntimeClass ()->m_nObjectSize, FALSE ) )
	{
		AfxTraceLine ( lpszFileName, nLine, _T("ASSERT_VALID fails with illegal pointer.\n") );
		if ( AfxAssertFailedLine ( lpszFileName, nLine ) )
			AfxDebugBreak ();
		return;
	}
	pOb->AssertValid ();
}
