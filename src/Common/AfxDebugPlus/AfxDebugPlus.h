// AfxDebugPlus.h : prost�edky diagnostiky
//
/////////////////////////////////////////////////////////////////////////////

#ifndef AFX_DEBUG_PLUS_INCLUDED
#define AFX_DEBUG_PLUS_INCLUDED

#include <afx.h>

#ifdef _DEBUG

// zni�� nahrazovan� prost�edky diagnostiky
#undef ASSERT
#undef ASSERT_VALID
#undef TRACE
#undef TRACE0
#undef TRACE1
#undef TRACE2
#undef TRACE3

// prom�nn� ��d�c� automatick� ukon�ov�n� ��dk� diagnostick�ch zpr�v
extern BOOL afxTraceAutoEOLEnabled;

// pomocn� struktura pro v�pis diagnostick� zpr�vy
typedef struct
{
	void Output ( LPCTSTR lpszFormat, ... );
} SAfxTraceOutput;

// vyp�e diagnostickou zpr�vu selh�n� assertace
void AFXAPI AfxTraceAssertFailedLine ( LPCSTR lpszFileName, int nLine, LPCSTR lpszAssertation  );

// vyp�e diagnostickou zpr�vu v�etn� jm�na souboru a ��sla ��dky
void AFX_CDECL AfxTraceLine ( LPCSTR lpszFileName, int nLine, LPCTSTR lpszFormat, ... );
void AFX_CDECL AfxTraceNext ( LPCTSTR lpszFormat, ... );
SAfxTraceOutput AFXAPI AfxTraceLine ( LPCSTR lpszFileName, int nLine );

// zkontroluje objekt
void AFXAPI AfxAssertValidObjectLine ( const CObject *pOb, LPCSTR lpszFileName, int nLine );

// makro diagnostiky
#define ASSERT(f) \
	do \
	{ \
		if ( !(f) ) \
		{ \
			AfxTraceAssertFailedLine ( THIS_FILE, __LINE__, #f ); \
			if ( AfxAssertFailedLine ( THIS_FILE, __LINE__ ) ) \
				AfxDebugBreak(); \
		} \
	} while (0) \

#define ASSERT_VALID(pOb)	( ::AfxAssertValidObjectLine ( pOb, THIS_FILE, __LINE__ ) )

// makro diagnostick�ho v�pisu
#define TRACE			( ::AfxTraceLine ( THIS_FILE, __LINE__ ) ).Output
#define TRACE_NEXT	::AfxTraceNext

// makra v�pisu diagnostick�ch zpr�v pro zp�tnou kompatabilitu - vyp�� diagnostickou
//		zpr�vu v�etn� jm�na souboru a ��sla ��dky
#define TRACE0(sz)					::AfxTraceLine ( THIS_FILE, __LINE__, _T("%s"), _T(sz) )
#define TRACE1(sz, p1)				::AfxTraceLine ( THIS_FILE, __LINE__, _T(sz), p1 )
#define TRACE2(sz, p1, p2)			::AfxTraceLine ( THIS_FILE, __LINE__, _T(sz), p1, p2 )
#define TRACE3(sz, p1, p2, p3)	::AfxTraceLine ( THIS_FILE, __LINE__, _T(sz), p1, p2, p3 )
#define TRACE0_NEXT(sz)						::AfxTraceNext ( _T("%s"), _T(sz) )
#define TRACE1_NEXT(sz, p1)				::AfxTraceNext ( _T(sz), p1 )
#define TRACE2_NEXT(sz, p1, p2)			::AfxTraceNext ( _T(sz), p1, p2 )
#define TRACE3_NEXT(sz, p1, p2, p3)		::AfxTraceNext ( _T(sz), p1, p2, p3 )

#else //!_DEBUG

#define TRACE_NEXT							TRACE
#define TRACE0_NEXT(sz)						TRACE0 ( sz )
#define TRACE1_NEXT(sz, p1)				TRACE1 ( sz, p1 )
#define TRACE2_NEXT(sz, p1, p2)			TRACE2 ( sz, p1, p2 )
#define TRACE3_NEXT(sz, p1, p2, p3)		TRACE3 ( sz, p1, p2, p3 )

#endif //_DEBUG

#endif //AFX_DEBUG_PLUS_INCLUDED
