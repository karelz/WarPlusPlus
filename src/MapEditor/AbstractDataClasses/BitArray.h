// BitArray.h: implementation of the CBitArray class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __BIT_ARRAY__HEADER_INCLUDED__
#define __BIT_ARRAY__HEADER_INCLUDED__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxwin.h>

#include <memory.h>

//////////////////////////////////////////////////////////////////////
// T¯Ìda bitovÈho pole pevnÈ dÈlky (tj. jeho velikost je zn·m· p¯i 
//		vytv·¯enÌ instance objektu)
class CBitArray
{
// Metody
public:
// Konstrukce a destrukce

	// konstruktor
	CBitArray ();
	// destruktor
	~CBitArray ();

// Inicializace a zniËenÌ dat objektu

	// inicializuje bitovÈ pole velikosti "dwSize" na hodnotu "bValue"
	//		v˝jimky: CMemoryException
	void Create ( DWORD dwSize, BOOL bValue = FALSE );
	// zniËÌ bitovÈ pole
	void Delete ();

// Operace s bity

	// vr·tÌ hodnotu bitu "dwIndex" v poli bit˘
	BOOL operator [] ( DWORD dwIndex ) const;
	// vr·tÌ hodnotu bitu "dwIndex" v poli bit˘
	BOOL GetAt ( DWORD dwIndex ) const;
	// nastavÌ bit "dwIndex" (na hodnotu 1)
	void SetAt ( DWORD dwIndex );
	// vynuluje bit "dwIndex" (na hodnotu 0)
	void ClearAt ( DWORD dwIndex );
	// nastavÌ vöechny bity na hodnotu "bValue"
	void SetAt ( DWORD dwIndex, BOOL bNewValue );

// Data
private:
	// ukazatel na pole bit˘
	BYTE *m_pBitArray;

#ifdef _DEBUG
	// velikost pole bit˘
	DWORD m_dwSize;
#endif //_DEBUG
};

//////////////////////////////////////////////////////////////////////
// Konstrukce a destrukce
//////////////////////////////////////////////////////////////////////

// konstruktor
inline CBitArray::CBitArray () 
{
	// zneökodnÌ data objektu
	m_pBitArray = NULL;
#ifdef _DEBUG
	m_dwSize = 0;
#endif //_DEBUG
}

// destruktor
inline CBitArray::~CBitArray ()
{
	ASSERT ( m_pBitArray == NULL );
	ASSERT ( m_dwSize == 0 );
}

//////////////////////////////////////////////////////////////////////
// Inicializace a zniËenÌ dat objektu
//////////////////////////////////////////////////////////////////////

// inicializuje bitovÈ pole na hodnotu "bValue"
//		v˝jimky: CMemoryException
inline void CBitArray::Create ( DWORD dwSize, BOOL bValue ) 
{
	ASSERT ( m_pBitArray == NULL );
	ASSERT ( m_dwSize == 0 );

	// alokuje pot¯ebnou pamÏù pro bitovÈ pole
	if ( ( m_pBitArray = (BYTE *)malloc ( (dwSize+7)/8 ) ) == NULL )
	{	// nepoda¯ilo se alokovat pot¯ebnou pamÏù
		AfxThrowMemoryException ();
	}

#ifdef _DEBUG
	// inicializuje velikost alokovanÈho pole
	m_dwSize = dwSize;
#endif //_DEBUG

	// vyplnÌ pole bit˘ p¯Ìsluönou hodnotou
	memset ( m_pBitArray, ( bValue ? 0xff : 0x00 ), (dwSize+7)/8 );
}

// zniËÌ bitovÈ pole
inline void CBitArray::Delete () 
{
	ASSERT ( m_pBitArray != NULL );
	ASSERT ( m_dwSize > 0 );

	// zniËÌ alokovanou pamÏù
	free ( m_pBitArray );

	// zneökodnÌ data objektu
	m_pBitArray = NULL;
#ifdef _DEBUG
	m_dwSize = 0;
#endif //_DEBUG
}

//////////////////////////////////////////////////////////////////////
// Operace s bity
//////////////////////////////////////////////////////////////////////

// vr·tÌ hodnotu bitu "dwIndex" v poli bit˘
inline BOOL CBitArray::operator [] ( DWORD dwIndex ) const 
{
	ASSERT ( m_pBitArray != NULL );
	ASSERT ( dwIndex < m_dwSize );

	return ( m_pBitArray[dwIndex/8] & ( 0x01 << ( dwIndex & 0x07 ) ) ) ? TRUE : FALSE;
}

// vr·tÌ hodnotu bitu "dwIndex" v poli bit˘
inline BOOL CBitArray::GetAt ( DWORD dwIndex ) const 
{
	ASSERT ( m_pBitArray != NULL );
	ASSERT ( dwIndex < m_dwSize );

	return ( m_pBitArray[dwIndex/8] & ( 0x01 << ( dwIndex & 0x07 ) ) ) ? TRUE : FALSE;
}

// nastavÌ bit "dwIndex" (na hodnotu 1)
inline void CBitArray::SetAt ( DWORD dwIndex ) 
{
	ASSERT ( m_pBitArray != NULL );
	ASSERT ( dwIndex < m_dwSize );

	// zachov· hodnoty okolnÌch bit˘ a poûadovan˝ bit nastavÌ
	m_pBitArray[dwIndex/8] |= (BYTE)( 0x01 << ( dwIndex & 0x07 ) );
}

// vynuluje bit "dwIndex" (na hodnotu 0)
inline void CBitArray::ClearAt ( DWORD dwIndex ) 
{
	ASSERT ( m_pBitArray != NULL );
	ASSERT ( dwIndex < m_dwSize );

	// zachov· hodnoty okolnÌch bit˘ a poûadovan˝ bit vynuluje
	m_pBitArray[dwIndex/8] &= ~(BYTE)( 0x01 << ( dwIndex & 0x07 ) );
}

// nastavÌ hodnotu bitu "dwIndex" v poli bit˘ na novou hodnotu "bNewValue"
inline void CBitArray::SetAt ( DWORD dwIndex, BOOL bNewValue ) 
{
	ASSERT ( m_pBitArray != NULL );
	ASSERT ( dwIndex < m_dwSize );

	// zjistÌ, je-li nov· hodnota 1
	if ( bNewValue )
	{	// nov· hodnota bitu m· b˝t 1
		// zachov· hodnoty okolnÌch bit˘ a poûadovan˝ bit nastavÌ
		m_pBitArray[dwIndex/8] |= (BYTE)( 0x01 << ( dwIndex & 0x07 ) );
	}
	else
	{	// nov· hodnota bitu m· b˝t 0
		// zachov· hodnoty okolnÌch bit˘ a poûadovan˝ bit vynuluje
		m_pBitArray[dwIndex/8] &= ~(BYTE)( 0x01 << ( dwIndex & 0x07 ) );
	}
}

#endif //__BIT_ARRAY__HEADER_INCLUDED__
