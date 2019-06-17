/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Obecné tøídy a šablony
 *   Autor: Karel Zikmund
 * 
 *   Popis: Tøída velikosti (cx,cy) (souøadnice typu: DWORD)
 * 
 ***********************************************************/

#ifndef __SERVER_SIZEDW__INLINE_INCLUDED__
#define __SERVER_SIZEDW__INLINE_INCLUDED__

#include "SizeDW.h"

// Constructors
inline CSizeDW::CSizeDW ()
	{ /* random fill */ };
inline CSizeDW::CSizeDW ( DWORD dwInitCX, DWORD dwInitCY )
	{ cx = dwInitCX; cy = dwInitCY; }
inline CSizeDW::CSizeDW ( CSizeDW &sizeInit )
	{ cx = sizeInit.cx; cy = sizeInit.cy; }
inline CSizeDW::CSizeDW ( SIZE sizeInit )
	{ cx = (DWORD)sizeInit.cx; cy = (DWORD)sizeInit.cy; }
inline CSizeDW::CSizeDW ( POINT pointInit )
	{ cx = (DWORD)pointInit.x; cy = (DWORD)pointInit.y; }

// Opearators of type casting
inline CSizeDW::operator CSize ()
	{ return CSize ( (int)cx, (int)cy ); }
inline CSizeDW::operator SIZE ()
	{ return (SIZE)CSize ( (int)cx, (int)cy ); }

// Operations of comparison
inline BOOL CSizeDW::operator == ( CSizeDW size ) const
	{ return ( cx == size.cx && cy == size.cy ); }
inline BOOL CSizeDW::operator != ( CSizeDW size ) const
	{ return ( cx != size.cx || cy != size.cy ); }

// Operations of change
inline const CSizeDW &CSizeDW::operator += ( CSizeDW size )
	{ cx += size.cx; cy += size.cy; return *this; }
inline const CSizeDW &CSizeDW::operator -= ( CSizeDW size )
	{ cx -= size.cx; cy -= size.cy; return *this; }

// Operations with size without change
inline CSizeDW CSizeDW::operator + ( CSizeDW size ) const
	{ return CSizeDW ( cx + size.cx, cy + size.cy ); }
inline CSizeDW CSizeDW::operator - ( CSizeDW size ) const
	{ return CSizeDW ( cx - size.cx, cy - size.cy ); }

// Operations with point CPointDW
inline CPointDW CSizeDW::operator + ( CPointDW point ) const
	{ return CPointDW ( cx + point.x, cy + point.y ); }
inline CPointDW CSizeDW::operator - ( CPointDW point ) const
	{ return CPointDW ( cx - point.x, cy - point.y ); }

// Operations with rectangle CRectDW
//inline CRectDW CSizeDW::operator + ( const CRectDW rect ) const
//	{ return rect + *this; }
//inline CRectDW CSizeDW::operator - ( const CRectDW rect ) const
//	{ return ((CRectDW)rect) - *this; }

#endif //__SERVER_SIZEDW__INLINE_INCLUDED__
