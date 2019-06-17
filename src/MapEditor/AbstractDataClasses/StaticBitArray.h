// StaticBitArray.h: implementation of the CStaticBitArray template.
//
//////////////////////////////////////////////////////////////////////

#ifndef __STATIC_BIT_ARRAY__HEADER_INCLUDED__
#define __STATIC_BIT_ARRAY__HEADER_INCLUDED__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <memory.h>

//////////////////////////////////////////////////////////////////////
// Šablona statického bitového pole (tj. jeho velikost je známá v dobì 
//		kompilace).
template <class IndexType, DWORD nSize>
class CStaticBitArray
{
// Metody
public:
// Konstrukce a destrukce

	// konstruktor
	CStaticBitArray () {};
	// destruktor
	~CStaticBitArray () {};

// Operace s bity

	// vrátí hodnotu bitu "index" v poli bitù
	BOOL operator [] ( IndexType index ) const;
	// vrátí hodnotu bitu "index" v poli bitù
	BOOL GetAt ( IndexType index ) const;
	// nastaví bit "index" (na hodnotu 1)
	void SetAt ( IndexType index );
	// vynuluje bit "index" (na hodnotu 0)
	void ClearAt ( IndexType index );
	// nastaví hodnotu bitu "index" v poli bitù na novou hodnotu "bNewValue"
	void SetAt ( IndexType index, BOOL bNewValue );
	// nastaví hodnoty všech bitù pole na hodnotu "bValue"
	void FillArray ( BOOL bValue = FALSE );

// Operace s celým polem bitù

	// vrátí velikost pole bitù (v bytech)
	DWORD GetDataSize () { return ((nSize+7)/ 8); };
	// vrátí ukazatel na pole bitù (za úèelem save/load pole)
	BYTE *GetData () { return m_pBitArray; };

// Data
private:
	// pole bitù
	BYTE m_pBitArray[(nSize+7)/8];
};

//////////////////////////////////////////////////////////////////////
// Operace s bity
//////////////////////////////////////////////////////////////////////

// vrátí hodnotu bitu "index" v poli bitù
template <class IndexType, DWORD nSize>
inline BOOL CStaticBitArray<IndexType,nSize>::operator [] ( IndexType index ) const 
{
	return ( m_pBitArray[index/8] & ( 0x01 << ( index & 0x07 ) ) ) ? TRUE : FALSE;
};

// vrátí hodnotu bitu "index" v poli bitù
template <class IndexType, DWORD nSize>
inline BOOL CStaticBitArray<IndexType,nSize>::GetAt ( IndexType index ) const 
{
	return ( m_pBitArray[index/8] & ( 0x01 << ( index & 0x07 ) ) ) ? TRUE : FALSE;
};

// nastaví bit "index" (na hodnotu 1)
template <class IndexType, DWORD nSize>
inline void CStaticBitArray<IndexType,nSize>::SetAt ( IndexType index ) 
{
	// zachová hodnoty okolních bitù a požadovaný bit nastaví
	m_pBitArray[index/8] |= (BYTE)( 0x01 << ( index & 0x07 ) );
};

// vynuluje bit "index" (na hodnotu 0)
template <class IndexType, DWORD nSize>
inline void CStaticBitArray<IndexType,nSize>::ClearAt ( IndexType index ) 
{
	// zachová hodnoty okolních bitù a požadovaný bit vynuluje
	m_pBitArray[index/8] &= ~(BYTE)( 0x01 << ( index & 0x07 ) );
};

// nastaví hodnotu bitu "index" v poli bitù na novou hodnotu "bNewValue"
template <class IndexType, DWORD nSize>
inline void CStaticBitArray<IndexType,nSize>::SetAt ( IndexType index, BOOL bNewValue ) 
{
	// zjistí, je-li nová hodnota 1
	if ( bNewValue )
	{	// nová hodnota bitu má být 1
		// zachová hodnoty okolních bitù a požadovaný bit nastaví
		m_pBitArray[index/8] |= (BYTE)( 0x01 << ( index & 0x07 ) );
	}
	else
	{	// nová hodnota bitu má být 0
		// zachová hodnoty okolních bitù a požadovaný bit vynuluje
		m_pBitArray[index/8] &= ~(BYTE)( 0x01 << ( index & 0x07 ) );
	}
};

// nastaví hodnoty všech bitù pole na hodnotu "bValue"
template <class IndexType, DWORD nSize>
inline void CStaticBitArray<IndexType,nSize>::FillArray ( BOOL bValue ) 
{
	// vyplní pole bitù pøíslušnou hodnotou
	memset ( m_pBitArray, ( bValue ? 0xff : 0x00 ), (nSize+7)/8 );
};

#endif //__STATIC_BIT_ARRAY__HEADER_INCLUDED__
