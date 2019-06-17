/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: Bitov� pole civilizac�
 * 
 ***********************************************************/

#ifndef __SERVER_CIVILIZATION_BIT_ARRAY__HEADER_INCLUDED__
#define __SERVER_CIVILIZATION_BIT_ARRAY__HEADER_INCLUDED__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "AbstractDataClasses\StaticBitArray.h"

#include "GameServerCompileSettings.h"

//////////////////////////////////////////////////////////////////////
// Bitov� pole civilizac� (tj. jeden bit pro ka�dou civilizaci)
class CSCivilizationBitArray : 
	public CStaticBitArray<DWORD,DWORD,CIVILIZATION_COUNT_MAX> 
{
// Metody
public:
// Konstrukce a destrukce

	// konstruktor
	CSCivilizationBitArray () {};
	// "copy" konstruktor
	inline CSCivilizationBitArray ( CSCivilizationBitArray *pCivilizations );
	// destruktor
	~CSCivilizationBitArray () {};

// Operace s bity

	// vr�t� p��znak, je-li n�kter� civilizace nastaven�
	inline BOOL IsSet () const;
	// vr�t� p��znak, je-li civilizace "dwCivilizationIndex" jedin� mo�n� nastaven�
	inline BOOL IsExclusive ( DWORD dwCivilizationIndex ) const;
	// sma�e v�echny bity
	inline void ClearAll ();
	// vr�t� XOR s bitov�m polem "cCivilizations"
	inline CSCivilizationBitArray operator ^ ( CSCivilizationBitArray &cCivilizations ) const;
	// provede OR s bitov�m polem "cCivilizations"
	inline CSCivilizationBitArray &operator |= ( CSCivilizationBitArray &cCivilizations );
};

#if CIVILIZATION_COUNT_MAX != 32
	#error Nen� implementov�no v�ce jak 32 civilizac� - kr�tk� doimplementov�n� zde!
#else	//CIVILIZATION_COUNT_MAX == 32

//////////////////////////////////////////////////////////////////////
// Konstrukce a destrukce
//////////////////////////////////////////////////////////////////////

// "copy" konstruktor
inline CSCivilizationBitArray::CSCivilizationBitArray ( 
	CSCivilizationBitArray *pCivilizations ) 
{
	// zkop�ruje obsah bitov�ho pole
	*GetData () = *pCivilizations->GetData ();
}

//////////////////////////////////////////////////////////////////////
// Operace s bity
//////////////////////////////////////////////////////////////////////

// vr�t� p��znak, je-li n�kter� civilizace nastaven�
inline BOOL CSCivilizationBitArray::IsSet () const 
{
	// zjist�, je-li n�kter� civilizace nastaven�
	return *GetData ();
}

// vr�t� p��znak, je-li civilizace "dwCivilizationIndex" jedin� mo�n� nastaven�
inline BOOL CSCivilizationBitArray::IsExclusive ( DWORD dwCivilizationIndex ) const
{
	ASSERT ( dwCivilizationIndex < CIVILIZATION_COUNT_MAX );
	// zjist�, je-li civilizace jedin� mo�n� nastaven�
	return !( ( *GetData () ) & ~(DWORD)( 0x01 << dwCivilizationIndex ) );
}

// sma�e v�echny bity
inline void CSCivilizationBitArray::ClearAll () 
{
	// sma�e v�echny bity
	*GetData () = 0;
}

// vr�t� XOR s bitov�m polem "cCivilizations"
inline CSCivilizationBitArray CSCivilizationBitArray::operator ^ ( 
	CSCivilizationBitArray &cCivilizations ) const 
{
	// vracen� bitov� pole
	CSCivilizationBitArray cNewCivilizations ( *this );
	// provede XOR s obsahem vracen�ho bitov�ho pole
	*cNewCivilizations.GetData () ^= *cCivilizations.GetData ();
	// vr�t� XOR bitov�ch pol�
	return cNewCivilizations;
}

// provede OR s bitov�m polem "cCivilizations"
inline CSCivilizationBitArray &CSCivilizationBitArray::operator |= ( 
	CSCivilizationBitArray &cCivilizations ) 
{
	// provede OR s obsahem bitov�ho pole
	*GetData () |= *cCivilizations.GetData ();
	// vr�t� s�m sebe po provedn�m ORu
	return *this;
}

#endif //CIVILIZATION_COUNT_MAX == 32

#endif //__SERVER_CIVILIZATION_BIT_ARRAY__HEADER_INCLUDED__
