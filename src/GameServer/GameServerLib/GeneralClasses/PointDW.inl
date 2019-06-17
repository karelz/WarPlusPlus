/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Obecné tøídy a šablony
 *   Autor: Karel Zikmund
 * 
 *   Popis: Tøída bodu (x,y) (souøadnice typu: DWORD)
 * 
 ***********************************************************/

#ifndef __SERVER_POINTDW__INLINE_INCLUDED__
#define __SERVER_POINTDW__INLINE_INCLUDED__

#include "PointDW.h"

// Constructors
inline CPointDW::CPointDW () 
	{ /* random fill */ };
inline CPointDW::CPointDW ( DWORD dwInitX, DWORD dwInitY ) 
	{ x = dwInitX; y = dwInitY; }
inline CPointDW::CPointDW ( CPointDW &pointInit ) 
	{ x = pointInit.x; y = pointInit.y; }
inline CPointDW::CPointDW ( POINT pointInit ) 
	{ x = (DWORD)pointInit.x; y = (DWORD)pointInit.y; }
inline CPointDW::CPointDW ( CSizeDW sizeInit ) 
	{ x = sizeInit.cx; y = sizeInit.cy; }
inline CPointDW::CPointDW ( SIZE sizeInit ) 
	{ x = (DWORD)sizeInit.cx; y = (DWORD)sizeInit.cy; }

// Opearators of type casting
inline CPointDW::operator CPoint () 
	{ return CPoint ( (int)x, (int)y ); }
inline CPointDW::operator POINT () 
	{ return (POINT)CPoint ( (int)x, (int)y ); }

// Operations of comparison
inline BOOL CPointDW::operator == ( CPointDW point ) const 
	{ return ( x == point.x && y == point.y ); }
inline BOOL CPointDW::operator != ( CPointDW point ) const 
	{ return ( x != point.x || y != point.y ); }

// Translate the point
inline void CPointDW::Offset ( DWORD dwOffsetX, DWORD dwOffsetY ) 
	{ x += dwOffsetX; y += dwOffsetY; }
inline void CPointDW::Offset ( POINT pointOffset ) 
	{ x += pointOffset.x; y += pointOffset.y; }
inline void CPointDW::Offset ( CPointDW pointOffset ) 
	{ x += pointOffset.x; y += pointOffset.y; }
inline void CPointDW::Offset ( SIZE sizeOffset ) 
	{ x += sizeOffset.cx; y += sizeOffset.cy; }
inline void CPointDW::Offset ( CSizeDW sizeOffset ) 
	{ x += sizeOffset.cx; y += sizeOffset.cy; }

// Operations of change
inline const CPointDW &CPointDW::operator += ( CPointDW point ) 
	{ x += point.x; y += point.y; return *this; }
inline const CPointDW &CPointDW::operator -= ( CPointDW point ) 
	{ x -= point.x; y -= point.y; return *this; }
inline const CPointDW &CPointDW::operator += ( CSizeDW size ) 
	{ x += size.cx; y += size.cy; return *this; }
inline const CPointDW &CPointDW::operator -= ( CSizeDW size ) 
	{ x -= size.cx; y -= size.cy; return *this; }

// Operations with point without change
inline CPointDW CPointDW::operator + ( CPointDW point ) const 
	{ return CPointDW ( x + point.x, y + point.y ); }
inline CSizeDW CPointDW::operator - ( CPointDW point ) const 
	{ return CSizeDW ( x - point.x, y - point.y ); }
inline CSizeDW CPointDW::GetRectSize ( CPointDW point ) const 
{
	CSizeDW sizeRect;
	sizeRect.cx = ( x <= point.x ) ? ( point.x - x ) : ( x - point.x );
	sizeRect.cy = ( y <= point.y ) ? ( point.y - y ) : ( y - point.y );
	return sizeRect;
}
inline DWORD CPointDW::GetDistanceSquare ( CPointDW point ) const 
{
	CSizeDW sizeRect = GetRectSize ( point );
	return ( sizeRect.cx * sizeRect.cx + sizeRect.cy * sizeRect.cy );
}

// Operations with size CSizeDW
inline CPointDW CPointDW::operator + ( CSizeDW size ) const 
	{ return CPointDW ( x + size.cx, y + size.cy ); }
inline CPointDW CPointDW::operator - ( CSizeDW size ) const 
	{ return CPointDW ( x - size.cx, y - size.cy ); }

// Operations with rectangle CRectDW
//inline CRectDW CPointDW::operator + ( const CRectDW rect ) const 
//	{ return rect + *this; }
//inline CRectDW CPointDW::operator - ( const CRectDW rect ) const 
//	{ return ((CRectDW)rect) - *this; }

#endif //__SERVER_POINTDW__INLINE_INCLUDED__
