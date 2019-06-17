/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Obecné tøídy a šablony
 *   Autor: Karel Zikmund
 * 
 *   Popis: Tøída velikosti (cx,cy) (souøadnice typu: DWORD)
 * 
 ***********************************************************/

#ifndef __SERVER_SIZEDW__HEADER_INCLUDED__
#define __SERVER_SIZEDW__HEADER_INCLUDED__

//////////////////////////////////////////////////////////////////////
// Tøída velikosti (cx,cy) (souøadnice typu: DWORD)
class CSizeDW
{
// methods
public:
// Constructors
	// create an unincialized size
	CSizeDW ();
	// create from two DWORDs
	CSizeDW ( DWORD dwInitCX, DWORD dwInitCY );
	// create from another size
	CSizeDW ( CSizeDW &sizeInit );
	// create from another size
	CSizeDW ( SIZE sizeInit );
	// create from a point
	CSizeDW ( POINT pointInit );

// Operators of type casting
	// type casting to CSize
	operator CSize ();
	// type casting to SIZE
	operator SIZE ();

// Operations of comparison
	// compare with another size for equality
	BOOL operator == ( CSizeDW size ) const;
	// compare with another size for nonequality
	BOOL operator != ( CSizeDW size ) const;

// Operations of change
	// addition of another size
	const CSizeDW &operator += ( CSizeDW size );
	// subtraction of another size
	const CSizeDW &operator -= ( CSizeDW size );

// Operations with size without change
	// addition with another size
	CSizeDW operator + ( CSizeDW size ) const;
	// subtraction with another size
	CSizeDW operator - ( CSizeDW size ) const;

// Operations with point CPointDW
	// addition with point
	CPointDW operator + ( CPointDW point ) const;
	// subtraction with point
	CPointDW operator - ( CPointDW point ) const;

// Operations with rectangle CRectDW
	// addition with rectangle
//	CRectDW operator + ( const CRectDW rect ) const;
	// subtraction with rectangle
//	CRectDW operator - ( const CRectDW rect ) const;

// data
public:
	// x and y proportions - names are compatible with CSize and SIZE stuctures
	DWORD cx, cy;
};

#endif //__SERVER_SIZEDW__HEADER_INCLUDED__
