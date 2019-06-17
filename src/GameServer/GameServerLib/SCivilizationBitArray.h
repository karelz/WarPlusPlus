/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: Bitové pole civilizací
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
// Bitové pole civilizací (tj. jeden bit pro každou civilizaci)
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

	// vrátí pøíznak, je-li nìkterá civilizace nastavená
	inline BOOL IsSet () const;
	// vrátí pøíznak, je-li civilizace "dwCivilizationIndex" jediná možná nastavená
	inline BOOL IsExclusive ( DWORD dwCivilizationIndex ) const;
	// smaže všechny bity
	inline void ClearAll ();
	// vrátí XOR s bitovým polem "cCivilizations"
	inline CSCivilizationBitArray operator ^ ( CSCivilizationBitArray &cCivilizations ) const;
	// provede OR s bitovým polem "cCivilizations"
	inline CSCivilizationBitArray &operator |= ( CSCivilizationBitArray &cCivilizations );
};

#if CIVILIZATION_COUNT_MAX != 32
	#error Není implementováno více jak 32 civilizací - krátké doimplementování zde!
#else	//CIVILIZATION_COUNT_MAX == 32

//////////////////////////////////////////////////////////////////////
// Konstrukce a destrukce
//////////////////////////////////////////////////////////////////////

// "copy" konstruktor
inline CSCivilizationBitArray::CSCivilizationBitArray ( 
	CSCivilizationBitArray *pCivilizations ) 
{
	// zkopíruje obsah bitového pole
	*GetData () = *pCivilizations->GetData ();
}

//////////////////////////////////////////////////////////////////////
// Operace s bity
//////////////////////////////////////////////////////////////////////

// vrátí pøíznak, je-li nìkterá civilizace nastavená
inline BOOL CSCivilizationBitArray::IsSet () const 
{
	// zjistí, je-li nìkterá civilizace nastavená
	return *GetData ();
}

// vrátí pøíznak, je-li civilizace "dwCivilizationIndex" jediná možná nastavená
inline BOOL CSCivilizationBitArray::IsExclusive ( DWORD dwCivilizationIndex ) const
{
	ASSERT ( dwCivilizationIndex < CIVILIZATION_COUNT_MAX );
	// zjistí, je-li civilizace jediná možná nastavená
	return !( ( *GetData () ) & ~(DWORD)( 0x01 << dwCivilizationIndex ) );
}

// smaže všechny bity
inline void CSCivilizationBitArray::ClearAll () 
{
	// smaže všechny bity
	*GetData () = 0;
}

// vrátí XOR s bitovým polem "cCivilizations"
inline CSCivilizationBitArray CSCivilizationBitArray::operator ^ ( 
	CSCivilizationBitArray &cCivilizations ) const 
{
	// vracené bitové pole
	CSCivilizationBitArray cNewCivilizations ( *this );
	// provede XOR s obsahem vraceného bitového pole
	*cNewCivilizations.GetData () ^= *cCivilizations.GetData ();
	// vrátí XOR bitových polí
	return cNewCivilizations;
}

// provede OR s bitovým polem "cCivilizations"
inline CSCivilizationBitArray &CSCivilizationBitArray::operator |= ( 
	CSCivilizationBitArray &cCivilizations ) 
{
	// provede OR s obsahem bitového pole
	*GetData () |= *cCivilizations.GetData ();
	// vrátí sám sebe po provedném ORu
	return *this;
}

#endif //CIVILIZATION_COUNT_MAX == 32

#endif //__SERVER_CIVILIZATION_BIT_ARRAY__HEADER_INCLUDED__
