// AfxDebugPlus.cpp : prost�edky diagnostiky
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include <afxwin.h>
#include <stdarg.h>

#include "AfxDebugPlus.h"

/////////////////////////////////////////////////////////////////////////////

// prom�nn� ��d�c� automatick� ukon�ov�n� ��dk� diagnostick�ch zpr�v
BOOL afxTraceAutoEOLEnabled = FALSE;

// vyp�e diagnostickou zpr�vu v�etn� jm�na souboru a ��sla ��dky
void AFX_CDECL AfxTraceLine ( LPCSTR lpszFileName, int nLine, LPCTSTR lpszFormat, ... )
{
	// ve�ker� diagnostick� v�stup je ��zen afxTraceEnabled
	if ( !afxTraceEnabled )
		return;

	// vyp�e hlavi�ku diagnostick� zpr�vy
	afxDump << lpszFileName << _T("(") << nLine << _T("): ");

	// p�iprav� prom�nnou ��st parametr� funkce
	va_list args;
	va_start ( args, lpszFormat );

	int nBuf;
	TCHAR szBuffer[512];

	nBuf = _vsntprintf ( szBuffer, sizeof ( szBuffer ) / sizeof ( TCHAR ), lpszFormat, args );

	ASSERT ( nBuf >= 0 );

	if ( ( afxTraceFlags & traceMultiApp ) && ( AfxGetApp () != NULL) )
		afxDump << AfxGetApp ()->m_pszExeName << _T(": ");
	afxDump << szBuffer;

	// automatick� ukon�en� ��dk� diagnostick�ho v�stupu je ��zeno afxTraceAutoEOLEnabled
	if ( afxTraceAutoEOLEnabled && ( ( nBuf == 0 ) || ( szBuffer[nBuf-1] != _T('\n') ) ) )
		afxDump << _T("\n");

	// ukon�� pr�ci s prom�nnou ��st� parametr� funkce
	va_end ( args );
}

// vyp�e diagnostickou zpr�vu bez jm�na souboru a ��sla ��dky
void AFX_CDECL AfxTraceNext ( LPCTSTR lpszFormat, ... )
{
	// ve�ker� diagnostick� v�stup je ��zen afxTraceEnabled
	if ( !afxTraceEnabled )
		return;

	// p�iprav� prom�nnou ��st parametr� funkce
	va_list args;
	va_start ( args, lpszFormat );

	int nBuf;
	TCHAR szBuffer[512];

	nBuf = _vsntprintf ( szBuffer, sizeof ( szBuffer ) / sizeof ( TCHAR ), lpszFormat, args );

	ASSERT ( nBuf >= 0 );

	if ( ( afxTraceFlags & traceMultiApp ) && ( AfxGetApp () != NULL) )
		afxDump << AfxGetApp ()->m_pszExeName << _T(": ");
	afxDump << szBuffer;

	// automatick� ukon�en� ��dk� diagnostick�ho v�stupu je ��zeno afxTraceAutoEOLEnabled
	if ( afxTraceAutoEOLEnabled && ( ( nBuf == 0 ) || ( szBuffer[nBuf-1] != _T('\n') ) ) )
		afxDump << _T("\n");

	// ukon�� pr�ci s prom�nnou ��st� parametr� funkce
	va_end ( args );
}

// vyp�e diagnostickou zpr�vu selh�n� assertace
void AFXAPI AfxTraceAssertFailedLine ( LPCSTR lpszFileName, int nLine, LPCSTR lpszAssertation )
{
	// vyp�e diagnostickou zpr�vu selh�n� assertace
	AfxTraceLine ( lpszFileName, nLine, _T("Assertation failed: %s\n"), lpszAssertation );
}

// vyp�e diagnostickou zpr�vu bez jm�na souboru a ��sla ��dky
void SAfxTraceOutput::Output ( LPCTSTR lpszFormat, ... )
{
	// ve�ker� diagnostick� v�stup je ��zen afxTraceEnabled
	if ( !afxTraceEnabled )
		return;

	// p�iprav� prom�nnou ��st parametr� funkce
	va_list args;
	va_start ( args, lpszFormat );

	int nBuf;
	TCHAR szBuffer[512];

	nBuf = _vsntprintf ( szBuffer, sizeof ( szBuffer ) / sizeof ( TCHAR ), lpszFormat, args );

	ASSERT ( nBuf >= 0 );

	if ( ( afxTraceFlags & traceMultiApp ) && ( AfxGetApp () != NULL) )
		afxDump << AfxGetApp ()->m_pszExeName << ": ";
	afxDump << szBuffer;

	// automatick� ukon�en� ��dk� diagnostick�ho v�stupu je ��zeno afxTraceAutoEOLEnabled
	if ( afxTraceAutoEOLEnabled && ( ( nBuf == 0 ) || ( szBuffer[nBuf-1] != _T('\n') ) ) )
		afxDump << _T("\n");

	// ukon�� pr�ci s prom�nnou ��st� parametr� funkce
	va_end ( args );
}

// vyp�e diagnostickou zpr�vu v�etn� jm�na souboru a ��sla ��dky
SAfxTraceOutput AFXAPI AfxTraceLine ( LPCSTR lpszFileName, int nLine )
{
	// vyp�e hlavi�ku diagnostick� zpr�vy
	afxDump << lpszFileName << _T("(") << nLine << _T("): ");

	// vr�t� strukturu v�pisu diagnostick� zpr�vu bez jm�na souboru a ��sla ��dky
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

	// ujist� se, �e ukazatel na virtu�ln� tabulku funkc� je spr�vn�
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
