/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Obecné tøídy a šablony
 *   Autor: Karel Zikmund
 * 
 *   Popis: Tøída bodu (x,y) (souøadnice typu: DWORD)
 * 
 ***********************************************************/

#ifndef __SERVER_POINTDW__HEADER_INCLUDED__
#define __SERVER_POINTDW__HEADER_INCLUDED__

//////////////////////////////////////////////////////////////////////
// Tøída bodu (x,y) (souøadnice typu: DWORD)
class CPointDW 
{
// methods
public:
// Constructors
	// create an unincialized point
	inline CPointDW ();
	// create from two DWORDs
	inline CPointDW ( DWORD dwInitX, DWORD dwInitY );
	// create from another point
	inline CPointDW ( CPointDW &pointInit );
	// create from another point
	inline CPointDW ( POINT pointInit );
	// create from a size
	inline CPointDW ( CSizeDW sizeInit );
	// create from a size
	inline CPointDW ( SIZE sizeInit );

// Operators of type casting
	// type casting to CPoint
	inline operator CPoint ();
	// type casting to POINT
	inline operator POINT ();

// Operations of comparison
	// compare with another point for equality
	inline BOOL operator == ( CPointDW point ) const;
	// compare with another point for nonequality
	inline BOOL operator != ( CPointDW point ) const;

// Translate the point
	// addition of offset to point
	inline void Offset ( DWORD dwOffsetX, DWORD dwOffsetY );
	// addition of offset to point
	inline void Offset ( CPointDW pointOffset );
	// addition of offset to point
	inline void Offset ( POINT pointOffset );
	// addition of offset to point
	inline void Offset ( CSizeDW sizeOffset );
	// addition of offset to point
	inline void Offset ( SIZE sizeOffset );

// Operations of change
	// addition of another point
	inline const CPointDW &operator += ( CPointDW point );
	// subtraction of another point
	inline const CPointDW &operator -= ( CPointDW point );
	// addition of another size
	inline const CPointDW &operator += ( CSizeDW size );
	// subtraction of another size
	inline const CPointDW &operator -= ( CSizeDW size );

// Operations with point without change
	// addition with another point
	inline CPointDW operator + ( CPointDW point ) const;
	// subtraction with another point
	inline CSizeDW operator - ( CPointDW point ) const;
	// get size of rectangle with another point
	inline CSizeDW GetRectSize ( CPointDW point ) const;
	// distance square from another point
	inline DWORD GetDistanceSquare ( CPointDW point ) const;

// Operations with size CSizeDW
	// addition with another size
	inline CPointDW operator + ( CSizeDW size ) const;
	// subtraction with another size
	inline CPointDW operator - ( CSizeDW size ) const;

// Operations with rectangle CRectDW
	// addition with rectangle
//	CRectDW operator + ( const CRectDW rect ) const;
	// subtraction with rectangle
//	CRectDW operator - ( const CRectDW rect ) const;

// data
public:
	// x and y proportions - names are compatible with CPoint and POINT stuctures
	DWORD x, y;
};

#endif //__SERVER_POINTDW__HEADER_INCLUDED__
